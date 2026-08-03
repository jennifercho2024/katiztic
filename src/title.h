/* title.h — the Katiztic title screen (embedded splash image).
 *
 * Shown when the game opens; pressing any key or clicking begins play. The
 * image is embedded as raw pixels (see title.c) so there's no external file.
 */
#ifndef KATIZTIC_TITLE_H
#define KATIZTIC_TITLE_H

#include <SDL3/SDL.h>

/* Build the title texture (caller owns it; destroy with SDL_DestroyTexture). */
SDL_Texture *title_create(SDL_Renderer *r);

#endif /* KATIZTIC_TITLE_H */