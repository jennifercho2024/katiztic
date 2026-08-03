/* title.h — the Katiztic title screen, drawn in code (crisp at any size).
 *
 * A lavender splash in the game's own hand-drawn pixel style: gradient sky,
 * sparkles, soft clouds, a little cottage, lavender sprigs, the game title, and
 * a pulsing "press start". Drawn fresh each frame — no image file, no blur.
 */
#ifndef KATIZTIC_TITLE_H
#define KATIZTIC_TITLE_H

#include <SDL3/SDL.h>

/* Draw the whole title screen for this frame (240x160 logical canvas). */
void title_draw(SDL_Renderer *r, Uint64 frame);

#endif /* KATIZTIC_TITLE_H */