/* forest.h — the forest, the first zone touched by the story.
 *
 * A quiet wood past the meadow: mauve-barked trees under a layered mint
 * canopy, a winding path over mossy ground, ferns and little mushrooms, and
 * soft golden motes drifting up — the forest's sleeping magic.
 *
 * When you first arrive, its color has faded to grey. Visiting with a cat
 * you've bonded with slowly brings the warmth back (see story.h); the zone is
 * drawn through the render layer's warmth filter, so the re-coloring happens
 * to this art literally, pixel by pixel.
 */
#ifndef KATIZTIC_FOREST_H
#define KATIZTIC_FOREST_H

#include <SDL3/SDL.h>

/* Draw the forest. `frame` drives the gentle sway and drifting motes;
 * `night` dims it after dark like the other places. */
void forest_draw(SDL_Renderer *r, Uint64 frame, bool night);

#endif /* KATIZTIC_FOREST_H */