/* cafecats.c — see cafecats.h. The adoptable cats at the cat café. */
#include "cafecats.h"
#include "render.h"
#include "palette.h"
#include "cattype.h"
#include "text.h"
#include <math.h>

/* lounging spots around the café interior (240x160) */
static const float SPOT_X[CAFE_CATS_MAX] = { 52.0f, 120.0f, 176.0f, 92.0f };
static const float SPOT_Y[CAFE_CATS_MAX] = { 120.0f, 128.0f, 118.0f, 132.0f };

/* The café's resident cats: the same familiar faces greet you every visit.
 * Fixed name, type, and spot so they feel like they truly live here. */
static const char    *RES_NAME[CAFE_CATS_MAX] = { "Mochi", "Biscuit", "Clover", "Tofu" };
static const CatType  RES_TYPE[CAFE_CATS_MAX] = { KZ_SUNNY, KZ_GENTLE, KZ_PLAYFUL, KZ_CLEVER };

static void spawn_one(CafeCat *c, int slot) {
    c->present = true;
    c->type = RES_TYPE[slot];
    c->shiny = false;
    SDL_strlcpy(c->name, RES_NAME[slot], sizeof c->name);
    c->friendship = 0;
    c->adopted = false;
    c->home_x = SPOT_X[slot];
    c->home_y = SPOT_Y[slot];
    c->anim = cat_make(c->home_x, c->home_y);
    c->anim.act = (SDL_rand(2) == 0) ? ACT_SIT : ACT_GROOM;
}

CafeCats cafecats_new(void) {
    CafeCats cc;
    /* all four residents are always here — the café's regulars */
    for (int i = 0; i < CAFE_CATS_MAX; i++)
        spawn_one(&cc.cats[i], i);
    cc.refresh_timer = 300.0f;
    /* a different little crowd of patrons each visit */
    cc.patron_count = 1 + SDL_rand(3);   /* 1..3 people */
    for (int i = 0; i < cc.patron_count; i++) {
        cc.patron_x[i] = 30.0f + i * 70.0f + (float)SDL_rand(20);
        cc.patron_shirt[i] = SDL_rand(5);
        cc.patron_has_cat[i] = SDL_rand(2);
    }
    return cc;
}

/* draw one seated café patron: a simple, friendly figure at a table */
static void draw_patron(SDL_Renderer *r, float x, int shirt, int has_cat,
                        Uint64 frame) {
    Color shirts[5] = { KZ_PETAL_PINK, KZ_MINT, KZ_BUTTER, KZ_LAVENDER,
                        rgb(0xB8, 0xC8, 0xE0) };
    Color skin[3] = { rgb(0xF0, 0xC8, 0xA8), rgb(0xC8, 0x94, 0x6E),
                      rgb(0x9A, 0x6E, 0x50) };
    Color sk = skin[(int)((x + shirt) ) % 3];
    float y = 96;
    /* chair back */
    px_rect(r, x - 8, y - 2, 2, 20, rgb(0xB0, 0x8E, 0x76));
    /* body/shirt */
    px_rect(r, x - 6, y + 4, 12, 12, shirts[shirt % 5]);
    /* head */
    px_rect(r, x - 4, y - 5, 8, 8, sk);
    /* hair */
    px_rect(r, x - 4, y - 6, 8, 3, rgb(0x6A, 0x52, 0x44));
    px_rect(r, x - 5, y - 5, 1, 4, rgb(0x6A, 0x52, 0x44));
    px_rect(r, x + 4, y - 5, 1, 4, rgb(0x6A, 0x52, 0x44));
    /* a coffee cup on the table, steam curling */
    px_rect(r, x + 8, y + 12, 4, 3, KZ_CLOUD);
    px_rect(r, x + 8, y + 12, 4, 1, rgb(0xC8, 0xA6, 0x8E));
    if ((frame / 20) % 2 == 0)
        px_rect(r, x + 9, y + 9, 1, 2, rgb(0xE0, 0xD8, 0xE4));
    /* a little cat sitting with them, sometimes */
    if (has_cat) {
        px_rect(r, x + 10, y + 14, 6, 4, rgb(0xD8, 0xB0, 0x90));  /* body */
        px_rect(r, x + 14, y + 11, 3, 4, rgb(0xD8, 0xB0, 0x90));  /* head */
        px_rect(r, x + 14, y + 10, 1, 2, rgb(0xD8, 0xB0, 0x90));  /* ear */
        px_rect(r, x + 16, y + 10, 1, 2, rgb(0xD8, 0xB0, 0x90));
    }
}

void cafecats_draw_patrons(SDL_Renderer *r, const CafeCats *cc, Uint64 frame) {
    for (int i = 0; i < cc->patron_count; i++)
        draw_patron(r, cc->patron_x[i], cc->patron_shirt[i],
                    cc->patron_has_cat[i], frame);
}

void cafecats_update(CafeCats *cc, Uint64 frame) {
    for (int i = 0; i < CAFE_CATS_MAX; i++) {
        CafeCat *c = &cc->cats[i];
        if (!c->present) continue;
        cat_update(&c->anim);
        if (c->friendship > 0) c->friendship--;   /* heart fades after petting */
        /* gentle idle: occasionally shift pose */
        if ((frame + (Uint64)i * 37) % 360 == 0) {
            int r = SDL_rand(3);
            c->anim.act = (r == 0) ? ACT_SIT : (r == 1) ? ACT_GROOM : ACT_SLEEP;
        }
    }
    (void)cc->refresh_timer;   /* residents are permanent — no refresh */
}

void cafecats_draw(SDL_Renderer *r, const CafeCats *cc, Uint64 frame) {
    for (int i = 0; i < CAFE_CATS_MAX; i++) {
        const CafeCat *c = &cc->cats[i];
        if (!c->present) continue;
        CatColors col = cattype_colors(c->type);
        cat_draw(r, &c->anim, col, frame);

        /* a brief happy heart floats up right after you pet this resident */
        if (c->friendship > 0) {
            float mx = c->home_x, my = c->home_y - 22 - (c->friendship % 20) * 0.4f;
            px_rect(r, mx, my, 2, 2, KZ_HEART);
            px_rect(r, mx + 3, my, 2, 2, KZ_HEART);
            px_rect(r, mx - 1, my + 1, 7, 2, KZ_HEART);
            px_rect(r, mx, my + 3, 5, 1, KZ_HEART);
            px_rect(r, mx + 1, my + 4, 3, 1, KZ_HEART);
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

void cafecats_pet(CafeCats *cc, int index) {
    if (index < 0 || index >= CAFE_CATS_MAX) return;
    CafeCat *c = &cc->cats[index];
    if (!c->present) return;
    cat_pet(&c->anim);
    c->friendship = 40;   /* shows a happy heart for a little while */
}