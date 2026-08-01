/* cottage.c — see cottage.h. The home interior, drawn back-to-front. */
#include "cottage.h"
#include "render.h"
#include "palette.h"
#include <math.h>

/* The bed sits in the lower-left, a comfortable tap target. */
static const Rect BED = { 14, 96, 64, 40 };

Rect cottage_bed_rect(void) { return BED; }

bool cottage_bed_hit(float px_, float py_) {
    return px_ >= BED.x && px_ <= BED.x + BED.w
        && py_ >= BED.y && py_ <= BED.y + BED.h;
}

/* Local warm-wood palette for the interior. */
#define WALL      rgb(0xF3, 0xE4, 0xE8)   /* soft rosy plaster */
#define WALL_LOW  rgb(0xE8, 0xD4, 0xDC)
#define FLOOR     rgb(0xE4, 0xCE, 0xB8)   /* warm wood         */
#define FLOOR_LN  rgb(0xD8, 0xBE, 0xA6)   /* plank lines       */
#define BED_FRAME rgb(0xC8, 0xA8, 0xB0)
#define BED_QUILT rgb(0xF7, 0xC8, 0xD8)   /* petal-pink quilt  */
#define BED_PILLW rgb(0xFB, 0xF6, 0xF2)
#define RUG       rgb(0xD4, 0xC2, 0xE8)   /* lavender rug      */

void cottage_draw(SDL_Renderer *r, Uint64 frame, bool night) {
    /* ---- walls + floor ---- */
    px_rect(r, 0, 0, KZ_W, 104, WALL);
    px_rect(r, 0, 96, KZ_W, 8, WALL_LOW);      /* baseboard shade */
    px_rect(r, 0, 104, KZ_W, KZ_H - 104, FLOOR);
    /* plank lines */
    for (int i = 0; i < 6; i++) {
        px_rect(r, 0, 104 + i * 10, KZ_W, 1, FLOOR_LN);
    }

    /* ---- window with sky + swaying curtains ---- */
    float wx = 150, wy = 18, ww = 62, wh = 46;
    px_rect(r, wx - 3, wy - 3, ww + 6, wh + 6, BED_FRAME);   /* frame */
    /* sky inside: day = soft blue-pink, night = deep lavender + a star */
    Color sky_top = night ? rgb(0x7B,0x6B,0xA0) : rgb(0xC2,0xDF,0xE8);
    Color sky_bot = night ? rgb(0x95,0x85,0xB8) : rgb(0xF7,0xD8,0xE0);
    for (int y = 0; y < (int)wh; y++) {
        float f = (float)y / wh;
        Uint8 rr = (Uint8)(sky_top.r + (sky_bot.r - sky_top.r) * f);
        Uint8 gg = (Uint8)(sky_top.g + (sky_bot.g - sky_top.g) * f);
        Uint8 bb = (Uint8)(sky_top.b + (sky_bot.b - sky_top.b) * f);
        px_rect(r, wx, wy + y, ww, 1, rgb(rr, gg, bb));
    }
    if (night) {
        px_rect(r, wx + 14, wy + 10, 1, 1, KZ_CLOUD);
        px_rect(r, wx + 40, wy + 18, 1, 1, KZ_CLOUD);
        px_rect(r, wx + 26, wy + 28, 1, 1, KZ_CLOUD);
    } else {
        px_rect_a(r, wx + 40, wy + 8, 10, 10, rgb(0xFB,0xF0,0xD8), 150); /* sun */
    }
    /* muntin bars */
    px_rect(r, wx + ww/2, wy, 1, wh, BED_FRAME);
    px_rect(r, wx, wy + wh/2, ww, 1, BED_FRAME);
    /* swaying curtains */
    float sway = sinf((float)frame * 0.04f) * 1.5f;
    px_rect(r, wx - 3,             wy - 3, 8, wh + 6, WALL_LOW);
    px_rect(r, wx + ww - 5 + sway, wy - 3, 8, wh + 6, WALL_LOW);

    /* ---- rug ---- */
    px_rect(r, 92, 128, 56, 22, RUG);
    px_rect(r, 96, 132, 48, 14, rgb(0xE0,0xD2,0xF0));

    /* ---- food bowl (where feeding happens) ---- */
    px_rect(r, 120, 112, 16, 6, BED_FRAME);
    px_rect(r, 122, 112, 12, 3, rgb(0xF5,0xB8,0x8B));  /* kibble */

    /* ---- bed ---- */
    px_rect(r, BED.x - 2, BED.y - 2, BED.w + 4, BED.h + 4, BED_FRAME);
    px_rect(r, BED.x, BED.y + 8, BED.w, BED.h - 8, BED_QUILT);   /* quilt */
    /* quilt patchwork dots */
    for (int i = 0; i < 5; i++)
        for (int j = 0; j < 2; j++)
            px_rect(r, BED.x + 6 + i * 12, BED.y + 16 + j * 12, 3, 3,
                    rgb(0xF5,0xE6,0xC8));
    px_rect(r, BED.x + 4, BED.y, 24, 12, BED_PILLW);            /* pillow */

    /* ---- night dim: a soft cool wash over the whole room ---- */
    if (night) {
        px_rect_a(r, 0, 0, KZ_W, KZ_H, rgb(0x6B,0x5B,0x8B), 70);
    }

    /* ---- dust motes drifting in the window light (day only) ---- */
    if (!night) {
        for (int i = 0; i < 6; i++) {
            float mx = wx + 6 + i * 9 + sinf((float)frame * 0.02f + i) * 4;
            float my = wy + 20 + ((frame / 2 + (Uint64)(i * 13)) % 50);
            px_rect_a(r, mx, my, 1, 1, rgb(0xFB,0xF0,0xD8), 120);
        }
    }
}