/* music.c — see music.h. Procedural cozy music, generated sample by sample.
 *
 * How it works: each theme is a list of chords (sets of note frequencies).
 * We step through the chords slowly; for each, we sum a few soft sine voices
 * shaped by a gentle attack/release envelope, plus a quiet higher "twinkle"
 * melody note. The result is smooth, quiet, and loops forever. All mixing is
 * in floating point, output as stereo F32.
 */
#include "music.h"



#define SR        44100          /* sample rate                        */
#define CHANS     2              /* stereo                             */
#define CHORD_SEC 2.6f           /* seconds per chord — slow and calm  */

/* Note frequencies (Hz), a soft major-ish palette. */
#define C4 261.63f
#define D4 293.66f
#define E4 329.63f
#define F4 349.23f
#define G4 392.00f
#define A4 440.00f
#define B4 493.88f
#define C5 523.25f
#define E5 659.25f
#define G5 783.99f

/* A chord is up to 3 base frequencies plus one melody note. 0 = unused. */
typedef struct { float a, b, c, mel; } Chord;

/* Cottage: warm I–vi–IV–V feel in C, low and settled. */
static const Chord COTTAGE[] = {
    { C4, E4, G4, C5 },
    { A4, C5, E5, E5 },
    { F4, A4, C5, A4 },
    { G4, B4, D4, G5 },
};
/* Meadow: airier, brighter — moves up a little, more open voicings. */
static const Chord MEADOW[] = {
    { G4, B4, D4, G5 },
    { C4, E4, G4, E5 },
    { D4, F4, A4, A4 },
    { E4, G4, B4, C5 },
};

static const Chord *THEMES[MUSIC_THEME_COUNT] = { COTTAGE, MEADOW };
static const int THEME_LEN[MUSIC_THEME_COUNT] = {
    (int)(sizeof COTTAGE / sizeof COTTAGE[0]),
    (int)(sizeof MEADOW  / sizeof MEADOW[0]),
};

/* Player state, shared with the audio callback. Kept tiny and plain. */
typedef struct {
    SDL_AudioStream *stream;
    MusicTheme theme;
    int   chord_index;      /* which chord we're on                 */
    Uint64 sample_pos;      /* samples into the current chord       */
    double phase_a, phase_b, phase_c, phase_mel;  /* oscillator phases */
} Music;

static Music M;

/* Retro chip waveforms. Real 8-bit music (NES, Game Boy) is built from these
 * instead of smooth sines — that's what gives the crunchy, nostalgic sound. */
typedef enum { WAVE_SQUARE, WAVE_TRIANGLE } Wave;

/* A phase in 0..1 (one full cycle) makes the waveshapes easy to write. */
static float phase01(double *phase, float freq) {
    double inc = (double)freq / (double)SR;
    float p = (float)(*phase);
    *phase += inc;
    if (*phase >= 1.0) *phase -= 1.0;
    return p;
}

/* One chip voice at the given waveform. `duty` (for square) sets the pulse
 * width — 0.5 is a full square, 0.25/0.125 give thinner, brighter tones like
 * the classic NES pulse channels. */
static float voice_wave(double *phase, float freq, float amp,
                        Wave wave, float duty) {
    float p = phase01(phase, freq);   /* 0..1 within the cycle */
    float s;
    switch (wave) {
        case WAVE_SQUARE:
            s = (p < duty) ? 1.0f : -1.0f;
            break;
        case WAVE_TRIANGLE:
        default:
            /* rise 0->1 over first half, fall 1->0 over second, mapped -1..1 */
            s = (p < 0.5f) ? (p * 4.0f - 1.0f) : (3.0f - p * 4.0f);
            break;
    }
    return s * amp;
}

/* Envelope over one chord: gentle rise, long hold, gentle fall (0..1). */
static float envelope(Uint64 pos, Uint64 total) {
    float t = (float)pos / (float)total;         /* 0..1 across the chord */
    float atk = 0.18f, rel = 0.30f;
    if (t < atk)          return t / atk;                 /* fade in  */
    if (t > 1.0f - rel)   return (1.0f - t) / rel;        /* fade out */
    return 1.0f;                                          /* hold     */
}

/* The audio callback: fill `additional_amount` bytes of stereo F32. */
static void SDLCALL feed(void *userdata, SDL_AudioStream *stream,
                         int additional_amount, int total_amount) {
    (void)userdata; (void)total_amount;
    if (additional_amount <= 0) return;

    int frames = additional_amount / (int)(sizeof(float) * CHANS);
    Uint64 chord_samples = (Uint64)(CHORD_SEC * SR);

    /* Small temp buffer, filled in chunks to avoid a big stack array. */
    enum { CHUNK = 512 };
    float buf[CHUNK * CHANS];

    const Chord *theme = THEMES[M.theme];
    int len = THEME_LEN[M.theme];

    while (frames > 0) {
        int n = frames < CHUNK ? frames : CHUNK;
        for (int i = 0; i < n; i++) {
            const Chord *ch = &theme[M.chord_index];
            float env = envelope(M.sample_pos, chord_samples);

            /* Base triad — soft triangle waves, quiet, a mellow bed (like the
             * NES triangle channel). */
            float s = 0.0f;
            s += voice_wave(&M.phase_a, ch->a, 0.09f, WAVE_TRIANGLE, 0.5f);
            if (ch->b > 0) s += voice_wave(&M.phase_b, ch->b, 0.07f, WAVE_TRIANGLE, 0.5f);
            if (ch->c > 0) s += voice_wave(&M.phase_c, ch->c, 0.06f, WAVE_TRIANGLE, 0.5f);
            /* Melody — a bright pulse/square wave, the classic chiptune lead.
             * A 25% duty cycle gives that thin, nostalgic NES tone. */
            if (ch->mel > 0)
                s += voice_wave(&M.phase_mel, ch->mel, 0.07f, WAVE_SQUARE, 0.25f);

            s *= env * 0.6f;   /* overall gentle level */

            buf[i * 2 + 0] = s;   /* left  */
            buf[i * 2 + 1] = s;   /* right */

            /* advance time; move to next chord and loop */
            M.sample_pos++;
            if (M.sample_pos >= chord_samples) {
                M.sample_pos = 0;
                M.chord_index = (M.chord_index + 1) % len;
            }
        }
        SDL_PutAudioStreamData(stream, buf, n * (int)(sizeof(float) * CHANS));
        frames -= n;
    }
}

bool music_init(void) {
    SDL_AudioSpec spec;
    spec.freq = SR;
    spec.format = SDL_AUDIO_F32;
    spec.channels = CHANS;

    M.stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
                                         &spec, feed, NULL);
    if (!M.stream) return false;

    M.theme = MUSIC_COTTAGE;
    M.chord_index = 0;
    M.sample_pos = 0;
    M.phase_a = M.phase_b = M.phase_c = M.phase_mel = 0.0;

    SDL_ResumeAudioStreamDevice(M.stream);
    return true;
}

void music_set_theme(MusicTheme theme) {
    if (theme < 0 || theme >= MUSIC_THEME_COUNT) return;
    if (theme == M.theme) return;      /* no change */
    M.theme = theme;
    M.chord_index = 0;                 /* start the new progression fresh */
    M.sample_pos = 0;
}

void music_shutdown(void) {
    if (M.stream) {
        SDL_DestroyAudioStream(M.stream);
        M.stream = NULL;
    }
}