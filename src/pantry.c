/* pantry.c — see pantry.h. Coins, food stock, and remembering them. */
#include "pantry.h"

static const char *NAMES[FOOD_COUNT] = { "Cat Food", "Milk", "Treat", "Water" };
static const int   PRICES[FOOD_COUNT] = { 5, 4, 6, 2 };

const char *food_name(FoodKind f) {
    if (f < 0 || f >= FOOD_COUNT) return "?";
    return NAMES[f];
}
int food_price(FoodKind f) {
    if (f < 0 || f >= FOOD_COUNT) return 0;
    return PRICES[f];
}

Pantry pantry_new(void) {
    Pantry p;
    p.coins = 12;                 /* a small starting purse */
    p.stock[FOOD_KIBBLE] = 3;
    p.stock[FOOD_MILK]   = 2;
    p.stock[FOOD_TREAT]  = 1;
    p.stock[FOOD_WATER]  = 4;
    return p;
}

void pantry_earn(Pantry *p, int coins) {
    if (coins <= 0) return;
    long c = (long)p->coins + coins;
    if (c > 9999) c = 9999;
    p->coins = (Uint16)c;
}

bool pantry_spend(Pantry *p, int coins) {
    if (coins < 0) return false;
    if ((int)p->coins < coins) return false;
    p->coins = (Uint16)(p->coins - coins);
    return true;
}

bool pantry_buy(Pantry *p, FoodKind f) {
    if (f < 0 || f >= FOOD_COUNT) return false;
    if (!pantry_spend(p, PRICES[f])) return false;
    if (p->stock[f] < 999) p->stock[f]++;
    return true;
}

bool pantry_use(Pantry *p, FoodKind f) {
    if (f < 0 || f >= FOOD_COUNT) return false;
    if (p->stock[f] == 0) return false;
    p->stock[f]--;
    return true;
}

/* ---- persistence ---- */

static const char MAGIC[4] = { 'K', 'Z', 'P', 'N' };
#define PANTRY_SAVE_VERSION 1u

bool pantry_save(const Pantry *p, const char *path) {
    SDL_IOStream *io = SDL_IOFromFile(path, "wb");
    if (!io) return false;
    bool ok = SDL_WriteIO(io, MAGIC, 4) == 4;
    Uint8 ver = PANTRY_SAVE_VERSION;
    Uint8 count = (Uint8)FOOD_COUNT;
    ok = ok && SDL_WriteIO(io, &ver, 1) == 1;
    ok = ok && SDL_WriteIO(io, &count, 1) == 1;
    ok = ok && SDL_WriteIO(io, &p->coins, sizeof p->coins) == sizeof p->coins;
    for (int i = 0; i < FOOD_COUNT && ok; i++)
        ok = ok && SDL_WriteIO(io, &p->stock[i], sizeof p->stock[i])
                     == sizeof p->stock[i];
    SDL_CloseIO(io);
    return ok;
}

bool pantry_load(Pantry *p, const char *path) {
    SDL_IOStream *io = SDL_IOFromFile(path, "rb");
    if (!io) return false;
    char magic[4];
    Uint8 ver = 0, count = 0;
    bool ok = SDL_ReadIO(io, magic, 4) == 4;
    ok = ok && SDL_memcmp(magic, MAGIC, 4) == 0;
    ok = ok && SDL_ReadIO(io, &ver, 1) == 1 && ver == PANTRY_SAVE_VERSION;
    ok = ok && SDL_ReadIO(io, &count, 1) == 1
            && count >= 1 && count <= FOOD_COUNT;
    Pantry tmp = pantry_new();
    tmp.stock[FOOD_KIBBLE] = tmp.stock[FOOD_MILK] = 0;
    tmp.stock[FOOD_TREAT] = tmp.stock[FOOD_WATER] = 0;
    ok = ok && SDL_ReadIO(io, &tmp.coins, sizeof tmp.coins) == sizeof tmp.coins;
    for (int i = 0; i < (int)count && ok; i++)
        ok = ok && SDL_ReadIO(io, &tmp.stock[i], sizeof tmp.stock[i])
                     == sizeof tmp.stock[i];
    SDL_CloseIO(io);
    if (ok) *p = tmp;
    return ok;
}