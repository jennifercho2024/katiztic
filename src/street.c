/* street.c — see street.h. The village lane, drawn back-to-front. */
#include "street.h"
#include "render.h"
#include "palette.h"
#include <math.h>

void street_draw(SDL_Renderer *r, Uint64 frame, bool night) {
    /* ---- sky above the rooftops ---- */
    px_rect(r, 0, 0, KZ_W, 30, rgb(0xC2, 0xDF, 0xE8));
    px_rect(r, 0, 30, KZ_W, 12, rgb(0xE2, 0xE4, 0xEC));

    /* ---- a row of pastel house fronts, four across ---- */
    /* house 1: peach */
    px_rect(r, 0, 42, 62, 62, rgb(0xF2, 0xD8, 0xC8));
    px_rect(r, 0, 34, 62, 10, rgb(0xC8, 0x9C, 0x9C));       /* roof */
    /* house 2: powder blue */
    px_rect(r, 62, 46, 58, 58, rgb(0xD8, 0xE2, 0xF0));
    px_rect(r, 62, 38, 58, 10, rgb(0x9C, 0xB0, 0xC8));
    /* house 3: soft lilac */
    px_rect(r, 120, 42, 62, 62, rgb(0xE6, 0xD8, 0xEE));
    px_rect(r, 120, 34, 62, 10, rgb(0xB0, 0x9C, 0xC4));
    /* house 4: pale butter */
    px_rect(r, 182, 46, 58, 58, rgb(0xF5, 0xEC, 0xC8));
    px_rect(r, 182, 38, 58, 10, rgb(0xC8, 0xB8, 0x8B));

    /* doors and windows on each house */
    static const float HX[4] = { 0, 62, 120, 182 };
    for (int h = 0; h < 4; h++) {
        float x = HX[h];
        /* door */
        px_rect(r, x + 8, 82, 12, 22, rgb(0xB0, 0x8E, 0x9C));
        px_rect(r, x + 16, 92, 2, 2, KZ_BUTTER);             /* knob */
        /* windows, warm-lit after dark */
        Color pane = night ? rgb(0xF5, 0xE0, 0xA8) : rgb(0xC2, 0xDF, 0xE8);
        px_rect(r, x + 28, 58, 12, 12, pane);
        px_rect(r, x + 44, 58, 10, 12, pane);
        px_rect(r, x + 28, 63, 12, 1, KZ_CLOUD);             /* mullions */
        px_rect(r, x + 33, 58, 1, 12, KZ_CLOUD);
        /* flower box under the first window */
        px_rect(r, x + 27, 70, 14, 3, rgb(0xB0, 0x8E, 0x9C));
        px_rect(r, x + 28, 68, 3, 2, KZ_PETAL_PINK);
        px_rect(r, x + 33, 68, 3, 2, KZ_HEART);
        px_rect(r, x + 37, 68, 3, 2, KZ_PETAL_PINK);
    }

    /* ---- sidewalk, then cobbled lane ---- */
    px_rect(r, 0, 104, KZ_W, 16, rgb(0xE0, 0xD8, 0xCC));     /* pavement */
    px_rect(r, 0, 104, KZ_W, 1,  rgb(0xC8, 0xBE, 0xB0));     /* kerb     */
    px_rect(r, 0, 120, KZ_W, KZ_H - 120, rgb(0xC8, 0xBE, 0xB4)); /* lane */
    for (int cy = 0; cy < 4; cy++) {                          /* cobbles  */
        for (int cx = 0; cx < 12; cx++) {
            float ox = (cy % 2) ? 10.0f : 0.0f;
            px_rect(r, cx * 20.0f + ox + 4, 124.0f + cy * 9.0f, 12, 5,
                    rgb(0xBE, 0xB2, 0xA8));
        }
    }

    /* ---- lampposts; lit and haloed after dark ---- */
    static const float LX[2] = { 44, 186 };
    for (int l = 0; l < 2; l++) {
        float x = LX[l];
        px_rect(r, x, 66, 3, 38, rgb(0x8A, 0x7A, 0x88));     /* pole  */
        px_rect(r, x - 2, 60, 7, 7, rgb(0x8A, 0x7A, 0x88));  /* head  */
        Color glass = night ? rgb(0xF7, 0xE4, 0xA0) : KZ_CLOUD;
        px_rect(r, x - 1, 61, 5, 5, glass);                  /* lamp  */
        if (night) {
            /* a soft pulsing halo */
            float p = 0.7f + 0.3f * sinf((float)frame * 0.05f + (float)l);
            px_rect_a(r, x - 5, 57, 13, 13, rgb(0xF7, 0xE4, 0xA0),
                      (Uint8)(60.0f * p));
        }
    }

    /* ---- night dim, matching the other places ---- */
    if (night) {
        px_rect_a(r, 0, 0, KZ_W, KZ_H, rgb(0x6B, 0x5B, 0x8B), 60);
    }
}