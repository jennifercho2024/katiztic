/* forest.c — see forest.h. The quiet wood, drawn back-to-front. */
#include "forest.h"
#include "render.h"
#include "palette.h"
#include <math.h>

void forest_draw(SDL_Renderer *r, Uint64 frame, bool night) {
    forest_draw_wide(r, frame, night, KZ_W);
}

void forest_draw_wide(SDL_Renderer *r, Uint64 frame, bool night, int room_w) {
    const int FW = room_w;
    /* ---- sky glimpsed through the trees ---- */
    px_rect(r, 0, 0, FW, 36, rgb(0xD4, 0xC2, 0xE8));    /* lavender     */
    px_rect(r, 0, 36, FW, 24, rgb(0xE2, 0xD4, 0xEC));   /* paler below  */

    /* ---- distant canopy: soft banks of far leaves ---- */
    px_rect(r, 0, 24, FW, 30, rgb(0xC2, 0xE2, 0xCC));
    px_rect(r, 0, 46, FW, 16, rgb(0xB2, 0xD8, 0xBE));

    /* ---- mossy ground ---- */
    px_rect(r, 0, 112, FW, KZ_H - 112, rgb(0xB4, 0xD8, 0xB6));
    for (int i = 0; i < FW / 30 + 1; i++) {   /* soft moss tufts */
        float mx = 10.0f + (float)i * 30.0f + (float)(i % 3) * 5.0f;
        px_rect(r, mx, 118 + (float)(i % 4) * 9.0f, 8, 2,
                rgb(0xA4, 0xCC, 0xA8));
    }

    /* ---- a winding path leading deeper in ---- */
    px_rect(r, 96, 112, 52, 10, rgb(0xE8, 0xD8, 0xB8));
    px_rect(r, 88, 122, 60, 12, rgb(0xE8, 0xD8, 0xB8));
    px_rect(r, 80, 134, 70, 26, rgb(0xE8, 0xD8, 0xB8));

    /* ---- trees: mauve trunks under swaying leaf clusters, across the room -- */
    for (int t = 0; t * 48 < FW; t++) {
        float x = 20.0f + (float)t * 48.0f;
        float sway = sinf((float)frame * 0.015f + (float)t) * 1.5f;
        px_rect(r, x, 48, 9, 66, rgb(0xB0, 0x8E, 0x9C));
        px_rect(r, x, 48, 3, 66, rgb(0x9A, 0x7A, 0x88));
        px_rect(r, x - 3, 110, 15, 4, rgb(0x9A, 0x7A, 0x88));
        px_rect(r, x - 14 + sway, 18, 38, 20, rgb(0xA8, 0xD4, 0xB8));
        px_rect(r, x - 8 + sway,  8,  26, 14, rgb(0xC8, 0xE8, 0xD4));
        px_rect(r, x - 18 + sway, 32, 44, 12, rgb(0x94, 0xC4, 0xA4));
    }

    /* ---- ferns at the trees' feet ---- */
    for (int f = 0; f * 52 < FW; f++) {
        float fx = 48.0f + (float)f * 52.0f;
        Color fern = rgb(0x8F, 0xC0, 0xA4);
        px_rect(r, fx,     116, 2, 8, fern);
        px_rect(r, fx - 4, 118, 2, 6, fern);
        px_rect(r, fx + 4, 118, 2, 6, fern);
        px_rect(r, fx - 4, 124, 10, 2, rgb(0xA4, 0xCC, 0xA8));
    }

    /* ---- little mushrooms ---- */
    for (int m = 0; m * 88 < FW; m++) {
        float mx = 36.0f + (float)m * 88.0f;
        px_rect(r, mx + 3, 132, 3, 5, KZ_CLOUD);
        px_rect(r, mx, 129, 9, 4, KZ_PETAL_PINK);
        px_rect(r, mx + 2, 130, 2, 1, KZ_CLOUD);
        px_rect(r, mx + 6, 131, 1, 1, KZ_CLOUD);
    }

    /* ---- drifting motes: the forest's sleeping magic, rising softly ---- */
    for (int m = 0; m < FW / 40; m++) {
        Uint64 fm = frame + (Uint64)(m * 67);
        float cyc = (float)(fm % 300);
        float mx = 18.0f + (float)m * 38.0f
                 + sinf((float)fm * 0.01f + (float)m) * 8.0f;
        float my = 128.0f - cyc * 0.3f;              /* drift upward */
        float fade_in  = cyc < 40.0f ? cyc / 40.0f : 1.0f;
        float fade_out = cyc > 240.0f ? (300.0f - cyc) / 60.0f : 1.0f;
        Uint8 a = (Uint8)(fade_in * fade_out * 150.0f);
        px_rect_a(r, mx, my, 2, 2, rgb(0xF7, 0xE8, 0xC0), a);
    }

    /* ---- night dim, matching the other places ---- */
    if (night) {
        px_rect_a(r, 0, 0, FW, KZ_H, rgb(0x6B, 0x5B, 0x8B), 70);
    }
}