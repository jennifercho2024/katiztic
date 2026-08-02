/* icon.h — the embedded window icon.
 *
 * icon_create() returns a freshly created SDL_Surface of the Katiztic cat
 * face, built from pixels baked into the program (no external file, no image
 * library). Pass it to SDL_SetWindowIcon, then free it with SDL_DestroySurface.
 */
#ifndef KATIZTIC_ICON_H
#define KATIZTIC_ICON_H

#include <SDL3/SDL.h>

SDL_Surface *icon_create(void);

#endif /* KATIZTIC_ICON_H */