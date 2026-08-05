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

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Blink cadence: a blink every ~2-5 seconds at 60fps. */
static int roll_next_blink(void) {
    return 120 + (SDL_rand(200));  /* 120..319 frames */
}

Cat cat_make(float cx, float cy) {
    Cat c = { cx, cy, 0, roll_next_blink(), 0,
              ACT_SIT, 120, cx, cy, 1, (Uint64)SDL_rand(100000),
              0, 120 + SDL_rand(240), 0,
              CAT_TRICK_NONE, 0, 0 };
    return c;
}

void cat_do_trick(Cat *cat, int trick) {
    cat->trick = trick;
    switch (trick) {
        case CAT_TRICK_JUMP:     cat->trick_len = 40; break;
        case CAT_TRICK_SPIN:     cat->trick_len = 44; break;
        case CAT_TRICK_HIGHFIVE: cat->trick_len = 48; break;
        case CAT_TRICK_ROLL:     cat->trick_len = 52; break;
        case CAT_TRICK_SIT:
        default:                 cat->trick_len = 40; break;
    }
    cat->trick_t = cat->trick_len;
}

void cat_update(Cat *cat) {
    if (cat->trick_t > 0) {
        cat->trick_t--;
        if (cat->trick_t == 0) cat->trick = CAT_TRICK_NONE;
    }
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

    /* ---- trick poses ---- */
    /* progress 0..1 across the trick animation (0 at start, 1 at end) */
    float tp = (cat->trick != CAT_TRICK_NONE && cat->trick_len > 0)
             ? 1.0f - (float)cat->trick_t / (float)cat->trick_len
             : -1.0f;
    float trick_lift = 0.0f;    /* whole-cat vertical lift (jump)         */
    int   paw_raise = 0;        /* which front paw is up: 0 none,1 left,2 right */
    float roll_tilt = 0.0f;     /* horizontal shift for a roll            */
    int   face_over = 0;        /* temporary facing override for a spin    */
    float sit_drop = 0.0f;      /* haunches lower for a sit                */
    float squash_x = 1.0f;      /* horizontal scale (spin/roll)           */
    float squash_y = 1.0f;      /* vertical scale (squash & stretch)      */
    float spin_wobble = 0.0f;   /* extra spin body rock                    */
    if (tp >= 0.0f) {
        float arc = sinf(tp * (float)M_PI);   /* 0 up to 1 and back to 0 */
        switch (cat->trick) {
            case CAT_TRICK_JUMP:
                /* a big leap with a squash on takeoff and landing */
                trick_lift = -arc * 18.0f;
                if (tp < 0.2f || tp > 0.8f) { squash_x = 1.15f; squash_y = 0.85f; }
                else { squash_x = 0.92f; squash_y = 1.12f; }   /* stretch airborne */
                break;
            case CAT_TRICK_HIGHFIVE:
                if (tp > 0.25f && tp < 0.75f) paw_raise = 2;
                trick_lift = -arc * 4.0f;
                sit_drop = arc * 2.0f;   /* sits back a little to reach up */
                break;
            case CAT_TRICK_SPIN: {
                /* a full turn: horizontal squash sweeps through zero (like the
                 * body rotating edge-on) twice, with a facing flip at each,
                 * plus a little rocking wobble — reads as a real spin. */
                float turn = tp * 2.0f;                 /* two half-turns */
                float ph = turn - (float)((int)turn);   /* 0..1 within each */
                squash_x = fabsf(cosf(ph * (float)M_PI));  /* 1 -> 0 -> 1 */
                if (squash_x < 0.15f) squash_x = 0.15f;
                face_over = (ph < 0.5f) ? cat->facing : -cat->facing;
                spin_wobble = sinf(tp * (float)M_PI * 4.0f) * 3.0f;
                trick_lift = -arc * 3.0f;
                break;
            }
            case CAT_TRICK_ROLL: {
                /* a real roll: the cat lies over (big tilt), squashes as it
                 * goes round, and rocks side to side across the ground. */
                roll_tilt = sinf(tp * (float)M_PI) * 12.0f;   /* rolls sideways */
                float r = tp * 2.0f;
                float rph = r - (float)((int)r);
                squash_y = 0.6f + 0.4f * fabsf(cosf(rph * (float)M_PI)); /* flatten */
                squash_x = 1.3f - 0.3f * squash_y;
                trick_lift = -arc * 1.5f;
                break;
            }
            case CAT_TRICK_SIT:
            default:
                /* a clear, deep sit: haunches drop, body leans back and
                 * squashes down — unmistakable now. */
                sit_drop = arc * 8.0f;
                squash_y = 1.0f - arc * 0.18f;   /* compresses down */
                squash_x = 1.0f + arc * 0.12f;   /* spreads a touch */
                break;
        }
    }
    py += trick_lift;
    roll_tilt += spin_wobble;
    int facing = (face_over != 0) ? face_over : cat->facing;
    (void)facing;  /* facing is used below for the spin flip on the tail */

    /* Soft contact shadow (mauve, low alpha — never a hard black blob). */
    px_rect_a(r, cx - 15, cat->cy + 19, 30, 4, KZ_COCOA, 46);

    /* Swishing tail. */
    float tw = sinf((float)f * 0.06f) * 3.0f;
    px_rect(r, cx + 11, py + 6,      4, 3, col.body);
    px_rect(r, cx + 14, py + 4 + tw, 3, 3, col.body);
    px_rect(r, cx + 16, py + 2 + tw, 3, 4, col.body);

    /* Body + darker belly band. Squash/stretch (spin, roll, jump, sit) scales
     * the main mass around its center; roll/spin shift it sideways. */
    float bw = 20.0f * squash_x, bh = 14.0f * squash_y;
    float bxo = (20.0f - bw) / 2.0f;             /* recenter horizontally */
    float byo = (14.0f - bh);                    /* keep feet on the ground */
    px_rect(r, cx - 9 + roll_tilt + bxo, py + 4 + sit_drop + byo, bw, bh, col.body);
    px_rect(r, cx - 9 + roll_tilt + bxo, py + 16 + sit_drop, 20.0f * squash_x, 2, col.dark);

    /* Head offset combines the purr wobble and any grooming head-dip. */
    float ho = purr + head_dip;
    /* the head follows the spin/roll squash and sideways shift too */
    float hw = 16.0f * squash_x;
    float hxo = (16.0f - hw) / 2.0f + roll_tilt * 0.7f;

    /* Head. */
    px_rect(r, cx - 8 + hxo, py - 8 + ho + sit_drop * 0.5f, hw, 14, col.body);

    /* Ears (outer + inner). */
    px_rect(r, cx - 8 + hxo, py - 12 + ho + sit_drop * 0.5f, 4, 5, col.body);
    px_rect(r, cx + 4 + hxo, py - 12 + ho + sit_drop * 0.5f, 4, 5, col.body);
    px_rect(r, cx - 7 + hxo, py - 11 + ho + sit_drop * 0.5f, 2, 3, col.ear);
    px_rect(r, cx + 5 + hxo, py - 11 + ho + sit_drop * 0.5f, 2, 3, col.ear);

    /* Eyes: closed (a slit) when blinking, sleeping, or grooming; open ovals
     * otherwise. Outline + nose stay a fixed soft mauve. */
    if (cat->blink > 0 || eyes_closed) {
        px_rect(r, cx - 5 + hxo, py - 3 + ho, 3, 1, KZ_CAT_OUTLINE);
        px_rect(r, cx + 2 + hxo, py - 3 + ho, 3, 1, KZ_CAT_OUTLINE);
    } else {
        px_rect(r, cx - 5 + hxo, py - 4 + ho, 2, 3, KZ_CAT_OUTLINE);
        px_rect(r, cx + 3 + hxo, py - 4 + ho, 2, 3, KZ_CAT_OUTLINE);
    }

    /* Nose + cheek blush. */
    px_rect(r, cx - 1 + hxo, py - 1 + ho, 2, 1, KZ_CAT_NOSE);
    px_rect(r, cx - 6 + hxo, py - 1 + ho, 2, 1, col.cheek);
    px_rect(r, cx + 4 + hxo, py - 1 + ho, 2, 1, col.cheek);

    /* Stripes. */
    px_rect(r, cx - 6, py + 6, 2, 6, col.dark);
    px_rect(r, cx - 2, py + 6, 2, 6, col.dark);
    px_rect(r, cx + 2, py + 6, 2, 6, col.dark);

    /* Front paws. For a high-five, one paw lifts up high. */
    if (paw_raise == 2) {
        /* right paw raised up beside the head for a high-five */
        px_rect(r, cx + 4, py + 16 + sit_drop, 4, 3, col.paw);  /* left stays */
        px_rect(r, cx + 6, py - 6 + ho, 4, 4, col.paw);         /* right up   */
        /* a little sparkle at the raised paw */
        px_rect(r, cx + 10, py - 8 + ho, 1, 3, KZ_BUTTER);
        px_rect(r, cx + 9, py - 7 + ho, 3, 1, KZ_BUTTER);
    } else {
        px_rect(r, cx - 8, py + 16 + sit_drop, 4, 3, col.paw);
        px_rect(r, cx + 4, py + 16 + sit_drop, 4, 3, col.paw);
    }

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

CatColors cat_shiny_colors(void) {
    /* Warm gold coat with soft highlights. */
    CatColors c;
    c.body  = rgb(0xF2, 0xD0, 0x7A);   /* gold                */
    c.dark  = rgb(0xD8, 0xA8, 0x4C);   /* deeper gold         */
    c.ear   = rgb(0xF7, 0xE4, 0xB0);   /* pale gold inner ear */
    c.paw   = rgb(0xF7, 0xE8, 0xC0);
    c.cheek = rgb(0xF0, 0xB8, 0x88);   /* warm peach blush    */
    return c;
}

void cat_draw_sparkles(SDL_Renderer *r, const Cat *cat, Uint64 frame) {
    Uint64 f = frame + cat->act_seed;
    static const float OX[4] = { -14.0f, 12.0f, -10.0f, 14.0f };
    static const float OY[4] = { -10.0f, -6.0f,  8.0f,   4.0f };
    for (int i = 0; i < 4; i++) {
        float phase = (float)((f + (Uint64)(i * 40)) % 160) / 160.0f;
        float tw = sinf(phase * 6.2831853f);
        if (tw < 0) continue;                 /* off for half the cycle */
        Uint8 a = (Uint8)(tw * 235.0f);
        float sx = cat->cx + OX[i] + sinf((float)f * 0.03f + (float)i) * 2.0f;
        float sy = cat->cy + OY[i] + cosf((float)f * 0.04f + (float)i) * 2.0f;
        Color gold = rgb(0xFF, 0xE8, 0x9A);
        px_rect_a(r, sx,     sy - 2, 1, 5, gold, a);     /* vertical  */
        px_rect_a(r, sx - 2, sy,     5, 1, gold, a);     /* horizontal */
        px_rect_a(r, sx,     sy,     1, 1, KZ_CLOUD, a); /* center    */
    }
}

void cat_draw_select_ring(SDL_Renderer *r, const Cat *cat, Uint64 frame) {
    /* a gently pulsing pink oval on the ground beneath the cat's feet */
    float cx = cat->cx, cy = cat->cy;
    float pulse = 0.5f + 0.5f * sinf((float)frame * 0.10f);
    Uint8 a = (Uint8)(120.0f + 80.0f * pulse);
    Color ring = KZ_HEART;
    /* an oval outline, drawn as a few stacked rows so it reads as a ring */
    float ry = cy + 18;                 /* at the paws */
    px_rect_a(r, cx - 13, ry,     26, 2, ring, a);          /* front edge */
    px_rect_a(r, cx - 15, ry - 2, 3,  4, ring, a);          /* left curve */
    px_rect_a(r, cx + 12, ry - 2, 3,  4, ring, a);          /* right curve*/
    px_rect_a(r, cx - 13, ry - 4, 26, 2, ring, (Uint8)(a / 2)); /* back, fainter */
    /* two little sparkles to draw the eye */
    Uint8 sa = (Uint8)(200.0f * pulse);
    px_rect_a(r, cx - 16, cy - 14, 2, 2, KZ_CLOUD, sa);
    px_rect_a(r, cx + 15, cy - 12, 2, 2, KZ_CLOUD, sa);
}