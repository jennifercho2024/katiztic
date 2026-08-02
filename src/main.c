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
#include "forest.h"
#include "street.h"
#include "story.h"
#include "quests.h"
#include <math.h>
#include "icon.h"
#include "music.h"

/* Where the player currently is. Sleeping happens in the cottage; the meadow
 * is the outdoors. Moving between them is a tap on an on-screen button. */
typedef enum { LOC_COTTAGE, LOC_MEADOW, LOC_CAFE, LOC_FOREST, LOC_STREET } Location;

/* Which story zone a location belongs to, or -1 if it isn't one. New zones
 * arrive faded (option A): only these get the warmth treatment. */
static int story_zone_for(Location l) {
    if (l == LOC_FOREST) return (int)STORY_ZONE_FOREST;
    if (l == LOC_STREET) return (int)STORY_ZONE_STREET;
    return -1;
}
static const char *STORY_ZONE_NAMES[STORY_ZONE_COUNT] = { "forest", "street" };

#define KZ_QUESTS_PATH  "katiztic-quests.sav"

/* A quest just completed: every cat earns the reward XP, and a banner
 * celebrates. Kept here so every hook site stays one line. */
static void quest_fanfare(Roster *ro, Quests *qs, QuestId id,
                          char *banner, size_t blen, int *banner_timer) {
    const QuestInfo *qi = quest_info(id);
    for (int i = 0; i < ro->count; i++)
        stats_gain_xp(&ro->cats[i].stats, (int)qi->reward_xp);
    SDL_snprintf(banner, blen, "Quest done! +%u xp for all",
                 (unsigned)qi->reward_xp);
    *banner_timer = 320;
    quests_save(qs, KZ_QUESTS_PATH);   /* a finished quest is never lost */
}

/* Window opens at 4x the logical canvas: 960x640. */
#define KZ_SCALE  4
#define WIN_W     (KZ_W * KZ_SCALE)
#define WIN_H     (KZ_H * KZ_SCALE)

/* Where the cat's stats are saved between sessions. */
#define KZ_SAVE_PATH "katiztic.sav"
#define KZ_FRIENDS_PATH "katiztic-friends.sav"
#define KZ_DECOR_PATH   "katiztic-decor.sav"
#define KZ_STORY_PATH   "katiztic-story.sav"

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

    /* Releasing a cat asks a proper "Are you sure?" with Yes/No buttons, so
     * it can never happen by accident. */
    bool release_open = false;
    bool quests_open = false;    /* is the quest log showing? */
    int  quests_scroll = 0;      /* first visible quest row       */
    float quests_drag_y = 0;     /* last pointer y while swiping   */
    bool quests_dragging = false;
    bool quests_scrolled = false;/* did this gesture actually scroll? */
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

    /* The story of the world's warmth — which places have their color back. */
    Story story;
    if (!story_load(&story, KZ_STORY_PATH)) {
        story = story_new();
    }

    /* The quest log — gentle goals, with XP for the whole family on finish. */
    Quests quests;
    if (!quests_load(&quests, KZ_QUESTS_PATH)) {
        quests = quests_new();
    }
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
                else if (e.key.key == SDLK_F) {
                    stats_feed(&roster_active(&roster)->stats);
                    if (quests_bump(&quests, QUEST_FEED))
                        quest_fanfare(&roster, &quests, QUEST_FEED, banner_line,
                                      sizeof banner_line, &banner_timer);
                }
                else if (e.key.key == SDLK_G) {
                    stats_groom(&roster_active(&roster)->stats);
                    if (quests_bump(&quests, QUEST_GROOM))
                        quest_fanfare(&roster, &quests, QUEST_GROOM, banner_line,
                                      sizeof banner_line, &banner_timer);
                }
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
                if (quests_open) {
                    /* start a potential swipe; button-up decides tap vs scroll */
                    quests_dragging = true;
                    quests_drag_y = ly;
                    quests_scrolled = false;
                    break;
                }

                /* If the release dialog is open, it owns every tap: Yes
                 * releases the active cat; No (or anywhere else) cancels. */
                if (release_open) {
                    int ans = ui_confirm_release_hit(lx, ly);
                    release_open = false;
                    if (ans == 1) {
                        if (roster_release(&roster, roster.active)) {
                            roster_save(&roster, KZ_SAVE_PATH);
                            SDL_strlcpy(banner_line, "Released to the world.",
                                        sizeof banner_line);
                            banner_timer = 200;
                        }
                    }
                    press_fx = 8;
                    break;
                }

                /* If the place-picker menu is open, it gets first claim on
                 * taps (it overlaps other buttons on the right edge). */
                if (travel_open) {
                    int pick = ui_place_menu_hit(lx, ly);
                    travel_open = false;
                    if (pick >= 0) {
                        Location newloc = (pick == 0) ? LOC_COTTAGE
                                        : (pick == 1) ? LOC_MEADOW
                                        : (pick == 2) ? LOC_CAFE
                                        : (pick == 3) ? LOC_FOREST
                                        : LOC_STREET;
                        if (newloc != location) {
                            /* leaving a story zone? keep its warmth safe */
                            if (story_zone_for(location) >= 0)
                                story_save(&story, KZ_STORY_PATH);
                            location = newloc;
                            btn_travel.kind = (location == LOC_COTTAGE)
                                              ? KZ_BTN_OUT : KZ_BTN_HOME;
                            /* Going out (anywhere but home) lifts everyone's
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
                            /* First time stepping into a faded zone: a
                             * story moment — its color is gone. */
                            {
                                int sz = story_zone_for(location);
                                if (sz >= 0 && !story.seen_intro[sz]) {
                                    story.seen_intro[sz] = true;
                                    SDL_snprintf(banner_line, sizeof banner_line,
                                                 "The %s's color has faded...",
                                                 STORY_ZONE_NAMES[sz]);
                                    banner_timer = 320;
                                    story_save(&story, KZ_STORY_PATH);
                                }
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

                /* 0b) quests button: open the quest log */
                if (ui_quests_button_hit(lx, ly)) {
                    quests_open = true;
                    quests_scroll = 0;
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
                /* release ("×") button: opens the Are-you-sure dialog */
                if (ui_release_hit(4, 4, lx, ly)) {
                    release_open = true;
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
                    /* a random type, so each adoption is a fresh surprise */
                    CatType t = (CatType)SDL_rand(KZ_TYPE_COUNT);
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
                                if (quests_bump(&quests, QUEST_PET))
                                    quest_fanfare(&roster, &quests, QUEST_PET,
                                                  banner_line,
                                                  sizeof banner_line,
                                                  &banner_timer);
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
                        /* Outdoors she's DRAWN at the outing spot (her stored
                         * position stays her home roaming spot), so hit-test
                         * where she actually appears on screen. */
                        OwnedCat *a = roster_active(&roster);
                        float hx = CAT_X, hy = CAT_Y;
                        if (location == LOC_STREET) {
                            hx = CAT_X + sinf((float)frame * 0.008f) * 26.0f;
                            hy = 96.0f;
                        }
                        float sx0 = a->anim.cx, sy0 = a->anim.cy;
                        a->anim.cx = hx; a->anim.cy = hy;
                        bool hit_out = cat_hit(&a->anim, lx, ly);
                        a->anim.cx = sx0; a->anim.cy = sy0;
                        if (hit_out) {
                            cat_pet(&a->anim);
                            stats_pet(&a->stats);   /* petting deepens bond */
                            if (quests_bump(&quests, QUEST_PET))
                                quest_fanfare(&roster, &quests, QUEST_PET, banner_line,
                                              sizeof banner_line,
                                              &banner_timer);
                            /* in a faded zone, petting makes her magic flare:
                             * a visible burst of color returns */
                            int sz = story_zone_for(location);
                            if (sz >= 0)
                                story_pet_boost(&story, (StoryZone)sz);
                        }
                    }
                }
                break;
            }

            case SDL_EVENT_MOUSE_MOTION: {
                float lx, ly;
                SDL_RenderCoordinatesFromWindow(renderer, e.motion.x,
                                                e.motion.y, &lx, &ly);
                /* Swiping the open quest log scrolls it. */
                if (quests_dragging) {
                    float dy = ly - quests_drag_y;
                    if (dy >  11) { quests_scroll--; quests_drag_y = ly; quests_scrolled = true; }
                    if (dy < -11) { quests_scroll++; quests_drag_y = ly; quests_scrolled = true; }
                    if (quests_scroll < 0) quests_scroll = 0;
                    if (quests_scroll > QUEST_COUNT - 5)
                        quests_scroll = QUEST_COUNT - 5;
                    if (quests_scroll < 0) quests_scroll = 0;
                    break;
                }
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
                if (quests_dragging) {
                    /* a gesture that never scrolled is a tap -> close */
                    if (!quests_scrolled) quests_open = false;
                    quests_dragging = false;
                    break;
                }
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
                      : (location == LOC_CAFE)    ? MUSIC_CAFE
                      : (location == LOC_FOREST)  ? MUSIC_FOREST
                      : MUSIC_STREET;
        music_set_theme(mt);
        if (location == LOC_MEADOW) encounter_update(&enc, &friends);

        /* In a faded zone, your cat's quiet company brings the color back —
         * faster the deeper her bond and the more friends you've made. */
        {
            int sz = story_zone_for(location);
            if (sz >= 0) {
                story_visit_tick(&story, (StoryZone)sz,
                                 (int)active->stats.bond,
                                 friends_befriended_count(&friends));
                if (story_warmth(&story, (StoryZone)sz) >= 1.0f
                    && !story.celebrated[sz]) {
                    story.celebrated[sz] = true;
                    SDL_snprintf(banner_line, sizeof banner_line,
                                 "Color returns to the %s!",
                                 STORY_ZONE_NAMES[sz]);
                    banner_timer = 360;
                    story_save(&story, KZ_STORY_PATH);
                }
            }
        }

        /* Value-tracked quests: raise each to its current truth. Any that
         * just completed gets the fanfare. Cheap enough to run every frame. */
        {
            int maxlvl = 0;
            bool playing = false;
            for (int i = 0; i < roster.count; i++) {
                if ((int)roster.cats[i].stats.level > maxlvl)
                    maxlvl = (int)roster.cats[i].stats.level;
                if (roster.cats[i].anim.act == ACT_PLAY) playing = true;
            }
            const QuestId QIDS[6] = {
                QUEST_FRIENDS, QUEST_FAMILY, QUEST_LEVEL5,
                QUEST_CAFE, QUEST_PLAY, QUEST_FOREST
            };
            const int QVALS[6] = {
                friends_befriended_count(&friends),
                roster.count,
                maxlvl,
                (location == LOC_CAFE) ? 1 : 0,
                playing ? 1 : 0,
                (story_warmth(&story, STORY_ZONE_FOREST) >= 1.0f) ? 1 : 0,
            };
            for (int i = 0; i < 6; i++)
                if (quests_set(&quests, QIDS[i], QVALS[i]))
                    quest_fanfare(&roster, &quests, QIDS[i], banner_line,
                                  sizeof banner_line, &banner_timer);
            if (quests_set(&quests, QUEST_STREET,
                           (story_warmth(&story, STORY_ZONE_STREET) >= 1.0f)
                               ? 1 : 0))
                quest_fanfare(&roster, &quests, QUEST_STREET, banner_line,
                              sizeof banner_line, &banner_timer);
        }
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

        /* ---- draw (back to front) ---- */
        render_clear_offset();       /* start each frame screen-fixed... */
        render_set_warmth(1.0f);     /* ...and at full color */
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
        } else if (location == LOC_FOREST || location == LOC_STREET) {
            /* A faded story zone. It's drawn through the warmth filter — grey
             * at first, its pastels returning as warmth rises. Your cat is
             * drawn at FULL color on purpose: she carries the warmth, a vivid
             * little companion in a grey world catching up to her. */
            float save_x = active->anim.cx, save_y = active->anim.cy;
            Activity save_act = active->anim.act;
            int sz = story_zone_for(location);
            if (location == LOC_STREET) {
                /* on the street she strolls the pavement with you, gently
                 * pacing back and forth */
                float pace = sinf((float)frame * 0.008f);
                active->anim.cx = CAT_X + pace * 26.0f;
                active->anim.cy = 96.0f;   /* on the sidewalk */
                active->anim.act = ACT_WALK;
                active->anim.facing = (cosf((float)frame * 0.008f) >= 0) ? 1 : -1;
            } else {
                active->anim.cx = CAT_X;
                active->anim.cy = CAT_Y;
                active->anim.act = ACT_SIT;
            }
            float wm = story_warmth(&story, (StoryZone)sz);
            /* never fully colorless — a whisper of pastel remains as a promise */
            render_set_warmth(0.22f + 0.78f * wm);
            if (location == LOC_FOREST) forest_draw(renderer, frame, is_night);
            else                        street_draw(renderer, frame, is_night);
            render_set_warmth(1.0f);
            cat_draw(renderer, &active->anim, col, frame);
            if (active->shiny)
                cat_draw_sparkles(renderer, &active->anim, frame);
            mood_draw(renderer, &active->anim, frame);
            active->anim.cx = save_x;
            active->anim.cy = save_y;
            active->anim.act = save_act;
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
        ui_draw_release_button(renderer, 4, 4, release_open);
        ui_button_draw(renderer, &btn_travel, press_fx > 0);
        ui_friends_button_draw(renderer, press_fx > 0);
        ui_quests_button_draw(renderer, false);   /* friends list */
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

        /* Friends list overlay sits above everything when open. */
        if (friends_open) {
            ui_friends_list(renderer, &friends);
        }

        /* Quest log overlay, above the scene. */
        if (quests_open) {
            ui_quests_list(renderer, &quests, quests_scroll);
        }

        /* The release confirmation sits above absolutely everything. */
        if (release_open) {
            ui_confirm_release(renderer, roster_active(&roster)->name);
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
    story_save(&story, KZ_STORY_PATH);
    quests_save(&quests, KZ_QUESTS_PATH);

    music_shutdown();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}