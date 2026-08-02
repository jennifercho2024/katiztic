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
#include "behavior.h"
#include "mood.h"
#include "friends.h"
#include "encounter.h"
#include "decor.h"
#include "ui.h"
#include "cottage.h"
#include "camera.h"
#include "cafe.h"
#include "icon.h"
#include "music.h"

/* Where the player currently is. Sleeping happens in the cottage; the meadow
 * is the outdoors. Moving between them is a tap on an on-screen button. */
typedef enum { LOC_COTTAGE, LOC_MEADOW, LOC_CAFE } Location;

/* Window opens at 4x the logical canvas: 960x640. */
#define KZ_SCALE  4
#define WIN_W     (KZ_W * KZ_SCALE)
#define WIN_H     (KZ_H * KZ_SCALE)

/* Where the cat's stats are saved between sessions. */
#define KZ_SAVE_PATH "katiztic.sav"
#define KZ_FRIENDS_PATH "katiztic-friends.sav"
#define KZ_DECOR_PATH   "katiztic-decor.sav"

/* Fixed timestep so the mood animations run the same on any machine. */
#define KZ_FPS       60
#define FRAME_NS     (SDL_NS_PER_SECOND / KZ_FPS)

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
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

    /* Give the window (and dock/taskbar) the Katiztic cat face. */
    SDL_Surface *icon = icon_create();
    if (icon) {
        SDL_SetWindowIcon(window, icon);
        SDL_DestroySurface(icon);   /* SDL copied it; we can free ours */
    }

    /* Start the cozy music. If audio can't open, the game just runs silent. */
    music_init();

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
    bool   travel_open = false;   /* is the place-picker menu showing? */

    /* The cottage is bigger than the screen so you can pan around it. The
     * camera is the view offset into that larger room. */
    #define COTTAGE_ROOM_W 360.0f
    #define COTTAGE_ROOM_H 240.0f
    Camera cam = camera_make(COTTAGE_ROOM_W, COTTAGE_ROOM_H);
    bool cam_dragging = false;    /* panning the room right now? */
    float cam_last_x = 0, cam_last_y = 0;

    /* Releasing a cat needs a confirm tap so it can't happen by accident:
     * first tap arms it (shows "sure?"), second tap within the window does it. */
    int release_confirm = 0;      /* frames left in the confirm window (0 = off) */
    Button btn_sleep  = { KZ_W - 48, 4,  20, 16, KZ_BTN_SLEEP };
    int    press_fx   = 0;   /* frames of button-press highlight remaining */

    /* Load the family, or start with two starter cats if there's no save. */
    Roster roster;
    if (!roster_load(&roster, KZ_SAVE_PATH, CAT_X, CAT_Y)) {
        roster = roster_new(CAT_X, CAT_Y);
    }
    /* Spread the cats to distinct spots so they don't begin stacked; from
     * there their own wandering takes over. */
    for (int i = 0; i < roster.count; i++) {
        float hx, hy;
        roster_home_spot(i, &hx, &hy);
        roster.cats[i].anim.cx = hx;
        roster.cats[i].anim.cy = hy;
    }

    /* Friends met on walks — a separate collection and save file. */
    Friends friends;
    if (!friends_load(&friends, KZ_FRIENDS_PATH)) {
        friends = friends_new();
    }

    /* Cottage décor — collectibles you unlock and arrange. */
    Decor decor;
    if (!decor_load(&decor, KZ_DECOR_PATH)) {
        decor = decor_new();
    }
    /* Items keep exactly the positions you placed them at — no auto-settling. */
    bool decor_open = false;      /* is the décor tray showing?         */
    int  drag_item = -1;          /* décor item being dragged, or -1    */

    /* The wild cat currently visiting the meadow (if any), the banner line
     * that describes the moment, and whether the friends-list overlay is open. */
    Encounter enc = encounter_none();
    char banner_line[48];
    banner_line[0] = '\0';
    int  banner_timer = 0;      /* frames the banner stays up (0 = hidden) */
    bool friends_open = false;

    /* Track each cat's level so we can celebrate when one goes up. */
    Uint16 prev_level[KZ_MAX_CATS];
    for (int i = 0; i < roster.count; i++)
        prev_level[i] = roster.cats[i].stats.level;

    /* Rename mode: when active, keystrokes edit `edit_buf` instead of doing
     * their usual jobs. Tapping the name on the stat card starts it; Enter
     * confirms, Escape cancels. Physical keyboard for now — forward-compatible
     * with the iOS on-screen keyboard when that port comes. */
    bool editing = false;
    char edit_buf[KZ_NAME_LEN];
    edit_buf[0] = '\0';
    int  edit_len = 0;

    /* Sleep transition: a brief fade out and back in, so sleeping always feels
     * like time passed even when the day was already fresh. Counts down from
     * SLEEP_FADE_FRAMES; the midpoint is when the day actually resets. */
    #define SLEEP_FADE_FRAMES 60
    int  sleep_fade = 0;          /* >0 while the fade animation plays   */
    bool sleep_applied = false;   /* did we apply the reset at midpoint? */

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
                /* In the cottage, the room is panned by the camera, so in-room
                 * hit-tests (cats, décor, bed) use room coordinates. UI stays
                 * in screen coordinates (lx,ly). */
                float rx = lx, ry = ly;
                if (location == LOC_COTTAGE)
                    camera_to_room(&cam, lx, ly, &rx, &ry);

                /* If the friends list is open, any tap just closes it. */
                if (friends_open) { friends_open = false; break; }

                /* If the place-picker menu is open, it gets first claim on
                 * taps (it overlaps other buttons on the right edge). */
                if (travel_open) {
                    int pick = ui_place_menu_hit(lx, ly);
                    travel_open = false;
                    if (pick >= 0) {
                        Location newloc = (pick == 0) ? LOC_COTTAGE
                                        : (pick == 1) ? LOC_MEADOW : LOC_CAFE;
                        if (newloc != location) {
                            location = newloc;
                            btn_travel.kind = (location == LOC_COTTAGE)
                                              ? KZ_BTN_OUT : KZ_BTN_HOME;
                            /* Going out (meadow or café) lifts everyone's
                             * happiness — a nice change of scene. */
                            if (newloc != LOC_COTTAGE) {
                                for (int i = 0; i < roster.count; i++)
                                    stats_outing(&roster.cats[i].stats);
                            }
                            if (location == LOC_MEADOW) {
                                enc = encounter_begin(&friends);
                                if (enc.present) {
                                    Friend *known = friends_find(&friends, enc.name);
                                    if (known && known->befriended)
                                        SDL_snprintf(banner_line, sizeof banner_line,
                                                     "%s comes to say hello!", enc.name);
                                    else if (known)
                                        SDL_snprintf(banner_line, sizeof banner_line,
                                                     "%s is here again.", enc.name);
                                    else
                                        SDL_strlcpy(banner_line,
                                                    "A shy cat watches you...",
                                                    sizeof banner_line);
                                    banner_timer = 240;
                                }
                            } else {
                                enc = encounter_none();
                            }
                        }
                    }
                    break;
                }

                /* 0) friends button: open the friends list */
                if (ui_friends_button_hit(lx, ly)) {
                    friends_open = true;
                    press_fx = 8;
                    break;
                }

                /* 0b) décor button (cottage only): toggle the décor tray */
                if (location == LOC_COTTAGE && ui_decor_button_hit(lx, ly)) {
                    decor_open = !decor_open;
                    press_fx = 8;
                    break;
                }

                /* 0c) while the décor tray is open, a tap on a tray slot grabs
                 * a fresh copy of that item to drag out into the room. */
                if (location == LOC_COTTAGE && decor_open) {
                    int tray = ui_decor_tray_hit(&decor, lx, ly);
                    if (tray >= 0) {
                        drag_item = tray;
                        break;
                    }
                }

                /* 0c2) The stat card is screen-fixed UI, so its buttons must be
                 * checked in SCREEN coords BEFORE any room-space hit-tests —
                 * otherwise a décor item sitting under the card (in room space)
                 * would swallow the tap. */
                /* release ("×") button: two-tap confirm */
                if (ui_release_hit(4, 4, lx, ly)) {
                    if (release_confirm > 0) {
                        int idx = roster.active;
                        if (roster_release(&roster, idx)) {
                            roster_save(&roster, KZ_SAVE_PATH);
                            SDL_strlcpy(banner_line, "Released to the world.",
                                        sizeof banner_line);
                            banner_timer = 200;
                        }
                        release_confirm = 0;
                    } else {
                        release_confirm = 90;   /* arm: ~1.5s to confirm */
                    }
                    press_fx = 8;
                    break;
                }
                /* name row: tap to rename */
                if (ui_name_hit(4, 4, lx, ly)) {
                    editing = true;
                    SDL_strlcpy(edit_buf, roster_active(&roster)->name,
                                KZ_NAME_LEN);
                    edit_len = (int)SDL_strlen(edit_buf);
                    SDL_StartTextInput(window);
                    break;
                }

                /* 0d) grabbing an item already placed in the room works anytime
                 * you're home — tray open or not — so you can rearrange your
                 * cottage freely. Checked before the pet-the-cat handler so a
                 * décor item on top of a cat grabs rather than pets. */
                if (location == LOC_COTTAGE) {
                    int placed = decor_hit(&decor, rx, ry);
                    if (placed >= 0) {
                        drag_item = placed;
                        break;
                    }
                }

                /* 2) roster strip: select a cat, or adopt a new one. */
                int slot = ui_roster_hit(&roster, lx, ly);
                if (slot == -2) {
                    /* adopt: cycle through types by how many you have */
                    CatType t = (CatType)(roster.count % KZ_TYPE_COUNT);
                    if (roster_adopt(&roster, t, CAT_X, CAT_Y)) {
                        /* new cat starts at level 1; seed its tracker */
                        int ni = roster.count - 1;
                        prev_level[ni] = roster.cats[ni].stats.level;
                        if (roster.cats[ni].shiny) {
                            SDL_snprintf(banner_line, sizeof banner_line,
                                         "A shiny cat! So rare!");
                            banner_timer = 300;
                        }
                    }
                    press_fx = 8;
                    break;
                } else if (slot >= 0) {
                    roster_select(&roster, slot);
                    press_fx = 8;
                    break;
                }

                /* 3) travel button: open the place-picker menu */
                if (ui_button_hit(&btn_travel, lx, ly)) {
                    travel_open = true;
                    press_fx = 8;
                    break;
                }
                /* 3b) offer a treat to the visiting cat (meadow only) */
                else if (location == LOC_MEADOW && enc.present
                         && ui_treat_button_hit(lx, ly)) {
                    friends_meet(&friends, enc.name, enc.type);
                    bool now_friend = friends_offer_treat(&friends, enc.name);
                    friends_save(&friends, KZ_FRIENDS_PATH);
                    if (now_friend)
                        SDL_snprintf(banner_line, sizeof banner_line,
                                     "%s is your friend now!", enc.name);
                    else
                        SDL_snprintf(banner_line, sizeof banner_line,
                                     "%s nibbles the treat.", enc.name);
                    banner_timer = 240;
                    press_fx = 8;
                }
                /* 4) sleep button (cottage only): begin the sleep fade */
                else if (location == LOC_COTTAGE
                         && ui_button_hit(&btn_sleep, lx, ly)) {
                    if (sleep_fade == 0) {
                        sleep_fade = SLEEP_FADE_FRAMES;
                        sleep_applied = false;
                    }
                    press_fx = 8;
                }
                /* 5) tapping the bed also sleeps */
                else if (location == LOC_COTTAGE && cottage_bed_hit(rx, ry)) {
                    if (sleep_fade == 0) {
                        sleep_fade = SLEEP_FADE_FRAMES;
                        sleep_applied = false;
                    }
                    press_fx = 8;
                }
                /* 6) tap a cat. In the cottage the whole family is present, so
                 * a tap can land on any of them: that cat becomes active and
                 * gets petted. In the meadow, only the active cat is there. */
                else {
                    bool hit_one = false;
                    if (location == LOC_COTTAGE) {
                        for (int i = 0; i < roster.count; i++) {
                            if (cat_hit(&roster.cats[i].anim, rx, ry)) {
                                roster_select(&roster, i);
                                cat_pet(&roster.cats[i].anim);
                                stats_pet(&roster.cats[i].stats);
                                hit_one = true;
                                break;
                            }
                        }
                        /* 7) empty space in the cottage -> start panning the room */
                        if (!hit_one) {
                            cam_dragging = true;
                            cam_last_x = lx;
                            cam_last_y = ly;
                        }
                    } else {
                        OwnedCat *a = roster_active(&roster);
                        if (cat_hit(&a->anim, lx, ly)) {
                            cat_pet(&a->anim);
                            stats_pet(&a->stats);   /* petting deepens bond */
                        }
                    }
                }
                break;
            }

            case SDL_EVENT_MOUSE_MOTION: {
                float lx, ly;
                SDL_RenderCoordinatesFromWindow(renderer, e.motion.x,
                                                e.motion.y, &lx, &ly);
                /* Panning the room: move the camera opposite the drag. */
                if (cam_dragging) {
                    camera_pan(&cam, cam_last_x - lx, cam_last_y - ly);
                    cam_last_x = lx;
                    cam_last_y = ly;
                }
                /* While dragging a décor item, follow the pointer (in room
                 * coordinates, since the cottage is panned). */
                else if (drag_item >= 0) {
                    float rx = lx, ry = ly;
                    if (location == LOC_COTTAGE)
                        camera_to_room(&cam, lx, ly, &rx, &ry);
                    decor.items[drag_item].x = rx - 8;
                    decor.items[drag_item].y = ry - 8;
                    decor.items[drag_item].placed = true;
                }
                break;
            }

            case SDL_EVENT_MOUSE_BUTTON_UP: {
                if (cam_dragging) { cam_dragging = false; break; }
                if (drag_item >= 0) {
                    float lx, ly;
                    SDL_RenderCoordinatesFromWindow(renderer, e.button.x,
                                                    e.button.y, &lx, &ly);
                    float rx = lx, ry = ly;
                    if (location == LOC_COTTAGE)
                        camera_to_room(&cam, lx, ly, &rx, &ry);
                    /* Dropping onto the (screen-fixed) tray puts it away. */
                    if (decor_open && ly >= ui_decor_tray_top()) {
                        decor.items[drag_item].placed = false;
                    } else {
                        decor.items[drag_item].placed = true;
                        decor.items[drag_item].x = rx - 8;
                        decor.items[drag_item].y = ry - 8;
                        /* items stay exactly where you place them */
                    }
                    decor_save(&decor, KZ_DECOR_PATH);
                    drag_item = -1;
                }
                break;
            }
            default: break;
            }
        }

        /* ---- update ---- */
        if (location == LOC_MEADOW) meadow_update(&meadow);
        OwnedCat *active = roster_active(&roster);
        /* Animate every cat (they all breathe and blink); the active one also
         * carries any petting glow. */
        for (int i = 0; i < roster.count; i++) {
            cat_update(&roster.cats[i].anim);
            mood_update(&roster.cats[i].anim, &roster.cats[i].stats);
        }
        /* At home and at the café, the whole family roams and socializes.
         * Décor (yarn/milk to react to) exists only in the cottage. */
        if (location == LOC_COTTAGE)
            behavior_update(&roster, &decor, frame);
        else if (location == LOC_CAFE)
            behavior_update(&roster, NULL, frame);
        if (press_fx > 0) press_fx--;
        if (release_confirm > 0) release_confirm--;

        /* Time of day follows the real clock: the world lightens and darkens
         * with the actual time where you are. */
        {
            SDL_Time now;
            SDL_DateTime dt;
            if (SDL_GetCurrentTime(&now) && SDL_TimeToDateTime(now, &dt, true)) {
                meadow.time = time_from_hour(dt.hour);
            }
        }

        /* Music follows the place you're in. */
        MusicTheme mt = (location == LOC_COTTAGE) ? MUSIC_COTTAGE
                      : (location == LOC_MEADOW)  ? MUSIC_MEADOW
                      : MUSIC_CAFE;
        music_set_theme(mt);
        if (location == LOC_MEADOW) encounter_update(&enc, &friends);
        if (banner_timer > 0) banner_timer--;

        /* Celebrate any cat who just leveled up (from care or socializing). */
        for (int i = 0; i < roster.count; i++) {
            Uint16 lv = roster.cats[i].stats.level;
            if (lv > prev_level[i]) {
                SDL_snprintf(banner_line, sizeof banner_line,
                             "%s reached level %u!",
                             roster.cats[i].name, (unsigned)lv);
                banner_timer = 240;
            }
            prev_level[i] = lv;
        }

        /* Décor unlocks: check current progress. If something new unlocks,
         * announce it in the banner. */
        {
            int max_bond = 0, total_levels = 0;
            for (int i = 0; i < roster.count; i++) {
                if (roster.cats[i].stats.bond > max_bond)
                    max_bond = roster.cats[i].stats.bond;
                total_levels += roster.cats[i].stats.level;
            }
            int fcount = friends_befriended_count(&friends);
            int newly = decor_check_unlocks(&decor, max_bond, fcount,
                                            roster.count, total_levels);
            if (newly > 0) {
                SDL_strlcpy(banner_line, "You unlocked new decor!",
                            sizeof banner_line);
                banner_timer = 240;
                decor_save(&decor, KZ_DECOR_PATH);
            }
        }

        /* Sleep fade: at the darkest midpoint, the whole family wakes rested.
         * Refreshing every cat (not just the active one) gives sleeping a
         * visible, felt consequence — several bars jump up at once. */
        if (sleep_fade > 0) {
            sleep_fade--;
            if (!sleep_applied && sleep_fade <= SLEEP_FADE_FRAMES / 2) {
                /* time of day now follows the real clock, so sleeping simply
                 * rests the family rather than skipping to morning */
                for (int i = 0; i < roster.count; i++) {
                    roster.cats[i].stats.energy = KZ_STAT_MAX;   /* fully rested */
                    stats_wake(&roster.cats[i].stats);           /* a mood lift  */
                }
                roster_save(&roster, KZ_SAVE_PATH);
                sleep_applied = true;
            }
        }

        /* ---- draw (back to front) ---- */
        render_clear_offset();   /* start each frame screen-fixed */
        SDL_SetRenderDrawColor(renderer, KZ_CLOUD.r, KZ_CLOUD.g, KZ_CLOUD.b, 255);
        SDL_RenderClear(renderer);

        CatColors col = active->shiny ? cat_shiny_colors()
                                      : cattype_colors(active->type);
        bool is_night = (meadow.time == KZ_NIGHT || meadow.time == KZ_DUSK);
        if (location == LOC_COTTAGE || location == LOC_CAFE) {
            /* Indoor places where the family roams and socializes. */
            if (location == LOC_COTTAGE) {
                /* Pan the whole cottage by the camera offset. */
                render_set_offset(cam.x, cam.y);
                cottage_draw(renderer, frame, is_night,
                             COTTAGE_ROOM_W, COTTAGE_ROOM_H);
                decor_draw(renderer, &decor, frame);   /* décor only at home */
            } else {
                cafe_draw(renderer, frame);
            }
            /* Draw the roaming cats sorted by y so nearer (lower) cats overlap
             * farther ones. */
            int order[KZ_MAX_CATS];
            for (int i = 0; i < roster.count; i++) order[i] = i;
            for (int a = 1; a < roster.count; a++) {
                int key = order[a];
                float ky = roster.cats[key].anim.cy;
                int b = a - 1;
                while (b >= 0 && roster.cats[order[b]].anim.cy > ky) {
                    order[b + 1] = order[b];
                    b--;
                }
                order[b + 1] = key;
            }
            for (int k = 0; k < roster.count; k++) {
                int i = order[k];
                CatColors cc = roster.cats[i].shiny
                             ? cat_shiny_colors()
                             : cattype_colors(roster.cats[i].type);
                cat_draw(renderer, &roster.cats[i].anim, cc, frame);
                if (roster.cats[i].shiny)
                    cat_draw_sparkles(renderer, &roster.cats[i].anim, frame);
            }
            /* mood bubbles float above everyone */
            for (int i = 0; i < roster.count; i++)
                mood_draw(renderer, &roster.cats[i].anim, frame);
            render_clear_offset();   /* UI and everything else is screen-fixed */
        } else {
            /* Outdoors is a one-cat outing: just the active cat, at the meadow
             * spot. We save and restore her real position so her roaming spot
             * back home isn't clobbered by the walk. A visiting wild cat (if
             * any) sits off to the side. */
            float save_x = active->anim.cx, save_y = active->anim.cy;
            Activity save_act = active->anim.act;
            active->anim.cx = CAT_X;
            active->anim.cy = CAT_Y;
            active->anim.act = ACT_SIT;      /* she sits calmly on the walk */
            meadow_draw(renderer, &meadow, frame);
            encounter_draw(renderer, &enc, frame);   /* the visitor, if present */
            cat_draw(renderer, &active->anim, col, frame);
            if (active->shiny)
                cat_draw_sparkles(renderer, &active->anim, frame);
            mood_draw(renderer, &active->anim, frame);
            meadow_draw_wash(renderer, &meadow);   /* mood overlay, on top */
            active->anim.cx = save_x;
            active->anim.cy = save_y;
            active->anim.act = save_act;
        }

        /* ---- UI (both locations) ---- */
        ui_draw_panel(renderer, active, 4, 4, editing, edit_buf, frame);
        ui_draw_release_button(renderer, 4, 4, release_confirm > 0);
        ui_button_draw(renderer, &btn_travel, press_fx > 0);
        if (location == LOC_COTTAGE)
            ui_button_draw(renderer, &btn_sleep, press_fx > 0);
        ui_friends_button_draw(renderer, press_fx > 0);   /* friends list */
        if (location == LOC_COTTAGE)
            ui_decor_button_draw(renderer, press_fx > 0);  /* décor tray   */
        ui_roster_draw(renderer, &roster);                /* the family strip */

        /* Travel place-picker menu, when open. */
        if (travel_open) {
            ui_place_menu(renderer,
                          location == LOC_COTTAGE ? 0
                        : location == LOC_MEADOW  ? 1 : 2);
        }

        /* Décor tray, when open (cottage only). */
        if (location == LOC_COTTAGE && decor_open) {
            ui_decor_tray(renderer, &decor, frame);
        }

        /* Encounter UI: the treat button and the dialogue banner, when a wild
         * cat is visiting on a walk. */
        if (location == LOC_MEADOW && enc.present) {
            ui_treat_button_draw(renderer, press_fx > 0);
        }
        if (banner_timer > 0) {
            ui_banner(renderer, banner_line);
        }

        /* Sleep fade overlay: a soft lavender-dark veil that peaks at the
         * midpoint and eases back out — the unmistakable "you slept" signal. */
        if (sleep_fade > 0) {
            int half = SLEEP_FADE_FRAMES / 2;
            int dist = sleep_fade > half ? (SLEEP_FADE_FRAMES - sleep_fade)
                                         : sleep_fade;
            float t = 1.0f - (float)dist / (float)half;   /* 0..1, peak at mid */
            Uint8 a = (Uint8)(t * 230.0f);
            px_rect_a(renderer, 0, 0, KZ_W, KZ_H, rgb(0x3B, 0x30, 0x50), a);
        }

        /* Friends list overlay sits above everything when open. */
        if (friends_open) {
            ui_friends_list(renderer, &friends);
        }

        SDL_RenderPresent(renderer);

        /* ---- pace to 60fps without busy-spinning ---- */
        frame++;
        next += FRAME_NS;
        Uint64 now = SDL_GetTicksNS();
        if (next > now) SDL_DelayNS(next - now);
        else            next = now;   /* fell behind; don't spiral */
    }

    /* Save the whole family and your friends so they remember you next time. */
    roster_save(&roster, KZ_SAVE_PATH);
    friends_save(&friends, KZ_FRIENDS_PATH);
    decor_save(&decor, KZ_DECOR_PATH);

    music_shutdown();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}