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
    Uint8 mood;     /* happiness — feeding and outings raise it            */
    Uint8 energy;   /* restored by feeding                                  */
    Uint8 growth;   /* slow long-term "care given" meter                    */
    Uint32 care_given; /* running count of care actions, feeds growth       */
    Uint16 level;   /* the cat's level — starts at 1, caps at 100          */
    Uint16 xp;      /* experience toward the next level                     */
} Stats;

/* XP needed to reach the next level. Grows a bit each level, so leveling
 * gently slows down but never stops. */
Uint16 stats_xp_for_level(Uint16 level);

/* A brand-new cat starts content, not empty — she's happy from the start. */
Stats stats_new(void);

/* Care actions. Each returns nothing; they just nudge stats up (capped)
 * and advance growth a touch. */
void stats_feed(Stats *s);    /* +energy, +a little mood            */

/* Feed a specific food (FoodKind as int): each gives a different stat blend.
 * Returns XP gained. */
int  stats_feed_food(Stats *s, int food);
void stats_groom(Stats *s);   /* +mood, +bond                       */
void stats_pet(Stats *s);     /* +bond (called when the cat is pet) */

/* A good night's sleep: a gentle mood lift (energy is refilled separately). */
void stats_wake(Stats *s);

/* Award experience for a positive action. Handles level-ups (possibly several
 * at once) internally. Returns the number of levels gained (0 if none), so the
 * caller can celebrate a "Level up!". */
int stats_gain_xp(Stats *s, int amount);

/* Taking a cat out (to the meadow or café) lifts her happiness. */
void stats_outing(Stats *s);

/* ---- persistence ----
 * A tiny binary save written next to the executable. Returns true on success.
 * Loading a missing/corrupt file just leaves `out` untouched and returns false,
 * so the caller can fall back to stats_new().
 */
bool stats_save(const Stats *s, const char *path);
bool stats_load(Stats *out, const char *path);

#endif /* KATIZTIC_STATS_H */