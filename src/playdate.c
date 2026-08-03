/* playdate.c — see playdate.h. Two cats playing in a sunny spot. */
#include "playdate.h"
#include "render.h"
#include "palette.h"
#include "text.h"
#include <math.h>

/* the two cats' home spots on the picnic blanket */
#define PD_YOUR_X  92.0f
#define PD_GUEST_X 148.0f
#define PD_Y       104.0f

Playdate playdate_none(void) {
    Playdate pd;
    pd.active = false;
    pd.owner[0] = '\0';
    pd.joy = 0.0f;
    pd.started = 0;
    pd.guest_type = KZ_SUNNY;
    return pd;
}

Playdate playdate_begin(const char *owner, CatType guest_type, Uint64 frame) {
    Playdate pd = playdate_none();
    pd.active = true;
    SDL_strlcpy(pd.owner, owner, sizeof pd.owner);
    pd.guest_type = guest_type;
    pd.guest = cat_make(PD_GUEST_X, PD_Y);
    pd.guest.facing = -1;      /* facing your cat */
    pd.guest.act = ACT_PLAY;
    pd.started = frame;
    pd.joy = 0.0f;
    return pd;
}

bool playdate_update(Playdate *pd, Cat *your_cat, Uint64 frame) {
    if (!pd->active) return false;
    cat_update(&pd->guest);

    float t = (float)(frame - pd->started);
    float bounce = sinf(t * 0.06f) * 10.0f;
    pd->guest.cx = PD_GUEST_X - (bounce > 0 ? bounce : 0);
    pd->guest.cy = PD_Y - fabsf(sinf(t * 0.12f)) * 4.0f;
    pd->guest.act = ACT_PLAY;

    if (your_cat) {
        your_cat->cx = PD_YOUR_X + (bounce < 0 ? -bounce : 0);
        your_cat->cy = PD_Y - fabsf(sinf(t * 0.12f + 1.5f)) * 4.0f;
        your_cat->facing = 1;
        your_cat->act = ACT_PLAY;
    }

    pd->joy += 1.0f / 1800.0f;   /* ~30s of play fills it */
    if (pd->joy > 1.0f) pd->joy = 1.0f;
    return pd->joy >= 1.0f;
}

void playdate_draw(SDL_Renderer *r, const Playdate *pd, const Cat *your_cat,
                   CatColors your_colors, Uint64 frame) {
    if (!pd->active) return;

    px_rect(r, 0, 0, KZ_W, 60, rgb(0xD9, 0xEC, 0xF2));      /* sky   */
    px_rect(r, 0, 60, KZ_W, KZ_H - 60, rgb(0xC7, 0xE2, 0xB8)); /* grass */

    /* a shady tree on the left */
    px_rect(r, 26, 40, 6, 40, rgb(0xA8, 0x86, 0x6E));
    px_rect(r, 10, 18, 40, 26, rgb(0x9C, 0xC6, 0x8E));
    px_rect(r, 16, 10, 28, 16, rgb(0xB6, 0xDA, 0xA0));

    /* checked picnic blanket */
    for (int gy = 0; gy < 4; gy++)
        for (int gx = 0; gx < 8; gx++) {
            bool a = (gx + gy) % 2 == 0;
            px_rect(r, 70 + gx * 12, 96 + gy * 8, 12, 8,
                    a ? rgb(0xF0, 0xC6, 0xC6) : rgb(0xF6, 0xEC, 0xEC));
        }

    /* basket */
    px_rect(r, 60, 96, 10, 8, rgb(0xC8, 0xA6, 0x7A));
    px_rect(r, 60, 96, 10, 2, rgb(0xB0, 0x8E, 0x62));

    if (your_cat)
        cat_draw(r, your_cat, your_colors, frame);
    cat_draw(r, &pd->guest, cattype_colors(pd->guest_type), frame);

    if (pd->joy > 0.3f) {
        float hx = 120.0f + sinf((float)frame * 0.05f) * 6.0f;
        float hy = 84.0f - ((frame % 60) / 60.0f) * 10.0f;
        px_rect(r, hx, hy, 2, 2, KZ_HEART);
        px_rect(r, hx + 4, hy + 3, 2, 2, KZ_PETAL_PINK);
    }

    char line[40];
    SDL_snprintf(line, sizeof line, "Playdate with %s", pd->owner);
    px_rect_a(r, 4, 4, text_width(line) + 6, 10, KZ_CLOUD, 210);
    text_draw(r, line, 7, 6, KZ_COCOA);

    float bx = 6, by = KZ_H - 12, bw = 120, bh = 7;
    px_rect(r, bx, by, bw, bh, rgb(0xE0, 0xD6, 0xE0));
    px_rect(r, bx, by, bw * pd->joy, bh, KZ_PETAL_PINK);
    px_rect(r, bx, by, bw, 1, KZ_COCOA);
    px_rect(r, bx, by + bh - 1, bw, 1, KZ_COCOA);
    text_draw(r, "joy", bx + bw + 4, by + 1, KZ_COCOA);

    text_draw(r, "pet the cats!  -  travel to leave", 6, KZ_H - 22, KZ_COCOA);
}

int playdate_hit(const Playdate *pd, const Cat *your_cat, float px_, float py_) {
    if (!pd->active) return 0;
    if (your_cat && cat_hit(your_cat, px_, py_)) return 1;
    if (cat_hit(&pd->guest, px_, py_)) return 2;
    return 0;
}