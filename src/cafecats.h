/* cafecats.h — the resident cats (and patrons) of the cat café.
 *
 * The café is a proper cat café: four cats live here — the same friendly faces
 * every visit — lounging on cushions and the floor. You can pet them any time
 * you drop by (they show a happy little heart), and your own cats can play
 * with them too. A rotating handful of people (patrons) also enjoy the café,
 * a different little crowd each visit.
 */
#ifndef KATIZTIC_CAFECATS_H
#define KATIZTIC_CAFECATS_H

#include <SDL3/SDL.h>
#include "cattype.h"
#include "cat.h"

#define CAFE_CATS_MAX 4     /* how many cats live in the café */
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
    float   refresh_timer;  /* (unused now — residents are permanent)   */
    /* café patrons: a few people enjoying the café, different each visit */
    int     patron_count;
    float   patron_x[3];
    int     patron_shirt[3];   /* which shirt color                     */
    int     patron_has_cat[3]; /* is a little cat sitting with them?     */
} CafeCats;

/* A fresh café population (called when you arrive). */
CafeCats cafecats_new(void);

/* Advance ambient behavior (gentle idle, occasional new arrival). */
void cafecats_update(CafeCats *cc, Uint64 frame);

/* Draw the lounging cats, with a friendship heart over any ready to adopt. */
void cafecats_draw(SDL_Renderer *r, const CafeCats *cc, Uint64 frame);

/* Which café cat is at a screen point, or -1. */
int  cafecats_hit(const CafeCats *cc, float px, float py);

/* Draw the café patrons (people) — call before drawing the cats. */
void cafecats_draw_patrons(SDL_Renderer *r, const CafeCats *cc, Uint64 frame);

/* Pet a café resident: they show a happy heart. */
void cafecats_pet(CafeCats *cc, int index);

#endif /* KATIZTIC_CAFECATS_H */