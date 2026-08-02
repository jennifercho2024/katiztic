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

void camera_pan(Camera *c, float dx, float dy) {
    c->x += dx;
    c->y += dy;
    camera_clamp(c);
}

void camera_to_room(const Camera *c, float sx, float sy, float *rx, float *ry) {
    *rx = sx + c->x;
    *ry = sy + c->y;
}