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
    Cat c = { cx, cy, 0, roll_next_blink(), 0 };
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
 
void cat_draw(SDL_Renderer *r, const Cat *cat, Uint64 frame) {
    float cx = cat->cx;
    /* Breathing: gentle vertical bob. Petting adds a faster purr wobble. */
    float bob  = sinf((float)frame * 0.04f) * 1.2f;
    float purr = cat->pet > 0 ? sinf((float)frame * 0.5f) * 0.6f : 0.0f;
    float py   = cat->cy + bob;
 
    /* Soft contact shadow (mauve, low alpha — never a hard black blob). */
    px_rect_a(r, cx - 15, cat->cy + 19, 30, 4, KZ_COCOA, 46);
 
    /* Swishing tail. */
    float tw = sinf((float)frame * 0.06f) * 3.0f;
    px_rect(r, cx + 11, py + 6,      4, 3, KZ_CAT_BODY);
    px_rect(r, cx + 14, py + 4 + tw, 3, 3, KZ_CAT_BODY);
    px_rect(r, cx + 16, py + 2 + tw, 3, 4, KZ_CAT_BODY);
 
    /* Body + darker belly band. */
    px_rect(r, cx - 9, py + 4,  20, 14, KZ_CAT_BODY);
    px_rect(r, cx - 9, py + 16, 20,  2, KZ_CAT_DARK);
 
    /* Head. */
    px_rect(r, cx - 8, py - 8 + purr, 16, 14, KZ_CAT_BODY);
 
    /* Ears (outer + inner). */
    px_rect(r, cx - 8, py - 12 + purr, 4, 5, KZ_CAT_BODY);
    px_rect(r, cx + 4, py - 12 + purr, 4, 5, KZ_CAT_BODY);
    px_rect(r, cx - 7, py - 11 + purr, 2, 3, KZ_CAT_EAR);
    px_rect(r, cx + 5, py - 11 + purr, 2, 3, KZ_CAT_EAR);
 
    /* Eyes: a slit line when blinking, tall ovals when open. */
    if (cat->blink > 0) {
        px_rect(r, cx - 5, py - 3 + purr, 3, 1, KZ_CAT_OUTLINE);
        px_rect(r, cx + 2, py - 3 + purr, 3, 1, KZ_CAT_OUTLINE);
    } else {
        px_rect(r, cx - 5, py - 4 + purr, 2, 3, KZ_CAT_OUTLINE);
        px_rect(r, cx + 3, py - 4 + purr, 2, 3, KZ_CAT_OUTLINE);
    }
 
    /* Nose + cheek blush. */
    px_rect(r, cx - 1, py - 1 + purr, 2, 1, KZ_CAT_NOSE);
    px_rect(r, cx - 6, py - 1 + purr, 2, 1, KZ_CAT_CHEEK);
    px_rect(r, cx + 4, py - 1 + purr, 2, 1, KZ_CAT_CHEEK);
 
    /* Stripes. */
    px_rect(r, cx - 6, py + 6, 2, 6, KZ_CAT_DARK);
    px_rect(r, cx - 2, py + 6, 2, 6, KZ_CAT_DARK);
    px_rect(r, cx + 2, py + 6, 2, 6, KZ_CAT_DARK);
 
    /* Front paws. */
    px_rect(r, cx - 8, py + 16, 4, 3, KZ_CAT_PAW);
    px_rect(r, cx + 4, py + 16, 4, 3, KZ_CAT_PAW);
 
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
}
 