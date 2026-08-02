/* pantry.h — your coins and the food supplies you've stocked up.
 *
 * Coins are earned by caring for your cats and finishing quests, and spent at
 * the flea market on the four foods: cat food, milk, treats, and water. The
 * pantry remembers how many of each you own (used by the cottage feed array),
 * plus your coin purse. It saves to its own file.
 */
#ifndef KATIZTIC_PANTRY_H
#define KATIZTIC_PANTRY_H

#include <SDL3/SDL.h>

typedef enum {
    FOOD_KIBBLE,   /* cat food */
    FOOD_MILK,
    FOOD_TREAT,
    FOOD_WATER,
    FOOD_COUNT
} FoodKind;

typedef struct {
    Uint16 coins;
    Uint16 stock[FOOD_COUNT];   /* how many of each food you own */
} Pantry;

/* Display name and price for a food. */
const char *food_name(FoodKind f);
int         food_price(FoodKind f);

/* A fresh pantry: a small starting purse and a couple of each food so the
 * cottage feed array works from the very first play. */
Pantry pantry_new(void);

/* Coins. add is clamped to a sane ceiling; spend returns false (buys nothing)
 * if you can't afford it. */
void pantry_earn(Pantry *p, int coins);
bool pantry_spend(Pantry *p, int coins);

/* Buy one unit of a food if affordable: spends its price, adds it to stock.
 * Returns true on success. */
bool pantry_buy(Pantry *p, FoodKind f);

/* Use one unit of a food (the cottage feed array). Returns false if none left. */
bool pantry_use(Pantry *p, FoodKind f);

bool pantry_save(const Pantry *p, const char *path);
bool pantry_load(Pantry *p, const char *path);

#endif /* KATIZTIC_PANTRY_H */