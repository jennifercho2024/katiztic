/* tricks.h — tricks your cats learn through practice.
 *
 * Five tricks: Sit, Spin, Jump, High-five, and Roll. Each cat has a skill
 * level per trick (0 = doesn't know it yet, rising to mastered with practice).
 * Practicing a trick a little raises that skill; once mastered, the cat can
 * show it off at playdates and, later, compete at the Olympics.
 *
 * Skills are stored per cat by name (matching the roster) with their own save
 * file, so the roster's own save format stays untouched.
 */
#ifndef KATIZTIC_TRICKS_H
#define KATIZTIC_TRICKS_H

#include <SDL3/SDL.h>

typedef enum {
    TRICK_SIT,
    TRICK_SPIN,
    TRICK_JUMP,
    TRICK_HIGHFIVE,
    TRICK_ROLL,
    TRICK_COUNT
} TrickId;

#define TRICK_MASTER   100    /* skill at which a trick is mastered */
#define TRICK_NAME_LEN 16

/* One cat's skills, keyed by name. */
typedef struct {
    char  name[TRICK_NAME_LEN];
    Uint8 skill[TRICK_COUNT];   /* 0..TRICK_MASTER */
    bool  used;
} CatTricks;

#define TRICKS_MAX_CATS 7

typedef struct {
    CatTricks cats[TRICKS_MAX_CATS];
} Tricks;

const char *trick_name(TrickId t);

Tricks tricks_new(void);

/* Skill of a named cat in a trick (0 if unknown cat/trick). */
int tricks_skill(const Tricks *tr, const char *cat, TrickId t);

/* Is this trick mastered by the named cat? */
bool tricks_mastered(const Tricks *tr, const char *cat, TrickId t);

/* How many tricks the named cat has mastered. */
int tricks_mastered_count(const Tricks *tr, const char *cat);

/* Practice a trick with a named cat: raises skill a little (more if you have a
 * treat to reward them). Adds the cat if new. Returns:
 *   1 if this practice just mastered the trick,
 *   0 for normal progress. */
int tricks_practice(Tricks *tr, const char *cat, TrickId t, bool have_treat);

bool tricks_save(const Tricks *tr, const char *path);
bool tricks_load(Tricks *tr, const char *path);

#endif /* KATIZTIC_TRICKS_H */