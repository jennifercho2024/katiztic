/* scene.c — see scene.h. The meadow, drawn back-to-front. */
#include "scene.h"
#include "render.h"
#include <math.h>

/* The four moments of day, as a static table. Tuning the whole mood of the
 * game is editing these numbers — which is exactly the point. */
static const TimeOfDay TIMES[KZ_TIME_COUNT] = {
    [KZ_DAWN] = {
        "dawn",
        RGB(0xF7,0xD8,0xE0), RGB(0xF3,0xD0,0xDE), RGB(0xE8,0xCB,0xE4),
        RGB(0xC8,0xE8,0xD4), RGB(0xB4,0xDE,0xC6), RGB(0xD4,0xC2,0xE8),
        RGB(0xF7,0xC8,0xD8), 108,
    },
    [KZ_NOON] = {
        "noon",
        RGB(0xCF,0xE9,0xF2), RGB(0xD6,0xEC,0xEC), RGB(0xE4,0xF3,0xEC),
        RGB(0xC8,0xE8,0xD4), RGB(0xA8,0xDC,0xBE), RGB(0xC2,0xDF,0xE8),
        RGB(0xF5,0xE6,0xC8), 76,
    },
    [KZ_DUSK] = {
        "dusk",
        RGB(0xF7,0xC8,0xD8), RGB(0xD4,0xC2,0xE8), RGB(0xC2,0xDF,0xE8),
        RGB(0xB8,0xDE,0xC8), RGB(0xA2,0xCE,0xBA), RGB(0xB8,0xA6,0xD0),
        RGB(0xF7,0xC8,0xD8), 140,
    },
    [KZ_NIGHT] = {
        "night",
        RGB(0x8B,0x7B,0xB0), RGB(0x7B,0x6B,0xA0), RGB(0x95,0x85,0xB8),
        RGB(0x8F,0xB8,0xA0), RGB(0x7C,0xA8,0x90), RGB(0x6B,0x5B,0x8B),
        RGB(0x6B,0x5B,0x8B), 158,
    },
};

const TimeOfDay *time_of_day(TimeIndex i) { return &TIMES[i]; }

TimeIndex time_from_hour(int hour) {
    /* Simple, cozy mapping of the day. */
    if (hour >= 5  && hour < 8)  return KZ_DAWN;   /* early morning glow */
    if (hour >= 8  && hour < 17) return KZ_NOON;   /* bright daytime     */
    if (hour >= 17 && hour < 20) return KZ_DUSK;   /* golden evening     */
    return KZ_NIGHT;                                /* 20:00–05:00 dark   */
}

Meadow meadow_make(void) {
    Meadow m;
    m.time = KZ_DUSK;  /* open on the dusk vibe from the mockup */
    for (int i = 0; i < KZ_PETAL_COUNT; i++) {
        m.petals[i].x     = (float)(SDL_rand(MEADOW_ROOM_W));
        m.petals[i].y     = (float)(SDL_rand(KZ_H));
        m.petals[i].speed = 0.2f + SDL_randf() * 0.4f;
        m.petals[i].drift = SDL_randf() * 6.28f;
        m.petals[i].size  = 1 + SDL_rand(2);
    }
    return m;
}

void meadow_cycle_time(Meadow *m) {
    m->time = (TimeIndex)((m->time + 1) % KZ_TIME_COUNT);
}

void meadow_update(Meadow *m) {
    for (int i = 0; i < KZ_PETAL_COUNT; i++) {
        Petal *p = &m->petals[i];
        p->y += p->speed;
        p->x += sinf(p->drift) * 0.15f;
        p->drift += 0.01f;
        if (p->y > KZ_H) { p->y = -4; p->x = (float)(SDL_rand(MEADOW_ROOM_W)); }
    }
}

/* Vertical 3-band gradient, drawn as rows so it stays crisp/retro. */
static void draw_sky_wide(SDL_Renderer *r, const TimeOfDay *t, int room_w);

static void draw_sky(SDL_Renderer *r, const TimeOfDay *t) {
    draw_sky_wide(r, t, KZ_W);
}

/* the gradient sky, drawn across a room of the given width (KZ_W or wider) */
static void draw_sky_wide(SDL_Renderer *r, const TimeOfDay *t, int room_w) {
    for (int y = 0; y < KZ_H; y++) {
        float f = (float)y / (float)KZ_H;
        Color a, b; float lf;
        if (f < 0.5f) { a = t->sky_top; b = t->sky_mid; lf = f / 0.5f; }
        else          { a = t->sky_mid; b = t->sky_bot; lf = (f - 0.5f) / 0.5f; }
        Uint8 rr = (Uint8)(a.r + (b.r - a.r) * lf);
        Uint8 gg = (Uint8)(a.g + (b.g - a.g) * lf);
        Uint8 bb = (Uint8)(a.b + (b.b - a.b) * lf);
        px_rect(r, 0, (float)y, (float)room_w, 1, rgb(rr, gg, bb));
    }
}

void meadow_draw(SDL_Renderer *r, const Meadow *m, Uint64 frame) {
    const TimeOfDay *t = &TIMES[m->time];
    const int MW = MEADOW_ROOM_W;   /* the meadow is wider than the screen */

    draw_sky_wide(r, t, MW);

    /* Stars, only at night, gently twinkling. */
    if (m->time == KZ_NIGHT) {
        for (int i = 0; i < 40; i++) {
            float sx = (float)((i * 53) % MW);
            float sy = (float)((i * 29) % 70);
            float b  = 0.4f + 0.6f * fabsf(sinf((float)frame * 0.03f + i));
            px_rect_a(r, sx, sy, 1, 1, KZ_CLOUD, (Uint8)(b * 255));
        }
    }

    /* Soft sun / moon — sits in the sky part-way across the room. */
    Color orb = (m->time == KZ_NIGHT) ? rgb(0xE8,0xE0,0xF0) : rgb(0xFB,0xF0,0xD8);
    px_rect_a(r, 300, 30, 18, 18, orb, 130);

    /* Distant hills — one soft rounded band across the whole room. */
    for (int x = 0; x < MW; x++) {
        float h = 88.0f + sinf((float)x * 0.03f) * 10.0f;
        px_rect(r, (float)x, h, 1, (float)KZ_H - h, t->hill);
    }

    /* Near grass. */
    px_rect(r, 0, 120, MW, KZ_H - 120, t->grass);

    /* Swaying grass blades across the room. */
    for (int b = 0; b < MW / 6; b++) {
        float bx   = (float)(b * 6 + 3);
        float sway = sinf((float)frame * 0.05f + b) * 1.5f;
        px_rect(r, bx + sway, 132, 1, 6, t->grass2);
    }

    /* Scattered flowers across the room. */
    Color fc[4] = { KZ_PETAL_PINK, KZ_BUTTER, KZ_LAVENDER, KZ_CLOUD };
    for (int f = 0; f < MW / 23; f++) {
        float fx = 18.0f + f * 23.0f;
        float fy = 128.0f + (f % 3) * 8.0f;
        px_rect(r, fx, fy, 2, 2, fc[f % 4]);
        px_rect(r, fx - 1, fy + 1, 1, 1, fc[(f + 1) % 4]);
        px_rect(r, fx + 2, fy + 1, 1, 1, fc[(f + 1) % 4]);
    }

    /* A friendly tree partway across, to give the wider meadow a landmark. */
    {
        float tx = 210;
        px_rect(r, tx, 96, 6, 24, rgb(0xA6, 0x7C, 0x5A));       /* trunk */
        px_rect(r, tx - 12, 74, 30, 24, t->hill);              /* canopy back */
        px_rect(r, tx - 8, 68, 22, 20, rgb(0x8F, 0xC0, 0x7A)); /* canopy */
        px_rect(r, tx - 4, 64, 14, 12, rgb(0x9C, 0xC6, 0x8E)); /* canopy top */
    }

    /* Drifting petals (drawn above the ground, below the wash). */
    for (int i = 0; i < KZ_PETAL_COUNT; i++) {
        const Petal *p = &m->petals[i];
        px_rect_a(r, p->x, p->y, (float)p->size, (float)p->size,
                  rgb(0xF7,0xD0,0xDE), 216);
    }
}

void meadow_draw_wash(SDL_Renderer *r, const Meadow *m) {
    const TimeOfDay *t = &TIMES[m->time];
    /* A single flat wash. (True soft-light blending is a later polish pass;
     * a low-alpha overlay already sells the mood shift convincingly.) */
    px_rect_a(r, 0, 0, KZ_W, KZ_H, t->wash, t->wash_alpha / 3);
}