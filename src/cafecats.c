/* cafecats.c — see cafecats.h. The adoptable cats at the cat café. */
#include "cafecats.h"
#include "render.h"
#include "palette.h"
#include "cattype.h"
#include "text.h"
#include <math.h>

static const char *CNAMES[] = {
    "Mochi", "Biscuit", "Pumpkin", "Clover", "Tofu", "Peaches",
    "Marshmallow", "Ginger", "Olive", "Pepper", "Honey", "Nutmeg",
};
#define CNAME_COUNT ((int)(sizeof CNAMES / sizeof CNAMES[0]))

/* lounging spots around the café interior (240x160) */
static const float SPOT_X[CAFE_CATS_MAX] = { 52.0f, 120.0f, 176.0f, 92.0f };
static const float SPOT_Y[CAFE_CATS_MAX] = { 120.0f, 128.0f, 118.0f, 132.0f };

static void spawn_one(CafeCat *c, int slot) {
    c->present = true;
    c->type = (CatType)SDL_rand(KZ_TYPE_COUNT);
    c->shiny = (SDL_rand(12) == 0);        /* a rare sparkly visitor */
    SDL_strlcpy(c->name, CNAMES[SDL_rand(CNAME_COUNT)], sizeof c->name);
    c->friendship = 0;
    c->adopted = false;
    c->home_x = SPOT_X[slot];
    c->home_y = SPOT_Y[slot];
    c->anim = cat_make(c->home_x, c->home_y);
    c->anim.act = (SDL_rand(2) == 0) ? ACT_SIT : ACT_GROOM;
}

CafeCats cafecats_new(void) {
    CafeCats cc;
    for (int i = 0; i < CAFE_CATS_MAX; i++) {
        /* most slots filled to start, so the café feels busy */
        if (SDL_rand(4) != 0) spawn_one(&cc.cats[i], i);
        else cc.cats[i].present = false;
    }
    cc.refresh_timer = 300.0f;
    return cc;
}

void cafecats_update(CafeCats *cc, Uint64 frame) {
    for (int i = 0; i < CAFE_CATS_MAX; i++) {
        CafeCat *c = &cc->cats[i];
        if (!c->present) continue;
        cat_update(&c->anim);
        /* gentle idle: occasionally shift pose */
        if ((frame + (Uint64)i * 37) % 360 == 0) {
            int r = SDL_rand(3);
            c->anim.act = (r == 0) ? ACT_SIT : (r == 1) ? ACT_GROOM : ACT_SLEEP;
        }
    }
    /* a new cat may wander into an empty spot now and then */
    cc->refresh_timer -= 1.0f;
    if (cc->refresh_timer <= 0.0f) {
        for (int i = 0; i < CAFE_CATS_MAX; i++) {
            if (!cc->cats[i].present) { spawn_one(&cc->cats[i], i); break; }
        }
        cc->refresh_timer = 600.0f + (float)SDL_rand(600);
    }
}

void cafecats_draw(SDL_Renderer *r, const CafeCats *cc, Uint64 frame) {
    for (int i = 0; i < CAFE_CATS_MAX; i++) {
        const CafeCat *c = &cc->cats[i];
        if (!c->present || c->adopted) continue;
        CatColors col = c->shiny ? cat_shiny_colors()
                                 : cattype_colors(c->type);
        cat_draw(r, &c->anim, col, frame);
        if (c->shiny) cat_draw_sparkles(r, &c->anim, frame);

        /* a small friendship meter floats above the cat */
        float mx = c->home_x - 10, my = c->home_y - 26;
        if (c->friendship >= CAFE_FRIEND_FULL) {
            /* ready to adopt: a bobbing heart */
            float bob = sinf((float)frame * 0.15f + i) * 1.5f;
            px_rect(r, mx + 7, my + bob, 2, 2, KZ_HEART);
            px_rect(r, mx + 10, my + bob, 2, 2, KZ_HEART);
            px_rect(r, mx + 6, my + 1 + bob, 7, 2, KZ_HEART);
            px_rect(r, mx + 7, my + 3 + bob, 5, 1, KZ_HEART);
            px_rect(r, mx + 8, my + 4 + bob, 3, 1, KZ_HEART);
        } else if (c->friendship > 0) {
            /* a little progress bar as you befriend it */
            px_rect(r, mx, my + 2, 20, 3, rgb(0xE0, 0xD6, 0xE0));
            px_rect(r, mx, my + 2,
                    20.0f * (float)c->friendship / CAFE_FRIEND_FULL, 3,
                    KZ_PETAL_PINK);
            px_rect(r, mx, my + 2, 20, 1, KZ_COCOA);
            px_rect(r, mx, my + 4, 20, 1, KZ_COCOA);
        }
    }
}

int cafecats_hit(const CafeCats *cc, float px_, float py_) {
    for (int i = 0; i < CAFE_CATS_MAX; i++) {
        const CafeCat *c = &cc->cats[i];
        if (!c->present || c->adopted) continue;
        if (px_ >= c->home_x - 14 && px_ <= c->home_x + 14
            && py_ >= c->home_y - 18 && py_ <= c->home_y + 14)
            return i;
    }
    return -1;
}

bool cafecats_pet(CafeCats *cc, int index) {
    if (index < 0 || index >= CAFE_CATS_MAX) return false;
    CafeCat *c = &cc->cats[index];
    if (!c->present || c->adopted) return false;
    cat_pet(&c->anim);
    if (c->friendship >= CAFE_FRIEND_FULL) return false;  /* already ready */
    c->friendship += 20;
    if (c->friendship >= CAFE_FRIEND_FULL) {
        c->friendship = CAFE_FRIEND_FULL;
        return true;   /* just became ready to adopt! */
    }
    return false;
}

bool cafecats_ready(const CafeCats *cc, int index) {
    if (index < 0 || index >= CAFE_CATS_MAX) return false;
    const CafeCat *c = &cc->cats[index];
    return c->present && !c->adopted && c->friendship >= CAFE_FRIEND_FULL;
}