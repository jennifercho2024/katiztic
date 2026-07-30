/* palette.h — Katiztic's color system.
 *
 * One rule does most of the "ethereal" work: no pure black, no pure white,
 * no harsh saturation. Outlines are dark mauve, backgrounds are cream.
 * Every color the game draws lives here so the whole aesthetic can be
 * retuned from one file.
 */
#ifndef KATIZTIC_PALETTE_H
#define KATIZTIC_PALETTE_H

#include <SDL3/SDL_stdinc.h>

/* A color is just four bytes. Kept plain and visible on purpose. */
typedef struct {
    Uint8 r, g, b, a;
} Color;

/* Helper so call sites read like rgb(...) instead of a brace-list.
 * These are fine anywhere a runtime value is expected. */
static inline Color rgb(Uint8 r, Uint8 g, Uint8 b) {
    Color c = { r, g, b, 255 };
    return c;
}
static inline Color rgba(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    Color c = { r, g, b, a };
    return c;
}

/* Constant-expression form: expands to a brace-initializer, not a call, so
 * it's legal inside `static const` tables (function calls are not). Use this
 * for compile-time data like the time-of-day table; use rgb()/rgba() for
 * anything computed at runtime. */
#define RGB(r, g, b)  { (Uint8)(r), (Uint8)(g), (Uint8)(b), 255 }

/* ---- Core pastel palette (from the design doc) ---- */
#define KZ_PETAL_PINK   rgb(0xF7, 0xC8, 0xD8)  /* primary accent            */
#define KZ_LAVENDER     rgb(0xD4, 0xC2, 0xE8)  /* skies, magic              */
#define KZ_MINT         rgb(0xC8, 0xE8, 0xD4)  /* foliage, calm zones       */
#define KZ_BUTTER       rgb(0xF5, 0xE6, 0xC8)  /* warm light, sand          */
#define KZ_SKY_WASH     rgb(0xC2, 0xDF, 0xE8)  /* water, distance           */
#define KZ_COCOA        rgb(0x8B, 0x7B, 0x8B)  /* soft text (never black)   */
#define KZ_CLOUD        rgb(0xFB, 0xF6, 0xF2)  /* backgrounds (never white) */

/* ---- Cat colors (the pink starter, "Gentle" type flavor) ---- */
#define KZ_CAT_BODY     rgb(0xF5, 0xD8, 0xE4)
#define KZ_CAT_DARK     rgb(0xE8, 0xBB, 0xD0)  /* belly, stripes            */
#define KZ_CAT_EAR      rgb(0xF0, 0xC4, 0xD8)  /* inner ear                 */
#define KZ_CAT_PAW      rgb(0xFB, 0xF0, 0xF5)  /* paws, socks               */
#define KZ_CAT_OUTLINE  rgb(0x8B, 0x6B, 0x7B)  /* eyes/outline — mauve      */
#define KZ_CAT_NOSE     rgb(0xD4, 0x8B, 0xA8)
#define KZ_CAT_CHEEK    rgb(0xF0, 0xAE, 0xC8)
#define KZ_HEART        rgb(0xE8, 0x8B, 0xAE)  /* petting feedback          */

#endif /* KATIZTIC_PALETTE_H */