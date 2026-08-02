/* worldmap.c — see worldmap.h. The world laid out, and how you move on it. */
#include "worldmap.h"
#include "render.h"
#include "palette.h"
#include "text.h"
#include <math.h>

/* Cities: soft regions of the world. Names chosen for a dreamy, pastel feel. */
static const MapCity CITIES[] = {
    { "Pearl City",  8,   28,  92, 58, 0xF0, 0xDD, 0xE8 },  /* home, left      */
    { "Satin City",  150, 26,  84, 62, 0xDD, 0xE4, 0xF2 },  /* town, right     */
    { "Fern Hollow", 30,  98, 116, 50, 0xD6, 0xE8, 0xD8 },  /* wild, low-middle */
};
#define CITY_COUNT ((int)(sizeof CITIES / sizeof CITIES[0]))

/* Places, dotted inside their cities. Index order MUST match main's Location
 * enum: 0 Cottage, 1 Meadow, 2 Cafe, 3 Forest, 4 Street. Positions chosen to
 * sit on land (clear of the river winding down the center). */
static const MapPlace PLACES[MAP_PLACE_COUNT] = {
    { "Cottage",  0,  30,  46 },   /* Pearl City, upper */
    { "Meadow",   0,  72,  66 },   /* Pearl City, lower */
    { "Cafe",     1, 176,  46 },   /* Satin City, upper */
    { "Forest",   2,  62, 122 },   /* Fern Hollow, left */
    { "Street",   1, 210,  70 },   /* Satin City, lower */
    { "Market",   1, 176,  74 },   /* Satin City, mid — the flea market */
};

int             map_place_count(void)   { return MAP_PLACE_COUNT; }
const MapPlace *map_place(int i) {
    if (i < 0 || i >= MAP_PLACE_COUNT) i = 0;
    return &PLACES[i];
}
int             map_city_count(void)     { return CITY_COUNT; }
const MapCity  *map_city(int i) {
    if (i < 0 || i >= CITY_COUNT) i = 0;
    return &CITIES[i];
}

int map_move(int selected, int dx, int dy) {
    if (selected < 0 || selected >= MAP_PLACE_COUNT) return 0;
    if (dx == 0 && dy == 0) return selected;
    float sx = PLACES[selected].x, sy = PLACES[selected].y;

    int best = selected;
    float best_score = 1e9f;
    for (int i = 0; i < MAP_PLACE_COUNT; i++) {
        if (i == selected) continue;
        float ddx = PLACES[i].x - sx, ddy = PLACES[i].y - sy;
        /* the candidate must lie generally in the requested direction */
        float along = ddx * dx + ddy * dy;
        if (along <= 0) continue;
        /* prefer close and well-aligned: distance plus off-axis penalty */
        float off = fabsf(ddx * dy - ddy * dx);   /* perpendicular spread */
        float dist = sqrtf(ddx * ddx + ddy * ddy);
        float score = dist + off * 2.0f;
        if (score < best_score) { best_score = score; best = i; }
    }
    return best;
}

int map_hit(float px, float py) {
    for (int i = 0; i < MAP_PLACE_COUNT; i++) {
        float ddx = px - PLACES[i].x, ddy = py - PLACES[i].y;
        if (ddx * ddx + ddy * ddy <= 10.0f * 10.0f) return i;   /* ~10px dot */
    }
    return -1;
}

/* a small location pin: a dot with a soft ring */
static void draw_pin(SDL_Renderer *r, float x, float y, Color c, bool filled) {
    if (filled) {
        px_rect(r, x - 3, y - 3, 6, 6, c);
        px_rect(r, x - 4, y - 1, 8, 2, c);
        px_rect(r, x - 1, y - 4, 2, 8, c);
    } else {
        px_rect(r, x - 3, y - 3, 6, 1, c);
        px_rect(r, x - 3, y + 2, 6, 1, c);
        px_rect(r, x - 3, y - 3, 1, 6, c);
        px_rect(r, x + 2, y - 3, 1, 6, c);
    }
}

/* a little tree cluster for the map's terrain */
static void map_tree(SDL_Renderer *r, float x, float y, Uint64 frame, int seed) {
    float sway = sinf((float)frame * 0.02f + (float)seed) * 0.8f;
    px_rect(r, x + 1, y + 4, 2, 4, rgb(0xA8, 0x86, 0x8E));       /* trunk */
    px_rect(r, x - 2 + sway, y - 2, 8, 6, rgb(0x9C, 0xC6, 0xA4)); /* canopy */
    px_rect(r, x - 1 + sway, y - 4, 6, 4, rgb(0xB6, 0xDA, 0xBC));
}

void map_draw(SDL_Renderer *r, int selected, int current, Uint64 frame) {
    /* ---- terrain base: soft green land under a gentle sky band ---- */
    px_rect(r, 0, 0, KZ_W, KZ_H, rgb(0xC7, 0xE2, 0xC4));         /* meadow green */
    px_rect(r, 0, 0, KZ_W, 20, rgb(0xD9, 0xEC, 0xF2));           /* sky at top   */
    px_rect(r, 0, 18, KZ_W, 4, rgb(0xCF, 0xE6, 0xDA));           /* horizon blend */

    /* ---- rolling hills: layered soft bands with lighter crests ---- */
    for (int h = 0; h < 3; h++) {
        float base = 40.0f + h * 34.0f;
        Color hill = (h % 2) ? rgb(0xBC, 0xDC, 0xB8) : rgb(0xB2, 0xD6, 0xB0);
        for (float x = 0; x < KZ_W; x += 2) {
            float crest = sinf(x * 0.03f + h * 1.7f) * 6.0f;
            px_rect(r, x, base + crest, 2, KZ_H - (base + crest), hill);
        }
    }

    /* ---- a winding river down the middle, with a little pond ---- */
    for (float y = 22; y < KZ_H; y += 1) {
        float rx = 118.0f + sinf(y * 0.06f) * 22.0f;
        px_rect(r, rx, y, 6, 1, rgb(0xAF, 0xD6, 0xEC));          /* water */
        px_rect(r, rx + 1, y, 2, 1, rgb(0xC6, 0xE6, 0xF4));      /* shimmer */
    }
    /* pond in Fern Hollow's lower area */
    px_rect(r, 40, 132, 26, 12, rgb(0xAF, 0xD6, 0xEC));
    px_rect(r, 43, 130, 20, 3,  rgb(0xAF, 0xD6, 0xEC));
    px_rect(r, 44, 134, 14, 4,  rgb(0xC6, 0xE6, 0xF4));

    /* a faint title banner */
    px_rect_a(r, 4, 4, 56, 12, KZ_CLOUD, 200);
    text_draw(r, "where to?", 8, 6, KZ_COCOA);

    /* ---- city regions: soft translucent land tints with a name ---- */
    for (int c = 0; c < CITY_COUNT; c++) {
        const MapCity *ct = &CITIES[c];
        /* a gentle translucent wash so the terrain shows through */
        Color tint = rgb(ct->r, ct->g, ct->b);
        px_rect_a(r, ct->x + 3, ct->y, ct->w - 6, ct->h, tint, 90);
        px_rect_a(r, ct->x, ct->y + 3, ct->w, ct->h - 6, tint, 90);
        /* a dashed border to mark the region softly */
        for (float x = ct->x + 2; x < ct->x + ct->w - 2; x += 6) {
            px_rect(r, x, ct->y, 3, 1, rgb(0xB0, 0xA0, 0xB0));
            px_rect(r, x, ct->y + ct->h - 1, 3, 1, rgb(0xB0, 0xA0, 0xB0));
        }
        for (float y = ct->y + 2; y < ct->y + ct->h - 2; y += 6) {
            px_rect(r, ct->x, y, 1, 3, rgb(0xB0, 0xA0, 0xB0));
            px_rect(r, ct->x + ct->w - 1, y, 1, 3, rgb(0xB0, 0xA0, 0xB0));
        }
        /* name on a little chip so it stays readable over terrain */
        float nw = text_width(ct->name) + 6;
        px_rect_a(r, ct->x + 4, ct->y + 3, nw, 9, KZ_CLOUD, 210);
        text_draw(r, ct->name, ct->x + 6, ct->y + 4, rgb(0x86, 0x72, 0x82));
    }

    /* ---- scattered trees for woodland texture ---- */
    static const float TREES[][2] = {
        {20, 96}, {30, 108}, {174, 40}, {186, 100}, {206, 44},
        {70, 30}, {96, 92}, {150, 118}, {14, 64}, {222, 120},
    };
    for (int t = 0; t < (int)(sizeof TREES / sizeof TREES[0]); t++)
        map_tree(r, TREES[t][0], TREES[t][1], frame, t);

    /* ---- dirt paths between neighboring places ---- */
    for (int i = 0; i + 1 < MAP_PLACE_COUNT; i++) {
        float ax = PLACES[i].x, ay = PLACES[i].y;
        float bx = PLACES[i + 1].x, by = PLACES[i + 1].y;
        for (float t = 0.06f; t < 0.94f; t += 0.09f) {
            float dx = ax + (bx - ax) * t, dy = ay + (by - ay) * t;
            px_rect(r, dx, dy, 2, 2, rgb(0xD8, 0xC4, 0xA6));      /* stepping stones */
        }
    }

    /* ---- place pins ---- */
    for (int i = 0; i < MAP_PLACE_COUNT; i++) {
        Color pin = (i == current) ? KZ_HEART : KZ_COCOA;
        draw_pin(r, PLACES[i].x, PLACES[i].y, pin, i == current);
        /* label on a soft chip for readability over terrain */
        float lw = text_width(PLACES[i].name) + 4;
        px_rect_a(r, PLACES[i].x - lw / 2, PLACES[i].y + 5, lw, 8, KZ_CLOUD, 210);
        text_draw_centered(r, PLACES[i].name, PLACES[i].x,
                           PLACES[i].y + 6, KZ_COCOA);
    }

    /* ---- the moving cursor: a gently pulsing ring around the selected place ---- */
    if (selected >= 0 && selected < MAP_PLACE_COUNT) {
        float px_ = PLACES[selected].x, py_ = PLACES[selected].y;
        float pulse = 7.0f + sinf((float)frame * 0.12f) * 1.5f;
        Color ring = KZ_PETAL_PINK;
        px_rect(r, px_ - pulse, py_ - pulse, pulse * 2, 1, ring);
        px_rect(r, px_ - pulse, py_ + pulse, pulse * 2, 1, ring);
        px_rect(r, px_ - pulse, py_ - pulse, 1, pulse * 2, ring);
        px_rect(r, px_ + pulse, py_ - pulse, 1, pulse * 2 + 1, ring);
    }

    /* ---- corner plaque with the selected place's name ---- */
    const char *nm = (selected >= 0 && selected < MAP_PLACE_COUNT)
                     ? PLACES[selected].name : "";
    float pw = text_width(nm) + 12;
    float px0 = KZ_W - pw - 6, py0 = KZ_H - 20;
    px_rect(r, px0, py0, pw, 14, KZ_CLOUD);
    px_rect(r, px0, py0, pw, 1, KZ_COCOA);
    px_rect(r, px0, py0 + 13, pw, 1, KZ_COCOA);
    px_rect(r, px0, py0, 1, 14, KZ_COCOA);
    px_rect(r, px0 + pw - 1, py0, 1, 14, KZ_COCOA);
    text_draw(r, nm, px0 + 6, py0 + 4, KZ_COCOA);

    /* a hint at the bottom-left, on a chip */
    px_rect_a(r, 4, KZ_H - 11, 118, 10, KZ_CLOUD, 200);
    text_draw(r, "arrows or tap  -  A/tap to go", 6, KZ_H - 9, KZ_COCOA);
}