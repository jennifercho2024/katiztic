/* music.c — see music.h. Procedural cozy lo-fi music, generated per sample.
 *
 * The vibe: dreamy, hazy, warm, and calm — the kind of slow lo-fi you'd
 * unwind to. It's built from a few ingredients that together give that feel:
 *
 *   - soft rounded tones (sine-ish, not sharp square waves)
 *   - a SLIGHT DETUNE between paired voices — two oscillators a hair apart
 *     beat gently against each other, the classic warm, woozy lo-fi shimmer
 *   - long, spacious chords with slow attacks and long release tails, so
 *     notes bleed into one another (the hazy quality)
 *   - a quiet, slow BASS PULSE for a gentle head-nod groove, kept low in the
 *     mix so it's felt more than heard
 *   - a drifting ARPEGGIO that rolls softly through each chord instead of a
 *     bright lead poking out
 *
 * Everything is deliberately quiet — this is background, not foreground.
 * Output is stereo F32, generated in the audio callback.
 */
#include "music.h"
#include <math.h>

#define KZ_TAU 6.283185307179586

#define SR        44100          /* sample rate                          */
#define CHANS     2              /* stereo                               */
#define CHORD_SEC 4.4f           /* long, drifting chords — dreamy & slow */
#define BPM       66.0f          /* slow lo-fi head-nod tempo            */

/* Note frequencies (Hz), a soft major-ish palette. */
#define C3 130.81f
#define E3 164.81f
#define F3 174.61f
#define G3 196.00f
#define A3 220.00f
#define C4 261.63f
#define D4 293.66f
#define E4 329.63f
#define F4 349.23f
#define G4 392.00f
#define A4 440.00f
#define B4 493.88f
#define C5 523.25f
#define D5 587.33f
#define E5 659.25f
#define F5 698.46f
#define G5 783.99f
#define A5 880.00f

/* A chord: a low bass note, three mid tones for the pad, and a set of notes
 * the arpeggio drifts through. 0 = unused. */
typedef struct {
    float bass;
    float a, b, c;      /* pad triad */
    float arp[4];       /* arpeggio notes (rolled through slowly) */
} Chord;

/* Cottage: warm, settled — a mellow ii–V–I–vi kind of drift in C. */
static const Chord COTTAGE[] = {
    { C3, C4, E4, G4, { C4, E4, G4, C5 } },
    { A3, A4, C5, E5, { A4, C5, E4, A4 } },
    { F3, F4, A4, C5, { F4, A4, C5, E5 } },
    { G3, G4, B4, D5, { G4, B4, D5, G4 } },
};
/* Meadow: a touch brighter and more open, but the same dreamy calm. */
static const Chord MEADOW[] = {
    { G3, G4, B4, D5, { G4, B4, D5, G4 } },
    { E3, E4, G4, B4, { E4, G4, B4, E5 } },
    { C4, C4, E4, G4, { C4, E4, G4, C5 } },
    { D4, D4, F4, A4, { D4, F4, A4, D5 } },
};
/* Café: cozy and a little jazzy — warmer 7th-ish colors, an intimate,
 * social lounge feel. Still slow and soft, just a touch more soulful. */
static const Chord CAFE[] = {
    { A3, C4, E4, G4, { C4, E4, G4, B4 } },   /* Am7 warmth      */
    { D4, F4, A4, C5, { F4, A4, C5, E5 } },   /* Dm7             */
    { G3, G4, B4, E4, { G4, B4, D5, E5 } },   /* G-ish           */
    { C4, E4, G4, B4, { E4, G4, B4, C5 } },   /* Cmaj7 resolve   */
};
/* Forest: wistful and hushed — a minor-leaning drift, the sound of a quiet
 * wood waiting for its color to come back. */
static const Chord FOREST[] = {
    { A3, A4, C5, E5, { A4, C5, E5, C5 } },   /* Am — the ache    */
    { F3, F4, A4, C5, { F4, A4, C5, A4 } },   /* F — soft comfort */
    { C4, C4, E4, G4, { E4, G4, C5, G4 } },   /* C — a warm turn  */
    { G3, G4, B4, D5, { G4, B4, D5, B4 } },   /* G — gentle hope  */
};
/* Street: bright and ambling — the friendliest progression there is, for a
 * neighborly stroll past the pastel house fronts. */
static const Chord STREET[] = {
    { C4, C4, E4, G4, { C4, E4, G4, E5 } },   /* C — hello        */
    { G3, G4, B4, D5, { G4, B4, D5, G4 } },   /* G — a wave       */
    { A3, A4, C5, E5, { A4, C5, E5, C5 } },   /* Am — soft shade  */
    { F3, F4, A4, C5, { F4, A4, C5, F4 } },   /* F — home again   */
};

/* Park: bright and adventurous — a bold, uplifting lift that feels like a day
 * out full of play. The most energetic, "let's go!" progression of the set. */
static const Chord PARK[] = {
    { C4, C4, E4, G4, { C4, E4, G4, C5 } },   /* C — set off!     */
    { G3, G4, B4, D5, { G4, B4, D5, G5 } },   /* G — striding on  */
    { A3, A4, C5, E5, { A4, C5, E5, A5 } },   /* Am — a flourish  */
    { F3, F4, A4, C5, { F4, A4, C5, F5 } },   /* F — onward       */
    { D4, D4, F4, A4, { D4, F4, A4, D5 } },   /* Dm — a turn      */
    { G3, G4, B4, D5, { G4, B4, D5, B4 } },   /* G — homeward lift */
};

static const Chord *THEMES[MUSIC_THEME_COUNT] = {
    COTTAGE, MEADOW, CAFE, FOREST, STREET, PARK
};
static const int THEME_LEN[MUSIC_THEME_COUNT] = {
    (int)(sizeof COTTAGE / sizeof COTTAGE[0]),
    (int)(sizeof MEADOW  / sizeof MEADOW[0]),
    (int)(sizeof CAFE    / sizeof CAFE[0]),
    (int)(sizeof FOREST  / sizeof FOREST[0]),
    (int)(sizeof STREET  / sizeof STREET[0]),
    (int)(sizeof PARK    / sizeof PARK[0]),
};

/* Player state, shared with the audio callback. Kept plain. */
typedef struct {
    SDL_AudioStream *stream;
    MusicTheme theme;
    int    chord_index;
    Uint64 sample_pos;      /* samples into the current chord */
    /* Oscillator phases. Paired voices (…and their _det detuned twins) give
     * the warm lo-fi beating. */
    double ph_a, ph_a_det;
    double ph_b, ph_b_det;
    double ph_c, ph_c_det;
    double ph_bass;
    double ph_arp, ph_arp_det;
    /* one-pole low-pass state per channel, for a soft hazy top-end */
    float lp_l, lp_r;
} Music;

static Music M;

/* A soft, rounded tone: mostly a sine, with a whisper of the octave above to
 * give it a little body without any harshness. Advances `phase` (in radians)
 * and returns the sample scaled by amp. */
static float tone(double *phase, float freq, float amp) {
    double inc = KZ_TAU * (double)freq / (double)SR;
    double p = *phase;
    float s = (float)(sin(p) + 0.15 * sin(2.0 * p)) * amp;
    *phase = p + inc;
    if (*phase > KZ_TAU) *phase -= KZ_TAU;
    return s;
}

/* Envelope across one chord: long slow rise, long hold, long release — so the
 * chords bleed into each other hazily (0..1). */
static float envelope(Uint64 pos, Uint64 total) {
    float t = (float)pos / (float)total;
    float atk = 0.30f, rel = 0.40f;
    if (t < atk)         return t / atk;
    if (t > 1.0f - rel)  return (1.0f - t) / rel;
    return 1.0f;
}

/* A gentle bass pulse: one soft swell per beat, so the groove is felt but soft.
 * Returns a 0..1 amplitude shaped like a slow heartbeat. */
static float bass_pulse(Uint64 sample_pos) {
    float beat_samples = (60.0f / BPM) * SR;
    float phase = fmodf((float)sample_pos, beat_samples) / beat_samples; /* 0..1 */
    /* quick soft rise, gentle fall — a mellow thump */
    float env = phase < 0.15f ? (phase / 0.15f)
                              : (1.0f - (phase - 0.15f) / 0.85f);
    if (env < 0) env = 0;
    return env * env;   /* rounder */
}

/* Per-note envelope for each arpeggio pluck: a soft attack and a long gentle
 * decay so notes ring like a music box instead of clicking on and off. `pos`
 * is samples into the current arp step, `total` the step length. */
static float arp_env(Uint64 pos, Uint64 total) {
    float t = (float)pos / (float)total;   /* 0..1 across the note */
    float atk = 0.06f;
    if (t < atk) return t / atk;           /* quick soft attack */
    /* smooth exponential-ish decay over the rest of the note */
    float d = (t - atk) / (1.0f - atk);
    return (1.0f - d) * (1.0f - d);
}

static void SDLCALL feed(void *userdata, SDL_AudioStream *stream,
                         int additional_amount, int total_amount) {
    (void)userdata; (void)total_amount;
    if (additional_amount <= 0) return;

    int frames = additional_amount / (int)(sizeof(float) * CHANS);
    Uint64 chord_samples = (Uint64)(CHORD_SEC * SR);
    /* arpeggio steps: one note every ~0.55s, drifting slowly */
    Uint64 arp_step = (Uint64)(0.55f * SR);

    enum { CHUNK = 512 };
    float buf[CHUNK * CHANS];

    const Chord *theme = THEMES[M.theme];
    int len = THEME_LEN[M.theme];

    /* small detune amounts (Hz) — a hair apart for warmth */
    const float DET = 0.6f;

    while (frames > 0) {
        int n = frames < CHUNK ? frames : CHUNK;
        for (int i = 0; i < n; i++) {
            const Chord *ch = &theme[M.chord_index];
            float env = envelope(M.sample_pos, chord_samples);

            /* pad + bass go to both channels (centered) */
            float mid = 0.0f;

            /* Pad triad — each note is a voice plus a detuned twin, very soft. */
            mid += tone(&M.ph_a,     ch->a,        0.055f);
            mid += tone(&M.ph_a_det, ch->a + DET,  0.055f);
            mid += tone(&M.ph_b,     ch->b,        0.045f);
            mid += tone(&M.ph_b_det, ch->b + DET,  0.045f);
            mid += tone(&M.ph_c,     ch->c,        0.040f);
            mid += tone(&M.ph_c_det, ch->c - DET,  0.040f);

            /* Soft bass pulse — low and quiet, the head-nod groove. */
            float bp = bass_pulse(M.sample_pos);
            mid += tone(&M.ph_bass, ch->bass, 0.11f * bp);

            /* Drifting arpeggio — now with a soft per-note envelope so each
             * pluck rings and fades like a music box, and panned slightly to
             * one side for a touch of stereo width. */
            Uint64 into_step = M.sample_pos % arp_step;
            int step = (int)((M.sample_pos / arp_step) % 4);
            float arpf = ch->arp[step];
            float arp_l = 0.0f, arp_r = 0.0f;
            if (arpf > 0) {
                float ae = arp_env(into_step, arp_step);
                float a1 = tone(&M.ph_arp,     arpf,       0.060f) * ae;
                float a2 = tone(&M.ph_arp_det, arpf + DET, 0.060f) * ae;
                /* pan the two arp voices gently apart (alternating per step) */
                if (step % 2 == 0) { arp_l = a1 * 1.0f + a2 * 0.6f;
                                     arp_r = a1 * 0.6f + a2 * 1.0f; }
                else               { arp_l = a1 * 0.6f + a2 * 1.0f;
                                     arp_r = a1 * 1.0f + a2 * 0.6f; }
            }

            /* Overall level: gentle, background. Chord envelope shapes it. */
            float l = (mid + arp_l) * env * 0.5f;
            float r = (mid + arp_r) * env * 0.5f;

            /* One-pole low-pass: softens the high end into a warm haze. */
            const float k = 0.35f;   /* lower = hazier */
            M.lp_l += k * (l - M.lp_l);
            M.lp_r += k * (r - M.lp_r);

            buf[i * 2 + 0] = M.lp_l;
            buf[i * 2 + 1] = M.lp_r;

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
    M.ph_a = M.ph_a_det = M.ph_b = M.ph_b_det = 0.0;
    M.ph_c = M.ph_c_det = M.ph_bass = M.ph_arp = M.ph_arp_det = 0.0;
    M.lp_l = M.lp_r = 0.0f;

    SDL_ResumeAudioStreamDevice(M.stream);
    return true;
}

void music_set_theme(MusicTheme theme) {
    if (theme < 0 || theme >= MUSIC_THEME_COUNT) return;
    if (theme == M.theme) return;
    M.theme = theme;
    M.chord_index = 0;
    M.sample_pos = 0;
}

void music_shutdown(void) {
    if (M.stream) {
        SDL_DestroyAudioStream(M.stream);
        M.stream = NULL;
    }
}