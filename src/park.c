/* park.c — see park.h. A cheerful playground park for cats. */
#include "park.h"
#include "render.h"
#include "palette.h"
#include "cat.h"
#include <math.h>

/* ---- the playground scene ---- */

static void climbing_tree(SDL_Renderer *r, float x, float y) {
    /* a cat tree: posts with carpeted platforms */
    Color post = rgb(0xC8, 0xA6, 0x8E), plat = rgb(0xB0, 0x92, 0xCE),
          platd = rgb(0x98, 0x7E, 0xB2);
    px_rect(r, x + 6, y, 5, 44, post);            /* main post */
    px_rect(r, x + 16, y + 14, 5, 30, post);      /* second post */
    px_rect(r, x, y, 22, 6, plat);                /* top platform */
    px_rect(r, x, y + 5, 22, 1, platd);
    px_rect(r, x + 10, y + 16, 20, 6, plat);      /* mid platform */
    px_rect(r, x + 10, y + 21, 20, 1, platd);
    px_rect(r, x - 2, y + 30, 16, 6, plat);       /* low platform */
    px_rect(r, x - 2, y + 35, 16, 1, platd);
    /* a dangling toy */
    px_rect(r, x + 3, y + 6, 1, 5, KZ_COCOA);
    px_rect(r, x + 2, y + 11, 3, 3, KZ_PETAL_PINK);
}

static void slide(SDL_Renderer *r, float x, float y) {
    Color frame = rgb(0xC8, 0xA6, 0x8E), chute = rgb(0x9C, 0xC0, 0xD8),
          chuted = rgb(0x7C, 0xA6, 0xC4);
    px_rect(r, x, y, 3, 26, frame);               /* ladder side */
    px_rect(r, x + 10, y, 3, 26, frame);
    for (int i = 0; i < 5; i++)                    /* rungs */
        px_rect(r, x + 3, y + 3 + i * 5, 7, 1, frame);
    px_rect(r, x, y - 4, 14, 5, chute);            /* top */
    /* the sloping chute */
    for (int i = 0; i < 20; i++) {
        px_rect(r, x + 13 + i, y - 3 + i, 4, 3, chute);
        px_rect(r, x + 13 + i, y + i, 4, 1, chuted);
    }
}

static void tunnel(SDL_Renderer *r, float x, float y) {
    Color t = rgb(0xC8, 0xE8, 0xD4), td = rgb(0xA8, 0xC8, 0xB4);
    /* a striped play tunnel (a soft arch) */
    for (int i = 0; i < 5; i++) {
        Color c = (i % 2) ? t : td;
        px_rect(r, x + i * 6, y - (i == 2 ? 2 : 0), 6, 14, c);
    }
    px_rect(r, x + 2, y + 3, 26, 8, rgb(0x6B, 0x5B, 0x8B));  /* dark opening */
    px_rect(r, x + 4, y + 5, 22, 5, rgb(0x4B, 0x40, 0x60));
}

static void ball_pit(SDL_Renderer *r, float x, float y, Uint64 frame) {
    Color rim = rgb(0xE8, 0xC6, 0x8E);
    px_rect(r, x, y, 40, 14, rgb(0xF3, 0xE6, 0xD6));   /* pit floor */
    px_rect(r, x, y, 40, 2, rim);                       /* rim */
    px_rect(r, x, y + 12, 40, 2, rim);
    /* colorful balls, gently bobbing */
    Color balls[4] = { KZ_PETAL_PINK, KZ_MINT, KZ_BUTTER, KZ_LAVENDER };
    for (int i = 0; i < 10; i++) {
        float bx = x + 3 + (i % 5) * 7;
        float by = y + 3 + (i / 5) * 5 + sinf((float)frame * 0.05f + i) * 0.6f;
        px_rect(r, bx, by, 4, 4, balls[i % 4]);
    }
}

void park_draw(SDL_Renderer *r, Uint64 frame, bool night) {
    /* sky + grass */
    px_rect(r, 0, 0, KZ_W, 54, night ? rgb(0x9C, 0x92, 0xC0)
                                     : rgb(0xCF, 0xE6, 0xF2));
    px_rect(r, 0, 54, KZ_W, KZ_H - 54, night ? rgb(0x8C, 0xA0, 0x84)
                                             : rgb(0xB6, 0xD6, 0xA0));
    px_rect(r, 0, 54, KZ_W, 3, night ? rgb(0x9C, 0xB0, 0x90)
                                     : rgb(0xC6, 0xE0, 0xAC));

    /* distant soft hills */
    for (int i = 0; i < 3; i++) {
        float base = 44.0f + i * 4;
        Color hill = night ? rgb(0x86, 0x9A, 0x80) : rgb(0xAC, 0xCE, 0x98);
        for (float x = 0; x < KZ_W; x += 2)
            px_rect(r, x, base + sinf(x * 0.04f + i) * 4, 2, 14, hill);
    }

    /* a winding path */
    for (int i = 0; i < 26; i++) {
        float px_ = i * 10;
        float py_ = 120 + sinf(i * 0.4f) * 8;
        px_rect(r, px_, py_, 11, 6, rgb(0xE0, 0xCE, 0xB4));
    }

    /* a couple of shade trees */
    for (int t = 0; t < 2; t++) {
        float tx = t == 0 ? 20 : 210;
        px_rect(r, tx, 60, 5, 20, rgb(0xA8, 0x86, 0x6E));
        px_rect(r, tx - 8, 44, 21, 18, rgb(0x9C, 0xC6, 0x8E));
        px_rect(r, tx - 5, 38, 15, 12, rgb(0xB6, 0xDA, 0xA0));
    }

    /* ---- the playground equipment ---- */
    climbing_tree(r, 34, 62);
    slide(r, 168, 70);
    tunnel(r, 96, 96);
    ball_pit(r, 30, 116, frame);

    /* a little bench */
    px_rect(r, 150, 128, 20, 3, rgb(0xC8, 0xA6, 0x8E));
    px_rect(r, 151, 131, 2, 5, rgb(0xB0, 0x8E, 0x76));
    px_rect(r, 167, 131, 2, 5, rgb(0xB0, 0x8E, 0x76));
    px_rect(r, 150, 123, 20, 2, rgb(0xC8, 0xA6, 0x8E));

    /* a fluttering butterfly for ambiance */
    float bx = 130 + sinf((float)frame * 0.03f) * 30;
    float by = 40 + sinf((float)frame * 0.06f) * 8;
    Color wing = (frame / 8) % 2 ? KZ_PETAL_PINK : KZ_LAVENDER;
    px_rect(r, bx, by, 2, 2, wing);
    px_rect(r, bx + 3, by, 2, 2, wing);
    px_rect(r, bx + 1, by + 1, 2, 1, KZ_COCOA);
}

/* ---- park visitors ---- */

static const char *VNAMES[] = {
    "Willow", "Otis", "Juniper", "Pip", "Clementine", "Bodhi", "Hazel",
};
#define VNAME_COUNT ((int)(sizeof VNAMES / sizeof VNAMES[0]))

ParkLife parklife_new(void) {
    ParkLife pl;
    for (int i = 0; i < PARK_VISITOR_MAX; i++) pl.v[i].active = false;
    pl.spawn_timer = 30.0f;
    return pl;
}

static void spawn_visitor(ParkVisitor *v) {
    v->active = true;
    v->dir = (SDL_rand(2) == 0) ? 1 : -1;
    /* wander within the park grounds, staying in view */
    v->x = 50.0f + SDL_randf() * 140.0f;
    v->y = 128.0f + SDL_randf() * 12.0f;
    v->speed = 0.12f + SDL_randf() * 0.10f;
    v->cat_type = (CatType)SDL_rand(KZ_TYPE_COUNT);
    v->owner = VNAMES[SDL_rand(VNAME_COUNT)];
    v->glow = 0;
    v->wander_timer = 100.0f + SDL_rand(160);
}

void parklife_update(ParkLife *pl, Uint64 frame) {
    (void)frame;
    int count = 0;
    for (int i = 0; i < PARK_VISITOR_MAX; i++)
        if (pl->v[i].active) count++;
    if (count < PARK_VISITOR_MAX) {
        pl->spawn_timer -= 1.0f;
        if (pl->spawn_timer <= 0.0f) {
            for (int i = 0; i < PARK_VISITOR_MAX; i++)
                if (!pl->v[i].active) { spawn_visitor(&pl->v[i]); break; }
            pl->spawn_timer = 120.0f + (float)SDL_rand(240);
        }
    }
    const float LEFT = 34.0f, RIGHT = (float)KZ_W - 40.0f;
    for (int i = 0; i < PARK_VISITOR_MAX; i++) {
        ParkVisitor *v = &pl->v[i];
        if (!v->active) continue;
        v->x += v->speed * v->dir;
        if (v->glow > 0) v->glow--;
        if (v->x < LEFT)  { v->x = LEFT;  v->dir = 1; }
        if (v->x > RIGHT) { v->x = RIGHT; v->dir = -1; }
        v->wander_timer -= 1.0f;
        if (v->wander_timer <= 0.0f) {
            if (SDL_rand(2) == 0) v->dir = -v->dir;
            v->wander_timer = 100.0f + SDL_rand(160);
        }
    }
}

void parklife_draw(SDL_Renderer *r, const ParkLife *pl, Uint64 frame) {
    for (int i = 0; i < PARK_VISITOR_MAX; i++) {
        const ParkVisitor *v = &pl->v[i];
        if (!v->active) continue;
        Cat c = cat_make(v->x, v->y);
        c.facing = v->dir;
        c.act = ACT_WALK;
        cat_draw(r, &c, cattype_colors(v->cat_type), frame);
        if (v->glow > 0 && (v->glow / 4) % 2 == 0) {
            px_rect(r, v->x + 4, v->y - 14, 2, 2, KZ_HEART);
            px_rect(r, v->x + 8, v->y - 16, 1, 1, KZ_PETAL_PINK);
        }
    }
}

int parklife_hit(const ParkLife *pl, float px_, float py_) {
    for (int i = 0; i < PARK_VISITOR_MAX; i++) {
        const ParkVisitor *v = &pl->v[i];
        if (!v->active) continue;
        if (px_ >= v->x - 14 && px_ <= v->x + 14
            && py_ >= v->y - 16 && py_ <= v->y + 14)
            return i;
    }
    return -1;
}

const char *parklife_greet(ParkLife *pl, int index, CatType *out_type) {
    if (index < 0 || index >= PARK_VISITOR_MAX) return NULL;
    ParkVisitor *v = &pl->v[index];
    if (!v->active) return NULL;
    v->glow = 40;
    if (out_type) *out_type = v->cat_type;
    return v->owner;
}