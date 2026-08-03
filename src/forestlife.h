/* forestlife.h — the woodland creatures your cats meet in the forest.
 *
 * A couple of animals wander the forest at a time — rabbits, birds, deer, and
 * the occasional gentle bear. Your cat can approach and interact with each,
 * building trust in a way that suits the animal: rabbits warm to gentle
 * repeated visits, birds need patience, deer are won with a treat, and the
 * shy bear takes the most time of all. Once trust is full, that animal is a
 * friend — recorded so you can see who your cats have befriended.
 */
#ifndef KATIZTIC_FORESTLIFE_H
#define KATIZTIC_FORESTLIFE_H

#include <SDL3/SDL.h>

typedef enum {
    ANIMAL_RABBIT,
    ANIMAL_BIRD,
    ANIMAL_DEER,
    ANIMAL_BEAR,
    ANIMAL_KIND_COUNT
} AnimalKind;

#define FORESTLIFE_MAX  2      /* a couple wandering at once (calm) */
#define ANIMAL_TRUST_FULL 100

typedef struct {
    bool       active;
    AnimalKind kind;
    float      x, y;
    int        dir;            /* facing/heading +1 or -1           */
    float      speed;
    float      wander_timer;   /* frames until it changes direction  */
    int        glow;           /* happy glow after an interaction     */
    int        hop;            /* rabbit/bird bob phase               */
} Animal;

/* Persistent befriending progress, one entry per animal kind (there's one
 * "species friend" per kind — befriend a rabbit and rabbits are your friends). */
typedef struct {
    Uint8 trust[ANIMAL_KIND_COUNT];      /* 0..ANIMAL_TRUST_FULL */
    bool  befriended[ANIMAL_KIND_COUNT];
} ForestFriends;

typedef struct {
    Animal        animals[FORESTLIFE_MAX];
    float         spawn_timer;
    ForestFriends friends;     /* trust per kind (saved)             */
} ForestLife;

/* names + how each is befriended (for banners) */
const char *animal_name(AnimalKind k);

ForestLife forestlife_new(void);

/* One frame: wander the animals, retire/spawn as they roam. */
void forestlife_update(ForestLife *fl, Uint64 frame);

/* Draw the animals (tinted by the forest's warmth filter is handled by caller;
 * these draw in their own colors). */
void forestlife_draw(SDL_Renderer *r, const ForestLife *fl, Uint64 frame);

/* Which animal is under a tapped point, or -1. */
int forestlife_hit(const ForestLife *fl, float px, float py);

/* Interact with the animal at `index`: builds trust in that kind's own way.
 * `have_treat` says whether the player has a treat to offer (for deer, etc.).
 * Writes a short result message into `msg`. Returns:
 *   1 if this interaction just befriended the kind,
 *   0 for normal progress,
 *  -1 if nothing happened (e.g. deer wanted a treat you don't have). */
int forestlife_interact(ForestLife *fl, int index, bool have_treat,
                        char *msg, size_t msglen);

/* How many kinds are befriended. */
int forestfriends_count(const ForestFriends *ff);

bool forestfriends_save(const ForestFriends *ff, const char *path);
bool forestfriends_load(ForestFriends *ff, const char *path);

#endif /* KATIZTIC_FORESTLIFE_H */