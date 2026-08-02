/* behavior.h — the little brain that makes the cats feel alive at home.
 *
 * Each cat picks an activity — sitting, wandering, grooming, sleeping, or
 * playing with a nearby friend — holds it for a while, then chooses another.
 * They roam independently, so the cottage feels like a room full of cats each
 * doing their own thing. Playing happens when two wandering cats drift close
 * to each other and pair up for a little bounce.
 *
 * This is purely visual/behavioral: it never touches stats or saves.
 */
#ifndef KATIZTIC_BEHAVIOR_H
#define KATIZTIC_BEHAVIOR_H

#include <SDL3/SDL.h>
#include "cat.h"   /* Activity enum lives here */
#include "roster.h"
#include "decor.h"

/* Advance every cat's behavior one frame: tick activity timers, move walkers
 * toward their targets, pair up nearby cats to play, and let cats notice and
 * react to placed yarn (bat at it) and milk (lap it). `decor` may be NULL in
 * places without décor (the meadow, café). */
void behavior_update(Roster *ro, Decor *decor, Uint64 frame);

#endif /* KATIZTIC_BEHAVIOR_H */