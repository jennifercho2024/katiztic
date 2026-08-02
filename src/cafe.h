/* cafe.h — the cozy cat café, a social spot to visit.
 *
 * Drawn in the same pastel language as the cottage and meadow. It's a warm
 * little lounge — a counter with treats, small tables, floor cushions — where
 * your cats hang out and socialize (they roam and play here just like at home,
 * reusing the behavior system). Its own gentle, jazzy music theme plays here.
 */
#ifndef KATIZTIC_CAFE_H
#define KATIZTIC_CAFE_H

#include <SDL3/SDL.h>

/* Draw the café interior. `frame` drives gentle ambient motion (steam curling
 * off a cup, a swaying hanging plant). */
void cafe_draw(SDL_Renderer *r, Uint64 frame);

#endif /* KATIZTIC_CAFE_H */