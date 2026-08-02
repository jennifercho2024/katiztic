/* mood.c — see mood.h. Choosing an emoji from the cat's state, and drawing
 * the little bubble. */
#include "mood.h"
#include "render.h"
#include "palette.h"
#include <math.h>

#define BUBBLE_FRAMES 110    /* how long a bubble stays up */

/* Pick a mood that fits what she's doing and feeling. Activity comes first
 * (it's the clearest signal), then stats fill in for calmer moments. */
static MoodKind choose_mood(const Cat *cat, const Stats *s) {
    switch (cat->act) {
        case ACT_PLAY:  return MOOD_NOTE;
        case ACT_GROOM: return MOOD_SPARKLE;
        case ACT_SLEEP: return MOOD_SLEEP;
        default: break;
    }
    /* Sitting / walking: reflect how she feels. */
    if (s->energy < 30) return MOOD_FOOD;      /* peckish */
    return MOOD_HEART;                          /* a happy, content cat */
}

void mood_update(Cat *cat, const Stats *s) {
    if (cat->mood_timer > 0) {
        cat->mood_timer--;
        return;
    }
    if (cat->mood_next > 0) {
        cat->mood_next--;
        return;
    }
    /* time for a new bubble */
    cat->mood_kind  = (int)choose_mood(cat, s);
    cat->mood_timer = BUBBLE_FRAMES;
    cat->mood_next  = 300 + SDL_rand(360);   /* 5–11s until the next one */
}

/* ---- tiny emoji glyphs, ~7x7, drawn at (x,y) ---- */

static void emoji_heart(SDL_Renderer *r, float x, float y, Uint8 a) {
    px_rect_a(r, x + 1, y,     2, 1, KZ_HEART, a);
    px_rect_a(r, x + 4, y,     2, 1, KZ_HEART, a);
    px_rect_a(r, x,     y + 1, 7, 2, KZ_HEART, a);
    px_rect_a(r, x + 1, y + 3, 5, 1, KZ_HEART, a);
    px_rect_a(r, x + 2, y + 4, 3, 1, KZ_HEART, a);
    px_rect_a(r, x + 3, y + 5, 1, 1, KZ_HEART, a);
}

static void emoji_note(SDL_Renderer *r, float x, float y, Uint8 a) {
    Color c = rgb(0x9C, 0x88, 0xC0);   /* soft violet note */
    px_rect_a(r, x + 4, y,     2, 5, c, a);   /* stem */
    px_rect_a(r, x + 1, y + 4, 4, 3, c, a);   /* head */
    px_rect_a(r, x + 5, y,     2, 2, c, a);   /* flag */
}

static void emoji_sparkle(SDL_Renderer *r, float x, float y, Uint8 a) {
    Color c = KZ_BUTTER;
    px_rect_a(r, x + 3, y,     1, 7, c, a);   /* vertical */
    px_rect_a(r, x,     y + 3, 7, 1, c, a);   /* horizontal */
    px_rect_a(r, x + 2, y + 2, 3, 3, KZ_CLOUD, a); /* bright center */
}

static void emoji_sleep(SDL_Renderer *r, float x, float y, Uint8 a) {
    Color c = KZ_COCOA;
    /* a little "z" */
    px_rect_a(r, x + 1, y + 1, 5, 1, c, a);
    px_rect_a(r, x + 3, y + 2, 2, 1, c, a);
    px_rect_a(r, x + 2, y + 3, 2, 1, c, a);
    px_rect_a(r, x + 1, y + 4, 5, 1, c, a);
}

static void emoji_food(SDL_Renderer *r, float x, float y, Uint8 a) {
    /* a little fish, matching the treat icon */
    Color c = rgb(0xE8, 0x8B, 0x6B);
    px_rect_a(r, x,     y + 2, 5, 3, c, a);   /* body */
    px_rect_a(r, x + 5, y + 1, 2, 2, c, a);   /* tail */
    px_rect_a(r, x + 5, y + 4, 2, 2, c, a);
    px_rect_a(r, x + 1, y + 3, 1, 1, KZ_CLOUD, a); /* eye */
}

void mood_draw(SDL_Renderer *r, const Cat *cat, Uint64 frame) {
    if (cat->mood_timer <= 0) return;

    /* progress 0..1 across the bubble's life */
    float t = 1.0f - (float)cat->mood_timer / (float)BUBBLE_FRAMES;
    float rise = t * 8.0f;               /* drift up gently */
    float fade;
    if (t < 0.2f)      fade = t / 0.2f;          /* fade in  */
    else if (t > 0.7f) fade = (1.0f - t) / 0.3f; /* fade out */
    else               fade = 1.0f;
    Uint8 a = (Uint8)(fade * 235.0f);

    float bx = cat->cx + 6;
    float by = cat->cy - 22 - rise;
    float sway = sinf((float)(frame + cat->act_seed) * 0.05f) * 1.5f;
    bx += sway;

    Uint8 ba = (Uint8)(a * 0.9f);
    /* soft rounded bubble backing */
    px_rect_a(r, bx,     by,     9, 9, KZ_CLOUD, ba);
    px_rect_a(r, bx - 1, by + 1, 1, 7, KZ_CLOUD, ba);
    px_rect_a(r, bx + 9, by + 1, 1, 7, KZ_CLOUD, ba);
    px_rect_a(r, bx + 1, by - 1, 7, 1, KZ_CLOUD, ba);
    px_rect_a(r, bx + 1, by + 9, 7, 1, KZ_CLOUD, ba);
    /* little tail toward the cat */
    px_rect_a(r, bx + 1, by + 10, 2, 1, KZ_CLOUD, ba);
    px_rect_a(r, bx,     by + 11, 1, 1, KZ_CLOUD, ba);

    /* the emoji, centered in the bubble */
    float ex = bx + 1, ey = by + 2;
    switch ((MoodKind)cat->mood_kind) {
        case MOOD_HEART:   emoji_heart(r, ex, ey, a);   break;
        case MOOD_NOTE:    emoji_note(r, ex, ey, a);    break;
        case MOOD_SPARKLE: emoji_sparkle(r, ex, ey, a); break;
        case MOOD_SLEEP:   emoji_sleep(r, ex, ey, a);   break;
        case MOOD_FOOD:    emoji_food(r, ex, ey, a);    break;
        default: break;
    }
}