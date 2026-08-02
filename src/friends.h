/* friends.h — the cats you meet out in the world.
 *
 * These are NOT your cats. Your own family lives in the Roster (up to 5,
 * adopted with the + button). Friends are the strays and neighbors you meet
 * on walks: you offer treats, earn their trust, and once trust is full they
 * become a lasting friend you can revisit in your friends list. Befriending
 * never adds a cat to your roster — it's the warm social reward of walking.
 *
 * A Friends collection remembers everyone you've met (by a stable id), how
 * much they trust you, and whether they've become a full friend yet.
 */
#ifndef KATIZTIC_FRIENDS_H
#define KATIZTIC_FRIENDS_H

#include <SDL3/SDL.h>
#include "cattype.h"
#include "stats.h"   /* for KZ_STAT_MAX as the trust ceiling */

#define KZ_MAX_FRIENDS   16
#define KZ_FRIEND_NAME   12
#define KZ_TRUST_FULL    KZ_STAT_MAX   /* trust needed to become a friend */

typedef struct {
    char    name[KZ_FRIEND_NAME];
    CatType type;
    Uint8   trust;       /* 0..KZ_TRUST_FULL                    */
    bool    befriended;  /* true once trust first reached full  */
} Friend;

typedef struct {
    Friend list[KZ_MAX_FRIENDS];
    int    count;
} Friends;

Friends friends_new(void);

/* Find a friend by name, or NULL if not met yet. */
Friend *friends_find(Friends *f, const char *name);

/* Record meeting a cat (adds them at trust 0 if new). Returns the entry, or
 * NULL if the collection is full. Safe to call repeatedly for the same cat. */
Friend *friends_meet(Friends *f, const char *name, CatType type);

/* Offer a treat to a named cat: raises trust. Returns true if this offer is
 * the one that tips them into full friendship (for the "joined" moment). */
bool friends_offer_treat(Friends *f, const char *name);

/* Pet a wild cat: a gentler trust gain than a treat. Returns true if this pet
 * is the one that tips her into full friendship. */
bool friends_pet(Friends *f, const char *name);

/* How many have become full friends. */
int friends_befriended_count(const Friends *f);

/* ---- persistence: its own file, separate from the roster save ---- */
bool friends_save(const Friends *f, const char *path);
bool friends_load(Friends *out, const char *path);

#endif /* KATIZTIC_FRIENDS_H */