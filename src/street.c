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