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

/* Your score in the trick showcase, based on the SPECIFIC tricks you chose:
 * each chosen trick contributes its skill, so pick your cat's best ones. */
static int score_chosen_tricks(const char *cat, const Tricks *tr,
                               const Stats *st, const TrickId *chosen,
                               int count) {
    if (count <= 0) return 10;   /* showed up, at least */
    int total = 0;
    for (int i = 0; i < count; i++) {
        int sk = tricks_skill(tr, cat, chosen[i]);   /* 0..100 */
        /* mastered tricks earn a polish bonus */
        total += sk + (sk >= TRICK_MASTER ? 15 : 0);
    }
    /* average across the chosen tricks, then add mood/bond polish */
    int avg = total / count;                          /* ~0..115 */
    int polish = (st->mood / 20) + (st->bond / 20);   /* up to ~24 */
    return avg * 3 / 4 + polish;                       /* scaled to ~0..110 */
}

const char *action_name(CourseAction a) {
    switch (a) {
        case ACTION_JUMP:   return "Jump";
        case ACTION_CRAWL:  return "Crawl";
        case ACTION_ZIGZAG: return "Zigzag";
        case ACTION_DASH:   return "Dash";
        default:            return "?";
    }
}

/* Each obstacle is best cleared with a particular action. */
CourseAction katlympics_obstacle_wants(int obstacle) {
    static const CourseAction WANTS[KAT_OBSTACLES] = {
        ACTION_JUMP,    /* the hurdle wants a jump   */
        ACTION_ZIGZAG,  /* the poles want a zigzag   */
        ACTION_CRAWL,   /* the tunnel wants a crawl  */
        ACTION_DASH,    /* the straightaway wants a dash */
    };
    if (obstacle < 0 || obstacle >= KAT_OBSTACLES) return ACTION_JUMP;
    return WANTS[obstacle];
}

/* Obstacle score: each obstacle you clear with the RIGHT action scores full;
 * a wrong action still clears it but for fewer points. Agility (energy/level)
 * and relevant trick skill add a base. */
static int score_chosen_obstacle(const char *cat, const Tricks *tr,
                                 const Stats *st,
                                 const CourseAction *actions) {
    int agility = (st->energy / 6) + (int)st->level / 2;   /* base fitness */
    int course = 0;
    for (int i = 0; i < KAT_OBSTACLES; i++) {
        CourseAction want = katlympics_obstacle_wants(i);
        bool right = actions[i] == want;
        course += right ? 22 : 8;   /* right choice scores much better */
        /* a matching mastered trick gives an extra flourish */
        if (right && want == ACTION_JUMP
            && tricks_skill(tr, cat, TRICK_JUMP) >= TRICK_MASTER) course += 6;
        if (right && want == ACTION_ZIGZAG
            && tricks_skill(tr, cat, TRICK_SPIN) >= TRICK_MASTER) course += 6;
    }
    return agility + course;   /* ~0..110 */
}

/* shared: fill rivals, rank, set medal + rewards */
static void finish_setup(Katlympics *k, int center,
                         const char *const *owner_names,
                         const CatType *owner_types, int owner_count) {
    for (int i = 0; i < KAT_RIVALS; i++) {
        Rival *rv = &k->rivals[i];
        if (i < owner_count && owner_names[i]) {
            SDL_strlcpy(rv->name, owner_names[i], sizeof rv->name);
            rv->cat_type = owner_types ? owner_types[i]
                                       : (CatType)SDL_rand(KZ_TYPE_COUNT);
        } else {
            SDL_strlcpy(rv->name, LOCALS[SDL_rand(LOCAL_COUNT)], sizeof rv->name);
            rv->cat_type = (CatType)SDL_rand(KZ_TYPE_COUNT);
        }
        rv->score = center - 18 + (int)SDL_rand(40);
        if (rv->score < 5) rv->score = 5;
    }
    int place = 1;
    for (int i = 0; i < KAT_RIVALS; i++)
        if (k->rivals[i].score > k->your_score) place++;
    k->place = place;
    switch (place) {
        case 1: k->your_medal = MEDAL_GOLD;   break;
        case 2: k->your_medal = MEDAL_SILVER; break;
        case 3: k->your_medal = MEDAL_BRONZE; break;
        default: k->your_medal = MEDAL_NONE;  break;
    }
    switch (k->your_medal) {
        case MEDAL_GOLD:   k->coins_won = 20; k->xp_won = 40; break;
        case MEDAL_SILVER: k->coins_won = 12; k->xp_won = 25; break;
        case MEDAL_BRONZE: k->coins_won = 8;  k->xp_won = 15; break;
        default:           k->coins_won = 3;  k->xp_won = 8;  break;
    }
}

Katlympics katlympics_begin_tricks(const char *your_cat, const Tricks *tr,
                                   const Stats *st,
                                   const TrickId *chosen, int chosen_count,
                                   const char *const *owner_names,
                                   const CatType *owner_types, int owner_count) {
    Katlympics k = katlympics_none();
    k.active = true;
    k.event = EVENT_TRICKS;
    k.phase = 0;
    k.chosen_count = chosen_count;
    for (int i = 0; i < chosen_count && i < 3; i++)
        k.chosen_tricks[i] = chosen[i];
    k.your_score = score_chosen_tricks(your_cat, tr, st, chosen, chosen_count);
    finish_setup(&k, 60, owner_names, owner_types, owner_count);
    return k;
}

Katlympics katlympics_begin_obstacle(const char *your_cat, const Tricks *tr,
                                     const Stats *st,
                                     const CourseAction *actions,
                                     const char *const *owner_names,
                                     const CatType *owner_types, int owner_count) {
    Katlympics k = katlympics_none();
    k.active = true;
    k.event = EVENT_OBSTACLE;
    k.phase = 0;
    for (int i = 0; i < KAT_OBSTACLES; i++)
        k.chosen_actions[i] = actions[i];
    k.your_score = score_chosen_obstacle(your_cat, tr, st, actions);
    finish_setup(&k, 55, owner_names, owner_types, owner_count);
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
            /* your cat performs YOUR CHOSEN tricks in sequence, center-stage */
            Cat c = cat_make(120, 108);
            c.facing = 1;
            if (k->chosen_count > 0) {
                /* each chosen trick gets a slice of the performance */
                int per = 240 / k->chosen_count;   /* phase-1 lasts ~240f */
                int which = (k->timer / per);
                if (which >= k->chosen_count) which = k->chosen_count - 1;
                c.trick = (int)k->chosen_tricks[which];
                c.trick_len = 40;
                c.trick_t = 40 - (int)(k->timer % 40);
                /* show which trick is being performed */
                text_draw_centered(r, trick_name(k->chosen_tricks[which]),
                                   KZ_W / 2.0f, 40, rgb(0x6E, 0x58, 0x92));
            }
            cat_draw(r, &c, yc, frame);
            if (your_shiny) cat_draw_sparkles(r, &c, frame);
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
            /* the current obstacle and the action you chose for it */
            int step = k->obstacle_step;
            if (step > KAT_OBSTACLES - 1) step = KAT_OBSTACLES - 1;
            CourseAction act = k->chosen_actions[step];
            bool right = act == katlympics_obstacle_wants(step);

            /* animate the chosen action */
            float hop = 0.0f;
            Cat c = cat_make(cx, 120);
            c.facing = 1;
            switch (act) {
                case ACTION_JUMP:
                    hop = fabsf(sinf((float)frame * 0.3f)) * 14.0f;
                    c.trick = CAT_TRICK_JUMP; c.trick_len = 40;
                    c.trick_t = 40 - (int)(k->timer % 40);
                    break;
                case ACTION_CRAWL:
                    hop = -4.0f;   /* low to the ground */
                    c.act = ACT_WALK;
                    break;
                case ACTION_ZIGZAG:
                    hop = sinf((float)frame * 0.4f) * 6.0f;
                    c.trick = CAT_TRICK_SPIN; c.trick_len = 40;
                    c.trick_t = 40 - (int)(k->timer % 40);
                    break;
                case ACTION_DASH:
                default:
                    hop = fabsf(sinf((float)frame * 0.6f)) * 5.0f;  /* fast bob */
                    c.act = ACT_WALK;
                    break;
            }
            c.cy = 120 - hop;
            cat_draw(r, &c, yc, frame);
            if (your_shiny) cat_draw_sparkles(r, &c, frame);

            /* label the action, green if it's the ideal one for this obstacle */
            char lbl[24];
            SDL_snprintf(lbl, sizeof lbl, "%s!", action_name(act));
            text_draw_centered(r, lbl, cx, 92,
                               right ? rgb(0x6A, 0xA0, 0x7A) : rgb(0x9A, 0x7A, 0x5A));
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