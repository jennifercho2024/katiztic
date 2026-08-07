/* quests.c — see quests.h. The goal table, progress, and remembering it. */
#include "quests.h"

static const QuestInfo INFO[QUEST_COUNT] = {
    /* desc                    target reward repeat  icon        */
    { "Pet cats 15 times",        15,   25,  true,  QICON_PAW     },
    { "Serve 10 meals",           10,   25,  true,  QICON_FISH    },
    { "Groom 8 times",             8,   25,  true,  QICON_SPARKLE },
    { "Befriend 2 wild cats",      2,   40,  false, QICON_HEART   },
    { "Grow to 4 cats",            4,   30,  false, QICON_CATS    },
    { "Reach level 5",             5,   40,  false, QICON_STAR    },
    { "Visit the cafe",            1,   15,  false, QICON_CUP     },
    { "See cats play",             1,   15,  false, QICON_CATS    },
    { "Recolor the forest",        1,   60,  false, QICON_LEAF    },
    { "Recolor the street",        1,   60,  false, QICON_LEAF    },
    { "Raise a cat to grown-up",  25,   75,  false, QICON_STAR    },
    { "Grow 3 cats to adults",     3,  120,  false, QICON_CATS    },
};

const QuestInfo *quest_info(QuestId q) {
    if (q < 0 || q >= QUEST_COUNT) q = QUEST_PET;
    return &INFO[q];
}

Uint16 quest_live_target(const Quests *qs, QuestId q) {
    if (q < 0 || q >= QUEST_COUNT) return 1;
    Uint16 base = INFO[q].target;
    if (!INFO[q].repeatable) return base;
    /* each completion nudges the target up ~20% of the base, so repeats stay
     * fresh and slowly ask a little more */
    Uint16 step = (Uint16)(base / 5); if (step < 1) step = 1;
    long t = (long)base + (long)qs->completions[q] * step;
    if (t > 60000) t = 60000;
    return (Uint16)t;
}

Quests quests_new(void) {
    Quests q;
    for (int i = 0; i < QUEST_COUNT; i++) {
        q.progress[i] = 0;
        q.done[i] = false;
        q.completions[i] = 0;
    }
    return q;
}

/* Shared completion check against the live target. Returns true on the moment
 * of completion. A repeatable quest resets (progress 0, a fresh higher target)
 * so it's ready to do again; a milestone latches done forever. */
static bool check_done(Quests *q, QuestId id) {
    if (q->done[id]) return false;
    if (q->progress[id] >= quest_live_target(q, (QuestId)id)) {
        q->completions[id]++;
        if (INFO[id].repeatable) {
            q->progress[id] = 0;      /* ready for another round */
            /* done stays false: it never shows as permanently checked */
        } else {
            q->done[id] = true;       /* milestone: latched */
        }
        return true;
    }
    return false;
}

bool quests_bump(Quests *q, QuestId id) {
    if (id < 0 || id >= QUEST_COUNT) return false;
    if (q->done[id]) return false;
    if (q->progress[id] < 60000) q->progress[id]++;
    return check_done(q, id);
}

bool quests_set(Quests *q, QuestId id, int value) {
    if (id < 0 || id >= QUEST_COUNT) return false;
    if (q->done[id]) return false;
    if (value < 0) value = 0;
    if (value > 60000) value = 60000;
    if ((Uint16)value > q->progress[id])
        q->progress[id] = (Uint16)value;   /* monotonic while unfinished */
    return check_done(q, id);
}

int quests_done_count(const Quests *q) {
    int n = 0;
    for (int i = 0; i < QUEST_COUNT; i++)
        if (q->done[i]) n++;
    return n;
}

int quests_total_completed(const Quests *q) {
    int n = 0;
    for (int i = 0; i < QUEST_COUNT; i++)
        n += (int)q->completions[i];
    return n;
}

/* ---- persistence ---- */

static const char MAGIC[4] = { 'K', 'Z', 'Q', 'S' };
#define QUESTS_SAVE_VERSION 2u

bool quests_save(const Quests *q, const char *path) {
    SDL_IOStream *io = SDL_IOFromFile(path, "wb");
    if (!io) return false;

    bool ok = SDL_WriteIO(io, MAGIC, 4) == 4;
    Uint8 ver = QUESTS_SAVE_VERSION;
    Uint8 count = (Uint8)QUEST_COUNT;
    ok = ok && SDL_WriteIO(io, &ver, 1) == 1;
    ok = ok && SDL_WriteIO(io, &count, 1) == 1;
    for (int i = 0; i < QUEST_COUNT && ok; i++) {
        Uint8 done = q->done[i] ? 1 : 0;
        ok = ok && SDL_WriteIO(io, &q->progress[i], sizeof q->progress[i])
                     == sizeof q->progress[i];
        ok = ok && SDL_WriteIO(io, &done, 1) == 1;
        ok = ok && SDL_WriteIO(io, &q->completions[i], sizeof q->completions[i])
                     == sizeof q->completions[i];
    }
    SDL_CloseIO(io);
    return ok;
}

bool quests_load(Quests *q, const char *path) {
    SDL_IOStream *io = SDL_IOFromFile(path, "rb");
    if (!io) return false;

    char magic[4];
    Uint8 ver = 0, count = 0;
    bool ok = SDL_ReadIO(io, magic, 4) == 4;
    ok = ok && SDL_memcmp(magic, MAGIC, 4) == 0;
    ok = ok && SDL_ReadIO(io, &ver, 1) == 1 && ver == QUESTS_SAVE_VERSION;
    /* Accept logs from before newer quests existed — old progress survives
     * when the quest list grows, same as the story saves. */
    ok = ok && SDL_ReadIO(io, &count, 1) == 1
            && count >= 1 && count <= QUEST_COUNT;

    Quests tmp = quests_new();
    for (int i = 0; i < (int)count && ok; i++) {
        Uint8 done = 0;
        ok = ok && SDL_ReadIO(io, &tmp.progress[i], sizeof tmp.progress[i])
                     == sizeof tmp.progress[i];
        ok = ok && SDL_ReadIO(io, &done, 1) == 1;
        ok = ok && SDL_ReadIO(io, &tmp.completions[i],
                              sizeof tmp.completions[i])
                     == sizeof tmp.completions[i];
        if (ok) tmp.done[i] = (done != 0);
    }
    SDL_CloseIO(io);

    if (ok) *q = tmp;
    return ok;
}