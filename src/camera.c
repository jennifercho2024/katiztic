/* camera.c — see camera.h. */
#include "camera.h"
#include "render.h"

Camera camera_make(float room_w, float room_h) {
    Camera c;
    c.room_w = room_w;
    c.room_h = room_h;
    /* start centered on the room */
    c.x = (room_w - KZ_W) / 2.0f;
    c.y = (room_h - KZ_H) / 2.0f;
    camera_clamp(&c);
    return c;
}

void camera_clamp(Camera *c) {
    float maxx = c->room_w - KZ_W;
    float maxy = c->room_h - KZ_H;
    if (maxx < 0) maxx = 0;
    if (maxy < 0) maxy = 0;
    if (c->x < 0) c->x = 0;
    if (c->y < 0) c->y = 0;
    if (c->x > maxx) c->x = maxx;
    if (c->y > maxy) c->y = maxy;
}

/* Clamp accounting for a zoom factor: when zoomed out (<1) the screen shows
 * more of the room (KZ_W/zoom wide), so the camera must stop sooner; when
 * zoomed in (>1) it shows less. Keeps the view from ever sliding off the room
 * and revealing empty space. */
void camera_clamp_zoom(Camera *c, float zoom) {
    if (zoom <= 0.0f) zoom = 1.0f;
    float view_w = (float)KZ_W / zoom;
    float view_h = (float)KZ_H / zoom;
    float maxx = c->room_w - view_w;
    float maxy = c->room_h - view_h;
    /* if the room is smaller than the view, center it (no empty edges) */
    if (maxx < 0) { c->x = maxx / 2.0f; } else {
        if (c->x < 0) c->x = 0;
        if (c->x > maxx) c->x = maxx;
    }
    if (maxy < 0) { c->y = maxy / 2.0f; } else {
        if (c->y < 0) c->y = 0;
        if (c->y > maxy) c->y = maxy;
    }
}

void camera_pan(Camera *c, float dx, float dy) {
    c->x += dx;
    c->y += dy;
    camera_clamp(c);
}

void camera_to_room(const Camera *c, float sx, float sy, float *rx, float *ry) {
    *rx = sx + c->x;
    *ry = sy + c->y;
}