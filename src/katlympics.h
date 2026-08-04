/* katlympics.h — the Katlympics: your cats compete in trick events and an
 * obstacle course against neighbors you've befriended.
 *
 * Everything you've built toward comes together here. Your active cat enters
 * events; how well they do depends on their mastered tricks and their stats
 * (energy, mood, bond). Rivals are the owners you've met around town, each with
 * their own cat. Place well to earn medals, coins, and XP for the whole family.
 *
 * The flow: pick an event -> watch it play out -> see the results and medals.
 */
#ifndef KATIZTIC_KATLYMPICS_H
#define KATIZTIC_KATLYMPICS_H

#include <SDL3/SDL.h>
#include "tricks.h"
#include "stats.h"
#include "cattype.h"

typedef enum {
    EVENT_TRICKS,      /* a trick showcase — perform your mastered tricks   */
    EVENT_OBSTACLE,    /* an obstacle course — jump, weave, tunnel, slide   */
    EVENT_COUNT
} EventId;

const char *event_name(EventId e);

typedef enum { MEDAL_NONE, MEDAL_BRONZE, MEDAL_SILVER, MEDAL_GOLD } Medal;

#define KAT_RIVALS 3    /* how many rivals line up against you */

typedef struct {
    char  name[16];      /* the rival owner's name */
    CatType cat_type;
    int   score;
} Rival;

typedef struct {
    bool    active;       /* is an event running/showing?          */
    EventId event;
    int     phase;        /* 0 = intro, 1 = performing, 2 = results */
    int     timer;        /* frames in the current phase            */
    int     your_score;
    Medal   your_medal;
    int     place;        /* 1st..4th                               */
    Rival   rivals[KAT_RIVALS];
    int     coins_won;
    int     xp_won;
    /* obstacle course progress (for the little animation) */
    int     obstacle_step;   /* which obstacle the cat is on (0..3)  */
} Katlympics;

Katlympics katlympics_none(void);

/* Begin an event with your active cat. Computes your score from mastered
 * tricks + stats, rolls rival scores from owners you've met, ranks everyone,
 * and sets the medal/rewards. `owner_names`/`owner_types` are the befriended
 * owners to draw rivals from (may be fewer than KAT_RIVALS; the rest are
 * filled with friendly locals). */
Katlympics katlympics_begin(EventId event, const char *your_cat,
                            const Tricks *tr, const Stats *st,
                            const char *const *owner_names,
                            const CatType *owner_types, int owner_count);

/* Advance the event one frame. Returns true when the whole event is complete
 * (results shown and dismissed-ready). */
bool katlympics_update(Katlympics *k);

/* Draw the current event scene (intro / performance / results). `your_*`
 * describe your competing cat's look. */
void katlympics_draw(SDL_Renderer *r, const Katlympics *k,
                     CatType your_type, bool your_shiny, Uint64 frame);

/* Medal name for banners. */
const char *medal_name(Medal m);

#endif /* KATIZTIC_KATLYMPICS_H */