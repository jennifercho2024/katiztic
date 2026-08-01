/* ui.c — see ui.h. The status panel, drawn from primitives. */
#include "ui.h"
#include "render.h"
#include "palette.h"

/* ---- tiny icons, each ~5x5, drawn at (x,y) ---- */

static void icon_heart(SDL_Renderer *r, float x, float y, Color c) {
    px_rect(r, x,     y,     2, 1, c);
    px_rect(r, x + 3, y,     2, 1, c);
    px_rect(r, x,     y + 1, 5, 2, c);
    px_rect(r, x + 1, y + 3, 3, 1, c);
    px_rect(r, x + 2, y + 4, 1, 1, c);
}

static void icon_smile(SDL_Renderer *r, float x, float y, Color c) {
    px_rect(r, x + 1, y + 1, 1, 1, c);   /* eyes */
    px_rect(r, x + 3, y + 1, 1, 1, c);
    px_rect(r, x,     y + 3, 1, 1, c);   /* smile corners + curve */
    px_rect(r, x + 4, y + 3, 1, 1, c);
    px_rect(r, x + 1, y + 4, 3, 1, c);
}

static void icon_leaf(SDL_Renderer *r, float x, float y, Color c) {
    px_rect(r, x + 3, y,     2, 1, c);
    px_rect(r, x + 2, y + 1, 2, 1, c);
    px_rect(r, x + 1, y + 2, 2, 1, c);
    px_rect(r, x,     y + 3, 2, 2, c);
}

static void icon_star(SDL_Renderer *r, float x, float y, Color c) {
    px_rect(r, x + 2, y,     1, 1, c);
    px_rect(r, x,     y + 2, 5, 1, c);
    px_rect(r, x + 1, y + 3, 3, 1, c);
    px_rect(r, x + 1, y + 4, 1, 1, c);
    px_rect(r, x + 3, y + 4, 1, 1, c);
}

/* A single stat row: icon, then a bar filled proportional to value/100. */
static void stat_row(SDL_Renderer *r, float x, float y,
                     void (*icon)(SDL_Renderer *, float, float, Color),
                     Color fill, Uint8 value) {
    icon(r, x, y, fill);

    float bx = x + 8, bw = 46, bh = 5;
    /* trough */
    px_rect(r, bx, y, bw, bh, KZ_CLOUD);
    px_rect(r, bx, y, bw, 1, rgba(0x8B,0x7B,0x8B,60)); /* soft top shade */
    /* fill */
    float fw = bw * ((float)value / (float)KZ_STAT_MAX);
    if (fw > 0) px_rect(r, bx, y, fw, bh, fill);
    /* 1px mauve frame */
    px_rect(r, bx,          y,          bw, 1, KZ_COCOA);
    px_rect(r, bx,          y + bh - 1, bw, 1, KZ_COCOA);
    px_rect(r, bx,          y,          1,  bh, KZ_COCOA);
    px_rect(r, bx + bw - 1, y,          1,  bh, KZ_COCOA);
}

void ui_draw_panel(SDL_Renderer *r, const Stats *s, float x, float y) {
    float w = 62, h = 44;

    /* Panel: cream fill, 1px mauve border, soft drop shadow. */
    px_rect_a(r, x + 2, y + 2, w, h, KZ_COCOA, 40);       /* shadow */
    px_rect(r, x, y, w, h, KZ_CLOUD);                      /* fill   */
    px_rect(r, x,         y,         w, 1, KZ_COCOA);      /* border */
    px_rect(r, x,         y + h - 1, w, 1, KZ_COCOA);
    px_rect(r, x,         y,         1, h, KZ_COCOA);
    px_rect(r, x + w - 1, y,         1, h, KZ_COCOA);

    float rx = x + 5, ry = y + 5;
    stat_row(r, rx, ry,      icon_heart, KZ_PETAL_PINK, s->bond);
    stat_row(r, rx, ry + 9,  icon_smile, KZ_BUTTER,     s->mood);
    stat_row(r, rx, ry + 18, icon_leaf,  KZ_MINT,       s->energy);
    stat_row(r, rx, ry + 27, icon_star,  KZ_LAVENDER,   s->growth);
}

/* Small centered hint at the bottom: three soft dots as a placeholder legend
 * for feed / groom / pet, colored to match their bars. (A real font comes in
 * a later polish pass; for now the colors teach which key does what.) */
void ui_draw_hint(SDL_Renderer *r) {
    float cx = KZ_W / 2.0f, y = KZ_H - 10;
    float w = 66, x = cx - w / 2;

    px_rect(r, x, y, w, 8, KZ_CLOUD);
    px_rect(r, x,         y,     w, 1, KZ_COCOA);
    px_rect(r, x,         y + 7, w, 1, KZ_COCOA);
    px_rect(r, x,         y,     1, 8, KZ_COCOA);
    px_rect(r, x + w - 1, y,     1, 8, KZ_COCOA);

    /* F feed (mint), G groom (pink), tap = pet (butter) */
    px_rect(r, x + 8,  y + 3, 3, 3, KZ_MINT);
    px_rect(r, x + 30, y + 3, 3, 3, KZ_PETAL_PINK);
    px_rect(r, x + 52, y + 3, 3, 3, KZ_BUTTER);
}