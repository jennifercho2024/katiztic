/* worldmap.c — see worldmap.h. The world laid out, and how you move on it. */
#include "worldmap.h"
#include "render.h"
#include "palette.h"
#include "text.h"
#include <math.h>

/* Cities: soft regions of the world. Names chosen for a dreamy, pastel feel. */
static const MapCity CITIES[] = {
    { "Pearl City",  14,  30,  96, 54, 0xF0, 0xDD, 0xE8 },  /* home region   */
    { "Satin City",  128, 26, 100, 60, 0xDD, 0xE4, 0xF2 },  /* town region   */
    { "Fern Hollow", 60,  98, 120, 46, 0xD6, 0xE8, 0xD8 },  /* wild region   */
};
#define CITY_COUNT ((int)(sizeof CITIES / sizeof CITIES[0]))

/* Places, dotted inside their cities. Index order MUST match main's Location
 * enum: 0 Cottage, 1 Meadow, 2 Cafe, 3 Forest, 4 Street. */
static const MapPlace PLACES[MAP_PLACE_COUNT] = {
    { "Cottage",  0,  38,  50 },   /* Pearl City  */
    { "Meadow",   0,  84,  62 },   /* Pearl City  */
    { "Cafe",     1, 154,  48 },   /* Satin City  */
    { "Forest",   2, 100, 120 },   /* Fern Hollow */
    { "Street",   1, 196,  66 },   /* Satin City  */
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

void map_draw(SDL_Renderer *r, int selected, int current, Uint64 frame) {
    /* soft sky-parchment backdrop */
    px_rect(r, 0, 0, KZ_W, KZ_H, rgb(0xF6, 0xF1, 0xE8));
    /* a faint title */
    text_draw(r, "where to?", 8, 6, KZ_COCOA);

    /* city regions: rounded pastel blobs with their names */
    for (int c = 0; c < CITY_COUNT; c++) {
        const MapCity *ct = &CITIES[c];
        Color fill = rgb(ct->r, ct->g, ct->b);
        /* body */
        px_rect(r, ct->x + 3, ct->y, ct->w - 6, ct->h, fill);
        px_rect(r, ct->x, ct->y + 3, ct->w, ct->h - 6, fill);
        /* soft rounded corners */
        px_rect(r, ct->x + 1, ct->y + 1, 3, 3, fill);
        px_rect(r, ct->x + ct->w - 4, ct->y + 1, 3, 3, fill);
        px_rect(r, ct->x + 1, ct->y + ct->h - 4, 3, 3, fill);
        px_rect(r, ct->x + ct->w - 4, ct->y + ct->h - 4, 3, 3, fill);
        /* city name, tucked at the top-left inside */
        text_draw(r, ct->name, ct->x + 6, ct->y + 4,
                  rgb(0x9A, 0x86, 0x94));
    }

    /* little dotted paths between neighboring places (gentle, decorative) */
    for (int i = 0; i + 1 < MAP_PLACE_COUNT; i++) {
        float ax = PLACES[i].x, ay = PLACES[i].y;
        float bx = PLACES[i + 1].x, by = PLACES[i + 1].y;
        for (float t = 0.1f; t < 0.9f; t += 0.2f) {
            float dx = ax + (bx - ax) * t, dy = ay + (by - ay) * t;
            px_rect(r, dx, dy, 1, 1, rgb(0xCF, 0xC2, 0xB4));
        }
    }

    /* place pins */
    for (int i = 0; i < MAP_PLACE_COUNT; i++) {
        Color pin = (i == current) ? KZ_HEART : KZ_COCOA;
        draw_pin(r, PLACES[i].x, PLACES[i].y, pin, i == current);
        /* small label under each pin */
        text_draw_centered(r, PLACES[i].name, PLACES[i].x,
                           PLACES[i].y + 6, KZ_COCOA);
    }

    /* the moving cursor: a gently pulsing ring around the selected place */
    if (selected >= 0 && selected < MAP_PLACE_COUNT) {
        float px_ = PLACES[selected].x, py_ = PLACES[selected].y;
        float pulse = 7.0f + sinf((float)frame * 0.12f) * 1.5f;
        Color ring = KZ_PETAL_PINK;
        px_rect(r, px_ - pulse, py_ - pulse, pulse * 2, 1, ring);
        px_rect(r, px_ - pulse, py_ + pulse, pulse * 2, 1, ring);
        px_rect(r, px_ - pulse, py_ - pulse, 1, pulse * 2, ring);
        px_rect(r, px_ + pulse, py_ - pulse, 1, pulse * 2 + 1, ring);
    }

    /* corner plaque with the selected place's name */
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

    /* a hint at the bottom-left */
    text_draw(r, "arrows or tap  -  A/tap to go", 6, KZ_H - 9, KZ_COCOA);
}