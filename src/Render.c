/* render.c — see render.h. Pixel-rect primitives with an optional camera
 * offset so whole rooms can be panned as one. */
#include "render.h"

/* Global drawing offset for camera panning. Room drawing sets this so the
 * entire room shifts together; UI drawing clears it to stay screen-fixed.
 * Every primitive subtracts it, so no other file needs to know about it. */
static float g_off_x = 0.0f, g_off_y = 0.0f;
static float g_zoom = 1.0f;   /* room zoom factor (1 = normal) */

void render_set_offset(float x, float y) { g_off_x = x; g_off_y = y; }
void render_clear_offset(void) { g_off_x = 0.0f; g_off_y = 0.0f; g_zoom = 1.0f; }
void render_set_zoom(float z) { g_zoom = (z < 0.1f) ? 0.1f : z; }
float render_get_zoom(void) { return g_zoom; }

/* An optional pivot scale: everything drawn is scaled by g_pscale around the
 * point (g_pvx, g_pvy). Used to grow/shrink a whole sprite (e.g. a kitten)
 * cleanly, so every part scales together around its center. Applied before the
 * camera offset/zoom. */
static float g_pscale = 1.0f, g_pvx = 0.0f, g_pvy = 0.0f;

void render_set_pivot_scale(float px_, float py_, float s) {
    g_pvx = px_; g_pvy = py_; g_pscale = (s < 0.05f) ? 0.05f : s;
}
void render_clear_pivot_scale(void) { g_pscale = 1.0f; g_pvx = 0.0f; g_pvy = 0.0f; }

/* Transform a world point through the pivot scale, then camera offset+zoom. */
static void xform(float x, float y, float *ox, float *oy) {
    float wx = (x - g_pvx) * g_pscale + g_pvx;
    float wy = (y - g_pvy) * g_pscale + g_pvy;
    *ox = (wx - g_off_x) * g_zoom;
    *oy = (wy - g_off_y) * g_zoom;
}

/* Global warmth for the story's re-coloring magic: 1 = full color, 0 = faded
 * grey. Zone drawing sets it from the zone's story warmth; everything else
 * draws at 1. Colors are pulled toward their grey luminance as warmth drops,
 * so a faded place literally loses its pastels — and regains them. */
static float g_warmth = 1.0f;

void render_set_warmth(float w) {
    if (w < 0.0f) w = 0.0f;
    if (w > 1.0f) w = 1.0f;
    g_warmth = w;
}

static Color apply_warmth(Color c) {
    if (g_warmth >= 1.0f) return c;
    float gray = 0.299f * (float)c.r + 0.587f * (float)c.g + 0.114f * (float)c.b;
    Color out = c;
    out.r = (Uint8)(gray + ((float)c.r - gray) * g_warmth);
    out.g = (Uint8)(gray + ((float)c.g - gray) * g_warmth);
    out.b = (Uint8)(gray + ((float)c.b - gray) * g_warmth);
    return out;
}

void px_rect(SDL_Renderer *r, float x, float y, float w, float h, Color c) {
    Color cc = apply_warmth(c);
    SDL_SetRenderDrawColor(r, cc.r, cc.g, cc.b, cc.a);
    float sx, sy;
    xform(x, y, &sx, &sy);
    SDL_FRect rect = { sx, sy, w * g_pscale * g_zoom, h * g_pscale * g_zoom };
    SDL_RenderFillRect(r, &rect);
}

void px(SDL_Renderer *r, float x, float y, Color c) {
    px_rect(r, x, y, 1.0f, 1.0f, c);
}

void px_rect_a(SDL_Renderer *r, float x, float y, float w, float h,
               Color c, Uint8 alpha) {
    /* Blend mode is set once in main; here we just vary alpha. */
    Color cc = apply_warmth(c);
    SDL_SetRenderDrawColor(r, cc.r, cc.g, cc.b, alpha);
    float sx, sy;
    xform(x, y, &sx, &sy);
    SDL_FRect rect = { sx, sy, w * g_pscale * g_zoom, h * g_pscale * g_zoom };
    SDL_RenderFillRect(r, &rect);
}