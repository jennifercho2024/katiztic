/* katlympics.c — see katlympics.h. The cat olympics. */
#include "katlympics.h"
#include "render.h"
#include "palette.h"
#include "text.h"
#include "cat.h"
#include <math.h>

static const char *ENAMES[EVENT_COUNT] = { "Trick Showcase", "Obstacle Course" };
const char *event_name(EventId e) {
    if (e < 0 || e >= EVENT_COUNT) return "?";
    return ENAMES[e];
}

const char *medal_name(Medal m) {
    switch (m) {
        case MEDAL_GOLD:   return "Gold";
        case MEDAL_SILVER: return "Silver";
        case MEDAL_BRONZE: return "Bronze";
        default:           return "no medal";
    }
}

/* friendly fallback rivals if you haven't met many owners yet */
static const char *LOCALS[] = {
    "Poppy", "Ziggy", "Maple", "Cricket", "Waffle", "Sesame",
};
#define LOCAL_COUNT ((int)(sizeof LOCALS / sizeof LOCALS[0]))

Katlympics katlympics_none(void) {
    Katlympics k;
    SDL_memset(&k, 0, sizeof k);
    k.active = false;
    return k;
}

/* Your score in the trick showcase: mastered tricks are the big factor, with
 * mood and bond adding polish. 0..100-ish. */
static int score_tricks(const char *cat, const Tricks *tr, const Stats *st) {
    int mastered = tricks_mastered_count(tr, cat);      /* 0..5 */
    /* partial credit for tricks in progress, too */
    int progress = 0;
    for (int t = 0; t < TRICK_COUNT; t++)
        progress += tricks_skill(tr, cat, (TrickId)t);   /* 0..500 */
    int base = mastered * 14 + progress / 20;            /* up to ~95 */
    int polish = (st->mood / 16) + (st->bond / 16);      /* up to ~30 */
    return base + polish;
}

/* Your score in the obstacle course: energy and level drive speed/agility,
 * with jumping/rolling tricks helping on the course. */
static int score_obstacle(const char *cat, const Tricks *tr, const Stats *st) {
    int agility = (st->energy / 4) + (int)st->level;     /* energy + level */
    int jump = tricks_skill(tr, cat, TRICK_JUMP) / 8;    /* jumping helps */
    int roll = tricks_skill(tr, cat, TRICK_ROLL) / 10;
    int mood = st->mood / 20;
    return agility + jump + roll + mood;
}

Katlympics katlympics_begin(EventId event, const char *your_cat,
                            const Tricks *tr, const Stats *st,
                            const char *const *owner_names,
                            const CatType *owner_types, int owner_count) {
    Katlympics k = katlympics_none();
    k.active = true;
    k.event = event;
    k.phase = 0;
    k.timer = 0;
    k.obstacle_step = 0;

    k.your_score = (event == EVENT_TRICKS)
                 ? score_tricks(your_cat, tr, st)
                 : score_obstacle(your_cat, tr, st);

    /* Build the rival field: prefer befriended owners, fill with locals. */
    for (int i = 0; i < KAT_RIVALS; i++) {
        Rival *rv = &k.rivals[i];
        if (i < owner_count && owner_names[i]) {
            SDL_strlcpy(rv->name, owner_names[i], sizeof rv->name);
            rv->cat_type = owner_types ? owner_types[i] : (CatType)SDL_rand(KZ_TYPE_COUNT);
        } else {
            SDL_strlcpy(rv->name, LOCALS[SDL_rand(LOCAL_COUNT)], sizeof rv->name);
            rv->cat_type = (CatType)SDL_rand(KZ_TYPE_COUNT);
        }
        /* Rivals score in a friendly band around a moderate level, so a
         * well-trained cat reliably places well but it's never a walkover. */
        int center = (event == EVENT_TRICKS) ? 60 : 55;
        rv->score = center - 18 + (int)SDL_rand(40);   /* ~ center±20 */
        if (rv->score < 5) rv->score = 5;
    }

    /* Rank: count how many rivals you beat. */
    int place = 1;
    for (int i = 0; i < KAT_RIVALS; i++)
        if (k.rivals[i].score > k.your_score) place++;
    k.place = place;

    switch (place) {
        case 1: k.your_medal = MEDAL_GOLD;   break;
        case 2: k.your_medal = MEDAL_SILVER; break;
        case 3: k.your_medal = MEDAL_BRONZE; break;
        default: k.your_medal = MEDAL_NONE;  break;
    }

    /* Rewards scale with placing — everyone gets a little for taking part. */
    switch (k.your_medal) {
        case MEDAL_GOLD:   k.coins_won = 20; k.xp_won = 40; break;
        case MEDAL_SILVER: k.coins_won = 12; k.xp_won = 25; break;
        case MEDAL_BRONZE: k.coins_won = 8;  k.xp_won = 15; break;
        default:           k.coins_won = 3;  k.xp_won = 8;  break;
    }
    return k;
}

bool katlympics_update(Katlympics *k) {
    if (!k->active) return false;
    k->timer++;
    switch (k->phase) {
        case 0:  /* intro */
            if (k->timer > 90) { k->phase = 1; k->timer = 0; }
            break;
        case 1:  /* performance */
            /* the obstacle course advances the cat through 4 obstacles */
            if (k->event == EVENT_OBSTACLE) {
                if (k->timer % 45 == 0 && k->obstacle_step < 4)
                    k->obstacle_step++;
            }
            if (k->timer > 240) { k->phase = 2; k->timer = 0; }
            break;
        case 2:  /* results — stays until dismissed by the caller */
        default:
            break;
    }
    return false;
}

/* ---- drawing ---- */

static void draw_medal(SDL_Renderer *r, Medal m, float x, float y) {
    Color c = (m == MEDAL_GOLD)   ? rgb(0xF2, 0xD0, 0x7A)
            : (m == MEDAL_SILVER) ? rgb(0xD8, 0xD8, 0xE0)
            : (m == MEDAL_BRONZE) ? rgb(0xD0, 0xA6, 0x7A)
            : rgb(0xC8, 0xBE, 0xC8);
    /* ribbon */
    px_rect(r, x + 2, y, 2, 5, KZ_PETAL_PINK);
    px_rect(r, x + 6, y, 2, 5, KZ_MINT);
    /* disc */
    px_rect(r, x, y + 4, 10, 8, c);
    px_rect(r, x + 2, y + 3, 6, 1, c);
    px_rect(r, x + 2, y + 12, 6, 1, c);
    px_rect(r, x + 3, y + 6, 4, 4, rgb(0xFB, 0xF6, 0xF2));  /* shine */
    px_rect(r, x + 4, y + 7, 2, 2, c);
}

/* a small podium */
static void draw_podium(SDL_Renderer *r, float x, float y) {
    px_rect(r, x + 10, y - 10, 12, 10, rgb(0xF2, 0xD0, 0x7A));   /* 1st */
    px_rect(r, x, y - 6, 10, 6, rgb(0xD8, 0xD8, 0xE0));          /* 2nd */
    px_rect(r, x + 22, y - 4, 10, 4, rgb(0xD0, 0xA6, 0x7A));     /* 3rd */
    px_rect(r, x, y, 32, 3, KZ_COCOA);
}

void katlympics_draw(SDL_Renderer *r, const Katlympics *k,
                     CatType your_type, bool your_shiny, Uint64 frame) {
    if (!k->active) return;

    /* stadium: sky, field, a bunting banner */
    px_rect(r, 0, 0, KZ_W, 60, rgb(0xCF, 0xE6, 0xF2));
    px_rect(r, 0, 60, KZ_W, KZ_H - 60, rgb(0xB6, 0xD6, 0xA0));
    px_rect(r, 0, 60, KZ_W, 3, rgb(0xC6, 0xE0, 0xAC));
    for (int i = 0; i < 12; i++) {
        Color f = (i % 3 == 0) ? KZ_PETAL_PINK : (i % 3 == 1) ? KZ_BUTTER : KZ_MINT;
        px_rect(r, 6 + i * 20, 8 + (int)(sinf((float)i * 0.9f) * 2), 6, 5, f);
    }

    CatColors yc = your_shiny ? cat_shiny_colors() : cattype_colors(your_type);

    if (k->phase == 0) {
        /* intro card */
        text_draw_scaled(r, "Katlympics!", 54, 24, rgb(0x6E, 0x58, 0x92), 2);
        px_rect_a(r, 40, 74, 160, 40, KZ_CLOUD, 230);
        px_rect(r, 40, 74, 160, 1, KZ_COCOA);
        px_rect(r, 40, 113, 160, 1, KZ_COCOA);
        text_draw_centered(r, event_name(k->event), KZ_W / 2.0f, 84, KZ_COCOA);
        text_draw_centered(r, "get ready!", KZ_W / 2.0f, 98,
                           rgb(0x9A, 0x7A, 0x5A));
    } else if (k->phase == 1) {
        /* performance */
        text_draw(r, event_name(k->event), 8, 8, KZ_COCOA);
        if (k->event == EVENT_TRICKS) {
            /* your cat performs a rolling series of tricks center-stage */
            Cat c = cat_make(120, 108);
            int trick_cycle = (int)((frame / 40) % 5);
            c.trick = trick_cycle;
            c.trick_len = 40;
            c.trick_t = 40 - (int)(frame % 40);
            c.facing = 1;
            cat_draw(r, &c, yc, frame);
            if (your_shiny) cat_draw_sparkles(r, &c, frame);
            /* a little applause of hearts */
            for (int i = 0; i < 3; i++) {
                float hx = 90 + i * 30 + sinf((float)frame * 0.1f + i) * 4;
                float hy = 70 - ((frame + i * 20) % 50) * 0.3f;
                px_rect(r, hx, hy, 2, 2, KZ_HEART);
            }
        } else {
            /* obstacle course: the cat hops along past 4 obstacles */
            /* obstacles: hurdle, weave poles, tunnel, slide */
            px_rect(r, 40, 116, 4, 12, rgb(0xC8, 0xA6, 0x8E));   /* hurdle */
            px_rect(r, 38, 114, 8, 2, KZ_PETAL_PINK);
            for (int i = 0; i < 3; i++)                            /* weave */
                px_rect(r, 88 + i * 8, 116, 2, 12, rgb(0x9C, 0xC0, 0xD8));
            px_rect(r, 140, 116, 26, 12, rgb(0x6B, 0x5B, 0x8B));  /* tunnel */
            px_rect(r, 142, 118, 22, 8, rgb(0x4B, 0x40, 0x60));
            for (int i = 0; i < 12; i++)                           /* slide */
                px_rect(r, 190 + i, 110 + i, 4, 2, rgb(0x9C, 0xC0, 0xD8));

            /* the cat advances across as obstacle_step rises */
            float progress = (float)k->obstacle_step / 4.0f
                           + (float)(k->timer % 45) / 45.0f / 4.0f;
            if (progress > 1.0f) progress = 1.0f;
            float cx = 40 + progress * 160;
            float hop = fabsf(sinf((float)frame * 0.3f)) * 8.0f;
            Cat c = cat_make(cx, 120 - hop);
            c.act = ACT_WALK;
            c.facing = 1;
            cat_draw(r, &c, yc, frame);
            if (your_shiny) cat_draw_sparkles(r, &c, frame);
        }
        /* a progress cheer bar */
        text_draw_centered(r, "your cat is doing great!", KZ_W / 2.0f,
                           KZ_H - 12, rgb(0x6E, 0x58, 0x92));
    } else {
        /* results */
        text_draw_scaled(r, "Results", 78, 12, rgb(0x6E, 0x58, 0x92), 2);
        draw_podium(r, 104, 60);

        /* the standings list: you + rivals, sorted by score */
        struct Row { const char *name; int score; bool you; };
        struct Row rows[KAT_RIVALS + 1];
        rows[0].name = "You"; rows[0].score = k->your_score; rows[0].you = true;
        for (int i = 0; i < KAT_RIVALS; i++) {
            rows[i + 1].name = k->rivals[i].name;
            rows[i + 1].score = k->rivals[i].score;
            rows[i + 1].you = false;
        }
        /* sort desc by score (tiny bubble sort) */
        for (int a = 0; a < KAT_RIVALS + 1; a++)
            for (int b = a + 1; b < KAT_RIVALS + 1; b++)
                if (rows[b].score > rows[a].score) {
                    struct Row tmp = rows[a]; rows[a] = rows[b]; rows[b] = tmp;
                }

        float ly = 74;
        for (int i = 0; i < KAT_RIVALS + 1; i++) {
            Color rowc = rows[i].you ? KZ_PETAL_PINK : KZ_CLOUD;
            px_rect_a(r, 54, ly, 132, 11, rowc, 230);
            char line[40];
            SDL_snprintf(line, sizeof line, "%d.  %s", i + 1, rows[i].name);
            text_draw(r, line, 60, ly + 2, KZ_COCOA);
            char sc[16];
            SDL_snprintf(sc, sizeof sc, "%d", rows[i].score);
            text_draw(r, sc, 168, ly + 2, rgb(0x9A, 0x7A, 0x5A));
            ly += 13;
        }

        /* your medal + rewards */
        if (k->your_medal != MEDAL_NONE) {
            draw_medal(r, k->your_medal, 60, ly + 2);
            char m[48];
            SDL_snprintf(m, sizeof m, "%s medal!  +%d coins, +%d xp",
                         medal_name(k->your_medal), k->coins_won, k->xp_won);
            text_draw(r, m, 74, ly + 6, rgb(0x6E, 0x58, 0x92));
        } else {
            char m[48];
            SDL_snprintf(m, sizeof m, "Good effort!  +%d coins, +%d xp",
                         k->coins_won, k->xp_won);
            text_draw(r, m, 60, ly + 6, rgb(0x6E, 0x58, 0x92));
        }
        text_draw_centered(r, "tap to continue", KZ_W / 2.0f, KZ_H - 10,
                           KZ_COCOA);
    }
}