/* street.h — the village street, a lane to walk your cat along.
 *
 * A row of pastel house fronts, a sidewalk, cobbles, lampposts that glow after
 * dark, flower boxes under the windows. Your cat strolls the pavement with you
 * rather than sitting — it's a walk, after all.
 *
 * Like every new zone (the story's option A), the street arrives FADED: its
 * color has drained to grey, and visiting with your bonded cat — and petting
 * her — brings the warmth back.
 */
#ifndef KATIZTIC_STREET_H
#define KATIZTIC_STREET_H

#include <SDL3/SDL.h>

/* Draw the street. `frame` drives ambient motion; `night` dims the scene and
 * lights the lamps. */
void street_draw(SDL_Renderer *r, Uint64 frame, bool night);
void street_draw_wide(SDL_Renderer *r, Uint64 frame, bool night, int room_w);
/* The continuous street stroll: scenery (lampposts, benches, bicycles, vending
 * machines, bushes) scrolls endlessly past as your cat walks the sidewalk. */
void street_walk_draw(SDL_Renderer *r, float scroll, Uint64 frame, bool night);

/* The street is wider than the screen so you can stroll down the lane. */
#define STREET_ROOM_W 480

#endif /* KATIZTIC_STREET_H */