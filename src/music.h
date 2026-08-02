/* music.h — gentle, synthesized cozy music. One calm theme per location.
 *
 * No audio files: the music is generated in code from soft sine tones with
 * slow envelopes, mixed and looped. Each location (cottage, meadow) has its
 * own short chord progression, so the place you're in has its own mood. The
 * whole thing is deliberately quiet and smooth — no percussion, no sharp
 * attacks — matching the game's cozy, GBA-era feel.
 *
 * Usage: music_init() once at startup, music_set_theme() when the location
 * changes, music_shutdown() at the end. Generation happens in an audio
 * callback, so the game loop never has to feed it.
 */
#ifndef KATIZTIC_MUSIC_H
#define KATIZTIC_MUSIC_H

#include <SDL3/SDL.h>

typedef enum {
    MUSIC_COTTAGE,   /* warm, settled, home            */
    MUSIC_MEADOW,    /* airier, open, a little brighter */
    MUSIC_CAFE,      /* cozy, jazzy-warm, social        */
    MUSIC_THEME_COUNT
} MusicTheme;

/* Start the audio device and begin playing. Returns false if audio can't be
 * opened (the game still runs fine, just silent). */
bool music_init(void);

/* Switch which theme is playing. Smoothly — the current note finishes and the
 * new progression takes over. Safe to call every frame; only changes on a real
 * theme change. */
void music_set_theme(MusicTheme theme);

/* Stop and free the audio device. */
void music_shutdown(void);

#endif /* KATIZTIC_MUSIC_H */