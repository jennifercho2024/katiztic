/* owners.c — see owners.h. Befriending the neighbors and their invitations. */
#include "owners.h"

Owners owners_new(void) {
    Owners o;
    o.count = 0;
    return o;
}

Owner *owners_find(Owners *o, const char *name) {
    for (int i = 0; i < o->count; i++)
        if (SDL_strcmp(o->list[i].name, name) == 0)
            return &o->list[i];
    return NULL;
}

int owners_greet(Owners *o, const char *name, CatType cat_type) {
    Owner *ow = owners_find(o, name);
    if (!ow) {
        if (o->count >= KZ_MAX_OWNERS) return 0;   /* address book full */
        ow = &o->list[o->count++];
        SDL_strlcpy(ow->name, name, KZ_OWNER_NAME);
        ow->cat_type = cat_type;
        ow->greets = 0;
        ow->befriended = false;
        ow->invite_pending = false;
        ow->invite_read = false;
    }

    if (ow->greets < 255) ow->greets++;

    if (!ow->befriended && ow->greets >= OWNER_GREETS_TO_FRIEND) {
        ow->befriended = true;
        ow->invite_pending = true;   /* a friend sends a playdate letter */
        ow->invite_read = false;
        return 1;                    /* just became friends */
    }
    return 0;
}

int owners_friend_count(const Owners *o) {
    int n = 0;
    for (int i = 0; i < o->count; i++)
        if (o->list[i].befriended) n++;
    return n;
}

int owners_invite_count(const Owners *o) {
    int n = 0;
    for (int i = 0; i < o->count; i++)
        if (o->list[i].invite_pending) n++;
    return n;
}

void owners_clear_invite(Owners *o, const char *name) {
    Owner *ow = owners_find(o, name);
    if (ow) {
        ow->invite_pending = false;
        ow->invite_read = true;
    }
}

/* ---- persistence ---- */

static const char MAGIC[4] = { 'K', 'Z', 'O', 'W' };
#define OWNERS_SAVE_VERSION 1u

bool owners_save(const Owners *o, const char *path) {
    SDL_IOStream *io = SDL_IOFromFile(path, "wb");
    if (!io) return false;
    bool ok = SDL_WriteIO(io, MAGIC, 4) == 4;
    Uint8 ver = OWNERS_SAVE_VERSION;
    Uint8 count = (Uint8)o->count;
    ok = ok && SDL_WriteIO(io, &ver, 1) == 1;
    ok = ok && SDL_WriteIO(io, &count, 1) == 1;
    for (int i = 0; i < o->count && ok; i++) {
        const Owner *ow = &o->list[i];
        Uint8 type = (Uint8)ow->cat_type;
        Uint8 flags = (ow->befriended ? 1 : 0)
                    | (ow->invite_pending ? 2 : 0)
                    | (ow->invite_read ? 4 : 0);
        ok = ok && SDL_WriteIO(io, ow->name, KZ_OWNER_NAME) == KZ_OWNER_NAME;
        ok = ok && SDL_WriteIO(io, &type, 1) == 1;
        ok = ok && SDL_WriteIO(io, &ow->greets, 1) == 1;
        ok = ok && SDL_WriteIO(io, &flags, 1) == 1;
    }
    SDL_CloseIO(io);
    return ok;
}

bool owners_load(Owners *o, const char *path) {
    SDL_IOStream *io = SDL_IOFromFile(path, "rb");
    if (!io) return false;
    char magic[4];
    Uint8 ver = 0, count = 0;
    bool ok = SDL_ReadIO(io, magic, 4) == 4;
    ok = ok && SDL_memcmp(magic, MAGIC, 4) == 0;
    ok = ok && SDL_ReadIO(io, &ver, 1) == 1 && ver == OWNERS_SAVE_VERSION;
    ok = ok && SDL_ReadIO(io, &count, 1) == 1 && count <= KZ_MAX_OWNERS;
    Owners tmp = owners_new();
    for (int i = 0; i < (int)count && ok; i++) {
        Owner *ow = &tmp.list[i];
        Uint8 type = 0, greets = 0, flags = 0;
        ok = ok && SDL_ReadIO(io, ow->name, KZ_OWNER_NAME) == KZ_OWNER_NAME;
        ok = ok && SDL_ReadIO(io, &type, 1) == 1;
        ok = ok && SDL_ReadIO(io, &greets, 1) == 1;
        ok = ok && SDL_ReadIO(io, &flags, 1) == 1;
        if (ok) {
            ow->name[KZ_OWNER_NAME - 1] = '\0';
            ow->cat_type = (CatType)(type % KZ_TYPE_COUNT);
            ow->greets = greets;
            ow->befriended = (flags & 1) != 0;
            ow->invite_pending = (flags & 2) != 0;
            ow->invite_read = (flags & 4) != 0;
        }
    }
    tmp.count = (int)count;
    SDL_CloseIO(io);
    if (ok) *o = tmp;
    return ok;
}