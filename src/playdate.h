/* playdate.h — a cozy playdate: your cat and a friend's cat play together.
 *
 * Accepting a letter from a befriended owner brings you here: a soft, sunny
 * spot (a picnic blanket under a tree) where your active cat and the visiting
 * owner's cat bounce and chase and nap together. You watch, and you can pet
 * either cat; their bond grows the longer they play. A little meter fills as
 * they enjoy themselves, and when it's full the playdate is a happy success.
 */
#ifndef KATIZTIC_PLAYDATE_H
#define KATIZTIC_PLAYDATE_H

#include <SDL3/SDL.h>
#include "cattype.h"
#include "cat.h"

typedef struct {
    bool     active;
    char     owner[16];
    Cat      guest;        /* the friend's cat                 */
    CatType  guest_type;
    float    joy;          /* 0..1 how much fun they've had     */
    Uint64   started;      /* frame the playdate began          */
} Playdate;

/* Begin a playdate with a named owner's cat of the given type. */
Playdate playdate_begin(const char *owner, CatType guest_type, Uint64 frame);
Playdate playdate_none(void);

/* One frame: animate the guest cat, drift the two cats through play, and raise
 * joy a little. `your_cat` is your active cat's anim (moved for the scene by
 * the caller). Returns true once joy is full (playdate complete). */
bool playdate_update(Playdate *pd, Cat *your_cat, Uint64 frame);

/* Draw the playdate scene: the cozy spot, both cats, and the joy meter. */
void playdate_draw(SDL_Renderer *r, const Playdate *pd, const Cat *your_cat,
                   CatColors your_colors, Uint64 frame);

/* Tap handling: pet whichever cat is under the point. Returns 1 for your cat,
 * 2 for the guest, 0 for neither. */
int playdate_hit(const Playdate *pd, const Cat *your_cat, float px, float py);

#endif /* KATIZTIC_PLAYDATE_H */