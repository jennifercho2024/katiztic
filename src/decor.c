/* decor.c — see decor.h. The catalog, the unlock rules, and how each item
 * is drawn in the pastel style. */
#include "decor.h"
#include "render.h"
#include "palette.h"
#include <string.h>
#include <math.h>

/* The catalog. Each item says how it's unlocked and at what threshold. */
static const DecorInfo INFO[DECOR_COUNT] = {
    { DECOR_PLANT,   "Plant",   UNLOCK_START,   0 },
    { DECOR_CUSHION, "Cushion", UNLOCK_START,   0 },
    { DECOR_LAMP,    "Lamp",    UNLOCK_BOND,    50 },  /* a cat bond >= 50 */
    { DECOR_PICTURE, "Picture", UNLOCK_FRIENDS, 1  },  /* 1 friend made    */
    { DECOR_RUG2,    "Rug",     UNLOCK_FAMILY,  3  },  /* 3 cats owned     */
    { DECOR_TOWER,   "Tower",   UNLOCK_BOND,    90 },  /* a cat bond >= 90 */
    { DECOR_YARN,    "Yarn",    UNLOCK_FRIENDS, 2  },  /* 2 friends made   */
    { DECOR_MILK,    "Milk",    UNLOCK_LEVEL,   8  },  /* total levels >= 8 */
};

const DecorInfo *decor_info(DecorKind k) {
    if (k < 0 || k >= DECOR_COUNT) k = DECOR_PLANT;
    /* INFO isn't indexed by enum value directly (it's ordered by unlock), so
     * find the matching entry. */
    for (int i = 0; i < DECOR_COUNT; i++)
        if (INFO[i].kind == k) return &INFO[i];
    return &INFO[0];
}

Decor decor_new(void) {
    Decor d;
    for (int i = 0; i < DECOR_COUNT; i++) {
        d.items[i].owned  = false;
        d.items[i].placed = false;
        d.items[i].x = 120.0f;
        d.items[i].y = 110.0f;
    }
    /* START items begin owned. */
    for (int i = 0; i < DECOR_COUNT; i++) {
        const DecorInfo *in = decor_info((DecorKind)i);
        if (in->unlock == UNLOCK_START) d.items[i].owned = true;
    }
    return d;
}

int decor_check_unlocks(Decor *d, int max_bond, int friends_count,
                        int family_count, int total_levels) {
    int newly = 0;
    for (int i = 0; i < DECOR_COUNT; i++) {
        if (d->items[i].owned) continue;
        const DecorInfo *in = decor_info((DecorKind)i);
        bool ok = false;
        switch (in->unlock) {
            case UNLOCK_START:   ok = true; break;
            case UNLOCK_BOND:    ok = (max_bond      >= in->threshold); break;
            case UNLOCK_FRIENDS: ok = (friends_count >= in->threshold); break;
            case UNLOCK_FAMILY:  ok = (family_count  >= in->threshold); break;
            case UNLOCK_LEVEL:   ok = (total_levels  >= in->threshold); break;
        }
        if (ok) { d->items[i].owned = true; newly++; }
    }
    return newly;
}

/* ---- drawing each item, in the pastel palette ---- */

static void draw_plant(SDL_Renderer *r, float x, float y, Uint64 frame) {
    float sway = sinf((float)frame * 0.03f) * 1.0f;
    /* pot */
    px_rect(r, x, y + 10, 12, 8, rgb(0xE8,0xBB,0xA0));
    px_rect(r, x + 1, y + 9, 10, 2, rgb(0xF0,0xCE,0xB4));
    /* leaves */
    px_rect(r, x + 4 + sway, y - 2, 4, 12, KZ_MINT);
    px_rect(r, x + 1 + sway, y + 2, 4, 6,  rgb(0xB4,0xDE,0xC6));
    px_rect(r, x + 8 + sway, y + 1, 4, 7,  rgb(0xB4,0xDE,0xC6));
}

static void draw_lamp(SDL_Renderer *r, float x, float y, Uint64 frame) {
    (void)frame;
    /* soft glow */
    px_rect_a(r, x - 2, y - 4, 16, 12, KZ_BUTTER, 90);
    /* shade */
    px_rect(r, x + 1, y - 2, 10, 7, KZ_BUTTER);
    px_rect(r, x + 2, y - 4, 8, 2, rgb(0xF7,0xEE,0xD4));
    /* stand */
    px_rect(r, x + 5, y + 5, 2, 12, rgb(0xC8,0xA8,0xB0));
    px_rect(r, x + 2, y + 17, 8, 2, rgb(0xC8,0xA8,0xB0));
}

static void draw_picture(SDL_Renderer *r, float x, float y, Uint64 frame) {
    (void)frame;
    px_rect(r, x, y, 18, 14, rgb(0xC8,0xA8,0xB0));       /* frame */
    px_rect(r, x + 2, y + 2, 14, 10, KZ_SKY_WASH);        /* sky   */
    px_rect(r, x + 4, y + 8, 10, 2, KZ_MINT);             /* hill  */
    px_rect(r, x + 11, y + 4, 3, 3, KZ_BUTTER);           /* sun   */
}

static void draw_tower(SDL_Renderer *r, float x, float y, Uint64 frame) {
    (void)frame;
    px_rect(r, x + 2, y + 6, 12, 18, rgb(0xE0,0xC6,0xC0)); /* post */
    px_rect(r, x - 2, y, 20, 8, rgb(0xF5,0xD8,0xE4));      /* top platform */
    px_rect(r, x - 2, y, 20, 2, rgb(0xE8,0xBB,0xD0));
    px_rect(r, x, y + 22, 16, 3, rgb(0xE0,0xC6,0xC0));    /* base */
}

static void draw_cushion(SDL_Renderer *r, float x, float y, Uint64 frame) {
    (void)frame;
    px_rect(r, x, y + 2, 18, 8, KZ_LAVENDER);
    px_rect(r, x + 1, y + 1, 16, 2, rgb(0xE0,0xD2,0xF0));
    px_rect(r, x + 1, y + 9, 16, 2, rgb(0xC0,0xAE,0xDC));
    px_rect(r, x + 8, y + 4, 2, 4, rgb(0xC0,0xAE,0xDC)); /* tuft */
}

static void draw_rug2(SDL_Renderer *r, float x, float y, Uint64 frame) {
    (void)frame;
    px_rect(r, x, y, 30, 14, KZ_PETAL_PINK);
    px_rect(r, x + 3, y + 3, 24, 8, rgb(0xF5,0xDC,0xE6));
    px_rect(r, x + 6, y + 6, 18, 2, KZ_PETAL_PINK);
}

static void draw_yarn(SDL_Renderer *r, float x, float y, Uint64 frame) {
    (void)frame;
    Color yarn = rgb(0xE8, 0x9C, 0xB4);   /* rosy pink yarn */
    Color dark = rgb(0xD0, 0x82, 0x9C);
    /* a round ball */
    px_rect(r, x + 2, y + 1, 8, 1, yarn);
    px_rect(r, x + 1, y + 2, 10, 8, yarn);
    px_rect(r, x + 2, y + 10, 8, 1, yarn);
    /* wound-thread lines */
    px_rect(r, x + 2, y + 3, 8, 1, dark);
    px_rect(r, x + 3, y + 6, 7, 1, dark);
    px_rect(r, x + 2, y + 8, 6, 1, dark);
    /* a little loose end trailing off */
    px_rect(r, x + 10, y + 8, 3, 1, yarn);
    px_rect(r, x + 12, y + 9, 2, 1, yarn);
}

static void draw_milk(SDL_Renderer *r, float x, float y, Uint64 frame) {
    (void)frame;
    /* a shallow saucer of milk */
    Color saucer = rgb(0xC8, 0xB8, 0xC8);
    Color milk   = KZ_CLOUD;
    px_rect(r, x, y + 6, 16, 4, saucer);      /* dish */
    px_rect(r, x + 1, y + 4, 14, 2, milk);     /* milk surface */
    px_rect(r, x + 3, y + 3, 10, 1, milk);     /* little meniscus */
    px_rect(r, x, y + 9, 16, 1, rgb(0xB0,0xA0,0xB4)); /* base shadow */
}

void decor_draw_one(SDL_Renderer *r, DecorKind k, float x, float y,
                    Uint64 frame) {
    switch (k) {
        case DECOR_PLANT:   draw_plant(r, x, y, frame);   break;
        case DECOR_LAMP:    draw_lamp(r, x, y, frame);    break;
        case DECOR_PICTURE: draw_picture(r, x, y, frame); break;
        case DECOR_TOWER:   draw_tower(r, x, y, frame);   break;
        case DECOR_CUSHION: draw_cushion(r, x, y, frame); break;
        case DECOR_RUG2:    draw_rug2(r, x, y, frame);    break;
        case DECOR_YARN:    draw_yarn(r, x, y, frame);    break;
        case DECOR_MILK:    draw_milk(r, x, y, frame);    break;
        default: break;
    }
}

/* Rough bounding size per item, for hit-testing (w,h). */
static void item_size(DecorKind k, float *w, float *h) {
    switch (k) {
        case DECOR_PLANT:   *w = 12; *h = 20; break;
        case DECOR_LAMP:    *w = 12; *h = 19; break;
        case DECOR_PICTURE: *w = 18; *h = 14; break;
        case DECOR_TOWER:   *w = 20; *h = 25; break;
        case DECOR_CUSHION: *w = 18; *h = 11; break;
        case DECOR_RUG2:    *w = 30; *h = 14; break;
        case DECOR_YARN:    *w = 14; *h = 12; break;
        case DECOR_MILK:    *w = 16; *h = 10; break;
        default:            *w = 16; *h = 16; break;
    }
}



void decor_draw(SDL_Renderer *r, const Decor *d, Uint64 frame) {
    for (int i = 0; i < DECOR_COUNT; i++) {
        if (d->items[i].placed)
            decor_draw_one(r, (DecorKind)i, d->items[i].x, d->items[i].y, frame);
    }
}

int decor_hit(const Decor *d, float px_, float py_) {
    /* Topmost first: scan in reverse so the last-drawn item wins. */
    for (int i = DECOR_COUNT - 1; i >= 0; i--) {
        if (!d->items[i].placed) continue;
        float w, h; item_size((DecorKind)i, &w, &h);
        float x = d->items[i].x, y = d->items[i].y;
        if (px_ >= x && px_ <= x + w && py_ >= y - 4 && py_ <= y + h)
            return i;
    }
    return -1;
}

/* ---- save: "KZDC", version 1 ---- */
static const char KZ_DMAGIC[4] = { 'K', 'Z', 'D', 'C' };
#define KZ_DECOR_VERSION 1u

bool decor_save(const Decor *d, const char *path) {
    SDL_IOStream *io = SDL_IOFromFile(path, "wb");
    if (!io) return false;
    bool ok = true;
    Uint8 ver = KZ_DECOR_VERSION, count = DECOR_COUNT;
    ok = ok && SDL_WriteIO(io, KZ_DMAGIC, 4) == 4;
    ok = ok && SDL_WriteIO(io, &ver, 1) == 1;
    ok = ok && SDL_WriteIO(io, &count, 1) == 1;
    for (int i = 0; i < DECOR_COUNT && ok; i++) {
        Uint8 flags = (d->items[i].owned ? 1 : 0) | (d->items[i].placed ? 2 : 0);
        Sint16 x = (Sint16)d->items[i].x, y = (Sint16)d->items[i].y;
        ok = ok && SDL_WriteIO(io, &flags, 1) == 1;
        ok = ok && SDL_WriteIO(io, &x, 2) == 2;
        ok = ok && SDL_WriteIO(io, &y, 2) == 2;
    }
    SDL_CloseIO(io);
    return ok;
}

bool decor_load(Decor *out, const char *path) {
    SDL_IOStream *io = SDL_IOFromFile(path, "rb");
    if (!io) return false;
    bool ok = true;
    char magic[4];
    Uint8 ver = 0, count = 0;
    ok = ok && SDL_ReadIO(io, magic, 4) == 4;
    ok = ok && memcmp(magic, KZ_DMAGIC, 4) == 0;
    ok = ok && SDL_ReadIO(io, &ver, 1) == 1;
    ok = ok && ver == KZ_DECOR_VERSION;
    ok = ok && SDL_ReadIO(io, &count, 1) == 1;
    ok = ok && count == DECOR_COUNT;

    Decor tmp = decor_new();
    for (int i = 0; i < (int)count && ok; i++) {
        Uint8 flags = 0; Sint16 x = 0, y = 0;
        ok = ok && SDL_ReadIO(io, &flags, 1) == 1;
        ok = ok && SDL_ReadIO(io, &x, 2) == 2;
        ok = ok && SDL_ReadIO(io, &y, 2) == 2;
        if (ok) {
            tmp.items[i].owned  = (flags & 1) != 0;
            tmp.items[i].placed = (flags & 2) != 0;
            tmp.items[i].x = (float)x;
            tmp.items[i].y = (float)y;
        }
    }
    SDL_CloseIO(io);
    if (ok) *out = tmp;
    return ok;
}