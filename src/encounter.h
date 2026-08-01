/* encounter.h — the wild cat you meet on a walk.
 *
 * When you're out in the meadow, sometimes a cat is there: a shy stray or a
 * neighbor's cat. Some are regulars you've met before, some are new faces.
 * You can offer her a treat to build trust; enough trust and she becomes a
 * lasting friend (recorded in Friends). She never joins your roster.
 *
 * An Encounter tracks who is currently visiting (if anyone), where she sits,
 * and a gentle approach animation as trust grows (a trusting cat sits closer).
 */
#ifndef KATIZTIC_ENCOUNTER_H
#define KATIZTIC_ENCOUNTER_H

#include <SDL3/SDL.h>
#include "cat.h"
#include "cattype.h"
#include "friends.h"

typedef struct {
    bool     present;      /* is a cat visiting right now?          */
    char     name[KZ_FRIEND_NAME];
    CatType  type;
    Cat      anim;         /* her sprite state (blink, etc.)        */
    float    home_x;       /* where she settles; nearer when trusted */
} Encounter;

/* No cat visiting. */
Encounter encounter_none(void);

/* Called when the player arrives in the meadow. With some chance, a cat comes
 * to visit — a mix of regulars (already-met friends) and brand-new faces.
 * `f` is consulted so regulars can reappear. Places her in the scene. */
Encounter encounter_begin(Friends *f);

/* Advance her animation; she edges a little closer as trust rises. */
void encounter_update(Encounter *e, const Friends *f);

/* Is point (px,py) on the visiting cat? (tap to approach / offer) */
bool encounter_hit(const Encounter *e, float px, float py);

/* Draw her in her type colors. */
void encounter_draw(SDL_Renderer *r, const Encounter *e, Uint64 frame);

#endif /* KATIZTIC_ENCOUNTER_H */