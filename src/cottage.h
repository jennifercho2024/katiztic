/* cottage.h — the cat's home base, drawn in the same pastel language.
 *
 * A cozy one-room interior: warm floor, a window to the sky, a soft bed you
 * can tap to sleep, a rug, a food bowl. Sleeping in the bed always wakes you
 * to a fresh morning (the coziest choice — you're never locked out by time).
 *
 * The bed's tappable area is exposed so main.c can hit-test taps against it.
 */
#ifndef KATIZTIC_COTTAGE_H
#define KATIZTIC_COTTAGE_H

#include <SDL3/SDL.h>

/* The bed's rectangle in logical 240x160 space, for tap detection. */
typedef struct { float x, y, w, h; } Rect;
Rect cottage_bed_rect(void);

/* Is point (px,py) on the bed? (tap to sleep) */
bool cottage_bed_hit(float px, float py);

/* Draw the cottage interior. `frame` drives gentle ambient motion (a swaying
 * curtain, dust motes in the window light). `night` softly dims the room. */
void cottage_draw(SDL_Renderer *r, Uint64 frame, bool night);

#endif /* KATIZTIC_COTTAGE_H */