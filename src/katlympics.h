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

/* the actions a cat can take at each obstacle */
typedef enum {
    ACTION_JUMP,      /* leap over a hurdle          */
    ACTION_CRAWL,     /* crawl through a tunnel       */
    ACTION_ZIGZAG,    /* weave through poles          */
    ACTION_DASH,      /* sprint a straightaway        */
    ACTION_COUNT
} CourseAction;

const char *action_name(CourseAction a);

/* what each obstacle "wants" — matching action scores best */
#define KAT_OBSTACLES 4

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
    int     obstacle_step;   /* which obstacle the cat is on (0..3)  */
    /* --- the player's chosen performance --- */
    TrickId chosen_tricks[3];   /* the tricks to show (trick event)   */
    int     chosen_count;       /* how many tricks chosen (0..3)      */
    CourseAction chosen_actions[KAT_OBSTACLES];  /* action per obstacle */
} Katlympics;

Katlympics katlympics_none(void);

/* What action each obstacle rewards most (so the player can strategize). */
CourseAction katlympics_obstacle_wants(int obstacle);

/* Begin a TRICK SHOWCASE with the player's chosen tricks (up to 3). Score
 * reflects how well the cat performs those specific tricks. */
Katlympics katlympics_begin_tricks(const char *your_cat, const Tricks *tr,
                                   const Stats *st,
                                   const TrickId *chosen, int chosen_count,
                                   const char *const *owner_names,
                                   const CatType *owner_types, int owner_count);

/* Begin an OBSTACLE COURSE with the player's chosen action per obstacle.
 * Picking the action each obstacle rewards scores better. */
Katlympics katlympics_begin_obstacle(const char *your_cat, const Tricks *tr,
                                     const Stats *st,
                                     const CourseAction *actions,
                                     const char *const *owner_names,
                                     const CatType *owner_types, int owner_count);

/* Advance the event one frame. Returns true when the whole event is complete
 * (results shown and dismissed-ready). */
bool katlympics_update(Katlympics *k);

/* Draw the current event scene (intro / performance / results). `your_*`
 * describe your competing cat's look. */
void katlympics_draw(SDL_Renderer *r, const Katlympics *k,
                     CatType your_type, bool your_shiny, Uint64 frame);

/* Draw the ceremonial stadium background (stands, flags, banner, rings emblem)
 * — used for the event picker screen so it looks official from the start. */
void katlympics_draw_arena(SDL_Renderer *r, Uint64 frame);

/* Medal name for banners. */
const char *medal_name(Medal m);

#endif /* KATIZTIC_KATLYMPICS_H */