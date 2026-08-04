/* park.h — the neighborhood park: a playground for cats.
 *
 * An open green with cat-friendly playground equipment — a climbing tree, a
 * little slide, a tunnel, and a ball pit — where your cats roam and play, you
 * can practice tricks out in the open, and other cats and owners drop by to
 * enjoy the day. A cheerful, social place.
 *
 * The park scene itself is drawn here; roaming of your own cats, trick practice,
 * and the visiting cats are wired in main using the existing systems.
 */
#ifndef KATIZTIC_PARK_H
#define KATIZTIC_PARK_H

#include <SDL3/SDL.h>
#include "cattype.h"

/* Draw the park scene: grass, path, and the playground equipment. `night`
 * tints it for evening visits. */
void park_draw(SDL_Renderer *r, Uint64 frame, bool night);

/* Draw the scenic walking path: scenery drifts by as you and your cat stroll.
 * `scroll` advances as you walk, moving trees, flowers, and lamp posts past. */
void park_walk_draw(SDL_Renderer *r, float scroll, Uint64 frame, bool night);

/* ---- park visitors: other cats (with owners) enjoying the park ---- */

#define PARK_VISITOR_MAX 2

typedef struct {
    bool     active;
    float    x, y;
    int      dir;
    float    speed;
    CatType  cat_type;
    const char *owner;      /* the visiting owner's name */
    int      glow;          /* happy glow after you greet them */
    float    wander_timer;
} ParkVisitor;

typedef struct {
    ParkVisitor v[PARK_VISITOR_MAX];
    float       spawn_timer;
} ParkLife;

ParkLife parklife_new(void);
void parklife_update(ParkLife *pl, Uint64 frame);
void parklife_draw(SDL_Renderer *r, const ParkLife *pl, Uint64 frame);
int  parklife_hit(const ParkLife *pl, float px, float py);
/* greet a visitor: lights a glow, returns the owner name (or NULL). */
const char *parklife_greet(ParkLife *pl, int index, CatType *out_type);

#endif /* KATIZTIC_PARK_H */