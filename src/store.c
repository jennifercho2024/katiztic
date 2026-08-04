/* store.c — see store.h. The department store screen. */
#include "store.h"
#include "render.h"
#include "palette.h"
#include "text.h"
#include "cattype.h"
#include <math.h>

/* which décor kinds are sold on the furniture floor (in display order) */
static const DecorKind SHELF[] = {
    DECOR_PLANT, DECOR_LAMP, DECOR_PICTURE, DECOR_TOWER,
    DECOR_CUSHION, DECOR_RUG2, DECOR_YARN, DECOR_MILK,
};
#define SHELF_COUNT ((int)(sizeof SHELF / sizeof SHELF[0]))

/* grid layout for the furniture shelves: 4 columns, 2 rows */
#define COLS 4
#define CELL_W 52
#define CELL_H 42
#define GRID_X0 12
#define GRID_Y0 40

static void cell_xy(int i, float *x, float *y) {
    *x = GRID_X0 + (i % COLS) * (CELL_W + 4);
    *y = GRID_Y0 + (i / COLS) * (CELL_H + 6);
}

/* the floor toggle button, top-right */
#define FLOOR_BTN_X 150
#define FLOOR_BTN_Y 6
#define FLOOR_BTN_W 84
#define FLOOR_BTN_H 16

/* a tiny preview icon for a piece of furniture */
static void decor_icon(SDL_Renderer *r, DecorKind k, float x, float y) {
    switch (k) {
        case DECOR_PLANT:
            px_rect(r, x + 4, y + 6, 4, 4, rgb(0xC8, 0xA6, 0x8E));  /* pot */
            px_rect(r, x + 3, y, 6, 6, rgb(0x8F, 0xC0, 0x7A));       /* leaves */
            px_rect(r, x + 5, y - 2, 2, 3, rgb(0x9C, 0xC6, 0x8E));
            break;
        case DECOR_LAMP:
            px_rect(r, x + 5, y + 2, 2, 8, rgb(0xB0, 0x92, 0xCE));  /* stem */
            px_rect(r, x + 2, y - 2, 8, 4, KZ_BUTTER);              /* shade */
            break;
        case DECOR_PICTURE:
            px_rect(r, x + 1, y, 10, 9, rgb(0xC8, 0xA6, 0x8E));     /* frame */
            px_rect(r, x + 3, y + 2, 6, 5, KZ_SKY_WASH);           /* scene */
            break;
        case DECOR_TOWER:
            px_rect(r, x + 4, y, 4, 12, rgb(0xC8, 0xA6, 0x8E));    /* post */
            px_rect(r, x + 1, y, 10, 3, rgb(0xB0, 0x92, 0xCE));    /* top */
            px_rect(r, x + 2, y + 6, 8, 3, rgb(0xB0, 0x92, 0xCE)); /* mid */
            break;
        case DECOR_CUSHION:
            px_rect(r, x + 1, y + 5, 10, 5, KZ_PETAL_PINK);
            px_rect(r, x + 2, y + 4, 8, 1, KZ_HEART);
            break;
        case DECOR_RUG2:
            px_rect(r, x, y + 6, 12, 4, KZ_MINT);
            px_rect(r, x + 2, y + 7, 8, 2, KZ_LAVENDER);
            break;
        case DECOR_YARN:
            px_rect(r, x + 3, y + 3, 6, 6, KZ_PETAL_PINK);
            px_rect(r, x + 4, y + 4, 4, 1, KZ_HEART);
            px_rect(r, x + 4, y + 6, 4, 1, KZ_HEART);
            break;
        case DECOR_MILK:
        default:
            px_rect(r, x + 2, y + 5, 8, 4, KZ_CLOUD);   /* saucer */
            px_rect(r, x + 3, y + 4, 6, 2, rgb(0xF3, 0xF0, 0xF6));
            break;
    }
}

static void food_icon(SDL_Renderer *r, FoodKind f, float x, float y) {
    switch (f) {
        case FOOD_KIBBLE:
            px_rect(r, x + 2, y + 4, 8, 5, rgb(0xC8, 0xA6, 0x8E));
            px_rect(r, x + 3, y + 3, 2, 2, rgb(0x9A, 0x7A, 0x5A));
            px_rect(r, x + 6, y + 3, 2, 2, rgb(0x9A, 0x7A, 0x5A));
            break;
        case FOOD_MILK:
            px_rect(r, x + 3, y + 1, 6, 9, KZ_CLOUD);
            px_rect(r, x + 3, y + 1, 6, 2, KZ_SKY_WASH);
            break;
        case FOOD_TREAT:
            px_rect(r, x + 3, y + 3, 6, 5, KZ_PETAL_PINK);
            px_rect(r, x + 4, y + 2, 4, 2, KZ_HEART);
            break;
        case FOOD_WATER:
        default:
            px_rect(r, x + 3, y + 2, 6, 8, KZ_SKY_WASH);
            px_rect(r, x + 3, y + 2, 6, 2, KZ_MINT);
            break;
    }
}

static void store_backdrop(SDL_Renderer *r, StoreFloor floor) {
    /* warm shop interior */
    px_rect(r, 0, 0, KZ_W, KZ_H, rgb(0xF4, 0xEC, 0xE4));
    /* wood floor band */
    px_rect(r, 0, KZ_H - 20, KZ_W, 20, rgb(0xD8, 0xC0, 0xA4));
    px_rect(r, 0, KZ_H - 20, KZ_W, 1, rgb(0xC0, 0xA6, 0x88));
    /* header bar */
    px_rect(r, 0, 0, KZ_W, 26, rgb(0xB0, 0x92, 0xCE));
    px_rect(r, 0, 26, KZ_W, 1, KZ_COCOA);
    text_draw(r, "Department Store", 6, 4, KZ_CLOUD);
    text_draw(r, floor == STORE_FURNITURE ? "Furniture" : "Supplies",
              6, 14, KZ_BUTTER);

    /* floor toggle button */
    px_rect(r, FLOOR_BTN_X, FLOOR_BTN_Y, FLOOR_BTN_W, FLOOR_BTN_H, KZ_CLOUD);
    px_rect(r, FLOOR_BTN_X, FLOOR_BTN_Y, FLOOR_BTN_W, 1, KZ_COCOA);
    px_rect(r, FLOOR_BTN_X, FLOOR_BTN_Y + FLOOR_BTN_H - 1, FLOOR_BTN_W, 1, KZ_COCOA);
    px_rect(r, FLOOR_BTN_X, FLOOR_BTN_Y, 1, FLOOR_BTN_H, KZ_COCOA);
    px_rect(r, FLOOR_BTN_X + FLOOR_BTN_W - 1, FLOOR_BTN_Y, 1, FLOOR_BTN_H, KZ_COCOA);
    text_draw_centered(r,
        floor == STORE_FURNITURE ? "to Supplies >" : "< to Furniture",
        FLOOR_BTN_X + FLOOR_BTN_W / 2.0f, FLOOR_BTN_Y + 5, KZ_COCOA);
}

void store_draw(SDL_Renderer *r, StoreFloor floor, const Decor *decor,
                const Pantry *pantry, Uint64 frame) {
    (void)frame;
    store_backdrop(r, floor);

    /* coin balance, top-right corner under the toggle */
    char coins[24];
    SDL_snprintf(coins, sizeof coins, "coins: %u", (unsigned)pantry->coins);
    text_draw(r, coins, KZ_W - 66, 30, rgb(0x9A, 0x7A, 0x5A));

    if (floor == STORE_FURNITURE) {
        for (int i = 0; i < SHELF_COUNT; i++) {
            float x, y; cell_xy(i, &x, &y);
            DecorKind k = SHELF[i];
            bool owned = decor->items[k].owned;
            /* shelf cell */
            px_rect(r, x, y, CELL_W, CELL_H, KZ_CLOUD);
            px_rect(r, x, y, CELL_W, 1, KZ_COCOA);
            px_rect(r, x, y + CELL_H - 1, CELL_W, 1, KZ_COCOA);
            px_rect(r, x, y, 1, CELL_H, KZ_COCOA);
            px_rect(r, x + CELL_W - 1, y, 1, CELL_H, KZ_COCOA);
            /* icon */
            decor_icon(r, k, x + CELL_W / 2 - 6, y + 6);
            /* name */
            text_draw_centered(r, decor_info(k)->name, x + CELL_W / 2.0f,
                               y + 22, KZ_COCOA);
            /* price or owned */
            if (owned) {
                text_draw_centered(r, "owned", x + CELL_W / 2.0f, y + 31,
                                   rgb(0x6A, 0xA0, 0x7A));
            } else {
                char pr[16];
                SDL_snprintf(pr, sizeof pr, "%d coins", decor_price(k));
                bool afford = pantry->coins >= (Uint16)decor_price(k);
                text_draw_centered(r, pr, x + CELL_W / 2.0f, y + 31,
                                   afford ? rgb(0x9A, 0x7A, 0x5A)
                                          : rgb(0xC8, 0x9A, 0x9A));
            }
        }
    } else {
        /* supplies floor: 4 food items in a row */
        for (int i = 0; i < FOOD_COUNT; i++) {
            float x = 14 + i * 56, y = 46;
            px_rect(r, x, y, 50, 60, KZ_CLOUD);
            px_rect(r, x, y, 50, 1, KZ_COCOA);
            px_rect(r, x, y + 59, 50, 1, KZ_COCOA);
            px_rect(r, x, y, 1, 60, KZ_COCOA);
            px_rect(r, x + 49, y, 1, 60, KZ_COCOA);
            food_icon(r, (FoodKind)i, x + 19, y + 6);
            text_draw_centered(r, food_name((FoodKind)i), x + 25, y + 22,
                               KZ_COCOA);
            char have[16];
            SDL_snprintf(have, sizeof have, "have %u",
                         (unsigned)pantry->stock[i]);
            text_draw_centered(r, have, x + 25, y + 32, rgb(0x9A, 0x7A, 0x5A));
            char pr[16];
            SDL_snprintf(pr, sizeof pr, "%d coins", food_price((FoodKind)i));
            bool afford = pantry->coins >= (Uint16)food_price((FoodKind)i);
            /* buy chip */
            px_rect(r, x + 8, y + 44, 34, 11, afford ? KZ_MINT
                                                     : rgb(0xE0, 0xD0, 0xD0));
            px_rect(r, x + 8, y + 44, 34, 1, KZ_COCOA);
            px_rect(r, x + 8, y + 54, 34, 1, KZ_COCOA);
            text_draw_centered(r, pr, x + 25, y + 46, KZ_COCOA);
        }
    }

    text_draw(r, "tap to buy  -  travel to leave", 6, KZ_H - 9,
              rgb(0x9A, 0x7A, 0x5A));
}

StoreTap store_hit(StoreFloor floor, float px_, float py_) {
    StoreTap t = { STORE_TAP_NONE, -1 };
    /* floor toggle */
    if (px_ >= FLOOR_BTN_X && px_ <= FLOOR_BTN_X + FLOOR_BTN_W
        && py_ >= FLOOR_BTN_Y && py_ <= FLOOR_BTN_Y + FLOOR_BTN_H) {
        t.kind = STORE_TAP_SWITCH_FLOOR;
        return t;
    }
    if (floor == STORE_FURNITURE) {
        for (int i = 0; i < SHELF_COUNT; i++) {
            float x, y; cell_xy(i, &x, &y);
            if (px_ >= x && px_ <= x + CELL_W
                && py_ >= y && py_ <= y + CELL_H) {
                t.kind = STORE_TAP_BUY_DECOR;
                t.index = (int)SHELF[i];
                return t;
            }
        }
    } else {
        for (int i = 0; i < FOOD_COUNT; i++) {
            float x = 14 + i * 56, y = 46;
            if (px_ >= x && px_ <= x + 50 && py_ >= y && py_ <= y + 60) {
                t.kind = STORE_TAP_BUY_FOOD;
                t.index = i;
                return t;
            }
        }
    }
    return t;
}