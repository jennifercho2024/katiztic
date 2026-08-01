/* cattype.h — the five cat types and their pastel color stories.
 *
 * From the design doc: types set personality and a light aesthetic flavor —
 * never a combat chart. The most important job a type does here is give each
 * cat a signature color set, so a team of five *looks* like a chosen aesthetic
 * rather than five identical cats.
 */
#ifndef KATIZTIC_CATTYPE_H
#define KATIZTIC_CATTYPE_H

#include "palette.h"

typedef enum {
    KZ_SUNNY,    /* warm, energetic — loves walks, sunbathing */
    KZ_DREAMY,   /* sleepy, mystical — naps, stargazing        */
    KZ_PLAYFUL,  /* bouncy, social — play, other cats          */
    KZ_GENTLE,   /* shy, sweet — quiet time, grooming          */
    KZ_CLEVER,   /* curious, tidy — puzzles, exploring         */
    KZ_TYPE_COUNT
} CatType;

/* The colors that make a cat look like its type. */
typedef struct {
    Color body;     /* main coat            */
    Color dark;     /* belly, stripes       */
    Color ear;      /* inner ear            */
    Color paw;      /* paws / socks         */
    Color cheek;    /* cheek blush          */
} CatColors;

/* Look up a type's colors and its display name. */
CatColors    cattype_colors(CatType t);
const char  *cattype_name(CatType t);

#endif /* KATIZTIC_CATTYPE_H */