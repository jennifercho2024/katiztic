/* market.c — see market.h. A cozy flea market of striped stalls. */
#include "market.h"
#include "render.h"
#include "palette.h"
#include "text.h"
#include <math.h>

/* Four stalls across the lower half of the screen. */
#define STALL_W   52
#define STALL_TOP 62
#define STALL_H   72
static float stall_x(int i) { return 8.0f + i * (STALL_W + 4); }

/* a little icon for each food, centered in a stall's counter */
static void food_icon(SDL_Renderer *r, FoodKind f, float x, float y) {
    switch (f) {
        case FOOD_KIBBLE: {
            Color bowl = rgb(0xC8, 0x9C, 0xB0), bit = rgb(0xB0, 0x86, 0x62);
            px_rect(r, x, y + 6, 18, 6, bowl);
            px_rect(r, x + 1, y + 4, 16, 3, rgb(0xE0, 0xBC, 0xCC));
            px_rect(r, x + 3, y + 3, 3, 2, bit);
            px_rect(r, x + 8, y + 2, 3, 3, bit);
            px_rect(r, x + 12, y + 3, 3, 2, bit);
            break;
        }
        case FOOD_MILK: {
            Color carton = KZ_CLOUD;
            px_rect(r, x + 4, y + 2, 10, 12, carton);
            px_rect(r, x + 4, y + 2, 10, 1, rgb(0xC8, 0xD8, 0xE8));
            px_rect(r, x + 6, y - 1, 6, 3, carton);         /* spout */
            px_rect(r, x + 6, y + 6, 6, 3, rgb(0x9C, 0xC0, 0xD8)); /* label */
            break;
        }
        case FOOD_TREAT: {
            Color t = rgb(0xE8, 0xA6, 0x8B);
            px_rect(r, x + 3, y + 4, 5, 5, t);              /* fish-shaped */
            px_rect(r, x + 8, y + 3, 4, 7, t);
            px_rect(r, x + 12, y + 5, 3, 3, t);
            px_rect(r, x + 4, y + 6, 1, 1, KZ_CLOUD);
            break;
        }
        case FOOD_WATER:
        default: {
            Color w = rgb(0xAF, 0xD6, 0xEC);
            px_rect(r, x + 5, y + 2, 8, 12, rgb(0xD6, 0xEC, 0xF6)); /* glass */
            px_rect(r, x + 5, y + 6, 8, 8, w);              /* water level */
            px_rect(r, x + 6, y + 7, 2, 5, rgb(0xC6, 0xE6, 0xF4));
            break;
        }
    }
}

void market_draw(SDL_Renderer *r, const Pantry *p, Uint64 frame) {
    /* ---- cobbled market square ---- */
    px_rect(r, 0, 0, KZ_W, 40, rgb(0xD9, 0xEC, 0xF2));         /* sky */
    px_rect(r, 0, 40, KZ_W, KZ_H - 40, rgb(0xE2, 0xD6, 0xC6)); /* ground */
    for (int cy = 0; cy < 5; cy++)                             /* cobbles */
        for (int cx = 0; cx < 14; cx++) {
            float ox = (cy % 2) ? 9.0f : 0.0f;
            px_rect(r, cx * 18.0f + ox, 44.0f + cy * 16.0f, 12, 5,
                    rgb(0xD2, 0xC4, 0xB2));
        }

    /* ---- little bunting strung across the top ---- */
    for (int i = 0; i < 12; i++) {
        Color flag = (i % 3 == 0) ? KZ_PETAL_PINK
                   : (i % 3 == 1) ? KZ_BUTTER : KZ_MINT;
        float fx = 6.0f + i * 20.0f;
        float dip = sinf((float)i * 0.9f + (float)frame * 0.03f) * 2.0f;
        px_rect(r, fx, 6 + dip, 6, 5, flag);
        px_rect(r, fx + 1, 11 + dip, 4, 2, flag);
    }

    text_draw(r, "Flea Market", 8, 16, KZ_COCOA);

    /* ---- the four stalls ---- */
    for (int i = 0; i < FOOD_COUNT; i++) {
        float x = stall_x(i);
        /* striped awning */
        for (int s = 0; s < STALL_W / 6; s++) {
            Color str = (s % 2) ? KZ_PETAL_PINK : KZ_CLOUD;
            px_rect(r, x + s * 6, STALL_TOP, 6, 8, str);
        }
        px_rect(r, x, STALL_TOP + 8, STALL_W, 1, KZ_COCOA);
        /* awning scallops */
        for (int s = 0; s < STALL_W / 6; s++)
            px_rect(r, x + s * 6 + 1, STALL_TOP + 8, 4, 2,
                    (s % 2) ? KZ_PETAL_PINK : KZ_CLOUD);
        /* posts */
        px_rect(r, x + 1, STALL_TOP + 8, 2, STALL_H - 20, rgb(0xB0, 0x94, 0x84));
        px_rect(r, x + STALL_W - 3, STALL_TOP + 8, 2, STALL_H - 20,
                rgb(0xB0, 0x94, 0x84));
        /* counter */
        float cy = STALL_TOP + STALL_H - 24;
        px_rect(r, x, cy, STALL_W, 12, rgb(0xC6, 0xA6, 0x8E));
        px_rect(r, x, cy, STALL_W, 2, rgb(0xD8, 0xBC, 0xA6));

        /* the ware on the counter */
        food_icon(r, (FoodKind)i, x + STALL_W / 2 - 9, cy - 12);

        /* name + price + how many you own */
        text_draw_centered(r, food_name((FoodKind)i), x + STALL_W / 2.0f,
                           cy + 2, KZ_COCOA);
        char price[16];
        SDL_snprintf(price, sizeof price, "%dc", food_price((FoodKind)i));
        text_draw_centered(r, price, x + STALL_W / 2.0f, cy + 14,
                           rgb(0x9A, 0x7A, 0x5A));
        char have[16];
        SDL_snprintf(have, sizeof have, "have %u",
                     (unsigned)p->stock[i]);
        text_draw_centered(r, have, x + STALL_W / 2.0f, cy + 22, KZ_COCOA);

        /* a little "buy" chip */
        float by = cy + 30;
        bool afford = p->coins >= food_price((FoodKind)i);
        px_rect(r, x + 10, by, STALL_W - 20, 11,
                afford ? KZ_MINT : rgb(0xD2, 0xC8, 0xCC));
        px_rect(r, x + 10, by, STALL_W - 20, 1, KZ_COCOA);
        px_rect(r, x + 10, by + 10, STALL_W - 20, 1, KZ_COCOA);
        text_draw_centered(r, "buy", x + STALL_W / 2.0f, by + 3, KZ_COCOA);
    }

    /* ---- coin purse, top-right ---- */
    char purse[24];
    SDL_snprintf(purse, sizeof purse, "%u coins", (unsigned)p->coins);
    float pw = text_width(purse) + 18;
    float px0 = KZ_W - pw - 6, py0 = 6;
    px_rect(r, px0, py0, pw, 13, KZ_CLOUD);
    px_rect(r, px0, py0, pw, 1, KZ_COCOA);
    px_rect(r, px0, py0 + 12, pw, 1, KZ_COCOA);
    px_rect(r, px0, py0, 1, 13, KZ_COCOA);
    px_rect(r, px0 + pw - 1, py0, 1, 13, KZ_COCOA);
    /* a little coin */
    px_rect(r, px0 + 4, py0 + 3, 7, 7, rgb(0xF2, 0xD0, 0x7A));
    px_rect(r, px0 + 6, py0 + 5, 3, 3, rgb(0xE0, 0xB0, 0x40));
    text_draw(r, purse, px0 + 13, py0 + 4, KZ_COCOA);

    /* hint */
    text_draw(r, "tap a stall to buy  -  travel to leave", 6, KZ_H - 9,
              KZ_COCOA);
}

int market_hit(float px, float py) {
    for (int i = 0; i < FOOD_COUNT; i++) {
        float x = stall_x(i);
        /* the whole stall column is tappable (awning down through buy chip) */
        if (px >= x && px <= x + STALL_W
            && py >= STALL_TOP && py <= STALL_TOP + STALL_H)
            return i;
    }
    return -1;
}