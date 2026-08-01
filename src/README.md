# Katiztic

A cozy, ethereal cat-raising game with a GBA soul. Modern game, retro heart —
soft pastels, calm mood, cats you bond with rather than battle.

This repo is the **meadow vibe slice**: the first playable proof of the feeling.
A pastel meadow you can watch, a cat you can pet, and a time of day you can
drift through dawn, noon, dusk, and night.

## Controls

| Input | Action |
|-------|--------|
| click / tap the cat | pet her (purr + hearts, deepens bond) |
| `F` | feed (restores energy, lifts mood) |
| `G` | groom (lifts mood, deepens bond) |
| `space` or `T` | shift time of day |
| `esc` or `Q` | quit (saves automatically) |

Her stats show in the top-left panel — bond (heart), mood (smile), energy
(leaf), and growth (star). Everything is saved to `katiztic.sav` when you quit,
so she remembers you next time. Stats only ever change when you care for her;
nothing decays while you're away.

## Build

Requires SDL3 and a C11 compiler.

```sh
# Arch
sudo pacman -S sdl3 gcc make

make run
```

That's it — `make` builds `./katiztic`, `make run` builds and launches it,
`make clean` removes artifacts.

## How it works

Everything draws in a **240×160 logical canvas** (GBA proportions). SDL's
`INTEGER_SCALE` presentation blows that up to the window with crisp, square
pixels — the honest retro look, no blur. The window opens at 4× (960×640).

### Layout

```
src/
  palette.h    the pastel color system — one source of truth for the whole look
  render.h/.c  the entire drawing API: fill a rect, fill a pixel, fill with alpha
  scene.h/.c   the meadow — sky gradient, hills, grass, flowers, petals, and the
               four-moment time-of-day table that gives the scene its mood
  cat.h/.c     the cat — a small bundle of state (position, blink, petting glow)
               and behavior (breathe, blink, purr), drawn from primitives
  stats.h/.c   the cat's inner life — bond, mood, energy, growth; the feed/groom/
               pet care actions; and a tiny versioned save file
  ui.h/.c      the cozy status panel — icon-labeled pastel stat bars
  main.c       window, fixed-60fps loop, input, save/load, and the draw order
```

### Design notes

- **No pure black, no pure white, no harsh saturation.** Outlines are dark
  mauve, backgrounds are cream. This one rule does most of the "ethereal" work.
- **Time-of-day tinting is the biggest mood lever.** The same meadow, recolored,
  reads as four different feelings. All four live in one editable table in
  `scene.c` — retune the whole game's mood from there.
- **Idle motion is what makes it cozy.** Swaying grass, drifting petals, the
  cat's breathing bob and occasional blink. None of it is interactive-critical;
  remove it and the scene dies.
- **No assets yet.** Every sprite is drawn from primitives so the project builds
  from source with zero image files. Hand-drawn sprite art is a later pass.

## Roadmap

Rung 1 (the *feeling* — palette, idle motion, time-of-day) and rung 2 (the
**care loop** — stats, feed/groom/pet, and saving) are in. Next rungs:
a proper day cycle tied to a cottage home base, then choosing and swapping a
team of up to 5 cats, then befriending new cats out in the world.

## License

TBD.