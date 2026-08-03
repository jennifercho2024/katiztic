/* forestlife.c — see forestlife.h. Rabbits, birds, deer, and a shy bear. */
#include "forestlife.h"
#include "render.h"
#include "palette.h"
#include <math.h>

static const char *NAMES[ANIMAL_KIND_COUNT] = {
    "rabbit", "bird", "deer", "bear"
};
const char *animal_name(AnimalKind k) {
    if (k < 0 || k >= ANIMAL_KIND_COUNT) return "creature";
    return NAMES[k];
}

/* how much trust each interaction gives, and what it needs */
static int trust_step(AnimalKind k, bool have_treat) {
    switch (k) {
        case ANIMAL_RABBIT: return 20;                 /* gentle visits    */
        case ANIMAL_BIRD:   return 16;                 /* patience         */
        case ANIMAL_DEER:   return have_treat ? 34 : 0;/* wants a treat    */
        case ANIMAL_BEAR:   return 12;                 /* slow and shy     */
        default:            return 10;
    }
}

ForestLife forestlife_new(void) {
    ForestLife fl;
    for (int i = 0; i < FORESTLIFE_MAX; i++) fl.animals[i].active = false;
    fl.spawn_timer = 40.0f;
    for (int k = 0; k < ANIMAL_KIND_COUNT; k++) {
        fl.friends.trust[k] = 0;
        fl.friends.befriended[k] = false;
    }
    return fl;
}

static void spawn_animal(Animal *a) {
    a->active = true;
    a->kind = (AnimalKind)SDL_rand(ANIMAL_KIND_COUNT);
    a->dir = (SDL_rand(2) == 0) ? 1 : -1;
    a->x = (a->dir > 0) ? -12.0f : (float)KZ_W + 12.0f;
    /* birds fly higher; ground animals walk the forest floor */
    a->y = (a->kind == ANIMAL_BIRD) ? (52.0f + SDL_randf() * 16.0f)
                                    : (120.0f + SDL_randf() * 20.0f);
    a->speed = (a->kind == ANIMAL_BIRD) ? 0.30f + SDL_randf() * 0.18f
             : (a->kind == ANIMAL_BEAR) ? 0.10f + SDL_randf() * 0.05f
             : 0.16f + SDL_randf() * 0.12f;
    a->wander_timer = 120.0f + SDL_rand(180);
    a->glow = 0;
    a->hop = SDL_rand(60);
}

void forestlife_update(ForestLife *fl, Uint64 frame) {
    (void)frame;
    int count = 0;
    for (int i = 0; i < FORESTLIFE_MAX; i++) {
        Animal *a = &fl->animals[i];
        if (!a->active) continue;
        a->x += a->speed * a->dir;
        a->hop++;
        if (a->glow > 0) a->glow--;
        /* occasionally pause/turn (except birds, which keep gliding) */
        if (a->kind != ANIMAL_BIRD) {
            a->wander_timer -= 1.0f;
            if (a->wander_timer <= 0.0f) {
                if (SDL_rand(3) == 0) a->dir = -a->dir;  /* sometimes turn */
                a->wander_timer = 120.0f + SDL_rand(180);
            }
        }
        if ((a->dir > 0 && a->x > KZ_W + 14) ||
            (a->dir < 0 && a->x < -14)) {
            a->active = false;
        } else {
            count++;
        }
    }

    fl->spawn_timer -= 1.0f;
    if (fl->spawn_timer <= 0.0f && count < FORESTLIFE_MAX) {
        for (int i = 0; i < FORESTLIFE_MAX; i++)
            if (!fl->animals[i].active) { spawn_animal(&fl->animals[i]); break; }
        /* keep it calm: a relaxed gap, longer if one's already here */
        fl->spawn_timer = 180.0f + (float)SDL_rand(240)
                        + (count > 0 ? 240.0f : 0.0f);
    }
}

/* ---- drawing each animal ---- */

static void draw_rabbit(SDL_Renderer *r, const Animal *a, Uint64 frame) {
    (void)frame;
    float x = a->x, y = a->y;
    float hop = fabsf(sinf((float)a->hop * 0.2f)) * 3.0f;
    Color body = rgb(0xE8, 0xDE, 0xE0), ear = rgb(0xF0, 0xD0, 0xD6);
    y -= hop;
    px_rect(r, x, y, 8, 6, body);                 /* body   */
    px_rect(r, x + (a->dir > 0 ? 7 : -1), y - 1, 3, 4, body); /* head */
    px_rect(r, x + (a->dir > 0 ? 7 : 0), y - 5, 1, 4, ear);   /* ears */
    px_rect(r, x + (a->dir > 0 ? 9 : 2), y - 5, 1, 4, ear);
    px_rect(r, x - (a->dir > 0 ? 1 : -8), y + 3, 2, 2, KZ_CLOUD); /* tail */
}

static void draw_bird(SDL_Renderer *r, const Animal *a, Uint64 frame) {
    (void)frame;
    float x = a->x, y = a->y;
    float flap = sinf((float)a->hop * 0.4f) * 2.0f;
    Color body = rgb(0x9C, 0xC0, 0xD8), wing = rgb(0x7C, 0xA6, 0xC4);
    px_rect(r, x + 2, y, 5, 4, body);             /* body */
    px_rect(r, x + (a->dir > 0 ? 6 : 1), y - 1, 2, 2, body); /* head */
    px_rect(r, x + 1, y - flap, 4, 2, wing);      /* wing (flapping) */
    px_rect(r, x + (a->dir > 0 ? 8 : 0), y, 1, 1, KZ_BUTTER); /* beak */
}

static void draw_deer(SDL_Renderer *r, const Animal *a, Uint64 frame) {
    (void)frame;
    float x = a->x, y = a->y;
    Color body = rgb(0xC8, 0xA6, 0x88), dark = rgb(0xAC, 0x8A, 0x6E);
    px_rect(r, x, y, 14, 8, body);                /* body */
    px_rect(r, x, y + 8, 2, 4, dark);             /* legs */
    px_rect(r, x + 5, y + 8, 2, 4, dark);
    px_rect(r, x + 11, y + 8, 2, 4, dark);
    float hx = x + (a->dir > 0 ? 12 : -4);
    px_rect(r, hx, y - 4, 4, 6, body);            /* head+neck */
    px_rect(r, hx + (a->dir > 0 ? 0 : 2), y - 7, 1, 4, dark); /* antlers */
    px_rect(r, hx + (a->dir > 0 ? 2 : 0), y - 7, 1, 4, dark);
    px_rect(r, x + 2, y + 1, 2, 2, KZ_CLOUD);     /* soft spots */
    px_rect(r, x + 8, y + 3, 2, 2, KZ_CLOUD);
}

static void draw_bear(SDL_Renderer *r, const Animal *a, Uint64 frame) {
    (void)frame;
    float x = a->x, y = a->y;
    Color body = rgb(0xA6, 0x8A, 0x82), dark = rgb(0x8A, 0x70, 0x6A);
    px_rect(r, x, y - 2, 18, 12, body);           /* big body */
    px_rect(r, x, y + 10, 3, 4, dark);            /* legs */
    px_rect(r, x + 14, y + 10, 3, 4, dark);
    float hx = x + (a->dir > 0 ? 15 : -5);
    px_rect(r, hx, y - 4, 6, 6, body);            /* head */
    px_rect(r, hx, y - 6, 2, 2, dark);            /* ears */
    px_rect(r, hx + 4, y - 6, 2, 2, dark);
    px_rect(r, hx + (a->dir > 0 ? 4 : 1), y - 1, 1, 1, KZ_CAT_OUTLINE); /* nose */
}

void forestlife_draw(SDL_Renderer *r, const ForestLife *fl, Uint64 frame) {
    for (int i = 0; i < FORESTLIFE_MAX; i++) {
        const Animal *a = &fl->animals[i];
        if (!a->active) continue;
        switch (a->kind) {
            case ANIMAL_RABBIT: draw_rabbit(r, a, frame); break;
            case ANIMAL_BIRD:   draw_bird(r, a, frame);   break;
            case ANIMAL_DEER:   draw_deer(r, a, frame);   break;
            case ANIMAL_BEAR:   draw_bear(r, a, frame);   break;
            default: break;
        }
        /* a happy glow (little hearts) just after interacting */
        if (a->glow > 0 && (a->glow / 4) % 2 == 0) {
            px_rect(r, a->x + 4, a->y - 8, 2, 2, KZ_HEART);
            px_rect(r, a->x + 8, a->y - 10, 1, 1, KZ_PETAL_PINK);
        }
    }
}

int forestlife_hit(const ForestLife *fl, float px_, float py_) {
    for (int i = 0; i < FORESTLIFE_MAX; i++) {
        const Animal *a = &fl->animals[i];
        if (!a->active) continue;
        /* a generous box scaled loosely to the animal's size */
        float w = (a->kind == ANIMAL_BEAR) ? 22.0f
                : (a->kind == ANIMAL_DEER) ? 20.0f
                : (a->kind == ANIMAL_BIRD) ? 12.0f : 12.0f;
        if (px_ >= a->x - 4 && px_ <= a->x + w
            && py_ >= a->y - 10 && py_ <= a->y + 14)
            return i;
    }
    return -1;
}

int forestlife_interact(ForestLife *fl, int index, bool have_treat,
                        char *msg, size_t msglen) {
    if (index < 0 || index >= FORESTLIFE_MAX) return -1;
    Animal *a = &fl->animals[index];
    if (!a->active) return -1;
    AnimalKind k = a->kind;

    int step = trust_step(k, have_treat);
    if (step <= 0) {
        /* deer with no treat: it just watches shyly */
        SDL_snprintf(msg, msglen, "The %s eyes your treat pouch...",
                     animal_name(k));
        return -1;
    }

    a->glow = 40;
    if (fl->friends.befriended[k]) {
        SDL_snprintf(msg, msglen, "Your friend the %s is happy to see you!",
                     animal_name(k));
        return 0;
    }

    int t = (int)fl->friends.trust[k] + step;
    if (t > ANIMAL_TRUST_FULL) t = ANIMAL_TRUST_FULL;
    fl->friends.trust[k] = (Uint8)t;

    if (fl->friends.trust[k] >= ANIMAL_TRUST_FULL) {
        fl->friends.befriended[k] = true;
        SDL_snprintf(msg, msglen, "You befriended the %s!", animal_name(k));
        return 1;
    }

    /* a per-animal flavor line for progress */
    switch (k) {
        case ANIMAL_RABBIT:
            SDL_snprintf(msg, msglen, "The rabbit hops a little closer.");
            break;
        case ANIMAL_BIRD:
            SDL_snprintf(msg, msglen, "The bird chirps and stays a while.");
            break;
        case ANIMAL_DEER:
            SDL_snprintf(msg, msglen, "The deer takes the treat gently.");
            break;
        case ANIMAL_BEAR:
        default:
            SDL_snprintf(msg, msglen, "The bear gives a slow, shy sniff.");
            break;
    }
    return 0;
}

int forestfriends_count(const ForestFriends *ff) {
    int n = 0;
    for (int k = 0; k < ANIMAL_KIND_COUNT; k++)
        if (ff->befriended[k]) n++;
    return n;
}

/* ---- persistence ---- */

static const char MAGIC[4] = { 'K', 'Z', 'F', 'A' };
#define FF_SAVE_VERSION 1u

bool forestfriends_save(const ForestFriends *ff, const char *path) {
    SDL_IOStream *io = SDL_IOFromFile(path, "wb");
    if (!io) return false;
    bool ok = SDL_WriteIO(io, MAGIC, 4) == 4;
    Uint8 ver = FF_SAVE_VERSION, count = (Uint8)ANIMAL_KIND_COUNT;
    ok = ok && SDL_WriteIO(io, &ver, 1) == 1;
    ok = ok && SDL_WriteIO(io, &count, 1) == 1;
    for (int k = 0; k < ANIMAL_KIND_COUNT && ok; k++) {
        Uint8 b = ff->befriended[k] ? 1 : 0;
        ok = ok && SDL_WriteIO(io, &ff->trust[k], 1) == 1;
        ok = ok && SDL_WriteIO(io, &b, 1) == 1;
    }
    SDL_CloseIO(io);
    return ok;
}

bool forestfriends_load(ForestFriends *ff, const char *path) {
    SDL_IOStream *io = SDL_IOFromFile(path, "rb");
    if (!io) return false;
    char magic[4];
    Uint8 ver = 0, count = 0;
    bool ok = SDL_ReadIO(io, magic, 4) == 4;
    ok = ok && SDL_memcmp(magic, MAGIC, 4) == 0;
    ok = ok && SDL_ReadIO(io, &ver, 1) == 1 && ver == FF_SAVE_VERSION;
    ok = ok && SDL_ReadIO(io, &count, 1) == 1
            && count >= 1 && count <= ANIMAL_KIND_COUNT;
    ForestFriends tmp;
    for (int k = 0; k < ANIMAL_KIND_COUNT; k++) {
        tmp.trust[k] = 0; tmp.befriended[k] = false;
    }
    for (int k = 0; k < (int)count && ok; k++) {
        Uint8 tr = 0, b = 0;
        ok = ok && SDL_ReadIO(io, &tr, 1) == 1;
        ok = ok && SDL_ReadIO(io, &b, 1) == 1;
        if (ok) { tmp.trust[k] = tr; tmp.befriended[k] = (b != 0); }
    }
    SDL_CloseIO(io);
    if (ok) *ff = tmp;
    return ok;
}