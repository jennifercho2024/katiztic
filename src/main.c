/* main.c — Katiztic, the meadow vibe slice.
 *
 * A cozy, ethereal cat game with a GBA soul. This slice: a pastel meadow you
 * can watch, a cat you can pet, and a time-of-day you can shift through dawn,
 * noon, dusk, and night.
 *
 *   controls
 *     click / tap the cat ..... pet her (purr + hearts)
 *     space / T ............... shift time of day
 *     esc / Q ................. quit
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
#include "stats.h"
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
    Cat    cat    = cat_make(112.0f, 118.0f);

    /* Start at home in the cottage — you wake up here. */
    Location location = LOC_COTTAGE;

    /* On-screen buttons, top-right. One is a toggle (home<->out); the sleep
     * button only shows in the cottage. Positions in logical 240x160 space. */
    Button btn_travel = { KZ_W - 24, 4,  20, 16, KZ_BTN_OUT };
    Button btn_sleep  = { KZ_W - 48, 4,  20, 16, KZ_BTN_SLEEP };
    int    press_fx   = 0;   /* frames of button-press highlight remaining */

    /* Load the cat's stats, or start her fresh and content if there's no save. */
    Stats stats;
    if (!stats_load(&stats, KZ_SAVE_PATH)) {
        stats = stats_new();
    }

    bool   running = true;
    Uint64 frame   = 0;
    Uint64 next    = SDL_GetTicksNS();

    while (running) {
        /* ---- input ---- */
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
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
                    stats_feed(&stats);
                else if (e.key.key == SDLK_G)
                    stats_groom(&stats);
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN: {
                /* Window pixels -> 240x160 logical space, then hit-test.
                 * This point is a "tap" — identical whether it came from a
                 * mouse click (now) or a finger (on iOS later). */
                float lx, ly;
                SDL_RenderCoordinatesFromWindow(renderer, e.button.x,
                                                e.button.y, &lx, &ly);

                /* 1) travel button: toggle cottage <-> meadow */
                if (ui_button_hit(&btn_travel, lx, ly)) {
                    location = (location == LOC_COTTAGE) ? LOC_MEADOW
                                                         : LOC_COTTAGE;
                    btn_travel.kind = (location == LOC_COTTAGE) ? KZ_BTN_OUT
                                                                : KZ_BTN_HOME;
                    press_fx = 8;
                }
                /* 2) sleep button (cottage only): fresh morning + save */
                else if (location == LOC_COTTAGE
                         && ui_button_hit(&btn_sleep, lx, ly)) {
                    meadow.time = KZ_DAWN;          /* wake to a fresh morning */
                    stats.energy = KZ_STAT_MAX;     /* a good rest             */
                    stats_save(&stats, KZ_SAVE_PATH);
                    press_fx = 8;
                }
                /* 3) tapping the bed also sleeps */
                else if (location == LOC_COTTAGE && cottage_bed_hit(lx, ly)) {
                    meadow.time = KZ_DAWN;
                    stats.energy = KZ_STAT_MAX;
                    stats_save(&stats, KZ_SAVE_PATH);
                    press_fx = 8;
                }
                /* 4) otherwise, a tap on the cat pets her */
                else if (cat_hit(&cat, lx, ly)) {
                    cat_pet(&cat);
                    stats_pet(&stats);   /* petting deepens the bond */
                }
                break;
            }
            default: break;
            }
        }

        /* ---- update ---- */
        if (location == LOC_MEADOW) meadow_update(&meadow);
        cat_update(&cat);
        if (press_fx > 0) press_fx--;

        /* ---- draw (back to front) ---- */
        SDL_SetRenderDrawColor(renderer, KZ_CLOUD.r, KZ_CLOUD.g, KZ_CLOUD.b, 255);
        SDL_RenderClear(renderer);

        bool is_night = (meadow.time == KZ_NIGHT);
        if (location == LOC_COTTAGE) {
            cottage_draw(renderer, frame, is_night);
            cat_draw(renderer, &cat, frame);
        } else {
            meadow_draw(renderer, &meadow, frame);
            cat_draw(renderer, &cat, frame);
            meadow_draw_wash(renderer, &meadow);   /* mood overlay, on top */
        }

        /* ---- UI (both locations) ---- */
        ui_draw_panel(renderer, &stats, 4, 4);            /* stats, top-left */
        ui_button_draw(renderer, &btn_travel, press_fx > 0);
        if (location == LOC_COTTAGE)
            ui_button_draw(renderer, &btn_sleep, press_fx > 0);
        ui_draw_hint(renderer);

        SDL_RenderPresent(renderer);

        /* ---- pace to 60fps without busy-spinning ---- */
        frame++;
        next += FRAME_NS;
        Uint64 now = SDL_GetTicksNS();
        if (next > now) SDL_DelayNS(next - now);
        else            next = now;   /* fell behind; don't spiral */
    }

    /* Save the cat's stats so she remembers you next time. */
    stats_save(&stats, KZ_SAVE_PATH);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}