/* friends.c — see friends.h. Meeting cats, building trust, and saving it. */
#include "friends.h"
#include <string.h>

Friends friends_new(void) {
    Friends f;
    f.count = 0;
    return f;
}

Friend *friends_find(Friends *f, const char *name) {
    for (int i = 0; i < f->count; i++) {
        if (SDL_strcmp(f->list[i].name, name) == 0) return &f->list[i];
    }
    return NULL;
}

Friend *friends_meet(Friends *f, const char *name, CatType type) {
    Friend *existing = friends_find(f, name);
    if (existing) return existing;
    if (f->count >= KZ_MAX_FRIENDS) return NULL;

    Friend *nf = &f->list[f->count];
    SDL_strlcpy(nf->name, name, KZ_FRIEND_NAME);
    nf->type = type;
    nf->trust = 0;
    nf->befriended = false;
    f->count++;
    return nf;
}

bool friends_offer_treat(Friends *f, const char *name) {
    Friend *fr = friends_find(f, name);
    if (!fr) return false;

    int t = (int)fr->trust + 20;          /* each treat is a nice step up */
    if (t > KZ_TRUST_FULL) t = KZ_TRUST_FULL;
    fr->trust = (Uint8)t;

    if (!fr->befriended && fr->trust >= KZ_TRUST_FULL) {
        fr->befriended = true;
        return true;    /* this offer is the one that made a friend */
    }
    return false;
}

int friends_befriended_count(const Friends *f) {
    int n = 0;
    for (int i = 0; i < f->count; i++) if (f->list[i].befriended) n++;
    return n;
}

/* ---- save format: "KZFR", version 1 ---- */
static const char KZ_FMAGIC[4] = { 'K', 'Z', 'F', 'R' };
#define KZ_FRIENDS_VERSION 1u

bool friends_save(const Friends *f, const char *path) {
    SDL_IOStream *io = SDL_IOFromFile(path, "wb");
    if (!io) return false;

    bool ok = true;
    Uint8 ver = KZ_FRIENDS_VERSION, count = (Uint8)f->count;
    ok = ok && SDL_WriteIO(io, KZ_FMAGIC, 4) == 4;
    ok = ok && SDL_WriteIO(io, &ver, 1) == 1;
    ok = ok && SDL_WriteIO(io, &count, 1) == 1;
    for (int i = 0; i < f->count && ok; i++) {
        const Friend *fr = &f->list[i];
        Uint8 type = (Uint8)fr->type;
        Uint8 bef  = fr->befriended ? 1 : 0;
        ok = ok && SDL_WriteIO(io, fr->name, KZ_FRIEND_NAME) == KZ_FRIEND_NAME;
        ok = ok && SDL_WriteIO(io, &type, 1) == 1;
        ok = ok && SDL_WriteIO(io, &fr->trust, 1) == 1;
        ok = ok && SDL_WriteIO(io, &bef, 1) == 1;
    }
    SDL_CloseIO(io);
    return ok;
}

bool friends_load(Friends *out, const char *path) {
    SDL_IOStream *io = SDL_IOFromFile(path, "rb");
    if (!io) return false;

    bool ok = true;
    char magic[4];
    Uint8 ver = 0, count = 0;
    ok = ok && SDL_ReadIO(io, magic, 4) == 4;
    ok = ok && memcmp(magic, KZ_FMAGIC, 4) == 0;
    ok = ok && SDL_ReadIO(io, &ver, 1) == 1;
    ok = ok && ver == KZ_FRIENDS_VERSION;
    ok = ok && SDL_ReadIO(io, &count, 1) == 1;
    ok = ok && count <= KZ_MAX_FRIENDS;

    Friends tmp;
    tmp.count = 0;
    for (int i = 0; i < (int)count && ok; i++) {
        Friend *fr = &tmp.list[i];
        Uint8 type = 0, trust = 0, bef = 0;
        ok = ok && SDL_ReadIO(io, fr->name, KZ_FRIEND_NAME) == KZ_FRIEND_NAME;
        ok = ok && SDL_ReadIO(io, &type, 1) == 1;
        ok = ok && SDL_ReadIO(io, &trust, 1) == 1;
        ok = ok && SDL_ReadIO(io, &bef, 1) == 1;
        if (ok) {
            fr->name[KZ_FRIEND_NAME - 1] = '\0';
            fr->type = (type < KZ_TYPE_COUNT) ? (CatType)type : KZ_GENTLE;
            fr->trust = trust;
            fr->befriended = (bef != 0);
            tmp.count++;
        }
    }
    SDL_CloseIO(io);
    if (ok) *out = tmp;
    return ok;
}