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
#include "cattype.h"

/* What a cat is currently doing — drives both its behavior and how it's drawn.
 * (The logic that transitions between these lives in behavior.c.) */
typedef enum {
    ACT_SIT,      /* sitting still, breathing (the default resting pose) */
    ACT_WALK,     /* strolling toward a target spot                      */
    ACT_GROOM,    /* licking / grooming in place                         */
    ACT_SLEEP,    /* curled up napping                                   */
    ACT_PLAY,     /* batting at a nearby cat                             */
} Activity;

typedef struct {
    float cx, cy;      /* center of the cat, in logical pixels        */
    int   blink;       /* frames of blink remaining (0 = eyes open)   */
    int   next_blink;  /* frames until the next blink begins          */
    int   pet;         /* frames of petting glow remaining (0 = calm) */
    /* --- behavior state (managed by behavior.c) --- */
    Activity act;      /* what she's doing right now                  */
    int   act_timer;   /* frames left in the current activity         */
    float tx, ty;      /* wander target (for ACT_WALK)                */
    int   facing;      /* -1 faces left, +1 faces right               */
    Uint64 act_seed;   /* per-cat phase offset so they're not in sync */
    int   mood_timer;  /* frames a mood bubble is showing (0 = none)  */
    int   mood_next;   /* frames until the next bubble may appear      */
    int   mood_kind;   /* which emoji is showing (see mood.h)          */
    /* --- trick performance (set by main when practicing) --- */
    int   trick;       /* which trick is being performed (-1 = none)  */
    int   trick_t;     /* frames remaining in the trick animation     */
    int   trick_len;   /* total length of the current trick animation */
} Cat;

/* Place a cat at a spot. */
Cat cat_make(float cx, float cy);

/* Advance one frame: tick blink timer, decay petting glow. */
void cat_update(Cat *cat);

/* The player pets the cat — start the purr/heart animation. */
void cat_pet(Cat *cat);

/* Trick poses the cat can perform (match tricks.h TrickId order). */
#define CAT_TRICK_NONE     (-1)
#define CAT_TRICK_SIT      0
#define CAT_TRICK_SPIN     1
#define CAT_TRICK_JUMP     2
#define CAT_TRICK_HIGHFIVE 3
#define CAT_TRICK_ROLL     4

/* Make the cat perform a trick (a short animation you can watch). */
void cat_do_trick(Cat *cat, int trick);

/* Is point (px,py) inside the cat's tappable area? (for petting) */
bool cat_hit(const Cat *cat, float px, float py);

/* Draw the cat in its type's colors. `frame` drives breathing, tail sway,
 * and floating hearts. Color lives with the cat's type, not its animation
 * state, so it's passed in rather than stored on the Cat. */
void cat_draw(SDL_Renderer *r, const Cat *cat, CatColors col, Uint64 frame);

/* A golden recolor of any cat's palette, for shiny cats. */
CatColors cat_shiny_colors(void);

/* Draw sparkles around a shiny cat, on top of her sprite. Call after cat_draw
 * when the cat is shiny. */
void cat_draw_sparkles(SDL_Renderer *r, const Cat *cat, Uint64 frame);

/* A soft glowing ring under a cat, marking it as the currently selected one
 * so you can tell your cats apart at a glance. */
void cat_draw_select_ring(SDL_Renderer *r, const Cat *cat, Uint64 frame);

#endif /* KATIZTIC_CAT_H */