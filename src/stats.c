/* stats.c — see stats.h. Care logic and a tiny versioned save file. */
#include "stats.h"
#include <string.h>

/* Clamp helper: add `d` to a 0..100 stat without overflowing past the max. */
static Uint8 bump(Uint8 v, int d) {
    int n = (int)v + d;
    if (n > KZ_STAT_MAX) n = KZ_STAT_MAX;
    if (n < 0) n = 0;
    return (Uint8)n;
}

/* Every care action also feeds long-term growth: one point per few actions. */
static void grow(Stats *s) {
    s->care_given++;
    /* Growth rises slowly — every 4th act of care is one growth point. */
    if (s->care_given % 4 == 0 && s->growth < KZ_STAT_MAX) {
        s->growth++;
    }
}

Stats stats_new(void) {
    /* She starts content: mid-high mood and energy, a little bond already,
     * and at level 1 with no XP yet. */
    Stats s;
    s.bond   = 10;
    s.mood   = 70;
    s.energy = 80;
    s.growth = 0;
    s.care_given = 0;
    s.level = 1;
    s.xp    = 0;
    return s;
}

/* XP to go from `level` to the next. Starts modest and grows ~30% per level,
 * so early levels come quickly and later ones take longer — endless, but with
 * a gentle slowdown. */
Uint16 stats_xp_for_level(Uint16 level) {
    /* level 1->2: 20, then *1.3 each time, capped so it never overflows. */
    float need = 20.0f;
    for (Uint16 i = 1; i < level; i++) need *= 1.3f;
    if (need > 60000.0f) need = 60000.0f;
    return (Uint16)need;
}

int stats_gain_xp(Stats *s, int amount) {
    if (amount <= 0) return 0;
    int levels = 0;
    int xp = (int)s->xp + amount;
    /* roll over as many levels as the XP covers */
    while (xp >= (int)stats_xp_for_level(s->level)) {
        xp -= (int)stats_xp_for_level(s->level);
        if (s->level < 65000) s->level++;   /* effectively no cap */
        levels++;
    }
    s->xp = (Uint16)xp;
    return levels;
}

void stats_outing(Stats *s) {
    /* Taking her out lifts happiness — a breath of fresh air. */
    s->mood = bump(s->mood, 6);
}

void stats_feed(Stats *s) {
    s->energy = bump(s->energy, 15);
    s->mood   = bump(s->mood, 5);
    grow(s);
    stats_gain_xp(s, 4);
}

void stats_groom(Stats *s) {
    s->mood = bump(s->mood, 12);
    s->bond = bump(s->bond, 6);
    grow(s);
    stats_gain_xp(s, 5);
}

void stats_pet(Stats *s) {
    s->bond = bump(s->bond, 3);
    s->mood = bump(s->mood, 2);
    grow(s);
    stats_gain_xp(s, 3);
}

/* Waking from a good sleep: a gentle mood lift. Energy is set to full by the
 * caller; this is the little "well-rested and happy" bump on top. Does not
 * count as care (no growth) — it's just the morning feeling good. */
void stats_wake(Stats *s) {
    s->mood = bump(s->mood, 8);
}

/* ---- save format ----
 * A 4-byte magic + 1-byte version, then the fields written explicitly (not a
 * raw struct dump — that would break across compilers/padding). Small, sturdy,
 * and easy to extend: bump the version and append fields.
 */
static const char KZ_MAGIC[4] = { 'K', 'Z', 'S', 'V' };
#define KZ_SAVE_VERSION 1u

bool stats_save(const Stats *s, const char *path) {
    SDL_IOStream *io = SDL_IOFromFile(path, "wb");
    if (!io) return false;

    bool ok = true;
    Uint8 ver = KZ_SAVE_VERSION;
    ok = ok && SDL_WriteIO(io, KZ_MAGIC, 4) == 4;
    ok = ok && SDL_WriteIO(io, &ver, 1) == 1;
    ok = ok && SDL_WriteIO(io, &s->bond, 1) == 1;
    ok = ok && SDL_WriteIO(io, &s->mood, 1) == 1;
    ok = ok && SDL_WriteIO(io, &s->energy, 1) == 1;
    ok = ok && SDL_WriteIO(io, &s->growth, 1) == 1;
    ok = ok && SDL_WriteIO(io, &s->care_given, sizeof s->care_given)
                 == sizeof s->care_given;

    SDL_CloseIO(io);
    return ok;
}

bool stats_load(Stats *out, const char *path) {
    SDL_IOStream *io = SDL_IOFromFile(path, "rb");
    if (!io) return false;  /* no save yet — caller uses stats_new() */

    bool ok = true;
    char magic[4];
    Uint8 ver = 0;
    ok = ok && SDL_ReadIO(io, magic, 4) == 4;
    ok = ok && memcmp(magic, KZ_MAGIC, 4) == 0;
    ok = ok && SDL_ReadIO(io, &ver, 1) == 1;
    ok = ok && ver == KZ_SAVE_VERSION;

    Stats tmp = stats_new();
    ok = ok && SDL_ReadIO(io, &tmp.bond, 1) == 1;
    ok = ok && SDL_ReadIO(io, &tmp.mood, 1) == 1;
    ok = ok && SDL_ReadIO(io, &tmp.energy, 1) == 1;
    ok = ok && SDL_ReadIO(io, &tmp.growth, 1) == 1;
    ok = ok && SDL_ReadIO(io, &tmp.care_given, sizeof tmp.care_given)
                 == sizeof tmp.care_given;

    SDL_CloseIO(io);
    if (ok) *out = tmp;      /* only overwrite on a fully valid read */
    return ok;
}