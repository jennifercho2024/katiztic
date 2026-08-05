/* decor.h — cozy things to collect and arrange in your cottage.
 *
 * Décor items are unlocked by milestones you reach through normal play (a
 * strong bond, befriending cats, growing your family). Once unlocked, you can
 * place an item anywhere in the room by dragging it, and its position is
 * remembered. Décor is pure coziness — it has no stats effect, it just makes
 * the home feel like yours.
 */
#ifndef KATIZTIC_DECOR_H
#define KATIZTIC_DECOR_H

#include <SDL3/SDL.h>

/* The catalog of items. Keep this list and DECOR_INFO in decor.c in sync. */
typedef enum {
    DECOR_PLANT,     /* a potted plant                 */
    DECOR_LAMP,      /* a soft floor lamp              */
    DECOR_PICTURE,   /* a framed wall picture          */
    DECOR_TOWER,     /* a cat tower                    */
    DECOR_CUSHION,   /* a round floor cushion          */
    DECOR_RUG2,      /* a second little rug            */
    DECOR_YARN,      /* a ball of yarn — cats bat at it (earned)   */
    DECOR_MILK,      /* a saucer of milk — cats lap it (earned)    */
    DECOR_BOOKSHELF, /* a cozy bookshelf                */
    DECOR_CLOCK,     /* a wall clock                    */
    DECOR_BEANBAG,   /* a comfy bean bag                */
    DECOR_POST,      /* a scratching post               */
    DECOR_PLANT2,    /* a tall leafy plant              */
    DECOR_TABLE,     /* a little side table             */
    DECOR_COUNT
} DecorKind;

/* How an item is unlocked. Checked against the player's progress. */
typedef enum {
    UNLOCK_START,        /* available from the beginning        */
    UNLOCK_BOND,         /* a cat's bond reaches the threshold  */
    UNLOCK_FRIENDS,      /* befriend N cats on walks            */
    UNLOCK_FAMILY,       /* have N cats in your roster          */
    UNLOCK_LEVEL,        /* total levels across cats (socializing + care) */
    UNLOCK_STORE,        /* only obtainable by buying at the store */
} UnlockKind;

typedef struct {
    DecorKind   kind;
    const char *name;
    UnlockKind  unlock;
    int         threshold;   /* meaning depends on unlock kind */
} DecorInfo;

const DecorInfo *decor_info(DecorKind k);

/* The coin price to buy a piece of furniture at the department store. */
int decor_price(DecorKind k);

/* One placed item's saved state. */
typedef struct {
    bool  owned;     /* unlocked yet?                    */
    bool  placed;    /* currently shown in the room?     */
    float x, y;      /* position in logical 240x160 space */
} DecorItem;

typedef struct {
    DecorItem items[DECOR_COUNT];
} Decor;

/* A fresh décor set: nothing placed, only START items owned. */
Decor decor_new(void);

/* Re-check unlocks against current progress; newly-unlocked items become
 * owned (but not auto-placed). Returns the number newly unlocked this call,
 * so the caller can celebrate. `total_levels` is the sum of all cats' levels
 * (rises with socializing and care), gating the earned social items. */
int decor_check_unlocks(Decor *d, int max_bond, int friends_count,
                        int family_count, int total_levels);

/* Drawing: render every placed item into the cottage. */
void decor_draw(SDL_Renderer *r, const Decor *d, Uint64 frame);

/* Hit-test: which placed item is at (px,py), or -1. Topmost (last drawn)
 * wins so dragging grabs what you see. */
int  decor_hit(const Decor *d, float px, float py);

/* Draw one item at an arbitrary spot (used for the drag ghost + the tray). */
void decor_draw_one(SDL_Renderer *r, DecorKind k, float x, float y,
                    Uint64 frame);

/* Draw an item's preview neatly centered inside a tray slot of the given size,
 * so nothing spills outside the box regardless of the item's shape. */
void decor_draw_preview(SDL_Renderer *r, DecorKind k, float slot_x,
                        float slot_y, float slot_w, float slot_h, Uint64 frame);


/* ---- persistence: its own save file ---- */
bool decor_save(const Decor *d, const char *path);
bool decor_load(Decor *out, const char *path);

#endif /* KATIZTIC_DECOR_H */