/* render.c — see render.h. Three functions, no state. */
#include "render.h"

void px_rect(SDL_Renderer *r, float x, float y, float w, float h, Color c) {
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    SDL_FRect rect = { x, y, w, h };
    SDL_RenderFillRect(r, &rect);
}

void px(SDL_Renderer *r, float x, float y, Color c) {
    px_rect(r, x, y, 1.0f, 1.0f, c);
}

void px_rect_a(SDL_Renderer *r, float x, float y, float w, float h,
               Color c, Uint8 alpha) {
    /* Blend mode is set once in main; here we just vary alpha. */
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, alpha);
    SDL_FRect rect = { x, y, w, h };
    SDL_RenderFillRect(r, &rect);
}