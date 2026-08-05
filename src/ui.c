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

/* a compass: a round dial with a red/white needle pointing NE */
static void glyph_map(SDL_Renderer *r, float x, float y, Color c) {
    /* round rim (8x8 circle) */
    px_rect(r, x + 2, y,     4, 1, c);
    px_rect(r, x + 2, y + 7, 4, 1, c);
    px_rect(r, x,     y + 2, 1, 4, c);
    px_rect(r, x + 7, y + 2, 1, 4, c);
    px_rect(r, x + 1, y + 1, 1, 1, c);
    px_rect(r, x + 6, y + 1, 1, 1, c);
    px_rect(r, x + 1, y + 6, 1, 1, c);
    px_rect(r, x + 6, y + 6, 1, 1, c);
    /* dial face */
    px_rect(r, x + 2, y + 2, 4, 4, KZ_CLOUD);
    /* needle: red pointing up-right (N), white pointing down-left (S) */
    px_rect(r, x + 4, y + 2, 1, 2, KZ_HEART);       /* red north tip */
    px_rect(r, x + 3, y + 4, 2, 1, rgb(0x9A, 0x86, 0x94)); /* white south */
    px_rect(r, x + 3, y + 3, 2, 1, KZ_HEART);       /* needle center */
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

static void glyph_chair(SDL_Renderer *r, float x, float y, Color c) {
    /* a little armchair silhouette */
    px_rect(r, x + 1, y,     7, 4, c);   /* back */
    px_rect(r, x,     y + 3, 9, 3, c);   /* seat + arms */
    px_rect(r, x + 1, y + 6, 2, 2, c);   /* legs */
    px_rect(r, x + 6, y + 6, 2, 2, c);
}

/* Two little cat faces side by side — the "friends" icon. Each cat is a small
 * head with two pointy ears; they sit close so they read as a pair. `c` is the
 * pink coat, `o` the mauve outline for eyes. */
static void glyph_two_cats(SDL_Renderer *r, float x, float y, Color c, Color o) {
    /* left cat (a touch lower, in front) */
    px_rect(r, x,     y + 3, 1, 2, c);   /* ears */
    px_rect(r, x + 3, y + 3, 1, 2, c);
    px_rect(r, x,     y + 5, 4, 4, c);   /* head */
    px_rect(r, x + 1, y + 6, 1, 1, o);   /* eyes */
    px_rect(r, x + 3, y + 6, 1, 1, o);
    /* right cat (a touch higher, behind) */
    px_rect(r, x + 5, y + 1, 1, 2, c);   /* ears */
    px_rect(r, x + 8, y + 1, 1, 2, c);
    px_rect(r, x + 5, y + 3, 4, 4, c);   /* head */
    px_rect(r, x + 6, y + 4, 1, 1, o);   /* eyes */
    px_rect(r, x + 8, y + 4, 1, 1, o);
}

/* A tiny checklist — two checkbox rows with lines beside them. */
static void glyph_quests(SDL_Renderer *r, float x, float y, Color c) {
    px_rect(r, x,     y,     4, 4, c);          /* first checkbox   */
    px_rect(r, x + 1, y + 1, 2, 2, KZ_CLOUD);
    px_rect(r, x + 5, y + 1, 4, 2, c);          /* line beside it   */
    px_rect(r, x,     y + 5, 4, 4, c);          /* second checkbox  */
    px_rect(r, x + 1, y + 6, 2, 2, KZ_CLOUD);
    px_rect(r, x + 5, y + 6, 4, 2, c);
}

/* a food bowl with kibble — the feed array button */
static void glyph_bowl(SDL_Renderer *r, float x, float y, Color c) {
    px_rect(r, x,     y + 4, 9, 4, c);
    px_rect(r, x + 1, y + 3, 7, 1, rgb(0xE0, 0xBC, 0xCC));
    px_rect(r, x + 2, y + 1, 2, 2, rgb(0xB0, 0x86, 0x62));
    px_rect(r, x + 5, y + 2, 2, 1, rgb(0xB0, 0x86, 0x62));
}

/* an envelope — the mailbox button */
static void glyph_envelope(SDL_Renderer *r, float x, float y, Color c) {
    px_rect(r, x,     y + 1, 9, 6, KZ_CLOUD);       /* body      */
    px_rect(r, x,     y + 1, 9, 1, c);              /* top edge  */
    px_rect(r, x,     y + 6, 9, 1, c);              /* bottom    */
    px_rect(r, x,     y + 1, 1, 6, c);              /* left      */
    px_rect(r, x + 8, y + 1, 1, 6, c);             /* right     */
    /* the flap (a V) */
    px_rect(r, x + 1, y + 2, 3, 1, c);
    px_rect(r, x + 5, y + 2, 3, 1, c);
    px_rect(r, x + 3, y + 3, 3, 1, c);
}

/* a paw with a sparkle — the trick trainer button */
static void glyph_trick(SDL_Renderer *r, float x, float y, Color c) {
    px_rect(r, x + 1, y + 4, 5, 4, c);          /* pad  */
    px_rect(r, x,     y + 2, 2, 2, c);          /* toes */
    px_rect(r, x + 3, y + 1, 2, 2, c);
    px_rect(r, x + 6, y + 2, 1, 2, c);
    px_rect(r, x + 7, y,     1, 4, KZ_BUTTER);  /* sparkle */
    px_rect(r, x + 6, y + 1, 3, 1, KZ_BUTTER);
}

/* two little paw prints on a path — the walk button */
static void glyph_walk(SDL_Renderer *r, float x, float y, Color c) {
    /* first paw print (lower-left) */
    px_rect(r, x, y + 4, 3, 3, c);
    px_rect(r, x, y + 2, 1, 1, c);
    px_rect(r, x + 2, y + 2, 1, 1, c);
    /* second paw print (upper-right) */
    px_rect(r, x + 5, y, 3, 3, c);
    px_rect(r, x + 5, y - 2, 1, 1, c);
    px_rect(r, x + 7, y - 2, 1, 1, c);
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
        case KZ_BTN_HOME:    glyph_map(r, gx, gy, KZ_COCOA); break;
        case KZ_BTN_OUT:     glyph_map(r, gx, gy, KZ_COCOA); break;
        case KZ_BTN_SLEEP:   glyph_moon(r,  gx, gy, KZ_COCOA); break;
        case KZ_BTN_TREAT:   glyph_fish(r,  gx, gy, rgb(0xE8,0x8B,0x6B)); break;
        case KZ_BTN_FRIENDS: glyph_two_cats(r, gx, gy, KZ_PETAL_PINK, KZ_CAT_OUTLINE); break;
        case KZ_BTN_QUESTS:  glyph_quests(r, gx, gy, KZ_BUTTER); break;
        case KZ_BTN_FEED:    glyph_bowl(r, gx, gy, KZ_PETAL_PINK); break;
        case KZ_BTN_MAIL:    glyph_envelope(r, gx, gy, KZ_COCOA); break;
        case KZ_BTN_TRICK:   glyph_trick(r, gx, gy, KZ_PETAL_PINK); break;
        case KZ_BTN_WALK:    glyph_walk(r, gx, gy, KZ_COCOA); break;
        case KZ_BTN_ZOOM_IN:
            /* a bold plus sign in a bright color */
            px_rect(r, gx + 3, gy - 1, 3, 11, KZ_HEART);
            px_rect(r, gx - 1, gy + 3, 11, 3, KZ_HEART);
            break;
        case KZ_BTN_ZOOM_OUT:
            /* a bold minus sign */
            px_rect(r, gx - 1, gy + 3, 11, 3, KZ_HEART);
            break;
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

/* The release button sits at the top-right corner of the stat card. */
#define REL_W 9
#define REL_H 9
static void rel_pos(float panel_x, float panel_y, float *rx, float *ry) {
    *rx = panel_x + 62 - REL_W - 2;
    *ry = panel_y + 2;
}
bool ui_release_hit(float panel_x, float panel_y, float px_, float py_) {
    float rx, ry; rel_pos(panel_x, panel_y, &rx, &ry);
    return px_ >= rx && px_ <= rx + REL_W && py_ >= ry && py_ <= ry + REL_H;
}
void ui_draw_release_button(SDL_Renderer *r, float panel_x, float panel_y,
                            bool armed) {
    float rx, ry; rel_pos(panel_x, panel_y, &rx, &ry);
    /* a soft circle-ish button, pink when armed */
    Color bg = armed ? KZ_HEART : KZ_CLOUD;
    px_rect(r, rx, ry, REL_W, REL_H, bg);
    px_rect(r, rx, ry, REL_W, 1, KZ_COCOA);
    px_rect(r, rx, ry + REL_H - 1, REL_W, 1, KZ_COCOA);
    px_rect(r, rx, ry, 1, REL_H, KZ_COCOA);
    px_rect(r, rx + REL_W - 1, ry, 1, REL_H, KZ_COCOA);
    /* a little "×" */
    Color x = armed ? KZ_CLOUD : KZ_COCOA;
    px_rect(r, rx + 2, ry + 2, 1, 1, x);
    px_rect(r, rx + 6, ry + 2, 1, 1, x);
    px_rect(r, rx + 3, ry + 3, 1, 1, x);
    px_rect(r, rx + 5, ry + 3, 1, 1, x);
    px_rect(r, rx + 4, ry + 4, 1, 1, x);
    px_rect(r, rx + 3, ry + 5, 1, 1, x);
    px_rect(r, rx + 5, ry + 5, 1, 1, x);
    px_rect(r, rx + 2, ry + 6, 1, 1, x);
    px_rect(r, rx + 6, ry + 6, 1, 1, x);
}

void ui_draw_panel(SDL_Renderer *r, const OwnedCat *cat, float x, float y,
                   bool editing, const char *edit_buf, Uint64 frame,
                   bool show) {
    if (!show) return;   /* auto-hidden — draw nothing */
    const Stats *s = &cat->stats;
    float w = 62, h = 66;   /* taller now: name + type + 3 stats + level/xp */

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
    /* Shiny cats get a small gold sparkle on the name line, positioned to the
     * left of the release button so it never overlaps it or runs past the box. */
    if (cat->shiny) {
        float sx = x + 41, sy = y + 3;
        Color gold = rgb(0xE0, 0xB0, 0x40);
        px_rect(r, sx + 1, sy,     1, 5, gold);      /* vertical   */
        px_rect(r, sx,     sy + 2, 3, 1, gold);      /* horizontal */
        px_rect(r, sx + 1, sy + 2, 1, 1, KZ_CLOUD);  /* bright center */
    }
    text_draw(r, cattype_name(cat->type), x + 4, y + 10,
              cattype_colors(cat->type).dark);
    px_rect(r, x + 3, y + 17, w - 6, 1, KZ_COCOA);   /* divider */

    float rx = x + 5, ry = y + 21;
    stat_row(r, rx, ry,      icon_heart, KZ_PETAL_PINK, s->bond);
    stat_row(r, rx, ry + 9,  icon_smile, KZ_BUTTER,     s->mood);
    stat_row(r, rx, ry + 18, icon_leaf,  KZ_MINT,       s->energy);

    /* Level line + XP bar (replaces the old hidden growth meter). */
    float ly2 = ry + 28;
    char lvl[16];
    SDL_snprintf(lvl, sizeof lvl, "Lv %u", (unsigned)s->level);
    text_draw(r, lvl, rx, ly2, KZ_COCOA);
    /* XP bar to the right of the level text */
    float bx = rx + 24, bw = 30, bh = 5;
    if (s->level >= 100) {
        /* maxed out: a full bar with a soft glow */
        px_rect(r, bx, ly2, bw, bh, KZ_LAVENDER);
        text_draw(r, "MAX", bx + 8, ly2, KZ_COCOA);
    } else {
        Uint16 need = stats_xp_for_level(s->level);
        float frac = need > 0 ? (float)s->xp / (float)need : 0.0f;
        if (frac > 1.0f) frac = 1.0f;
        px_rect(r, bx, ly2, bw, bh, KZ_CLOUD);
        if (frac > 0) px_rect(r, bx, ly2, bw * frac, bh, KZ_LAVENDER);
    }
    px_rect(r, bx,          ly2,          bw, 1, KZ_COCOA);
    px_rect(r, bx,          ly2 + bh - 1, bw, 1, KZ_COCOA);
    px_rect(r, bx,          ly2,          1,  bh, KZ_COCOA);
    px_rect(r, bx + bw - 1, ly2,          1,  bh, KZ_COCOA);
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

/* A tiny cat face in the given type color, faded for the auto-hiding roster. */
static void portrait_a(SDL_Renderer *r, float x, float y, CatColors col,
                       Uint8 a) {
    px_rect_a(r, x + 4, y + 6, 14, 12, col.body, a);
    px_rect_a(r, x + 4,  y + 2, 4, 5, col.body, a);
    px_rect_a(r, x + 14, y + 2, 4, 5, col.body, a);
    px_rect_a(r, x + 5,  y + 3, 2, 3, col.ear, a);
    px_rect_a(r, x + 15, y + 3, 2, 3, col.ear, a);
    px_rect_a(r, x + 7,  y + 10, 2, 2, KZ_CAT_OUTLINE, a);
    px_rect_a(r, x + 13, y + 10, 2, 2, KZ_CAT_OUTLINE, a);
    px_rect_a(r, x + 10, y + 13, 2, 1, KZ_CAT_NOSE, a);
    px_rect_a(r, x + 5,  y + 12, 2, 1, col.cheek, a);
    px_rect_a(r, x + 15, y + 12, 2, 1, col.cheek, a);
}

void ui_roster_draw(SDL_Renderer *r, const Roster *ro, Uint8 fade) {
    if (fade == 0) return;   /* fully hidden — draw nothing */
    for (int i = 0; i < ro->count; i++) {
        float x = rs_slot_x(i), y = RS_Y;
        bool active = (i == ro->active);

        /* slot background: cream, with a pink ring when active */
        px_rect_a(r, x + 1, y + 2, RS_SLOT, RS_SLOT, KZ_COCOA,
                  (Uint8)(40 * fade / 255)); /* shadow */
        px_rect_a(r, x, y, RS_SLOT, RS_SLOT, KZ_CLOUD, fade);
        Color border = active ? KZ_PETAL_PINK : KZ_COCOA;
        int th = active ? 2 : 1;   /* thicker ring when active */
        for (int t = 0; t < th; t++) {
            px_rect_a(r, x + t,             y + t,             RS_SLOT - 2*t, 1, border, fade);
            px_rect_a(r, x + t,             y + RS_SLOT-1 - t, RS_SLOT - 2*t, 1, border, fade);
            px_rect_a(r, x + t,             y + t,             1, RS_SLOT - 2*t, border, fade);
            px_rect_a(r, x + RS_SLOT-1 - t, y + t,             1, RS_SLOT - 2*t, border, fade);
        }

        CatColors pc = ro->cats[i].shiny ? cat_shiny_colors()
                                         : cattype_colors(ro->cats[i].type);
        portrait_a(r, x, y, pc, fade);

        /* shiny cats get a little gold sparkle in the corner of their slot */
        if (ro->cats[i].shiny) {
            float sx = x + RS_SLOT - 6, sy = y + 3;
            Color gold = rgb(0xFF, 0xE8, 0x9A);
            px_rect_a(r, sx,     sy - 1, 1, 3, gold, fade);
            px_rect_a(r, sx - 1, sy,     3, 1, gold, fade);
        }
    }

    /* "+" adopt slot, if there's room */
    if (ro->count < KZ_MAX_CATS) {
        float x = rs_slot_x(ro->count), y = RS_Y;
        px_rect_a(r, x + 1, y + 2, RS_SLOT, RS_SLOT, KZ_COCOA,
                  (Uint8)(40 * fade / 255));
        px_rect_a(r, x, y, RS_SLOT, RS_SLOT, KZ_CLOUD, fade);
        px_rect_a(r, x, y, RS_SLOT, 1, KZ_COCOA, fade);
        px_rect_a(r, x, y + RS_SLOT-1, RS_SLOT, 1, KZ_COCOA, fade);
        px_rect_a(r, x, y, 1, RS_SLOT, KZ_COCOA, fade);
        px_rect_a(r, x + RS_SLOT-1, y, 1, RS_SLOT, KZ_COCOA, fade);
        px_rect_a(r, x + 6,  y + 10, 10, 2, KZ_MINT, fade);
        px_rect_a(r, x + 10, y + 6,  2, 10, KZ_MINT, fade);
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

const Button KZ_QUESTS_BUTTON = { KZ_W - 24, 64, 20, 16, KZ_BTN_QUESTS };

void ui_quests_button_draw(SDL_Renderer *r, bool pressed) {
    ui_button_draw(r, &KZ_QUESTS_BUTTON, pressed);
}
bool ui_quests_button_hit(float px_, float py_) {
    return ui_button_hit(&KZ_QUESTS_BUTTON, px_, py_);
}

/* A soft dialogue banner across the bottom-center with one line of text. */
void ui_banner(SDL_Renderer *r, const char *line) {
    float maxw = 220;                     /* widest the banner may be */
    float tw = text_width(line);

    if (tw + 12 <= maxw) {
        /* short message: a single tidy line, box sized to fit */
        float w = tw + 16, h = 16;
        if (w < 120) w = 120;
        float x = (KZ_W - w) / 2.0f, y = 26;
        px_rect_a(r, x + 2, y + 2, w, h, KZ_COCOA, 40);
        px_rect(r, x, y, w, h, KZ_CLOUD);
        px_rect(r, x, y, w, 1, KZ_COCOA);
        px_rect(r, x, y + h - 1, w, 1, KZ_COCOA);
        px_rect(r, x, y, 1, h, KZ_COCOA);
        px_rect(r, x + w - 1, y, 1, h, KZ_COCOA);
        text_draw_centered(r, line, KZ_W / 2.0f, y + 5, KZ_COCOA);
        return;
    }

    /* long message: wrap onto two lines at a space near the middle */
    char l1[64] = {0}, l2[64] = {0};
    int len = (int)SDL_strlen(line);
    int split = len / 2;
    /* find the nearest space to the midpoint to break cleanly */
    int best = -1;
    for (int i = 0; i < len; i++) {
        if (line[i] == ' ') {
            if (best < 0 || SDL_abs(i - split) < SDL_abs(best - split)) best = i;
        }
    }
    if (best < 0) best = split;   /* no space: hard split */
    SDL_strlcpy(l1, line, (size_t)best + 1);
    SDL_strlcpy(l2, line + best + 1, sizeof l2);

    float w1 = text_width(l1), w2 = text_width(l2);
    float w = (w1 > w2 ? w1 : w2) + 16, h = 26;
    if (w < 140) w = 140;
    float x = (KZ_W - w) / 2.0f, y = 26;
    px_rect_a(r, x + 2, y + 2, w, h, KZ_COCOA, 40);
    px_rect(r, x, y, w, h, KZ_CLOUD);
    px_rect(r, x, y, w, 1, KZ_COCOA);
    px_rect(r, x, y + h - 1, w, 1, KZ_COCOA);
    px_rect(r, x, y, 1, h, KZ_COCOA);
    px_rect(r, x + w - 1, y, 1, h, KZ_COCOA);
    text_draw_centered(r, l1, KZ_W / 2.0f, y + 5, KZ_COCOA);
    text_draw_centered(r, l2, KZ_W / 2.0f, y + 15, KZ_COCOA);
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
            /* little portrait dot in her type color, with a soft outline so
             * each friend's face stands out */
            px_rect(r, x + 5, ry - 1, 8, 8, KZ_COCOA);            /* outline */
            px_rect(r, x + 6, ry, 6, 6, cattype_colors(fr->type).body);
            px_rect(r, x + 6, ry, 6, 1, cattype_colors(fr->type).dark);
            /* name */
            text_draw(r, fr->name, x + 16, ry, KZ_COCOA);
            /* trust bar */
            float bx = x + 78, bw = 42;
            px_rect(r, bx, ry, bw, 5, KZ_CLOUD);
            float fw = bw * ((float)fr->trust / (float)KZ_TRUST_FULL);
            if (fw > 0) px_rect(r, bx, ry, fw, 5, KZ_PETAL_PINK);
            px_rect(r, bx, ry, bw, 1, KZ_COCOA);
            px_rect(r, bx, ry + 4, bw, 1, KZ_COCOA);
            px_rect(r, bx, ry, 1, 5, KZ_COCOA);
            px_rect(r, bx + bw - 1, ry, 1, 5, KZ_COCOA);
            /* heart if fully befriended */
            if (fr->befriended) {
                float hx = bx + bw + 2;
                px_rect(r, hx, ry, 1, 1, KZ_HEART);
                px_rect(r, hx + 2, ry, 1, 1, KZ_HEART);
                px_rect(r, hx, ry + 1, 3, 1, KZ_HEART);
                px_rect(r, hx + 1, ry + 2, 1, 1, KZ_HEART);
            }
            /* "Visit" button: invites this friend over for a playdate */
            float hbx = x + w - 40, hby = ry - 1;
            px_rect(r, hbx, hby, 36, 9, KZ_MINT);
            px_rect(r, hbx, hby, 36, 1, KZ_COCOA);
            px_rect(r, hbx, hby + 8, 36, 1, KZ_COCOA);
            px_rect(r, hbx, hby, 1, 9, KZ_COCOA);
            px_rect(r, hbx + 35, hby, 1, 9, KZ_COCOA);
            text_draw_centered(r, "Visit", hbx + 18, ry, KZ_COCOA);
        }
    }
    text_draw_centered(r, "tap to close", KZ_W / 2.0f, y + h - 9, KZ_COCOA);
}

/* Which friend's "Hang out?" button was tapped, or -1. Layout mirrors
 * ui_friends_list above. */
int ui_friends_hangout_hit(const Friends *f, float px_, float py_) {
    float w = 180, h = 132;
    float x = (KZ_W - w) / 2.0f, y = (KZ_H - h) / 2.0f;
    for (int i = 0; i < f->count && i < 8; i++) {
        float ry = y + 18 + i * 13;
        float hbx = x + w - 40, hby = ry - 1;
        if (px_ >= hbx && px_ <= hbx + 36 && py_ >= hby && py_ <= hby + 9)
            return i;
    }
    return -1;
}

/* ---- décor tray ---- */

const Button KZ_DECOR_BUTTON = { KZ_W - 24, 44, 20, 16, KZ_BTN_DECOR };

void ui_decor_button_draw(SDL_Renderer *r, bool pressed) {
    ui_button_draw(r, &KZ_DECOR_BUTTON, pressed);
}
bool ui_decor_button_hit(float px_, float py_) {
    return ui_button_hit(&KZ_DECOR_BUTTON, px_, py_);
}

const Button KZ_FEED_BUTTON = { KZ_W - 24, 84, 20, 16, KZ_BTN_FEED };

void ui_feed_button_draw(SDL_Renderer *r, bool pressed) {
    ui_button_draw(r, &KZ_FEED_BUTTON, pressed);
}
bool ui_feed_button_hit(float px_, float py_) {
    return ui_button_hit(&KZ_FEED_BUTTON, px_, py_);
}

/* ---- the feed array: a tray of foods to give the active cat ---- */
#define FEED_H     40
#define FEED_Y     (KZ_H - FEED_H - 26)
#define FEED_SLOT  54

static float feed_slot_x(int i) { return 6.0f + i * (FEED_SLOT + 2); }

/* a small food icon for the feed tray */
static void feed_food_icon(SDL_Renderer *r, FoodKind f, float x, float y) {
    switch (f) {
        case FOOD_KIBBLE: {
            px_rect(r, x, y + 5, 16, 5, rgb(0xC8, 0x9C, 0xB0));
            px_rect(r, x + 1, y + 4, 14, 2, rgb(0xE0, 0xBC, 0xCC));
            px_rect(r, x + 3, y + 2, 3, 2, rgb(0xB0, 0x86, 0x62));
            px_rect(r, x + 8, y + 2, 3, 2, rgb(0xB0, 0x86, 0x62));
            break;
        }
        case FOOD_MILK: {
            px_rect(r, x + 4, y, 9, 11, KZ_CLOUD);
            px_rect(r, x + 4, y, 9, 1, rgb(0xC8, 0xD8, 0xE8));
            px_rect(r, x + 6, y + 4, 5, 3, rgb(0x9C, 0xC0, 0xD8));
            break;
        }
        case FOOD_TREAT: {
            Color t = rgb(0xE8, 0xA6, 0x8B);
            px_rect(r, x + 3, y + 3, 5, 5, t);
            px_rect(r, x + 8, y + 2, 4, 7, t);
            px_rect(r, x + 12, y + 4, 3, 3, t);
            break;
        }
        case FOOD_WATER:
        default: {
            px_rect(r, x + 4, y, 8, 11, rgb(0xD6, 0xEC, 0xF6));
            px_rect(r, x + 4, y + 5, 8, 6, rgb(0xAF, 0xD6, 0xEC));
            break;
        }
    }
}

float ui_feed_tray_top(void) { return (float)FEED_Y; }

void ui_feed_tray(SDL_Renderer *r, const Pantry *p, Uint64 frame) {
    (void)frame;
    px_rect_a(r, 0, FEED_Y - 2, KZ_W, FEED_H + 2, KZ_CLOUD, 235);
    px_rect(r, 0, FEED_Y - 2, KZ_W, 1, KZ_COCOA);
    text_draw(r, "feed - tap a food", 6, FEED_Y - 10, KZ_COCOA);

    /* show only foods you actually have, so the strip never runs off-screen */
    int slot = 0;
    for (int i = 0; i < FOOD_COUNT; i++) {
        if (p->stock[i] == 0) continue;
        float x = feed_slot_x(slot);
        if (x + FEED_SLOT > KZ_W) break;   /* safety: don't draw past the edge */
        px_rect(r, x, (float)FEED_Y + 2, FEED_SLOT, FEED_H - 4, KZ_CLOUD);
        px_rect(r, x, (float)FEED_Y + 2, FEED_SLOT, 1, KZ_COCOA);
        px_rect(r, x, (float)FEED_Y + FEED_H - 3, FEED_SLOT, 1, KZ_COCOA);
        px_rect(r, x, (float)FEED_Y + 2, 1, FEED_H - 4, KZ_COCOA);
        px_rect(r, x + FEED_SLOT - 1, (float)FEED_Y + 2, 1, FEED_H - 4, KZ_COCOA);
        feed_food_icon(r, (FoodKind)i, x + 6, (float)FEED_Y + 5);
        text_draw(r, food_name((FoodKind)i), x + 24, (float)FEED_Y + 6, KZ_COCOA);
        char cnt[16];
        SDL_snprintf(cnt, sizeof cnt, "x%u", (unsigned)p->stock[i]);
        text_draw(r, cnt, x + 24, (float)FEED_Y + 16, rgb(0x9A, 0x7A, 0x5A));
        slot++;
    }
    if (slot == 0)
        text_draw(r, "no food - buy some at the store!", 6, (float)FEED_Y + 14,
                  rgb(0x9A, 0x7A, 0x5A));
}

int ui_feed_tray_hit(const Pantry *p, float px_, float py_) {
    if (py_ < FEED_Y + 2 || py_ > FEED_Y + FEED_H - 2) return -1;
    int slot = 0;
    for (int i = 0; i < FOOD_COUNT; i++) {
        if (p->stock[i] == 0) continue;
        float x = feed_slot_x(slot);
        if (px_ >= x && px_ <= x + FEED_SLOT) return i;
        slot++;
    }
    return -1;
}

/* ---- mailbox: playdate invitations ---- */

const Button KZ_MAIL_BUTTON = { 4, KZ_H - 46, 20, 16, KZ_BTN_MAIL };

void ui_mail_button_draw(SDL_Renderer *r, const Owners *o, bool pressed) {
    ui_button_draw(r, &KZ_MAIL_BUTTON, pressed);
    /* a little red dot if letters are waiting */
    int n = owners_invite_count(o);
    if (n > 0) {
        float dx = KZ_MAIL_BUTTON.x + KZ_MAIL_BUTTON.w - 5;
        float dy = KZ_MAIL_BUTTON.y + 1;
        px_rect(r, dx, dy, 4, 4, KZ_HEART);
    }
}
bool ui_mail_button_hit(float px_, float py_) {
    return ui_button_hit(&KZ_MAIL_BUTTON, px_, py_);
}

#define MAIL_ROW 26
#define MAIL_X   24
#define MAIL_Y   22
#define MAIL_W   192

void ui_mailbox(SDL_Renderer *r, const Owners *o, Uint64 frame) {
    (void)frame;
    px_rect_a(r, 0, 0, KZ_W, KZ_H, rgb(0x3B, 0x30, 0x50), 150);

    int n = owners_invite_count(o);
    float h = 30 + (n > 0 ? n : 1) * MAIL_ROW + 8;
    if (h > 150) h = 150;
    float x = MAIL_X, y = MAIL_Y;
    px_rect(r, x, y, MAIL_W, h, KZ_CLOUD);
    px_rect(r, x, y, MAIL_W, 1, KZ_COCOA);
    px_rect(r, x, y + h - 1, MAIL_W, 1, KZ_COCOA);
    px_rect(r, x, y, 1, h, KZ_COCOA);
    px_rect(r, x + MAIL_W - 1, y, 1, h, KZ_COCOA);

    text_draw_scaled(r, "Mailbox", x + 8, y + 6, KZ_COCOA, 2);
    px_rect(r, x + 6, y + 22, MAIL_W - 12, 1, KZ_COCOA);

    if (n == 0) {
        text_draw(r, "No letters right now.", x + 10, y + 30, KZ_COCOA);
        text_draw_centered(r, "tap to close", KZ_W / 2.0f, y + h - 9, KZ_COCOA);
        return;
    }

    int row = 0;
    for (int i = 0; i < o->count; i++) {
        if (!o->list[i].invite_pending) continue;
        float ry = y + 28 + row * MAIL_ROW;
        if (ry + MAIL_ROW > y + h - 8) break;
        /* an envelope chip */
        px_rect(r, x + 8, ry, MAIL_W - 16, MAIL_ROW - 4,
                o->list[i].invite_read ? rgb(0xEC, 0xE4, 0xE8) : KZ_BUTTER);
        px_rect(r, x + 8, ry, MAIL_W - 16, 1, KZ_COCOA);
        px_rect(r, x + 8, ry + MAIL_ROW - 5, MAIL_W - 16, 1, KZ_COCOA);
        /* text — two lines so a long name never overflows the letter */
        char line[40];
        SDL_snprintf(line, sizeof line, "%s invites you", o->list[i].name);
        text_draw(r, line, x + 14, ry + 3, KZ_COCOA);
        text_draw(r, "to a playdate!  (tap to accept)", x + 14, ry + 12,
                  rgb(0x9A, 0x7A, 0x5A));
        row++;
    }
    text_draw_centered(r, "tap a letter to accept  -  or tap outside",
                       KZ_W / 2.0f, y + h - 9, KZ_COCOA);
}

int ui_mailbox_hit(const Owners *o, float px_, float py_) {
    int n = owners_invite_count(o);
    if (n == 0) return -1;
    float y = MAIL_Y;
    int row = 0;
    for (int i = 0; i < o->count; i++) {
        if (!o->list[i].invite_pending) continue;
        float ry = y + 28 + row * MAIL_ROW;
        if (px_ >= MAIL_X + 8 && px_ <= MAIL_X + MAIL_W - 8
            && py_ >= ry && py_ <= ry + MAIL_ROW - 4)
            return i;
        row++;
    }
    return -1;
}

/* ---- trick popup: a small row of trick icons beside the active cat ---- */

#define TRK_ICON   18      /* each icon cell is 18x18 */
#define TRK_GAP    2
#define TRK_N      TRICK_COUNT
/* the popup's total width and height */
#define TRK_POP_W  (TRK_N * TRK_ICON + (TRK_N - 1) * TRK_GAP + 6)
#define TRK_POP_H  (TRK_ICON + 6)

/* Place the popup above the anchor, clamped to stay on screen. */
static void trk_pop_origin(float ax, float ay, float *ox, float *oy) {
    float x = ax - TRK_POP_W / 2.0f;   /* centered over the cat */
    float y = ay - TRK_POP_H - 18;     /* floats above the cat's head */
    if (x < 2) x = 2;
    if (x > KZ_W - TRK_POP_W - 2) x = KZ_W - TRK_POP_W - 2;
    if (y < 2) y = 2;                  /* if too high, sit near the top */
    *ox = x; *oy = y;
}

/* a little icon for each trick, drawn in an 18x18 cell at (x,y) */
static void trick_icon(SDL_Renderer *r, TrickId t, float x, float y, Color c) {
    float cx = x + 9, cy = y + 9;
    switch (t) {
        case TRICK_SIT:       /* a seated cat silhouette */
            px_rect(r, cx - 3, cy - 4, 6, 6, c);        /* head/body */
            px_rect(r, cx - 4, cy + 2, 8, 3, c);        /* haunches  */
            px_rect(r, cx - 3, cy - 6, 1, 2, c);        /* ears      */
            px_rect(r, cx + 2, cy - 6, 1, 2, c);
            break;
        case TRICK_SPIN: {    /* a circular arrow */
            px_rect(r, cx - 3, cy - 4, 6, 1, c);
            px_rect(r, cx - 4, cy - 3, 1, 6, c);
            px_rect(r, cx + 3, cy - 3, 1, 4, c);
            px_rect(r, cx - 3, cy + 3, 6, 1, c);
            px_rect(r, cx + 2, cy - 5, 3, 1, c);        /* arrowhead */
            px_rect(r, cx + 4, cy - 5, 1, 3, c);
            break;
        }
        case TRICK_JUMP:      /* an up arrow */
            px_rect(r, cx - 1, cy - 5, 2, 10, c);
            px_rect(r, cx - 4, cy - 2, 3, 1, c);
            px_rect(r, cx + 2, cy - 2, 3, 1, c);
            px_rect(r, cx - 3, cy - 1, 1, 1, c);
            px_rect(r, cx + 3, cy - 1, 1, 1, c);
            break;
        case TRICK_HIGHFIVE:  /* a raised paw/hand */
            px_rect(r, cx - 3, cy - 2, 6, 6, c);        /* palm */
            px_rect(r, cx - 3, cy - 5, 1, 3, c);        /* fingers */
            px_rect(r, cx - 1, cy - 6, 1, 4, c);
            px_rect(r, cx + 1, cy - 6, 1, 4, c);
            px_rect(r, cx + 3, cy - 5, 1, 3, c);
            break;
        case TRICK_ROLL:      /* a spiral/curl */
        default:
            px_rect(r, cx - 3, cy - 3, 6, 6, c);
            px_rect(r, cx - 1, cy - 1, 2, 2, KZ_CLOUD);
            px_rect(r, cx + 1, cy - 3, 2, 1, KZ_CLOUD);
            break;
    }
}

void ui_trick_popup(SDL_Renderer *r, const Tricks *tr, const char *cat,
                    float anchor_x, float anchor_y, Uint64 frame) {
    (void)frame;
    float ox, oy;
    trk_pop_origin(anchor_x, anchor_y, &ox, &oy);

    /* soft panel with a little pointer toward the cat */
    px_rect_a(r, ox, oy, TRK_POP_W, TRK_POP_H, KZ_CLOUD, 240);
    px_rect(r, ox, oy, TRK_POP_W, 1, KZ_COCOA);
    px_rect(r, ox, oy + TRK_POP_H - 1, TRK_POP_W, 1, KZ_COCOA);
    px_rect(r, ox, oy, 1, TRK_POP_H, KZ_COCOA);
    px_rect(r, ox + TRK_POP_W - 1, oy, 1, TRK_POP_H, KZ_COCOA);
    /* pointer */
    float ptx = anchor_x;
    if (ptx < ox + 4) ptx = ox + 4;
    if (ptx > ox + TRK_POP_W - 4) ptx = ox + TRK_POP_W - 4;
    px_rect(r, ptx - 2, oy + TRK_POP_H, 4, 2, KZ_CLOUD);
    px_rect(r, ptx - 1, oy + TRK_POP_H + 2, 2, 2, KZ_CLOUD);

    for (int i = 0; i < TRICK_COUNT; i++) {
        float ix = ox + 3 + i * (TRK_ICON + TRK_GAP);
        float iy = oy + 3;
        int sk = tricks_skill(tr, cat, (TrickId)i);
        bool mastered = sk >= TRICK_MASTER;
        /* icon cell */
        px_rect(r, ix, iy, TRK_ICON, TRK_ICON,
                mastered ? rgb(0xE8, 0xF0, 0xE0) : rgb(0xF3, 0xEC, 0xF2));
        px_rect(r, ix, iy, TRK_ICON, 1, KZ_COCOA);
        px_rect(r, ix, iy + TRK_ICON - 1, TRK_ICON, 1, KZ_COCOA);
        px_rect(r, ix, iy, 1, TRK_ICON, KZ_COCOA);
        px_rect(r, ix + TRK_ICON - 1, iy, 1, TRK_ICON, KZ_COCOA);
        /* the trick icon */
        trick_icon(r, (TrickId)i, ix, iy,
                   mastered ? rgb(0x6A, 0xA0, 0x7A) : rgb(0x7A, 0x62, 0x94));
        /* progress: a thin bar along the bottom, or a gold star if mastered */
        if (mastered) {
            px_rect(r, ix + TRK_ICON / 2 - 1, iy + 1, 1, 3, KZ_BUTTER);
            px_rect(r, ix + TRK_ICON / 2 - 2, iy + 2, 3, 1, KZ_BUTTER);
        } else {
            float bw = (TRK_ICON - 4) * ((float)sk / TRICK_MASTER);
            px_rect(r, ix + 2, iy + TRK_ICON - 3, TRK_ICON - 4, 2,
                    rgb(0xE0, 0xD6, 0xE0));
            px_rect(r, ix + 2, iy + TRK_ICON - 3, bw, 2, KZ_PETAL_PINK);
        }
    }
}

int ui_trick_popup_hit(float anchor_x, float anchor_y, float px_, float py_) {
    float ox, oy;
    trk_pop_origin(anchor_x, anchor_y, &ox, &oy);
    if (py_ < oy + 3 || py_ > oy + 3 + TRK_ICON) return -1;
    for (int i = 0; i < TRICK_COUNT; i++) {
        float ix = ox + 3 + i * (TRK_ICON + TRK_GAP);
        if (px_ >= ix && px_ <= ix + TRK_ICON) return i;
    }
    return -1;
}

/* ---- walk button: start/stop a scenic park walk ---- */

const Button KZ_WALK_BUTTON = { 4, KZ_H - 66, 20, 16, KZ_BTN_WALK };

void ui_walk_button_draw(SDL_Renderer *r, bool walking, bool pressed) {
    ui_button_draw(r, &KZ_WALK_BUTTON, pressed);
    if (walking) {
        /* a soft ring to show a walk is in progress */
        px_rect(r, KZ_WALK_BUTTON.x - 1, KZ_WALK_BUTTON.y - 1,
                KZ_WALK_BUTTON.w + 2, 1, KZ_HEART);
        px_rect(r, KZ_WALK_BUTTON.x - 1,
                KZ_WALK_BUTTON.y + KZ_WALK_BUTTON.h,
                KZ_WALK_BUTTON.w + 2, 1, KZ_HEART);
    }
}
bool ui_walk_button_hit(float px_, float py_) {
    return ui_button_hit(&KZ_WALK_BUTTON, px_, py_);
}


#define TRAY_H     38                      /* height of the décor tray    */
#define TRAY_Y     (KZ_H - TRAY_H - 26)   /* sits above the roster strip */
#define TRAY_SLOT  30
#define TRAY_PAGE  6                       /* item slots shown per page   */

float ui_decor_tray_top(void) { return (float)TRAY_Y; }
float ui_decor_tray_bottom(void) { return (float)(TRAY_Y + TRAY_H); }

static float tray_slot_x(int slot_on_page) {
    return 6.0f + slot_on_page * (TRAY_SLOT + 2);
}

/* The "+more" button lives after the last slot on the page. */
static float tray_more_x(void) {
    return tray_slot_x(TRAY_PAGE);
}

/* How many owned items are in the tray. */
int ui_decor_tray_count(const Decor *d) {
    int n = 0;
    for (int i = 0; i < DECOR_COUNT; i++)
        if (d->items[i].owned) n++;
    return n;
}

/* How many pages of items there are (at least 1). */
int ui_decor_tray_pages(const Decor *d) {
    int n = ui_decor_tray_count(d);
    int pages = (n + TRAY_PAGE - 1) / TRAY_PAGE;
    return pages < 1 ? 1 : pages;
}

void ui_decor_tray(SDL_Renderer *r, const Decor *d, Uint64 frame, int page) {
    /* backing strip */
    px_rect_a(r, 0, TRAY_Y - 2, KZ_W, TRAY_H + 2, KZ_CLOUD, 235);
    px_rect(r, 0, TRAY_Y - 2, KZ_W, 1, KZ_COCOA);

    int pages = ui_decor_tray_pages(d);
    if (page < 0) page = 0;
    if (page >= pages) page = pages - 1;

    if (pages > 1) {
        char lbl[28];
        SDL_snprintf(lbl, sizeof lbl, "drag to place  -  page %d/%d",
                     page + 1, pages);
        text_draw(r, lbl, 6, TRAY_Y - 10, KZ_COCOA);
    } else {
        text_draw(r, "drag to place", 6, TRAY_Y - 10, KZ_COCOA);
    }

    /* draw this page's slots */
    int start = page * TRAY_PAGE;
    int vis = 0, shown = 0;
    for (int i = 0; i < DECOR_COUNT; i++) {
        if (!d->items[i].owned) continue;
        int idx = vis++;
        if (idx < start || idx >= start + TRAY_PAGE) continue;
        float sx = tray_slot_x(shown++);
        /* slot */
        px_rect(r, sx, (float)TRAY_Y + 2, TRAY_SLOT, TRAY_SLOT - 4, KZ_CLOUD);
        px_rect(r, sx, (float)TRAY_Y + 2, TRAY_SLOT, 1, KZ_COCOA);
        px_rect(r, sx, (float)TRAY_Y + TRAY_SLOT - 3, TRAY_SLOT, 1, KZ_COCOA);
        px_rect(r, sx, (float)TRAY_Y + 2, 1, TRAY_SLOT - 4, KZ_COCOA);
        px_rect(r, sx + TRAY_SLOT - 1, (float)TRAY_Y + 2, 1, TRAY_SLOT - 4, KZ_COCOA);
        decor_draw_preview(r, (DecorKind)i, sx, (float)TRAY_Y + 2,
                           TRAY_SLOT, TRAY_SLOT - 4, frame);
        if (d->items[i].placed) {
            px_rect(r, sx + TRAY_SLOT - 6, (float)TRAY_Y + 3, 3, 1, KZ_MINT);
            px_rect(r, sx + TRAY_SLOT - 4, (float)TRAY_Y + 4, 1, 2, KZ_MINT);
        }
        text_draw_centered(r, decor_info((DecorKind)i)->name,
                           sx + TRAY_SLOT / 2.0f,
                           (float)TRAY_Y + TRAY_SLOT - 1, KZ_COCOA);
    }

    /* the "+more" button (only when there's more than one page) */
    if (pages > 1) {
        float mx = tray_more_x();
        px_rect(r, mx, (float)TRAY_Y + 2, TRAY_SLOT, TRAY_SLOT - 4, KZ_BUTTER);
        px_rect(r, mx, (float)TRAY_Y + 2, TRAY_SLOT, 1, KZ_COCOA);
        px_rect(r, mx, (float)TRAY_Y + TRAY_SLOT - 3, TRAY_SLOT, 1, KZ_COCOA);
        px_rect(r, mx, (float)TRAY_Y + 2, 1, TRAY_SLOT - 4, KZ_COCOA);
        px_rect(r, mx + TRAY_SLOT - 1, (float)TRAY_Y + 2, 1, TRAY_SLOT - 4, KZ_COCOA);
        /* a "+" and the word more */
        px_rect(r, mx + 8, (float)TRAY_Y + 11, 8, 2, KZ_COCOA);
        px_rect(r, mx + 11, (float)TRAY_Y + 8, 2, 8, KZ_COCOA);
        text_draw_centered(r, "more", mx + TRAY_SLOT / 2.0f,
                           (float)TRAY_Y + TRAY_SLOT - 1, KZ_COCOA);
    }
}

int ui_decor_tray_hit(const Decor *d, float px_, float py_, int page) {
    if (py_ < TRAY_Y + 2 || py_ > TRAY_Y + TRAY_SLOT - 2) return -1;
    int pages = ui_decor_tray_pages(d);
    if (page < 0) page = 0;
    if (page >= pages) page = pages - 1;
    int start = page * TRAY_PAGE;
    int vis = 0, shown = 0;
    for (int i = 0; i < DECOR_COUNT; i++) {
        if (!d->items[i].owned) continue;
        int idx = vis++;
        if (idx < start || idx >= start + TRAY_PAGE) continue;
        float sx = tray_slot_x(shown++);
        if (px_ >= sx && px_ <= sx + TRAY_SLOT) return i;
    }
    return -1;
}

/* Did the tap hit the "+more" button? (only meaningful when >1 page) */
bool ui_decor_tray_more_hit(const Decor *d, float px_, float py_) {
    if (ui_decor_tray_pages(d) <= 1) return false;
    if (py_ < TRAY_Y + 2 || py_ > TRAY_Y + TRAY_SLOT - 2) return false;
    float mx = tray_more_x();
    return px_ >= mx && px_ <= mx + TRAY_SLOT;
}


/* ---- travel place-picker menu ---- */

/* Stacked rows near the top-right, under the travel button. */
#define PM_X     (KZ_W - 88)
#define PM_Y     24
#define PM_W     84
#define PM_ROW   16
#define PM_COUNT 5

static const char *PM_NAMES[PM_COUNT] = { "Cottage", "Meadow", "Cafe", "Forest", "Street" };

void ui_place_menu(SDL_Renderer *r, int current) {
    /* soft panel */
    float h = PM_ROW * PM_COUNT + 6;
    px_rect_a(r, PM_X + 2, PM_Y + 2, PM_W, h, KZ_COCOA, 40);
    px_rect(r, PM_X, PM_Y, PM_W, h, KZ_CLOUD);
    px_rect(r, PM_X, PM_Y, PM_W, 1, KZ_COCOA);
    px_rect(r, PM_X, PM_Y + h - 1, PM_W, 1, KZ_COCOA);
    px_rect(r, PM_X, PM_Y, 1, h, KZ_COCOA);
    px_rect(r, PM_X + PM_W - 1, PM_Y, 1, h, KZ_COCOA);

    for (int i = 0; i < PM_COUNT; i++) {
        float ry = PM_Y + 3 + i * PM_ROW;
        if (i == current) {
            px_rect(r, PM_X + 2, ry, PM_W - 4, PM_ROW - 2, KZ_PETAL_PINK);
        }
        text_draw(r, PM_NAMES[i], PM_X + 8, ry + 4, KZ_COCOA);
        /* a small dot marker in a place-ish color */
        Color dot = (i == 0) ? KZ_LAVENDER
                  : (i == 1) ? KZ_MINT
                  : (i == 2) ? KZ_BUTTER
                  : (i == 3) ? rgb(0x8F, 0xC0, 0xA4)    /* forest green */
                  : rgb(0xE0, 0xC8, 0xB8);              /* street stone */
        px_rect(r, PM_X + PM_W - 12, ry + 4, 5, 5, dot);
    }
}

int ui_place_menu_hit(float px_, float py_) {
    if (px_ < PM_X || px_ > PM_X + PM_W) return -1;
    for (int i = 0; i < PM_COUNT; i++) {
        float ry = PM_Y + 3 + i * PM_ROW;
        if (py_ >= ry && py_ <= ry + PM_ROW) return i;
    }
    return -1;
}

/* ---- release confirmation dialog ---- */

#define CF_W 150
#define CF_H 52
#define CF_X ((KZ_W - CF_W) / 2.0f)
#define CF_Y 48.0f
#define CF_BTN_W 40
#define CF_BTN_H 14

static void cf_btn_pos(int which, float *bx, float *by) {
    /* two buttons side by side under the text: 0 = Yes (left), 1 = No */
    float total = CF_BTN_W * 2 + 14;
    float x0 = CF_X + (CF_W - total) / 2.0f;
    *bx = x0 + (float)which * (CF_BTN_W + 14);
    *by = CF_Y + CF_H - CF_BTN_H - 6;
}

void ui_confirm_release(SDL_Renderer *r, const char *cat_name) {
    /* dim the world behind the question */
    px_rect_a(r, 0, 0, KZ_W, KZ_H, KZ_COCOA, 90);

    /* panel */
    px_rect_a(r, CF_X + 2, CF_Y + 2, CF_W, CF_H, KZ_COCOA, 60);
    px_rect(r, CF_X, CF_Y, CF_W, CF_H, KZ_CLOUD);
    px_rect(r, CF_X, CF_Y, CF_W, 1, KZ_COCOA);
    px_rect(r, CF_X, CF_Y + CF_H - 1, CF_W, 1, KZ_COCOA);
    px_rect(r, CF_X, CF_Y, 1, CF_H, KZ_COCOA);
    px_rect(r, CF_X + CF_W - 1, CF_Y, 1, CF_H, KZ_COCOA);

    /* the question, centered */
    char line[48];
    SDL_snprintf(line, sizeof line, "Release %s?", cat_name);
    text_draw_centered(r, line, CF_X + CF_W / 2.0f, CF_Y + 8, KZ_COCOA);
    text_draw_centered(r, "She won't come back.", CF_X + CF_W / 2.0f,
                       CF_Y + 17, KZ_COCOA);

    /* Yes / No buttons */
    for (int b = 0; b < 2; b++) {
        float bx, by; cf_btn_pos(b, &bx, &by);
        Color fill = (b == 0) ? KZ_HEART : KZ_MINT;
        px_rect(r, bx, by, CF_BTN_W, CF_BTN_H, fill);
        px_rect(r, bx, by, CF_BTN_W, 1, KZ_COCOA);
        px_rect(r, bx, by + CF_BTN_H - 1, CF_BTN_W, 1, KZ_COCOA);
        px_rect(r, bx, by, 1, CF_BTN_H, KZ_COCOA);
        px_rect(r, bx + CF_BTN_W - 1, by, 1, CF_BTN_H, KZ_COCOA);
        text_draw_centered(r, (b == 0) ? "Yes" : "No",
                           bx + CF_BTN_W / 2.0f, by + 4, KZ_COCOA);
    }
}

int ui_confirm_release_hit(float px_, float py_) {
    for (int b = 0; b < 2; b++) {
        float bx, by; cf_btn_pos(b, &bx, &by);
        if (px_ >= bx && px_ <= bx + CF_BTN_W
            && py_ >= by && py_ <= by + CF_BTN_H)
            return (b == 0) ? 1 : 0;
    }
    return -1;
}

/* ---- the quest log overlay ---- */

/* Draw a small (~9px) quest-kind icon at (x,y). Each is a simple pastel glyph. */
static void quest_icon(SDL_Renderer *r, QuestIcon k, float x, float y) {
    switch (k) {
        case QICON_PAW: {   /* a paw: pad + toes */
            Color c = KZ_PETAL_PINK;
            px_rect(r, x + 2, y + 4, 5, 4, c);
            px_rect(r, x + 1, y + 1, 2, 2, c);
            px_rect(r, x + 4, y,     2, 2, c);
            px_rect(r, x + 7, y + 1, 1, 2, c);
            break;
        }
        case QICON_FISH: {  /* a little fish */
            Color c = rgb(0xE8, 0x8B, 0x6B);
            px_rect(r, x, y + 3, 6, 4, c);
            px_rect(r, x + 6, y + 2, 2, 2, c);
            px_rect(r, x + 6, y + 6, 2, 2, c);
            px_rect(r, x + 1, y + 4, 1, 1, KZ_CLOUD);
            break;
        }
        case QICON_SPARKLE: {  /* a four-point sparkle */
            Color c = KZ_BUTTER;
            px_rect(r, x + 3, y,     1, 9, c);
            px_rect(r, x,     y + 4, 9, 1, c);
            px_rect(r, x + 2, y + 3, 3, 3, KZ_CLOUD);
            break;
        }
        case QICON_HEART: {
            Color c = KZ_HEART;
            px_rect(r, x + 1, y + 1, 2, 1, c);
            px_rect(r, x + 5, y + 1, 2, 1, c);
            px_rect(r, x,     y + 2, 8, 2, c);
            px_rect(r, x + 1, y + 4, 6, 1, c);
            px_rect(r, x + 2, y + 5, 4, 1, c);
            px_rect(r, x + 3, y + 6, 2, 1, c);
            break;
        }
        case QICON_CATS: {  /* two tiny cat heads */
            Color c = KZ_LAVENDER;
            px_rect(r, x,     y + 2, 1, 2, c);
            px_rect(r, x + 3, y + 2, 1, 2, c);
            px_rect(r, x,     y + 4, 4, 4, c);
            px_rect(r, x + 5, y,     1, 2, c);
            px_rect(r, x + 8, y,     1, 2, c);
            px_rect(r, x + 5, y + 2, 4, 4, c);
            break;
        }
        case QICON_STAR: {
            Color c = rgb(0xF2, 0xD0, 0x7A);
            px_rect(r, x + 4, y,     1, 9, c);
            px_rect(r, x,     y + 3, 9, 1, c);
            px_rect(r, x + 1, y + 1, 7, 5, c);
            break;
        }
        case QICON_CUP: {   /* a coffee cup */
            Color c = rgb(0xC0, 0x98, 0x88);
            px_rect(r, x + 1, y + 2, 6, 5, KZ_CLOUD);
            px_rect(r, x + 1, y + 2, 6, 1, c);
            px_rect(r, x + 7, y + 3, 2, 2, c);
            px_rect(r, x + 2, y,     3, 1, KZ_CLOUD);
            break;
        }
        case QICON_LEAF:
        default: {          /* a leaf: the world / re-coloring */
            Color c = KZ_MINT;
            px_rect(r, x + 1, y + 1, 6, 6, c);
            px_rect(r, x,     y + 4, 3, 3, c);
            px_rect(r, x + 4, y,     3, 3, c);
            px_rect(r, x + 3, y + 3, 3, 1, rgb(0x8F, 0xC0, 0xA4));
            break;
        }
    }
}

void ui_quests_list(SDL_Renderer *r, const Quests *q, int scroll) {
    /* dim the scene */
    px_rect_a(r, 0, 0, KZ_W, KZ_H, rgb(0x3B, 0x30, 0x50), 150);

    float w = 208, h = 150;
    float x = (KZ_W - w) / 2.0f, y = (KZ_H - h) / 2.0f;
    px_rect(r, x, y, w, h, KZ_CLOUD);
    px_rect(r, x,         y,         w, 1, KZ_COCOA);
    px_rect(r, x,         y + h - 1, w, 1, KZ_COCOA);
    px_rect(r, x,         y,         1, h, KZ_COCOA);
    px_rect(r, x + w - 1, y,         1, h, KZ_COCOA);

    /* header, at 2x scale for a clear title */
    char head[28];
    SDL_snprintf(head, sizeof head, "Quests  %d done",
                 quests_total_completed(q));
    text_draw_scaled(r, head, x + 8, y + 6, KZ_COCOA, 2);
    px_rect(r, x + 6, y + 22, w - 12, 1, KZ_COCOA);

    /* rows: normal-size font (crisp, not chunky) with roomy spacing so it's
     * easy to read without feeling oversized. */
    const int ROW_H = 15;
    const int VIS   = 8;                /* more rows fit at this size  */
    float list_top  = y + 27;
    int maxscroll = QUEST_COUNT - VIS;
    if (maxscroll < 0) maxscroll = 0;
    if (scroll < 0) scroll = 0;
    if (scroll > maxscroll) scroll = maxscroll;

    for (int v = 0; v < VIS; v++) {
        int i = scroll + v;
        if (i >= QUEST_COUNT) break;
        const QuestInfo *in = quest_info((QuestId)i);
        float ry = list_top + v * ROW_H;

        /* the kind-of-quest icon at the very left */
        quest_icon(r, in->icon, x + 8, ry + 1);

        if (q->done[i]) {
            /* mint check box (milestones that are permanently complete) */
            px_rect(r, x + 20, ry + 1, 8, 8, KZ_MINT);
            px_rect(r, x + 22, ry + 4, 1, 2, KZ_CLOUD);
            px_rect(r, x + 23, ry + 5, 1, 2, KZ_CLOUD);
            px_rect(r, x + 24, ry + 3, 1, 3, KZ_CLOUD);
            text_draw(r, in->desc, x + 34, ry + 1, rgb(0xB0, 0x9E, 0xA8));
        } else {
            /* empty box */
            px_rect(r, x + 20, ry + 1, 8, 8, KZ_CLOUD);
            px_rect(r, x + 20, ry + 1, 8, 1, KZ_COCOA);
            px_rect(r, x + 20, ry + 8, 8, 1, KZ_COCOA);
            px_rect(r, x + 20, ry + 1, 1, 8, KZ_COCOA);
            px_rect(r, x + 27, ry + 1, 1, 8, KZ_COCOA);
            text_draw(r, in->desc, x + 34, ry + 1, KZ_COCOA);
            /* progress toward the LIVE target (repeatables grow each round) */
            char prog[16];
            SDL_snprintf(prog, sizeof prog, "%u/%u",
                         (unsigned)q->progress[i],
                         (unsigned)quest_live_target(q, (QuestId)i));
            text_draw(r, prog, x + w - 8 - text_width(prog), ry + 1, KZ_COCOA);
        }
    }

    /* a little scrollbar on the right if the list overflows */
    if (maxscroll > 0) {
        float track_x = x + w - 5, track_y = list_top, track_h = VIS * ROW_H;
        px_rect(r, track_x, track_y, 2, track_h, rgb(0xE0, 0xD6, 0xE0));
        float thumb_h = track_h * (float)VIS / (float)QUEST_COUNT;
        float thumb_y = track_y
                      + (track_h - thumb_h) * (float)scroll / (float)maxscroll;
        px_rect(r, track_x, thumb_y, 2, thumb_h, KZ_PETAL_PINK);
    }

    text_draw_centered(r, "swipe to scroll  -  tap to close",
                       KZ_W / 2.0f, y + h - 9, KZ_COCOA);
}
/* ---- travel confirmation dialog ("Go here?") ---- */

#define TC_W 156
#define TC_H 50
#define TC_X ((KZ_W - TC_W) / 2.0f)
#define TC_Y 52.0f
#define TC_BTN_W 44
#define TC_BTN_H 14

static void tc_btn_pos(int which, float *bx, float *by) {
    float total = TC_BTN_W * 2 + 14;
    float x0 = TC_X + (TC_W - total) / 2.0f;
    *bx = x0 + (float)which * (TC_BTN_W + 14);
    *by = TC_Y + TC_H - TC_BTN_H - 6;
}

void ui_confirm_travel(SDL_Renderer *r, const char *place_name) {
    px_rect_a(r, 0, 0, KZ_W, KZ_H, KZ_COCOA, 70);

    px_rect_a(r, TC_X + 2, TC_Y + 2, TC_W, TC_H, KZ_COCOA, 60);
    px_rect(r, TC_X, TC_Y, TC_W, TC_H, KZ_CLOUD);
    px_rect(r, TC_X, TC_Y, TC_W, 1, KZ_COCOA);
    px_rect(r, TC_X, TC_Y + TC_H - 1, TC_W, 1, KZ_COCOA);
    px_rect(r, TC_X, TC_Y, 1, TC_H, KZ_COCOA);
    px_rect(r, TC_X + TC_W - 1, TC_Y, 1, TC_H, KZ_COCOA);

    char line[48];
    SDL_snprintf(line, sizeof line, "Go to %s?", place_name);
    text_draw_centered(r, line, TC_X + TC_W / 2.0f, TC_Y + 10, KZ_COCOA);

    for (int b = 0; b < 2; b++) {
        float bx, by; tc_btn_pos(b, &bx, &by);
        Color fill = (b == 0) ? KZ_MINT : KZ_PETAL_PINK;
        px_rect(r, bx, by, TC_BTN_W, TC_BTN_H, fill);
        px_rect(r, bx, by, TC_BTN_W, 1, KZ_COCOA);
        px_rect(r, bx, by + TC_BTN_H - 1, TC_BTN_W, 1, KZ_COCOA);
        px_rect(r, bx, by, 1, TC_BTN_H, KZ_COCOA);
        px_rect(r, bx + TC_BTN_W - 1, by, 1, TC_BTN_H, KZ_COCOA);
        text_draw_centered(r, (b == 0) ? "Go (A)" : "No (B)",
                           bx + TC_BTN_W / 2.0f, by + 4, KZ_COCOA);
    }
}

int ui_confirm_travel_hit(float px_, float py_) {
    for (int b = 0; b < 2; b++) {
        float bx, by; tc_btn_pos(b, &bx, &by);
        if (px_ >= bx && px_ <= bx + TC_BTN_W
            && py_ >= by && py_ <= by + TC_BTN_H)
            return (b == 0) ? 1 : 0;
    }
    return -1;
}