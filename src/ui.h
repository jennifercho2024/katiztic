/* ui.h — the cozy status panel.
 *
 * Draws a soft, rounded-feeling panel in the corner showing the cat's four
 * stats as little pastel bars, each with a tiny icon so no font is needed:
 *   heart  = bond      (petal pink)
 *   smile  = mood      (butter)
 *   leaf   = energy    (mint)
 *   star   = growth    (lavender)
 *
 * Also draws the brief action hint at the bottom of the screen.
 */
#ifndef KATIZTIC_UI_H
#define KATIZTIC_UI_H

#include <SDL3/SDL.h>
#include "stats.h"
#include "roster.h"
#include "friends.h"

/* ---- touch-first buttons ----
 * A Button is just a labeled rectangle in logical 240x160 space. It's drawn
 * the same way and hit-tested the same way whether the player uses a mouse
 * (now) or a finger (on iOS later) — that's the whole point: input is a tap
 * at a point, and nothing in the game logic cares which device made it.
 *
 * Labels are drawn as a tiny icon glyph (no font needed yet), chosen by kind.
 */
typedef enum {
    KZ_BTN_HOME,    /* a little house — go home to the cottage */
    KZ_BTN_OUT,     /* a little sun/tree — go outside          */
    KZ_BTN_SLEEP,   /* a moon — sleep                          */
    KZ_BTN_TREAT,   /* a fish treat — offer to a wild cat      */
    KZ_BTN_FRIENDS, /* a heart — open the friends list         */
    KZ_BTN_DECOR,   /* a chair — open the décor tray           */
    KZ_BTN_QUESTS,  /* a checklist — open the quest log        */
} ButtonKind;

typedef struct {
    float x, y, w, h;
    ButtonKind kind;
} Button;

/* Is point (px,py) inside the button? Works for click or tap alike. */
bool ui_button_hit(const Button *b, float px, float py);

/* Draw the button: soft cream pill, mauve border, icon glyph. `pressed`
 * briefly darkens it for tap feedback. */
void ui_button_draw(SDL_Renderer *r, const Button *b, bool pressed);

/* Draw the stat panel for a cat at (x,y) in logical 240x160 space. Shows the
 * cat's name and type as a header, then her four stat bars. When `editing` is
 * true, the name row shows `edit_buf` with a blinking caret instead. */
void ui_draw_panel(SDL_Renderer *r, const OwnedCat *cat, float x, float y,
                   bool editing, const char *edit_buf, Uint64 frame);

/* Is point (px,py) on the name row of the stat card drawn at (x,y)?
 * (tap to rename) */
bool ui_name_hit(float panel_x, float panel_y, float px, float py);

/* Is point (px,py) on the little release ("×") button on the stat card?
 * `armed` shows it in a confirming state. */
bool ui_release_hit(float panel_x, float panel_y, float px, float py);
void ui_draw_release_button(SDL_Renderer *r, float panel_x, float panel_y,
                            bool armed);

/* A centered "Are you sure?" dialog for releasing a cat, with Yes/No buttons.
 * Draw it on top of everything; hit returns 1 = Yes, 0 = No, -1 = neither
 * (a tap outside also reads as No in the caller). */
void ui_confirm_release(SDL_Renderer *r, const char *cat_name);
int  ui_confirm_release_hit(float px, float py);

/* ---- encounter UI ----
 * A soft dialogue banner along the bottom with a line of text, used when a
 * wild cat is visiting on a walk. */
void ui_banner(SDL_Renderer *r, const char *line);

/* A treat-offer button (bottom-right). Draw it, and hit-test it. */
extern const Button KZ_TREAT_BUTTON;   /* fixed position */
void ui_treat_button_draw(SDL_Renderer *r, bool pressed);
bool ui_treat_button_hit(float px, float py);

/* A small "friends" button (top area) to open the friends list. */
extern const Button KZ_FRIENDS_BUTTON;
void ui_friends_button_draw(SDL_Renderer *r, bool pressed);
bool ui_friends_button_hit(float px, float py);

/* The friends-list overlay: everyone you've met, trust shown as a bar, full
 * friends marked. Returns nothing; tapping anywhere closes it (handled in
 * main). */
void ui_friends_list(SDL_Renderer *r, const Friends *f);

/* ---- quests: a checklist button and the quest-log overlay ---- */
#include "quests.h"
extern const Button KZ_QUESTS_BUTTON;
void ui_quests_button_draw(SDL_Renderer *r, bool pressed);
bool ui_quests_button_hit(float px, float py);
void ui_quests_list(SDL_Renderer *r, const Quests *q, int scroll);

/* ---- décor tray ----
 * A décor button (top-right) opens a tray along the bottom showing the items
 * you own. Drag one out into the room to place it. */
extern const Button KZ_DECOR_BUTTON;
void ui_decor_button_draw(SDL_Renderer *r, bool pressed);
bool ui_decor_button_hit(float px, float py);

/* Draw the tray of owned items. Returns via out-params the tray's top y so
 * main can tell "dragged out of the tray into the room". */
#include "decor.h"
void ui_decor_tray(SDL_Renderer *r, const Decor *d, Uint64 frame);

/* Which owned item's tray slot is at (px,py), or -1. Only owned items appear. */
int  ui_decor_tray_hit(const Decor *d, float px, float py);

/* The y-coordinate of the top of the tray (things dropped above it land in
 * the room; things below are still "in the tray"). */
float ui_decor_tray_top(void);

/* Draw a small centered hint bar near the bottom (e.g. "F feed  G groom").
 * `flash` > 0 briefly highlights the panel to acknowledge an action. */
void ui_draw_hint(SDL_Renderer *r);

/* ---- travel place-picker ----
 * Tapping the travel button opens a small menu of the three places. Draw the
 * menu (highlighting `current`), and hit-test a tap. Returns 0=cottage,
 * 1=meadow, 2=cafe, or -1 for no hit (a tap outside closes it). */
void ui_place_menu(SDL_Renderer *r, int current);
int  ui_place_menu_hit(float px, float py);

/* ---- roster strip ----
 * A row of little cat portraits along the bottom, each in its type's color,
 * with the active cat highlighted, plus a "+" slot to adopt if there's room.
 * Draw it, and hit-test taps against it.
 */
void ui_roster_draw(SDL_Renderer *r, const Roster *ro);

/* Given a tap at (px,py), return the portrait index tapped (0..count-1),
 * or -2 for the "+" adopt slot, or -1 for no hit. */
int  ui_roster_hit(const Roster *ro, float px, float py);

#endif /* KATIZTIC_UI_H */