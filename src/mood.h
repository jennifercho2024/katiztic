/* mood.h — little emoji thought bubbles that show how a cat feels.
 *
 * Every so often a soft bubble drifts up over a cat with a tiny pixel emoji
 * chosen from what she's doing and how she's feeling: a heart when she's
 * happy, a music note when playing, a sparkle when grooming, sleepy z's when
 * napping, a food thought when her energy is low. The bubbles are gentle and
 * occasional — a glimpse of each cat's inner life, not constant chatter.
 */
#ifndef KATIZTIC_MOOD_H
#define KATIZTIC_MOOD_H

#include <SDL3/SDL.h>
#include "cat.h"
#include "stats.h"

/* The emoji kinds a bubble can show. */
typedef enum {
    MOOD_HEART,    /* happy / content / high bond */
    MOOD_NOTE,     /* playing, having fun         */
    MOOD_SPARKLE,  /* grooming, feeling fresh     */
    MOOD_SLEEP,    /* napping                     */
    MOOD_FOOD,     /* low energy, a bit hungry    */
    MOOD_KIND_COUNT
} MoodKind;

/* Advance a cat's mood-bubble timers and, when it's time, choose a bubble that
 * fits her current activity and stats. Call once per frame per cat. */
void mood_update(Cat *cat, const Stats *s);

/* Draw the cat's mood bubble if one is showing. `frame` drives the gentle
 * rise-and-fade. Drawn above the cat at (cat->cx, cat->cy). */
void mood_draw(SDL_Renderer *r, const Cat *cat, Uint64 frame);

#endif /* KATIZTIC_MOOD_H */