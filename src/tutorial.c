/* tutorial.c — see tutorial.h. A friendly first-launch walkthrough. */
#include "tutorial.h"
#include "render.h"
#include "palette.h"
#include "text.h"

/* Each card: a title and up to three short lines of body text. */
typedef struct { const char *title; const char *l1, *l2, *l3; } Card;

static const Card CARDS[] = {
    { "Welcome to Katiztic!",
      "A cozy world of cats to raise,",
      "play with, and love.",
      "Tap to continue..." },
    { "Meet your cats",
      "Tap a cat to pet it.",
      "The strip at the bottom-left",
      "shows your whole family." },
    { "Teach tricks",
      "Press and HOLD a cat for 3s",
      "to open the trick trainer.",
      "A bar fills to show progress." },
    { "Carry your cats",
      "Keep holding a cat for 5s to",
      "pick it up, then drag it",
      "anywhere and release to place." },
    { "Feed and decorate",
      "At home, use the food and",
      "decor buttons on the right to",
      "feed cats and furnish the room." },
    { "Zoom and pan",
      "At home, +/- buttons zoom.",
      "Drag empty space to pan the",
      "cottage, meadow, and park." },
    { "Explore the world",
      "Tap the top-right button to",
      "open the map and travel: cafe,",
      "park, forest, store, and more." },
    { "Walks and games",
      "Hold left/right at the park to",
      "walk your cat. Enter the",
      "Katlympics to compete for medals!" },
    { "Have fun!",
      "Your cats grow as you care for",
      "them. There's lots to discover.",
      "Tap to begin your adventure!" },
};
#define CARD_COUNT ((int)(sizeof CARDS / sizeof CARDS[0]))

Tutorial tutorial_new(void) {
    Tutorial t = { true, 0, CARD_COUNT };
    return t;
}

/* the skip button sits at the top-right of the card */
#define SKIP_X 176
#define SKIP_Y 40
#define SKIP_W 52
#define SKIP_H 12

void tutorial_draw(SDL_Renderer *r, const Tutorial *t, Uint64 frame) {
    (void)frame;
    if (!t->active || t->step >= t->total) return;
    const Card *c = &CARDS[t->step];

    /* dim the whole screen so the card stands out */
    px_rect_a(r, 0, 0, KZ_W, KZ_H, rgb(0x2E, 0x28, 0x40), 150);

    /* the card */
    float cx = 20, cy = 36, cw = KZ_W - 40, ch = 92;
    px_rect(r, cx + 2, cy + 2, cw, ch, rgb(0x2E, 0x28, 0x40));   /* shadow */
    px_rect(r, cx, cy, cw, ch, KZ_CLOUD);                         /* fill */
    /* pink header band */
    px_rect(r, cx, cy, cw, 16, KZ_PETAL_PINK);
    px_rect(r, cx, cy, cw, 1, KZ_COCOA);
    px_rect(r, cx, cy + ch - 1, cw, 1, KZ_COCOA);
    px_rect(r, cx, cy, 1, ch, KZ_COCOA);
    px_rect(r, cx + cw - 1, cy, 1, ch, KZ_COCOA);

    text_draw(r, c->title, cx + 6, cy + 5, KZ_COCOA);
    if (c->l1) text_draw(r, c->l1, cx + 6, cy + 24, KZ_COCOA);
    if (c->l2) text_draw(r, c->l2, cx + 6, cy + 36, KZ_COCOA);
    if (c->l3) text_draw(r, c->l3, cx + 6, cy + 48, rgb(0x86, 0x72, 0x82));

    /* progress dots */
    for (int i = 0; i < t->total; i++) {
        Color d = (i == t->step) ? KZ_HEART : rgb(0xD8, 0xCC, 0xD8);
        px_rect(r, cx + 6 + i * 6, cy + ch - 10, 3, 3, d);
    }

    /* step counter + tap hint on the right */
    char nn[16];
    SDL_snprintf(nn, sizeof nn, "%d/%d", t->step + 1, t->total);
    text_draw(r, nn, cx + cw - 30, cy + ch - 11, rgb(0x86, 0x72, 0x82));

    /* skip button */
    px_rect(r, SKIP_X, SKIP_Y, SKIP_W, SKIP_H, KZ_CLOUD);
    px_rect(r, SKIP_X, SKIP_Y, SKIP_W, 1, KZ_COCOA);
    px_rect(r, SKIP_X, SKIP_Y + SKIP_H - 1, SKIP_W, 1, KZ_COCOA);
    px_rect(r, SKIP_X, SKIP_Y, 1, SKIP_H, KZ_COCOA);
    px_rect(r, SKIP_X + SKIP_W - 1, SKIP_Y, 1, SKIP_H, KZ_COCOA);
    text_draw_centered(r, "skip", SKIP_X + SKIP_W / 2.0f, SKIP_Y + 3, KZ_COCOA);
}

bool tutorial_tap(Tutorial *t, float px_, float py_) {
    if (!t->active) return false;
    /* skip button ends the whole tutorial */
    if (px_ >= SKIP_X && px_ <= SKIP_X + SKIP_W
        && py_ >= SKIP_Y && py_ <= SKIP_Y + SKIP_H) {
        t->active = false;
        return true;
    }
    /* any other tap advances to the next card */
    t->step++;
    if (t->step >= t->total) t->active = false;
    return true;
}