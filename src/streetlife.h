/* streetlife.h — people out walking their cats along the village street.
 *
 * A calm, living street: one or two neighbors stroll across at a time, each
 * with their own cat padding alongside. They enter from one edge, amble to the
 * other, and leave; a new walker wanders in a little later. You can tap a
 * walker to say hi and give their cat a friendly pet.
 *
 * This is the seed of the social world — later these owners can be befriended
 * and invite you on playdates. For now they're gentle ambient life you can
 * greet.
 */
#ifndef KATIZTIC_STREETLIFE_H
#define KATIZTIC_STREETLIFE_H

#include <SDL3/SDL.h>
#include "cattype.h"

#define STREETLIFE_MAX 2       /* at most two walkers at once (calm) */

typedef struct {
    bool     active;
    float    x, y;             /* the person's feet position          */
    int      dir;              /* +1 walking right, -1 walking left    */
    float    speed;
    CatType  cat_type;         /* the color of their cat               */
    Uint8    shirt_r, shirt_g, shirt_b;   /* person's shirt color      */
    Uint8    hair_r, hair_g, hair_b;      /* hair color                */
    const char *name;          /* the owner's name (for greetings)     */
    int      greet_glow;       /* frames of a happy glow after a tap   */
} Walker;

typedef struct {
    Walker  walkers[STREETLIFE_MAX];
    float   spawn_timer;       /* frames until the next walker wanders in */
} StreetLife;

/* Start an empty street (no walkers yet; first arrives shortly). */
StreetLife streetlife_new(void);

/* One frame: move walkers, retire those who've left, occasionally spawn a new
 * one (up to STREETLIFE_MAX). */
void streetlife_update(StreetLife *sl, Uint64 frame);

/* Draw all walkers (people + their cats). `night` tints them after dark. */
void streetlife_draw(SDL_Renderer *r, const StreetLife *sl, Uint64 frame,
                     bool night);

/* Which walker (if any) is under a tapped point; returns index or -1. */
int streetlife_hit(const StreetLife *sl, float px, float py);

/* Give a walker's cat a friendly pet: lights a happy glow, returns the owner's
 * name for a greeting (or NULL if the index is invalid). If out_type is
 * non-NULL, the walker's cat type is written there. */
const char *streetlife_greet(StreetLife *sl, int index, CatType *out_type);

#endif /* KATIZTIC_STREETLIFE_H */