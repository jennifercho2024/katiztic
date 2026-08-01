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

/* Draw the stat panel at (x,y) in logical 240x160 space. */
void ui_draw_panel(SDL_Renderer *r, const Stats *s, float x, float y);

/* Draw a small centered hint bar near the bottom (e.g. "F feed  G groom").
 * `flash` > 0 briefly highlights the panel to acknowledge an action. */
void ui_draw_hint(SDL_Renderer *r);

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