/* tutorial.h — a gentle first-time walkthrough of the game's mechanics.
 *
 * On the very first launch (no save yet) a series of friendly tip cards guides
 * the player through the core actions: petting, tricks, carrying cats, feeding,
 * décor, travelling the map, walks, and the Katlympics. Each card is dismissed
 * by a tap; after the last one the tutorial never shows again. The player can
 * also skip the whole thing at any time.
 */
#ifndef KATIZTIC_TUTORIAL_H
#define KATIZTIC_TUTORIAL_H

#include <SDL3/SDL.h>

typedef struct {
    bool active;     /* is the tutorial currently showing?     */
    int  step;       /* which card (0-based)                   */
    int  total;      /* how many cards there are               */
} Tutorial;

/* A fresh tutorial, ready to show from the first card. */
Tutorial tutorial_new(void);

/* Draw the current tip card (a soft overlay + card). Call last, over the UI. */
void tutorial_draw(SDL_Renderer *r, const Tutorial *t, Uint64 frame);

/* Handle a tap: returns true if the tap was consumed by the tutorial (advance
 * to the next card, or finish). Tapping the "skip" area ends it immediately. */
bool tutorial_tap(Tutorial *t, float px, float py);

#endif /* KATIZTIC_TUTORIAL_H */