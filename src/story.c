/* story.c — see story.h. Warmth rising, and remembering it. */
#include "story.h"

Story story_new(void) {
    Story st;
    for (int i = 0; i < STORY_ZONE_COUNT; i++) {
        st.warmth[i]     = 0.0f;
        st.seen_intro[i] = false;
        st.celebrated[i] = false;
    }
    return st;
}

float story_warmth(const Story *st, StoryZone z) {
    if (z < 0 || z >= STORY_ZONE_COUNT) return 1.0f;
    return st->warmth[z];
}

void story_visit_tick(Story *st, StoryZone z, int bond, int friends) {
    if (z < 0 || z >= STORY_ZONE_COUNT) return;
    if (st->warmth[z] >= 1.0f) return;   /* already fully warm */

    /* Warmth returns faster the deeper your cat's bond and the more friends
     * you've made — care and friendship are the magic. The base rate alone
     * works too, just slowly: simply being there helps. Tuned so a first
     * visit takes a couple of minutes of quiet company to fully re-color. */
    float rate = (0.6f + (float)bond / 60.0f + (float)friends * 0.25f)
                 / 7200.0f;   /* per frame at 60fps */
    st->warmth[z] += rate;
    if (st->warmth[z] > 1.0f) st->warmth[z] = 1.0f;
}

/* ---- persistence ---- */

static const char MAGIC[4] = { 'K', 'Z', 'S', 'T' };
#define STORY_SAVE_VERSION 1u

bool story_save(const Story *st, const char *path) {
    SDL_IOStream *io = SDL_IOFromFile(path, "wb");
    if (!io) return false;

    bool ok = SDL_WriteIO(io, MAGIC, 4) == 4;
    Uint8 ver = STORY_SAVE_VERSION;
    Uint8 count = (Uint8)STORY_ZONE_COUNT;
    ok = ok && SDL_WriteIO(io, &ver, 1) == 1;
    ok = ok && SDL_WriteIO(io, &count, 1) == 1;
    for (int i = 0; i < STORY_ZONE_COUNT && ok; i++) {
        Uint8 intro = st->seen_intro[i] ? 1 : 0;
        Uint8 celeb = st->celebrated[i] ? 1 : 0;
        ok = ok && SDL_WriteIO(io, &st->warmth[i], sizeof st->warmth[i])
                     == sizeof st->warmth[i];
        ok = ok && SDL_WriteIO(io, &intro, 1) == 1;
        ok = ok && SDL_WriteIO(io, &celeb, 1) == 1;
    }
    SDL_CloseIO(io);
    return ok;
}

bool story_load(Story *st, const char *path) {
    SDL_IOStream *io = SDL_IOFromFile(path, "rb");
    if (!io) return false;

    char magic[4];
    Uint8 ver = 0, count = 0;
    bool ok = SDL_ReadIO(io, magic, 4) == 4;
    ok = ok && SDL_memcmp(magic, MAGIC, 4) == 0;
    ok = ok && SDL_ReadIO(io, &ver, 1) == 1 && ver == STORY_SAVE_VERSION;
    ok = ok && SDL_ReadIO(io, &count, 1) == 1 && count == STORY_ZONE_COUNT;

    Story tmp = story_new();
    for (int i = 0; i < STORY_ZONE_COUNT && ok; i++) {
        Uint8 intro = 0, celeb = 0;
        ok = ok && SDL_ReadIO(io, &tmp.warmth[i], sizeof tmp.warmth[i])
                     == sizeof tmp.warmth[i];
        ok = ok && SDL_ReadIO(io, &intro, 1) == 1;
        ok = ok && SDL_ReadIO(io, &celeb, 1) == 1;
        if (ok) {
            if (tmp.warmth[i] < 0.0f) tmp.warmth[i] = 0.0f;
            if (tmp.warmth[i] > 1.0f) tmp.warmth[i] = 1.0f;
            tmp.seen_intro[i] = (intro != 0);
            tmp.celebrated[i] = (celeb != 0);
        }
    }
    SDL_CloseIO(io);

    if (ok) *st = tmp;
    return ok;
}