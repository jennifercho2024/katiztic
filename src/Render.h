/* render.h — the thinnest possible pixel-drawing layer over SDL_Render.
 *
 * The whole game draws in a 240x160 logical space (GBA proportions). SDL's
 * integer-scale presentation blows that up to the window with crisp, square
 * pixels. Everything here works in that 240x160 space.
 *
 * Deliberately tiny: two fill primitives and an alpha helper. If you can see
 * the whole drawing API on one screen, you understand exactly what the game
 * can and can't put on screen.
 */
#ifndef KATIZTIC_RENDER_H
#define KATIZTIC_RENDER_H

#include <SDL3/SDL.h>
#include "palette.h"

/* Logical canvas — the retro constraint, honored everywhere. */
#define KZ_W 240
#define KZ_H 160

/* Fill a rectangle in logical pixels. Floats because SDL_FRect is float,
 * but we always pass whole numbers so pixels land on the grid. */
void px_rect(SDL_Renderer *r, float x, float y, float w, float h, Color c);

/* A single logical pixel (or a small square block). Convenience for sprites. */
void px(SDL_Renderer *r, float x, float y, Color c);

/* Same as px_rect but with an explicit alpha 0..255, for soft overlays
 * (dappled light, shadows, the time-of-day wash). */
void px_rect_a(SDL_Renderer *r, float x, float y, float w, float h,
               Color c, Uint8 alpha);

/* Camera offset for panning: room drawing sets this so the whole room shifts
 * as one; UI drawing clears it to stay fixed on screen. The primitives above
 * subtract it automatically, so nothing else needs to handle panning. */
void render_set_offset(float x, float y);
void render_clear_offset(void);

/* Story warmth for the re-coloring magic: 1 = full color, 0 = faded grey.
 * Zone drawing sets it from the zone's warmth; set it back to 1 for anything
 * that should keep its color (your cats, the UI). */
void render_set_warmth(float w);

#endif /* KATIZTIC_RENDER_H */