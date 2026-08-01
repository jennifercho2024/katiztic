/* text.h — a tiny 4x6 pixel font, drawn in any pastel color.
 *
 * A bitmap font is just a lookup table: each character maps to 6 rows of
 * 4 pixels. We store each glyph as 6 bytes, one per row; the low 4 bits of
 * each byte say which pixels in that row are lit (bit 3 = leftmost).
 *
 * This unlocks readable UI everywhere — cat names, stat labels, button text —
 * with no font-file dependency, matching the project's "build from source,
 * zero assets" spirit.
 */
#ifndef KATIZTIC_TEXT_H
#define KATIZTIC_TEXT_H

#include <SDL3/SDL.h>
#include "palette.h"

/* Glyph cell is 4 wide, 6 tall. Characters are spaced 1px apart. */
#define KZ_GLYPH_W 4
#define KZ_GLYPH_H 6
#define KZ_CHAR_ADVANCE (KZ_GLYPH_W + 1)

/* Draw a string at (x,y) in the given color, left-aligned. Unknown characters
 * render as a blank space. Returns the pixel width drawn. */
float text_draw(SDL_Renderer *r, const char *s, float x, float y, Color c);

/* Pixel width a string would occupy, for centering/right-aligning. */
float text_width(const char *s);

/* Draw centered on `cx`. Convenience for labels and titles. */
void text_draw_centered(SDL_Renderer *r, const char *s, float cx, float y,
                        Color c);

#endif /* KATIZTIC_TEXT_H */