/* roster.c — see roster.h. The family, and its save file. */
#include "roster.h"
#include <string.h>

/* A small pool of default names, handed out as cats are adopted. */
static const char *DEFAULT_NAMES[] = {
    "Mochi", "Luna", "Pumpkin", "Clover", "Biscuit",
    "Pearl", "Maple", "Suki", "Dandy", "Poppy",
};
#define DEFAULT_NAME_COUNT ((int)(sizeof DEFAULT_NAMES / sizeof DEFAULT_NAMES[0]))

static void set_name(OwnedCat *c, const char *name) {
    SDL_strlcpy(c->name, name, KZ_NAME_LEN);
}

static OwnedCat make_cat(CatType type, const char *name,
                         float cat_x, float cat_y) {
    OwnedCat c;
    set_name(&c, name);
    c.type  = type;
    c.stats = stats_new();
    c.anim  = cat_make(cat_x, cat_y);
    /* A rare treat: 1 in 100 cats is shiny — golden, with sparkles. */
    c.shiny = (SDL_rand(100) == 0);
    return c;
}

Roster roster_new(float cat_x, float cat_y) {
    Roster ro;
    ro.count  = 0;
    ro.active = 0;
    /* Two starters of different types, so swapping means something at once. */
    ro.cats[ro.count++] = make_cat(KZ_GENTLE, "Mochi", cat_x, cat_y);
    ro.cats[ro.count++] = make_cat(KZ_CLEVER, "Luna",  cat_x, cat_y);
    return ro;
}

OwnedCat *roster_active(Roster *ro) {
    return &ro->cats[ro->active];
}

void roster_select(Roster *ro, int i) {
    if (i >= 0 && i < ro->count) ro->active = i;
}

bool roster_adopt(Roster *ro, CatType type, float cat_x, float cat_y) {
    if (ro->count >= KZ_MAX_CATS) return false;
    /* A random name so each new cat is a little surprise (not tied to how many
     * you have, which would repeat the same name as you release and re-adopt). */
    const char *name = DEFAULT_NAMES[SDL_rand(DEFAULT_NAME_COUNT)];
    ro->cats[ro->count] = make_cat(type, name, cat_x, cat_y);
    ro->active = ro->count;   /* focus the newcomer */
    ro->count++;
    return true;
}

bool roster_release(Roster *ro, int i) {
    if (i < 0 || i >= ro->count) return false;
    if (ro->count <= 1) return false;         /* never release your last cat */
    /* shift the rest down to fill the gap */
    for (int j = i; j < ro->count - 1; j++)
        ro->cats[j] = ro->cats[j + 1];
    ro->count--;
    /* keep the active index valid */
    if (ro->active >= ro->count) ro->active = ro->count - 1;
    return true;
}

void roster_rename(Roster *ro, int i, const char *name) {
    if (i < 0 || i >= ro->count) return;
    if (!name || name[0] == '\0') return;   /* never leave a cat nameless */
    set_name(&ro->cats[i], name);
}

/* Fixed lounging spots around the cottage floor, so the family spreads out
 * instead of stacking. Chosen to sit on the floor/rug, clear of the bed and
 * window. Up to KZ_MAX_CATS spots. */
void roster_home_spot(int i, float *x, float *y) {
    static const float SX[KZ_MAX_CATS] = { 140.0f, 200.0f, 100.0f, 240.0f, 180.0f, 120.0f, 220.0f };
    static const float SY[KZ_MAX_CATS] = { 185.0f, 175.0f, 190.0f, 195.0f, 205.0f, 210.0f, 188.0f };
    if (i < 0 || i >= KZ_MAX_CATS) { *x = 120.0f; *y = 132.0f; return; }
    *x = SX[i];
    *y = SY[i];
}

/* ---- save format v2 ----
 * magic "KZSV", version=2, then count, active, and for each cat:
 * name[KZ_NAME_LEN], type (1 byte), and its stats fields.
 */
static const char KZ_MAGIC[4] = { 'K', 'Z', 'S', 'V' };
#define KZ_SAVE_VERSION 4u

static bool write_stats(SDL_IOStream *io, const Stats *s) {
    bool ok = true;
    ok = ok && SDL_WriteIO(io, &s->bond, 1) == 1;
    ok = ok && SDL_WriteIO(io, &s->mood, 1) == 1;
    ok = ok && SDL_WriteIO(io, &s->energy, 1) == 1;
    ok = ok && SDL_WriteIO(io, &s->growth, 1) == 1;
    ok = ok && SDL_WriteIO(io, &s->care_given, sizeof s->care_given)
                 == sizeof s->care_given;
    ok = ok && SDL_WriteIO(io, &s->level, sizeof s->level) == sizeof s->level;
    ok = ok && SDL_WriteIO(io, &s->xp, sizeof s->xp) == sizeof s->xp;
    return ok;
}

static bool read_stats(SDL_IOStream *io, Stats *s) {
    bool ok = true;
    ok = ok && SDL_ReadIO(io, &s->bond, 1) == 1;
    ok = ok && SDL_ReadIO(io, &s->mood, 1) == 1;
    ok = ok && SDL_ReadIO(io, &s->energy, 1) == 1;
    ok = ok && SDL_ReadIO(io, &s->growth, 1) == 1;
    ok = ok && SDL_ReadIO(io, &s->care_given, sizeof s->care_given)
                 == sizeof s->care_given;
    ok = ok && SDL_ReadIO(io, &s->level, sizeof s->level) == sizeof s->level;
    ok = ok && SDL_ReadIO(io, &s->xp, sizeof s->xp) == sizeof s->xp;
    return ok;
}

bool roster_save(const Roster *ro, const char *path) {
    SDL_IOStream *io = SDL_IOFromFile(path, "wb");
    if (!io) return false;

    bool ok = true;
    Uint8 ver = KZ_SAVE_VERSION;
    Uint8 count = (Uint8)ro->count, active = (Uint8)ro->active;
    ok = ok && SDL_WriteIO(io, KZ_MAGIC, 4) == 4;
    ok = ok && SDL_WriteIO(io, &ver, 1) == 1;
    ok = ok && SDL_WriteIO(io, &count, 1) == 1;
    ok = ok && SDL_WriteIO(io, &active, 1) == 1;
    for (int i = 0; i < ro->count && ok; i++) {
        const OwnedCat *c = &ro->cats[i];
        Uint8 type = (Uint8)c->type;
        Uint8 shiny = c->shiny ? 1 : 0;
        ok = ok && SDL_WriteIO(io, c->name, KZ_NAME_LEN) == KZ_NAME_LEN;
        ok = ok && SDL_WriteIO(io, &type, 1) == 1;
        ok = ok && write_stats(io, &c->stats);
        ok = ok && SDL_WriteIO(io, &shiny, 1) == 1;
    }
    SDL_CloseIO(io);
    return ok;
}

bool roster_load(Roster *out, const char *path, float cat_x, float cat_y) {
    SDL_IOStream *io = SDL_IOFromFile(path, "rb");
    if (!io) return false;

    bool ok = true;
    char magic[4];
    Uint8 ver = 0, count = 0, active = 0;
    ok = ok && SDL_ReadIO(io, magic, 4) == 4;
    ok = ok && memcmp(magic, KZ_MAGIC, 4) == 0;
    ok = ok && SDL_ReadIO(io, &ver, 1) == 1;
    ok = ok && ver == KZ_SAVE_VERSION;
    ok = ok && SDL_ReadIO(io, &count, 1) == 1;
    ok = ok && SDL_ReadIO(io, &active, 1) == 1;
    ok = ok && count >= 1 && count <= KZ_MAX_CATS;

    Roster tmp;
    tmp.count = 0;
    tmp.active = 0;
    for (int i = 0; i < (int)count && ok; i++) {
        OwnedCat *c = &tmp.cats[i];
        Uint8 type = 0, shiny = 0;
        ok = ok && SDL_ReadIO(io, c->name, KZ_NAME_LEN) == KZ_NAME_LEN;
        ok = ok && SDL_ReadIO(io, &type, 1) == 1;
        ok = ok && read_stats(io, &c->stats);
        ok = ok && SDL_ReadIO(io, &shiny, 1) == 1;
        if (ok) {
            c->name[KZ_NAME_LEN - 1] = '\0';   /* ensure termination */
            c->type = (type < KZ_TYPE_COUNT) ? (CatType)type : KZ_GENTLE;
            c->anim = cat_make(cat_x, cat_y);
            c->shiny = (shiny != 0);
            tmp.count++;
        }
    }
    SDL_CloseIO(io);

    if (ok) {
        tmp.active = (active < tmp.count) ? active : 0;
        *out = tmp;
    }
    return ok;
}