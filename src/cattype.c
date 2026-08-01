/* cattype.c — see cattype.h. One color story per type. */
#include "cattype.h"

/* Each type's coat, tuned to stay in the pastel family (soft, no pure
 * black/white, mauve-leaning outlines handled by the drawing code). */
static const CatColors COLORS[KZ_TYPE_COUNT] = {
    /* Sunny — warm butter-cream with a peachy belly. */
    [KZ_SUNNY] = {
        RGB(0xF7,0xE2,0xB8), RGB(0xF0,0xC8,0x9E), RGB(0xF5,0xD0,0xB0),
        RGB(0xFB,0xF3,0xE4), RGB(0xF5,0xC0,0x9E),
    },
    /* Dreamy — soft lavender, mystical and cool. */
    [KZ_DREAMY] = {
        RGB(0xD8,0xCA,0xEC), RGB(0xC0,0xAE,0xDC), RGB(0xE0,0xD4,0xF0),
        RGB(0xF4,0xEE,0xFB), RGB(0xC8,0xB0,0xE0),
    },
    /* Playful — bright peachy-pink, bouncy. */
    [KZ_PLAYFUL] = {
        RGB(0xF7,0xC8,0xC0), RGB(0xE8,0xA8,0xA0), RGB(0xF5,0xD0,0xC8),
        RGB(0xFB,0xF0,0xEE), RGB(0xF0,0xA0,0x98),
    },
    /* Gentle — the soft rose-pink (our original starter). */
    [KZ_GENTLE] = {
        RGB(0xF5,0xD8,0xE4), RGB(0xE8,0xBB,0xD0), RGB(0xF0,0xC4,0xD8),
        RGB(0xFB,0xF0,0xF5), RGB(0xF0,0xAE,0xC8),
    },
    /* Clever — cool mint-blue, tidy and curious. */
    [KZ_CLEVER] = {
        RGB(0xC4,0xE4,0xDE), RGB(0xA4,0xCE,0xC8), RGB(0xD4,0xEC,0xE6),
        RGB(0xF0,0xF8,0xF5), RGB(0xA8,0xD0,0xC8),
    },
};

static const char *NAMES[KZ_TYPE_COUNT] = {
    "Sunny", "Dreamy", "Playful", "Gentle", "Clever",
};

CatColors cattype_colors(CatType t) {
    if (t < 0 || t >= KZ_TYPE_COUNT) t = KZ_GENTLE;
    return COLORS[t];
}

const char *cattype_name(CatType t) {
    if (t < 0 || t >= KZ_TYPE_COUNT) t = KZ_GENTLE;
    return NAMES[t];
}