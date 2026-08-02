/* story.h — the world's gentle magic, and the heart of Katiztic's story.
 *
 * The world's warmth is fading in places: some zones have lost their color and
 * gone soft grey. Cared-for cats radiate a quiet magic — so when you visit a
 * faded place with a cat you've bonded with, color slowly seeps back into it.
 * The deeper her bond, and the more friends you've made on your walks, the
 * faster the warmth returns. When a zone reaches full color, it stays that
 * way: a place you healed together.
 *
 * Each zone has a "warmth" from 0 (faded grey) to 1 (full color). The render
 * layer uses it to desaturate the zone's art, so the re-coloring is literal —
 * the pastel palette itself is the reward.
 */
#ifndef KATIZTIC_STORY_H
#define KATIZTIC_STORY_H

#include <SDL3/SDL.h>

/* The zones the story tracks. (More join this list as the world grows.) */
typedef enum {
    STORY_ZONE_FOREST,
    STORY_ZONE_COUNT
} StoryZone;

typedef struct {
    float warmth[STORY_ZONE_COUNT];      /* 0 = faded grey .. 1 = full color */
    bool  seen_intro[STORY_ZONE_COUNT];  /* shown the "it's faded" moment?   */
    bool  celebrated[STORY_ZONE_COUNT];  /* shown the "color returns!" one?  */
} Story;

/* A fresh story: every tracked zone starts faded. */
Story story_new(void);

/* The zone's current warmth, 0..1. */
float story_warmth(const Story *st, StoryZone z);

/* One frame of visiting a zone with your active cat. Warmth rises a little,
 * scaled by her bond and by how many wild cats you've befriended — bonding
 * and friendship are what bring the color back. */
void story_visit_tick(Story *st, StoryZone z, int bond, int friends);

/* ---- persistence: its own save file ---- */
bool story_save(const Story *st, const char *path);
bool story_load(Story *st, const char *path);

#endif /* KATIZTIC_STORY_H */