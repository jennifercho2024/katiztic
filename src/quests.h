/* quests.h — gentle goals to reach, straight from the design doc's "villagers
 * & gentle quests" spirit (minus the villagers, for now).
 *
 * Each quest is a soft target — pet your cats so many times, befriend wild
 * cats, bring the color back to a faded place. Completing one celebrates with
 * a banner and awards XP to EVERY cat in the family, so quests literally
 * level your cats up — and since levels and friendships are what unlock the
 * earned items and re-color the world, the quest list doubles as a gentle
 * guide through everything there is to discover.
 *
 * Progress is monotonic (it only rises) and saved to its own file.
 */
#ifndef KATIZTIC_QUESTS_H
#define KATIZTIC_QUESTS_H

#include <SDL3/SDL.h>

typedef enum {
    QUEST_PET,       /* pet your cats 15 times          */
    QUEST_FEED,      /* serve 10 meals                  */
    QUEST_GROOM,     /* groom 8 times                   */
    QUEST_FRIENDS,   /* befriend 2 wild cats            */
    QUEST_FAMILY,    /* grow the family to 4 cats       */
    QUEST_LEVEL5,    /* reach level 5 with a cat        */
    QUEST_CAFE,      /* visit the cafe                  */
    QUEST_PLAY,      /* see two cats play together      */
    QUEST_FOREST,    /* bring color back to the forest  */
    QUEST_STREET,    /* bring color back to the street  */
    QUEST_COUNT
} QuestId;

typedef struct {
    const char *desc;      /* short line for the quest list      */
    Uint16      target;    /* progress needed to complete        */
    Uint16      reward_xp; /* XP granted to EVERY cat on finish  */
} QuestInfo;

typedef struct {
    Uint16 progress[QUEST_COUNT];
    bool   done[QUEST_COUNT];
} Quests;

/* The static description/target/reward for a quest. */
const QuestInfo *quest_info(QuestId q);

/* A fresh quest log: everything at zero. */
Quests quests_new(void);

/* Add 1 to a counting quest (pets, meals...). Returns true if this bump just
 * completed it — the caller celebrates and awards the reward. */
bool quests_bump(Quests *q, QuestId id);

/* Raise a value-tracked quest (family size, best level, zone recolored) to
 * `value` if that's higher than current progress — progress never regresses.
 * Returns true if this call just completed it. */
bool quests_set(Quests *q, QuestId id, int value);

/* How many quests are done (for the list header). */
int quests_done_count(const Quests *q);

/* ---- persistence: its own save file ---- */
bool quests_save(const Quests *q, const char *path);
bool quests_load(Quests *q, const char *path);

#endif /* KATIZTIC_QUESTS_H */