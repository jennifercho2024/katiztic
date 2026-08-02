/* render.c — see render.h. Pixel-rect primitives with an optional camera
 * offset so whole rooms can be panned as one. */
#include "render.h"

/* Global drawing offset for camera panning. Room drawing sets this so the
 * entire room shifts together; UI drawing clears it to stay screen-fixed.
 * Every primitive subtracts it, so no other file needs to know about it. */
static float g_off_x = 0.0f, g_off_y = 0.0f;

void render_set_offset(float x, float y) { g_off_x = x; g_off_y = y; }
void render_clear_offset(void) { g_off_x = 0.0f; g_off_y = 0.0f; }

void px_rect(SDL_Renderer *r, float x, float y, float w, float h, Color c) {
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    SDL_FRect rect = { x - g_off_x, y - g_off_y, w, h };
    SDL_RenderFillRect(r, &rect);
}

void px(SDL_Renderer *r, float x, float y, Color c) {
    px_rect(r, x, y, 1.0f, 1.0f, c);
}

void px_rect_a(SDL_Renderer *r, float x, float y, float w, float h,
               Color c, Uint8 alpha) {
    /* Blend mode is set once in main; here we just vary alpha. */
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, alpha);
    SDL_FRect rect = { x - g_off_x, y - g_off_y, w, h };
    SDL_RenderFillRect(r, &rect);
}