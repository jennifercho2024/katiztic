/* roster.h — your family of cats: up to five, each its own little life.
 *
 * This is the rung where Katiztic goes from "a cat" to "your cats." A Roster
 * holds up to KZ_MAX_CATS owned cats. Each cat has a name, a type (which gives
 * its colors and personality), its own Stats, and its own animation state.
 * One cat is "active" — the one who appears in the scene and receives care.
 *
 * The save file (bumped to version 2) stores the whole roster.
 */
#ifndef KATIZTIC_ROSTER_H
#define KATIZTIC_ROSTER_H

#include <SDL3/SDL.h>
#include "cat.h"
#include "cattype.h"
#include "stats.h"

#define KZ_MAX_CATS   5
#define KZ_NAME_LEN   12   /* including the null terminator */

typedef struct {
    char    name[KZ_NAME_LEN];
    CatType type;
    Stats   stats;
    Cat     anim;    /* per-cat animation state (blink, pet glow)   */
} OwnedCat;

typedef struct {
    OwnedCat cats[KZ_MAX_CATS];
    int      count;    /* how many cats you have (1..KZ_MAX_CATS)    */
    int      active;   /* index of the cat currently in the scene    */
} Roster;

/* A fresh roster: two starter cats of different types so you can swap right
 * away, placed at the scene's cat spot. */
Roster roster_new(float cat_x, float cat_y);

/* The active cat — convenience accessors. */
OwnedCat *roster_active(Roster *ro);

/* Make cat `i` the active one (bounds-checked; no-op if out of range). */
void roster_select(Roster *ro, int i);

/* Adopt a new cat of the given type. Returns false if the team is full. */
bool roster_adopt(Roster *ro, CatType type, float cat_x, float cat_y);

/* ---- persistence (save version 2) ---- */
bool roster_save(const Roster *ro, const char *path);
bool roster_load(Roster *out, const char *path, float cat_x, float cat_y);

#endif /* KATIZTIC_ROSTER_H */