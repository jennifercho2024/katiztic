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
/* A proper award podium: three tiers (2nd left, 1st tall center, 3rd right),
 * each labelled, with a little cat standing on top of its block. `types` gives
 * the cat type for each of the three places (index 0=1st,1=2nd,2=3rd). */
static void draw_podium(SDL_Renderer *r, float cx, float base_y,
                        const CatType *types, Uint64 frame) {
    /* block sizes */
    float bw = 30;
    float h1 = 34, h2 = 26, h3 = 20;   /* 1st tallest */
    float x1 = cx - bw / 2.0f;          /* center (1st) */
    float x2 = cx - bw - bw / 2.0f - 4; /* left (2nd)   */
    float x3 = cx + bw / 2.0f + 4;      /* right (3rd)  */

    /* a soft spotlight glow behind the winner */
    px_rect_a(r, x1 - 4, base_y - h1 - 40, bw + 8, h1 + 40, KZ_BUTTER, 40);

    /* the three blocks, gold/silver/bronze */
    Color gold = rgb(0xF2, 0xD0, 0x7A), silver = rgb(0xD8, 0xD8, 0xE0),
          bronze = rgb(0xD8, 0xAE, 0x82);
    /* 2nd */
    px_rect(r, x2, base_y - h2, bw, h2, silver);
    px_rect(r, x2, base_y - h2, bw, 1, KZ_CLOUD);
    px_rect(r, x2, base_y - h2, 1, h2, KZ_COCOA);
    px_rect(r, x2 + bw - 1, base_y - h2, 1, h2, KZ_COCOA);
    text_draw_centered(r, "2", x2 + bw / 2.0f, base_y - h2 / 2.0f - 3, KZ_COCOA);
    /* 3rd */
    px_rect(r, x3, base_y - h3, bw, h3, bronze);
    px_rect(r, x3, base_y - h3, bw, 1, rgb(0xE8, 0xC8, 0xA8));
    px_rect(r, x3, base_y - h3, 1, h3, KZ_COCOA);
    px_rect(r, x3 + bw - 1, base_y - h3, 1, h3, KZ_COCOA);
    text_draw_centered(r, "3", x3 + bw / 2.0f, base_y - h3 / 2.0f - 3, KZ_COCOA);
    /* 1st (center, tallest) */
    px_rect(r, x1, base_y - h1, bw, h1, gold);
    px_rect(r, x1, base_y - h1, bw, 1, rgb(0xFB, 0xEE, 0xC0));
    px_rect(r, x1, base_y - h1, 1, h1, KZ_COCOA);
    px_rect(r, x1 + bw - 1, base_y - h1, 1, h1, KZ_COCOA);
    text_draw_centered(r, "1", x1 + bw / 2.0f, base_y - h1 / 2.0f - 3, KZ_COCOA);

    /* a little cat standing on each block (a gentle winner's bounce on 1st) */
    float bounce = fabsf(sinf((float)frame * 0.12f)) * 2.0f;
    Cat c1 = cat_make(x1 + bw / 2.0f, base_y - h1 - 8 - bounce);
    c1.facing = 1;
    cat_draw(r, &c1, cattype_colors(types[0]), frame);
    /* a sparkle crown on the winner */
    px_rect(r, x1 + bw / 2.0f - 1, base_y - h1 - 20 - bounce, 2, 3, KZ_BUTTER);
    px_rect(r, x1 + bw / 2.0f - 3, base_y - h1 - 18 - bounce, 6, 2, KZ_BUTTER);

    Cat c2 = cat_make(x2 + bw / 2.0f, base_y - h2 - 8);
    c2.facing = 1;
    cat_draw(r, &c2, cattype_colors(types[1]), frame);
    Cat c3 = cat_make(x3 + bw / 2.0f, base_y - h3 - 8);
    c3.facing = 1;
    cat_draw(r, &c3, cattype_colors(types[2]), frame);

    /* podium floor line */
    px_rect(r, x2 - 2, base_y, bw * 3 + 12, 2, KZ_COCOA);
}

/* A proper stadium: tiered stands packed with spectators, flags along the
 * rim, floodlights, and a clean arena floor with lane markings. Drawn behind
 * every phase so the Katlympics feels like a real venue. */
static void stadium_backdrop(SDL_Renderer *r, Uint64 frame) {
    /* bright daytime sky over the stadium */
    px_rect(r, 0, 0, KZ_W, 46, rgb(0xCF, 0xE6, 0xF2));
    px_rect(r, 0, 0, KZ_W, 14, rgb(0xBE, 0xDA, 0xEE));

    /* floodlight towers at the corners */
    for (int s = 0; s < 2; s++) {
        float lx = s == 0 ? 18 : KZ_W - 22;
        px_rect(r, lx + 1, 6, 2, 22, rgb(0x8A, 0x8A, 0x9A));   /* pole */
        px_rect(r, lx - 3, 2, 10, 5, rgb(0x6A, 0x6A, 0x7A));   /* housing */
        for (int b = 0; b < 6; b++)                             /* bulbs */
            px_rect(r, lx - 2 + (b % 3) * 3, 3 + (b / 3) * 2, 2, 1,
                    rgb(0xFC, 0xF4, 0xC8));
    }

    /* tiered stands: three banded rows of seats full of spectators */
    Color tier[3] = { rgb(0x8E, 0x7C, 0xB0), rgb(0x9E, 0x8C, 0xC0),
                      rgb(0xAE, 0x9C, 0xCE) };
    for (int row = 0; row < 3; row++) {
        float y = 20 + row * 10;
        px_rect(r, 0, y, KZ_W, 10, tier[row]);
        px_rect(r, 0, y, KZ_W, 1, rgb(0x6E, 0x5C, 0x90));
        /* spectators: little dotted heads that gently shift (a lively crowd) */
        for (int c = 0; c < KZ_W / 6; c++) {
            int seed = (row * 97 + c * 13);
            Color head = ((seed % 4) == 0) ? KZ_PETAL_PINK
                       : ((seed % 4) == 1) ? KZ_BUTTER
                       : ((seed % 4) == 2) ? KZ_MINT : KZ_CLOUD;
            float wob = ((int)((frame / 20 + seed) % 2)) ? 0.0f : 1.0f;
            px_rect(r, 2 + c * 6, y + 3 + wob, 3, 3, head);
        }
    }

    /* pennant flags strung along the top of the stands */
    for (int i = 0; i < 16; i++) {
        Color f = (i % 3 == 0) ? KZ_PETAL_PINK : (i % 3 == 1) ? KZ_BUTTER : KZ_MINT;
        px_rect(r, i * 16, 16, 1, 4, KZ_COCOA);       /* string tick */
        px_rect(r, i * 16 - 2, 16, 5, 2, f);          /* flag */
    }

    /* arena wall + floor */
    px_rect(r, 0, 50, KZ_W, 4, rgb(0xE6, 0xDE, 0xE8));      /* barrier wall */
    px_rect(r, 0, 50, KZ_W, 1, rgb(0x6E, 0x5C, 0x90));
    px_rect(r, 0, 54, KZ_W, KZ_H - 54, rgb(0xC2, 0xB6, 0x8E));  /* sandy track */
    px_rect(r, 0, 54, KZ_W, 2, rgb(0xB2, 0xA6, 0x7E));
    /* lane lines on the track for an athletic look */
    for (int lane = 1; lane < 5; lane++)
        px_rect(r, 0, 54 + lane * ((KZ_H - 54) / 5), KZ_W, 1,
                rgb(0xD6, 0xCC, 0xB0));
    /* a green infield strip */
    px_rect(r, 0, KZ_H - 22, KZ_W, 22, rgb(0xA8, 0xC8, 0x92));
    px_rect(r, 0, KZ_H - 22, KZ_W, 1, rgb(0x8E, 0xB0, 0x7C));
}

/* the Katlympics rings emblem, centered at (ex, ey) */
static void draw_rings(SDL_Renderer *r, float ex, float ey) {
    Color rings[5] = { KZ_PETAL_PINK, KZ_MINT, KZ_BUTTER, KZ_LAVENDER, KZ_HEART };
    for (int i = 0; i < 5; i++) {
        float cx = ex + (i % 3) * 18 + ((i >= 3) ? 9 : 0);
        float cy = ey + ((i >= 3) ? 6 : 0);
        px_rect(r, cx, cy, 12, 2, rings[i]);
        px_rect(r, cx, cy + 8, 12, 2, rings[i]);
        px_rect(r, cx, cy, 2, 10, rings[i]);
        px_rect(r, cx + 10, cy, 2, 10, rings[i]);
    }
}

void katlympics_draw_arena(SDL_Renderer *r, Uint64 frame) {
    stadium_backdrop(r, frame);
    /* a ceremonial banner header with the title */
    px_rect(r, 20, 4, KZ_W - 40, 16, rgb(0x6E, 0x58, 0x92));
    px_rect(r, 20, 4, KZ_W - 40, 1, KZ_BUTTER);
    px_rect(r, 20, 19, KZ_W - 40, 1, KZ_BUTTER);
    text_draw_centered(r, "KATLYMPICS", KZ_W / 2.0f, 8, KZ_CLOUD);
    /* the five-ring emblem beneath the banner */
    draw_rings(r, KZ_W / 2.0f - 22, 26);
}

void katlympics_draw(SDL_Renderer *r, const Katlympics *k,
                     CatType your_type, bool your_shiny, Uint64 frame) {
    if (!k->active) return;

    stadium_backdrop(r, frame);

    CatColors yc = your_shiny ? cat_shiny_colors() : cattype_colors(your_type);

    if (k->phase == 0) {
        /* ---- an official opening-ceremony intro ---- */

        /* a grand ceremonial banner draped across the top */
        px_rect(r, 20, 30, KZ_W - 40, 30, rgb(0x6E, 0x58, 0x92));
        px_rect(r, 20, 30, KZ_W - 40, 2, KZ_BUTTER);
        px_rect(r, 20, 58, KZ_W - 40, 2, KZ_BUTTER);
        /* banner tassels hanging below */
        for (int i = 0; i < 9; i++) {
            float bx = 30 + i * 22;
            px_rect(r, bx, 60, 4, 4, KZ_BUTTER);
            px_rect(r, bx + 1, 64, 2, 2, rgb(0xE8, 0xC8, 0x8A));
        }
        /* the title, large and centered on the banner */
        text_draw_scaled(r, "KATLYMPICS", 44, 38, KZ_CLOUD, 2);

        /* the Katlympics emblem: five interlocking rings (its own official mark) */
        Color rings[5] = { KZ_PETAL_PINK, KZ_MINT, KZ_BUTTER, KZ_LAVENDER,
                           KZ_HEART };
        float rx0 = 78, ry0 = 82;
        for (int i = 0; i < 5; i++) {
            float cx = rx0 + (i % 3) * 18 + ((i >= 3) ? 9 : 0);
            float cy = ry0 + ((i >= 3) ? 6 : 0);
            /* draw a little ring (hollow square) */
            px_rect(r, cx, cy, 12, 2, rings[i]);
            px_rect(r, cx, cy + 8, 12, 2, rings[i]);
            px_rect(r, cx, cy, 2, 10, rings[i]);
            px_rect(r, cx + 10, cy, 2, 10, rings[i]);
        }

        /* a ceremonial torch on each side, flames flickering */
        for (int s = 0; s < 2; s++) {
            float tx = s == 0 ? 30 : KZ_W - 38;
            px_rect(r, tx + 2, 96, 4, 14, rgb(0xC8, 0xA6, 0x8E));   /* handle */
            px_rect(r, tx, 92, 8, 5, rgb(0xC0, 0x9A, 0x6E));       /* bowl */
            /* flame */
            float flick = ((frame / 6 + s) % 2) ? 0.0f : 1.0f;
            px_rect(r, tx + 2, 86 - flick, 4, 6, rgb(0xF2, 0xA0, 0x50));
            px_rect(r, tx + 3, 83 - flick, 2, 4, KZ_BUTTER);
        }

        /* the event name plate */
        px_rect_a(r, 50, 116, 140, 26, KZ_CLOUD, 235);
        px_rect(r, 50, 116, 140, 1, KZ_COCOA);
        px_rect(r, 50, 141, 140, 1, KZ_COCOA);
        px_rect(r, 50, 116, 1, 26, KZ_COCOA);
        px_rect(r, 189, 116, 1, 26, KZ_COCOA);
        text_draw_centered(r, event_name(k->event), KZ_W / 2.0f, 121, KZ_COCOA);
        /* a blinking "get ready" so it feels like a countdown */
        if ((frame / 24) % 2 == 0)
            text_draw_centered(r, "get ready!", KZ_W / 2.0f, 132,
                               rgb(0x6E, 0x58, 0x92));
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
        /* results — an award ceremony on the winners' stage */
        /* a broad spotlight cone shining down onto the podium */
        for (int i = 0; i < 20; i++) {
            float halfw = 8 + i * 3.0f;
            px_rect_a(r, KZ_W / 2.0f - halfw, 40 + i * 2.6f, halfw * 2, 3,
                      KZ_BUTTER, 18);
        }
        /* a ceremony banner across the top */
        px_rect(r, 40, 4, KZ_W - 80, 16, rgb(0x6E, 0x58, 0x92));
        px_rect(r, 40, 4, KZ_W - 80, 1, KZ_BUTTER);
        px_rect(r, 40, 19, KZ_W - 80, 1, KZ_BUTTER);
        text_draw_centered(r, "Awards Ceremony", KZ_W / 2.0f, 8, KZ_CLOUD);

        /* build the ranked list: you + rivals, sorted by score */
        struct Row { const char *name; int score; bool you; CatType type; };
        struct Row rows[KAT_RIVALS + 1];
        rows[0].name = "You"; rows[0].score = k->your_score; rows[0].you = true;
        rows[0].type = your_type;
        for (int i = 0; i < KAT_RIVALS; i++) {
            rows[i + 1].name = k->rivals[i].name;
            rows[i + 1].score = k->rivals[i].score;
            rows[i + 1].you = false;
            rows[i + 1].type = k->rivals[i].cat_type;
        }
        for (int a = 0; a < KAT_RIVALS + 1; a++)
            for (int b = a + 1; b < KAT_RIVALS + 1; b++)
                if (rows[b].score > rows[a].score) {
                    struct Row tmp = rows[a]; rows[a] = rows[b]; rows[b] = tmp;
                }

        /* the podium, with the top three cats standing on their places */
        CatType top3[3] = { rows[0].type, rows[1].type, rows[2].type };
        draw_podium(r, KZ_W / 2.0f, 96, top3, frame);

        /* a compact standings strip across the bottom */
        float ly = 116;
        for (int i = 0; i < KAT_RIVALS + 1; i++) {
            Color rowc = rows[i].you ? KZ_PETAL_PINK : KZ_CLOUD;
            px_rect_a(r, 20, ly, 200, 9, rowc, 220);
            /* a little medal for the top three */
            if (i < 3) {
                Medal m = (i == 0) ? MEDAL_GOLD : (i == 1) ? MEDAL_SILVER
                                                           : MEDAL_BRONZE;
                draw_medal(r, m, 21, ly - 2);
            }
            char line[48];
            SDL_snprintf(line, sizeof line, "%d.  %s", i + 1, rows[i].name);
            text_draw(r, line, 34, ly + 1, KZ_COCOA);
            char sc[16];
            SDL_snprintf(sc, sizeof sc, "%d pts", rows[i].score);
            text_draw(r, sc, 180, ly + 1, rgb(0x9A, 0x7A, 0x5A));
            ly += 10;
        }

        /* your reward line */
        char m[52];
        if (k->your_medal != MEDAL_NONE)
            SDL_snprintf(m, sizeof m, "%s medal!  +%d coins, +%d xp",
                         medal_name(k->your_medal), k->coins_won, k->xp_won);
        else
            SDL_snprintf(m, sizeof m, "Nice try!  +%d coins, +%d xp",
                         k->coins_won, k->xp_won);
        text_draw_centered(r, m, KZ_W / 2.0f, KZ_H - 8, rgb(0x6E, 0x58, 0x92));
    }
}