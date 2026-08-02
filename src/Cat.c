/* cat.c — see cat.h.
 *
 * The sprite is drawn from primitives (no image files yet) so the whole
 * project builds from source with zero assets. Coordinates are relative to
 * the cat's center, so moving the cat is just changing cx/cy.
 */
#include "cat.h"
#include "render.h"
#include "palette.h"
#include <math.h>

/* Blink cadence: a blink every ~2-5 seconds at 60fps. */
static int roll_next_blink(void) {
    return 120 + (SDL_rand(200));  /* 120..319 frames */
}

Cat cat_make(float cx, float cy) {
    Cat c = { cx, cy, 0, roll_next_blink(), 0,
              ACT_SIT, 120, cx, cy, 1, (Uint64)SDL_rand(100000),
              0, 120 + SDL_rand(240), 0 };
    return c;
}

void cat_update(Cat *cat) {
    if (cat->blink > 0) {
        cat->blink--;
    } else {
        cat->next_blink--;
        if (cat->next_blink <= 0) {
            cat->blink = 8;                 /* blink lasts 8 frames */
            cat->next_blink = roll_next_blink();
        }
    }
    if (cat->pet > 0) cat->pet--;
}

void cat_pet(Cat *cat) {
    cat->pet = 90;  /* ~1.5s of purring hearts */
}

bool cat_hit(const Cat *cat, float px_, float py_) {
    /* Generous tappable box around head+body. */
    float dx = px_ - cat->cx;
    float dy = py_ - cat->cy;
    return dx > -12 && dx < 12 && dy > -16 && dy < 22;
}

void cat_draw(SDL_Renderer *r, const Cat *cat, CatColors col, Uint64 frame) {
    float cx = cat->cx;
    Uint64 f = frame + cat->act_seed;   /* per-cat phase so they differ */

    /* Breathing: gentle vertical bob. Petting adds a faster purr wobble. */
    float bob  = sinf((float)f * 0.04f) * 1.2f;
    float purr = cat->pet > 0 ? sinf((float)frame * 0.5f) * 0.6f : 0.0f;

    /* Activity affects motion and pose. */
    float act_bob = 0.0f;      /* extra vertical motion for the activity */
    float head_dip = 0.0f;     /* grooming dips the head down            */
    bool  eyes_closed = false; /* sleeping / grooming close the eyes     */
    switch (cat->act) {
        case ACT_PLAY:
            act_bob = -fabsf(sinf((float)frame * 0.14f)) * 2.5f;  /* slow, soft bounce */
            break;
        case ACT_GROOM:
            head_dip = 3.0f;
            eyes_closed = ((frame / 20) % 2 == 0);   /* half-lidded licking */
            break;
        case ACT_SLEEP:
            eyes_closed = true;
            bob = sinf((float)f * 0.02f) * 0.8f;     /* slow sleep breathing */
            break;
        default: break;
    }
    float py = cat->cy + bob + act_bob;

    /* Soft contact shadow (mauve, low alpha — never a hard black blob). */
    px_rect_a(r, cx - 15, cat->cy + 19, 30, 4, KZ_COCOA, 46);

    /* Swishing tail. */
    float tw = sinf((float)f * 0.06f) * 3.0f;
    px_rect(r, cx + 11, py + 6,      4, 3, col.body);
    px_rect(r, cx + 14, py + 4 + tw, 3, 3, col.body);
    px_rect(r, cx + 16, py + 2 + tw, 3, 4, col.body);

    /* Body + darker belly band. */
    px_rect(r, cx - 9, py + 4,  20, 14, col.body);
    px_rect(r, cx - 9, py + 16, 20,  2, col.dark);

    /* Head offset combines the purr wobble and any grooming head-dip. */
    float ho = purr + head_dip;

    /* Head. */
    px_rect(r, cx - 8, py - 8 + ho, 16, 14, col.body);

    /* Ears (outer + inner). */
    px_rect(r, cx - 8, py - 12 + ho, 4, 5, col.body);
    px_rect(r, cx + 4, py - 12 + ho, 4, 5, col.body);
    px_rect(r, cx - 7, py - 11 + ho, 2, 3, col.ear);
    px_rect(r, cx + 5, py - 11 + ho, 2, 3, col.ear);

    /* Eyes: closed (a slit) when blinking, sleeping, or grooming; open ovals
     * otherwise. Outline + nose stay a fixed soft mauve. */
    if (cat->blink > 0 || eyes_closed) {
        px_rect(r, cx - 5, py - 3 + ho, 3, 1, KZ_CAT_OUTLINE);
        px_rect(r, cx + 2, py - 3 + ho, 3, 1, KZ_CAT_OUTLINE);
    } else {
        px_rect(r, cx - 5, py - 4 + ho, 2, 3, KZ_CAT_OUTLINE);
        px_rect(r, cx + 3, py - 4 + ho, 2, 3, KZ_CAT_OUTLINE);
    }

    /* Nose + cheek blush. */
    px_rect(r, cx - 1, py - 1 + ho, 2, 1, KZ_CAT_NOSE);
    px_rect(r, cx - 6, py - 1 + ho, 2, 1, col.cheek);
    px_rect(r, cx + 4, py - 1 + ho, 2, 1, col.cheek);

    /* Stripes. */
    px_rect(r, cx - 6, py + 6, 2, 6, col.dark);
    px_rect(r, cx - 2, py + 6, 2, 6, col.dark);
    px_rect(r, cx + 2, py + 6, 2, 6, col.dark);

    /* Front paws. */
    px_rect(r, cx - 8, py + 16, 4, 3, col.paw);
    px_rect(r, cx + 4, py + 16, 4, 3, col.paw);

    /* Floating hearts while being petted. */
    if (cat->pet > 0) {
        for (int h = 0; h < 3; h++) {
            float t  = (float)((frame + (Uint64)(h * 20)) % 60);
            float hy = py - 14 - t * 0.4f;
            float hx = cx + (float)(h - 1) * 7.0f
                     + sinf((float)(frame + (Uint64)(h * 30)) * 0.08f) * 3.0f;
            Uint8 a = (Uint8)((1.0f - t / 60.0f) * 220.0f);
            /* tiny pixel heart */
            px_rect_a(r, hx - 1, hy,     1, 1, KZ_HEART, a);
            px_rect_a(r, hx + 1, hy,     1, 1, KZ_HEART, a);
            px_rect_a(r, hx - 1, hy + 1, 3, 1, KZ_HEART, a);
            px_rect_a(r, hx,     hy + 2, 1, 1, KZ_HEART, a);
        }
    }

    /* Sleeping: drifting "z" marks rising from the cat. */
    if (cat->act == ACT_SLEEP) {
        for (int zi = 0; zi < 2; zi++) {
            float t = (float)((f + (Uint64)(zi * 45)) % 90);
            float zy = py - 10 - t * 0.25f;
            float zx = cx + 8 + zi * 3 + sinf(t * 0.05f) * 2.0f;
            Uint8 a = (Uint8)((1.0f - t / 90.0f) * 200.0f);
            /* a tiny 3x3 "z" */
            px_rect_a(r, zx,     zy,     3, 1, KZ_COCOA, a);
            px_rect_a(r, zx + 1, zy + 1, 1, 1, KZ_COCOA, a);
            px_rect_a(r, zx,     zy + 2, 3, 1, KZ_COCOA, a);
        }
    }
}