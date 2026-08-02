/* behavior.c — see behavior.h. The cats' little brains. */
#include "behavior.h"
#include "roster.h"
#include <math.h>

/* The cottage floor area cats roam within (logical 240x160 space). Kept clear
 * of the very top (walls/window) and the very bottom (UI strips). */
#define ROAM_X0   24.0f
#define ROAM_X1   210.0f
#define ROAM_Y0   112.0f
#define ROAM_Y1   146.0f

/* How close two cats must be to start playing together. */
#define PLAY_DIST 26.0f

static float frand_range(float lo, float hi) {
    return lo + SDL_randf() * (hi - lo);
}

/* Pick a fresh activity and how long to hold it. Weighted toward calm. */
static void choose_activity(Cat *c) {
    int roll = SDL_rand(100);
    if (roll < 34) {
        c->act = ACT_SIT;
        c->act_timer = 180 + SDL_rand(240);       /* 3–7s sitting     */
    } else if (roll < 64) {
        c->act = ACT_WALK;
        c->tx = frand_range(ROAM_X0, ROAM_X1);
        c->ty = frand_range(ROAM_Y0, ROAM_Y1);
        c->facing = (c->tx >= c->cx) ? 1 : -1;
        c->act_timer = 300;                        /* up to 5s to arrive */
    } else if (roll < 80) {
        c->act = ACT_GROOM;
        c->act_timer = 150 + SDL_rand(150);        /* 2.5–5s grooming  */
    } else {
        c->act = ACT_SLEEP;
        c->act_timer = 360 + SDL_rand(360);        /* 6–12s napping    */
    }
}

/* Distance between two cats. */
static float dist(const Cat *a, const Cat *b) {
    float dx = a->cx - b->cx, dy = a->cy - b->cy;
    return sqrtf(dx * dx + dy * dy);
}

void behavior_update(struct Roster *ro_opaque, Uint64 frame) {
    Roster *ro = (Roster *)ro_opaque;
    (void)frame;

    /* First pass: advance each cat's own activity. */
    for (int i = 0; i < ro->count; i++) {
        Cat *c = &ro->cats[i].anim;

        /* Cats in a play bout are handled in the pairing pass; skip here so
         * their timer isn't double-ticked. */
        if (c->act == ACT_PLAY) continue;

        if (c->act_timer > 0) c->act_timer--;

        if (c->act == ACT_WALK) {
            /* glide toward the target */
            float dx = c->tx - c->cx, dy = c->ty - c->cy;
            float d = sqrtf(dx * dx + dy * dy);
            if (d < 1.5f || c->act_timer <= 0) {
                choose_activity(c);                /* arrived (or gave up) */
            } else {
                float sp = 0.45f;                  /* gentle stroll speed */
                c->cx += dx / d * sp;
                c->cy += dy / d * sp;
                c->facing = (dx >= 0) ? 1 : -1;
            }
        } else if (c->act_timer <= 0) {
            choose_activity(c);
        }
    }

    /* Second pass: pair up nearby idle/sitting cats to play together. */
    for (int i = 0; i < ro->count; i++) {
        Cat *a = &ro->cats[i].anim;
        if (a->act == ACT_PLAY) {
            /* tick the play bout down; when done, both return to sitting */
            if (a->act_timer > 0) a->act_timer--;
            if (a->act_timer <= 0) { a->act = ACT_SIT; a->act_timer = 120; }
            continue;
        }
        /* only calm cats look for a playmate */
        if (a->act != ACT_SIT && a->act != ACT_WALK) continue;

        for (int j = i + 1; j < ro->count; j++) {
            Cat *b = &ro->cats[j].anim;
            if (b->act == ACT_PLAY) continue;
            if (b->act != ACT_SIT && b->act != ACT_WALK) continue;
            if (dist(a, b) <= PLAY_DIST && SDL_rand(180) == 0) {
                /* start a play bout: they face each other and bounce */
                a->act = ACT_PLAY; a->act_timer = 200 + SDL_rand(120);
                b->act = ACT_PLAY; b->act_timer = a->act_timer;
                a->facing = (b->cx >= a->cx) ? 1 : -1;
                b->facing = (a->cx >= b->cx) ? 1 : -1;
                break;
            }
        }
    }
}