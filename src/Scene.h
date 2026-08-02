/* scene.h — the meadow, and the time-of-day system that gives it mood.
 *
 * The single biggest "ethereal" lever in Katiztic is time-of-day tinting:
 * the same meadow, recolored, reads as four different feelings. A TimeOfDay
 * bundles the palette for one moment (sky bands, grass, hills) plus the
 * soft-light wash color and its strength.
 */
#ifndef KATIZTIC_SCENE_H
#define KATIZTIC_SCENE_H

#include <SDL3/SDL.h>
#include "palette.h"

typedef struct {
    const char *name;
    Color sky_top, sky_mid, sky_bot;  /* vertical sky gradient bands */
    Color grass, grass2;              /* near grass, shaded grass    */
    Color hill;                       /* distant hills               */
    Color wash;                       /* soft-light overlay color    */
    Uint8 wash_alpha;                 /* overlay strength 0..255     */
} TimeOfDay;

/* The four moments. Index with a TimeIndex; cycle with (i+1)%KZ_TIME_COUNT. */
typedef enum { KZ_DAWN, KZ_NOON, KZ_DUSK, KZ_NIGHT, KZ_TIME_COUNT } TimeIndex;

const TimeOfDay *time_of_day(TimeIndex i);

/* Map a real local hour (0–23) to a time-of-day phase, so the game's lighting
 * follows the actual time of day: dawn in the early morning, noon through the
 * day, dusk in the evening, night after dark. */
TimeIndex time_from_hour(int hour);

/* Drifting petals live with the scene. A fixed pool, no allocation. */
#define KZ_PETAL_COUNT 14
typedef struct { float x, y, speed, drift; int size; } Petal;

typedef struct {
    TimeIndex time;
    Petal petals[KZ_PETAL_COUNT];
} Meadow;

Meadow meadow_make(void);
void   meadow_update(Meadow *m);                 /* drift the petals   */
void   meadow_cycle_time(Meadow *m);             /* advance the clock  */
void   meadow_draw(SDL_Renderer *r, const Meadow *m, Uint64 frame);

/* Drawn last, over everything, as the mood wash. */
void   meadow_draw_wash(SDL_Renderer *r, const Meadow *m);

#endif /* KATIZTIC_SCENE_H */