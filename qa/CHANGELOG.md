# Star Drag v19

v19 fixes the designer's v18 playtest report (Nov 19, 2026) — two changes. **(1) Death drift tuned down:** the v18 drift (~1.4 s, 1 px/frame, every ship-type enemy) is now a brief dying wobble — **36 frames (~0.6 s)**, **0.5 px/frame** vertical, and only **TWO enemy types drift: the drifter (type 0) and the weaver (type 1)**, the two most ship-like. Every other type gets its per-type pixelated explosion **immediately** at the killing hit, as in v8. **Edge rule:** if a drifting wreck reaches the top or bottom of the screen it explodes there at once instead of finishing the drift. Direction is still from where it dies (above the middle → up, below → down, locked), and drifting ships still blink and trail embers. **(2) Late-wave HP capped:** the v9 tier scaling from wave 7 grew without bound (a base-1 enemy was 4 HP at wave 7 and 10 at wave 13; a base-8 capital 18 at wave 7 and 38 at wave 13) — the designer found wave 8 nearly impossible. Now waves 7+ just add a **flat +2 HP and never grow**: base-1 is 3 HP and base-8 is 10 HP at every wave from 7 on. Waves 1–6 are exactly unchanged, boss HP untouched (boss 1 stays 70). Measured at wave 8 with SINGLE at rate 0: 13 kills/86 s in v18 → **44 kills/86 s in v19 (3.4×)**; LASER at rate 3 held at 44 kills/86 s. Everything else is v18 unchanged: SFX ×0.7, music 127, 16-bit loops, boss-music rule (same area track at 1.25× through the fight, next track fades in 1.5 s after the kill), boss 1 HP 70, reduced weapon-bonus frequency, 1UP pickup, pickup chime, boss-damage tick, player-damage buzz, hit sparks, arcade title screen, clean touchpad UI, rainbow border, boss-3 floor sweep, boss 1 reverse / boss 2 halt / boss 3 scroll-only upward / boss 4 shield+charge, alarm ×3 every boss, labeled pickups, relative drag, GAME OVER hold screen.

v18 fixes the designer's v17 playtest report (Nov 19, 2026) — three changes. **(1) SFX down another clear step** so the music sits on top: shoot 85->60, laser 100->70, alarm 110->80, pickup 100->70, boom 110->80, boss tick 70->50, player-damage buzz 100->70 (×0.7 ≈ −3 dB each). Music untouched (channel 127, loop peak 0.72). **(2) Boss music rule, fixed for all four bosses:** when a boss appears the game now KEEPS playing the current area's music at the 1.25× tempo lift — it used to switch to the NEXT area's track at the intro, at full volume immediately. Only AFTER the boss is defeated does the next area's track start, and it now FADES IN from silence over ~1.5 s (new `music_set_area_fadein`, `MUSIC_FADEIN 90`); bosses whose area doesn't change (2 and 4) just keep the same track at normal tempo after the kill. **(3) Visible death drift is back** (the designer: what happened to the drifting ship?): on a fatal hit a ship-type enemy now keeps its sprite, blinking and trailing embers, and drifts for ~1.4 s (84 frames) before the per-type pixelated explosion fires in `kill_enemy_final` — v8 had hidden the ship at the killing hit and exploded instantly. The drift direction is NOT random: it comes from where the enemy dies — centre y < 96 drifts UP, y >= 96 drifts DOWN (on the line itself, down) — picked once at the fatal hit and locked for the whole drift, so y moves monotonically one way. Bosses keep their multi-explosion death sequence. Everything else is v17 unchanged: 16-bit music, boss 1 HP 70, arcade title screen, clean touchpad UI, rainbow border, boss-3 floor sweep, boss 1 reverse / boss 2 halt / boss 3 scroll-only upward / boss 4 shield+charge, alarm ×3 every boss, post-boss-2 HP scaling, pixelated type-specific explosions, labeled pickups, relative drag, GAME OVER hold screen.

v17 fixes the designer's v16 playtest report (Nov 19, 2026): the 16-bit music is **much clearer, but too quiet in game**. Root cause (measured): the 16-bit path plays at ~0.47x nominal in the emulators' SPU emulation — and too quiet on her phone (NooDS) too, so it is not just a melonDS quirk. v17 raises the music ~+9 dB while keeping 16-bit: channel volume 80 -> 127 and shipped loop peak 0.42 -> 0.72 (`tools/convert_music_v17.py`), back to roughly v12's perceived level; the SFX come down a tad across the board (shoot 100->85, laser 120->100, alarm 127->110, pickup 120->100, boom 127->110, hit retired, boss tick 80->70) to keep mixer headroom — the designer confirmed that balance herself. Also new in v17: a distinct **player-damage sound** (low 280->70 Hz buzz, replaces the old hit sound) and a tiny **hit spark** on multi-HP enemies and bosses. Comparison MP3s (clean source vs in-game) are in `~/workspace/your_files/star-drag-v17-compare/`. **Boss 1 HP stays 70** (from v14; its pattern and bullets unchanged). Everything else is v14 unchanged: arcade title screen, clean touchpad UI, rainbow border, boss-3 floor sweep, boss 1 reverse / boss 2 halt / boss 3 scroll-only upward / boss 4 shield+charge, alarm ×3 every boss, post-boss-2 HP scaling, pixelated type-specific explosions, boss multi-explosion deaths, labeled pickups, relative drag, GAME OVER hold screen.

v12 fixes the designer's v11 playtest report (Nov 19, 2026): **no music at all, only the shooting SFX.** Root cause: v11 moved the 7 tracks into the ROM filesystem (NitroFS) with a chunked streaming loader. The music plays correctly in melonDS (verified by audio capture), but the loader fails silently on NooDS — the designer's iPhone emulator — so the music never starts there. v12 reverts to the proven v10 architecture: **the same 7 unique tracks are EMBEDDED in the ARM9 binary** (`audio/music_aN.bin` via bin2o, converted by `tools/convert_music_v12.py` to 18 s loops from the start of each source, area 1 at 22050 Hz, areas 2–7 at 16000 Hz) and played with `soundPlaySample` at full volume (80) — no file I/O, no streaming, no volume-0 start. Area changes crossfade (new track at full volume immediately, old track fades out over 0.5 s), and **boss fights still lift the music tempo 25%**. Everything else is v11 unchanged: boss 1 reverse / boss 2 halt / boss 3 scroll-only upward with open ceiling / boss 4 shield+charge, alarm x3 every boss, post-boss-2 HP scaling, pixelated type-specific explosions, boss multi-explosion deaths, labeled pickups, glowing fading weapon border, relative drag, GAME OVER hold screen.

v11 answers the designer's v10 playtest notes (both, Nov 19, 2026): **each of the 7 areas now has its own REAL unique music track** — area 1 is the existing 85-second Speechma track (looped as a 36 s slice), areas 2–7 are six new Speechma tracks, one per area. Area changes crossfade (1 s fade-out/fade-in, no hard cut), and **boss fights lift the music tempo 25%** (ramping up at the intro, back to normal after the kill, fading into the next area's track). The tracks live in the ROM filesystem (NitroFS, `nitro/music/areaN.raw`) because 7 embedded tracks overflow the ARM9 ELF's 3.5 MB limit — they load into a double RAM buffer in 64 KB chunks per frame, so there is no gameplay hitch. And **the selected-weapon border now glows instead of blinking**: a smooth sine fade over 2 s (brightness breathing between ~40% and 100%, never switching off abruptly), same colours (red SINGLE, yellow SPREAD, blue LASER, green MISSILE). Everything else is v10 unchanged: boss 1 reverse / boss 2 halt / boss 3 scroll-only upward with open ceiling / boss 4 shield+charge, alarm x3 every boss, post-boss-2 HP scaling, pixelated type-specific explosions, boss multi-explosion deaths, labeled pickups, relative drag, GAME OVER hold screen.

v9 answers the designer's v8 playtest notes (all three, Nov 19, 2026): **boss 3 is now a scroll-only vertical flight, like boss 1's reverse** — the ship stays at its unchanged position facing right and firing right, the boss parks on the right facing left and shoots left at the player, and only the world scrolls: the flight slows to a stop, then the starfield/background scrolls upward (faster than normal, like boss 1's max-speed reverse), with the **ceiling OPEN** in the boss section (floor/ground kept) so you can fly up — then the ceiling comes back and normal horizontal flight resumes after he dies. All v7 upward-mode code is gone (no ship teleport, no facing-up, no vertical shots, no shaft). Two real bugs were found and fixed while building it: the BG scroll Y sign was backwards (increasing it moves content up, not down — v7's stars had actually been scrolling the wrong way), and the terrain's vertical offset was never reset after the fight, stranding the floor band mid-screen. **Enemies after boss 2 have much more HP** (tiered by wave from wave 7: a 1-HP type is 4 HP at wave 7, 10 HP at wave 13; an 8-HP capital is 18 at wave 7, 38 at wave 13) so they survive several hits and stay visible instead of dying instantly; waves 1–6 are exactly unchanged. **The alarm sounds for every boss fight** — verified in code (one shared intro path plays it ×3 plus "!! WARNING !!" and the mode's named cue for all four bosses) and in QA (REVERSE! / HALT! / UPWARD! / SHIELD! all seen on screen with the WARNING; the sound itself can't be heard headless). Boss 1 (reverse) and boss 2 (halt) are untouched and perfect; boss 4 unchanged.

A touch-and-drag **horizontal** shoot'em up for the Nintendo DS
(R-Type / Life Force style — fly left-to-right, enemies come from the
right). v1 was vertical; the designer asked for horizontal, smaller sprites,
more enemy variety, a weapon panel, and a starfield that scrolls the
right way. Built as the follow-up to Folded Letter (rejected in
playtest for poor graphics, no sound, and a broken drag mechanic) —
this one is all real sprites, real sound, and a drag that works.
v8 answers the designer's v7 playtest notes (all four):
(R-Type / Life Force style — fly left-to-right, enemies come from the
right). v1 was vertical; the designer asked for horizontal, smaller sprites,
more enemy variety, a weapon panel, and a starfield that scrolls the
right way. Built as the follow-up to Folded Letter (rejected in
playtest for poor graphics, no sound, and a broken drag mechanic) —
this one is all real sprites, real sound, and a drag that works.
v3 added single-shot start with weapon capsules, a laser beam, missile
sprites, 14 enemies, 7 terrain areas and relative-drag-only movement.
v4 adds mini-bosses, death-drift explosions, sprite-based terrain
obstacles, 32 enemies, slower fire with fire-rate power-ups, and a
clean bottom screen.
v5 answers the designer's playtest notes: a much easier first boss and first
level (difficulty ramps by boss/area), animated enemies (spinning,
flapping wings, pulsing, wobbling), coloured stars in the starfield,
and a reverse-flight first-boss intro — alarm x3, "!! WARNING !!",
then the ship flies backwards with the starfield reversed until the
boss dies.
v6 corrects that intro exactly as the designer described it, plus makes the
boss fight fair: **no other enemies appear while a boss is present**
(regular spawns stop, boss minions are gone, and any enemies/bullets
on screen are cleared when the fight starts — the player fights the
boss ALONE), and the reverse effect is now a **scroll transition
only** — the ship stays exactly where it is on the left, facing
right, firing right, boss parks on the right as normal; only the
starfield and the ceiling/floor/terrain scroll ease down to a stop,
then accelerate in reverse up to **max speed, faster than normal
flight (160% scroll speed)** for the boss fight, then ease back
through a stop to normal speed/direction when the boss dies
("REVERSE!" / "NORMAL FLIGHT" cues on the bottom screen).
v8 answers the designer's v7 playtest notes (all four): **proper pixelated
explosions, different per enemy type** (orange fireball + sparks for
ships, green/purple goo for creatures, cyan shards for crystals, grey
sparks + smoke for robots/metal, splat for jellyfish/blobs, bigger
multi-stage blasts for tough enemies — no more dying ship swapping to
another ship sprite); **boss deaths are a chain of consecutive
explosions** around the boss over ~1.5 s with a shake and a flash,
then the final big blast, for every boss including boss 1 (the fight
itself is unchanged); **level-1 stalagmite bug fixed** — root cause was
the sprite VRAM mapping: `oamInit` used 1D_32 (32 KB) but the game
allocates ~43 KB of sprite tiles, so everything past 32 KB (terrain
obstacles, pickups, booms, score digits) aliased to other sprites'
tiles and the stalagmites rendered as ship tiles — now 1D_128; **weapon
pickups and panel buttons are identifiable at a glance** — every pickup
carries a big letter on a coloured capsule (green S = SPREAD, cyan L =
LASER, orange M = MISSILES, red-brown bolt + chevrons = fire-rate) and
every panel button carries the matching letter and colour, with the
selected weapon starred and uncollected weapons greyed out.
v7 (the designer: "the first boss is perfect!"): **different bosses for the
other levels** — boss 2 (wave 6) is the HALT boss (the world slows to
a full stop for the battle, then speeds back up), boss 3 (wave 9) is
the UPWARD boss (an opening appears in the ceiling and you fly upward
through it, ship facing up, shots firing up, boss above — v9 redid
this as a scroll-only vertical flight, see above), boss 4
(wave 12) is the SHIELD + CHARGE boss (periodic invulnerability
shield, charge dashes across the screen); the cycle repeats after
that. Boss 1 is untouched. Plus a **proper death screen**: the ship's
destruction plays a ~1.5 s explosion, then a GAME OVER screen (final
score, wave reached, best score) that STAYS until a fresh touch or
START press — it can no longer auto-restart from a finger still held
down at the moment of death. And **8 more enemy types with totally
different sprites from the web** (40 total): worms, a green alien
creature, a purple jellyfish, a creeper eye, a green robot and a
tentacled orb — see Graphics for sources. The white-screen cause was re-investigated (see
`qa/MELONDS-INVESTIGATION.md` v5 appendix) — NOT claimed fixed:
melonDS JIT-on here renders exactly the white screen the designer describes,
JIT-off renders the game. See `qa/VERIFICATION.md` for the v4 playtest
record and `qa/MELONDS-INVESTIGATION.md` for the melonDS white-screen
story (not claimed fixed — see there).

## The game

- **Top screen:** the playfield — 3-layer parallax starfield scrolling
  leftward, your ship, alien waves, bullets, explosions, score / lives /
  wave HUD (runtime-drawn digit sprites) and the mini-boss HP bar.
- **Bottom screen:** touch control only. It shows **just the area name
  and the selected weapon** (plus the weapon panel and a star marker
  above the selected button) — nothing else.
  **Drag the stylus and the ship follows by the drag delta. A tap
  never moves the ship — only a drag does.** Tap one of the four
  weapon buttons to switch guns (only weapons you have collected);
  tapping a button never moves the ship either. D-pad moves the ship
  as a fallback. Auto-fire is always on.
- **Weapons:** you start with SINGLE shot only. Weapon capsules
  (star / bolt / ring, dropped by enemies ~22% and floating by every
  ~18 s) grant SPREAD (3-way fan, 1 dmg), LASER (long piercing beam,
  3 dmg, unique zap SFX) or MISSILES (rocket sprites with yellow spark
  trails, 2 dmg). Getting hit drops you back to SINGLE. A separate
  red-bolt capsule raises fire rate in steps (+1 / +2 / +3, shown on
  the weapon line); getting hit resets it too.
- **Fire rate (v4):** base intervals are deliberate, not a bullet hose —
  SINGLE 22 / SPREAD 26 / LASER 40 / MISSILE 34 frames; each fire-rate
  level cuts the interval 12% (level 3 → 64% of base).
- **Enemies (40 types, area-unique sets, HP 1–8, damage flash):**
  drifter, weaver, diver, shooter, asteroid, mine, turret, bug, alien,
  charger, spinner, orbiter, crystal, capital, plus v4's dart, rogue,
  ufo, meteor2, drone, sentry, wasp, beetle, bat, fish, jelly, shard,
  blob, crawler, eel, prism, spore, hunter — cave bats/bugs, water
  fish/jellyfish/eels, crystal shards/prisms, goo blobs/spores, base
  drones/sentries/hunters, each with its own movement/attack pattern
  (10 styles: drift, sine, dive, hover, tumble, zigzag, slow, dash,
  orbit, snake). v7 adds 8 more with totally different web sprites
  (see Graphics): green/red/blue **worms**, a green **alien creature**
  (2-frame walk), a purple **jellyfish** (2-frame pulse), a **creeper
  eye**, a green **robot**, and a **tentacled orb** — cave/planet
  worms, water jellyfish, crystal eyes, base robots, goo tentacles.
- **Death drift (v4):** a fatally hit enemy keeps flying forward,
  tumbling and drifting up or down while trailing fire/sparks for 40
  frames, then bursts into a big explosion (3 fireballs + 8 particles).
- **Mini-bosses (v4, v6, v7):** every 3rd wave (3, 6, 9, …) a 64×64 boss parks
  at the right side, fires aimed shots / spread fans, HP 40 + wave×5
  (first boss only 25), with a visible HP bar top-centre. Waves and
  areas do NOT advance until it is destroyed (+1500, drops weapon and
  fire-rate capsules). **v6: no other enemies during a boss fight** —
  regular spawns stop, minions are gone, the field is cleared when the
  fight starts; the player fights the boss alone. **First-boss intro
  (v6):** alarm x3 + "!! WARNING !!", then the world scroll eases
  through a stop into reverse at up to 160% speed ("REVERSE!" cue)
  while the ship stays put on the left; on boss death the scroll eases
  back to normal ("NORMAL FLIGHT").
  **v7: every boss has its own gimmick (boss 1 unchanged):** boss 2
  (wave 6) is the HALT boss — the scroll eases to a full stop for the
  battle ("HALT!") and speeds back up after he dies; boss 3 (wave 9)
  is the UPWARD boss — an opening appears in the ceiling, the
  starfield scrolls vertically and you fly upward ("UPWARD!") — v7 had
  the ship facing up and firing up with the boss parked above; v9
  redid this as a scroll-only flight (ship and boss keep their normal
  positions/orientations, only the world scrolls up), see above; boss 4 (wave 12) is the SHIELD +
  CHARGE boss ("SHIELD!") — a periodic shield that blocks all damage
  while up, and charge dashes across the screen; the cycle repeats
  (boss 5 = reverse again, etc.). Each boss has its own 64×64 sprite.
- **Death / game over (v7):** losing your last life plays the ship's
  destruction — a big explosion at the ship for ~1.5 s — then a GAME
  OVER screen (final score, wave reached, best score for the session)
  that STAYS on screen until you release and then touch or press
  START to fly again. It can no longer skip straight past the screen
  from a finger still held down at the moment of death.
- **Terrain (v4):** 7 areas changing every 2 waves — OPEN SPACE, SPACE
  BASE, PLANET, CAVE, CRYSTAL, WATER, GOO — each with its own ceiling/
  floor palette and **sprite obstacles** (pipes/columns/wires, trees/
  roots, stalactites/stalagmites, crystal spikes/claws, plants/coral/
  tubes, slime barbs/blobs) hanging down or sticking up at 32 or 64 px
  heights, plus a narrow wavy tunnel (8-tile gap, sine centre) for
  ~8 s every ~1400–2000 frames in terrain areas. Terrain collision
  costs a life with the invulnerability blink; gaps are always passable.
- Waves escalate with kills (wave = 1 + kills/12): faster spawns, faster
  enemies, more enemy types.
- Score, 3 lives, 150-frame invulnerability blink after a hit, game-over
  screen with final score, wave reached and best score (v7: death
  animation first, screen waits for a fresh touch/START), title screen
  with attract-mode ships flying left.
- **Sound:** 85-second AI music track (Speechma, synthwave chiptune)
  looped, plus synthesized shoot / explosion / hit SFX, played through
  libnds' sound server.

## Controls

| Input | Action |
| --- | --- |
| Stylus drag (bottom screen, above the panel) | Fly the ship (1:1) |
| Stylus tap on a weapon button | Switch weapon (never moves the ship) |
| Stylus tap elsewhere | Nothing (never teleports the ship) |
| D-pad | Fly the ship (fallback) |
| Touch / START on title | Start |
| Touch / START on game over | Fly again |

## Graphics

All sprites are real converted art, no ASCII:

- **Kenney Space Shooter Redux** — CC0 1.0, by Kenney (https://kenney.nl).
  Player ship, the four enemy ships, both lasers, the meteor, the two
  UFOs, the UI weapon buttons, the star reticle, and the explosion flame
  frames all come from this pack. Credit appreciated, not required —
  credited here and in the in-game title credits.
  Pack source: GitHub mirror `krixi/SpaceShooterRedux` (kenney.nl direct
  download is bot-blocked from this environment). Ships are rotated 90°
  in `tools/convert_sprites.py` so they face along the flight axis.
  v4 uses ~60 images from the pack (v3 used only ~14); attempts to
  download further packs for v4 (kenney.nl direct 403, KenneyNL org has
  no asset repos, opengameart search yielded no usable links) failed,
  so v4 draws its extra enemies and all 12 terrain obstacles from this
  same CC0 pack plus procedural drawing in `convert_sprites.py`.
- **v7 web-sourced enemies (totally different sprites, not ships):**
  converted by `tools/convert_v7.py` from `assets/web-sprites/`:
  - Green alien creature — "alienSpriteSheet" by rrodi411, **CC0**,
    lpc.opengameart.org (2 walk frames).
  - Purple jellyfish — "Jellyfish" by StendhalGame, **CC-BY-SA 4.0**,
    opengameart.org/content/jellyfish-0 (2 pulse frames).
  - Green/red/blue worms — "32x32 Worms Pack", **CC-BY-SA 4.0**,
    opengameart.org/content/32x32-worms-pack.
  - Green robot — "Green Robot Enemy", **CC0**,
    opengameart.org/content/green-robot-enemy (background chroma-keyed
    out before slicing; boss 4 is another robot from the same sheet).
  - Creeper eye — "creeperEyeBlue" / "creeperEyesGreen" from
    "creepy-thing", **CC0**, opengameart.org/content/creepy-thing
    (boss 3 is the big green eye).
  - Tentacled orb — drawn procedurally in `tools/convert_v7.py`
    (same style as the v4 terrain obstacles).
  All credited here and in the in-game title credits. Sources that
  failed for v7: kenney.nl direct (403), KenneyNL GitHub org (no asset
  repos, 404), itch.io asset packs (paid / no direct download).
- The starfield background is 4 procedural tiles generated in code, on
  3 parallax BG layers (near / mid / far, different palettes and scroll
  speeds, all scrolling leftward).
- The explosion is the pack's blue exhaust flame re-tinted by luminance
  into a white-yellow → orange → deep-red fireball ramp, composited at
  0/90/180/270 degrees into a radial burst (the raw flame is a thin
  vertical streak on its own).

## Sound

- **Music (v16, reverted from v11 NitroFS in v12):** one unique track per area, EMBEDDED in the ARM9 binary. Area 1: `assets/music-v2.mp3` — 85 s instrumental, generated on Speechma with the R-Type / Life Force synthwave chiptune prompt, no sign-in, commercial use stated. Areas 2–4: `assets/music-area2-spacebase-v2.mp3`, `music-area3-planet-v2.mp3`, `music-area4-cave-v2.mp3` — fresh clean Speechma regenerations (Nov 19, 2026; the v11 originals' loudest sections were overdriven at the source). Areas 5–7: `assets/music-area5.mp3` … `music-area7.mp3` — Speechma tracks in the same style. Converted by `tools/convert_music_v17.py` to raw signed 16-bit PCM mono (v12 recipe: plain resample, no EQ, no RMS normalization; area 1 window 0 exactly as v12; areas 2–7 loudest CLEAN window, peak <= 1.00, squash <= 0.05%; peak-safe gain only, shipped peaks ~0.72, all at 22050 Hz, 0.1 s fade-in, 0.5 s fade-out at the loop seam; 7.5 s loops because 16-bit doubles the size and the ARM9 load cap is 0x27c000) into `audio/music_aN.bin` (bin2o), played with `SoundFormat_16Bit`. v15 used the same recipe in unsigned 8-bit (`tools/convert_music_v15.py`, 14 s loops) — its music-only capture was clean on peaks but the 8-bit quantization SNR was only 28.5 dB, which is the distortion the designer heard in the v15 in-game clips; v16 measures 75–82 dB. v14 used `tools/convert_music_v14.py` (best 14 s windows, gentle +2 dB highshelf, RMS −18 dBFS, peaks capped at 0.70) — its music-only capture was clean but the gameplay mix clipped constantly (11,278 pinned samples over 33 s in area 1), which is what the designer heard as distorted even in area 1; v15 verifies on gameplay captures (see `qa/VERIFICATION.md` v15 section for the full measured diagnosis). v13 used `tools/convert_music_v13.py` (aggressive +5 dB @ 1200 Hz / +3 dB @ 3 kHz presence lift, RMS −17 dBFS, peaks 0.92, music channel volume 110) — the music sounded garbled/distorted because the hot mix summed the DS mixer over full scale. v11 put the tracks in NitroFS (`nitro/music/areaN.raw`, 36 s loops) with a chunked streaming loader — the music played in melonDS but was SILENT on NooDS (the designer's iPhone emulator), because the loader fails silently there; v12 reverts to the v10 embedded architecture so the music works on every emulator. `main.c` plays the embedded track at volume 127. On a NON-boss area change the new track starts at 127 immediately and the old track fades out over 0.5 s. **Boss fights (v18 rule): the current area's track keeps playing and lifts to 1.25× tempo at the intro; only after the boss is defeated does the next area's track start, fading in from silence over 1.5 s (`music_set_area_fadein`, `MUSIC_FADEIN 90`) while the tempo ramps back to 1.00×** — before v18 the track switched to the next area at the intro, at full volume immediately (see `music_set_area` / `music_set_area_fadein` / `music_update` / `music_boss_start` / `music_boss_end`). 7.5 s loops (not 14 s) because 16-bit doubles the size and calico's `ds9.ld` caps the ARM9 load image at 0x27c000 (2,621,440 bytes) — 14 s 16-bit loops would be 4,321,800 bytes. (The earlier sites musiccreator.ai, onlymusic.ai and freebeat.ai all failed on login / human-check — do not retry them.)
- **SFX:** `tools/make_audio.py` synthesizes raw unsigned 8-bit PCM at
  11025 Hz: `shoot.raw`, `boom.raw`, `hit.raw`. The v1 synthesized
  music loop is preserved as `qa/music_synth_v1_raw.bin`; re-running
  `make_audio.py` writes it as `music_synth_raw.bin` and never touches
  the AI track.
- Headless QA can only verify the soundbank loads and plays without
  crashing — the mix is for the player's ears (the designer's iPhone is the gate).

## Build

Requires the devkitARM toolchain extracted at `~/workspace/devkitpro-image/`
(devkitARM gcc 16.1.0, libnds 2.0.2, calico 1.2.0, ndstool 2.3.1):

```sh
./build.sh        # builds, fixes the header, copies to ~/workspace/your_files/star-drag-v18.nds
```

v4 notes: the ARM9 is compiled in Thumb mode (`-mthumb` in
`arm9/Makefile`) — ARM-mode builds segfault melonDS 1.1's JIT
recompiler; see `qa/MELONDS-INVESTIGATION.md`. `build.sh` also pads
the ROM to its declared device capacity (v4 is 4 MB: the 85 s music
track makes the ROM too big for the old 512 KB capacity) and stamps
game code `####` / maker `0x0000` — **never stamp a real game code**:
v2's `SDRG`/`01` made melonDS classify the ROM as retail and demand
native BIOS files (see `qa/MELONDS-INVESTIGATION.md`). Init order is
console-first, videoSetMode on the main screen only (m10 order from
the v4 JIT bisect).

`tools/convert_sprites.py` converts the Kenney PNGs to 4bpp DS sprites
(`data/*.bin` + `*_pal.bin`; bin2o only handles `.bin` files, so palettes
and audio are named `*_pal.bin` / `*_raw.bin`).

## The pure-DS header (read this before shipping another DS game)

The calico ARM7 carries `.twl` (DSi) sections, so ndstool writes a
**DSi-enhanced** ROM header (unit code `0x02`, DSi magic at `0x180`).
That ROM boots in NooDS (DS-only, ignores the DSi sections) but shows a
**blank white screen in melonDS (DSi mode / no DSi BIOS) and DeSmuME
0.9.11** — which is exactly what killed Folded Letter v1 in playtest.
`build.sh` runs `tools/fix_ds_header.py` on every build: it zeroes the
unit code, the DSi magic at `0x180–0x187`, and the DSi header fields
`0x1B0–0x1FF`, then recomputes the header CRC16 (CRC-16/MODBUS, poly
`0xA001`, init `0xFFFF`, over `0x000–0x15D`). It also stamps game
code `####` and maker `0x0000` (see Build above — never a real game
code). Verify with
`ndstool -i` (Unit code `0x00`, no DSi lines, Header CRC OK) and the
DeSmuME log (`ROM serial: NTR-####`, no "DSi Enhanced" line).
Full diagnosis: `~/workspace/folded-letter-ds/qa/DS-BLANK-WHITE-FIX.md`.

## QA

See `qa/VERIFICATION.md`.
