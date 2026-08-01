/* main.c — Katiztic.
 *
 * A cozy, ethereal cat game with a GBA soul. You keep a family of up to five
 * cats, each its own type and color, and care for them across a cottage home
 * and the meadow outside — feeding, grooming, petting, and sleeping to a
 * fresh morning. Everything is driven by taps, so it's already touch-shaped.
 *
 * The rendering trick: everything draws in a 240x160 logical space, and SDL's
 * integer-scale presentation blows it up to the window with crisp, square
 * pixels — the honest GBA look, no blur.
 */
#include <SDL3/SDL.h>
#include "render.h"
#include "palette.h"
#include "scene.h"
#include "cat.h"
#include "cattype.h"
#include "stats.h"
#include "roster.h"
#include "ui.h"
#include "cottage.h"

/* Where the player currently is. Sleeping happens in the cottage; the meadow
 * is the outdoors. Moving between them is a tap on an on-screen button. */
typedef enum { LOC_COTTAGE, LOC_MEADOW } Location;

/* Window opens at 4x the logical canvas: 960x640. */
#define KZ_SCALE  4
#define WIN_W     (KZ_W * KZ_SCALE)
#define WIN_H     (KZ_H * KZ_SCALE)

/* Where the cat's stats are saved between sessions. */
#define KZ_SAVE_PATH "katiztic.sav"

/* Fixed timestep so the mood animations run the same on any machine. */
#define KZ_FPS       60
#define FRAME_NS     (SDL_NS_PER_SECOND / KZ_FPS)

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window   *window   = NULL;
    SDL_Renderer *renderer = NULL;
    if (!SDL_CreateWindowAndRenderer("Katiztic", WIN_W, WIN_H,
                                     0, &window, &renderer)) {
        SDL_Log("CreateWindowAndRenderer failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    /* The GBA scaling: draw at 240x160, present at integer multiples. */
    SDL_SetRenderLogicalPresentation(renderer, KZ_W, KZ_H,
                                     SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);

    /* Needed for the soft alpha overlays (shadow, hearts, mood wash). */
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    Meadow meadow = meadow_make();

    /* The spot in the scene where the active cat sits. */
    const float CAT_X = 112.0f, CAT_Y = 118.0f;

    /* Start at home in the cottage — you wake up here. */
    Location location = LOC_COTTAGE;

    /* On-screen buttons, top-right. One is a toggle (home<->out); the sleep
     * button only shows in the cottage. Positions in logical 240x160 space. */
    Button btn_travel = { KZ_W - 24, 4,  20, 16, KZ_BTN_OUT };
    Button btn_sleep  = { KZ_W - 48, 4,  20, 16, KZ_BTN_SLEEP };
    int    press_fx   = 0;   /* frames of button-press highlight remaining */

    /* Load the family, or start with two starter cats if there's no save. */
    Roster roster;
    if (!roster_load(&roster, KZ_SAVE_PATH, CAT_X, CAT_Y)) {
        roster = roster_new(CAT_X, CAT_Y);
    }

    /* Rename mode: when active, keystrokes edit `edit_buf` instead of doing
     * their usual jobs. Tapping the name on the stat card starts it; Enter
     * confirms, Escape cancels. Physical keyboard for now — forward-compatible
     * with the iOS on-screen keyboard when that port comes. */
    bool editing = false;
    char edit_buf[KZ_NAME_LEN];
    edit_buf[0] = '\0';
    int  edit_len = 0;

    bool   running = true;
    Uint64 frame   = 0;
    Uint64 next    = SDL_GetTicksNS();

    while (running) {
        /* ---- input ---- */
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            /* ---- rename mode intercepts input ---- */
            if (editing) {
                if (e.type == SDL_EVENT_QUIT) { running = false; }
                else if (e.type == SDL_EVENT_TEXT_INPUT) {
                    /* Append typed characters that fit (leave room for '\0'). */
                    for (const char *p = e.text.text; *p; p++) {
                        if (edit_len < KZ_NAME_LEN - 1) {
                            edit_buf[edit_len++] = *p;
                            edit_buf[edit_len] = '\0';
                        }
                    }
                } else if (e.type == SDL_EVENT_KEY_DOWN) {
                    if (e.key.key == SDLK_BACKSPACE && edit_len > 0) {
                        edit_buf[--edit_len] = '\0';
                    } else if (e.key.key == SDLK_RETURN) {
                        roster_rename(&roster, roster.active, edit_buf);
                        roster_save(&roster, KZ_SAVE_PATH);
                        editing = false;
                        SDL_StopTextInput(window);
                    } else if (e.key.key == SDLK_ESCAPE) {
                        editing = false;      /* cancel, keep old name */
                        SDL_StopTextInput(window);
                    }
                }
                continue;   /* while editing, nothing else sees the event */
            }

            switch (e.type) {
            case SDL_EVENT_QUIT:
                running = false;
                break;

            case SDL_EVENT_KEY_DOWN:
                if (e.key.key == SDLK_ESCAPE || e.key.key == SDLK_Q)
                    running = false;
                else if (e.key.key == SDLK_SPACE || e.key.key == SDLK_T)
                    meadow_cycle_time(&meadow);
                else if (e.key.key == SDLK_F)
                    stats_feed(&roster_active(&roster)->stats);
                else if (e.key.key == SDLK_G)
                    stats_groom(&roster_active(&roster)->stats);
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN: {
                /* Window pixels -> 240x160 logical space, then hit-test.
                 * This point is a "tap" — identical whether it came from a
                 * mouse click (now) or a finger (on iOS later). */
                float lx, ly;
                SDL_RenderCoordinatesFromWindow(renderer, e.button.x,
                                                e.button.y, &lx, &ly);

                /* 1) tap the name on the stat card -> start renaming */
                if (ui_name_hit(4, 4, lx, ly)) {
                    editing = true;
                    SDL_strlcpy(edit_buf, roster_active(&roster)->name,
                                KZ_NAME_LEN);
                    edit_len = (int)SDL_strlen(edit_buf);
                    SDL_StartTextInput(window);
                    break;
                }

                /* 2) roster strip: select a cat, or adopt a new one. */
                int slot = ui_roster_hit(&roster, lx, ly);
                if (slot == -2) {
                    /* adopt: cycle through types by how many you have */
                    CatType t = (CatType)(roster.count % KZ_TYPE_COUNT);
                    roster_adopt(&roster, t, CAT_X, CAT_Y);
                    press_fx = 8;
                    break;
                } else if (slot >= 0) {
                    roster_select(&roster, slot);
                    press_fx = 8;
                    break;
                }

                /* 3) travel button: toggle cottage <-> meadow */
                if (ui_button_hit(&btn_travel, lx, ly)) {
                    location = (location == LOC_COTTAGE) ? LOC_MEADOW
                                                         : LOC_COTTAGE;
                    btn_travel.kind = (location == LOC_COTTAGE) ? KZ_BTN_OUT
                                                                : KZ_BTN_HOME;
                    press_fx = 8;
                }
                /* 4) sleep button (cottage only): fresh morning + save */
                else if (location == LOC_COTTAGE
                         && ui_button_hit(&btn_sleep, lx, ly)) {
                    meadow.time = KZ_DAWN;
                    roster_active(&roster)->stats.energy = KZ_STAT_MAX;
                    roster_save(&roster, KZ_SAVE_PATH);
                    press_fx = 8;
                }
                /* 5) tapping the bed also sleeps */
                else if (location == LOC_COTTAGE && cottage_bed_hit(lx, ly)) {
                    meadow.time = KZ_DAWN;
                    roster_active(&roster)->stats.energy = KZ_STAT_MAX;
                    roster_save(&roster, KZ_SAVE_PATH);
                    press_fx = 8;
                }
                /* 6) tap a cat. In the cottage the whole family is present, so
                 * a tap can land on any of them: that cat becomes active and
                 * gets petted. In the meadow, only the active cat is there. */
                else {
                    bool hit_one = false;
                    if (location == LOC_COTTAGE) {
                        for (int i = 0; i < roster.count; i++) {
                            if (cat_hit(&roster.cats[i].anim, lx, ly)) {
                                roster_select(&roster, i);
                                cat_pet(&roster.cats[i].anim);
                                stats_pet(&roster.cats[i].stats);
                                hit_one = true;
                                break;
                            }
                        }
                    }
                    if (!hit_one) {
                        OwnedCat *a = roster_active(&roster);
                        if (cat_hit(&a->anim, lx, ly)) {
                            cat_pet(&a->anim);
                            stats_pet(&a->stats);   /* petting deepens bond */
                        }
                    }
                }
                break;
            }
            default: break;
            }
        }

        /* ---- update ---- */
        if (location == LOC_MEADOW) meadow_update(&meadow);
        OwnedCat *active = roster_active(&roster);
        /* Animate every cat (they all breathe and blink at home); the active
         * one also carries any petting glow. */
        for (int i = 0; i < roster.count; i++)
            cat_update(&roster.cats[i].anim);
        if (press_fx > 0) press_fx--;

        /* ---- draw (back to front) ---- */
        SDL_SetRenderDrawColor(renderer, KZ_CLOUD.r, KZ_CLOUD.g, KZ_CLOUD.b, 255);
        SDL_RenderClear(renderer);

        CatColors col = cattype_colors(active->type);
        bool is_night = (meadow.time == KZ_NIGHT);
        if (location == LOC_COTTAGE) {
            cottage_draw(renderer, frame, is_night);
            /* The whole family lounges at home. Draw each cat at its spot,
             * back-to-front by row so nearer cats overlap farther ones, and
             * draw the active cat last so she sits on top with her hearts. */
            for (int i = 0; i < roster.count; i++) {
                if (i == roster.active) continue;
                float hx, hy;
                roster_home_spot(i, &hx, &hy);
                roster.cats[i].anim.cx = hx;
                roster.cats[i].anim.cy = hy;
                cat_draw(renderer, &roster.cats[i].anim,
                         cattype_colors(roster.cats[i].type), frame);
            }
            float ax, ay;
            roster_home_spot(roster.active, &ax, &ay);
            active->anim.cx = ax;
            active->anim.cy = ay;
            cat_draw(renderer, &active->anim, col, frame);
        } else {
            /* Outdoors is a one-cat outing: just the active cat, at the
             * meadow spot. */
            active->anim.cx = CAT_X;
            active->anim.cy = CAT_Y;
            meadow_draw(renderer, &meadow, frame);
            cat_draw(renderer, &active->anim, col, frame);
            meadow_draw_wash(renderer, &meadow);   /* mood overlay, on top */
        }

        /* ---- UI (both locations) ---- */
        ui_draw_panel(renderer, active, 4, 4, editing, edit_buf, frame);
        ui_button_draw(renderer, &btn_travel, press_fx > 0);
        if (location == LOC_COTTAGE)
            ui_button_draw(renderer, &btn_sleep, press_fx > 0);
        ui_roster_draw(renderer, &roster);                /* the family strip   */

        SDL_RenderPresent(renderer);

        /* ---- pace to 60fps without busy-spinning ---- */
        frame++;
        next += FRAME_NS;
        Uint64 now = SDL_GetTicksNS();
        if (next > now) SDL_DelayNS(next - now);
        else            next = now;   /* fell behind; don't spiral */
    }

    /* Save the whole family so they remember you next time. */
    roster_save(&roster, KZ_SAVE_PATH);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}