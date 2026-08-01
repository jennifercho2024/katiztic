/* stats.h — the cat's inner life: bond, mood, energy, growth.
 *
 * Design rule (cozy, non-punishing): stats ONLY change when the player acts.
 * Nothing decays over time. Being away from the game never costs you anything;
 * every change is the result of a kindness. There is no way to make the cat
 * sad through neglect — only happier through care.
 *
 * Each stat is a byte, 0..100. Care actions nudge them up and stop at 100.
 * `growth` is the slow long-term "level": it rises a little every time you
 * care for the cat, and never resets.
 */
#ifndef KATIZTIC_STATS_H
#define KATIZTIC_STATS_H

#include <SDL3/SDL.h>

#define KZ_STAT_MAX 100

typedef struct {
    Uint8 bond;     /* the headline stat — grows with grooming and petting */
    Uint8 mood;     /* how content she is — feeding and grooming raise it   */
    Uint8 energy;   /* restored by feeding                                  */
    Uint8 growth;   /* slow long-term level; care accumulates here          */
    Uint32 care_given; /* running count of care actions, feeds growth       */
} Stats;

/* A brand-new cat starts content, not empty — she's happy from the start. */
Stats stats_new(void);

/* Care actions. Each returns nothing; they just nudge stats up (capped)
 * and advance growth a touch. */
void stats_feed(Stats *s);    /* +energy, +a little mood            */
void stats_groom(Stats *s);   /* +mood, +bond                       */
void stats_pet(Stats *s);     /* +bond (called when the cat is pet) */

/* ---- persistence ----
 * A tiny binary save written next to the executable. Returns true on success.
 * Loading a missing/corrupt file just leaves `out` untouched and returns false,
 * so the caller can fall back to stats_new().
 */
bool stats_save(const Stats *s, const char *path);
bool stats_load(Stats *out, const char *path);

#endif /* KATIZTIC_STATS_H */