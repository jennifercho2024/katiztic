/* street.c — see street.h. The village lane, drawn back-to-front. */
#include "street.h"
#include "render.h"
#include "palette.h"
#include <math.h>

void street_draw(SDL_Renderer *r, Uint64 frame, bool night) {
    street_draw_wide(r, frame, night, KZ_W);
}

void street_draw_wide(SDL_Renderer *r, Uint64 frame, bool night, int room_w) {
    const int SW = room_w;
    /* ---- sky above the rooftops ---- */
    px_rect(r, 0, 0, SW, 30, rgb(0xC2, 0xDF, 0xE8));
    px_rect(r, 0, 30, SW, 12, rgb(0xE2, 0xE4, 0xEC));

    /* ---- a row of pastel house fronts repeating along the lane ---- */
    Color body[4] = { rgb(0xF2,0xD8,0xC8), rgb(0xD8,0xE2,0xF0),
                      rgb(0xE6,0xD8,0xEE), rgb(0xF5,0xEC,0xC8) };
    Color roof[4] = { rgb(0xC8,0x9C,0x9C), rgb(0x9C,0xB0,0xC8),
                      rgb(0xB0,0x9C,0xC4), rgb(0xC8,0xB8,0x8B) };
    /* houses are ~60px wide; tile them across the whole room */
    for (int i = 0; i * 60 < SW; i++) {
        float x = (float)(i * 60);
        int c = i % 4;
        px_rect(r, x, 44, 58, 60, body[c]);
        px_rect(r, x, 36, 58, 10, roof[c]);
        /* door */
        px_rect(r, x + 8, 82, 12, 22, rgb(0xB0, 0x8E, 0x9C));
        px_rect(r, x + 16, 92, 2, 2, KZ_BUTTER);
        /* windows, warm-lit after dark */
        Color pane = night ? rgb(0xF5, 0xE0, 0xA8) : rgb(0xC2, 0xDF, 0xE8);
        px_rect(r, x + 28, 58, 12, 12, pane);
        px_rect(r, x + 44, 58, 10, 12, pane);
        px_rect(r, x + 28, 63, 12, 1, KZ_CLOUD);
        px_rect(r, x + 33, 58, 1, 12, KZ_CLOUD);
        /* flower box */
        px_rect(r, x + 27, 70, 14, 3, rgb(0xB0, 0x8E, 0x9C));
        px_rect(r, x + 28, 68, 3, 2, KZ_PETAL_PINK);
        px_rect(r, x + 33, 68, 3, 2, KZ_HEART);
        px_rect(r, x + 37, 68, 3, 2, KZ_PETAL_PINK);
    }

    /* ---- sidewalk, then cobbled lane, across the whole room ---- */
    px_rect(r, 0, 104, SW, 16, rgb(0xE0, 0xD8, 0xCC));     /* pavement */
    px_rect(r, 0, 104, SW, 1,  rgb(0xC8, 0xBE, 0xB0));     /* kerb     */
    px_rect(r, 0, 120, SW, KZ_H - 120, rgb(0xC8, 0xBE, 0xB4)); /* lane */
    for (int cyi = 0; cyi < 4; cyi++) {                     /* cobbles  */
        for (int cx = 0; cx * 20 < SW; cx++) {
            float ox = (cyi % 2) ? 10.0f : 0.0f;
            px_rect(r, cx * 20.0f + ox + 4, 124.0f + cyi * 9.0f, 12, 5,
                    rgb(0xBE, 0xB2, 0xA8));
        }
    }

    /* ---- lampposts along the lane; lit and haloed after dark ---- */
    for (int l = 0; l * 142 < SW; l++) {
        float x = 44.0f + (float)l * 142.0f;
        px_rect(r, x, 66, 3, 38, rgb(0x8A, 0x7A, 0x88));     /* pole  */
        px_rect(r, x - 2, 60, 7, 7, rgb(0x8A, 0x7A, 0x88));  /* head  */
        Color glass = night ? rgb(0xF7, 0xE4, 0xA0) : KZ_CLOUD;
        px_rect(r, x - 1, 61, 5, 5, glass);                  /* lamp  */
        if (night) {
            float p = 0.7f + 0.3f * sinf((float)frame * 0.05f + (float)l);
            px_rect_a(r, x - 5, 57, 13, 13, rgb(0xF7, 0xE4, 0xA0),
                      (Uint8)(60.0f * p));
        }
    }

    /* ---- night dim, matching the other places ---- */
    if (night) {
        px_rect_a(r, 0, 0, SW, KZ_H, rgb(0x6B, 0x5B, 0x8B), 60);
    }
}

/* ---- roadside props for the continuous street walk ---- */

static void walk_lamppost(SDL_Renderer *r, float x, bool night) {
    px_rect(r, x, 60, 3, 46, rgb(0x8A, 0x7A, 0x88));      /* pole */
    px_rect(r, x - 2, 54, 7, 7, rgb(0x8A, 0x7A, 0x88));   /* head */
    Color glass = night ? rgb(0xF7, 0xE4, 0xA0) : KZ_CLOUD;
    px_rect(r, x - 1, 55, 5, 5, glass);
    if (night)
        px_rect_a(r, x - 5, 51, 13, 13, rgb(0xF7, 0xE4, 0xA0), 70);
}

static void walk_bench(SDL_Renderer *r, float x) {
    px_rect(r, x, 96, 26, 3, rgb(0xC8, 0xA6, 0x8E));      /* seat */
    px_rect(r, x, 90, 26, 3, rgb(0xC8, 0xA6, 0x8E));      /* back */
    px_rect(r, x + 1, 99, 3, 7, rgb(0xB0, 0x8E, 0x76));   /* legs */
    px_rect(r, x + 22, 99, 3, 7, rgb(0xB0, 0x8E, 0x76));
    px_rect(r, x + 1, 90, 2, 9, rgb(0xB0, 0x8E, 0x76));   /* back posts */
    px_rect(r, x + 23, 90, 2, 9, rgb(0xB0, 0x8E, 0x76));
}

static void walk_bicycle(SDL_Renderer *r, float x) {
    Color frame = rgb(0xC8, 0x8C, 0x9C);
    px_rect(r, x, 96, 12, 12, rgb(0x6E, 0x66, 0x72));     /* rear wheel */
    px_rect(r, x + 2, 98, 8, 8, rgb(0xE0, 0xD8, 0xE0));
    px_rect(r, x + 18, 96, 12, 12, rgb(0x6E, 0x66, 0x72));/* front wheel */
    px_rect(r, x + 20, 98, 8, 8, rgb(0xE0, 0xD8, 0xE0));
    px_rect(r, x + 6, 92, 18, 2, frame);                  /* top bar */
    px_rect(r, x + 6, 92, 2, 10, frame);                  /* seat tube */
    px_rect(r, x + 22, 88, 2, 14, frame);                 /* fork */
    px_rect(r, x + 4, 88, 6, 2, rgb(0x8A, 0x7A, 0x88));   /* seat */
    px_rect(r, x + 20, 86, 6, 2, rgb(0x8A, 0x7A, 0x88));  /* handlebar */
}

static void walk_vending(SDL_Renderer *r, float x, bool night) {
    px_rect(r, x, 74, 20, 32, rgb(0xC0, 0x9C, 0xB4));     /* body */
    px_rect(r, x, 74, 20, 1, KZ_COCOA);
    px_rect(r, x + 2, 78, 10, 18, night ? rgb(0xF5, 0xE0, 0xA8)
                                        : rgb(0xCF, 0xE6, 0xF2)); /* window */
    /* rows of little drinks */
    for (int rrow = 0; rrow < 3; rrow++)
        for (int c = 0; c < 3; c++)
            px_rect(r, x + 3 + c * 3, 80 + rrow * 5, 2, 3,
                    (rrow + c) % 2 ? KZ_PETAL_PINK : KZ_MINT);
    px_rect(r, x + 14, 80, 4, 6, rgb(0x8A, 0x7A, 0x88));  /* keypad */
    px_rect(r, x + 3, 100, 14, 3, rgb(0x6E, 0x66, 0x72)); /* dispenser slot */
}

static void walk_bush(SDL_Renderer *r, float x) {
    Color g = rgb(0x9C, 0xC6, 0x8E), g2 = rgb(0xB6, 0xDA, 0xA0);
    px_rect(r, x, 96, 24, 12, g);
    px_rect(r, x + 3, 92, 18, 8, g2);
    px_rect(r, x + 7, 89, 10, 5, g);
    px_rect(r, x + 5, 95, 2, 2, KZ_PETAL_PINK);           /* tiny blossoms */
    px_rect(r, x + 15, 93, 2, 2, KZ_BUTTER);
}

void street_walk_draw(SDL_Renderer *r, float scroll, Uint64 frame, bool night) {
    /* sky over the rooftops */
    px_rect(r, 0, 0, KZ_W, 30, night ? rgb(0x9C, 0x92, 0xC0)
                                     : rgb(0xC2, 0xDF, 0xE8));
    px_rect(r, 0, 30, KZ_W, 12, night ? rgb(0x8C, 0x86, 0xB0)
                                      : rgb(0xE2, 0xE4, 0xEC));

    /* distant rooftops drifting slowly (parallax) */
    float slow = scroll * 0.35f;
    Color roofc[3] = { rgb(0xC8,0x9C,0x9C), rgb(0x9C,0xB0,0xC8),
                       rgb(0xB0,0x9C,0xC4) };
    for (int i = -1; i < 7; i++) {
        float bx = i * 44 - fmodf(slow, 44.0f);
        px_rect(r, bx, 30, 40, 14, roofc[((i % 3) + 3) % 3]);
        px_rect(r, bx + 6, 24, 28, 8, roofc[((i % 3) + 3) % 3]);
    }

    /* building wall band behind the walk */
    px_rect(r, 0, 44, KZ_W, 60, night ? rgb(0x86, 0x80, 0xA0)
                                      : rgb(0xEC, 0xE0, 0xE8));

    /* the wide sidewalk the cat strolls along */
    px_rect(r, 0, 104, KZ_W, KZ_H - 104, rgb(0xE0, 0xD8, 0xCC));
    px_rect(r, 0, 104, KZ_W, 2, rgb(0xC8, 0xBE, 0xB0));      /* kerb line */
    /* paving joints drifting past to convey motion */
    for (int i = -1; i < 14; i++) {
        float cx = i * 22 - fmodf(scroll, 22.0f);
        px_rect(r, cx, 106, 1, KZ_H - 106, rgb(0xCE, 0xC4, 0xB6));
    }

    /* roadside props scrolling past: a repeating parade of street furniture.
     * Each 120px cell holds a different prop so the walk feels varied. */
    for (int i = -1; i < 6; i++) {
        float base = i * 120 - fmodf(scroll, 120.0f);
        walk_lamppost(r, base + 8, night);
        walk_bench(r, base + 30);
        walk_bicycle(r, base + 64);
        walk_bush(r, base + 96);
        walk_vending(r, base + 116, night);
    }

    /* a bird flitting along to keep the cat company */
    float bx = 150 + sinf((float)frame * 0.05f) * 30;
    float by = 30 + sinf((float)frame * 0.08f) * 6;
    px_rect(r, bx, by, 3, 1, rgb(0x7A, 0x86, 0x9C));
    px_rect(r, bx + 2, by - 1, 2, 1, rgb(0x7A, 0x86, 0x9C));

    if (night)
        px_rect_a(r, 0, 0, KZ_W, KZ_H, rgb(0x6B, 0x5B, 0x8B), 60);
}