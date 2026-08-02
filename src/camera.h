/* camera.h — a simple scrollable camera for panning around a room.
 *
 * Some rooms (the cottage) are bigger than the 240x160 screen. The camera is
 * an (x,y) offset into that larger room: everything in the room is drawn
 * shifted by -offset, and screen taps are converted back to room coordinates
 * by adding the offset. Dragging empty space pans the camera; the offset is
 * clamped so you never scroll past the room's edges.
 */
#ifndef KATIZTIC_CAMERA_H
#define KATIZTIC_CAMERA_H

#include <SDL3/SDL.h>

typedef struct {
    float x, y;          /* top-left of the view within the room */
    float room_w, room_h;/* the room's full size                 */
} Camera;

/* A camera for a room of the given size, centered to start. */
Camera camera_make(float room_w, float room_h);

/* Pan by a delta (e.g. from a drag), clamping to the room bounds. */
void camera_pan(Camera *c, float dx, float dy);

/* Clamp the offset so the view stays inside the room. */
void camera_clamp(Camera *c);

/* Convert a screen point to a room point (add the offset). */
void camera_to_room(const Camera *c, float sx, float sy, float *rx, float *ry);

#endif /* KATIZTIC_CAMERA_H */