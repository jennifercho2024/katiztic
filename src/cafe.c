/* cafe.c — see cafe.h. The cat café interior, drawn back-to-front. */
#include "cafe.h"
#include "render.h"
#include "palette.h"
#include <math.h>

/* Warm café palette — a little richer/cozier than the cottage. */
#define WALL     rgb(0xF0, 0xDE, 0xD0)   /* warm cream plaster   */
#define WALL_LOW rgb(0xE4, 0xCE, 0xBE)
#define FLOOR    rgb(0xD8, 0xBE, 0xA0)   /* honeyed wood         */
#define FLOOR_LN rgb(0xC8, 0xAC, 0x8E)
#define COUNTER  rgb(0xC0, 0x98, 0x88)   /* rosy wood counter    */
#define COUNTER2 rgb(0xB0, 0x86, 0x78)
#define JAR      rgb(0xC2, 0xDF, 0xE8)   /* glass treat jars     */
#define CUSHION  rgb(0xD4, 0xC2, 0xE8)

void cafe_draw(SDL_Renderer *r, Uint64 frame) {
    /* ---- walls + floor ---- */
    px_rect(r, 0, 0, KZ_W, 104, WALL);
    px_rect(r, 0, 96, KZ_W, 8, WALL_LOW);
    px_rect(r, 0, 104, KZ_W, KZ_H - 104, FLOOR);
    for (int i = 0; i < 6; i++)
        px_rect(r, 0, 104 + i * 10, KZ_W, 1, FLOOR_LN);

    /* ---- a hanging sign: "CAFE" suggested with soft blocks ---- */
    px_rect(r, 92, 10, 56, 16, KZ_CLOUD);
    px_rect(r, 92, 10, 56, 1, KZ_COCOA);
    px_rect(r, 92, 25, 56, 1, KZ_COCOA);
    px_rect(r, 92, 10, 1, 16, KZ_COCOA);
    px_rect(r, 147, 10, 1, 16, KZ_COCOA);
    /* little heart on the sign */
    px_rect(r, 116, 15, 2, 1, KZ_PETAL_PINK);
    px_rect(r, 121, 15, 2, 1, KZ_PETAL_PINK);
    px_rect(r, 115, 16, 8, 2, KZ_PETAL_PINK);
    px_rect(r, 117, 18, 4, 1, KZ_PETAL_PINK);
    px_rect(r, 118, 19, 2, 1, KZ_PETAL_PINK);

    /* ---- counter along the right ---- */
    float cx0 = 150;
    px_rect(r, cx0, 60, KZ_W - cx0, 44, COUNTER);
    px_rect(r, cx0, 60, KZ_W - cx0, 3, COUNTER2);
    px_rect(r, cx0, 60, 2, 44, COUNTER2);
    /* treat jars on the counter */
    for (int j = 0; j < 3; j++) {
        float jx = cx0 + 8 + j * 18;
        px_rect(r, jx, 50, 10, 12, JAR);
        px_rect(r, jx + 1, 51, 8, 2, KZ_CLOUD);         /* shine */
        px_rect(r, jx + 2, 55, 6, 6, rgb(0xF5,0xC0,0x9E)); /* treats inside */
        px_rect(r, jx, 48, 10, 2, COUNTER2);            /* lid */
    }

    /* ---- a couple of little round tables ---- */
    /* table 1 */
    px_rect(r, 30, 92, 24, 4, COUNTER);
    px_rect(r, 40, 96, 4, 10, COUNTER2);
    /* a cup with curling steam */
    px_rect(r, 36, 88, 6, 4, KZ_CLOUD);
    px_rect(r, 42, 89, 1, 2, KZ_CLOUD);   /* handle */
    for (int s = 0; s < 3; s++) {
        float t = (float)((frame + (Uint64)(s * 30)) % 90);
        float sy = 88 - t * 0.15f;
        float sx = 38 + sinf(t * 0.06f + s) * 2.0f;
        Uint8 a = (Uint8)((1.0f - t / 90.0f) * 120.0f);
        px_rect_a(r, sx, sy, 1, 1, KZ_CLOUD, a);
    }
    /* table 2 */
    px_rect(r, 78, 100, 22, 4, COUNTER);
    px_rect(r, 87, 104, 4, 8, COUNTER2);

    /* ---- floor cushions where cats socialize ---- */
    px_rect(r, 40, 130, 20, 9, CUSHION);
    px_rect(r, 42, 129, 16, 2, rgb(0xE0,0xD2,0xF0));
    px_rect(r, 100, 138, 20, 9, KZ_PETAL_PINK);
    px_rect(r, 102, 137, 16, 2, rgb(0xF5,0xDC,0xE6));

    /* ---- a hanging plant, gently swaying ---- */
    float sway = sinf((float)frame * 0.03f) * 1.5f;
    px_rect(r, 20, 0, 4, 10, COUNTER2);            /* hanger */
    px_rect(r, 14 + sway, 10, 16, 6, KZ_MINT);     /* leaves */
    px_rect(r, 12 + sway, 14, 8, 8, rgb(0xB4,0xDE,0xC6));
    px_rect(r, 24 + sway, 14, 8, 6, rgb(0xB4,0xDE,0xC6));
}