/* behavior.c — see behavior.h. The cats' little brains. */
#include "behavior.h"
#include "roster.h"
#include "decor.h"
#include <math.h>

/* The cottage floor area cats roam within. The cottage room is larger than the
 * screen (360x240) so cats spread through the whole space you can pan around. */
#define ROAM_X0   24.0f
#define ROAM_X1   330.0f
#define ROAM_Y0   165.0f
#define ROAM_Y1   224.0f

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

void behavior_update(Roster *ro, Decor *decor, Uint64 frame) {
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
                float sp = 0.28f;                  /* slow, cozy stroll */
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
                /* Socializing earns both cats a little experience. */
                stats_gain_xp(&ro->cats[i].stats, 6);
                stats_gain_xp(&ro->cats[j].stats, 6);
                break;
            }
        }
    }

    /* Fourth pass: cats notice placed yarn and milk and react to them. A calm
     * cat near yarn bats at it (a little play bounce); near milk she laps it
     * (a grooming pose). Only when décor exists here (the cottage). */
    if (decor) {
        for (int i = 0; i < ro->count; i++) {
            Cat *c = &ro->cats[i].anim;
            if (c->act != ACT_SIT && c->act != ACT_WALK) continue;

            for (int k = 0; k < DECOR_COUNT; k++) {
                if (!decor->items[k].placed) continue;
                DecorKind kind = (DecorKind)k;
                if (kind != DECOR_YARN && kind != DECOR_MILK) continue;

                float ix = decor->items[k].x + 8;   /* item center-ish */
                float iy = decor->items[k].y + 6;
                float dx = ix - c->cx, dy = iy - c->cy;
                float d = sqrtf(dx * dx + dy * dy);
                if (d <= 22.0f && SDL_rand(150) == 0) {
                    c->facing = (dx >= 0) ? 1 : -1;
                    if (kind == DECOR_YARN) {
                        c->act = ACT_PLAY;         /* bat at the yarn */
                    } else {
                        c->act = ACT_GROOM;        /* lap the milk    */
                    }
                    c->act_timer = 180 + SDL_rand(120);
                    stats_gain_xp(&ro->cats[i].stats, 3);  /* a little joy */
                    break;
                }
            }
        }
    }
}