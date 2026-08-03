/* streetlife.c — see streetlife.h. Neighbors strolling with their cats. */
#include "streetlife.h"
#include "render.h"
#include "palette.h"
#include "cat.h"
#include <math.h>

/* the sidewalk line the walkers stroll along */
#define WALK_Y   100.0f

/* a pool of owner names for the passersby */
static const char *OWNER_NAMES[] = {
    "Rosa", "Kenji", "Amara", "Theo", "Ines", "Milo", "Priya", "Sol",
};
#define OWNER_NAME_COUNT ((int)(sizeof OWNER_NAMES / sizeof OWNER_NAMES[0]))

/* pleasant shirt colors */
static const Uint8 SHIRTS[][3] = {
    { 0xE8, 0xA6, 0x8B }, { 0x9C, 0xC0, 0xD8 }, { 0xC8, 0xA8, 0xD0 },
    { 0xA8, 0xC8, 0x9C }, { 0xE0, 0xC0, 0x8B },
};
#define SHIRT_COUNT ((int)(sizeof SHIRTS / sizeof SHIRTS[0]))

static const Uint8 HAIRS[][3] = {
    { 0x5A, 0x46, 0x40 }, { 0x3C, 0x30, 0x2C }, { 0x8B, 0x6A, 0x4A },
    { 0xC0, 0x9A, 0x6A }, { 0x6A, 0x5A, 0x60 },
};
#define HAIR_COUNT ((int)(sizeof HAIRS / sizeof HAIRS[0]))

StreetLife streetlife_new(void) {
    StreetLife sl;
    for (int i = 0; i < STREETLIFE_MAX; i++)
        sl.walkers[i].active = false;
    sl.spawn_timer = 60.0f;   /* first walker arrives in ~1 second */
    return sl;
}

static void spawn_walker(Walker *w) {
    w->active = true;
    w->dir = (SDL_rand(2) == 0) ? 1 : -1;
    /* enter just off the edge, walking inward */
    w->x = (w->dir > 0) ? -14.0f : (float)KZ_W + 14.0f;
    w->y = WALK_Y;
    w->speed = 0.16f + SDL_randf() * 0.10f;   /* a relaxed, unhurried amble */
    w->cat_type = (CatType)SDL_rand(KZ_TYPE_COUNT);
    const Uint8 *sh = SHIRTS[SDL_rand(SHIRT_COUNT)];
    w->shirt_r = sh[0]; w->shirt_g = sh[1]; w->shirt_b = sh[2];
    const Uint8 *ha = HAIRS[SDL_rand(HAIR_COUNT)];
    w->hair_r = ha[0]; w->hair_g = ha[1]; w->hair_b = ha[2];
    w->name = OWNER_NAMES[SDL_rand(OWNER_NAME_COUNT)];
    w->greet_glow = 0;
}

void streetlife_update(StreetLife *sl, Uint64 frame) {
    (void)frame;
    int active_count = 0;
    for (int i = 0; i < STREETLIFE_MAX; i++) {
        Walker *w = &sl->walkers[i];
        if (!w->active) continue;
        w->x += w->speed * w->dir;
        if (w->greet_glow > 0) w->greet_glow--;
        /* retire once fully off the far edge */
        if ((w->dir > 0 && w->x > KZ_W + 16) ||
            (w->dir < 0 && w->x < -16)) {
            w->active = false;
        } else {
            active_count++;
        }
    }

    /* occasionally welcome a new walker, keeping it calm (1-2 at a time) */
    sl->spawn_timer -= 1.0f;
    if (sl->spawn_timer <= 0.0f && active_count < STREETLIFE_MAX) {
        for (int i = 0; i < STREETLIFE_MAX; i++) {
            if (!sl->walkers[i].active) { spawn_walker(&sl->walkers[i]); break; }
        }
        /* next arrival: a relaxed gap, longer if the street's already busy */
        sl->spawn_timer = 240.0f + (float)SDL_rand(360)
                        + (active_count > 0 ? 300.0f : 0.0f);
    }
}

/* a small pixel person: head, hair, body, legs mid-stride */
static void draw_person(SDL_Renderer *r, const Walker *w, Uint64 frame) {
    float x = w->x, y = w->y;
    int step = ((frame / 10) % 2 == 0) ? 1 : 0;   /* simple walk cycle */
    Color skin = rgb(0xF0, 0xD2, 0xB8);
    Color hair = rgb(w->hair_r, w->hair_g, w->hair_b);
    Color shirt = rgb(w->shirt_r, w->shirt_g, w->shirt_b);
    Color pants = rgb(0x7A, 0x8A, 0x9C);

    /* legs (mid-stride) */
    px_rect(r, x - 2, y - 4 + step, 2, 4 - step, pants);
    px_rect(r, x + 1, y - 4 + (1 - step), 2, 4 - (1 - step), pants);
    /* body */
    px_rect(r, x - 3, y - 11, 6, 7, shirt);
    /* head + hair */
    px_rect(r, x - 2, y - 17, 5, 5, skin);
    px_rect(r, x - 2, y - 18, 5, 2, hair);
    px_rect(r, x - 3, y - 17, 1, 3, hair);
    px_rect(r, x + 3, y - 17, 1, 3, hair);
    /* a happy little glow when just greeted */
    if (w->greet_glow > 0 && (w->greet_glow / 4) % 2 == 0) {
        px_rect(r, x - 1, y - 21, 1, 2, KZ_HEART);
        px_rect(r, x + 2, y - 21, 1, 2, KZ_HEART);
    }
}

void streetlife_draw(SDL_Renderer *r, const StreetLife *sl, Uint64 frame,
                     bool night) {
    for (int i = 0; i < STREETLIFE_MAX; i++) {
        const Walker *w = &sl->walkers[i];
        if (!w->active) continue;

        draw_person(r, w, frame);

        /* their cat pads along a little behind, at their feet */
        Cat pet;
        pet = cat_make(w->x - (float)w->dir * 12.0f, w->y + 2.0f);
        pet.facing = w->dir;
        pet.act = ACT_WALK;
        cat_draw(r, &pet, cattype_colors(w->cat_type), frame);
    }

    if (night) {
        /* handled by the street's own night dim; nothing extra here */
        (void)night;
    }
}

int streetlife_hit(const StreetLife *sl, float px_, float py_) {
    for (int i = 0; i < STREETLIFE_MAX; i++) {
        const Walker *w = &sl->walkers[i];
        if (!w->active) continue;
        /* a generous box around the person and their cat */
        float lo = w->x - 18.0f, hi = w->x + 6.0f;
        if (w->dir < 0) { lo = w->x - 6.0f; hi = w->x + 18.0f; }
        if (px_ >= lo && px_ <= hi && py_ >= w->y - 22.0f && py_ <= w->y + 12.0f)
            return i;
    }
    return -1;
}

const char *streetlife_greet(StreetLife *sl, int index, CatType *out_type) {
    if (index < 0 || index >= STREETLIFE_MAX) return NULL;
    Walker *w = &sl->walkers[index];
    if (!w->active) return NULL;
    w->greet_glow = 40;
    if (out_type) *out_type = w->cat_type;
    return w->name;
}