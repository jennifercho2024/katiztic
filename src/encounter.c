/* encounter.c — see encounter.h. Who's visiting the meadow, and how she sits. */
#include "encounter.h"
#include "render.h"
#include "cattype.h"
#include <string.h>

/* A pool of wild cats who might show up. Some become regulars once met. */
static const struct { const char *name; CatType type; } WILD[] = {
    { "Sox",    KZ_PLAYFUL },
    { "Ash",    KZ_CLEVER  },
    { "Ginger", KZ_SUNNY   },
    { "Misty",  KZ_DREAMY  },
    { "Bean",   KZ_GENTLE  },
    { "Nimbus", KZ_DREAMY  },
    { "Pip",    KZ_PLAYFUL },
    { "Willow", KZ_GENTLE  },
};
#define WILD_COUNT ((int)(sizeof WILD / sizeof WILD[0]))

/* She settles on the left side of the meadow, a bit apart from your cat. */
#define ENC_FAR_X   60.0f
#define ENC_NEAR_X  86.0f     /* closer once she trusts you */
#define ENC_Y       120.0f

Encounter encounter_none(void) {
    Encounter e;
    e.present = false;
    e.name[0] = '\0';
    e.type = KZ_GENTLE;
    e.anim = cat_make(ENC_FAR_X, ENC_Y);
    e.home_x = ENC_FAR_X;
    return e;
}

Encounter encounter_begin(Friends *f) {
    Encounter e = encounter_none();

    /* Not every walk has a visitor — about 3 in 5 do, so it feels like a
     * happy surprise rather than a chore. */
    if (SDL_rand(5) < 2) return e;   /* nobody today */

    /* Mix regulars and new faces: if you have friends, half the time bring
     * one of them back; otherwise (or the other half) pick from the wild pool. */
    bool bring_regular = (f->count > 0) && (SDL_rand(2) == 0);

    if (bring_regular) {
        int idx = SDL_rand(f->count);
        SDL_strlcpy(e.name, f->list[idx].name, KZ_FRIEND_NAME);
        e.type = f->list[idx].type;
    } else {
        int idx = SDL_rand(WILD_COUNT);
        SDL_strlcpy(e.name, WILD[idx].name, KZ_FRIEND_NAME);
        e.type = WILD[idx].type;
    }

    /* If we already know her, she starts a little closer (she trusts us). */
    Friend *known = friends_find(f, e.name);
    float startx = (known && known->trust > 40) ? ENC_NEAR_X : ENC_FAR_X;
    e.present = true;
    e.home_x = startx;
    e.anim = cat_make(startx, ENC_Y);
    return e;
}

void encounter_update(Encounter *e, const Friends *f) {
    if (!e->present) return;
    cat_update(&e->anim);

    /* Ease toward her home_x, which creeps nearer as trust grows. */
    Friend *known = friends_find((Friends *)f, e->name);
    float target = ENC_FAR_X;
    if (known) {
        float t = (float)known->trust / (float)KZ_TRUST_FULL;   /* 0..1 */
        target = ENC_FAR_X + (ENC_NEAR_X - ENC_FAR_X) * t;
    }
    e->home_x += (target - e->home_x) * 0.04f;   /* gentle glide */
    e->anim.cx = e->home_x;
}

bool encounter_hit(const Encounter *e, float px_, float py_) {
    if (!e->present) return false;
    /* A generous box around the visiting cat so taps land easily — she's a
     * small sprite off to the side, so we're forgiving here. */
    float dx = px_ - e->anim.cx;
    float dy = py_ - e->anim.cy;
    return dx > -16 && dx < 16 && dy > -20 && dy < 24;
}

void encounter_draw(SDL_Renderer *r, const Encounter *e, Uint64 frame) {
    if (!e->present) return;
    cat_draw(r, &e->anim, cattype_colors(e->type), frame);
}