/* ui.c — see ui.h. The status panel, drawn from primitives. */
#include "ui.h"
#include "render.h"
#include "palette.h"
#include "cattype.h"
#include "text.h"
#include "decor.h"

/* ---- button icon glyphs, ~9x9, drawn centered in the button ---- */

static void glyph_house(SDL_Renderer *r, float x, float y, Color c) {
    /* roof */
    px_rect(r, x + 3, y,     3, 1, c);
    px_rect(r, x + 2, y + 1, 5, 1, c);
    px_rect(r, x + 1, y + 2, 7, 1, c);
    px_rect(r, x,     y + 3, 9, 1, c);
    /* walls + door */
    px_rect(r, x + 1, y + 4, 7, 5, c);
    px_rect(r, x + 3, y + 6, 3, 3, KZ_CLOUD);
}

static void glyph_sun(SDL_Renderer *r, float x, float y, Color c) {
    /* rays */
    px_rect(r, x + 4, y,     1, 2, c);
    px_rect(r, x + 4, y + 7, 1, 2, c);
    px_rect(r, x,     y + 4, 2, 1, c);
    px_rect(r, x + 7, y + 4, 2, 1, c);
    /* body */
    px_rect(r, x + 3, y + 3, 3, 3, c);
    px_rect(r, x + 2, y + 4, 5, 1, c);
    px_rect(r, x + 4, y + 2, 1, 5, c);
}

static void glyph_moon(SDL_Renderer *r, float x, float y, Color c) {
    px_rect(r, x + 3, y + 1, 3, 1, c);
    px_rect(r, x + 2, y + 2, 2, 1, c);
    px_rect(r, x + 1, y + 3, 2, 3, c);
    px_rect(r, x + 2, y + 6, 2, 1, c);
    px_rect(r, x + 3, y + 7, 3, 1, c);
}

static void glyph_fish(SDL_Renderer *r, float x, float y, Color c) {
    /* little side-on fish: body + tail */
    px_rect(r, x + 1, y + 3, 6, 3, c);   /* body */
    px_rect(r, x + 2, y + 2, 4, 1, c);
    px_rect(r, x + 2, y + 6, 4, 1, c);
    px_rect(r, x + 7, y + 2, 2, 2, c);   /* tail top */
    px_rect(r, x + 7, y + 5, 2, 2, c);   /* tail bottom */
    px_rect(r, x + 2, y + 4, 1, 1, KZ_CLOUD); /* eye */
}

static void glyph_heart_big(SDL_Renderer *r, float x, float y, Color c) {
    px_rect(r, x + 1, y + 1, 2, 1, c);
    px_rect(r, x + 6, y + 1, 2, 1, c);
    px_rect(r, x,     y + 2, 9, 2, c);
    px_rect(r, x + 1, y + 4, 7, 1, c);
    px_rect(r, x + 2, y + 5, 5, 1, c);
    px_rect(r, x + 3, y + 6, 3, 1, c);
    px_rect(r, x + 4, y + 7, 1, 1, c);
}

static void glyph_chair(SDL_Renderer *r, float x, float y, Color c) {
    /* a little armchair silhouette */
    px_rect(r, x + 1, y,     7, 4, c);   /* back */
    px_rect(r, x,     y + 3, 9, 3, c);   /* seat + arms */
    px_rect(r, x + 1, y + 6, 2, 2, c);   /* legs */
    px_rect(r, x + 6, y + 6, 2, 2, c);
}

bool ui_button_hit(const Button *b, float px_, float py_) {
    return px_ >= b->x && px_ <= b->x + b->w
        && py_ >= b->y && py_ <= b->y + b->h;
}

void ui_button_draw(SDL_Renderer *r, const Button *b, bool pressed) {
    /* soft drop shadow */
    px_rect_a(r, b->x + 1, b->y + 2, b->w, b->h, KZ_COCOA, 40);
    /* fill — a touch darker while pressed, for tap feedback */
    Color fill = pressed ? KZ_LAVENDER : KZ_CLOUD;
    px_rect(r, b->x, b->y, b->w, b->h, fill);
    /* mauve border */
    px_rect(r, b->x,            b->y,            b->w, 1,    KZ_COCOA);
    px_rect(r, b->x,            b->y + b->h - 1, b->w, 1,    KZ_COCOA);
    px_rect(r, b->x,            b->y,            1,    b->h, KZ_COCOA);
    px_rect(r, b->x + b->w - 1, b->y,            1,    b->h, KZ_COCOA);
    /* centered glyph */
    float gx = b->x + (b->w - 9) / 2.0f;
    float gy = b->y + (b->h - 9) / 2.0f;
    switch (b->kind) {
        case KZ_BTN_HOME:    glyph_house(r, gx, gy, KZ_COCOA); break;
        case KZ_BTN_OUT:     glyph_sun(r,   gx, gy, KZ_COCOA); break;
        case KZ_BTN_SLEEP:   glyph_moon(r,  gx, gy, KZ_COCOA); break;
        case KZ_BTN_TREAT:   glyph_fish(r,  gx, gy, rgb(0xE8,0x8B,0x6B)); break;
        case KZ_BTN_FRIENDS: glyph_heart_big(r, gx, gy, KZ_PETAL_PINK); break;
        case KZ_BTN_DECOR:   glyph_chair(r, gx, gy, KZ_COCOA); break;
    }
}

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

/* The name row occupies roughly the top 8px of the panel interior. */
#define PANEL_NAME_H 8

bool ui_name_hit(float panel_x, float panel_y, float px_, float py_) {
    return px_ >= panel_x && px_ <= panel_x + 62
        && py_ >= panel_y && py_ <= panel_y + 3 + PANEL_NAME_H;
}

void ui_draw_panel(SDL_Renderer *r, const OwnedCat *cat, float x, float y,
                   bool editing, const char *edit_buf, Uint64 frame) {
    const Stats *s = &cat->stats;
    float w = 62, h = 54;   /* a little taller now, to fit the name header */

    /* Panel: cream fill, 1px mauve border, soft drop shadow. */
    px_rect_a(r, x + 2, y + 2, w, h, KZ_COCOA, 40);       /* shadow */
    px_rect(r, x, y, w, h, KZ_CLOUD);                      /* fill   */
    px_rect(r, x,         y,         w, 1, KZ_COCOA);      /* border */
    px_rect(r, x,         y + h - 1, w, 1, KZ_COCOA);
    px_rect(r, x,         y,         1, h, KZ_COCOA);
    px_rect(r, x + w - 1, y,         1, h, KZ_COCOA);

    /* Header: the cat's name (or the edit buffer + caret while renaming). */
    if (editing) {
        px_rect(r, x + 2, y + 2, w - 4, PANEL_NAME_H, KZ_PETAL_PINK); /* field */
        float nx = text_draw(r, edit_buf, x + 4, y + 3, KZ_COCOA) + x + 4;
        if ((frame / 20) % 2 == 0)             /* blinking caret */
            px_rect(r, nx, y + 3, 1, 6, KZ_COCOA);
    } else {
        text_draw(r, cat->name, x + 4, y + 3, KZ_COCOA);
    }
    text_draw(r, cattype_name(cat->type), x + 4, y + 10,
              cattype_colors(cat->type).dark);
    px_rect(r, x + 3, y + 17, w - 6, 1, KZ_COCOA);   /* divider */

    float rx = x + 5, ry = y + 21;
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

/* ---- roster strip geometry (shared by draw + hit so they always agree) ---- */
#define RS_SLOT   22          /* portrait slot width/height        */
#define RS_GAP    3           /* space between slots               */
#define RS_Y      (KZ_H - RS_SLOT - 2)   /* top of the strip, bottom-left */
#define RS_X      2

static float rs_slot_x(int i) {
    return (float)RS_X + i * (RS_SLOT + RS_GAP);
}

/* A tiny cat face in the given type color, filling a portrait slot at (x,y). */
static void portrait(SDL_Renderer *r, float x, float y, CatColors col) {
    /* head */
    px_rect(r, x + 4, y + 6, 14, 12, col.body);
    /* ears */
    px_rect(r, x + 4,  y + 2, 4, 5, col.body);
    px_rect(r, x + 14, y + 2, 4, 5, col.body);
    px_rect(r, x + 5,  y + 3, 2, 3, col.ear);
    px_rect(r, x + 15, y + 3, 2, 3, col.ear);
    /* eyes + nose (fixed mauve) */
    px_rect(r, x + 7,  y + 10, 2, 2, KZ_CAT_OUTLINE);
    px_rect(r, x + 13, y + 10, 2, 2, KZ_CAT_OUTLINE);
    px_rect(r, x + 10, y + 13, 2, 1, KZ_CAT_NOSE);
    /* cheeks */
    px_rect(r, x + 5,  y + 12, 2, 1, col.cheek);
    px_rect(r, x + 15, y + 12, 2, 1, col.cheek);
}

void ui_roster_draw(SDL_Renderer *r, const Roster *ro) {
    for (int i = 0; i < ro->count; i++) {
        float x = rs_slot_x(i), y = RS_Y;
        bool active = (i == ro->active);

        /* slot background: cream, with a pink ring when active */
        px_rect_a(r, x + 1, y + 2, RS_SLOT, RS_SLOT, KZ_COCOA, 40); /* shadow */
        px_rect(r, x, y, RS_SLOT, RS_SLOT, KZ_CLOUD);
        Color border = active ? KZ_PETAL_PINK : KZ_COCOA;
        int th = active ? 2 : 1;   /* thicker ring when active */
        for (int t = 0; t < th; t++) {
            px_rect(r, x + t,             y + t,             RS_SLOT - 2*t, 1, border);
            px_rect(r, x + t,             y + RS_SLOT-1 - t, RS_SLOT - 2*t, 1, border);
            px_rect(r, x + t,             y + t,             1, RS_SLOT - 2*t, border);
            px_rect(r, x + RS_SLOT-1 - t, y + t,             1, RS_SLOT - 2*t, border);
        }

        portrait(r, x, y, cattype_colors(ro->cats[i].type));
    }

    /* "+" adopt slot, if there's room */
    if (ro->count < KZ_MAX_CATS) {
        float x = rs_slot_x(ro->count), y = RS_Y;
        px_rect_a(r, x + 1, y + 2, RS_SLOT, RS_SLOT, KZ_COCOA, 40);
        px_rect(r, x, y, RS_SLOT, RS_SLOT, KZ_CLOUD);
        px_rect(r, x, y, RS_SLOT, 1, KZ_COCOA);
        px_rect(r, x, y + RS_SLOT-1, RS_SLOT, 1, KZ_COCOA);
        px_rect(r, x, y, 1, RS_SLOT, KZ_COCOA);
        px_rect(r, x + RS_SLOT-1, y, 1, RS_SLOT, KZ_COCOA);
        /* plus sign in mint */
        px_rect(r, x + RS_SLOT/2 - 4, y + RS_SLOT/2, 8, 2, KZ_MINT);
        px_rect(r, x + RS_SLOT/2, y + RS_SLOT/2 - 4, 2, 8, KZ_MINT);
    }
}

int ui_roster_hit(const Roster *ro, float px_, float py_) {
    if (py_ < RS_Y || py_ > RS_Y + RS_SLOT) return -1;
    for (int i = 0; i < ro->count; i++) {
        float x = rs_slot_x(i);
        if (px_ >= x && px_ <= x + RS_SLOT) return i;
    }
    if (ro->count < KZ_MAX_CATS) {
        float x = rs_slot_x(ro->count);
        if (px_ >= x && px_ <= x + RS_SLOT) return -2;   /* adopt slot */
    }
    return -1;
}

/* ---- encounter UI ---- */

/* Fixed-position buttons. Treat sits bottom-right (clear of the roster strip
 * which is bottom-left); friends button sits top-right under the travel/sleep. */
const Button KZ_TREAT_BUTTON   = { KZ_W - 26, KZ_H - 46, 22, 18, KZ_BTN_TREAT };
const Button KZ_FRIENDS_BUTTON = { KZ_W - 24, 24,        20, 16, KZ_BTN_FRIENDS };

void ui_treat_button_draw(SDL_Renderer *r, bool pressed) {
    ui_button_draw(r, &KZ_TREAT_BUTTON, pressed);
}
bool ui_treat_button_hit(float px_, float py_) {
    return ui_button_hit(&KZ_TREAT_BUTTON, px_, py_);
}
void ui_friends_button_draw(SDL_Renderer *r, bool pressed) {
    ui_button_draw(r, &KZ_FRIENDS_BUTTON, pressed);
}
bool ui_friends_button_hit(float px_, float py_) {
    return ui_button_hit(&KZ_FRIENDS_BUTTON, px_, py_);
}

/* A soft dialogue banner across the bottom-center with one line of text. */
void ui_banner(SDL_Renderer *r, const char *line) {
    float w = 176, h = 16;
    float x = (KZ_W - w) / 2.0f, y = KZ_H - h - 26;

    px_rect_a(r, x + 2, y + 2, w, h, KZ_COCOA, 40);
    px_rect(r, x, y, w, h, KZ_CLOUD);
    px_rect(r, x,         y,         w, 1, KZ_COCOA);
    px_rect(r, x,         y + h - 1, w, 1, KZ_COCOA);
    px_rect(r, x,         y,         1, h, KZ_COCOA);
    px_rect(r, x + w - 1, y,         1, h, KZ_COCOA);

    text_draw_centered(r, line, KZ_W / 2.0f, y + 5, KZ_COCOA);
}

/* The friends-list overlay: a soft panel listing everyone met, trust bars,
 * and a heart on full friends. */
void ui_friends_list(SDL_Renderer *r, const Friends *f) {
    /* dim the scene */
    px_rect_a(r, 0, 0, KZ_W, KZ_H, rgb(0x3B, 0x30, 0x50), 150);

    float w = 180, h = 132;
    float x = (KZ_W - w) / 2.0f, y = (KZ_H - h) / 2.0f;
    px_rect(r, x, y, w, h, KZ_CLOUD);
    px_rect(r, x,         y,         w, 1, KZ_COCOA);
    px_rect(r, x,         y + h - 1, w, 1, KZ_COCOA);
    px_rect(r, x,         y,         1, h, KZ_COCOA);
    px_rect(r, x + w - 1, y,         1, h, KZ_COCOA);

    text_draw(r, "Friends", x + 6, y + 5, KZ_COCOA);
    px_rect(r, x + 5, y + 13, w - 10, 1, KZ_COCOA);

    if (f->count == 0) {
        text_draw(r, "None yet. Take a walk!", x + 6, y + 20, KZ_COCOA);
    } else {
        for (int i = 0; i < f->count && i < 8; i++) {
            const Friend *fr = &f->list[i];
            float ry = y + 18 + i * 13;
            /* little portrait dot in her type color */
            px_rect(r, x + 6, ry, 6, 6, cattype_colors(fr->type).body);
            px_rect(r, x + 6, ry, 6, 1, cattype_colors(fr->type).dark);
            /* name */
            text_draw(r, fr->name, x + 16, ry, KZ_COCOA);
            /* trust bar */
            float bx = x + 96, bw = 60;
            px_rect(r, bx, ry, bw, 5, KZ_CLOUD);
            float fw = bw * ((float)fr->trust / (float)KZ_TRUST_FULL);
            if (fw > 0) px_rect(r, bx, ry, fw, 5, KZ_PETAL_PINK);
            px_rect(r, bx, ry, bw, 1, KZ_COCOA);
            px_rect(r, bx, ry + 4, bw, 1, KZ_COCOA);
            px_rect(r, bx, ry, 1, 5, KZ_COCOA);
            px_rect(r, bx + bw - 1, ry, 1, 5, KZ_COCOA);
            /* heart if fully befriended */
            if (fr->befriended) {
                float hx = bx + bw + 4;
                px_rect(r, hx, ry, 1, 1, KZ_HEART);
                px_rect(r, hx + 2, ry, 1, 1, KZ_HEART);
                px_rect(r, hx, ry + 1, 3, 1, KZ_HEART);
                px_rect(r, hx + 1, ry + 2, 1, 1, KZ_HEART);
            }
        }
    }
    text_draw_centered(r, "tap to close", KZ_W / 2.0f, y + h - 9, KZ_COCOA);
}

/* ---- décor tray ---- */

const Button KZ_DECOR_BUTTON = { KZ_W - 24, 44, 20, 16, KZ_BTN_DECOR };

void ui_decor_button_draw(SDL_Renderer *r, bool pressed) {
    ui_button_draw(r, &KZ_DECOR_BUTTON, pressed);
}
bool ui_decor_button_hit(float px_, float py_) {
    return ui_button_hit(&KZ_DECOR_BUTTON, px_, py_);
}

/* Tray geometry: a strip across the bottom, above the roster strip. */
#define TRAY_H     30
#define TRAY_Y     (KZ_H - TRAY_H - 26)   /* sits above the roster strip */
#define TRAY_SLOT  30

float ui_decor_tray_top(void) { return (float)TRAY_Y; }

static float tray_slot_x(int visible_index) {
    return 6.0f + visible_index * (TRAY_SLOT + 2);
}

void ui_decor_tray(SDL_Renderer *r, const Decor *d, Uint64 frame) {
    /* backing strip */
    px_rect_a(r, 0, TRAY_Y - 2, KZ_W, TRAY_H + 2, KZ_CLOUD, 235);
    px_rect(r, 0, TRAY_Y - 2, KZ_W, 1, KZ_COCOA);

    text_draw(r, "drag to place", 6, TRAY_Y - 10, KZ_COCOA);

    int vis = 0;
    for (int i = 0; i < DECOR_COUNT; i++) {
        if (!d->items[i].owned) continue;
        float sx = tray_slot_x(vis);
        /* slot */
        px_rect(r, sx, (float)TRAY_Y + 2, TRAY_SLOT, TRAY_SLOT - 4, KZ_CLOUD);
        px_rect(r, sx, (float)TRAY_Y + 2, TRAY_SLOT, 1, KZ_COCOA);
        px_rect(r, sx, (float)TRAY_Y + TRAY_SLOT - 3, TRAY_SLOT, 1, KZ_COCOA);
        px_rect(r, sx, (float)TRAY_Y + 2, 1, TRAY_SLOT - 4, KZ_COCOA);
        px_rect(r, sx + TRAY_SLOT - 1, (float)TRAY_Y + 2, 1, TRAY_SLOT - 4, KZ_COCOA);
        /* a small preview of the item, centered-ish in the slot */
        decor_draw_one(r, (DecorKind)i, sx + 8, (float)TRAY_Y + 6, frame);
        /* a check if it's currently placed in the room */
        if (d->items[i].placed) {
            px_rect(r, sx + TRAY_SLOT - 6, (float)TRAY_Y + 3, 3, 1, KZ_MINT);
            px_rect(r, sx + TRAY_SLOT - 4, (float)TRAY_Y + 4, 1, 2, KZ_MINT);
        }
        vis++;
    }
}

int ui_decor_tray_hit(const Decor *d, float px_, float py_) {
    if (py_ < TRAY_Y + 2 || py_ > TRAY_Y + TRAY_SLOT - 2) return -1;
    int vis = 0;
    for (int i = 0; i < DECOR_COUNT; i++) {
        if (!d->items[i].owned) continue;
        float sx = tray_slot_x(vis);
        if (px_ >= sx && px_ <= sx + TRAY_SLOT) return i;
        vis++;
    }
    return -1;
}