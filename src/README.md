# Katiztic

A cozy, ethereal cat-raising game with a GBA soul. Modern game, retro heart —
soft pastels, calm mood, cats you bond with rather than battle.

This repo is the **meadow vibe slice**: the first playable proof of the feeling.
A pastel meadow you can watch, a cat you can pet, and a time of day you can
drift through dawn, noon, dusk, and night.

## Controls

You start at home in the cottage. Tap the on-screen buttons (top-right) to go
outside to the meadow and back, and tap the bed (or the moon button) to sleep.

| Input | Action |
|-------|--------|
| tap the sun / house button | go outside / come home |
| tap the bed (or moon button) | sleep — wake to a fresh morning, saves the game |
| click / tap the cat | pet her (purr + hearts, deepens bond) |
| `F` | feed (restores energy, lifts mood) |
| `G` | groom (lifts mood, deepens bond) |
| `space` or `T` | shift time of day (meadow) |
| `esc` or `Q` | quit (saves automatically) |

Sleeping always wakes you to dawn — you're never locked out by the time of day.
Her stats show in the top-left panel: bond (heart), mood (smile), energy
(leaf), growth (star). Everything saves to `katiztic.sav`; stats only ever
change when you care for her, never decaying while you're away.

Every action is driven by a tap at a point, so the whole game already works
by touch — the same taps will drive the planned iOS version.

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
  cottage.h/.c the home interior — warm room, window, bed you tap to sleep;
               drawn in the same pastel language as the meadow
  ui.h/.c      the cozy status panel plus touch-first buttons (tap to travel
               and sleep) — same hit-test for mouse now and finger later
  main.c       window, fixed-60fps loop, location state, tap routing, save/load
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

Done: the *feeling* (palette, idle motion, time of day), the **care loop**
(stats, feed/groom/pet, saving), and the **cottage & day cycle** (a home base,
travel between rooms, and sleeping to a fresh morning — all touch-first).
Next rungs: choosing and swapping a team of up to 5 cats, then befriending new
cats out in the world. An iOS port (SDL3 supports it) comes once there's more
game to ship — the touch-first input means that port is a swap, not a rewrite.

## License

TBD.