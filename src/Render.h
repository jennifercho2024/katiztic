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

#endif /* KATIZTIC_RENDER_H */