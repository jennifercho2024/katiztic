/* tricks.c — see tricks.h. Learning tricks through practice. */
#include "tricks.h"

static const char *NAMES[TRICK_COUNT] = {
    "Sit", "Spin", "Jump", "High-five", "Roll"
};
const char *trick_name(TrickId t) {
    if (t < 0 || t >= TRICK_COUNT) return "?";
    return NAMES[t];
}

Tricks tricks_new(void) {
    Tricks tr;
    for (int i = 0; i < TRICKS_MAX_CATS; i++) {
        tr.cats[i].used = false;
        tr.cats[i].name[0] = '\0';
        for (int t = 0; t < TRICK_COUNT; t++) tr.cats[i].skill[t] = 0;
    }
    return tr;
}

static CatTricks *find(Tricks *tr, const char *cat) {
    for (int i = 0; i < TRICKS_MAX_CATS; i++)
        if (tr->cats[i].used && SDL_strcmp(tr->cats[i].name, cat) == 0)
            return &tr->cats[i];
    return NULL;
}

static CatTricks *find_or_add(Tricks *tr, const char *cat) {
    CatTricks *ct = find(tr, cat);
    if (ct) return ct;
    for (int i = 0; i < TRICKS_MAX_CATS; i++) {
        if (!tr->cats[i].used) {
            tr->cats[i].used = true;
            SDL_strlcpy(tr->cats[i].name, cat, TRICK_NAME_LEN);
            for (int t = 0; t < TRICK_COUNT; t++) tr->cats[i].skill[t] = 0;
            return &tr->cats[i];
        }
    }
    return NULL;   /* full (shouldn't happen: TRICKS_MAX_CATS == roster max) */
}

int tricks_skill(const Tricks *tr, const char *cat, TrickId t) {
    if (t < 0 || t >= TRICK_COUNT) return 0;
    for (int i = 0; i < TRICKS_MAX_CATS; i++)
        if (tr->cats[i].used && SDL_strcmp(tr->cats[i].name, cat) == 0)
            return tr->cats[i].skill[t];
    return 0;
}

bool tricks_mastered(const Tricks *tr, const char *cat, TrickId t) {
    return tricks_skill(tr, cat, t) >= TRICK_MASTER;
}

int tricks_mastered_count(const Tricks *tr, const char *cat) {
    int n = 0;
    for (int t = 0; t < TRICK_COUNT; t++)
        if (tricks_skill(tr, cat, (TrickId)t) >= TRICK_MASTER) n++;
    return n;
}

int tricks_practice(Tricks *tr, const char *cat, TrickId t, bool have_treat) {
    if (t < 0 || t >= TRICK_COUNT) return 0;
    CatTricks *ct = find_or_add(tr, cat);
    if (!ct) return 0;
    if (ct->skill[t] >= TRICK_MASTER) return 0;   /* already mastered */

    int step = have_treat ? 20 : 12;   /* a treat helps them learn faster */
    int s = (int)ct->skill[t] + step;
    if (s > TRICK_MASTER) s = TRICK_MASTER;
    ct->skill[t] = (Uint8)s;

    return (ct->skill[t] >= TRICK_MASTER) ? 1 : 0;
}

/* ---- persistence ---- */

static const char MAGIC[4] = { 'K', 'Z', 'T', 'R' };
#define TRICKS_SAVE_VERSION 1u

bool tricks_save(const Tricks *tr, const char *path) {
    SDL_IOStream *io = SDL_IOFromFile(path, "wb");
    if (!io) return false;
    bool ok = SDL_WriteIO(io, MAGIC, 4) == 4;
    Uint8 ver = TRICKS_SAVE_VERSION;
    Uint8 tcount = (Uint8)TRICK_COUNT;
    /* count used cats */
    Uint8 used = 0;
    for (int i = 0; i < TRICKS_MAX_CATS; i++) if (tr->cats[i].used) used++;
    ok = ok && SDL_WriteIO(io, &ver, 1) == 1;
    ok = ok && SDL_WriteIO(io, &tcount, 1) == 1;
    ok = ok && SDL_WriteIO(io, &used, 1) == 1;
    for (int i = 0; i < TRICKS_MAX_CATS && ok; i++) {
        if (!tr->cats[i].used) continue;
        ok = ok && SDL_WriteIO(io, tr->cats[i].name, TRICK_NAME_LEN)
                     == TRICK_NAME_LEN;
        ok = ok && SDL_WriteIO(io, tr->cats[i].skill, TRICK_COUNT)
                     == TRICK_COUNT;
    }
    SDL_CloseIO(io);
    return ok;
}

bool tricks_load(Tricks *tr, const char *path) {
    SDL_IOStream *io = SDL_IOFromFile(path, "rb");
    if (!io) return false;
    char magic[4];
    Uint8 ver = 0, tcount = 0, used = 0;
    bool ok = SDL_ReadIO(io, magic, 4) == 4;
    ok = ok && SDL_memcmp(magic, MAGIC, 4) == 0;
    ok = ok && SDL_ReadIO(io, &ver, 1) == 1 && ver == TRICKS_SAVE_VERSION;
    ok = ok && SDL_ReadIO(io, &tcount, 1) == 1 && tcount == TRICK_COUNT;
    ok = ok && SDL_ReadIO(io, &used, 1) == 1 && used <= TRICKS_MAX_CATS;
    Tricks tmp = tricks_new();
    for (int i = 0; i < (int)used && ok; i++) {
        Uint8 skill[TRICK_COUNT];
        char name[TRICK_NAME_LEN];
        ok = ok && SDL_ReadIO(io, name, TRICK_NAME_LEN) == TRICK_NAME_LEN;
        ok = ok && SDL_ReadIO(io, skill, TRICK_COUNT) == TRICK_COUNT;
        if (ok) {
            name[TRICK_NAME_LEN - 1] = '\0';
            tmp.cats[i].used = true;
            SDL_strlcpy(tmp.cats[i].name, name, TRICK_NAME_LEN);
            for (int t = 0; t < TRICK_COUNT; t++) tmp.cats[i].skill[t] = skill[t];
        }
    }
    SDL_CloseIO(io);
    if (ok) *tr = tmp;
    return ok;
}