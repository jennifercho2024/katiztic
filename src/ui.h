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

/* Draw the stat panel at (x,y) in logical 240x160 space. */
void ui_draw_panel(SDL_Renderer *r, const Stats *s, float x, float y);

/* Draw a small centered hint bar near the bottom (e.g. "F feed  G groom").
 * `flash` > 0 briefly highlights the panel to acknowledge an action. */
void ui_draw_hint(SDL_Renderer *r);

#endif /* KATIZTIC_UI_H */