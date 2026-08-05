/* store.h — the department store: buy furniture and cat supplies.
 *
 * A two-floor shop. The furniture floor sells décor (plants, lamps, a cat
 * tower, cushions, rugs…) that you can then place at home. The supplies floor
 * restocks your pantry food (kibble, milk, treats, water). Everything costs
 * coins, which you earn from quests, leveling, and the Katlympics.
 *
 * The store draws its own screen and reports which item a tap lands on; main
 * applies the purchase against the pantry (coins) and décor/pantry stores.
 */
#ifndef KATIZTIC_STORE_H
#define KATIZTIC_STORE_H

#include <SDL3/SDL.h>
#include "decor.h"
#include "pantry.h"

typedef enum { STORE_FURNITURE, STORE_SUPPLIES } StoreFloor;

/* Draw the store on the given floor and page. `owned` marks furniture you
 * already have (shown as "owned" rather than a price). */
void store_draw(SDL_Renderer *r, StoreFloor floor, int page, const Decor *decor,
                const Pantry *pantry, Uint64 frame);

/* How many pages the given floor needs (so main can wrap the page index). */
int store_page_count(StoreFloor floor);

/* A tap result: what the player tapped. */
typedef enum {
    STORE_TAP_NONE,
    STORE_TAP_SWITCH_FLOOR,   /* the floor toggle                */
    STORE_TAP_NEXT_PAGE,      /* the "more items" page button    */
    STORE_TAP_BUY_DECOR,      /* buy furniture; `index` = DecorKind */
    STORE_TAP_BUY_FOOD,       /* buy food; `index` = FoodKind     */
} StoreTapKind;

typedef struct {
    StoreTapKind kind;
    int          index;
} StoreTap;

/* Hit-test a tap on the store screen for the given floor and page. */
StoreTap store_hit(StoreFloor floor, int page, float px, float py);

#endif /* KATIZTIC_STORE_H */