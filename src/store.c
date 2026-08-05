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
    DECOR_BOOKSHELF, DECOR_CLOCK, DECOR_BEANBAG, DECOR_POST,
    DECOR_PLANT2, DECOR_TABLE,
};
#define SHELF_COUNT ((int)(sizeof SHELF / sizeof SHELF[0]))

/* grid layout: 4 columns x 2 rows = 8 items per page, with roomy cells so
 * icons, names, and prices never overflow */
#define COLS 4
#define ROWS 2
#define PER_PAGE (COLS * ROWS)
#define CELL_W 54
#define CELL_H 44
#define GRID_X0 10
#define GRID_Y0 40

static void cell_xy(int i, float *x, float *y) {
    int slot = i % PER_PAGE;   /* position within the page */
    *x = GRID_X0 + (slot % COLS) * (CELL_W + 3);
    *y = GRID_Y0 + (slot / COLS) * (CELL_H + 4);
}

/* the "more items" page button, bottom-right */
#define PAGE_BTN_X 150
#define PAGE_BTN_Y (KZ_H - 16)
#define PAGE_BTN_W 84
#define PAGE_BTN_H 14

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
        case DECOR_BOOKSHELF:
            px_rect(r, x + 1, y, 10, 11, rgb(0xB0, 0x8E, 0x76));
            px_rect(r, x + 2, y + 1, 2, 3, KZ_PETAL_PINK);
            px_rect(r, x + 5, y + 1, 2, 3, KZ_MINT);
            px_rect(r, x + 2, y + 6, 2, 3, KZ_BUTTER);
            px_rect(r, x + 5, y + 6, 2, 3, KZ_LAVENDER);
            break;
        case DECOR_CLOCK:
            px_rect(r, x + 2, y + 1, 8, 8, rgb(0xB0, 0x92, 0xCE));
            px_rect(r, x + 3, y + 2, 6, 6, KZ_CLOUD);
            px_rect(r, x + 5, y + 3, 1, 3, KZ_COCOA);
            break;
        case DECOR_BEANBAG:
            px_rect(r, x + 1, y + 3, 10, 6, KZ_PETAL_PINK);
            px_rect(r, x + 2, y + 2, 8, 2, KZ_PETAL_PINK);
            break;
        case DECOR_POST:
            px_rect(r, x + 4, y, 3, 10, rgb(0xD8, 0xC0, 0x9A));
            px_rect(r, x + 2, y + 9, 7, 2, rgb(0xC8, 0xA6, 0x8E));
            px_rect(r, x + 3, y - 1, 5, 2, KZ_MINT);
            break;
        case DECOR_PLANT2:
            px_rect(r, x + 4, y + 6, 4, 5, rgb(0xC8, 0x8E, 0x6E));
            px_rect(r, x + 5, y, 2, 7, rgb(0x7A, 0xA0, 0x62));
            px_rect(r, x + 2, y, 4, 3, rgb(0x8F, 0xC0, 0x7A));
            px_rect(r, x + 6, y - 1, 4, 3, rgb(0x9C, 0xC6, 0x8E));
            break;
        case DECOR_TABLE:
            px_rect(r, x + 1, y + 3, 10, 2, rgb(0xC8, 0xA6, 0x8E));
            px_rect(r, x + 2, y + 5, 2, 5, rgb(0xA8, 0x86, 0x6E));
            px_rect(r, x + 8, y + 5, 2, 5, rgb(0xA8, 0x86, 0x6E));
            px_rect(r, x + 5, y, 2, 3, KZ_MINT);
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
            px_rect(r, x + 3, y + 2, 6, 8, KZ_SKY_WASH);
            px_rect(r, x + 3, y + 2, 6, 2, KZ_MINT);
            break;
        case FOOD_TUNA:   /* a can */
            px_rect(r, x + 2, y + 3, 8, 6, rgb(0xC8, 0xC8, 0xD4));
            px_rect(r, x + 2, y + 3, 8, 1, rgb(0xE4, 0xE4, 0xEC));
            px_rect(r, x + 4, y + 5, 4, 2, rgb(0xE8, 0xA8, 0x88));  /* fish */
            break;
        case FOOD_CATNIP: /* a leafy sprig */
            px_rect(r, x + 4, y + 4, 4, 6, rgb(0x8F, 0xC0, 0x7A));
            px_rect(r, x + 2, y + 3, 3, 3, rgb(0x9C, 0xC6, 0x8E));
            px_rect(r, x + 7, y + 3, 3, 3, rgb(0x9C, 0xC6, 0x8E));
            break;
        case FOOD_SALMON: /* a pink fillet */
            px_rect(r, x + 2, y + 4, 8, 4, rgb(0xE8, 0x9A, 0x8A));
            px_rect(r, x + 2, y + 4, 8, 1, rgb(0xF2, 0xB4, 0xA4));
            px_rect(r, x + 3, y + 5, 1, 2, KZ_CLOUD);              /* stripe */
            px_rect(r, x + 6, y + 5, 1, 2, KZ_CLOUD);
            break;
        case FOOD_JERKY:  /* a chewy strip */
        default:
            px_rect(r, x + 3, y + 2, 4, 8, rgb(0xA8, 0x6E, 0x4E));
            px_rect(r, x + 3, y + 2, 4, 1, rgb(0xC0, 0x8E, 0x6E));
            px_rect(r, x + 4, y + 4, 2, 1, rgb(0x8A, 0x56, 0x3E));
            px_rect(r, x + 4, y + 7, 2, 1, rgb(0x8A, 0x56, 0x3E));
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

/* how many items each floor shows */
static int floor_item_count(StoreFloor floor) {
    return (floor == STORE_FURNITURE) ? SHELF_COUNT : (int)FOOD_COUNT;
}

int store_page_count(StoreFloor floor) {
    int n = floor_item_count(floor);
    int pages = (n + PER_PAGE - 1) / PER_PAGE;
    return pages < 1 ? 1 : pages;
}

/* draw the "More >" page button (only when there's more than one page) */
static void draw_page_button(SDL_Renderer *r, int page, int pages) {
    if (pages <= 1) return;
    px_rect(r, PAGE_BTN_X, PAGE_BTN_Y, PAGE_BTN_W, PAGE_BTN_H, KZ_BUTTER);
    px_rect(r, PAGE_BTN_X, PAGE_BTN_Y, PAGE_BTN_W, 1, KZ_COCOA);
    px_rect(r, PAGE_BTN_X, PAGE_BTN_Y + PAGE_BTN_H - 1, PAGE_BTN_W, 1, KZ_COCOA);
    px_rect(r, PAGE_BTN_X, PAGE_BTN_Y, 1, PAGE_BTN_H, KZ_COCOA);
    px_rect(r, PAGE_BTN_X + PAGE_BTN_W - 1, PAGE_BTN_Y, 1, PAGE_BTN_H, KZ_COCOA);
    char lbl[24];
    SDL_snprintf(lbl, sizeof lbl, "More  (%d/%d) >", page + 1, pages);
    text_draw_centered(r, lbl, PAGE_BTN_X + PAGE_BTN_W / 2.0f,
                       PAGE_BTN_Y + 4, KZ_COCOA);
}

void store_draw(SDL_Renderer *r, StoreFloor floor, int page, const Decor *decor,
                const Pantry *pantry, Uint64 frame) {
    (void)frame;
    store_backdrop(r, floor);

    int pages = store_page_count(floor);
    if (page < 0 || page >= pages) page = 0;
    int start = page * PER_PAGE;
    int end = start + PER_PAGE;
    int n = floor_item_count(floor);
    if (end > n) end = n;

    /* coin balance, top-right corner under the toggle */
    char coins[24];
    SDL_snprintf(coins, sizeof coins, "coins: %u", (unsigned)pantry->coins);
    text_draw(r, coins, KZ_W - 66, 30, rgb(0x9A, 0x7A, 0x5A));

    if (floor == STORE_FURNITURE) {
        for (int i = start; i < end; i++) {
            float x, y; cell_xy(i, &x, &y);
            DecorKind k = SHELF[i];
            bool owned = decor->items[k].owned;
            /* shelf cell */
            px_rect(r, x, y, CELL_W, CELL_H, KZ_CLOUD);
            px_rect(r, x, y, CELL_W, 1, KZ_COCOA);
            px_rect(r, x, y + CELL_H - 1, CELL_W, 1, KZ_COCOA);
            px_rect(r, x, y, 1, CELL_H, KZ_COCOA);
            px_rect(r, x + CELL_W - 1, y, 1, CELL_H, KZ_COCOA);
            /* icon centered near the top */
            decor_icon(r, k, x + CELL_W / 2 - 6, y + 5);
            /* name */
            text_draw_centered(r, decor_info(k)->name, x + CELL_W / 2.0f,
                               y + 22, KZ_COCOA);
            /* price or owned */
            if (owned) {
                text_draw_centered(r, "owned", x + CELL_W / 2.0f, y + 32,
                                   rgb(0x6A, 0xA0, 0x7A));
            } else {
                char pr[16];
                SDL_snprintf(pr, sizeof pr, "%d coins", decor_price(k));
                bool afford = pantry->coins >= (Uint16)decor_price(k);
                text_draw_centered(r, pr, x + CELL_W / 2.0f, y + 32,
                                   afford ? rgb(0x9A, 0x7A, 0x5A)
                                          : rgb(0xC8, 0x9A, 0x9A));
            }
        }
    } else {
        for (int i = start; i < end; i++) {
            float x, y; cell_xy(i, &x, &y);
            px_rect(r, x, y, CELL_W, CELL_H, KZ_CLOUD);
            px_rect(r, x, y, CELL_W, 1, KZ_COCOA);
            px_rect(r, x, y + CELL_H - 1, CELL_W, 1, KZ_COCOA);
            px_rect(r, x, y, 1, CELL_H, KZ_COCOA);
            px_rect(r, x + CELL_W - 1, y, 1, CELL_H, KZ_COCOA);
            food_icon(r, (FoodKind)i, x + CELL_W / 2 - 6, y + 4);
            text_draw_centered(r, food_name((FoodKind)i), x + CELL_W / 2.0f,
                               y + 18, KZ_COCOA);
            char have[16];
            SDL_snprintf(have, sizeof have, "x%u", (unsigned)pantry->stock[i]);
            text_draw_centered(r, have, x + CELL_W / 2.0f, y + 26,
                               rgb(0x9A, 0x7A, 0x5A));
            char pr[16];
            SDL_snprintf(pr, sizeof pr, "%d coins", food_price((FoodKind)i));
            bool afford = pantry->coins >= (Uint16)food_price((FoodKind)i);
            px_rect(r, x + 6, y + 34, CELL_W - 12, 8,
                    afford ? KZ_MINT : rgb(0xE0, 0xD0, 0xD0));
            text_draw_centered(r, pr, x + CELL_W / 2.0f, y + 35, KZ_COCOA);
        }
    }

    draw_page_button(r, page, pages);
    text_draw(r, "tap to buy - travel to leave", 6, KZ_H - 9,
              rgb(0x9A, 0x7A, 0x5A));
}

StoreTap store_hit(StoreFloor floor, int page, float px_, float py_) {
    StoreTap t = { STORE_TAP_NONE, -1 };
    int pages = store_page_count(floor);
    if (page < 0 || page >= pages) page = 0;

    /* floor toggle */
    if (px_ >= FLOOR_BTN_X && px_ <= FLOOR_BTN_X + FLOOR_BTN_W
        && py_ >= FLOOR_BTN_Y && py_ <= FLOOR_BTN_Y + FLOOR_BTN_H) {
        t.kind = STORE_TAP_SWITCH_FLOOR;
        return t;
    }
    /* page button */
    if (pages > 1
        && px_ >= PAGE_BTN_X && px_ <= PAGE_BTN_X + PAGE_BTN_W
        && py_ >= PAGE_BTN_Y && py_ <= PAGE_BTN_Y + PAGE_BTN_H) {
        t.kind = STORE_TAP_NEXT_PAGE;
        return t;
    }

    int start = page * PER_PAGE;
    int end = start + PER_PAGE;
    int n = floor_item_count(floor);
    if (end > n) end = n;
    for (int i = start; i < end; i++) {
        float x, y; cell_xy(i, &x, &y);
        if (px_ >= x && px_ <= x + CELL_W && py_ >= y && py_ <= y + CELL_H) {
            if (floor == STORE_FURNITURE) {
                t.kind = STORE_TAP_BUY_DECOR;
                t.index = (int)SHELF[i];
            } else {
                t.kind = STORE_TAP_BUY_FOOD;
                t.index = i;
            }
            return t;
        }
    }
    return t;
}