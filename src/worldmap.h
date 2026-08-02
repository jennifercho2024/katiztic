/* worldmap.h — the map of Katiztic's world.
 *
 * Instead of a little dropdown, travel happens on a map: soft city regions
 * with places dotted inside them. You move a cursor between places with the
 * arrow keys (or just tap one), the selected place's name shows in a corner,
 * and choosing it asks a gentle "Go here?" before you travel.
 *
 * The map is the foundation the growing world plugs into — new places (grocery
 * stores, the park, the Olympics) join a city simply by being added to the
 * tables here.
 */
#ifndef KATIZTIC_WORLDMAP_H
#define KATIZTIC_WORLDMAP_H

#include <SDL3/SDL.h>

/* Places must match main's Location enum values (same order). Kept as plain
 * ints here so the map doesn't depend on main.c. */
#define MAP_PLACE_COUNT 5

/* A place pinned on the map. */
typedef struct {
    const char *name;    /* "Cottage", "Cafe"...           */
    int         city;    /* which city region it belongs to */
    float       x, y;    /* dot position on the 240x160 map */
} MapPlace;

/* A soft city region drawn as a rounded pastel blob. */
typedef struct {
    const char *name;    /* "Pearl City"...                 */
    float       x, y, w, h;
    Uint8       r, g, b; /* region tint                     */
} MapCity;

int             map_place_count(void);
const MapPlace *map_place(int i);
int             map_city_count(void);
const MapCity  *map_city(int i);

/* Move the cursor to the nearest place in a direction (dx,dy in {-1,0,1}).
 * Returns the new selected index. */
int map_move(int selected, int dx, int dy);

/* Which place dot (if any) is under a tapped point; -1 if none. */
int map_hit(float px, float py);

/* Draw the whole map: city regions, place dots, the cursor on `selected`,
 * `current` marked as where you are, and the selected place's name in a
 * corner plaque. */
void map_draw(SDL_Renderer *r, int selected, int current, Uint64 frame);

#endif /* KATIZTIC_WORLDMAP_H */