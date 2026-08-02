# Katiztic

A cozy, ethereal cat-raising game with a GBA soul. Modern game, retro heart —
soft pastels, calm mood, cats you bond with rather than battle.

This repo is the **meadow vibe slice**: the first playable proof of the feeling.
A pastel meadow you can watch, a cat you can pet, and a time of day you can
drift through dawn, noon, dusk, and night.

## Controls

You start at home in the cottage with a little family of cats. The strip along
the bottom is your team — tap a cat to make her active (she appears in the
scene and receives your care), or tap the **+** to adopt a new one, up to five.
Each cat is a different type with its own pastel coat.

| Input | Action |
|-------|--------|
| tap a cat in the bottom strip | make that cat active |
| tap any cat lounging in the cottage | make that cat active and pet her |
| tap the cat's name on the card | rename her (type, Enter to confirm, Esc to cancel) |
| tap the **+** slot | adopt a new cat (up to 5) |
| tap the sun / house button | go outside / come home |
| tap the bed (or moon button) | sleep — fresh morning, rests the active cat, saves |
| click / tap the active cat | pet her (deepens bond) |
| `F` | feed the active cat |
| `G` | groom the active cat |
| `space` or `T` | shift time of day (meadow) |
| `esc` or `Q` | quit (saves automatically) |

At home, your whole family lounges in the cottage together — tap any of them to
switch to her. Step outside and it's a one-cat outing with whoever's active.
Renaming uses the keyboard for now (it'll use the on-screen keyboard on iOS).

## Befriending cats on walks

Out in the meadow, a wild cat sometimes comes to visit — a mix of regulars
you've met and new faces. Tap the fish button to offer her a treat; each treat
builds her trust, and once trust fills she becomes a lasting friend. Open the
friends list (the heart button, top-right) to see everyone you've met and how
much they trust you. Befriended cats are marked with a heart.

These friends are *not* your cats — they live out in the world. Your own family
(up to 5, adopted with the **+**) is separate. Befriending is the warm social
reward of walking, not a way to collect cats.

## Cottage décor

At home, tap the chair button (top-right) to open the décor tray. You unlock
cozy items by playing — a plant and cushion to start, a lamp once a cat's bond
is high, a picture once you've made a friend, a rug once you have three cats, a
cat tower at a very high bond. Two special items are **earned rewards** for
being social: **yarn** (after befriending a couple of cats) and a saucer of
**milk** (once your cats have leveled up through socializing and care). Place
them like any décor — and your cats notice them: a cat near the yarn bats at
it, and one near the milk stops to lap it up. Drag an item from the tray into
the room to place it anywhere you like; drag it back to the tray to put it away.
Your arrangement is saved. Items fall to rest on the floor (or stack on each
other) rather than floating. While the tray is open, each item shows its name.

## Shiny cats

Every so often — about 1 in 100 — an adopted cat is **shiny**: a rare golden
coat wreathed in twinkling sparkles. They're purely a lucky delight, a special
cat to treasure. When one appears, a banner celebrates the moment, and her
shininess is saved so she stays golden forever.

## Day & night

The world follows your real clock. Take a walk in the morning and the meadow
glows with dawn; midday is bright, evening turns golden at dusk, and after dark
it's night — the cottage dims in the evening too. It all shifts automatically
with the actual time of day where you are, so the game feels connected to your
own day.

## Levels & happiness

Every cat has a **level**, starting at 1 when you adopt her, with no cap. She
earns experience from positive moments — petting, grooming, feeding, and
socializing (when cats play together at home or the café). Each level needs a
bit more XP than the last, so leveling comes quickly at first and gently slows,
but never stops. Her level and an XP bar show on her card, and a banner
celebrates each level up. Her **happiness** (the smile stat) rises when you feed
her and when you take her out to the meadow or café.

## Places to go

Tap the travel button (top-right) to open the place picker and choose where to
go: your **cottage** (home, where you care for cats and decorate), the
**meadow** (walks, where you meet and befriend wild cats), or the **café** — a
cozy social lounge where your cats hang out and play together. Each place has
its own look and its own gentle music: warm and settled at home, airy in the
meadow, and soft and jazzy at the café.

## A living home

At home your cats aren't just sitting around — each one has a little mind of
her own. They wander the room, groom themselves, curl up for naps (with tiny
drifting z's), and when two of them drift close they'll pair up and play,
bouncing at each other. Every cat roams independently, so the cottage feels
like a room full of cats each doing her own thing. It's all just for the cozy
feel — it doesn't affect stats.

Every so often a little **mood bubble** drifts up over a cat showing how she
feels — a heart when she's content, a music note while playing, a sparkle when
grooming, sleepy z's while napping, or a fish when she's a bit hungry. They
appear everywhere, a gentle glimpse of each cat's inner life.

## Music

Each place has its own gentle, synthesized theme — a warm progression at home
in the cottage, an airier one out in the meadow — so the music shifts with
where you are. It's generated in code from soft sine tones (no audio files),
quiet and calm by design, matching the game's cozy GBA feel.

The five types — Sunny, Dreamy, Playful, Gentle, Clever — each have a signature
pastel color, so a team *looks* like a chosen aesthetic. Each cat keeps her own
bond, mood, energy, and growth. The active cat's card (top-left) now shows her
name and type in readable pixel text. The whole family saves to `katiztic.sav`.

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
  cattype.h/.c the five cat types (Sunny, Dreamy, Playful, Gentle, Clever) and
               the signature pastel color set that makes each one look distinct
  text.h/.c    a tiny 4x6 pixel font (built from source, no font file) — draws
               names, labels, and any string in any pastel color
  friends.h/.c the cats you meet on walks — trust levels, befriending, and their
               own save file (separate from your roster)
  encounter.h/.c the wild cat visiting the meadow — who shows up, where she sits,
               and how she edges closer as she trusts you
  decor.h/.c   collectible cottage décor — the catalog, milestone unlocks, and
               drag-to-place with its own save file
  icon.h/.c    the window/dock icon, embedded as raw pixels (no image file)
  music.h/.c   procedural cozy music — a synthesized theme per location, soft
               sine tones generated in code (no audio files)
  behavior.h/.c the cats' little brains at home — picking activities (wander,
               groom, sleep, play together), roaming, and pairing up to play
  mood.h/.c    emoji thought bubbles — picks an emoji from a cat's activity and
               stats, and drifts it up over her head
  roster.h/.c  your family of up to 5 cats — each with a name, type, stats, and
               animation state; the active selection, adopting, and save/load
  cat.h/.c     one cat's animation state (blink, pet glow) and how it's drawn,
               in whatever type colors it's given
  cottage.h/.c the home interior — warm room, window, bed you tap to sleep
  cafe.h/.c    the cat café — a cozy social lounge (counter, tables, cushions)
               where cats hang out; its own place with its own music
  ui.h/.c      status panel, touch-first buttons, and the roster strip along
               the bottom (tap a portrait to select, + to adopt)
  main.c       window, fixed-60fps loop, location + roster state, tap routing
```

## Roadmap

Done: the *feeling*, the **care loop**, the **cottage & day cycle**, a **team of
up to 5 cats**, a **pixel font**, **befriending cats on walks**, and **cottage
décor** (unlock cozy items and arrange them by dragging). Next: more zones and
gentle quests, and the soft main story. An iOS port comes once the game feels
ready to ship; the touch-first input throughout means that port stays a swap.

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

## License

TBD.