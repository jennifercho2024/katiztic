/* title.c — the Katiztic title screen, drawn in code (see title.h).
 *
 * Everything here is hand-placed pixels in the game's palette, so it stays
 * perfectly crisp at any window size — no image, no downscaling, no blur.
 */
#include "title.h"
#include "render.h"
#include "palette.h"
#include "text.h"
#include <math.h>

/* a soft four-point sparkle */
static void sparkle(SDL_Renderer *r, float x, float y, float s, Color c) {
    px_rect(r, x, y - s, 1, s * 2 + 1, c);       /* vertical  */
    px_rect(r, x - s, y, s * 2 + 1, 1, c);       /* horizontal */
    if (s >= 2) {                                 /* little diagonals */
        px(r, x - 1, y - 1, c);
        px(r, x + 1, y - 1, c);
        px(r, x - 1, y + 1, c);
        px(r, x + 1, y + 1, c);
    }
}

/* a puffy pastel cloud */
static void cloud(SDL_Renderer *r, float x, float y, Color c) {
    px_rect(r, x + 3, y, 10, 4, c);
    px_rect(r, x, y + 3, 18, 4, c);
    px_rect(r, x + 6, y - 2, 6, 3, c);
    px_rect(r, x + 1, y + 6, 16, 2, rgb(0xEC, 0xDD, 0xF0));  /* soft underside */
}

/* a single lavender sprig: a green stem with little purple buds */
static void lavender_sprig(SDL_Renderer *r, float x, float y) {
    Color stem = rgb(0x9C, 0xB0, 0x84), bud = rgb(0xB0, 0x92, 0xCE),
          budlt = rgb(0xC8, 0xAE, 0xE2);
    px_rect(r, x, y, 1, 10, stem);               /* stem */
    px_rect(r, x - 2, y + 4, 1, 4, stem);        /* side leaves */
    px_rect(r, x + 2, y + 5, 1, 4, stem);
    for (int i = 0; i < 4; i++) {                /* buds up the stem */
        px_rect(r, x - 1, y - 2 + i * 3, 3, 2, (i % 2) ? bud : budlt);
        px(r, x, y - 3 + i * 3, budlt);
    }
}

/* the little cottage in the corner */
static void cottage(SDL_Renderer *r, float x, float y) {
    Color wall = rgb(0xF3, 0xE6, 0xD6), roof = rgb(0xB0, 0x94, 0xC8),
          roofd = rgb(0x98, 0x7E, 0xB2), door = rgb(0x8E, 0x74, 0x9A),
          win = KZ_SKY_WASH, trim = rgb(0xC8, 0xAE, 0xE2);
    px_rect(r, x, y, 34, 22, wall);              /* body */
    px_rect(r, x - 3, y - 10, 40, 12, roof);     /* roof */
    px_rect(r, x - 3, y - 2, 40, 2, roofd);      /* roof shade */
    for (int i = 0; i < 20; i++)                 /* roof texture lines */
        px(r, x - 2 + i * 2, y - 8 + (i % 2), roofd);
    px_rect(r, x + 26, y - 18, 4, 9, roofd);     /* chimney */
    px_rect(r, x + 25, y - 19, 6, 2, roof);
    px_rect(r, x + 13, y + 8, 8, 14, door);      /* door */
    px_rect(r, x + 13, y + 8, 8, 1, trim);
    px(r, x + 19, y + 15, KZ_BUTTER);            /* doorknob */
    px_rect(r, x + 4, y + 5, 6, 6, win);         /* window */
    px_rect(r, x + 4, y + 5, 6, 6, win);
    px_rect(r, x + 6, y + 5, 1, 6, trim);        /* window cross */
    px_rect(r, x + 4, y + 7, 6, 1, trim);
    px_rect(r, x + 24, y + 4, 6, 6, win);        /* 2nd window */
    px_rect(r, x + 26, y + 4, 1, 6, trim);
    px_rect(r, x + 24, y + 6, 6, 1, trim);
}

void title_draw(SDL_Renderer *r, Uint64 frame) {
    float t = (float)frame;

    /* ---- lavender gradient sky (banded, lighter toward the middle) ---- */
    for (int y = 0; y < KZ_H; y++) {
        float f = (float)y / KZ_H;
        Uint8 rr = (Uint8)(0xD8 - f * 20);
        Uint8 gg = (Uint8)(0xCB - f * 14);
        Uint8 bb = (Uint8)(0xEC - f * 10);
        px_rect(r, 0, y, KZ_W, 1, rgb(rr, gg, bb));
    }

    /* ---- soft clouds ---- */
    cloud(r, 18, 30, KZ_CLOUD);
    cloud(r, 186, 22, KZ_CLOUD);
    cloud(r, 150, 48, rgb(0xF6, 0xEA, 0xF2));
    cloud(r, 40, 60, rgb(0xF6, 0xEA, 0xF2));

    /* ---- twinkling sparkles (gentle pulse) ---- */
    static const float SP[][3] = {
        {60, 18, 2}, {120, 12, 3}, {170, 40, 2}, {30, 44, 1},
        {200, 60, 2}, {96, 40, 1}, {210, 100, 2}, {150, 92, 1},
        {74, 100, 1}, {188, 128, 2},
    };
    for (int i = 0; i < (int)(sizeof SP / sizeof SP[0]); i++) {
        float tw = 0.5f + 0.5f * sinf(t * 0.05f + i);
        Uint8 a = (Uint8)(0x88 + tw * 0x60);
        sparkle(r, SP[i][0], SP[i][1], SP[i][2], rgb(a, a, 0xE0));
    }

    /* ---- rolling lavender field along the bottom ---- */
    px_rect(r, 0, 132, KZ_W, KZ_H - 132, rgb(0xB8, 0xC4, 0x9C));   /* meadow */
    px_rect(r, 0, 132, KZ_W, 2, rgb(0xC6, 0xD0, 0xAC));
    /* a winding path */
    for (int i = 0; i < 20; i++) {
        float px_ = 60 + i * 5;
        float py_ = 150 + sinf(i * 0.5f) * 3;
        px_rect(r, px_, py_, 6, 3, rgb(0xE0, 0xCE, 0xB4));
    }
    /* lavender sprigs across the foreground */
    for (int i = 0; i < 6; i++)
        lavender_sprig(r, 10 + i * 12, 140);
    for (int i = 0; i < 4; i++)
        lavender_sprig(r, 14 + i * 12, 150);

    /* the cottage nestled at bottom-right */
    cottage(r, 188, 128);

    /* ---- decorative border with heart corners ---- */
    Color frame_c = rgb(0xB0, 0x92, 0xCE);
    px_rect(r, 2, 2, KZ_W - 4, 2, frame_c);
    px_rect(r, 2, KZ_H - 4, KZ_W - 4, 2, frame_c);
    px_rect(r, 2, 2, 2, KZ_H - 4, frame_c);
    px_rect(r, KZ_W - 4, 2, 2, KZ_H - 4, frame_c);
    /* heart in each corner */
    for (int c = 0; c < 4; c++) {
        float hx = (c % 2 == 0) ? 8 : KZ_W - 14;
        float hy = (c < 2) ? 8 : KZ_H - 14;
        px_rect(r, hx, hy + 1, 2, 2, KZ_HEART);
        px_rect(r, hx + 3, hy + 1, 2, 2, KZ_HEART);
        px_rect(r, hx, hy + 2, 5, 2, KZ_HEART);
        px_rect(r, hx + 1, hy + 4, 3, 1, KZ_HEART);
        px_rect(r, hx + 2, hy + 5, 1, 1, KZ_HEART);
    }

    /* ---- the title: "katiztic" big, with a soft shadow ---- */
    const char *title = "katiztic";
    int scale = 4;
    float tw = text_width_scaled(title, scale);
    float tx = (KZ_W - tw) / 2.0f;
    float ty = 52;
    text_draw_scaled(r, title, tx + 1, ty + 1, rgb(0xB8, 0xA6, 0xD0), scale);
    text_draw_scaled(r, title, tx, ty, rgb(0x6E, 0x58, 0x92), scale);

    /* ---- "lavender version" on a soft chip ---- */
    const char *sub = "lavender version";
    float sw = text_width(sub);
    float cx = (KZ_W - (sw + 20)) / 2.0f;
    float cy = 86;
    px_rect(r, cx, cy, sw + 20, 12, KZ_PETAL_PINK);
    px_rect(r, cx, cy, sw + 20, 1, frame_c);
    px_rect(r, cx, cy + 11, sw + 20, 1, frame_c);
    /* tiny lavender sprig on each side of the chip */
    lavender_sprig(r, cx + 5, cy - 2);
    lavender_sprig(r, cx + sw + 14, cy - 2);
    text_draw(r, sub, cx + 10, cy + 3, rgb(0x7A, 0x62, 0x94));

    /* ---- "press start" button, gently pulsing ---- */
    const char *ps = "press start";
    float pw = text_width(ps);
    float bx = (KZ_W - (pw + 24)) / 2.0f;
    float by = 112;
    float pulse = 0.5f + 0.5f * sinf(t * 0.08f);
    Uint8 glow = (Uint8)(0xE0 + pulse * 0x1F);
    px_rect(r, bx, by, pw + 24, 14, rgb(0xFB, glow, 0xF2));
    px_rect(r, bx, by, pw + 24, 1, frame_c);
    px_rect(r, bx, by + 13, pw + 24, 1, frame_c);
    px_rect(r, bx, by, 1, 14, frame_c);
    px_rect(r, bx + pw + 23, by, 1, 14, frame_c);
    /* a heart on each side */
    px_rect(r, bx + 5, by + 5, 3, 2, KZ_HEART);
    px_rect(r, bx + pw + 16, by + 5, 3, 2, KZ_HEART);
    text_draw(r, ps, bx + 12, by + 4, rgb(0x6E, 0x58, 0x92));
}