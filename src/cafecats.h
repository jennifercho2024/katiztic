/* cafecats.h — the adoptable cats who live at the cat café.
 *
 * The café is now a proper cat café: several cats lounge around on cushions,
 * the counter, and the floor, each waiting to be petted and — once they warm
 * up to you — adopted into your family. Petting a café cat raises its little
 * "friendship" meter; once full, a heart appears and you can adopt it.
 *
 * These are separate from your owned roster until adopted. A fresh set of
 * visitors rotates in over time so the café always feels lived-in.
 */
#ifndef KATIZTIC_CAFECATS_H
#define KATIZTIC_CAFECATS_H

#include <SDL3/SDL.h>
#include "cattype.h"
#include "cat.h"

#define CAFE_CATS_MAX 4     /* how many cats lounge in the café at once */
#define CAFE_FRIEND_FULL 100

typedef struct {
    bool     present;       /* is this slot occupied?                 */
    Cat      anim;          /* the sprite (for drawing/petting)       */
    CatType  type;
    bool     shiny;
    char     name[16];
    int      friendship;    /* 0..CAFE_FRIEND_FULL; pet to raise       */
    bool     adopted;       /* already taken home this visit?          */
    float    home_x, home_y;/* lounging spot                           */
} CafeCat;

typedef struct {
    CafeCat cats[CAFE_CATS_MAX];
    float   refresh_timer;  /* frames until a new cat may wander in     */
} CafeCats;

/* A fresh café population (called when you arrive). */
CafeCats cafecats_new(void);

/* Advance ambient behavior (gentle idle, occasional new arrival). */
void cafecats_update(CafeCats *cc, Uint64 frame);

/* Draw the lounging cats, with a friendship heart over any ready to adopt. */
void cafecats_draw(SDL_Renderer *r, const CafeCats *cc, Uint64 frame);

/* Which café cat is at a screen point, or -1. */
int  cafecats_hit(const CafeCats *cc, float px, float py);

/* Pet a café cat: raises friendship, returns true if it JUST became ready
 * to adopt (friendship reached full on this pet). */
bool cafecats_pet(CafeCats *cc, int index);

/* Is this café cat ready to be adopted (friendship full)? */
bool cafecats_ready(const CafeCats *cc, int index);

#endif /* KATIZTIC_CAFECATS_H */