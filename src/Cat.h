/* cat.h — the cat: a small bundle of state and three behaviors.
 *
 * The cat is the emotional center of Katiztic, so its state is kept small
 * and legible: where it sits, whether it's mid-blink, and how much petting
 * affection is currently radiating. Everything animated is derived from a
 * frame counter, not stored — so the cat is fully described by these few
 * fields plus the current frame.
 */
#ifndef KATIZTIC_CAT_H
#define KATIZTIC_CAT_H

#include <SDL3/SDL.h>

typedef struct {
    float cx, cy;      /* center of the cat, in logical pixels        */
    int   blink;       /* frames of blink remaining (0 = eyes open)   */
    int   next_blink;  /* frames until the next blink begins          */
    int   pet;         /* frames of petting glow remaining (0 = calm) */
} Cat;

/* Place a cat at a spot. */
Cat cat_make(float cx, float cy);

/* Advance one frame: tick blink timer, decay petting glow. */
void cat_update(Cat *cat);

/* The player pets the cat — start the purr/heart animation. */
void cat_pet(Cat *cat);

/* Is point (px,py) inside the cat's tappable area? (for petting) */
bool cat_hit(const Cat *cat, float px, float py);

/* Draw the cat. `frame` drives breathing, tail sway, floating hearts. */
void cat_draw(SDL_Renderer *r, const Cat *cat, Uint64 frame);

#endif /* KATIZTIC_CAT_H */