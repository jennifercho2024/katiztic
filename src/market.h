/* market.h — the flea market, where you spend coins on food supplies.
 *
 * A row of cheerful little stalls under striped awnings, each selling one of
 * the four foods. Your coin purse shows in the corner; tap a stall (or its buy
 * button) to buy one unit if you can afford it. Purchases go straight into the
 * pantry for the cottage feed array to use.
 */
#ifndef KATIZTIC_MARKET_H
#define KATIZTIC_MARKET_H

#include <SDL3/SDL.h>
#include "pantry.h"

/* Draw the flea market scene: stalls, awnings, wares, prices, and the purse. */
void market_draw(SDL_Renderer *r, const Pantry *p, Uint64 frame);

/* Which stall (FoodKind) was tapped, or -1 if none. */
int market_hit(float px, float py);

#endif /* KATIZTIC_MARKET_H */