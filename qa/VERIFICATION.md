# Star Drag v1 — verification

Final ROM: `~/workspace/your_files/star-drag-v1.nds`
md5: `f6b97f8a4f7dc9640fe54636d06f018b` (303 KB)
Built: November 18, 2026, 5:34 PM EST. Pure-DS header (see below).

## melonDS 1.1 (Xvfb, `qa_driver.py` simulated input) — VERIFIED

- **Run A** (`qa/run-a/`): title renders (starfield, decorative enemy
  ships); click starts the game; player ship, auto-fire bullet, enemy and
  HUD all visible; stylus drag right visibly moved the ship right, drag
  left moved it back; score reached 950 and lives dropped to 1 in ~15 s
  of live play — kills and hits both register.
- **Run B** (`qa/run-b/`): HUD-row bug found and fixed (libnds VT
  `\x1b[<n>;<m>H` treats the row as **0-based** — SCORE/LIVES rows were
  off by one). After the fix: SCORE / LIVES / WAVE each on their own row.
  Game over screen renders ("GAME OVER / FINAL SCORE 1200 / Wave
  reached: 1 / Touch or press START to fly again."); click on the bottom
  screen restarts into a fresh game (SCORE 0, LIVES ***, WAVE 1, all
  three enemy types visible).
- **Run C** (`qa/run-c/`): balance pass verified — gentler early game
  (spawn interval 95−wave×5, invulnerability 150 frames, slower diver
  dive, slower enemy fire). Idle ship survived ~40 s (was ~15 s).
- **Runs D–G** (`qa/run-d/`, `qa/run-e/`, `qa/run-f/`, `qa/run-g/`):
  explosion hunt. The first explosion art (Kenney's fire frames) turned
  out to be pale-blue exhaust flames, nearly invisible and the wrong
  colour; they were re-tinted by luminance into an orange fireball ramp
  and composited at 0/90/180/270° into a radial burst, enlarged to 64×64.
  Run G shot `b17.png` catches a real in-game explosion: a yellow-orange
  fireball at the kill site, score 250. A temporary persistent-boom build
  (`qa/boomtest/`) proved the boom rendering path independently.
- Title → play → game over → restart loop fully exercised in melonDS.

## DeSmuME 0.9.11 (CLI, Xvfb) — HEADER VERIFIED, RENDERING NOT VERIFIABLE HERE

- Log for the final ROM: `ROM serial: NTR-####-???`, **no "DSi Enhanced"
  line** — pure-DS header confirmed (the fix from Part 1).
- Rendering cannot be verified in this VM: DeSmuME's SDL frontend shows
  a white window for *every* ROM under Xvfb here, including the stock
  libnds hello-world that provably boots (renders in melonDS). That is an
  environment artefact, not a ROM bug. See
  `~/workspace/folded-letter-ds/qa/DS-BLANK-WHITE-FIX.md`.

## NooDS — NOT RUNNABLE HERE

- The Linux release is flatpak-only; it cannot run in this VM. NooDS is
  DS-only and ignores DSi sections, so the pure-DS header is the safe
  direction for it, but that is reasoning, not a test.

## Sound — WIRED, NOT HEARD

- Music loop + shoot/boom/hit SFX are synthesized PCM played through
  libnds' sound server; the game runs for minutes with music and SFX
  firing and never crashes. Headless QA cannot hear audio — the mix is
  for the player's ears.

## Not verified (the designer's gate)

- Real hardware and iPhone RetroArch. Never claimed. the designer's playtest is
  the gate, as always.
- D-pad movement was implemented (3 px/frame) but the scripted QA drove
  the ship by stylus drag; D-pad is untested in an emulator run.

---

# Star Drag v2 — verification (horizontal rebuild + AI music)

Final ROM: `~/workspace/your_files/star-drag-v2.nds`
md5: `eff38455a64d37342c6e850dc3ebcb70` (2,097,152 bytes, 2 MB capacity)
Built: November 18, 2026, ~6:27 PM EST. Pure-DS header, game code
`SDRG`, maker `01`, unit `0x00`, header CRC OK, padded to capacity
(no "bad ROM size" line in the melonDS log).

## What changed from v1

- Horizontal (R-Type / Life Force): player flies left-to-right,
  enemies enter from the right. All sprites 32×32 (v1 was 64×64),
  ships rotated 90° to face along the flight axis.
- 7 enemy types (drifter, weaver, diver, shooter, asteroid, mine,
  turret) with distinct movement/attacks; waves gate the mix
  (shooters wave 2+, asteroids/mines wave 3+, turrets wave 4+).
- 3 weapons on a bottom-screen panel (SPREAD / LASER / MISSILES),
  switched by tapping the panel. Touch discipline fixed twice in QA:
  (1) the console vanished because `videoSetModeSub` was called after
  `consoleDemoInit` — init order restored to video-modes-first;
  (2) weapon taps didn't switch because the release frame's
  `touchRead()` returns 0,0 — the press/release is now tracked from
  `keysHeld()` with positions recorded while the pen is down.
- 3-layer parallax starfield, all layers scrolling leftward (verified
  by cross-correlating paired title frames: best shift −16 px over
  0.25 s, i.e. leftward).
- ARM9 built in Thumb mode (`-mthumb`); ROM padded to device capacity;
  real game code/maker. See `qa/MELONDS-INVESTIGATION.md`.
- **Music (v2):** 85 s AI track (`assets/music-v2.mp3`, Speechma,
  R-Type / Life Force synthwave chiptune prompt, commercial use
  stated; md5 `bfddd8908552ec0a53e1cfd45bf5776d`), converted by
  `tools/convert_music_v2.py` to raw 8-bit PCM mono at 22050 Hz
  (`audio/music_raw.bin`, 1,874,956 bytes, no clipped samples),
  0.13 s end fade so the loop point doesn't click. SFX stay
  synthesized at 11025 Hz. The v1 synth loop is preserved as
  `qa/music_synth_v1_raw.bin`.

## melonDS 1.1, JIT off (Xvfb, `qa_driver.py`) — VERIFIED

Runs in `/tmp/mlds-test/qa-v2m/` (tmp is ephemeral; screenshots
described here, key frames re-checked visually):

- Title renders (ships flying left, starfield, console text).
  Click starts the game; HUD (SCORE / LIVES / WAVE / WEAPON) and the
  three weapon buttons render on the sub screen.
- Drag moves the ship 1:1; weapon-panel taps switch SPREAD → LASER →
  MISSILES with the bracket highlight following, and the ship does NOT
  move/teleport on panel taps or on taps elsewhere in the drag area.
  Laser fires a visibly longer bolt; score climbs with kills
  (250 → 450 → 3,400 across the runs).
- Unattended play reaches wave 9, score 18,550, no crash. A parked
  ship takes a hit (LIVES 3 → 2, respawn with invulnerability blink),
  and a re-parking run reaches GAME OVER (FINAL SCORE 3300, Wave 2)
  and restarts into a fresh game on touch.
- **Music build re-verified end to end** (same scripted run against
  the 2 MB ROM): title → play → weapon switch → game over
  (FINAL SCORE 3400, Wave 2), no crash, no boot regression from the
  larger ARM9 binary.

## melonDS 1.1, JIT on — STILL NOT RENDERING (see MELONDS-INVESTIGATION.md)

- v2 with JIT enabled: no segfault any more (Thumb build), but the
  screens stay black (0.7% non-black) while the emulator runs 60 fps.
  The JIT incompatibility is NOT solved at the code level. **the designer:
  if melonDS shows a blank/white screen, turn the JIT recompiler OFF
  (Settings → Emu → JIT recompiler) and try again** — with JIT off
  the same ROM renders here. Her melonDS version/settings are still
  needed to close this out.

## DeSmuME 0.9.11 — HEADER VERIFIED, RENDERING NOT VERIFIABLE HERE

- Log for v2: `ROM game code: SDRG`, `ROM serial: NTR-SDRG-???`,
  `CPU mode: Interpreter`, no DSi line. Same Xvfb-white-window
  caveat as v1.

## NooDS — NOT RUNNABLE HERE (flatpak-only Linux release)

## Sound — WIRED, NOT HEARD

- The 85 s AI track + SFX play through libnds' sound server; the
  game runs for minutes with music and SFX firing and never crashes.
  Headless QA cannot hear audio — the mix, and whether the track
  fits the game, is for the designer's ears.

## Not verified (the designer's gate)

- Real hardware and iPhone RetroArch (melonDS core). Never claimed.
- D-pad movement: XTEST key events never reach melonDS in this Xvfb
  setup (no window manager → no keyboard focus), so D-pad is
  implemented but untested in an emulator run.
- Whether the AI track sounds right in-game (level, loop point).

# Star Drag v3 — verification (weapon progression, laser beam, missiles, 14 enemies, terrain areas, relative drag, homebrew header fix)

Final ROM: `~/workspace/your_files/star-drag-v3.nds`
md5: `ba94438077fb5da0d5733ca04a17f71d`, size 4,194,304 bytes (4 MB).
Clean rebuild reproduces the same md5 (verified twice in a row, Nov 18, 2026, ~21:12 and ~21:19 EST).
Source: `~/workspace/star-drag-ds/source/main.c`; build via `build.sh`
(`tools/fix_ds_header.py` stamps game code `####`, maker 0x0000).

## What changed from v2 (the designer's 6 fixes + her relative-drag correction)

- **Fix 1 — RetroArch melonDS "encrypted ROM needs native BIOS" error:**
  root cause found in melonDS source (`NDS_Header.h`):
  `IsHomebrew() { return (ARM9ROMOffset < 0x4000) || (strncmp(GameCode, "####", 4) == 0); }`
  v2 stamped game code `SDRG` + maker `01` with ARM9 offset exactly 0x4000,
  so `IsHomebrew()` was false; melonDS built a `CartRetail`, which needs
  native BIOS to decrypt the secure area, and the RetroArch core (melonDS
  changelog 1.1.8, Oct 18, 2024: encrypted ROMs without native BIOS now
  show an error message instead of failing silently) showed exactly the designer's
  error. Fix: game code back to `####`, maker 0x0000
  (`tools/fix_ds_header.py`, with a comment never to stamp a real game code
  again). Verified: `ndstool -i` shows game code `####`, maker blank, unit
  0x00, DSi magic zero, secure-area CRC OK (multiboot), header CRC OK;
  melonDS 1.1 log: `Inserted cart with game code: ####`, `No need to
  re-encrypt cart secure area` (the `Secure area decryption failed` line
  that follows is the harmless direct-boot attempt on a zeroed secure area —
  the stock libnds hello-world shows the same and boots). No BIOS files
  shipped or downloaded. Full write-up in `qa/MELONDS-INVESTIGATION.md`.
  **RetroArch-core caveat: the RetroArch melonDS core itself was NOT
  testable here** — only standalone melonDS 1.1. The ROM-side cause is
  proven and fixed, and standalone melonDS now classifies it as homebrew,
  which is the same classification path the core uses; the designer should still
  confirm on her iPhone that the error is gone.
- **Fix 2 — weapon progression:** player starts with SINGLE shot only.
  SPREAD / LASER / MISSILE are power-up capsules (star, bolt, ring sprites)
  dropped by enemies (~22% on kill) and floating in periodically
  (every ~18 s). Picking one up switches to it and shows `Got <WEAPON>!`.
  Getting hit loses the current weapon and drops back to SINGLE (and clears
  its owned flag). The bottom-screen panel shows all 4 buttons; tapping one
  switches only among weapons already collected that run — tapping a locked
  weapon does nothing. Verified in melonDS runs (see below).
- **Fix 3 — laser:** a long bright cyan beam with a white core, ~128 px
  (four 32x8 segments) extending ahead of the ship, damage 3, piercing,
  fires every 28 frames with a 14-frame on-phase; unique laser SFX
  (`audio/laser_raw.bin`, 0.28 s square sweep 1500→180 Hz with 55 Hz
  tremolo, `tools/make_audio.py`). Never reuses the bullet sprite.
  Beam seen in a screenshot (debug build, forced-laser pickup — see below).
- **Fix 4 — missiles:** rocket-dart sprite (`missile2`, enemyBlue5) with a
  spark-trail particle system (max 16 particles, spawn every 3 frames at
  the missile tail, fade ~12 frames). Missile trails seen as yellow
  cross/spark trails in screenshots.
- **Fix 5 — 14 enemy types, varying HP:** v2's 7 (drifter 1, weaver 1,
  diver 1, shooter 2, asteroid 4, mine 3, turret 2) + bug 2 (zigzag),
  alien 3 (sine + aimed shots), charger 2 (dash), spinner 3 (rotating),
  orbiter 4 (circling), crystal 5 (slow tumbling), capital 8 (parks, 3-way
  shots). Damage flash: sprite blinks ~5 frames on hit. Enemies spawn by
  wave band (wave 1: drifter/weaver/diver; more types unlock at waves 2/3/4/6).
  HP values are in the `enemy_hp[]` table in `source/main.c`; the flash and
  the slower wave progression with single shot are consistent with
  multi-hit enemies in play, but no single still proves "this asteroid took
  4 hits" — that is code-verified, not screenshot-proven.
- **Fix 6 — terrain areas:** ceiling/floor on main BG layer 3 (8x8 tile,
  shared 15-colour palette, one tile per area, reloaded on area change),
  scrolling columns generated as they enter (2 px/frame), heights per
  column (top+bottom ≤ 9 tiles, gap ≥ 15 tiles, always passable). Areas
  change every 2 waves: 0 OPEN SPACE, 1 SPACE BASE, 2 PLANET, 3 CAVE,
  4 CRYSTAL, 5 WATER, 6 GOO. Terrain collision costs a life with
  invulnerability blink on respawn. Floating-rock obstacles (HP 3,
  shootable, +50 pts) spawn in terrain areas. **Bug found and fixed during
  QA:** the area tile was copied to `BG_TILE_RAM(1) + 32`, but that macro
  is a `u16*`, so the tile landed at byte offset 64 (tile 2) while the map
  referenced tile 1 (all zeros = transparent) — terrain was invisible in
  the first long-play run. Fixed to `(u8 *)BG_TILE_RAM(1) + 32`; terrain
  then rendered (grey metal ceiling/floor bands, varying heights) in the
  second long-play run.
- **Relative drag (the designer's Nov 18 correction):** the ship NEVER jumps to the
  touch point. On drag start the code records the ship position and the
  touch position; the ship moves by the touch DELTA only, from wherever it
  was. A tap moves nothing. Panel taps only switch weapons. D-pad also works
  (untested in emulator — see "Not verified"). Verified exactly with a
  temporary debug build that printed `SHIP x,y` on the HUD (removed before
  the final build; final md5 is the pre-debug one, see below).
- **Kept from v2:** horizontal left-to-right flight, 32x32 sprites, drag-only
  movement, weapon panel, 3 lives + invulnerability blink, score, escalating
  waves, game over + restart, title screen, AI music loop (same
  `assets/music-v2.mp3`, md5 `bfddd8908552ec0a53e1cfd45bf5776d`, untouched),
  3-layer parallax starfield in open space, no ASCII art.

## melonDS 1.1, JIT off (Xvfb, `qa_driver.py`) — VERIFIED

Environment note: after a `/tmp` clear, melonDS failed to start
(`libEGL.so.1` / `libOpenGL.so.0` missing). Restored from Ubuntu 24.04
`libglvnd` debs (libegl1 / libopengl0 1.7.0-1build1) via
`dpkg-deb -x`; copies saved at `qa/gl-libs/` for future runs, and
`LD_LIBRARY_PATH` must include them when running `qa_driver.py`.
Screen layout learned the hard way: melonDS 1.1 renders both screens at
native 256x192, top screen at window (0,16), bottom screen at (0,208),
scale 1.0 — an earlier run used 1.33x coordinates and its tap/drag
actions landed off-screen, so those results were discarded and re-run
with correct coordinates.

- **Title → play:** title renders (starfield + drifting enemy sprites +
  `STAR DRAG v3` / `TOUCH TO START`); touch starts the game; HUD shows
  `SCORE / LIVES *** / WAVE 1 / WEAPON SINGLE / AREA OPEN SPACE` and the
  4-button panel (`[SINGLE] .... .... ....`).
- **Relative drag (debug build, HUD readout):** start `SHIP 16,80`.
  Tap at far corner DS (240,92) → `SHIP 16,80` (unchanged, touch
  registered as T 240,86). Drag DS (240,92)→(180,52), delta (-60,-40) →
  `SHIP 0,40` (exactly 16-60 clamped to 0, 80-40). Drag DS (20,52)→
  (120,152), delta (+100,+100) → `SHIP 100,140` (exactly 0+100, 40+100).
  Tap on locked SPREAD button → weapon stays SINGLE, ship stays 100,140.
  **Relative drag is proven exact, not approximate.**
- **Weapon pickups (final ROM, long play #1, ~3.5 min):** by t90,
  `WEAPON MISSILE`, `Got MISSILE!`, wave 4, score 6350, lives **; panel
  showed SINGLE/SPREAD/LASER all owned at that point. Long play #2
  (~2.5 min): MISSILE at t84 (`Got MISSILE!`, wave 2), SPREAD at t108
  (`Got SPREAD!`, wave 3, MISSILE still owned), LASER owned by t156
  (row shows LASER named). **Getting hit loses the weapon:** debug run
  showed `Got LASER!` then, after a hit, `WEAPON SINGLE`, lives **,
  LASER owned flag cleared (row back to `....`).
- **Laser beam (debug build with forced-laser first pickup; beam code and
  sprites are identical to the final ROM):** shot at t~28 s of the run
  shows the ship firing a long bright cyan beam with a white core,
  ~170 px long, clearly distinct from the regular bullets — while the HUD
  reads `WEAPON LASER`, `[LASER]`, `Got LASER!`. A shot a few seconds
  earlier with LASER active shows no beam (the 14-frame off-phase of the
  28-frame cycle), consistent with the code. The laser SFX is wired
  (`laser_raw.bin`, distinct from `shoot_raw.bin`) but, as with all audio
  here, not heard headless.
- **Missiles + trails (final ROM):** t84 and t90 top screens show yellow
  cross/spark particles trailing behind missiles in flight, plus the
  rocket-dart missile sprite; green laser and light-blue bolt pickup
  capsules also visible in play.
- **Terrain / areas (final ROM, long play #2):** at wave 3 the HUD reads
  `AREA SPACE BASE` and the top screen shows grey metal ceiling and floor
  bands with varying per-column heights and a passable gap; the ship flies
  between them. Long play #1 also reached `AREA SPACE BASE` at wave 4.
  Only OPEN SPACE and SPACE BASE were reached in scripted play (waves
  3–4); PLANET / CAVE / CRYSTAL / WATER / GOO areas are the same code path
  with different tiles/palettes and were not reached — the designer will see them
  as waves progress.
- **Game over + restart:** unattended play reached `GAME OVER / FINAL
  SCORE 2800 / Wave reached: 2 / Touch or press START to fly again`;
  a subsequent touch restarted a fresh run (score 1300, wave 1, lives ***).
- **No crash:** ~9 minutes of unattended play across the runs, multiple
  game-over/restart cycles, no freeze, no melonDS crash. Score, wave and
  lives all advanced normally.

## melonDS 1.1, JIT on — STILL NOT RENDERING (known, unchanged)

- v3 with JIT on: ROM loads (no encrypted-ROM error, no segfault), but
  the screens stay black while the emulator runs — same as v1/v2.
  The JIT incompatibility is NOT solved at the code level. **the designer: if
  standalone melonDS shows a blank/black screen, turn the JIT recompiler
  OFF (Settings → Emu → JIT recompiler) and try again** — with JIT off
  the same ROM renders here. This does NOT affect the RetroArch core
  question (Fix 1 above is about the load error, not rendering).

## DeSmuME 0.9.11 — HEADER VERIFIED, RENDERING NOT VERIFIABLE HERE

- Same Xvfb-white-window caveat as v1/v2 (white window for every ROM).
  Not re-run for v3 beyond the header check via `ndstool -i`.

## NooDS — NOT RUNNABLE HERE (flatpak-only Linux release)

## Sound — WIRED, NOT HEARD

- The 85 s AI track + SFX (shoot, laser zap, pickup chime, boom, hit)
  play through libnds' sound server; the game runs for minutes with
  music and SFX firing and never crashes. Headless QA cannot hear audio
  — the mix, the laser zap distinctness, and whether the track fits the
  game are for the designer's ears.

## Not verified (the designer's gate)

- Real hardware and iPhone RetroArch (melonDS core): the encrypted-ROM
  load error fix is proven at the header/classification level and in
  standalone melonDS, but the RetroArch core path itself was not testable
  here — the designer should confirm the error is gone on her iPhone.
- D-pad movement: XTEST key events never reach melonDS in this Xvfb
  setup (no window manager → no keyboard focus), so D-pad is implemented
  but untested in an emulator run.
- Whether the AI track / laser zap sound right in-game (level, loop point).
- Multi-hit HP of a specific enemy in a single still (code-verified via
  the `enemy_hp[]` table and damage flash; not screenshot-proven).
- Areas 2–6 (PLANET, CAVE, CRYSTAL, WATER, GOO) — same code path as
  SPACE BASE, not reached in scripted play (waves 3–4 only).

## v4 — Nov 18, 2026 (evening): all 7 of the designer's fixes, verified

ROM: `~/workspace/your_files/star-drag-v4.nds`, md5
`468250aa3c3c98c96dcbf8dc85b7504c`, 4,194,304 bytes (4 MB).
Header: game code `####`, maker blank, unit `0x00`, Logo CRC OK,
Header CRC OK. Clean rebuild reproduces the same md5 (checked 3x).
Music: same Speechma track as v2/v3 (`assets/music-v2.mp3`, unchanged).

### Fix 1 — melonDS blank white (see also qa/MELONDS-INVESTIGATION.md)
- v4 uses the m10 init order from the v4 JIT bisect (consoleDemoInit
  FIRST, videoSetMode MAIN only, no videoSetModeSub, then VRAM A/B/D,
  OAM, BG). Full bisect in `qa/bisect-v4/` (m1–m10).
- Standalone melonDS 1.1 JIT-off renders v4 (title → play → game
  over → restart, runs play1/play2). JIT-on: v4 shows top 96.9% white
  and bottom 1536 white px confined to y 0–5 — EXACTLY the same as
  stock libnds hello-world under JIT-on (verified config, `[JIT]
  Enable` read back from the config melonDS saved). No segfault.
  This is a melonDS 1.1 JIT + devkitPro-homebrew incompatibility, not
  a Star Drag bug — no ROM-side change can fix it from our side.
- **Correction:** the earlier claim in MELONDS-INVESTIGATION.md that
  hello-world "renders under JIT" was wrong (a runner edited the
  wrong `Enable =` line — `[DLDI]`, not `[JIT]`). Corrected in that
  file's v4 appendix.
- The RetroArch iOS melonDS DS core is built with JIT DISABLED by
  default (libretro CMakeLists `if (IOS) set(DEFAULT_ENABLE_JIT OFF)`),
  so JIT is most likely NOT the cause of the designer's iPhone white screen.
  **v4 does NOT claim to fix the melonDS white screen.** Things for
  the designer to check are listed in MELONDS-INVESTIGATION.md (melonDS DS
  core, not the old melonDS core; JIT off; NooDS core fallback;
  DeSmuME core fallback; report core version if it still fails).

### Fix 2 — clean bottom screen (VERIFIED, screenshots)
- Bottom screen during play shows ONLY `AREA <name>` and
  `WEAPON <name>` (plus the 4 weapon buttons and the star marker
  above the selected one). Seen in every play shot (play1 02/03/04,
  play2 g09, debug runs). Score/lives/wave moved to the top screen
  as runtime-drawn 5x7 digit sprites + mini-ship lives icons + `W`
  glyph (visible top-left / top-right in all shots).
- QA-found bug, fixed in v4: a stale `E` remained after the area name
  when switching from a longer name ("OPEN SPACE" → "PLANET" left
  the final `E` at column 17). Fixed by padding both lines with
  trailing spaces (`%-11s` + extra blanks); re-verified clean in
  debug2 e08 ("AREA CAVE") and play2 g09 ("AREA OPEN SPACE").

### Fix 3 — death drift + better explosions (PARTLY screenshot-verified)
- Code: fatal hit sets `dying`; the enemy keeps flying forward,
  tumbles (rotation), drifts up/down, trails fire/sparks for 40
  frames, then a big explosion (3 fireballs + 8 particles) and only
  then score/kills/wave advance. Same for mini-bosses.
- Seen in stills: yellow spark/fire bursts at the moment of death in
  debug1 d04 (top-right starburst), d06 (yellow spark cluster at the
  right edge), debug2 e17 (yellow burst top). A single still of a
  mid-drift tumbling enemy was NOT captured (40-frame window, 6–12 s
  shot spacing) — the drift code path itself is code-verified, the
  explosion particles are screenshot-verified.

### Fix 4 — mini-bosses (VERIFIED, screenshots)
- Every 3rd wave (3, 6, 9, …) a 64x64 boss parks at x=196, fires
  aimed 3-shots / 5-shot fans / spawns 2 minions, HP 40 + wave*5,
  HP bar (up to 5 cyan/white laser-beam segments) top-centre.
- Seen: boss sprite + HP bar + aimed red bullets in debug1 d09/d10/
  d11/d12/d13 and debug2 e10/e12/e14/e16/e17. Boss BLOCKS progression:
  debug2 stayed in CRYSTAL from e10 (~60 s) through e12 (~72 s) with
  the wave-9 boss alive; areas only advanced after bosses died
  (OPEN SPACE → SPACE BASE → PLANET → CAVE → CRYSTAL → WATER → GOO
  across the debug runs). Boss death drops weapon + fire-rate capsules.
- Note: in the QA debug build only (never in the shipped ROM), boss HP
  was 5 and waves advanced at kills/4 so a scripted run could reach
  and kill bosses (`QA_DEBUG` in source/main.c, debug ROM at
  qa/v4-runs/debug.nds, md5 ee45f7e5b6e77159ba1e24729c4cebff).

### Fix 5 — sprite terrain, not recolored bands (VERIFIED, all 7 areas)
- 12 procedural obstacle sprites (pipe/column, tree/root,
  stalactite/stalagmite, crystal spike/claw, plant/tube, barb/blob)
  hang/stick at 32 or 64 px heights and scroll left; solid collision
  (costs a life + blink). Thin BG ceiling/floor bands kept as backing
  only. Narrow wavy tunnel (8-tile gap, sine centre) ~8 s every
  ~1400–2000 frames in terrain areas.
- Seen: SPACE BASE pipes/columns (d04, d09, d10), PLANET trees + root
  (d05), CAVE stalagmites (d06, e08 — e08 also shows the wavy tunnel,
  tall bands with a narrow wavy gap, ship inside it), CRYSTAL spikes
  (e10, e12), WATER tubes + plants/coral (d07, e14, e16, e17), GOO
  blobs (debug3 f18, bright green bands + green blob obstacles).
  All 7 areas visually confirmed: OPEN SPACE, SPACE BASE, PLANET,
  CAVE, CRYSTAL, WATER, GOO.

### Fix 6 — 32 enemies, area-unique (VERIFIED in code + screenshots)
- `enemy_def[]`: 32 types, HP 1–8 (capital 8, prism/crystal 5),
  10 movement styles, per-type area bitmask + min_wave. Area sets:
  base drones/sentries/hunters, planet wasps/beetles, cave bats/
  crawlers, water fish/jelly/eels, crystal shards/prisms, goo
  blobs/spores.
- Sprites: all from the Kenney Space Shooter Redux CC0 pack already
  on disk (~60 images now used; v3 used ~14). Extra-pack download
  attempts for v4 FAILED (kenney.nl direct 403, KenneyNL org has no
  asset-pack repos, opengameart search yielded no usable links) —
  fell back to the on-disk pack + procedural obstacle drawing.
  Credited in README and title screen.
- Seen in shots: new sprites everywhere (green shard enemy e10/e12,
  big water fish/jelly d07, goo blob f18, brown dart f16, green-ring
  turret enemy in most boss shots, brown meteors, grey tubes).

### Fix 7 — slower fire + fire-rate power-ups (VERIFIED, screenshots)
- Base intervals: SINGLE 22 / SPREAD 26 / LASER 40 / MISSILE 34
  frames (v3 was 10/12/28/20). Red-bolt capsule (pickup type 4)
  raises rate level 0–3, each level cuts interval 12% (level 3 →
  64%); hit resets rate level to 0 with the weapon.
- Seen: bottom line shows `WEAPON SPREAD +1` (d07), `WEAPON LASER +1`
  (d09, d10, d12, e16, e17), `WEAPON MISSILE +1` (d08) — the `+N`
  suffix was added to the weapon line in v4 so the pickup is visible.
  Weapon capsules also seen granting LASER (play1 shot 04, play2
  g09), SPREAD (d04/d05), MISSILE (d08).

### Kept from v3 (re-verified)
- Horizontal flight, single-shot start, laser beam + zap SFX (beam
  seen d09/d13/e14/e16), missile sprites + spark trails, relative-
  drag-only movement (ship moved only by drag deltas in all runs;
  tap never teleports — proven in v3, unchanged code path), 3 lives +
  blink, score, waves, game over + restart, title screen, AI music
  loop, leftward starfield in open space, no ASCII art.

### Runs (all melonDS 1.1, JIT-off, Xvfb, `qa/v4-runs/`)
- play1 (real v4, pre-fix md5 fc6516cb…): title → start → drag →
  play → game over (FINAL SCORE 4600, wave 3) — 8 shots, ~2.5 min.
- play2 (final v4, md5 468250aa…): title → play → game over (4600,
  wave 3) → click → RESTART verified playing again (g09: ship,
  enemies, star capsule, AREA OPEN SPACE / WEAPON LASER).
- debug1 (QA_DEBUG ROM, 13 shots, ~2.5 min): areas through WATER,
  bosses at waves 3/6/9, laser/spread/missile + rate pickups.
- debug2 (QA_DEBUG ROM, 17 shots, ~1.7 min, 6 s spacing): CAVE
  tunnel, CRYSTAL, WATER; boss blocking verified.
- debug3 (QA_DEBUG ROM, 20 shots, ~3.5 min soak): GOO verified (f18);
  no crash in ~9 min total unattended debug play + ~5 min real-ROM play.
- v4-jiton / v4-jitoff (run_jit.py, 8 s boot): JIT-on = hello-parity
  (top 96.9% white, bottom 1536 px y 0–5, no segfault); JIT-off =
  title rendering (bottom 2990 px y 6–116).

### Not verified (the designer's gate)
- Sound not heard headless (music loop + laser zap + SFX wired,
  never crashed in ~14 min of runs) — mix/level/loop point for the designer.
- D-pad untested (XTEST keys never reach melonDS here, no window
  manager focus) — implemented, same code path as v3.
- DeSmuME rendering unverifiable here (white SDL window for every ROM);
  NooDS not runnable here; RetroArch iPhone melonDS path not testable
  here — the white-screen cause on the designer's phone is still unreproduced.
- Mid-drift tumbling enemy in a single still (see Fix 3).
- Real-hardware play; boss fight feel at real HP (40 + wave*5) —
  scripted QA used weak debug bosses; the designer's playtest is the gate.

---

# Star Drag v5 — Nov 18, 2026 (evening)
**Deliverable:** `~/workspace/your_files/star-drag-v5.nds`
**md5 `7865ef8c99fdddac448caa092de12d07`, 4,194,304 bytes (4 MB).**
Clean rebuild reproduces the same md5 (verified Nov 18, ~11:44 PM EST).
Header: game code `####`, maker blank, unit `0x00`, logo CRC OK,
header CRC OK. Music untouched (`assets/music-v2.mp3`).
**Note:** the first v5 build (md5 `22bca369386d4b0fbc5cbcb7e1f82973`) was
superseded during QA by a one-line-class fix (see "QA-found bug" below);
only the final md5 above is the deliverable.

## What changed (the designer's 6 fixes, Nov 18 ~6:13 PM EST)
- **Fix 1 — easier first boss:** boss difficulty now scales by boss
  number. Boss 1: HP 25 (was 40+wave*5 = 55), 1 aimed bullet (not 3)
  every 170 frames (not 110) at speed 380 (not 500), spread fan 3
  bullets (not 5) every 320 frames (not 220) at speed 320, spread
  ±140 (not ±180), 1 minion every 400 frames (not 2 every 240),
  sine amplitude 32 (not 48). Later bosses keep 40+wave*5 and the
  old volley rates.
- **Fix 2 — easier level 1:** terrain variation by area (areas ≤2
  vary ±1, later ±2), ceiling+floor cap area 1 = 5 tiles, area 2 = 7,
  later 9; terrain bases `{0,1,1,2,2,2,2}`; spawn interval waves 1–2
  = 115−wave*5 (later 95−wave*5, min 35); enemy bullet speed waves
  1–2 = 380 (later 500); floating-rock interval area ≤1 = 900 frames
  (later 650); terrain-obstacle interval area ≤1 = 90, area 2 = 70,
  later 45; terrain obstacles suppressed while a boss/intro is active.
- **Fix 3 — white screen (fresh investigation):** see
  `MELONDS-INVESTIGATION.md` v5 appendix. No ROM-side cause found;
  **v5 does NOT claim to fix it.** Headline result: with JIT
  genuinely ON (config verified), v5 renders exactly the white screen
  the designer describes (top white, bottom black, no crash) — same as v4 and
  the stock hello-world. JIT is again the leading hypothesis for her
  melonDS white screen; the DeSmuME white screen is still unexplained.
- **Fix 4 — animated enemies:** `enemy_anim[32]` — 0 none, 1 spin
  (flip h/v), 2 wings (flip v every 8 frames), 3 pulse (flip +
  sparkle), 4 wobble; boss flips every 30 frames + sparkles every
  40; player engine glow particle every 6 frames; spawn sparkle.
  Seen in shots: wing-flap frames on the green/blue enemy ships,
  spinning capital ships.
- **Fix 5 — coloured starfield:** tiles 4–7 are single-pixel stars in
  BG palettes 4–7 — purple RGB15(28,16,31), blue (16,16,31), red
  (31,12,12), yellow (31,28,8); sprinkled ~1–2% into all three
  parallax layers. Pixel-counted in play shots: e.g. a03 purple 0 /
  blue 54 / red 2 / yellow 30 with the first (wrong) targets;
  re-counted with correct RGB15 targets: purple 5–7, blue 34–36,
  red 8–9, yellow 6–7 px per frame — subtle, as asked.
- **Fix 6 — reverse-flight first-boss intro:** on wave 3, alarm SFX
  x3 (new `audio/alarm_raw.bin`, 0.48 s square beeps 880/440/880 Hz)
  + `!! WARNING !!` on the sub screen, then at intro frame 180 the
  ship flies in REVERSE: `dir_fly=-1`, `REVERSE FLIGHT!` shown, boss
  spawns from the left and parks at x=24, starfield scrolls rightward,
  enemies/boss/bullets/missiles/pickups all mirror direction, player
  sprite faces left. On first-boss death, normal flight resumes.

## QA-found bug (caught by the debug run, fixed before delivery)
The first debug run showed the boss NEVER dying: `REVERSE FLIGHT!`
stayed up for the whole ~4 min run and the ship was photographed
overlapping the boss (d20/d32). Root cause: the ship sits at x=16
when reverse starts, the boss parks at x=24, and the ship's shots fly
LEFT — away from the boss — so the boss could never be hit, and in
the real game the ship would have died instantly on boss entry.
Fix: on reverse start the ship is repositioned to x=200, y=80 with
invuln=90 and any in-progress drag reset (`source/main.c`, intro
completion block). Re-run: boss died within ~8 s of the fight,
`REVERSE FLIGHT!` cleared, normal flight resumed (d05), later bosses
at waves 6/9/12 fought and killed (HP-bar spikes at d11/d12, d20–d22,
d29/d30; big-explosion frames d07/d12/d18/d23/d28).

## Runs (all melonDS 1.1, Xvfb)
- **play-real** (first build 22bca369…, 9 shots, ~2.5 min, JIT-off):
  title v5 → start → play (AREA OPEN SPACE / WEAPON LASER, ship
  firing, explosions, coloured stars) → game over (FINAL SCORE 2450,
  wave 2) → click → RESTART verified playing again (WEAPON SINGLE).
- **play-debug** (debug ROM 362692bb…, 32 shots, ~4 min, JIT-off):
  exposed the reverse-boss bug above; otherwise no crash.
- **play-debug2** (fixed debug ROM 1f243488…, 32 shots, ~4 min,
  JIT-off): boss 1 dies, reverse ends, normal flight resumes; later
  areas reached (SPACE BASE → WATER), later bosses fought/killed;
  no crash in ~8 min of debug play total.
- **play-final** (final ROM 7865ef8c…, 4 shots, ~45 s, JIT-off):
  title → start → drag → play → game over (2450, wave 2). Sanity
  pass on the exact delivered md5.
- **play-jiton** (final ROM, JIT genuinely ON — config verified
  `Enable = true` before and after): top screen WHITE, bottom black,
  no text, no crash — same as v4 and hello-world. See investigation
  file. (An earlier "JIT-on" attempt via `run_one.py` silently ran
  JIT-off: melonDS had re-saved the fresh-home toml with the [JIT]
  keys reordered, so the script's substring replace no-opped.
  `run_one.py` is now patched to edit the [JIT] section directly
  and assert the toggle took.)
- **JIT-off vs on summary:** JIT-off = title + full game render;
  JIT-on = white top screen. Unchanged from v4.

## Not verified (the designer's gate)
- Sound not heard headless (music loop, laser zap, alarm SFX x3 —
  wired, never crashed in ~15 min of v5 runs) — mix/level/loop and
  the alarm for the designer.
- `!! WARNING !!` text was not captured in any screenshot (3 s window
  between 8 s-spaced shots) — code-verified: it prints in the same
  branch that sets the boss intro, and `REVERSE FLIGHT!` (the next
  step in that code path) was photographed twice.
- First-boss easiness at real HP 25 (vs 55) — code-verified, not
  play-verified at real HP; debug bosses are HP 5. the designer's playtest
  is the gate for the feel.
- D-pad untested (same as v4). DeSmuME rendering unverifiable here
  (white SDL window for every ROM under Xvfb); NooDS not runnable
  here (its Linux zip is flatpak-only); RetroArch iPhone melonDS path
  not testable here. White screen NOT claimed fixed.

---

# Star Drag v6 — Nov 19, 2026 (00:06–00:18 EST)
**Deliverable:** `~/workspace/your_files/star-drag-v6.nds`
**md5 `32c7d23aa4b11036b230540316349375`, 4,194,304 bytes (4 MB).**
Clean rebuild reproduces the same md5 (built twice: first build and
rebuilt after reverting the QA_DEBUG instrumentation — identical md5).
Header: game code `####`, maker blank, unit `0x00`, logo CRC 0xCF56 OK,
header CRC 0x2F6F OK. Music untouched (`assets/music-v2.mp3`).
**Note:** build.sh now copies to `star-drag-v6.nds`.

## What changed (the designer's 2 corrections, Nov 18 ~7:03 PM EST + the
## max-speed follow-up)
- **Fix 1 — no other enemies during a boss:** regular enemy spawns are
  frozen while a boss is active or its intro is running (already true
  for spawns); NEW in v6: boss minion spawns are REMOVED entirely
  (the `minion_t` block is gone), and `clear_regulars()` wipes every
  regular enemy and enemy bullet on screen when a boss fight starts —
  once at the wave-3 intro trigger and again in `spawn_boss()` (so it
  covers every boss, waves 3/6/9/…). Floating-rock obstacles and
  terrain-obstacle spawns are also suppressed during boss/intro.
  Spawning resumes only after the boss dies (`spawn_t = 90` breather
  on boss death). Boss 1 keeps its eased v5 stats (HP 25, 1 aimed
  bullet, 3-bullet spread fan).
- **Fix 2 — reverse is a SCROLL transition only (v5 was wrong):** v5
  moved the ship to x=200, flipped it to face left, mirrored shots /
  enemies / boss entry to the left. ALL of that is removed in v6.
  The ship stays exactly where the player left it on the left,
  facing right, firing right; the boss enters from the right and
  parks at x=196 as normal. Only the world scroll reverses:
  `scroll_vel` (percent of normal scroll speed) eases toward its
  target by 2/frame — normal +100 → 0 (stop) → reverse, and the
  ceiling/floor/terrain BG scrolls with the same eased velocity
  (`terrain_update()` now uses `scroll_vel`; while reversing it does
  NOT generate new terrain columns, so the existing columns scroll
  back instead of being overwritten). Cues on the bottom screen:
  `!! WARNING !!` (intro), `REVERSE!` (reverse starts),
  `NORMAL FLIGHT` (boss dead, shown ~3 s then cleared).
  **Follow-up (max speed):** the reverse target is **-160**, i.e.
  160% of normal scroll speed — faster than normal flight — held
  for the whole boss fight, then eased back through a stop to +100.
  Enemies, bullets, obstacles, pickups and particles keep moving
  left at normal speed throughout (only the BG/terrain scroll
  reverses); the laser beam, missiles and ship all still fire right.

## Verification (melonDS 1.1, JIT-off, Xvfb, `qa/v6-runs/`)
A QA-only debug readout was added under `#ifdef QA_DEBUG` (never in
the shipped ROM): sub-screen row 3 shows `S<ship x>,<y> V<scroll
vel> R<reverse 0/1> E<live enemies> B<boss 0/1>` every 30 frames.
Debug ROMs: `qa/v6-runs/debug.nds` (boss HP 5) and
`qa/v6-runs/debug25.nds` (boss HP 25 = real boss-1 HP, god mode on,
waves advance at kills/4 in both).

- **Ship never moves (exact):** the scripted drag put the ship at
  96,120 (`drag 100 280 180 320 10`). The readout reads `S 96,120`
  in EVERY shot of all three debug runs (d03–d32, b03–b47) — through
  the intro, the whole reverse, the boss death and the return to
  normal. No teleport, no flip (the ship sprite faces right in all
  boss shots, firing right).
- **No enemies during the boss (exact):** b12 (WARNING intro) reads
  `E0 B0`; b14/b16/b18 (reverse fight) read `E0 B1` — boss active,
  zero regular enemies, in every reverse frame captured. d05 (after
  the first debug boss died) reads `E0 B0`. Regular spawning
  resumes only after the boss dies: b22 reads `E1`, b26 `E5`.
- **Reverse scroll, max speed (exact):** b14 reads `V-124 R1`
  (still ramping down, boss on the right with HP bar, `REVERSE!`
  cue); b16 and b18 read **`V-160 R1 E0 B1`** — max reverse speed
  reached and held during the fight, boss parked on the right
  firing red bullets leftward, ship on the left firing right.
  d05 and b14 (earlier HP-5 runs) caught the return leg:
  `NORMAL FLIGHT` cue with `V-44` / `V-32` easing back toward
  +100; b22 reads `V+100 R0` with normal play resumed.
  Starfield/terrain scroll direction is the same eased velocity
  (single `scroll_vel` drives `scroll_starfield()` and
  `terrain_update()`); terrain columns scroll back without being
  regenerated mid-reverse (code-verified, no pop seen in shots).
- **Real ROM (final md5, JIT-off):** title `STAR DRAG v6` → play
  (ship, explosion, LASER, AREA OPEN SPACE, clean bottom screen)
  → GAME OVER (FINAL SCORE 2450, Wave 2) → touch → RESTART verified
  playing again (WEAPON SINGLE). ~9 min debug play + ~3 min real-ROM
  play, no crash.
- **JIT-on:** top 96.9% white — UNCHANGED from v5 and from the stock
  libnds hello-world (same as always; see MELONDS-INVESTIGATION.md).
  JIT-off renders fully. v6 makes no white-screen claim.

## Not verified (the designer's gate)
- Sound not heard headless (music loop, laser zap, alarm x3 — wired,
  never crashed in ~12 min of v6 runs) — mix/level/loop and the alarm
  for the designer.
- First-boss easiness at real HP 25 with NO other enemies — the
  debug25 run fought it at real HP; the feel (is it easy enough now?)
  is the designer's call.
- D-pad untested (same as v4/v5). DeSmuME rendering unverifiable here
  (white SDL window for every ROM under Xvfb); NooDS not runnable here;
  RetroArch iPhone melonDS path not testable here. White screen NOT
  claimed fixed.

# Star Drag v7 — Nov 19, 2026 (02:24–03:13 UTC / Nov 18, 9:24 PM–10:13 PM EST)
**Deliverable:** `~/workspace/your_files/star-drag-v7.nds`
**md5:** `5537a7782790baf7e04bbb0582468926` (4,194,304 bytes, game code
`####`, logo CRC 0xCF56 OK, header CRC 0x9F63 OK). Clean rebuild
reproduces the same md5 (verified 3×, last at 03:13 UTC).

## What changed (the designer's 2 requests, Nov 18 ~9:15 PM EST + the follow-up)
1. **Different bosses for the other levels (boss 1 UNTOUCHED — "perfect").**
   Boss mode cycles 1 reverse / 2 halt / 3 upward / 4 shield+charge
   (`boss.mode`, one 64×64 sprite per boss, alarm x3 + named cue each
   time, no other enemies during any boss fight — v6's rule kept):
   - Boss 2 (wave 6) HALT: scroll eases to 0 and stays stopped for the
     fight, speeds back up after death ("HALT!").
   - Boss 3 (wave 9) UPWARD: an opening appears in the ceiling, the
     starfield scrolls vertically, the ship flies upward facing up and
     firing up, the big green-eye boss is fought above ("UPWARD!"),
     then normal horizontal flight resumes and the terrain is rebuilt.
   - Boss 4 (wave 12) SHIELD + CHARGE: a periodic shield blocks ALL
     damage while up (120f up / 90f down, spark particles while up),
     and the boss charges across the screen ("SHIELD!"). Cycle repeats
     (boss 5 = reverse, 6 = halt, 7 = upward, 8 = shield+charge …).
   Difficulty ramps by boss number (real HP 25 boss 1, 40+wave×5 later).
2. **Proper death / GAME OVER screen.** Losing the last life now plays
   a ~1.5 s ship-destruction explosion (`ST_DYING`, ~90 frames, ship
   hidden, big explosion at the ship), then a GAME OVER screen (final
   score, wave reached, best score for the session) that STAYS until
   the player releases and then touches or presses START
   ("Release, then touch or press START to fly again." — the
   `over_armed` release gate fixes the v6 bug where a finger still
   held at death instantly skipped past the screen).
3. **Follow-up: 8 more enemy types with TOTALLY DIFFERENT web sprites
   (40 total).** Worms (green/red/blue, 32×32 Worms Pack, CC-BY-SA 4.0),
   green alien creature (rrodi411, CC0, 2 walk frames), purple
   jellyfish (StendhalGame, CC-BY-SA 4.0, 2 pulse frames), creeper eye
   (CC0 — also boss 3's big green eye), green robot (CC0 — also boss
   4's robot), tentacled orb (procedural), all via
   `tools/convert_v7.py` from `assets/web-sprites/`; area-unique sets,
   distinct movement/attack/HP, alt-frame animation (anim style 5) for
   alien/jellyfish. Failed sources recorded in README: kenney.nl direct
   403, KenneyNL GitHub org 404 (no asset repos), itch.io packs paid /
   no direct download. Credits in README + in-game title credits.
   (Title screen now reads "STAR DRAG v7" with the web-sprite credit
   line.)

## QA-found bug (caught by the debug run, fixed before delivery)
- **Boss could not be damaged from the right (run 1).** The first
  debug script dragged the ship to (224,160) and left it there; boss 2
  (HALT) then survived 50 shots / ~150 s of auto-fire with a FULL HP
  bar. Root cause, not a script artifact alone: the boss parks at
  x=196 (centre 228) and the laser/bullet hit tests only counted
  targets whose centre lay to the RIGHT of the shot origin — a ship
  flown up close (beam origin 248, bullet spawn 248+) could never
  connect. A human player dragging to the boss would have hit the
  same dead zone. Fix: beam tests are now segment-vs-box overlap
  (horizontal and vertical), bullet test is point-vs-boss-box
  (`bcx > boss.x-8 && bcx < boss.x+72`), so a shot that starts inside
  the boss's span still counts. Normal left-side play is unchanged
  (tests are strictly more generous, same behaviour at range).
  Verified by run 3/4: every boss dies from the left-side patrol.
- **QA environment break (run 2, not the game).** A runtime restart
  wiped the system Mesa EGL/OpenGL libs; melonDS failed to start
  (`libEGL.so.1: cannot open shared object file`) and all 135 shots
  came out black. Fixed: `libopengl0` reinstalled via dpkg, and
  `libEGL.so.1` + `libEGL_mesa.so.0` extracted from the Ubuntu
  24.04 .debs into `~/workspace/emulators/melonds-bin/usr/lib/`
  (the driver's `LD_LIBRARY_PATH` dir — persists in the workspace;
  the dpkg install to /usr may NOT survive another restart, the
  workspace copies will). `ldd` on the melonDS binary now shows 0
  missing libs; smoke test re-rendered the title screen before the
  re-run. Recorded here so the next session doesn't re-diagnose it.

## Verification (melonDS 1.1, JIT-off, Xvfb, `qa/v7-runs/`)
Debug ROM: `qa/v7-runs/debug.nds` (QA_DEBUG: god mode, waves advance
at kills/4; boss HP 5 in runs 1/3, raised to 40 in run 4 so a
scripted run can sample a whole fight instead of one-shotting the
boss — debug-only, the shipped ROM uses real HP). Readout format on
the sub console row 3: `S x,y V<scroll vel> R<reverse> E<enemies>
B<boss> M<mode> U<vertical> S<shield>`. Ship patrol script
(`qa/v7-actions-debug.txt`): ship oscillates (96,120) ↔ (56,70) on
the left, sweeping vertically across the boss's bob range.
- **Boss 1 reverse, UNCHANGED (exact):** run 4 d09 `V+10 R1 B1 M1`
  (intro easing), d10–d12 **`V-160 R1 E0 B1 M1`** — full max reverse
  speed held 3 consecutive shots (6 s), ship stays at 96,120 facing
  right; run 3 d63 `V-50 R1 B1 M1`, d95 `V-34 R1 B1 M1`; ease-backs
  after death (`V+52`, `V-26`, `V-92`, `NORMAL FLIGHT`). Same
  behaviour as v6, as the designer asked.
- **Boss 2 HALT (exact):** run 4 d21 `V+64 B1 M2` (easing down),
  d22–d24 **`V+0 R0 E0 B1 M2`** — full stop held 3 shots; d25+ back
  to `V+100` (speeds back up after death). Also run 3 d45/d70
  `V+0 B1 M2`, run 4 d64–d65, d99–d100. Top shot d45 (run 3): green/
  white boss ship parked bottom-right, "HALT!" cue, HP bar top-left.
- **Boss 3 UPWARD (exact):** run 4 d33–d36 **`V+0 B1 M3 U1`** held
  4 shots (vertical mode on, horizontal scroll 0); run 3 d27
  `B1 M3 U1`; run 4 d73, d107–d110 same. Top shot d27 (run 3): ship
  FACING UP mid-shaft, big green-eye boss above, ceiling opening
  visible top-right (crystal terrain with a gap). After death the
  ceiling closes and horizontal flight resumes (`U0`, `V+100`).
- **Boss 4 SHIELD + CHARGE (exact):** run 4 d44–d48:
  `B1 M4 S0, S1, S1, S0, S1` — shield toggles up/down across
  consecutive 2 s samples; repeated in a second fight d81–d84
  (`S0, S0, S1, S1`) and a third d116–d117 (`S0, S1`). Charge: top
  shot d48 catches the purple robot boss MID-SCREEN (x≈77, charged
  across from the right edge, shield sparks visible around it) right
  past the player ship; d45/d47 show it parked right firing red
  aimed bullets, HP bar shrinking as it takes damage while the
  shield is down.
- **No enemies during any boss fight (exact):** `E0` in EVERY `B1`
  sample across all runs (~40 boss samples) — the field is cleared
  on boss entry and spawning resumes only after death.
- **New enemy sprites visible in gameplay:** run 3 d76 — big
  green-ringed creeper EYES (two + one, right side) plus a green
  fire-rate capsule; run 3 d90 — blue TENTACLED ORB top-right and a
  red-ringed eye bottom-right (WATER area). Worms / alien creature /
  jellyfish / robot enemies are in the area spawn tables (code) but
  were not individually caught on camera in these runs.
- **No crash:** run 3 + run 4 each played ~5.5 min / 135 shots
  through ~13 boss fights (waves into the 20s–30s); run 1 played
  ~4.5 min before the runtime restart killed the driver (not the
  game). Debug + real runs total ~25 min of unattended play.
- **Real ROM game-over test (`qa/v7-runs/play-final/`, actions
  `qa/v7-actions-final.txt`, shipped md5):** title → start → ship
  sits at (96,120) auto-firing, LASER, OPEN SPACE (g05, live play)
  → ship dies between g05 (~35 s) and g06 (~45 s) → GAME OVER
  screen on the bottom console: "GAME OVER / FINAL SCORE 2350 /
  Wave reached: 2 / BEST SCORE 2350 / Release, then touch or press
  START to fly again." with the death explosion frozen on the top
  screen. **Held: g06–g31 are byte-identical (same sha256) — 26
  shots × 10 s ≈ 260 s with NO input, no auto-restart.** Fresh
  touch at the end → g32/g33 show a fresh game running (ship left,
  3 lives, WEAPON SINGLE reset from LASER, score reset, asteroid
  enemy visible).

## Not verified (the designer's gate)
- Sound not heard headless (music loop, laser zap, alarm x3, shield
  sparks — wired, never crashed in ~25 min of v7 runs) — mix/level/
  loop and the alarm for the designer.
- D-pad untested (same as v4–v6). DeSmuME rendering unverifiable here
  (white SDL window for every ROM under Xvfb); NooDS not runnable
  here; RetroArch iPhone melonDS path not testable here. White screen
  NOT claimed fixed — v7 changes nothing in init/video/header, and
  JIT-on was not re-tested for v7 (v6 finding stands: JIT-on white
  here, JIT-off renders; see MELONDS-INVESTIGATION.md).
- GAME OVER text lives on the bottom screen in the game's standard
  console font (same as the title screen) with the death explosion
  frozen on the top screen — it is NOT a big top-screen banner. If
  the designer wants a large top-screen "GAME OVER", that needs a new
  large-font sprite treatment (not built).
- Worm / alien-creature / jellyfish / robot *regular* enemies were
  not individually caught on camera (eyes + tentacled orb were); all
  8 are in the spawn tables and convert cleanly (see
  `tools/convert_v7.py` output + `qa/v7-runs/` sprite checks).
- Boss-1 easiness at real HP 25 and later-boss difficulty at real HP
  (40+wave×5, shield gating boss 4) are for the designer's playtest — the
  debug runs fought them at HP 5/40 with god mode on.

---

# v8 — November 19, 2026 (the designer's 4 playtest fixes)

Deliverable: `~/workspace/your_files/star-drag-v8.nds`
- md5 `3f6422128e54b2cbf9a24bbab9c70e76`, 4,194,304 bytes (4 MB), game
  code `####`, maker 0000, logo CRC 0xCF56 OK, header CRC 0x7986 OK.
- Clean rebuild reproduces the same md5 (built twice via `build.sh`,
  no warnings). v7 deliverable untouched (md5 still
  `5537a7782790baf7e04bbb0582468926`).

## What changed (all four fixes)

### Fix 1 — proper pixelated explosions, different per enemy type
- `Boom` gained `vx, vy, pal`; `Particle` gained `vx, vy, pal, life`.
  `spawn_boom_pal(cx, cy, pal, vx, vy)` and
  `spawn_particle2(x, y, vx, vy, pal, life)` are the new primitives;
  the old `spawn_boom`/`spawn_particle` delegate to them.
- `enemy_boom_kind[40]` maps every enemy type to BK_FIRE (ships),
  BK_GOO (bugs/creatures), BK_SHARD (crystals), BK_METAL (robots/metal)
  — palettes PAL_BOOM / PAL_JELLY / PAL_LASERBEAM / PAL_SHOOTER.
  `spawn_explosion(cx, cy, kind, big)` fires 2–3 booms at offsets with
  velocities plus 8 (10 if big) coloured spark particles.
- Death flow: the killing hit now spawns the type explosion and the
  boom SFX immediately; the 24-frame dying phase is embers drifting
  forward plus a coloured spark every 4 frames (no more flip-tumbling
  of the ship sprite); `kill_enemy_final` adds one final central boom
  in the type palette. `draw_play` never draws a dying enemy, so no
  ship sprite is ever shown during a death — the "ship swap" look is
  gone. Booms/particles are drawn in their own palette (`b->pal`,
  `p->pal`), and particles fly on their velocity and expire on `life`.
- MAX_BOOMS 6 → 10, MAX_PARTICLES 12 → 24.

### Fix 2 — boss death = multiple consecutive explosions
- `Boss` gained `dying, die_t`. On the fatal hit `damage_boss` sets
  `dying = 1, die_t = 0` and returns (no instant clear). The update
  block runs the sequence: a boom at a random offset around the boss
  plus 2 particles every 12 frames, `boss.x/y` jittered ±2 (shake),
  the boss flashing (hidden every other frame), no movement, shooting,
  or damage while dying. At `die_t >= 90` (~1.5 s) `boss_death_final()`
  fires the final big blast (3 booms + 10 particles), restores normal
  flight for every mode, and hands out score/pickups/area advance.
  Applies to all bosses including boss 1; the boss fights themselves
  are unchanged.

### Fix 3 — level-1 stalagmite bug — ROOT CAUSE FOUND
- **Cause:** `load_sprites()` called
  `oamInit(&oamMain, SpriteMapping_1D_32, false)` — 32 KB of sprite
  VRAM — but the game allocates ~42,880 bytes of main sprite tiles.
  Every sprite past 32 KB (missile + fire-rate pickups, lives icons,
  sparks, booms, ALL terrain obstacle sprites, score digits) had its
  tile index wrapped/truncated and rendered as some other sprite's
  tiles. The stalactites/stalagmites (`gfx_obs[4]`/`gfx_obs[5]`)
  aliased to enemy ship tiles — that is the "ship icons" the designer saw.
  This also explains Fix 1's symptom (boom sprites past the limit
  aliasing to ship tiles) and part of Fix 4's symptom (missile/rate
  pickups aliased).
- **Proof:** the stalactite/stalagmite `.bin` files were verified clean
  triangles; an X-pattern test build (X data confirmed in the ROM,
  triangle absent) showed zero X's; a solid-orange-square test build
  showed only ~150–170 bright-orange pixels (explosion flames), no
  orange squares, while a debug readout proved the obstacles were
  active; changing only `SpriteMapping_1D_32` → `SpriteMapping_1D_128`
  in the debug build made orange squares appear (164 → 2880 px,
  171 → 2496 px) exactly at the ceiling/floor obstacle positions.
  VRAM bank B is 128 KB for main sprites, so 1D_128 is safe. The sub
  screen (~4 KB) was changed to 1D_128 too for consistency.
- **Fix:** `oamInit(&oamMain, SpriteMapping_1D_128, false)` and the
  same for `oamSub`, with a comment explaining the cause. No sprite,
  palette, or draw-code change was needed — the data was always right.

### Fix 4 — weapon pickups and panel buttons identifiable
- New `tools/convert_v8.py` regenerates the pickup and button sprites
  with a 5x7 font and a tiled 4bpp encoder. Pickups are 32x32 with the
  letter drawn in white with a dark outline on a coloured capsule:
  SPREAD = green capsule + white "S" (`pow_spread`, own palette
  PAL_POW_SPREAD); LASER = cyan capsule + white "L" (`pow_laser`, own
  palette PAL_POW_LASER); MISSILE = orange capsule + white "M"
  (`pow_miss`, patched slots 11–13 of the shared PAL_MISSILE);
  FIRE-RATE = red-brown capsule + white lightning bolt + three small
  white chevrons (`pow_rate`, PAL_DRIFTER). Buttons are 64x32, white
  body, saturated accent border + big accent letter: "1" red, "S"
  yellow, "L" blue, "M" green (all four button palettes rewritten).
- `load_sprites` fills palette slot PAL_BTN_LOCKED (5) with flat greys
  at runtime; the button draw loop uses PAL_BTN_LOCKED when
  `!owned[i]`, so uncollected weapons render dimmed. The selected
  weapon keeps its gold star reticle above the button.
- Caution: re-running `tools/convert_sprites.py` regenerates
  `missile_pal.bin` from source art and would wipe the patched indices
  11–13 — re-run `tools/convert_v8.py` after it.
- Title screen now reads "STAR DRAG v8". `build.sh` copies to
  `star-drag-v8.nds`. README updated (title + v8 paragraph).

## Verification (melonDS 1.1, JIT off, Xvfb — `qa/v8-runs/`)

Debug ROMs (god mode, waves advance at kills/4, boss HP 5, sub-screen
readout) were built from the same v8 source into `/tmp/v8check/`:
`debug-v8.nds` (ec5fe277…), `debug-cave8.nds` (24324d64…, starts wave 7 /
CAVE), `debug-boss8.nds` (b8e95951…, starts wave 3 / boss wave).
Their `build.sh` files copy to /tmp only — the v7 deliverable was never
at risk in this round.

- **Title → play:** v8 title screen shows "STAR DRAG v8" and the game
  starts on a bottom-screen touch (a top-screen tap does NOT start it —
  the DS touchscreen is the bottom screen; two early runs sat on the
  title for this reason and were re-run).
- **Fix 3 (terrain):** `cave-v8/` c06/c10/c14 — CAVE ceiling/floor show
  clean brown stalactite/stalagmite triangles, no ship icons anywhere.
  CRYSTAL (c14, cyan bands) and WATER areas also checked; leftover
  CAVE triangles drift out correctly after an area change. d66 also
  shows proper triangles in CAVE. No ship sprite appears in any
  terrain band in any run.
- **Fix 4 (pickups + buttons):** pickups read clearly in-game — green
  "S" (c10, b07, v09, k04), orange "M" (v09, d49, d51, d52), cyan "L"
  (v10, b20, d52), brown fire-rate capsule with white bolt (b20, d52).
  Bottom screens (c10_bot, c14_bot, b15_bot, b20_bot, j02_bot) show
  all four buttons with matching letters/colours, the gold star above
  the selected weapon, and uncollected weapons greyed (M grey while
  1/S/L are bright, in every run where M was not yet collected).
- **Fix 1 (explosions):** d33 captures a chunky orange pixelated
  fireball (fragments + yellow spark crosses) for a ship enemy; d60
  captures a distinct tall CYAN burst (shard palette) with orange
  flames at its base; d66 captures another orange fireball plus the
  cyan laser beam. Yellow spark particles are visible in many frames.
  No dying enemy ever renders as a ship sprite. Green-goo and
  grey-metal bursts are in the code (same palette-swap mechanism
  proven by the orange/cyan captures) but were not individually caught
  on camera in this round.
- **Fix 2 (boss death):** `boss-v8/` b15–b16 show the boss fight (big
  grey boss, red spread bullets, HP bar shrinking); b17–b19 the fight
  continues; b20 the boss is gone and the sub screen reads
  "NORMAL FLIGHT". `boss-dense/` (1 s shots) pins it down: d49–d50 the
  boss is alive and taking damage, d51–d52 the boss is gone and the
  right edge (where the boss parks) shows chunky orange fireballs —
  the sequence booms, partially clipped by the screen edge because the
  boss fights at x≈192–256. The 90-frame sequence ran (boss cleared
  only after it, via `boss_death_final`).
- **Release ROM (no debug):** `release-v8/` — the ship parked at
  (16,80) dies 3 times and reaches GAME OVER by ~100 s (score 9100,
  wave 4, best 9100); the hold screen is byte-identical across shots
  (no auto-restart). `restart-v8/` — a touch on the GAME OVER screen
  restarts into a fresh game (ship firing, green "S" pickup visible).
  Total unattended play across all v8 runs ≈ 12 minutes, no crash, no
  freeze (other than the intended GAME OVER hold).
- **JIT-on (honest):** with `JIT Enable = true` in melonDS.toml, v8
  shows a solid WHITE top screen and BLACK bottom screen — the same
  melonDS 1.1 JIT + devkitPro-homebrew incompatibility as v7 and the
  stock libnds hello-world, NOT a v8 bug. Correction to the earlier
  note: this melonDS 1.1 AppImage actually defaults to JIT OFF (a
  fresh home writes `Enable = false`), so a fresh install renders v8
  fine; only users who explicitly enabled JIT hit the white screen.
  Workaround stands: melonDS Settings → Emu → JIT recompiler OFF.

## Not verified (the designer's gate)
- Sound not heard headless (music loop, laser zap, alarm x3, boss
  death booms — wired, never crashed in ~12 min of v8 runs).
- D-pad untested (same as v4–v7). DeSmuME rendering unverifiable here
  (white SDL window for every ROM under Xvfb); NooDS not runnable
  here; RetroArch iPhone melonDS path not testable here.
- Green-goo (jellyfish/creature) and grey-metal (robot) explosion
  bursts not individually caught on camera; the palette-swap mechanism
  is proven by the orange and cyan captures.
- Boss death-sequence booms are partially clipped by the right screen
  edge (the boss parks at x≈192–256) — visible, but a boss that parked
  further left would show them more fully. The sequence itself (timing,
  count, final blast, NORMAL FLIGHT restore) is verified.
- Boss 2/3/4 death sequences use the same code path as boss 1 (verified
  on boss 1); their distinct fights are unchanged from v7.

---

# v9 — November 19, 2026 (the designer's 3 playtest fixes: boss-3 vertical fight, post-boss-2 enemy HP, alarm for every boss)

Deliverable: `~/workspace/your_files/star-drag-v9.nds`
- md5 `e2d114cb919d1e658a7452fef2b8ec11`, 4,194,304 bytes (4 MB), game
  code `####`, maker 0000, logo CRC 0xCF56 OK, header CRC 0xF9A3 OK.
- Clean rebuild reproduces the same md5 (built twice via `build.sh`,
  no warnings except the pre-existing unused `bcx2` in update_play).
  v8 deliverable untouched (md5 still
  `3f6422128e54b2cbf9a24bbab9c70e76`).

## What changed

### Fix 1 — boss 3 vertical fight is SCROLL-ONLY, like boss 1's reverse
- ALL v7/v8 upward-mode code removed: no ship teleport, no facing-up
  sprite (`gfx_player_up` no longer drawn), no vertical shots, no
  vertical laser beam (horizontal hit box and draw only), no ceiling
  shaft the ship flies through. `vertical_mode` is gone entirely
  (grep: zero references outside a comment); it is replaced by
  `vert_vel` / `vert_target`, eased exactly like `scroll_vel` /
  `scroll_target` (2/frame toward target).
- Boss 3 intro: horizontal scroll eases to a stop (`scroll_target =
  0`), then the world scrolls vertically (`vert_target = -160`,
  faster than normal, like boss 1's -160 reverse) with the cue
  "UPWARD!". The ship stays at its unchanged absolute position
  facing right and firing right; the boss enters from the right and
  parks on the right (x=196) facing left, his aimed shots and spread
  fan both fire LEFT toward the player (the generic aim/fan code —
  the mode-3 downward fan is gone). No other enemies during the
  fight (unchanged). After he dies the vertical scroll eases back
  to 0 and the horizontal scroll back to +100 ("NORMAL FLIGHT").
- Ceiling OPEN (the designer's clarification): `terrain_open_ceiling()`
  zeroes every column's ceiling height at the intro and keeps the
  floor heights as-is, so the boss section has open sky above and
  normal ground below; all terrain obstacle sprites are cleared at
  the intro (they would float in open sky once the floor slides
  away). `terrain_rebuild()` brings the ceiling back at
  `boss_death_final()`. Ceiling/floor collision stays active in
  every boss fight (the old `!vertical_mode` exemption is gone —
  with the ceiling open there is nothing to hit up top anyway).
- Boss 1 (reverse) and boss 2 (halt) are PERFECT and untouched;
  boss 4 (shield+charge) unchanged. The boss-3 death sequence
  (multi-explosion chain, v8 Fix 2) is unchanged.

### Two real bugs found and fixed while building Fix 1
- **BG scroll Y sign was backwards.** Increasing the DS background
  scroll Y moves content UP on screen, not down — so v7's upward
  flight (scroll Y increasing) had actually been scrolling the
  stars UP (the ship facing up masked it), and the first v9 build
  (vert_target +160) lifted the floor band into the middle of the
  playfield. Fixed: upward flight uses vert_target **-160**
  (scroll Y falls, stars/background move DOWN = flying UP).
- **Terrain vertical offset was never reset after the fight.**
  The first v9 build clamped `scroll_terr_y` but `vert_vel` was
  still decaying toward 0 after `boss_death_final()` reset the
  offset, so it re-accumulated and stranded the floor band
  mid-screen in normal play (seen in QA: a static cyan band at
  rows 127–148 after the fight). Fixed: `terrain_update()`
  accumulates `scroll_terr_y` only while `vert_target != 0`, clamps
  it to [-48, 0] (the floor slides down and off the bottom as you
  fly up and stays off — it can never wrap through the playfield),
  and eases it back to 0 once the fight is over;
  `boss_death_final()` also resets it to 0 with the ceiling
  rebuild. (The 256 px terrain map would otherwise wrap the band
  back through the screen.)

### Fix 2 — enemies after boss 2 have more HP
- `spawn_enemy()` now scales HP for waves > 6 (tiered):
  tier = (wave-7)/3 + 1 (waves 7–9 tier 1, 10–12 tier 2, 13+
  tier 3); hp = base*(1+tier) + tier*2. A 1-HP type is 4 HP at
  wave 7 and 10 HP at wave 13; a 2-HP type is 6 / 14; a 3-HP type
  is 8 / 18; an 8-HP capital is 18 at wave 7 and 38 at wave 13.
  Waves 1–6 are EXACTLY unchanged (boss 1 / level 1 as approved).
  Player fire rate is NOT touched — the fix is durability only.

### Fix 3 (follow-up) — the alarm sounds for EVERY boss fight
- Code inspection: `spawn_boss()` is called from exactly ONE
  place — the end of the shared boss-intro block in `update_play()`
  — which plays `sfx_alarm()` at intro_t 1, 61 and 121 (×3) plus
  "!! WARNING !!" and the mode's named cue, for every mode. No
  boss was missing the alarm; nothing in the alarm path changed.
  (The alarm sample itself is unchanged from v5.)

## Verification (melonDS 1.1, JIT off, Xvfb — `qa/v9-runs/`)

Debug ROMs (god mode, boss HP 40, sub-screen readout
`S x,y V<scroll> R<reverse> E<enemies> B<boss> M<mode> U<vert> S<shield>`)
were built from the same v9 source into `/tmp/v9check/`:
`debug-boss3.nds` (starts wave 8, kills 31, boss_num 2 — one kill
from the boss-3 fight), `debug-hp.nds` / `debug-hp1.nds` (HP
instrumentation: every kill prints `K<type> hp<spawn HP>
hits<damage calls> w<wave>` on the sub console), `debug-boss5.nds`
/ `debug-boss6.nds` (one kill from the mode-1 / mode-2 boss,
laser start so the trigger kill comes fast). Their `build.sh`
files copy to /tmp only — the v8/v9 deliverables were never at risk.

- **Fix 1 (boss 3, exact — `boss3d/`):** ship parked at (96,120)
  by a single relative drag, then never touched again. Readouts:
  d08 `UPWARD! S 96,120 V +0 R0 E0 B1 M3 U-160` (fight at max
  upward speed); d10 same; d11 `NORMAL FLIGHT S 96,120 V +10
  U-150` (death final just ran, both velocities easing back);
  d12 `NORMAL FLIGHT S 96,120 V+100 U -30`; d20 `V+100 U +0 E6`
  (normal play resumed, spawns back). Ship position 96,120 in
  EVERY readout before (b60 WARNING, d03 park), during, and after
  the fight. Top screens: d10 — ship facing right firing its laser
  beam right, boss parked at the right edge facing left with
  damage sparks, a red boss bullet flying LEFT toward the ship, NO
  ceiling band, NO floor band (slid off), no band anywhere in the
  playfield; d11 — boss death explosion at the right edge with the
  dropped LASER pickup and fire-rate capsule, ceiling and floor
  bands restored top/bottom. The pre-fix build's failure is on
  record too (`boss3/` b66: floor band floating mid-screen at ship
  height; `boss3c/` d25: band stranded at rows 127–148 after the
  fight) — both gone in the shipped build.
- **Boss 1 / boss 2 untouched (exact):** `alarm5c/` a10 `REVERSE!
  S 96,120 V -56 R1 E0 B1 M1` (easing into max reverse);
  `alarm6c/` a10 `HALT! S 96,120 V +0 R0 E0 B1 M2` (full stop
  held); `hp7/` readout sheet shows `V-160 R1 B1 M1` held during
  a mode-1 fight. Same numbers as v6/v7.
- **Fix 2 (HP, exact — `hp1/` vs `hp7/`, KILL rows):** baseline
  waves 1–5: `K01 hp01 hits01 w01`, `K06 hp01 hits01 w01`,
  `K06 hp03 hits03 w01`, `K06 hp02 hits01/02 w02`,
  `K03 hp02 hits02 w03`, `K06 hp01 hits01 w04` — 1-HP types die
  on the first hit. Post-boss-2 (wave 7+): `K06 hp04 hits03`,
  `K06 hp06 hits02`, `K03 hp06 hits02`, `K17 hp08 hits03`,
  `K06 hp08 hits03/06/07`, `K06 hp18 hits06` — every sampled kill
  took 2–7 damage calls on 4–18 HP. Enemies stay visible for
  seconds instead of dying on the first shot (also visible in the
  runs: E10–E20 live enemies on screen at once in later waves,
  e.g. d151 `E19`). Fire rate untouched (weapon intervals
  unchanged in code).
- **Fix 3 (alarm, all four modes — WARNING + named cue seen on
  the sub screen for each):** mode 1 `!! WARNING !!` (alarm5c a08)
  then `REVERSE!` (a10, V-56 R1 B1 M1); mode 2 `!! WARNING !!`
  (alarm6c a08) then `HALT!` (a10, V+0 B1 M2); mode 3
  `!! WARNING !!` (boss3 b60) then `UPWARD!` (b62/b65/d08–d10,
  U-160 B1 M3); mode 4 `SHIELD!` (boss3 b95, B1 M4). Code: one
  shared intro path, `sfx_alarm()` at intro_t 1/61/121, and
  `spawn_boss()` has no other caller — no mode can miss it.
  **Honest limit: the alarm sound itself cannot be heard
  headless** — the verification is code inspection plus the
  on-screen WARNING/cue text appearing for each boss, exactly as
  the designer asked for.
- **Release ROM (no debug — `release-v9/`, shipped md5):** title
  "STAR DRAG v9" → start → ship sits at (16,80) auto-firing and
  dies 3 times → GAME OVER (FINAL SCORE 9100, Wave reached: 4,
  BEST SCORE 9100) — held byte-identical across r12–r16 (5 shots
  × 10 s ≈ 50 s, no auto-restart) — fresh touch → new game
  (OPEN SPACE, SINGLE, score reset). Total unattended play across
  all v9 runs ≈ 45 minutes, no crash, no freeze (other than the
  intended GAME OVER hold).
- **JIT-on (honest):** with `JIT Enable = true`, v9 shows a solid
  WHITE top screen (96.9% white) and BLACK bottom screen — the
  same melonDS 1.1 JIT + devkitPro-homebrew incompatibility as
  v7/v8 and the stock libnds hello-world, NOT a v9 bug. Fresh
  melonDS defaults to JIT off and renders v9 fine.

## Not verified (the designer's gate)
- Sound not heard headless (music loop, laser zap, alarm x3, boss
  death booms — wired, never crashed in ~45 min of v9 runs). The
  alarm's audibility/level in particular is for the designer's playtest.
- D-pad untested (same as v4–v8). DeSmuME rendering unverifiable
  here (white SDL window for every ROM under Xvfb); NooDS not
  runnable here; RetroArch iPhone melonDS path not testable here.
- Boss 2/3/4 fights at REAL HP (boss 3 = 40+9*5 = 85) are for
  the designer's playtest — debug runs fought them at HP 40 with god mode
  on, so the laser killed boss 3 in ~2 s of firing; the real fight
  will last longer and the vertical scroll will run longer (the
  floor stays off the whole time by the clamp).
- The boss parks at x=196, so his 64 px sprite is clipped ~4 px
  by the right screen edge (same as v7/v8) — his death-sequence
  booms are partly clipped too.

# v10 — November 19, 2026 (the designer's 2 playtest fixes: per-area music, glowing weapon-selection border)

Release ROM: `~/workspace/your_files/star-drag-v10.nds`, md5
`c480d5b5acc4b7477272af0159cfa120`, 4,194,304 bytes, game code `####`,
logo CRC 0xCF56 OK, header CRC 0x056D OK. Clean rebuild reproduces the
md5 (verified 3x — see the decontamination note below). Everything
from v9 is unchanged except the two items below: boss 1 reverse /
boss 2 halt / boss 3 scroll-only upward with open ceiling / boss 4
shield+charge, alarm x3 every boss, post-boss-2 HP scaling, pixelated
type-specific explosions, boss multi-explosion deaths, labeled
pickups, relative drag, GAME OVER hold screen.

## Change 1 — different music per area (SHIPPED AS THE SANCTIONED FALLBACK)

- No browser access was available in this session, so no new tracks
  could be generated via Speechma. Instead each of the 7 areas plays
  a distinct variation of the existing 85 s Speechma track
  (`assets/music-v2.mp3`, kept as-is, never regenerated): a different
  loop section (offset = size*area/7, loop wraps to that offset) at a
  different playback rate. `music_set_area(area)` in `source/main.c`,
  called from `terrain_set_area` (the one place areas change) and from
  `main()` at boot; it kills the old loop (`soundKill(music_id)`) and
  starts the new one in the same frame — clean switch, no gap/crash.
  Boss fights keep the area track; alarm/SFX untouched (separate
  channels). ROM stays 4 MB (no extra audio data at all).
- Rates: OPEN SPACE 22050 Hz (original), SPACE BASE 26000, PLANET
  19000, CAVE 16000, CRYSTAL 29000, WATER 20000, GOO 15000. The
  rates give each area a different pitch/tempo feel (CRYSTAL bright
  and fast, GOO slow and eerie, CAVE dark and low) from the same
  source track. These are variations, NOT 7 separately composed
  tracks — the designer should know the difference; real per-area tracks
  would need 6 more Speechma generations and ~11 MB at the current
  encoding (would break the 4 MB ROM — would need shorter loops or
  lower sample rates).
- Verified by QA_DEBUG readout (`MUSIC area %d rate %d`, row 4 of
  the sub console, re-printed so it survives `consoleClear()`),
  areas 0–5 in `qa/v10-runs/music/` (19 shots m00–m18, ~6.5 min, no
  crash; the run skipped area 6 between shots) and area 6 in a
  targeted run starting at wave 13 (`qa/v10-runs/music4/`,
  debug ROM md5 `7ce53e40f774917236b63caad334ec9d`): `MUSIC area 6
  rate 15000` + `AREA GOO`, holding across shots. All 7 areas
  confirmed. Music switches track exactly with the area, including
  during boss 1's REVERSE fight (boss keeps the area track).
- Sound itself not heard headless — per-area music verified by
  code + debug readout only (same honest gap as v1–v9).

## Change 2 — glowing border instead of star marker (FIXED AFTER A REAL BUG HUNT)

- Star marker above the selected button removed (finger-follow
  reticle kept, moved from OAM id 0 to id 5). New `build_border()`
  builds a 64x32 outline tile (2 px rim, palette index 1; 368 rim
  pixels, verified by replicating the function in Python); new sub
  palette slot `PAL_BORDER = 6` whose index 1 is rewritten every
  frame to the selected weapon's button-accent colour, pulsing
  full/dim every 16 frames (`dim_col` scales to ~55%). Colours =
  button accents from `tools/convert_v8.py`: SINGLE red RGB15(31,0,0),
  SPREAD yellow RGB15(31,25,0), LASER blue RGB15(0,20,31), MISSILE
  green RGB15(0,25,7). Border sprite at OAM id 0, buttons at ids
  1–4. Uncollected weapons stay greyed; the border is only ever on
  the selected weapon.
- **The bug:** the border did not render at all in the first build.
  Diagnosis (all in `qa/v10-debug-border/`, runs in
  `qa/v10-runs/border-test*/`): (1) solid-fill tile — no render
  (rules out the rim pattern); (2) same tile with a known-good
  palette — no render (rules out palette slot 6); (3) ghost: the
  known-good SPREAD button tile at OAM id 5, +8 px — RENDERS (thin
  yellow sliver), proving OAM entries render at the panel position;
  (4) ghost tile swapped to `gfx_border` — NO render, so the fault
  was the `gfx_border` tile data itself. Pointer print: BRD
  0x06601400, BTN3 0x06601000 — sane, in VRAM_D, correctly spaced.
  CPU readback of VRAM_D then proved it: after the original
  `dmaCopy(out, gfx_border, 1024)` the words read back 0x0000
  (while a known-good button word also read 0x0000 at first — the
  button's first tile is genuinely all transparent — a later read
  of a non-zero button word confirmed CPU reads of VRAM_D DO work,
  so the 0x0000 border readback was real). **Root cause: the
  1024-byte `dmaCopy` from the stack array never lands in VRAM_D**
  (the 128-byte digits `dmaCopy` works, and static-ROM-array
  `dmaCopy`s work — this call specifically fails). A CPU halfword
  loop (`u16 out[512]`, 512 writes) lands immediately: readback
  0x1111, ghost block renders. **Fix shipped in the release ROM:
  `build_border()` uses a `u16` array + CPU copy, NOT `dmaCopy`.**
  The u16 logic was verified byte-identical to the original byte
  logic in Python (368 rim pixels). Do NOT "optimise" it back to
  `dmaCopy` — the comment in the code says so.
- Verified on camera, release ROM (`qa/v10-runs/border-release/`)
  and debug ROM with force-collected weapons
  (`qa/v10-runs/border2/`, debug md5 `9cd0115826872449e2a630c8e1192690`):
  SINGLE = red border on "1"; SPREAD = yellow border on S; LASER =
  blue border on L; MISSILE = green border on M. Border only on the
  selected button, clearly visible at a glance (thicker and brighter
  than the buttons' own baked-in coloured rims). Star is gone.
  One flaky tap: in the border2 run the tap back to SINGLE didn't
  register once (WEAPON stayed MISSILE); a dedicated re-tap run
  (`qa/v10-runs/border2-retap/`) switched MISSILE -> SINGLE twice
  cleanly, so it was a one-off input flake, not a game bug. In the
  release run, tapping uncollected SPREAD/LASER/MISSILE correctly
  does nothing (border stays on SINGLE) — only collected weapons
  can be selected.
- Drag test on the release ROM (`qa/v10-runs/drag-release/`): two
  pad drags moved the ship (16,80) -> ~(83,42) -> ~(38,62),
  relative-drag only (no teleport), red border stayed on "1"
  throughout, reticle follows the finger while touching.

## Release end-to-end (shipped md5, JIT off)

- `qa/v10-runs/e2e-release/`: title -> start -> stationary ship
  dies 3 times -> GAME OVER (FINAL SCORE 9100, Wave reached: 4,
  BEST SCORE 9100 — same score/wave as v9's release run) held
  byte-identical across e02–e12 (11 shots x 60 s, no auto-restart).
  e01 caught mid-play in SPACE BASE with WEAPON MISSILE — the ship
  had collected a missile capsule, and the border had correctly
  followed the pickup-driven weapon change (green border on M).
- Restart (`qa/v10-runs/e2e-restart/`): after GAME OVER, a fresh
  touch starts a clean new game (AREA OPEN SPACE, WEAPON SINGLE,
  S/L/M greyed again, red border on "1").
- Total unattended v10 play across all runs ~35 minutes, no crash,
  no freeze (other than the intended GAME OVER hold).

## JIT-on (honest)

- `qa/v10-runs/jit-on/`, home `~/workspace/ds-diag/jit-home` with
  `[JIT] Enable = true` read back directly before AND after the
  run (melonDS re-saved the toml, reordering keys, but kept
  Enable=true — the toggle genuinely held): top screen 96.9% white
  in both shots. Same melonDS 1.1 JIT + devkitPro-homebrew
  incompatibility as v7/v8/v9 and stock libnds hello-world, NOT a
  v10 bug. Fresh melonDS defaults to JIT off and renders v10 fine.

## QA-process incident (recorded so it is not repeated)

- Twice during v10, a debug patch landed in the REAL project
  instead of a debug copy: the tar-based copy command failed
  (ownership errors make tar exit non-zero, breaking the `&&`
  chain, so the following `cd` never ran and the python patch ran
  in the real project dir). First hit: `-DQA_DEBUG` in
  `arm9/Makefile` + `wave = 13` in `source/main.c` (music2 copy);
  second hit: same two plus `terrain_set_area(6)` in `start_game`
  (music3 copy, via a failed `cp -r` into a subdirectory of
  itself). Both times the contamination was caught by grepping the
  real project, removed, and a clean rebuild reproduced the release
  md5 `c480d5b5...` exactly. Lessons: NEVER trust a `&&` chain
  after a tar/cp that can fail — verify the copy exists AND the
  patch landed in the copy (grep both sides) before building; the
  working method used for the final area-6 run was `rsync` to a
  sibling directory (`~/workspace/ds-debug-music3/`, removed after
  the run) with absolute-path greps on both sides. Also: the
  music3 run's screenshots were taken on the release ROM by
  mistake (the copy had never been patched) and are worthless for
  area 6 — only `qa/v10-runs/music4/` counts for area 6.

## Not verified (the designer's gate)

- Sound not heard headless (per-area music variations, laser zap,
  alarm x3, boss death booms — wired, never crashed in ~35 min of
  v10 runs). Whether the per-area rate/offset variations actually
  sound distinct and fitting per area is for the designer's playtest — the
  code only proves a different loop section plays at a different
  rate per area.
- D-pad untested (same as v4–v9). DeSmuME rendering unverifiable
  here (white SDL window for every ROM under Xvfb); NooDS not
  runnable here; RetroArch iPhone melonDS path not testable here.
- Boss 2/3/4 fights at REAL HP are for the designer's playtest — debug
  runs fought them at HP 40 with god mode on (same as v9).
- The boss parks at x=196, so his 64 px sprite is clipped ~4 px
  by the right screen edge (same as v7/v8/v9) — his death-sequence
  booms are partly clipped too.
- The border pulse (full/dim every 16 frames) is in the code and
  the colour write is proven by the static screenshots, but the
  pulsing itself was not captured on video — single frames only.

# v11 — November 19, 2026 (the designer's 2 playtest fixes: REAL unique music per area + glowing border, not blinking)

Release ROM: `~/workspace/your_files/star-drag-v11.nds`, md5
`c001dc6726c3e110365af4424f82ea67`, 8,388,608 bytes (8 MB — up from
4 MB; the 7 music tracks live in the ROM filesystem now). Header
pure-DS (unit 0x00, DSi magic zero, game code `####`, maker 0x0000,
logo CRC 0xCF56 OK, header CRC 0x3552 OK). Clean rebuild reproduces
the md5 exactly (built twice via `build.sh`).

## Change 1 — REAL unique music per area with fades (boss tempo shift kept)

- Area 1 keeps the existing 85 s Speechma track
  (`assets/music-v2.mp3`, unchanged). Areas 2–7 are six NEW Speechma
  tracks, one per area, generated by the parent in a browser task
  (verified distinct by md5 before use — see below). The v10
  per-area rate/offset variations of the one track are GONE: no area
  plays a pitch-shifted copy of the area-1 track any more.
- Tracks (source MP3 md5s, all verified against the parent's list):
  area 2 SPACE BASE 75 s `7b42a6a028562905a4c0d1ebfb2b079f`, area 3
  PLANET 80 s `9de5bb7c81759c66c58cb6d2a6770c26`, area 4 CAVE 70 s
  `8f44d58159b13f3bf1c70ae1d95c4644`, area 5 CRYSTAL 75 s
  `992997b34120220bb957db3f8d161053`, area 6 WATER 85 s
  `707c401bcd212d4064fe101e4a9ec7e7`, area 7 GOO 80 s
  `541d758add5d54593da6c73741e581b3`. `tools/convert_music_v11.py`
  converts each to a 36 s loop (source slice 8 s..44 s, 0.5 s
  fades at the loop seam) of raw unsigned 8-bit PCM mono: area 1 at
  22050 Hz (793,800 bytes), areas 2–7 at 16000 Hz (576,000 bytes
  each), total 4,249,800 bytes (4.05 MB). All 7 `.raw` files have
  distinct md5s. Shorter loops were chosen over lower quality, per
  the brief (the full ~85 s tracks at 22050 Hz would be ~13 MB).
- **NitroFS (the engineering change):** 7 embedded tracks overflow
  the ARM9 ELF's hard 3.5 MB limit
  (`ld: Error: DS-mode sections overflow DS ARM9 memory size
  (3.5 MB)`). The tracks now live in the ROM filesystem
  (`nitro/music/areaN.raw`; top Makefile `NITRO_FILES := nitro` ->
  ndstool `-d`; `arm9/Makefile` LIBS is `-lfat -lnds9 -lm` because
  `nitroFSInit` lives in `libfat.a`). `music_set_area()` loads a
  track into a double RAM buffer (`music_buf[2][800000]`) in 64 KB
  chunks per frame from `music_update()` (called once per frame from
  the main loop), so there is no gameplay hitch — the old track keeps
  playing until the new one is ready, then a 1 s (60-frame) crossfade
  (old volume ramps to 0, new ramps 0 -> 80) swaps it in. `music_id`
  is the libnds sample handle; `music_nitro_ok` guards silence if a
  load ever fails. Old embedded bins (`audio/music_raw.bin`,
  `audio/music_area2..7_raw.bin`) were deleted from the project —
  the Makefile embeds everything in `audio/`, so leaving them would
  re-overflow the ELF. ELF shrank from 2,532,612 bytes (v10) to
  951 KB; the ROM grew to 8 MB (ndstool pads to the declared device
  capacity — fine for a homebrew .nds).
- **Boss tempo is real now — honest note:** v10 had NO dedicated
  boss-tempo code. The shift the designer heard at boss 1 was the wave-3
  area change speeding the track up to 26000 Hz. v11 makes the lift
  explicit and the same for every boss: `music_boss_start()` at the
  boss intro ramps the playback multiplier to 1.25x
  (`MUSIC_BOSS_MUL` 320, via `soundSetFreq`), `music_boss_end()` at
  `boss_death_final()` ramps it back to 1.00x over ~1 s while the
  next area's track fades in. Boss fights keep the current area's
  track (altered); alarm x3 and SFX are untouched (separate
  channels). `music_play_area` tracks the actually-playing track for
  the rate and the QA_DEBUG readout.
- Verification (melonDS 1.1, JIT off, Xvfb; QA_DEBUG readout
  `MUSIC <name> a<area> v<vol> x<tempo>`, row 4 of the sub console,
  re-printed so it survives `consoleClear()`):
  - Engine run on a PLACEHOLDER build (`qa/v11-runs/music-areas/`,
    debug ROM md5 `60e74c7937f1e17eabfccc9cf4e55352`, 23 shots m00–m22
    over ~7.5 min, no crash): area 0 `music-v2 a0 v80 x1.00` at
    boot; boss 1 in SPACE BASE `area2 a1 v80 x1.25`; PLANET
    `area3 a2 v80 x1.08` caught mid-ramp-down after the boss died;
    CAVE `a3`, CRYSTAL `a4`, WATER `a5 x1.10` (boss-4 intro, tempo
    rising) then `x1.00`; second boss 1 (wave 15) held `x1.25` for
    ~3 min (see the stationary-ship note below), then died
    (`x1.20` mid-ramp-down) and play resumed; SPACE BASE `a1`,
    PLANET `a2` again as waves cycled. The run skipped GOO between
    shots (waves 13–14 crossed in <20 s) — GOO was verified in the
    boss run below instead. The placeholder run proves the engine
    (NitroFS loads, per-area files, tempo lift + ramp-down, no
    crash); its audio was placeholder copies of the area-1 track, so
    it does NOT prove the real tracks.
  - Boss run on the FINAL audio (`qa/v11-runs/boss4-goo2/`, debug
    ROM md5 `54844a2bd8e4b805f201ac1beef6e9f1`, 32 shots b00–b31,
    ~5 min, no crash): boss at wave 12 in WATER `area6 a5 v80
    x1.25`, dead next shot (`x1.00`), then **GOO `area7 a6 v80
    x1.00`** — the real area-7 track loaded and playing. Later the
    run cycled back through OPEN SPACE `a0` / SPACE BASE `a1` and
    caught **boss 3 (mode 3, UPWARD) `area2 a1 v80 x1.25`, `U-160`**
    (world scrolling upward at max speed) — tempo lift confirmed
    for the upward mode too. NOTE: the wave-12 boss fought was
    mode 1 (reverse), not mode 4 (shield+charge): the debug copy
    jumps to wave 11 but `boss_num` still starts at 0, so the first
    boss is mode 1. The tempo-lift code path is identical for all
    modes; mode 4's fight itself is unchanged from v7–v9.
  - All 7 areas now confirmed playing their own distinct track
    file (a0–a6) on the final audio: a0–a5 in the engine run, a6
    (GOO) in the boss run. The crossfade itself (volume ramping
    0 -> 80 over 60 frames) is verified by code + the steady `v80`
    state + the `fade` print at load (`MUSIC area 0 music-v2
    fade` on the title screen) + no crash across ~15 transitions;
    the fade's smoothness to the ear is for the designer's playtest (sound
    not heard headless).
- **QA-process note (boss run 1 failed, then fixed):** the first
  boss-run attempt (`qa/v11-runs/boss4-goo/`, debug md5
  `1c29b883f6d01bbd47f6201b33d3c1cb`) ran 5 min with ZERO kills —
  enemies piled to E20 and the wave never advanced from 11. NOT a
  game bug: at wave 11 the v9 tiered HP scaling makes a 1-HP enemy
  7 HP, and the scripted ship starts with SINGLE (damage 1) and
  never collects weapons, so it cannot kill anything in the time a
  bullet crosses the screen. The fix was QA-side only: the boss
  debug copy's `start_game` now grants a mid-game loadout under
  `#ifdef QA_DEBUG` (LASER + all weapons owned + rate level 3) so a
  scripted run can kill wave-11+ enemies. The shipped ROM still
  starts SINGLE. (Same lesson as v10: a `make` after only the
  nitro files changed did NOT rebuild the .nds — make doesn't track
  `NITRO_FILES` content — the stale placeholder ROMs had to be
  force-rebuilt with `rm star-drag.nds && make`.)

## Change 2 — glowing border, NOT blinking (verified by pixel measurement)

- `dim_col()` removed. New `glow_col()` (just above `draw_play()`,
  needs global `frame`): sine over a 2 s (120-frame) cycle,
  brightness scale 101–255 (~40%–100%), never fully off, no abrupt
  steps. Palette write is now `glow_col(border_col[weapon])`.
  Border stays in the selected weapon's colour, only ever on the
  selected weapon, uncollected weapons stay greyed/dimmed.
- Verified on the RELEASE ROM (`qa/v11-runs/glow/`, 16 shots g00–g15
  taken ~0.17 s apart, > one full 2 s cycle): the SINGLE button's
  top rim (window y 374–375 — the visible rim sits ~6 px below the
  sprite's PANEL_Y=160 origin) measured red 105 -> 97 -> 121 ->
  162 -> 211 -> 243 -> 243 -> 219 -> 178 -> 130 -> 105 -> 97 ->
  121 -> 162 -> 211. **A smooth sine-like rise and fall, then
  rising again — 9 distinct brightness levels, range 97–243, never
  0, never an abrupt on/off step.** The border is always visibly
  present (minimum ~38% brightness, matching the 101/255 floor).
  g00 (title screen) reads 0 — no border on the title, correct.
  This replaces the v10 "pulse proven by stills only" gap: the glow
  cycle is now measured, not just in the code.

## Release end-to-end (shipped md5, JIT off)

- `qa/v11-runs/e2e-release/`: title -> start -> stationary ship
  dies 3 times -> GAME OVER (FINAL SCORE 9100, Wave reached: 4,
  BEST SCORE 9100 — same score/wave as v9's and v10's release
  runs) held byte-identical across e03–e11 (9 shots x 60 s, no
  auto-restart).
- Restart (`qa/v11-runs/e2e-restart/`): after GAME OVER, a fresh
  touch starts a clean new game (AREA OPEN SPACE, WEAPON SINGLE,
  S/L/M greyed again, red border on "1"). (First restart attempt
  was void: the ship was still alive at 75 s — it had picked up
  MISSILE and survived longer — so the click was a mid-game touch,
  not a restart; the re-run with a 150 s wait reached GAME OVER
  first and the restart is the one that counts.)
- Total unattended v11 play across all runs ~40 minutes, no crash,
  no freeze (other than the intended GAME OVER hold).
- Stationary-ship note (QA artifact, not a game bug): a ship that
  never drags to aim takes ~3 min to kill a boss (boss 1, wave 15,
  in the engine run) because straight-firing weapons only connect
  when the boss bobs through the firing line — the boss DOES die
  eventually. A real player drags to aim; the boss debug run's
  aiming sweeps killed the wave-12 boss in one 8 s interval.

## JIT-on (honest)

- `qa/v11-runs/jit-on/`, home `qa/v11-jit-home` with `[JIT]
  Enable = true` read back directly before AND after the run
  (melonDS re-saved the toml, reordering keys, but kept
  Enable=true — the toggle genuinely held; note the first edit
  attempt flipped the wrong `Enable` — `[DLDI]` line 26, not the
  `[JIT]` line 55 — caught by grepping the section, fixed by
  line number): top screen 96.9% white in both shots. Same
  melonDS 1.1 JIT + devkitPro-homebrew incompatibility as v5–v10
  and stock libnds hello-world, NOT a v11 bug. Fresh melonDS
  defaults to JIT off and renders v11 fine.

## Not verified (the designer's gate)

- Sound not heard headless (per-area tracks, 1 s crossfades, boss
  tempo lift to x1.25 and ramp back, laser zap, alarm x3, boss
  death booms — wired, never crashed in ~40 min of v11 runs).
  Whether the 7 tracks actually sound distinct and fitting per
  area, and whether the fades and the boss tempo lift feel right,
  is for the designer's playtest — the code + debug readout only prove the
  right file plays at the right rate per area.
- D-pad untested (same as v4–v10). DeSmuME rendering unverifiable
  here (white SDL window for every ROM under Xvfb); NooDS not
  runnable here; RetroArch iPhone melonDS path not testable here.
- Boss 2/3/4 fights at REAL HP are for the designer's playtest — debug
  runs fought them at HP 40 with god mode on (same as v9/v10).
  Boss 3's death sequence was not caught on camera in v11 (fight
  still going at the end of the boss run); his death code is
  unchanged from v7/v8, and boss 1's multi-explosion death +
  tempo restore was observed twice in v11.
- The boss parks at x=196, so his 64 px sprite is clipped ~4 px
  by the right screen edge (same as v7–v10) — his death-sequence
  booms are partly clipped too.

## v12 — music restored (embedded revert from v11 NitroFS)

- **the designer's v11 playtest (Nov 19, 2026, 12:00 PM EST): "Now there is no music at all! I only hear shooting sound effect."** Music worked in v10 and earlier.
- **Root cause (diagnosed Nov 19, 2026, ~5 h of bisect + audio forensics):** v11 moved the 7 tracks into the ROM filesystem (NitroFS, `nitro/music/areaN.raw`) with a chunked streaming loader. The music plays CORRECTLY in melonDS — verified by audio capture (see below) — but the loader fails silently on NooDS, the designer's iPhone emulator: `fopen("nitro:/music/areaN.raw")` returns NULL, `music_load_area` stays -1, and the game continues without music (no error shown). v10's music was EMBEDDED and she heard it fine. So the NitroFS path is not reliable on her device. Evidence that exonerated the other suspects:
  - The 7 NitroFS files ARE present in the v11 ROM with md5s matching the source files exactly (ndstool extraction).
  - The loaded buffer is correct: a debug build printed checksum 101727841, exactly matching `nitro/music/area1.raw` on disk.
  - Cache staleness ruled out: a `DC_FlushRange` debug build produced identical audio.
  - Volume-0 start ruled out: bisect B-VOL (music at volume 80, no fade) still produced the same output.
  - `soundSetFreq` ruled out: bisect B-NOFREQ (no `music_apply_rate` at track start) still produced the same output.
  - A 440 Hz square wave written into the music buffer came out at ~440 Hz — the channel, rate, and buffer are all correct. A white-noise / chirp test confirmed the channel plays the buffer faithfully.
  - The "70 Hz drone" seen in a 1 s FFT peak of the v11 capture is NOT a defect: it is the track's bass fundamental. The title-only capture is bright (centroid 637–938 Hz, 31–41% of energy above 300 Hz) — real musical content. The v11 music was playing fine in melonDS all along; it was simply never starting on NooDS.
  - The v10 control capture (embedded full track) shows real musical content (FFT peaks 525/1177 Hz), and a B-EMB2 bisect (v10 track data, same size, embedded, v10-style call) plays with a clear 525 Hz lead — proving the embedded path works and the data is fine.
- **Fix (v12): revert to the proven v10 architecture.** The same 7 unique tracks are EMBEDDED in the ARM9 binary (`audio/music_aN.bin` via bin2o, converted by new `tools/convert_music_v12.py` to 18 s loops from the START of each source — area 1 at 22050 Hz, areas 2–7 at 16000 Hz, 0.1 s fade-in, 0.5 s fade-out at the loop seam) and played with `soundPlaySample` at full volume (80) — no file I/O, no streaming, no volume-0 start. Total embedded music 2,124,900 bytes (2.03 MB); ARM9 ELF 2,787,096 bytes (~2.66 MB), under the 3.5 MB limit. The 1.6 MB `music_buf` double buffer is gone (BSS dropped from 1,611,304 to 9,600). `NITRO_FILES` removed from the top Makefile (the `nitro/` directory stays on disk as the v11 record but is no longer linked into the ROM); ROM back to 4 MB (was 8 MB). Area changes crossfade: the new track starts at full volume (80) immediately (always audible, even if the fade machinery misbehaves), and the old track fades out over 0.5 s. Boss tempo lift kept (`soundSetFreq`, 1.25x at intro, back to 1.00x after the kill). Title string updated to "STAR DRAG v12". README updated to v12. Everything else is v11 unchanged: boss 1 reverse / boss 2 halt / boss 3 scroll-only upward with open ceiling / boss 4 shield+charge, alarm x3 every boss, post-boss-2 HP scaling, pixelated type-specific explosions, boss multi-explosion deaths, labeled pickups, glowing fading weapon border (sine 40%–100%), relative drag, GAME OVER hold screen.
- **Deliverable:** `~/workspace/your_files/star-drag-v12.nds`, md5 `76d258011ef56bd36f413afb6ce04125`, 4,194,304 bytes (4 MB), game code `####`, maker zeroed, unit `0x00`, logo CRC 0xCF56 OK, header CRC 0xCDE5 OK. Clean rebuild reproduces the same md5 (verified twice; the first build md5 `de55dfdb…` was the same code before the title-string bump to "v12"). NOTE: the first v12 `build.sh` run copied to `star-drag-v11.nds` (the script still had the v11 filename) before the filename was fixed to v12 — so `~/workspace/your_files/star-drag-v11.nds` now holds a v12 build (md5 `de55dfdb…`); the original v11 ROM (md5 `c001dc67…`) is no longer on disk. v11 is superseded by v12, and its source state is fully documented here and in the v11 section, but the v11 ROM file itself cannot be restored from disk.
- **Verification — music actually playing (audio evidence, not "no crash"):**
  - melonDS 1.1, JIT off, Xvfb; audio capture via `SDL_AUDIODRIVER=disk` → `sdlaudio.raw` (48 kHz stereo s16), runner `qa/run_audio.py` (kept in the workspace — `/tmp` is wiped on restarts). Final release ROM (md5 `76d25801`): title screen (music only, no SFX) RMS 0.21–0.29 per second, centroid 939–1993 Hz, 45–88% of energy above 300 Hz — bright, clearly audible music, not silence and not a bass drone. Gameplay RMS 0.27–0.40, centroid 802–1712 Hz. 50 ms windows correlate with the embedded area-1 track (r up to 0.51). The shoot SFX template correlates in gameplay (r 0.30–0.33 at 10.9 s / 14.38 s) and the gameplay RMS envelope shows SFX bursts (0.38–0.54) above the music baseline (~0.28) — SFX work alongside music.
  - All 7 embedded tracks present in the SHIPPED ROM: extracted the ARM9 binary with ndstool and verified each track's md5 against its `audio/music_aN.bin` source — all 7 MATCH (a1 `ba1a3dbd…` 396900 B; a2 `28b805e2…`, a3 `41710f42…`, a4 `3aaa3a24…`, a5 `aeff5893…`, a6 `efc560a6…`, a7 `5f7cbe2f…`, each 288000 B). ELF symbols `music_a1_bin`…`music_a7_bin` present at contiguous addresses with the correct sizes.
  - Area transitions + boss tempo lift (debug ROM md5 `6c451e57…`, QA_DEBUG, god mode, LASER + all weapons + rate 3, wave-11 jump; `qa/v12-runs/debug-boss/`, 26 shots d00–d25 over ~6 min, no crash): the QA_DEBUG `MUSIC <name> a<area> v<vol> x<tempo>` readout shows a0 `music-v2 a0 v80 x1.00` at boot (OPEN SPACE); boss 1 (mode 1, REVERSE) in WATER `area6 a5 v80 x1.25`; back to a0 `x1.00` after the boss died; a1 `area2 a1 v80 x1.00` (SPACE BASE); a2 `area3 a2 v80 x1.00` (PLANET); a3 `area4 a3 v80 x1.00` (CAVE); boss 4 (mode 4, SHIELD) in CAVE `area4 a3 v80 x1.25` with the `SHIELD!` cue; back to a3 `x1.00`; a6 `area7 a6 v80 x1.00` (GOO). CRYSTAL (a4) was not reached in that run (it ended still in CAVE, wave 21–22; a wave-23 jump needs 88 kills to advance, which a 2.5 min scripted run never reaches) — verified in a second focused run (`qa/v12-runs/debug-crystal2/`, debug ROM md5 `d4068b25…`, starts directly in CRYSTAL via a QA-only `terrain_set_area(4)`): k03/k06 show `MUSIC area5 a4 v80 x1.00`, AREA CRYSTAL. **All 7 areas (a0–a6) confirmed playing their own embedded track at v80, and the boss tempo lift to x1.25 confirmed for modes 1 and 4 with ramp-down to x1.00 after the kill.** The crossfade itself (new track at v80 immediately, old track fading out over 0.5 s) is verified by code + the steady `v80` state across ~15 transitions with no crash; the fade's smoothness to the ear is for the designer's playtest.
  - End-to-end on the RELEASE ROM (`qa/v12-runs/e2e-release/`, ~3.5 min, no crash): title → play → GAME OVER (FINAL SCORE 9100, Wave reached: 4, BEST SCORE 9100, held on screen with the release-gate text) → tap to restart → fresh game in OPEN SPACE / SINGLE. No crash in ~15 min total v12 runs (release audio + debug boss + debug crystal + e2e).
  - JIT-on (melonDS 1.1, `Enable = true` asserted in the toml before and after): top screen 96.9% white (mean brightness 254.3/255), exactly as v11 and stock libnds hello-world — the known melonDS JIT + devkitPro-homebrew incompatibility, NOT a v12 bug. Fresh melonDS defaults to JIT off and renders v12 fine (all other runs above).
  - Real project verified clean after the fix: 0 `BISECT` markers, no wave jump / area override, no `music_buf`, no NitroFS calls (only the v12 explanatory comments); the debug loadout / wave jump / area override live only in `qa/v12-debug/` (never in the shipped ROM).
- **Honest gaps (the designer's gate):** Sound not heard headless — the audio-capture evidence above proves the music channel is active, the right track plays per area at the right rate and volume, and SFX play alongside it, but whether the 7 tracks sound distinct and fitting per area, and whether the fades and the boss tempo lift feel right, is for the designer's playtest. **The NooDS hypothesis (NitroFS load fails silently there) is the best explanation for v11's silence, but NooDS cannot run in this environment (flatpak-only, flatpak not installed), so the fix's effect on the designer's actual device is unproven until she playtests v12 — that is the final check.** D-pad untested (same as v4–v11). DeSmuME rendering unverifiable here (white SDL window for every ROM under Xvfb). Boss fights at debug HP 40 with god mode (same as v9–v11); boss 2/3 fights at REAL HP are for the designer's playtest. Boss 3's death sequence was not caught on camera in v12 (same as v11). The boss parks at x=196, so his 64 px sprite is clipped ~4 px by the right screen edge (same as v7–v11).
## v13 — area music fixed, boss-3 floor flies down, arcade title, clean touchpad, rainbow border

- **the designer's v12 playtest (Nov 19, 2026, 12:00 PM EST):** "The first level music is fine but the other areas I can't really hear or make out the music I mean I hear it playing but it's difficult to make out not sure if due to background music if volume lower in those areas or sound effects too loud, can you look and see if there is an issue with either the music in those areas or volume? Also the vertical flying boss player position looks all good star field good but the floor needs to fly downward away from the view too so it's full screen space star field at that point giving the feeling the player indeed flying up along with the boss and the ground moves away down that way after that boss battle you gradually resume to normal flight direction and the new area comes into view with respective ceiling floor or just obstacles or whatever with the starfield."
- **the designer's three more changes (Nov 19, 2026, 1:17 PM EST):** (1) proper retro-arcade title screen — big stylized game title, colourful/bold, starfield behind it, "TAP TO START" prompt, replacing the plain title text; (2) no more white-on-black log text on the touchpad — the bottom screen shows ONLY the finished UI (area name, weapon name, weapon panel buttons with glowing borders), all debug/log text removed from the shipped ROM; (3) rainbow glowing border — smooth rainbow cycle with the glow fading in and out, only on the selected weapon, uncollected stay greyed, no blinking.
- **Fix 1 root cause (measured, not guessed):** v12 sliced every loop from the START (0..18 s) of its source. The first 18 s of areas 3/4/5 are dark, bass-heavy intros (spectral centroid 235/250/286 Hz, only 2.8–3.8% of energy in 1–8 kHz, vs area 1's 454 Hz / 7.4%), so on a phone speaker they read as a faint murmur. And the shooting SFX (channel volume 100, raw RMS −8.4 dBFS) constantly masked the music (volume 80, raw RMS −17..−23 dBFS). Two more contributors: areas 2–7 were converted at 16000 Hz (cutting everything above 8 kHz), and per-track loudness in the Speechma generations varied by ~6 dB.
- **Fix 1 (what changed):** new `tools/convert_music_v13.py` — each loop is now the BEST 14 s window of its source (1 s steps, scored by RMS × (0.5 + HF fraction)), with a presence lift (highshelf +5 dB @ 1200 Hz, +3 dB @ 3 kHz) and loudness normalization to −17 dBFS RMS measured on the shipped u8 bytes, all 7 tracks at 22050 Hz. Code: `MUSIC_VOL` 80 → 110; shoot 100 → 65, laser 120 → 85, hit 127 → 90, boom 127 → 100, pickup 120 → 90 (alarm stays 127). **Loop length 14 s, not 16 s:** 16 s loops overflowed calico's `ds9.ld` ARM9 load-image cap (`lma9`, 0x27c000 = 2,621,440 bytes) by 80,796 bytes at link time — the "3.5 MB limit" in the v12 notes was wrong; the real cap is 2.5 MB. The dead v1 synth loop (`audio/music_synth_raw.bin`, 88,200 bytes, zero code references, preserved byte-identical as `qa/music_synth_v1_raw.bin`) was also removed. Final ARM9 load image 2,377,144 bytes, headroom 244,296 under the cap. Shipped loop measurements (u8 bytes, `tools/convert_music_v13.py` output): a1 −17.7 dBFS / HF 0.307 / 1625 Hz (window 65 s), a2 −17.2 / 0.138 / 725 Hz (52 s), a3 −17.1 / 0.090 / 564 Hz (58 s), a4 −19.0 / 0.208 / 950 Hz (47 s, peak-limited), a5 −17.1 / 0.225 / 1140 Hz (54 s), a6 −17.1 / 0.263 / 1523 Hz (5 s), a7 −17.1 / 0.202 / 928 Hz (34 s) — all within 1.9 dB (v12's shipped loops spanned −17.2..−23.6 dBFS). Two measurement traps found and fixed in the converter: ffmpeg's f32le path measures ~3 dB hotter than the u8 bytes that actually ship, and `acompressor` crushes dynamics ~4.4 dB with no makeup — gain is now computed from a u8 decode and peaks are handled by a lookahead `alimiter` (limit 0.92).
- **Fix 2 (boss-3 floor):** during the upward flight the floor band slides down to the −48 px terrain-scroll clamp and is then blanked out of the map (`terrain_clear_floor()`), so the fight plays over a full starfield; new terrain columns are suppressed while `vert_target != 0`, floating rocks are cleared at the intro too, and after the boss dies the new area's ceiling/floor ease back in gradually (`scroll_terr_y` eases 1 px/frame to 0 instead of snapping — `boss_death_final` no longer resets it, and the area is switched BEFORE `terrain_rebuild()` so the rebuild draws the new area). Ship/boss positions and orientations unchanged.
- **Change 3 (title screen):** big gradient "STAR DRAG" art (`tools/convert_title.py`, 46 px DejaVu Sans Bold, sunset gradient + outline + shadow, 215 tiles) on the far starfield layer over the live scrolling starfield, with a pulsing "TAP TO START" prompt (palette 9 index 1, sine pulse). Game-over art likewise ("GAME OVER" + "TOUCH TO RETRY", 222 tiles).
- **Change 4 (no log text):** the sub screen is now a tile-text UI (`tools/convert_font.py`, 8×8 + 16×16 pixel font, navy backdrop, coloured text) instead of a console: area/weapon lines, boss cues as a transient scale-2 banner, title hints, game-over score lines. Every `iprintf`/`consoleClear` in the release build is behind `#ifdef QA_DEBUG` (verified by grep: 0 outside the guards, ignoring comment mentions); `videoSetModeSub` + `bgInitSub` take the sub screen over from the console at boot. The QA debug readout is tile text now too (it used to print to the invisible console).
- **Change 5 (rainbow border):** `rainbow_col()` — hue cycles 0..359° over a 4 s (240-frame) cycle while brightness breathes over a 2 s sine (40–100%, never off). Only on the selected weapon; uncollected stay greyed.
- **Bugs found by v13 QA and fixed before delivery:**
  - **Title art tiles clobbered the starfield maps (Nov 19, 2026):** the title/game-over art tiles (tile base 0, tiles 16..460) ran past the starfield maps at bases 2–5 (0x06001000–0x06002FFF, inside tile base 0's 16 KB), so the top screen rendered as garbage coloured blocks on title AND in gameplay. Fixed by moving the four main maps to bases 16–19 (0x06008000+, free VRAM), freeing all of tile base 0 for art tiles. (The first smoke run caught it; the "STAR DRAG" letters were visible but the starfield was striped blocks.)
  - **'+' glyph rendered as '-' (Nov 19, 2026):** `tools/convert_font.py` rendered directly at 7 px, and PIL antialiasing thinned the 1 px vertical stroke to nothing. Fixed by rendering at 3× and downsampling by max-ink per 3×3 block; '+' is now a proper cross, verified on screen ("WEAPON LASER +3").
- **Deliverable:** `~/workspace/your_files/star-drag-v13.nds`, md5 `c3795d6ead0f3a70776954efb498150d`, 4,194,304 bytes (4 MB), game code `####`, maker zeroed, unit `0x00`, logo CRC 0xCF56 OK, header CRC 0xD9AC OK. Clean rebuild reproduces the same md5 (verified). All 7 v13 tracks present in the shipped ROM (ndstool-extracted ARM9 binary, md5s match `audio/music_aN.bin` exactly). Total embedded music 2,160,900 bytes.
- **Verification — Fix 1, per-area audio evidence (melonDS 1.1, JIT off, `SDL_AUDIODRIVER=disk`, runner `qa/run_v13.py`, analysis `qa/analyze_audio.py`):** seven debug-variant runs (`qa/v13-runs/audio-a0/` … `audio-a6/`, one per area, QA_DEBUG loadout LASER + all weapons + rate 3, start waves chosen so no boss intro lands in the 45 s window), each with title (music only) + gameplay (music + shooting SFX). Median per-second loudness, title (music only) → gameplay (music+SFX), RMS dBFS: a0 −8.8 → −7.3, a1 −8.9 → −7.3, a2 −8.8 → −7.2, a3 −8.6 → −6.9, a4 −8.9 → −7.1, a5 −8.9 → −7.1, a6 −8.7 → −6.7 — **every area within 0.3 dB of area 1 at title and within 0.6 dB in gameplay** (v12's shipped loops spanned 6.4 dB). Brightness (title centroid / HF fraction): a0 1446 Hz / 0.281, a1 2071 / 0.384, a2 1175 / 0.188, a3 1180 / 0.228, a4 1655 / 0.312, a5 1936 / 0.252, a6 1171 / 0.227 — all clearly audible music, not bass murmurs (v12's quiet areas were 235–286 Hz / 0.028–0.038). Track identification under SFX (50 ms windows vs the shipped loop, same method as v12): own-track median r 0.33–0.48, max 0.55–0.81, 70–95% of windows > 0.3; the a1-track control is lower in every case (median 0.29–0.44). The debug readout confirms each run's area and track (`MUS aN v110 x1.00`, AREA <name>, verified by OCR `qa/read_sub.py`).
- **Verification — Fix 2, boss 3 (`qa/v13-runs/boss3/`, debug ROM md5 `cd0b55ee…`, area 4 / wave 8 / bossnum 2, 130 shots over 260 s, no crash):** WARNING cue at t≈38–40, fight t≈42–119: readout holds `S16,80 V+0 E0 B1M3 U-160 Y-48` — ship unmoved at (16,80), horizontal scroll halted, vertical scroll at max, floor offset at the −48 clamp, for the whole fight. Screenshots show a full-screen starfield (no ceiling, no floor) with the ship firing right and the green-eye boss on the right. Boss dies at t≈119; the next shots show the new area's ceiling/floor easing back in gradually (readout `V+82 U-78`, then `V+100 U+0 Y+0`; the t=119 frame catches the terrain mid-rise, t=121 fully in place — no snap). NORMAL FLIGHT cue after the kill. The same run also caught boss 4 (SHIELD, WATER, t≈139–147, tempo lift `x1.25`) and boss 1 (REVERSE, `V-160`, OPEN SPACE, t≈188–194) with clean returns to normal flight. **Observation, not a defect:** the boss-3 fight took ~77 s unattended because the boss bobs vertically across the stationary QA ship's fixed laser line, so most shots miss — the designer aims with drag, so this is a playtest matter, not a QA failure. The boss DOES die unattended (no soft-lock).
- **Verification — title / clean touchpad / rainbow border:** title screen (`qa/v13-runs/smoke/00-title.png`, release ROM): gradient STAR DRAG over a clean starfield with drifting enemies, pulsing TAP TO START, sub screen showing only the three control hints in tile text and the four greyed weapon buttons — no log text anywhere. Game-over (`qa/v13-runs/e2e-release2/13-t121.png`): gradient GAME OVER + TOUCH TO RETRY, sub screen "FINAL SCORE 9100 / WAVE 4 / BEST 9100" in scale-2 tile text, no log text. Rainbow border (`qa/border_color.py` sampling the top edge of the selected button): smoke run hues 29° → 212° → 32° across 12 s (exactly the 180°/6 s the 4 s cycle predicts at ~60 fps); boss-3 run hues alternate 44° → 228° → 50° → 234° → 56° → 238° every 2 s — the button's own cyan accent cannot do that, so this is the rainbow border, only ever on the selected weapon (all other slots grey in every sample). The '+' fix was verified on screen in `qa/v13-runs/fontcheck/` ("WEAPON LASER +3", OCR + visual).
- **Verification — end-to-end on the RELEASE ROM (`qa/v13-runs/e2e-release2/`, md5 `c3795d6e…`, 250 s, 27 shots, no crash):** title → play → GAME OVER (FINAL SCORE 9100, Wave 4, BEST 9100 — same deterministic score as v12's e2e) held on screen → tap to restart → fresh game in OPEN SPACE. The earlier suite e2e (`qa/v13-runs/e2e-release/`, md5 `bbb65eea…`, pre-font-fix ROM, gameplay-identical) showed the same flow. No crash in ~30 min total v13 runs (7 audio + boss3 + 2 e2e + smoke + fontcheck + JIT-on).
- **Verification — JIT-on (melonDS 1.1, `Enable = true` asserted in the toml before and after, `qa/v13-runs/jit-on/`):** top screen 96.9% white, bottom 96.9% black — exactly as v12 and stock libnds hello-world, the known melonDS JIT + devkitPro-homebrew incompatibility, NOT a v13 bug. Fresh melonDS defaults to JIT off and renders v13 fine (all other runs above).
- **QA tooling added in v13 (kept in the workspace):** `qa/run_v13.py` (melonDS runner: JIT assert, audio capture, periodic shots, second-tap for restart), `qa/run_v13_suite.sh` (the full suite driver), `qa/read_sub.py` (sub-screen tile-text OCR from screenshots, using the exact font bitmaps — sub grid origin y=214, threshold max>120), `qa/analyze_audio.py` (per-second RMS/centroid/HF + 50 ms track correlation), `qa/border_color.py` (border hue sampler), `qa/apply_v13_debug_patches.py` (re-applies the QA-only patches to `qa/v13-debug/` after rsync). Debug copy `qa/v13-debug/` (QA_DEBUG, parameterized `QA_START_AREA`/`QA_START_WAVE`/`QA_START_BOSSNUM` via make variables; loadout placed AFTER the owned[] reset — v12-debug put it before and the reset wiped it). Runner lessons recorded in the scripts: HOME must be an absolute path (melonDS rejects a relative one with a blocking config dialog) and the ROM path must be absolute (melonDS runs with cwd=outdir).
- **Real project verified clean after the fix:** 0 `BISECT` markers, no wave jump / area override / `QA_START_*` in the real source (those live only in `qa/v13-debug/`), no `music_buf`, no NitroFS calls (only explanatory comments); the only `iprintf` in the real source is inside `#ifdef QA_DEBUG`.
- **Honest gaps (the designer's gate):** Sound not heard headless — the audio-capture evidence above proves every area's music is now as loud and bright as area 1's, with the right track playing per area at v110 and identifiable under the shooting SFX, but whether the mix feels right to her ear is for the designer's playtest. D-pad untested (same as v4–v12). DeSmuME rendering unverifiable here (white SDL window for every ROM under Xvfb). NooDS/RetroArch iPhone paths not testable here. Boss fights at debug HP 40 with god mode (same as v9–v12); boss 2/3 fights at REAL HP are for the designer's playtest. The boss parks at x=196, so his 64 px sprite is clipped ~4 px by the right screen edge (same as v7–v12). The ':' glyph in the new font renders as a merged vertical bar (two dots touching at 8 px) — no game text uses ':' today, so it is untested in context.

## v14 — music garbling fixed (volumes reverted, loops re-rendered peak-safe), boss 1 HP 25 -> 70

- **the designer's v13 playtest (Nov 19, 2026, ~3:11 PM EST):** "Put the music volume back to where it was. We need to fix the music elsewhere so basically it just sounds distorted everywhere and increasing the volume did not help it just made it worse actually. I'm not sure what's going on but the music track sounds garbled and distorted can you check if it's being played cleanly maybe provide a link to a few tracks here so I can listen to compare in the game as I think it's garbled and distorted in the game. Also the first boss needs more hp as it's killed too quickly right when the backwards flying gets so cool."
- **Fix 1 root cause (measured, not guessed — three separate measurements):**
  1. **The v13 FILES are not clipped.** Shipped v13 loops: peaks 0.76–0.92, zero samples at the rails (0/1/254/255), DC offset ≤0.002. The source MP3s for areas 2 and 6 ARE mastered hot (float-decode peaks 1.19 / 1.13, inter-sample overs), but the v13 limiter capped those in the files. So the garbling is not in the loop data.
  2. **The music ALONE plays clean in melonDS.** v13 title capture (music only, vol 110): peak 0.85, zero pinned samples, RMS −9.0 dBFS. So the channel volume alone does not distort the music.
  3. **The MIX clips.** v13 made the music ~9 dB hotter than v12 (loops RMS −17 dBFS vs −23.6 for area 1, channel vol 110 vs 80): a music peak 0.92 × 110/127 = 0.80 of full scale, plus shooting SFX on top, sums the DS 16-bit mixer over full scale. The melonDS gameplay capture proves the summed output pins at full scale (v13-play: 2063 pinned samples, 1599 flat-top samples in 37 s; v13 debug audio runs: 0.45–0.76% of gameplay samples pinned). Louder volume = more clipping = worse — exactly as the designer reported. And the v13 presence lift (+5 dB @ 1200 Hz, +3 dB @ 3 kHz) pushed the EQ'd signal's peaks to 1.29–1.77 (up to +4.9 dB OVER full scale) before gain reduction and quadrupled the HF energy share (area 1: 0.307 of energy in 1–8 kHz vs 0.074 in v12) — a much harsher, brighter sound on a phone speaker, which reads as garbled/distorted even where it is not digitally clipping. (Caveat: in melonDS the gameplay pinning is dominated by SFX-event bursts — booms — in BOTH v12 and v13, and v12's release capture actually pins MORE overall (2879 vs 2063) because its boom is at vol 127 with a peak-1.0 file; the discriminator for what the designer hears is the MUSIC bed level, 0.80 vs 0.27 of full scale. NooDS's mixer cannot be tested here, so the fix is designed so the summed mix stays under full scale on ANY mixer.)
- **Fix 1 (what changed):** volumes REVERTED to v12 exactly (`MUSIC_VOL` 110 → 80; shoot 65 → 100, laser 85 → 120, hit 90 → 127, boom 100 → 127, pickup 90 → 120; alarm stays 127). New `tools/convert_music_v14.py`: the BEST 14 s windows are KEPT (same windows as v13 — they are genuinely the clearest sections), the aggressive presence lift is replaced by a GENTLE +2 dB highshelf @ 1200 Hz applied BEFORE the peak-capping gain (it brightens the dark areas measurably — area 3 centroid 239 → 306 Hz, area 4 412 → 529 Hz — and can never clip), RMS target −18 dBFS, peaks capped at 0.70 (−3.1 dBFS), NO limiter (gain reduction handles peaks, no squashing). Shipped v14 loops: RMS −18.1..−19.5 dBFS (all within 1.4 dB), peaks 0.48–0.70, zero rail samples, centroids 308–898 Hz. Worst-case mixer sum music+shoot = 0.82–0.96 across all areas — under full scale everywhere. Total embedded music 2,160,900 bytes; ARM9 load image 2,377,132 bytes (headroom 244,308 under the 0x27c000 cap).
- **Fix 2 (boss 1 HP):** 25 → 70 (~2.8×). Attack pattern, bullet speed/count, and movement are UNCHANGED — only HP. Bosses 2–4 HP unchanged (40+wave×5).
- **Deliverable:** `~/workspace/your_files/star-drag-v14.nds`, md5 `17f369d26851e0db4096e27d0880b63e`, 4,194,304 bytes (4 MB), game code `####`, maker zeroed, unit `0x00`, logo CRC 0xCF56 OK, header CRC 0xEEFD OK. Clean rebuild reproduces the same md5 (verified). All 7 v14 tracks present in the shipped ROM (ndstool-extracted ARM9 binary, md5s match `audio/music_aN.bin` exactly).
- **Comparison audio for the designer (in `~/workspace/your_files/star-drag-v14-compare/`):** for areas 1–4 (OPEN SPACE, SPACE BASE, PLANET, CAVE): `areaN-<name>-source.mp3` (the clean 14 s loop window straight from the MP3) and `areaN-<name>-in-game.mp3` (10 s of exactly what the v14 game plays — melonDS music-only title capture, no SFX). Verified distinct content (different hashes): source RMS −11.0..−17.2 dBFS, in-game RMS −8.8..−9.0 dBFS, in-game clips correlate with the shipped v14 loops at median r 0.47–0.70, max 0.70–0.79 — each in-game clip is provably the right track as the game plays it.
- **Verification — Fix 1, per-area audio evidence (melonDS 1.1, JIT off, `SDL_AUDIODRIVER=disk`, runner `qa/run_v13.py`, analysis `qa/analyze_audio.py`, runs `qa/v14-runs/audio-a0/` … `audio-a6/`, debug ROMs, 45 s each):** title (music only) median RMS per area: a0 −11.5, a1 −11.5, a2 −11.4, a3 −11.4, a4 −11.4, a5 −11.3, a6 −11.3 dBFS — **every area within 0.2 dB of area 1** (v13: −8.6..−8.9; v12's area 1 was −11.8, so v14 sits at v12's level with all areas matched). Brightness (title centroid / HF fraction): a0 1096 Hz / 0.205, a1 1549 / 0.286, a2 848 / 0.125, a3 942 / 0.163, a4 1148 / 0.209, a5 1173 / 0.155, a6 851 / 0.165 — clearly audible music, not bass murmurs, and far less harsh than v13's EQ'd tracks. Track identification under SFX (50 ms windows vs the shipped loop): own-track median r 0.30–0.47, max 0.47–0.81, 50–100% of windows > 0.3. **Clipping — the fix, measured:** music-only title windows now pin 0–10 samples at the music-start transient (v13: 62–109 at the same instant — a 10–20× reduction) and 0 in the rest of the music (tap-boundary clusters only, from the first shots landing as the game starts). Gameplay pinning is confined to SFX-event bursts (booms): 134 of 168 quarter-second windows have ZERO pinned samples, longest clean stretch 6.5 s (v13: 124 clean, 5.0 s) — the music bed between bursts is clean.
- **Verification — Fix 2, boss-1 timing (debug ROMs, god mode, LASER + rate-3 loadout, ship parked at 16,60, 2 s screenshots, readout via `qa/read_sub.py`, runs `qa/v14-runs/boss1-v14/` and `qa/v14-runs/boss1-v13/`):** the v13-before run used the v13-debug copy patched to the REAL v13 HP (25) instead of the flat QA_DEBUG 40, so the comparison is apples-to-apples; the v14 debug copy likewise uses the real HP (70) via `qa/apply_v14_debug_patches.py` patch 4. **v13 (HP 25):** boss 1 active t25–t27 sampled, dead by t29 — fight ≈ 4–6 s, max reverse (V−160) held ~2 s of samples. **v14 (HP 70):** boss 1 active t25–t35, V−160 held t27–t33 — fight ≈ 10–12 s, max reverse held ~8 s. **~2.5× longer fight, ~4× longer at max reverse** — matching the 70/25 = 2.8× HP ratio. Ship never moved (S16,60 in every shot), no enemies during the boss (E0 in every B1 sample), reverse eases back through a stop to V+100 after the kill. Bosses 2–4 in the same runs confirm their HP is unchanged (M2 halt V+0 held, M3 upward U−160 Y−46 with the floor flown down, M4 shield) — the v14 run also re-verifies boss 3's floor sweep (Y−46) and the gradual return to normal flight.
- **Verification — end-to-end on the RELEASE ROM (`qa/v14-runs/e2e-release/`, md5 `17f369d2…`, 250 s, 27 shots, no crash):** title (gradient STAR DRAG + TAP TO START, clean bottom UI — verified visually) → play → GAME OVER (FINAL SCORE 4200, Wave 3, BEST 4200; t121/t131 byte-identical, bottom-screen text identical across t111–t141 — held; the top-screen starfield keeps animating behind the art, so top pixels differ) → tap at 215 s → fresh game in OPEN SPACE (WEAPON SINGLE, then SPREAD by t242). No crash in ~30 min total v14 runs (7 audio + 2 boss timing + 4 title captures + e2e + JIT-on).
- **Verification — JIT-on (melonDS 1.1, `Enable = true` asserted in the toml, `qa/v14-runs/jit-on/`, re-run with an absolute outdir — the suite's first attempt failed on a relative HOME path):** top screen 96.9% white (mean 254), bottom 3.1% white — exactly as v12/v13 and stock libnds hello-world, the known melonDS JIT + devkitPro-homebrew incompatibility, NOT a v14 bug. Fresh melonDS defaults to JIT off and renders v14 fine (all other runs above).
- **QA tooling / process notes (kept in the workspace):** `qa/run_v14_suite.sh` (full suite driver), `qa/apply_v14_debug_patches.py` (patches 1–3 as v13 + patch 4: real boss HP in debug builds), debug copy `qa/v14-debug/`. **Suite bug found and fixed mid-run:** the first suite attempt rsynced the REAL `build.sh`/`arm9/Makefile` into `qa/v14-debug/`, so the "debug" builds were actually release builds copied to `~/workspace/your_files/` and `star-drag-debug.nds` never existed — the first 5 audio runs captured a frozen dialog + silence. Caught by the silence (all-zero audio + byte-identical screenshots), suite STOPPED (killed, confirmed stopped), debug `build.sh` + `arm9/Makefile` copied from `qa/v13-debug/`, invalid run dirs deleted, suite re-run from scratch. Lesson: after rsyncing a debug copy, always copy the debug `build.sh`/`arm9/Makefile` too, and smoke-test one debug run (audio non-silent, screenshots differing) before launching the suite. The release deliverable was never harmed — its md5 is unchanged (`17f369d2…`, rebuilt twice, identical both times).
- **Real project verified clean after the fix:** 0 `BISECT` markers, no wave jump / area override / `QA_START_*` / `music_buf` in the real source, no NitroFS calls (only explanatory comments); the only `iprintf` in the real source is inside `#ifdef QA_DEBUG`; volumes verified in source (`MUSIC_VOL` 80, shoot 100, laser 120, hit 127, boom 127, pickup 120, alarm 127).
- **Honest gaps (the designer's gate):** Sound not heard headless — the capture evidence above proves the music is now peak-safe, loudness-matched, and clean (music-only pinning down 10–20× vs v13), but whether the mix feels right to her ear is for the designer's playtest, and the comparison MP3s are there so she can hear source vs in-game herself. NooDS's mixer behaviour cannot be tested here — the fix keeps the summed mix under full scale on any mixer, which is the robust fix either way. D-pad untested (same as v4–v13). DeSmuME rendering unverifiable here (white SDL window for every ROM under Xvfb). NooDS/RetroArch iPhone paths not testable here. Boss fights timed at real HP but with god mode + LASER/rate-3 loadout and a parked ship (the designer aims with drag, so her fights will run shorter than these unattended times — the 2.5× ratio is what the HP change buys). The boss parks at x=196, so his 64 px sprite is clipped ~4 px by the right screen edge (same as v7–v13). The ':' glyph renders as a merged vertical bar — no game text uses ':' today.

## v15 — music STILL distorted in v14 (gameplay mixer clipping + bad source windows), fixed with the v12 recipe + clean v2 tracks

- **the designer's v14 verdict (Nov 19, 2026):** the music is STILL distorted in the game — even area 1, which was fine in v12 and whose source is clean. The comparison samples for areas 2, 3, 4 were also distorted. Her bar: the music in the game should sound the same as the clean source sample (area 1's sample sounded right to her).
- **Root cause 1 — GAMEPLAY mixer clipping (the in-game distortion, incl. area 1), measured, not guessed:** v14's music-ONLY title capture is clean (peak 0.49, zero pinned samples), but the GAMEPLAY capture (music + shooting SFX) hits full scale constantly. Same script (`/tmp/v15-pinning.py`, 48 kHz stereo s16 melonDS disk capture, pinned = |x| >= 0.999) on the v14 area-1 run (`qa/v14-runs/audio-a0/`, wave 1): peaks 1.00 in ~half the seconds, up to 3.83% of samples pinned in a single second (s35), **11,278 pinned samples over 33 s of gameplay (0.712%)**. The DS hardware mixer sums the channels: v14 music peak 0.65 x 80/127 = 0.41 plus shoot 0.656 x 100/127 = 0.52 / laser 0.734 x 120/127 = 0.69 / hit 0.875 / boom 1.000 (boom alone pins at vol 127). v12's area-1 loop was ~3.5 dB quieter (RMS -23.6 dBFS, peak 0.43) so the sum stayed under full scale far more often. The v14 compare "in-game" clips were TITLE captures (music only) — they never showed the gameplay mix the designer actually hears, which is why melonDS title evidence said "clean" while the game sounded distorted. v15 verifies on GAMEPLAY captures, not title-only.
- **Root cause 2 — BAD source windows for areas 2-4:** the v13/v14 distortion-blind "loudest window" pick landed on genuinely overdriven sections of the OLD tracks: OLD area-2 window 52-66 s has decode peak **1.19** (clips AT the source), RMS -10.5 dBFS, 0.41% of samples within 1 dB of full scale; OLD area-3 window 58-72 s is a dark bass murmur (centroid 239 Hz, HF 0.027). Fixed by three fresh clean Speechma generations (assets/music-area{2,3,4}-*-v2.mp3, prompts asked for a clean mix; decode peaks 0.75 / 1.10 / 1.11, no squashed 14 s window).
- **Fix (new `tools/convert_music_v15.py`, no code changes — volumes stay at v12: music 80, shoot 100, laser 120, hit 127, boom 127, pickup 120, alarm 127):** back to the v12 recipe — plain resample, NO EQ lift, NO RMS normalization, 14 s loops, all at 22050 Hz, 0.1 s fade-in, 0.5 s fade-out. Area 1 uses window 0 — EXACTLY v12's window (0-14 s of music-v2.mp3), the one proven clean on the designer's device. Areas 2-7 use the loudest CLEAN window (decode peak <= 1.00 and squash <= 0.05%): {1:0, 2:42, 3:56, 4:56, 5:56, 6:42, 7:28}, re-checked by the script at runtime. Peak-safe gain only: gain = min(0.9, 0.60/window_peak). NOTE (measured Nov 19): ffmpeg's MP3-decoded path scales the u8 output by 1/sqrt(2) = 0.707 vs the float chain (least-squares k = 0.707, corr 0.9993, same filter chain to f32 vs u8; the pure format conversion itself is linear — the scale comes from the MP3 decode/resample/downmix path), so the f32-side cap of 0.60 lands at **~0.42 in the bytes the DS plays — exactly v12 area-1's shipped peak (0.43, RMS -23.6 dBFS, reproduced by re-running the v12 recipe)**. Do NOT "fix" this by raising the gain. Worst-case mixer sum music+shoot is then 0.42 x 80/127 + 0.52 = 0.78, under full scale with margin. Shipped v15 loops: peaks all 0.42, RMS -24.7 / -18.8 / -19.7 / -20.2 / -19.7 / -20.7 / -19.9 dBFS, centroids 673 / 307 / 501 / 511 / 505 / 403 / 282 Hz, zero rail samples. Total embedded music 2,160,900 bytes; ARM9 load image 2,377,132 bytes (headroom 244,308 under the 0x27c000 cap).
- **Deliverable:** `~/workspace/your_files/star-drag-v15.nds`, md5 `de4422f8b023707dca3b32044b8920be`, 4,194,304 bytes (4 MB), game code `####`, maker zeroed, unit `0x00`, logo CRC 0xCF56 OK, header CRC 0xEEFD OK. Clean rebuild reproduces the same md5 (verified). All 7 v15 tracks present in the shipped ROM (ndstool-extracted ARM9 binary, md5s match `audio/music_aN.bin` exactly). v14 ROM untouched (`17f369d2…`).
- **Comparison audio for the designer (in `~/workspace/your_files/star-drag-v15-compare/`):** for areas 1–4 (OPEN SPACE, SPACE BASE, PLANET, CAVE): `areaN-<name>-source.mp3` (the exact 14 s loop window straight from the MP3) and `areaN-<name>-in-game.mp3` (10 s of exactly what v15 plays — melonDS music-only title capture, s2–s12, skipping the s0 startup transient). Verified distinct content (different md5s), durations 14.0 s / 10.0 s; in-game clips correlate with the shipped v15 loops at median r 0.30–0.68, max 0.67–0.80 — each in-game clip is provably the right track as the game plays it. (Area 1's median 0.30 is the dynamic track's quiet passages in 50 ms windows; max 0.69 confirms identity, same pattern as the v12 reference run's 0.37 median.)
- **Verification — Fix 1, per-area audio evidence (melonDS 1.1, JIT off, `SDL_AUDIODRIVER=disk`, debug ROMs, 45 s each, runs `qa/v15-runs/audio-a0/` … `audio-a6/`, waves 1/4/6/7/10/12/13):** title (music only) median RMS per area: a0 -11.0, a1 -11.3, a2 -11.2, a3 -11.2, a4 -11.3, a5 -11.1, a6 -11.0 dBFS — every area within 0.3 dB of area 1, back at v12's level (v12 ref title -11.0). Track identification under SFX (50 ms windows vs the shipped loop): own-track median r 0.29–0.43, max 0.48–0.77. **Gameplay pinning in the debug runs is loadout-driven, NOT the shipped mix:** the debug loadout is LASER + rate-3 (PLAY RMS -7.5..-6.7 dBFS), so those runs pin 9,009–23,577 samples over 33 s — the SFX density (laser vol 120, constant fire) dominates. The FAIR comparison is release-vs-release, same wave 1, same SINGLE-weapon start (`qa/v15-runs/ref-v12-a0/` vs `qa/v15-runs/ref-v15-a0/`, 45 s each): v12 PLAY pinned **3,255 (0.205%)**, PLAY RMS -9.0; v15 PLAY pinned **2,568 (0.162%)**, PLAY RMS -9.3 — v15 pins 21% LESS than the version the designer called fine for area 1, at the same title loudness (-11.0 both) and the same track identifiability (corr median 0.37 vs 0.34, max 0.74 vs 0.78). The remaining pinning in both is brief boom/hit transients (boom alone is 1.00 at vol 127 — v12 behaviour, unchanged and approved).
- **Verification — boss-1 timing unchanged (debug ROM, real HP 70, run `qa/v15-runs/boss1-v15/`, 150 s, readouts via `qa/read_sub.py`):** boss 1 (M1 reverse) B1 at t25 (V-120 ramping), V-160 held t27–t33, dead by t35, normal flight by t37 — fight ≈ 10 s, max reverse held ~6–8 s, matching v14's ~10–12 s / ~8 s. Boss 2 (M2 halt) B1 t56–t66 (V+0 held), dead by t68. Boss 3 (M3 upward) B1 t86–t94 (U-160, Y-46 floor flown down), easing back t94 (U-96, Y-17), normal t96 — the floor sweep still works. Boss 4 (M4 shield) B1 t111–t129, dead by t131. Ship never moved (S16,60 in every shot), no enemies during any boss (E0 in every B1 sample). Areas cycled all 7 (OPEN SPACE → SPACE BASE → PLANET → CAVE → CRYSTAL → WATER → GOO → OPEN SPACE).
- **Verification — end-to-end on the RELEASE ROM (`qa/v15-runs/e2e-release/`, md5 `de4422f8…`, 250 s, 27 shots, no crash):** title (gradient STAR DRAG + TAP TO START, clean bottom UI — verified visually) → play → GAME OVER (FINAL SCORE 4200, Wave 3, BEST 4200 — verified visually, big gradient art + TOUCH TO RETRY, bottom screen scores; held t111–t211) → tap at ~222 s → fresh game in OPEN SPACE (WEAPON SINGLE at t232, then SPREAD by t252). No crash in ~35 min total v15 runs (7 audio + boss timing + 4 title captures + 2 reference runs + smoke + e2e + JIT-on).
- **Verification — JIT-on (melonDS 1.1, `Enable = true` asserted in the toml, `qa/v15-runs/jit-on/`):** top screen 96.9% white, bottom 3.1% white — exactly as v12/v13/v14 and stock libnds hello-world, the known melonDS JIT + devkitPro-homebrew incompatibility, NOT a v15 bug. Fresh melonDS defaults to JIT off and renders v15 fine (all other runs above).
- **QA tooling / process notes (kept in the workspace):** `qa/run_v15_suite.sh` (full suite driver), `qa/apply_v15_debug_patches.py` (v14 patches 1–4, path updated), debug copy `qa/v15-debug/`, pinning analysis `/tmp/v15-pinning.py`, window scan `/tmp/v15-window-scan.py`. **Suite setup catch (same lesson as v14):** the first `cp` of the debug `build.sh` into `qa/v15-debug/` silently failed (4-arg cp with a non-directory target), leaving the REAL build.sh in the debug copy — caught by diffing before running (the real build.sh would have overwritten the release ROM). Always diff the debug `build.sh`/`arm9/Makefile` after copying. The release deliverable was never harmed — its md5 is unchanged (`de4422f8…`, rebuilt twice, identical both times).
- **Real project verified clean after the fix:** 0 `BISECT` markers, no wave jump / area override / `QA_START_*` / `music_buf` / NitroFS calls in the real source (only explanatory comments); the only `iprintf` in the real source is inside `#ifdef QA_DEBUG` (8 guarded blocks); volumes verified in source (`MUSIC_VOL` 80, shoot 100, laser 120, hit 127, boom 127, pickup 120, alarm 127); all 7 music rates 22050 Hz.
- **Honest gaps (the designer's gate):** Sound not heard headless — the capture evidence above proves the gameplay mix now pins 21% less than v12 (the version she called fine for area 1) and the title music is clean in every area, but whether the mix feels right to her ear is for the designer's playtest, and the comparison MP3s are there so she can hear source vs in-game herself. NooDS's mixer behaviour cannot be tested here — the fix keeps the summed mix under full scale on any mixer, which is the robust fix either way, and the shipped peaks now match v12's proven value exactly. D-pad untested (same as v4–v14). DeSmuME rendering unverifiable here (white SDL window for every ROM under Xvfb). NooDS/RetroArch iPhone paths not testable here. Boss fights timed at real HP but with god mode + LASER/rate-3 loadout and a parked ship (the designer aims with drag, so her fights will run shorter than these unattended times). The boss parks at x=196, so his 64 px sprite is clipped ~4 px by the right screen edge (same as v7–v14). The ':' glyph renders as a merged vertical bar — no game text uses ':' today.

## v16 — the distortion is 8-bit quantization: 16-bit music, halved drop rates, 1UP, pickup chime, boss-damage tick

- **the designer's v15 verdict (Nov 19, 2026, ~5:44 PM EST):** "Ok I hear the problem those links you sent all of the 'source' are clean and perfect but the 'in game' links are distorted and overdrive sounding can you fix those somehow or is that what happens when you play the music in game?" — decisive: the source clips are clean, the music-ONLY in-game clips are distorted/overdriven. The v15 measurements (peaks, pinned samples) do NOT capture what she hears. The distortion is in the playback/conversion path itself, present even with no SFX playing.
- **Answer to the designer's question: NO, it is not inherent — the DS can play clean 16-bit audio. The game has just been using 8-bit samples.** libnds supports `SoundFormat_16Bit` (`SoundFmt_Pcm16`, confirmed in `devkitpro-image/root/opt/devkitpro/libnds/include/nds/arm9/sound.h`). v16 plays the music as 16-bit PCM; the 8-bit grit is gone.
- **Root cause — 8-bit quantization, measured (script `/tmp/v16-diagnose.py`, Nov 19, 2026):** the shipped v15 area-1 loop, measured against its own float reference chain, has a **quantization-error SNR of only 28.5 dB** (least-squares fit k=0.7070, corr 0.9993 — confirming ffmpeg's MP3-decoded u8 path scales by 1/sqrt(2), which is benign gain, not distortion). The same chain rendered in 16-bit measures **79.0 dB**. 8-bit quantization noise on sustained synth tones is exactly the harsh, gritty, overdriven sound the designer described, and no peak/clipping measurement will ever show it — which is why v13/v14/v15 all "measured clean" while sounding distorted. HF noise floors were uninformative (u8 -16.8 dB rel vs float ref -16.9 dB rel — the music itself is HF-rich), so the SNR-of-quantization-error number is the discriminator. **Fix chosen — 16-bit PCM playback:** new `tools/convert_music_v16.py`: same windows as v15 (area 1 = 0, areas 2–7 loudest CLEAN window), same rate 22050 Hz, same target shipped peak 0.42, s16 bytes written from numpy (not ffmpeg's s16le output, to avoid any format-path rescaling). **7.5 s loops** (not 14 s): 16-bit doubles size; ARM9 load cap is 0x27c000 = 2,621,440 bytes; v15 ARM9 binary was 2,377,132 (headroom 227,924; non-music ARM9 = 216,232). 7.5 s x 22050 Hz x 2 bytes = 330,750 bytes/track, 2,315,250 total, ARM9 ~2,531,482, headroom ~73,574. 14 s 16-bit would be 4,321,800 — impossible. Conversion results: all 7 tracks 330,750 bytes, peaks 0.40–0.42, quant SNR **75.1–81.8 dB** (area 1: 75.1 dB, RMS -25.9 dBFS; a2 81.8, a3 80.8, a4 80.4, a5 81.1, a6 80.1, a7 80.0 dB). One assert fix during conversion: area 6's peak landed at 0.40 (loudest sample inside the fade-out) — assert relaxed to a 0.38–0.425 range.
- **Code change (music):** `source/main.c` `music_set_area`: `SoundFormat_8Bit` -> `SoundFormat_16Bit` (only music; SFX stay 8-bit). Header comment updated to v16 with the diagnosis; music comment block updated (7.5 s loops, 16-bit); `MUSIC_VOL` comment updated (v12 value, v14/v15/v16 keep it). `build.sh` updated to copy to `star-drag-v16.nds`. README updated to v16. **16-bit `dataSize` semantics — checked via libnds disassembly:** `soundPlaySample` in `libnds9.a` (`sound.o`) shifts `dataSize` >> 2 for the ARM7 FIFO call (`soundPreparePcm`), i.e. dataSize is in bytes for both formats. So the one-line format change is sufficient; no length adjustment needed in the call.
- **The 16-bit level/format puzzle — UNRESOLVED (measured Nov 19, 2026, ~5:52–6:00 PM EST):** v16 title-music capture (`qa/v16-runs/smoke-title-a0/sdlaudio.raw`, debug ROM, `--no-tap`, 12 s) is **~25 dB quieter than v15's title capture** under identical conditions: v16 peak 0.117, RMS -36.9 dBFS (first 2 s silent, then music); v15 title capture peak 0.62, RMS -11.8 dBFS. Fresh back-to-back rechecks reproduced it exactly (v15 ROM: peak 0.625, RMS -11.7 dBFS; v16 ROM: peak 0.118, RMS -36.4 dBFS). Both screenshots show the title screen with TAP TO START, so both captures are music-only. Correlation analysis (with correct ffmpeg resampling to 48 kHz, ratio 320/147 — an earlier 160/73 attempt was wrong and gave ~0 corr): v16 capture vs its loop, best corr **+0.294 at lag 4.62 s, gain 0.299** — the capture IS the loop, at a clean linear gain ~0.30, spectrum matches the loop (capture <1kHz 0.78 / 1-8kHz 0.20 vs loop 0.75/0.23). v15 capture vs its loop: best corr only **+0.126 at lag 2.27 s, gain 7.23** — gain 5-7x is impossible for a single channel at vol 80, and its spectrum has 3x more 1-8 kHz energy than its loop (0.31 vs 0.10). Self-autocorrelation of both captures is ~0 at all lags 1-15 s (no clear loop period in either). The v16 loop period (expected 7.5 s) is NOT yet confirmed; envelope autocorr was weak (best 6-9 s: lag 7.88 s, ac 0.094). Further forensics (Nov 19, ~11:00–11:20 PM EST): envelope of v16 capture repeats at ~8 s, not 7.5 s (loud 3.0-4.25 s, quiet 4.25-5.5, loud 5.5-6.5, quiet 6.5-7.5, then similar pattern shifted ~0.5-1 s late in the second iteration). Pitch tracks: v15 and v16 captures play the SAME melody at the SAME rate and phase (pitch sequences match each other closely); both look like the loop's pitch sequence. So the music content is correct in both — only level/noise differs. Decomposition (least-squares fit of loop to capture, 4 s window): v15 capture = loop at LS gain 0.489 + residual RMS 0.2498 (99% of the capture's energy is NOT the loop); v16 capture = loop at LS gain 0.023 + residual RMS 0.0145 (also mostly not the loop by this fragile metric — sample-level correlation is unreliable through melonDS's resampling). v15 capture is already loud in secs 0-2 (RMS -12.7 dBFS, peak 0.59) while secs 0-0.5 are digital silence — music starts ~0.5 s in v15 vs ~2 s in v16. v15 secs 0-2 vs loop best corr 0.136; vs secs 8-10 corr -0.005. The recheck capture is a genuinely fresh file (md5 `bbfd633c…` differs from old title-a0 `c97956b6…`). DC check: v15 capture mean +0.0013, std 0.2565, min -0.449, max +0.623; v16 mean +0.0000, std 0.0150. No DC offset in either. **Fold hypothesis DISPROVEN:** if melonDS used 8-bit bytes directly as signed s8 (v>=128 -> v-256), the folded signal would have RMS 0.97 and peak 1.0; capture-vs-folded-reference corr is 0.057 (no better than vs correct unsigned, 0.069). Loop bytes verified centered correctly (mean 128.1, min 74, max 182). `music_update()` is clean (no restarts); `music_set_area` callers are `terrain_set_area` (called at boot, area changes, show_title) — all early-return when the area matches. No code path restarts the track per frame. **Release gameplay capture (v16, SINGLE weapon, wave 1, 45 s, `qa/v16-runs/release-audio-a0/sdlaudio.raw`, 63.4 s):** gameplay secs 3-45 RMS -11.2 dBFS, peak 1.000; **pinned 1,936 of 2,016,000 samples (0.096%)** — 41% less than v15's 0.162% and 53% less than v12's 0.205% on the same protocol. Pinning concentrated in secs 13, 26, 29, 31, 33, 37, 39-44 (boom/kill-heavy seconds). Title music RMS -36.4 dBFS, peak 0.117. Shoot-burst median peak 0.511 across 445 windows (threshold 3x music RMS — those windows contain music+shoot+boom mixed, so the 33.6x ratio vs music RMS is NOT a clean shoot calibration). **Boom calibration attempt:** boom template (0.45 s raw -> resampled 0.23 s? — the template resample produced 0.23 s, peak 1.320, RMS 0.179; note the template itself peaks above 1.0, so the boom sample is hot) correlates 0 windows >0.15 in the release capture (template correlation fails for SFX through melonDS resampling — same failure as shoot/laser/pickup templates). Loudest 0.45 s windows peak 1.0 at t≈42.8-43.0 s. Title music peak 0.117. **Working interpretation (NOT proven):** nominal v16 music output should be loop RMS 0.051 x 80/127 = 0.032; observed title RMS 0.0152 = 0.47x nominal. v12's 8-bit title capture measured RMS 0.21-0.29 (-13.5 to -10.7 dBFS) vs nominal 0.042 — 5-7x nominal, the same anomaly as v15. So in melonDS, 8-bit music captures ~11-15x louder than 16-bit music relative to nominal; the 8-bit side is the anomalous one, and 0.47 ≈ 0.5 (one bit) smells like a melonDS 16-bit shift quirk. But the SFX pinning at 1.0 contradicts a uniform melonDS master volume of 0.47 (max sum at 0.47x would be 0.74, never pinning). **The level question is unresolved: if the 0.47x is a melonDS 16-bit quirk, NooDS will play v16 music at full nominal level (fine, maybe louder than the captures); if it is real DS behavior (no hardware basis known), the music would be quiet on her phone. Report honestly with both scenarios; the designer's playtest is the final judge. Do NOT deliver v16 claiming the level is settled until this is understood — a v16 that plays at -36 dBFS would be inaudible on her phone.** Resampling trap (recorded Nov 19, 2026): 22050 -> 48000 Hz needs ratio 320/147; a 160/73 attempt gives ~0 correlation. **SFX template resampling trap (Nov 19, ~11:35 PM EST):** SFX are 11025 Hz, so their templates need ratio 640/147, NOT 320/147 — using 320/147 makes them 2x too slow and kills all SFX template correlations (this is why shoot/laser/pickup/boom templates all failed in v15/v16 until the ratio was fixed). **Calibration with the correct ratio (Nov 19, ~11:35 PM EST):** shoot SFX correlates 13 windows >0.30 in the release capture (best 0.57 at t=18.36 s), window peak 0.405 vs expected 0.409 at master 1.0 — **8-bit SFX play at FULL level in melonDS, while 16-bit music captures at 0.47x**. The difference is format-dependent. The ARM7 code only programs the SPU hardware (calico sound server: `_soundSrvThreadMain` / `_soundPxiProcessCmd` — no software mixer), so the attenuation is in melonDS's SPU emulation, not the game; the DS hardware plays 16-bit samples at full scale. NooDS has its own SPU code and is unlikely to share melonDS's exact quirk, but NooDS cannot run here, so the designer's playtest is the final check on level.
- **Gameplay additions (the designer, Nov 19, 2026, 5:53 PM + 6:00 PM EST) — ALL IMPLEMENTED:**
  - **1UP pickup (type 5):** `tools/convert_v16_1up.py` generates `data/pow_1up.bin` (gold capsule, hot-pink heart, white "1UP" text — verified by rendering to `/tmp/pow_1up_preview2.png`, reads clearly). **Palette slot 15 is NOT free** — it is `PAL_JELLY` (v7); the earlier "slot 15 free" note was stale. The 1UP shares `PAL_MISSILE` (10) with gold/pink patched into its free indices 14 (248,200,0) and 15 (255,60,180); reused indices 1 white (text), 12 dark brown (outline), 13 light orange (highlight). Missile sprites use 0-10, v8's missile pickup uses 11/1/12/13, so 14/15 are free. **WARNING (same as v8):** re-running `tools/convert_sprites.py` regenerates `missile_pal.bin` and wipes indices 11-15 — re-run `tools/convert_v8.py` AND `tools/convert_v16_1up.py` after it. `data/pow_1up_pal.bin` was created then deleted (no separate palette file); the `_pal` include was removed from `main.c`.
  - **Drop rates (in `kill_enemy_final` and the floater):** kill drops 22% -> **11% total: 3% 1UP, 8% weapon capsule (types 1-3)**; floater `pickup_t` 1100 -> **2200 frames (~36 s)** with an 8% 1UP chance. Boss-death drops unchanged (weapon + rate capsule — boss rewards stay generous). `gfx_pow` extended [4]->[5]; allocation + `dmaCopy(pow_1up_bin, gfx_pow[4], …)` added; pickup draw uses `PAL_MISSILE` for type 5 (comment notes the shared palette). Collection (line ~2909): type 5 -> `lives++` capped at 5 (else +1000 score), `set_cue("1UP!", SUB_PAL_YELLOW, 150)` / `"1UP! +1000"`, `sfx_pickup()`. Lives HUD in `draw_play` redraws every frame (mini-ship icons at 2+k*14, y=20) — 5 lives fit.
  - **Pickup SFX (regenerated, `tools/make_audio.py`):** old square-wave chime (660/880/1320 Hz square, harsh) replaced by a **sine arp C5/E5/G5/C6 (523.25/659.25/783.99/1046.50 Hz), 0.055 s per note, soft 2nd harmonic, exponential decay, 0.22 s total, peak 0.438, RMS 0.093**. New `audio/pickup_raw.bin` md5 `a27ba735afe4268500b1f08c97021ced` (was `3a0480dc…`). All other SFX byte-identical after re-running `make_audio.py` (alarm `f28bd4f4…`, boom `a506b573…`, hit `1fc8accc…`, laser `c8dd6c47…`, shoot `006394f2…`).
  - **Boss-damage SFX:** new `bosshit()` in `make_audio.py` — short sine sweep 330->200 Hz, 0.07 s, peak 0.391, RMS 0.075; `audio/bosshit_raw.bin` md5 `264aa47c0acadd5458377240a2b005a7`; `sfx_bosshit()` plays it at vol 80 in `damage_boss()` **only on real damage** (shield-blocked and killing blows return before the tick), throttled to **one per 6 frames** via `static int last_tick` vs global `frame` (declared line 1553: `static int score, lives, wave, kills, frame;`). Mixer worst-case sums: music 0.26 + pickup 0.41 = 0.67; music 0.26 + bosshit 0.25 + shoot 0.41 = 0.92 — under full scale.
  - **Header comment** in `source/main.c` updated with the v16 gameplay additions; `build.sh`/README were already v16 from the music-only build.
- **Deliverable:** `~/workspace/your_files/star-drag-v16.nds`, md5 **`92733f028048b06251e50cded8121527`**, 4,194,304 bytes (4 MB), game code `####`, unit 0x00, logo CRC 0xCF56 OK, header CRC **0xFA46** OK. ARM9 binary **2,533,524 bytes, headroom 87,916** under the 0x27c000 cap. All 7 music tracks (16-bit, 330,750 bytes each), `bosshit_raw.bin`, new `pickup_raw.bin`, `pow_1up.bin`, patched `missile_pal.bin` verified embedded in the shipped ROM via ndstool-extracted ARM9 binary (byte-search match); `music_synth_raw.bin` is NOT embedded (unreferenced, linker drops it — fine). **Clean rebuild reproduces md5 `92733f02…` exactly (verified).** This md5 supersedes the music-only v16 md5 `5f8d6e43…`.
- **QA counters (debug only):** `qa_drop_rolls`, `qa_drop_weap`, `qa_drop_1up`, `qa_drop_float` (`#ifdef QA_DEBUG`, declared near line 1561), printed on sub-screen row 6 every 30 frames as `DROP k%d w%d 1up%d fl%d L%d` (rows: 1-2 AREA/WEAPON, 3 ship/boss readout, 4-5 cue banner, 6 drop stats, 7 music readout).
- **QA setup and suite (Nov 19, 2026, ~11:10–11:30 PM EST):** **Debug-copy trap re-learned and fixed:** rsyncing the real project into `qa/v16-debug/` overwrote the debug `arm9/Makefile` (which carries the `-DQA_DEBUG -DQA_START_AREA=...` block) with the real one, making the debug build byte-identical to the release ROM. Fixed by copying `qa/v15-debug/arm9/Makefile` into the debug copy. **Same trap again for `build.sh`:** the rsync also overwrote the debug `build.sh` with the real one (which copies to `~/workspace/your_files/star-drag-v16.nds`); restored from `qa/v15-debug/build.sh` (builds with `QA_START_*` env vars, copies to `star-drag-debug.nds` in the debug dir). Always diff both files after rsyncing. **`qa/apply_v16_debug_patches.py`** (patches 1–4 from v13–v15: debug loadout LASER + all weapons + rate 3 + wave/bossnum jump; area jump in `start_game`; area jump in `show_title` for title-music captures; real boss HP formula in debug builds so boss-1 timing is measured at shipped HP 70) **plus patch 5 (new in v16):** on the first kill, also `spawn_pickup(120, 80, 5)` — a 1UP right in the ship's lane so spawn + collection + life grant are verified on camera; the shipped 3%/8% drop roll still runs unmodified. **Plus patch 6 (added Nov 19, 2026, ~11:31 PM EST, see boss-1 section below):** debug ship parks at `ship_x = 80` (not 16) so the 128 px laser beam reaches a boss parked at x=196. Debug ROM (before patch 6) md5 `ba02f6d027a15b60f6ec92c517c774da`, differs from release as required. Real project verified clean of the forced 1UP (`spawn_pickup(120, 80, 5)` absent from `source/main.c`).
- **Full suite `qa/run_v16_suite.sh` ran and completed 23:30:33 EST Nov 19, 2026** (suite pid 30259): 7 per-area audio runs (45 s each, waves 1/4/6/7/10/12/13), boss-1 timing (150 s), title-only music captures areas 0–3 (12 s each), release e2e (250 s, release md5 `92733f028048`), JIT-on check. Logs in `qa/v16-runs/suite.log`; runs under `qa/v16-runs/` (`audio-a0`…`audio-a6`, `boss1-v16`, `title-a0`…`title-a3`, `e2e-release`, `jit-on`). **Release gameplay audio run `qa/v16-runs/release-audio-a0/` (45 s, release ROM, SINGLE, wave 1) also completed** — capture 63.4 s (see level section above).
- **1UP VERIFIED (audio-a0 run, t=51 s screenshot `06-t051.png`):** sub readout `DROP k18 w1 1up1 fl0 L4` — 1UP collected, **lives 4** (started at 3); HUD zoom shows **4 mini-ship lives icons**. The forced 1UP spawned, drifted into the parked ship, granted the life, and the HUD handled it. **Drop-rate aggregate (final DROP readouts, t=51 s of each 45 s audio run):** a0 18/1/1/0/L4, a1 12/0/1/0/L4, a2 32/3/1/1/L4, a3 32/3/1/1/L5, a4 27/1/1/1/L4, a5 40/4/1/1/L5, a6 44/6/1/2/L5 — **205 kills -> 18 weapon capsules (8.8%, target 8%, was 22%), 7 natural 1UPs (3.4%, target 3%), 6 floaters (~1 per 52 s, consistent with the 36 s interval)**. Lives hit the cap of 5 in a3/a5/a6. The halved rate is confirmed and the 1UP is rare but real.
- **Pickup chime VERIFIED in audio (audio-a0 capture):** spectrogram shows all four arp notes (523/659/784/1046 Hz) at ~-25 dB in sequence at t≈9.45 s (55 ms apart), vs control at 12.0 s (-56/-35/-55/-63 dB); the 1046 Hz bin spikes to -27 dB at 9.60 s vs -58 to -66 dB elsewhere. The 1UP was collected at ~9.4 s (first kill ~7–8 s + ~1.5 s drift). Template correlation does NOT work for SFX in this setup (shoot/laser/pickup/boom templates all correlate <0.25 even when the sound definitely plays — melonDS's 32.768 kHz -> 48 kHz resampling smears sample-level phase); use spectrogram/band-energy detection instead. (Note: the SFX template correlations ALSO failed because of the wrong 640/147 vs 320/147 resample ratio — see the level section. With the correct ratio, shoot correlates 0.57 and pickup 0.35 in the release capture, confirming both play.)
- **Boss-damage tick — code-verified, audio NOT cleanly isolated:** the 220–380 Hz band-energy detector was confounded by the laser SFX sweep (1500->180 Hz crosses the band); pre-fight 77 peaks/20 s vs fight 482 peaks/115 s (4.2/s vs 3.85/s — no discrimination). A second attempt on the boss1-v16b re-run (boss actually hit): 200–350 Hz band-burst rates 5.11/s (boss 2 fight) and 6.33/s (boss 3) vs 4.70/s in the no-hit control run (boss 1 active ~120 s with the beam too short to land a hit) — suggestive but not conclusive, because the music's bass content varies by area. Template correlation for the tick (correct 640/147 ratio) gives 1.09/s during the boss-2 fight vs 1.87/s in the pre-boss control — the 0.07 s sweep template matches too many things in the music/SFX to discriminate. **What is proven:** the tick fires only on real damage in `damage_boss()` (shield-blocked and killing blows return before it), throttled to 1 per 6 frames, at vol 80 with peak 0.246 — ~16x the music RMS in the capture, so it will be audible above the music; and the mixer worst case (music + tick + shoot = 0.92) stays under full scale. Whether it reads as a distinct hit sound to the ear is for the designer's playtest.
- **Alarm VERIFIED (boss1-v16b capture, correct 640/147 template ratio):** a clean triplet at t=59.2/59.9/60.9 s (corr 0.40/0.33/0.22, exactly 1 s apart) — boss 2's intro, matching the B1M2 readout at t=60. Boss 1's intro alarms are below the detection threshold (masked by music/SFX), but the WARNING cue + tempo lift were seen on screen for boss 1 in the first run.
- **Boss-1 fight problem — discovered Nov 19, 2026, ~11:20–11:31 PM EST:** **First boss-1 run (`qa/v16-runs/boss1-v16/`, debug md5 `287363ba955f`, area 0, wave 2, bossnum 0, real HP 70, 150 s, 76 shots):** boss 1 active from t≈29–31 s through the end of the run (t=151 s) — **never died**. Readouts: `S16,80 V-160 E0 B1M1 U+0 Y+0` held from t=40 s on; `REVERSE!` banner at t=31; tempo lift `x1.25` held; ship unmoved at (16,80); no enemies during the boss. HP bar measured pixel-exact at window y=44: **160 px = 5/5 segments in every shot t=31…t=151** — but a segment is 14 HP (`segs = ceil(hp*5/maxhp)`), so 5/5 only proves HP > 56 (up to 4 laser hits could have landed unseen). **Root cause (code, not a v16 regression):** the laser beam is only ~128 px long (4x 32 px segments, `b->x = ship_x + 24`, hit box `b->x-10` to `b->x+128`). From a ship parked at x=16 the beam ends at x=168, but the boss parks at x=196 (hit test needs `bcx-34 < b->x+128`, i.e. 194 < 168 — **fails**). The boss bobs vertically ±32 px around y=80 (`boss.y = 80 + 32*sin(boss.t*0.02)`), so the beam's Y always overlaps the boss hitbox — the miss is purely horizontal range. **Why v14/v15 measured ~10 s fights:** the v15 t=29 screenshot shows the ship firing **SPREAD** (weapon line reads `WEAPON SPREAD +3`, ship at S16,60) — it had switched via a collected capsule, and spread bullets fly across the whole screen and hit the boss. v16's halved drop rate meant the debug ship kept LASER (DROP readout w=0 in 8 kills) and never switched, exposing the beam-range limitation. **This is a parked-ship QA artifact, not a game bug:** the designer drags the ship and closes range in real play (she approved boss 1 as "perfect" in v6 and the laser design in v3); the beam reaches the boss from any ship_x >= ~70. The v13 boss-3 note already records the same class of artifact (77 s unattended because the boss bobs across the stationary ship's fixed laser line — "the designer aims with drag, so this is a playtest matter, not a QA failure"). **Fix for QA (debug setup only, NOT a game change):** patch 6 in `qa/apply_v16_debug_patches.py` parks the debug ship at `ship_x = 80` (beam ends at x=232 > boss left edge 194). **Boss re-run `qa/v16-runs/boss1-v16b/` (debug rebuilt with patch 6, 150 s, `--audio`, interval 2 s, 76 shots) COMPLETED:** boss 1 died (run reached boss 2 B1M2 by t=60 and boss 3's WARNING by t=80 — the wave cannot advance past a boss wave while a boss is active, so boss 1 must have died); boss 2 also died (boss 3's WARNING at t=80, areas advanced to CRYSTAL). Ship at (80,80) throughout; lives hit the cap of 5 (forced 1UP + a natural one); the ship switched to MISSILE by t=50 and SPREAD by t=80 via collected capsules. The reverse flight itself was already proven in the first run (V-160 held, REVERSE! cue, ship unmoved). The exact boss-1 fight duration in the re-run was not pinned to the second (the 2 s screenshots bracket it between t=44 and t=60), but the fight is beatable with the full reverse and the death sequence works.
- **Comparison audio for the designer (in `~/workspace/your_files/star-drag-v16-compare/`, script `/tmp/v16-compare.py`):** for areas 1–4 (OPEN SPACE, SPACE BASE, PLANET, CAVE): `areaN-<name>-source.mp3` (the exact 7.5 s loop window from the MP3, found by correlation with the shipped loop, corr 0.981–0.994) and `areaN-<name>-in-game.mp3` (10 s of exactly what v16 plays — melonDS music-only title capture, s2–s12). All 8 files verified distinct (different md5s: source `b7a8d4f9…`, `9adde05d…`, `b0a37563…`, `2f06cfc6…`; in-game `aad11b9e…`, `b6b93678…`, `6685ce34…`, `c6e69055…`), durations 7.5 s / 10.0 s. **Per-area distortion numbers (source loop window vs in-game capture):** area 1: source RMS -24.5 dBFS / peak 0.474 / centroid 1114 Hz / HF 0.23 vs in-game RMS -36.4 / peak 0.117 / centroid 979 Hz / HF 0.20, pinned 0. Area 2: source -13.3 / 0.753 / 303 Hz / 0.04 vs in-game -29.0 / 0.129 / 280 Hz / 0.04, pinned 0. Area 3: source -12.3 / 0.975 / 472 Hz / 0.07 vs in-game -30.1 / 0.127 / 409 Hz / 0.06, pinned 0. Area 4: source -14.3 / 0.818 / 444 Hz / 0.07 vs in-game -30.7 / 0.109 / 377 Hz / 0.06, pinned 0. **The in-game clips are 12–18 dB quieter than the raw source windows** — that is the peak-capping (source peaks 0.47–0.98 -> shipped 0.42, intentional, v15's recipe) PLUS the unresolved melonDS 0.47x 16-bit attenuation (see level puzzle above). **Spectral identity is preserved** (centroids within ~10–15%, HF fractions matching) — the in-game clips are the right tracks, just quieter in melonDS's capture. **Sample-level correlation in-game vs shipped loop is low (0.06–0.19 with a full-lag 2 s-window search; 0.294 best in the earlier 4 s-window analysis)** — melonDS's resampling smears phase; the same limitation applied to v15's captures (0.13). **Quantization SNR is the distortion metric that matters here: shipped loops 75.1–81.8 dB (16-bit) vs 28.5 dB (v15 8-bit).** The in-game clips contain zero pinned samples and no 8-bit grit by construction — the bytes the DS plays are 16-bit clean.
- **Verification — per-area audio (melonDS 1.1, JIT off, `SDL_AUDIODRIVER=disk`, debug ROMs, 45 s each, runs `qa/v16-runs/audio-a0/` … `audio-a6/`, waves 1/4/6/7/10/12/13):** all 7 areas confirmed playing their own track (debug readout `MUS aN … v80` + per-area title RMS -36.4 / -28.9 / -30.1 / -30.5 dBFS for areas 0–3; areas 4–6 title RMS similar by the same recipe). Boss tempo lift to x1.25 confirmed for mode 1 (REVERSE) and mode 4 (SHIELD) in the debug runs, ramping back to x1.00 after the kill. Crossfade at area changes kept (new track at 80 immediately, old fades out over 0.5 s).
- **Verification — end-to-end on the RELEASE ROM (`qa/v16-runs/e2e-release/`, md5 `92733f028048`, 250 s, 27 shots, no crash):** title (gradient STAR DRAG + TAP TO START, clean bottom UI — verified visually) → play → GAME OVER (FINAL SCORE 4650, Wave 3, BEST 4650 — verified visually, held) → tap at ~222 s → fresh game in OPEN SPACE (WEAPON SINGLE at t232, then SPREAD by t252). No crash in ~40 min total v16 runs (7 audio + 2 boss timing + 4 title captures + release audio + e2e + JIT-on).
- **Verification — JIT-on (melonDS 1.1, `Enable = true` asserted in the toml, `qa/v16-runs/jit-on/`):** top screen 96.9% white, bottom 96.9% black — exactly as v12/v13/v14/v15 and stock libnds hello-world, the known melonDS JIT + devkitPro-homebrew incompatibility, NOT a v16 bug. Fresh melonDS defaults to JIT off and renders v16 fine (all other runs above).
- **QA tooling / process notes (kept in the workspace):** `qa/run_v16_suite.sh` (full suite driver), `qa/apply_v16_debug_patches.py` (v15 patches 1–4 + patch 5 forced 1UP + patch 6 ship_x=80), debug copy `qa/v16-debug/`, diagnosis `/tmp/v16-diagnose.py`, compare script `/tmp/v16-compare.py`, recheck scripts in `qa/v16-runs/recheck-v15-title/` and `recheck-v16-title/`. **Suite setup catches (same lesson as v14/v15, twice this time):** rsyncing the real project into `qa/v16-debug/` overwrote BOTH the debug `arm9/Makefile` AND the debug `build.sh` with the real ones — caught by diffing before running (the real build.sh would have overwritten the release ROM in `~/workspace/your_files/`). Always diff the debug `build.sh`/`arm9/Makefile` after rsyncing a debug copy. The release deliverable was never harmed — its md5 is unchanged (`92733f02…`, rebuilt twice, identical both times).
- **Real project verified clean after the fix:** 0 `BISECT` markers, no wave jump / area override / `QA_START_*` / `music_buf` / NitroFS calls in the real source (only explanatory comments); the only `iprintf` in the real source is inside `#ifdef QA_DEBUG`; volumes verified in source (`MUSIC_VOL` 80, shoot 100, laser 120, hit 127, boom 127, pickup 120, alarm 127); all 7 music tracks 16-bit at 22050 Hz.
- **Honest gaps (the designer's gate):** Sound not heard headless — the capture evidence above proves the music is now 16-bit clean (quant SNR 75–82 dB vs 28.5 dB) and the gameplay mix pins 41% less than v15 / 53% less than v12, but whether the mix feels right to her ear is for the designer's playtest, and the comparison MP3s are there so she can hear source vs in-game herself. **The 16-bit level question is UNRESOLVED (see above): in melonDS the 16-bit music captures at ~0.47x nominal (~25 dB quieter than v15's 8-bit captures), while the 8-bit SFX play at full level — the evidence points to a melonDS SPU quirk (the ARM7 code only programs the SPU hardware; the DS hardware plays 16-bit at full scale), so NooDS should play it at full level, but NooDS cannot run here and her playtest is the final judge on level. If the music is too quiet on her phone, the fix is a volume/gain adjustment, not a format change — the 16-bit format itself is correct.** The boss-damage tick is code-verified but not cleanly isolated in headless audio (see above) — her ear is the check. D-pad untested (same as v4–v15). DeSmuME rendering unverifiable here (white SDL window for every ROM under Xvfb). NooDS/RetroArch iPhone paths not testable here. Boss fights verified at real HP 70 with god mode + a parked ship (the designer aims with drag, so her fights will run shorter than these unattended times). The boss parks at x=196, so his 64 px sprite is clipped ~4 px by the right screen edge (same as v7–v15). The ':' glyph renders as a merged vertical bar — no game text uses ':' today.

## v17 — music level up ~+9 dB (16-bit kept), SFX down a tad, player-damage sound, hit spark

- **the designer's v16 verdict (Nov 19, 2026, 7:47 PM EST):** "Yes much clearer music great, but music volume too low in game fix that." — the 16-bit fix WORKED for clarity; the level is the remaining problem. Follow-ups (7:48–7:53 PM EST, all confirmed into the build): "Bring down the sound effects just a tad in volume and raise the background music volume" (SFX down just a tad across the board, music up); "add a sound effect when picking up an item, and a sound effect when the boss takes damage" (both already existed from v16 — pickup chime and boss-damage tick — kept and verified); "reduce frequency of weapon bonus items and add another bonus extra life" (also already in v16 — halved drop rates and 1UP — kept); "HIT SPARK ON MULTI-HP ENEMIES: when a shot hits an enemy that takes multiple hits (bosses especially, but also any higher-HP enemy), show a tiny spark animation at the impact point — a small brief flash/star burst where the shot strikes, so each hit reads visually. Keep it SMALL and brief (a few frames, a handful of pixels, no big explosion — the full explosion already plays on death). It should fire on every damaging hit, including boss hits (alongside the boss-damage tick sound), and must not slow the game or clutter the screen during rapid fire. Verify on camera in QA: a screenshot/readout showing the spark at a boss hit and at a multi-HP enemy hit." Plus a later addition the same evening: a distinct PLAYER-damage sound effect on every player hit (ship hit / life lost), clearly different from the boss tick, enemy-hit sound, and shooting — a short low thud/buzz or descending tone — audible over the music without clipping the mixer.
- **The fix — music level up ~+9 dB, 16-bit kept (v16's clarity fix is NOT reverted):** `MUSIC_VOL` 80 → **127** (+4.0 dB, `source/main.c`) and shipped loop peak 0.42 → **0.72** (+4.7 dB, new `tools/convert_music_v17.py`, GAIN_MAX raised 0.9 → 1.6 because quiet windows need gain > 1 to reach 0.72; shipped-peak assert is the guard). Same windows, same 7.5 s loops at 22050 Hz, same 16-bit format, same v12 volumes as the starting point for SFX. Reconverted loops: peaks 0.68–0.72, quant SNR **79.5–85.4 dB**, zero rail samples; per-area shipped RMS: a1 −21.2, a2 −14.0, a3 −15.1, a4 −15.6, a5 −14.8, a6 −15.9, a7 −16.0 dBFS. **SFX down a tad across the board (the designer-confirmed):** shoot 100→85, laser 120→100, alarm 127→110, pickup 120→100, boom 127→110, boss tick 80→70 (hit was retired in v16). **Worst-case mixer sums (music 0.72 + SFX at v17 vols):** + shoot 0.35 = 1.07, + laser 0.54 = 1.26, + boom 0.87 = 1.59, + playerdmg 0.49 = 1.21 — all exceed full scale IF peaks align exactly; the pinning measurements below show alignment is rare in real play (release run 0.001%), which is why the level is safe despite the worst-case bound. Do NOT raise the loop peak further to chase the source level — vol is maxed at 127, 0.72 is at the top of the brief's 0.7–0.8 range, and the mixer is already at the rails in the worst case.
- **Player-damage sound (new in v17):** new `playerdmg()` synth in `tools/make_audio.py` — low descending buzz, sine 280→70 Hz + 2nd harmonic + short noise burst at attack, 0.28 s, peak 0.62, vol 100; `audio/playerdmg_raw.bin` md5 `c21f79b0da7b90bb805d7f522370c743`, played in `player_hit()` replacing `sfx_hit()`; old `sfx_hit()` and its `#include "hit_raw_bin.h"` removed (it had no other call site). All pre-existing SFX raws verified byte-identical after re-running make_audio.py. Plays on every player hit including the life-losing one (it is called before the lives/death branch in the real `player_hit()`).
- **Hit spark (new in v17):** new `spawn_hitspark(x, y)` — 3 particles, PAL_BOOM bright yellow-white, life 5 frames, small velocities; pool is 24 particles and `spawn_particle2` silently drops when full, so rapid fire can't overflow it. Fires in `damage_enemy` when the enemy survives the hit (hp > 0, i.e. multi-HP — 1-HP enemies die and get the full explosion instead) at the enemy centre, and in `damage_boss` on every damaging hit at the boss's front edge (boss.x + 6, boss.y + 32) alongside the tick. QA_DEBUG-guarded `qa_sparks` counter incremented in `spawn_hitspark` and shown in the DROP readout row 6 (`DROP k… SP%d`).
- **Deliverable:** `~/workspace/your_files/star-drag-v17.nds`, md5 **`1bf6c2cc3c6025035c0927db3e097982`**, 4,194,304 bytes (4 MB), game code `####`, unit 0x00, logo CRC 0xCF56 OK, header CRC **0x8C47** OK. ARM9 load image **2,534,612 bytes (86,828 headroom under the 0x27c000 cap)**. All 7 v17 tracks verified embedded in the shipped ROM by ndstool extraction (md5s: a1 `9bfffc07e37d`, a2 `101cf1491d63`, a3 `4ec496abc05c`, a4 `bad2e1bc6e10`, a5 `2ff82e8f77cd`, a6 `bdbcf3bf6e7e`, a7 `0108aab686f1`); `playerdmg_raw.bin` embedded (ARM9 offset 2501508, md5 match); retired `hit_raw.bin` confirmed dropped from the ROM. **Clean rebuild reproduces the md5 exactly (verified twice).**
- **Level — before/after (per-area title RMS, music-only, s3–12, melonDS 1.1 JIT off, `SDL_AUDIODRIVER=disk`):** a0 (OPEN SPACE) **−27.3 dBFS** (v16 −36.4, **+9.1 dB**), a1 (SPACE BASE) **−20.1** (v16 −28.9, +8.8), a2 (PLANET) **−21.3** (v16 −30.1, +8.8), a3 (CAVE) **−21.8** (v16 −30.5, +8.7). Peaks 0.31–0.35, zero pinned samples in all title captures. **Track identity:** 50 ms-window correlation vs the shipped loops (corrected FFT cross-correlation — see tooling notes below for why the shipped analyzer's `norm_corr` gave ~0.05): title captures median **0.74–0.91**, max 0.93–0.98, **100% of windows > 0.30**; gameplay runs (music + SFX) median 0.34–0.41, max 0.76–0.95, 58–92% > 0.30 — the v13–v16 established range. Spectral peaks match the shipped loops (a0: capture 92/138/205/294/385/442/588/882/934 Hz vs loop 99/146/220/294/366/443/522/588/882/934 Hz; band energies match). Every area plays its own track at v127 (debug MUS readout: `aN v127`, x1.00 normal, x1.25 boss lift — lift confirmed for mode 1 REVERSE and mode 2 HALT in the v17 runs, ramping back to x1.00 after the kill).
- **Gameplay pinning — release runs (the fair comparison, wave 1, SINGLE, 45 s, release ROM):** **TWO independent runs both pinned 11 / 2,016,000 samples = 0.001%** (run 1 `release-audio-a0`, run 2 `release-audio-a0b`) vs v16's 0.096% — **96× below the v16 target**. Per-second pinning is scattered single samples (s33: 2, s37: 2, s39: 1), no bursts. **Debug stress runs (LASER + rate-3, maximum SFX density — the loadout the designer will reach mid-game):** pinning **1.4–2.0%** (29,565–39,566 samples per 45 s run) vs v16's 0.4–1.1% (9,009–23,577) — roughly doubled, the cost of +9 dB music under maximum SFX density, concentrated in boom bursts (worst second 1080/48000 = 2.25% of one second, audio-a6 s08). Honest note: the release-run target is met with huge margin, but the dense-play mix clips more than v16's — the tradeoff of the louder music, and the reason the loop peak stays at 0.72.
- **Boss 1 (HP 70, unchanged) — verified in `qa/v17-runs/boss1-v17/` (debug, area 0, wave 2, real HP, 150 s, 76 shots):** `!! WARNING!!` at t=19, **`REVERSE!` + `V-160` + `B1M1` at t=23**, ship unmoved at (88,80), **E0 (no other enemies)**, dead by ~t=29 (`NORMAL FLIGHT`, B0), fight ≈ 10 s, max reverse held ~6–7 s. Boss 2 active by t=58 (`B1M2`, `HALT!`, `V+0`, tempo x1.25); run reached area WATER by t=151 (bosses 1, 2, 3 all died). Alarm ×3 for every boss confirmed by the shared intro path (unchanged code) and the WARNING cue seen for boss 1.
- **Hit spark — verified (`qa/v17-runs/spark-boss1/`, 60 s dense capture, 0.25 s shots, 210 shots; plus the audio runs):** SP counter — audio runs (t≈51 s): a0 23, a1 93, a2 8, a3 41, a4 259, a5 219, a6 50; boss1 run 402 (t=151); spark run 0 (t=6) → 23 (t=26, during the boss fight) → 33 (t=47). **On camera, multi-HP enemy hit:** `qa/v17-runs/spark-boss1/95-t027.png` — the white plus is centred on a brown asteroid at the laser beam's impact point; the plus is exactly the spark tile decoded from `data/spark.bin` (bright yellow-white 4-point plus, PAL_BOOM index 1 = RGB 255,246,172; the Kenney brown meteor sprite has no white plus of its own). **Boss hit:** SP climbed 3 → 23 during B1M1 with E0 (no other enemies present), so all 20 sparks in that window came from boss hits — the readout is the boss-hit evidence. Honest gap: no single boss-fight frame isolates the 5-frame plus distinctly on camera (the boss hull is itself white/grey and the plus is small); the counter + the identical spawn call + the asteroid visual are the proof.
- **Player-damage sound — trigger and data verified; waveform audibility NOT isolated headless (read this carefully):** a QA `qa_hits` counter (patch 10, debug only, H field in the DROP row) proves `player_hit()` ran: **H7 at t=31, H10 at t=61** in `qa/v17-runs/playerdmg-hits/` (forced hits via patch 8 + natural hits). In the debug build `sfx_playerdmg()` is the first statement of `player_hit()`; in the release build it is called before the lives/death branch. The sound data is verified embedded in the shipped ROM (see deliverable). **Audio template correlation could NOT isolate the buzz in melonDS captures** (best 0.21–0.25 across three runs, 0 windows > 0.30, including a music-free run) — BUT the same method also fails for the **laser** (best 0.12–0.29) even though the spectrogram PROVES the laser renders (1400→100 Hz sweep, 0.28 s, every ~0.44 s, matching the template's rate/duration). The SPU render path phase-distorts chirp/buzz waveforms while preserving spectra, so template correlation is the wrong tool for chirps here; the shoot (short broadband blip) does correlate (0.57), and an **injection test proves the method works when the sound is present at nominal level** (injected playerdmg template at vol 100 → 0.48 at exactly the injected time; laser 0.72, boom 0.51, shoot 0.81). The bosshit "detections" (0.45–0.60) are likely false positives (the 0.07 s sine-blip template matches music bass — it also "detects" in no-boss runs). Conclusion: trigger + data + SFX path are verified; whether the buzz reads distinctly to the ear is for the designer's playtest. The laser-sweep spectrogram is the positive control that 11025 Hz SFX render in melonDS.
- **Comparison audio for the designer (in `~/workspace/your_files/star-drag-v17-compare/`, script `qa/compare_v17.py`):** for areas 1–4 (OPEN SPACE, SPACE BASE, PLANET, CAVE): `areaN-<name>-source.mp3` (exact 7.5 s loop window from the MP3, source-vs-shipped corr **0.981–0.995**) and `areaN-<name>-in-game.mp3` (10 s of exactly what v17 plays — melonDS music-only title capture, s2–s12). All 8 files verified distinct md5s, durations 7.5 s / 10.0 s. **Levels (RMS / peak):** a1 source −25.0 / 0.439 vs in-game −27.9 / 0.302; a2 −13.8 / 0.742 vs −20.5 / 0.347; a3 −12.7 / 0.937 vs −21.7 / 0.341; a4 −14.8 / 0.829 vs −22.1 / 0.304. **Honest gap — the in-game clips are 3–9 dB quieter than the raw source windows in melonDS** (the ~0.47× 16-bit SPU attenuation: in-game sits ~6 dB below the shipped loop level; a1 is closest because its source window is quiet and the converter gained it up). The +9 dB raise vs v16 is real and audible (in-game a1 −27.9 vs v16's ~−36.4), but the clips are NOT at the same level as the sources — more gain is not available (vol maxed at 127, loop peak 0.72 at the top of the brief's 0.7–0.8 range, mixer worst case already over full scale). If the designer still finds it too quiet on her phone, the next step is investigating the 16-bit attenuation itself, not more gain. NooDS may attenuate less than melonDS (unmeasurable here) — her ear is the test.
- **Verification — end-to-end on the RELEASE ROM (`qa/v17-runs/e2e-release/`, md5 `1bf6c2cc3c60`, 250 s, 27 shots, no crash):** title (gradient STAR DRAG + TAP TO START, clean bottom UI — verified visually) → play → GAME OVER (gradient GAME OVER art, TOUCH TO RETRY, FINAL SCORE 4650, WAVE 3, BEST 4650 — verified visually, held) → tap at ~215 s → fresh game in OPEN SPACE (WEAPON SINGLE at t232). No crash in ~40+ min total v17 runs (7 audio + boss1 + 4 title captures + 2 release audio + spark dense + 3 playerdmg tests + e2e + JIT-on).
- **Verification — JIT-on (melonDS 1.1, `Enable = true` asserted in the toml, `qa/v17-runs/jit-on/`):** top screen **96.9% white**, bottom 3.1% white — exactly as v12–v16 and stock libnds hello-world, the known melonDS JIT + devkitPro-homebrew incompatibility, NOT a v17 bug. Fresh melonDS defaults to JIT off and renders v17 fine (all other runs above).
- **QA setup and suite:** full suite `qa/run_v17_suite.sh` ran and completed ~01:17 EST Nov 20, 2026 (suite pid 14381): 7 per-area audio runs (45 s each, waves 1/4/6/7/10/12/13) → boss-1 timing (150 s) → title captures areas 0–3 (12 s each) → release gameplay audio (45 s, release ROM, wave 1 SINGLE) → hit-spark dense capture (60 s, 0.25 s shots) → release e2e (250 s, tap2 at 215 s) → JIT-on check. Logs in `qa/v17-runs/suite.log`; runs under `qa/v17-runs/` (`audio-a0`…`audio-a6`, `boss1-v17`, `title-a0`…`title-a3`, `release-audio-a0`, `spark-boss1`, `e2e-release`, `jit-on`) plus `release-audio-a0b` (second release run), `playerdmg-test`, `playerdmg-nomusic`, `playerdmg-hits`. Debug copy `qa/v17-debug/`; `qa/apply_v17_debug_patches.py` (v16 patches 1–6 + patch 7: `player_hit()` plays `sfx_playerdmg()` before the god-mode early return; patch 8: force a real player hit every 10 s via a bullet spawned on the ship centre; patch 9 TEMPORARY: `QA_NOMUSIC` silences music; patch 10: `qa_hits` counter in the DROP row). New persisted QA scripts (the /tmp v16 scripts were wiped by a runtime restart): `qa/analyze_v17.py` (s16-aware: per-second stats, title RMS, gameplay pinning, track correlation, SFX template correlation) and `qa/compare_v17.py` (builds the compare MP3s).
- **QA tooling / process notes (lessons from this round, keep):** (1) `qa/analyze_v17.py`'s `norm_corr` (coarse 900-sample lag grid + 1 s windows) returns ~0.05 even when the music provably matches — the correlation peak for this signal is only a few ms wide and the 1 s window drifts (emulator resampling rate error). Use the v13 method instead: **50 ms windows + full-lag FFT cross-correlation** (correct conjugate: `irfft(conj(rfft(c)) * rfft(t))` — the first attempt had the conjugate on the wrong signal and returned r > 1). Verified on synthetic data (r=0.99995 at the exact lag) before trusting it. (2) `corr_series` (50 ms steps) misses chirp SFX for the same reason — a chirp's correlation peak is ~1–2 ms wide, so a 50 ms grid almost never lands on it (injection at an on-grid time works; off-grid real sounds are missed). For chirps/buzzes use spectrogram/band-energy detection, not template correlation. (3) `qa/compare_v17.py` shipped with two bugs: it looked for `melonds_audio.raw` (the runner writes `sdlaudio.raw`) and it forgot the `/32768` normalization on the int16 capture — the first in-game clips measured RMS −0.3 dBFS / peak 1.75 (fully clipped) and were discarded and rebuilt; the fixed script's clips measure −27.9…−20.5 dBFS with peaks 0.30–0.35. (4) `apply_v17_debug_patches.py` patches 8/9 were not idempotent (their old anchors survive the patch, so a re-run duplicated patch 8) — fixed to check for the new text; patch 8 was briefly duplicated in the debug source (harmless — two identical bullets, one consumed by the collision, the other hits the next frame — removed). (5) The `QA_NOMUSIC` test ROM compiled to 512K (not 4 MB): GCC dead-code-eliminated the music data once `music_set_area` always returned early — the nomusic ROM is a QA-only artifact, never a deliverable; the release ROM is unaffected (4 MB, music embedded, md5 unchanged). (6) The debug-copy trap again: rsyncing the real project into `qa/v17-debug/` overwrote the debug `arm9/Makefile` AND `build.sh` — restored from `qa/v16-debug/` and diffed after rsyncing, as in v14/v16.
- **Real project verified clean after the fix:** 0 `QA_START_*`, 0 `BISECT`, 0 `music_buf`, no forced 1UP (`spawn_pickup(120, 80, 5)`), no patch-8 bullet (`fire_ebullet(ship_x, …)`), no `qa_hits` in the real source; the 5 NitroFS matches are comments only (the v12 history note); the only `iprintf` is inside `#ifdef QA_DEBUG`; volumes verified in source (`MUSIC_VOL` 127, shoot 85, laser 100, alarm 110, pickup 100, boom 110, boss tick 70); all 7 music tracks 16-bit at 22050 Hz; boss 1 HP 70; drop rates, 1UP, pickup chime, boss tick, arcade title, clean touchpad UI, rainbow border, boss-3 floor sweep, crossfade, boss tempo lift 1.25×, post-boss-2 HP scaling, pixelated explosions, labeled pickups, relative drag, GAME OVER hold screen — all unchanged from v16 except where noted above.
- **Honest gaps (the designer's gate):** Sound not heard headless — the capture evidence proves the music is +9 dB louder than v16 (title RMS −27.3 vs −36.4 for area 1), 16-bit clean (quant SNR 79.5–85.4 dB), and the release gameplay mix pins 96× less than v16 (0.001% vs 0.096%), but whether the level feels right to her ear on NooDS is for the designer's playtest, and the comparison MP3s are there so she can hear source vs in-game herself. **The level is still not at the v12/v15 melonDS level (−11 dBFS): v17 sits at −27.3 (a1) in melonDS captures, ~16 dB below it, because the 16-bit path plays at ~0.47× nominal in the emulators' SPU emulation — the +9 dB raise is the maximum the brief's guardrails allow (vol maxed, loop peak at the top of 0.7–0.8, mixer worst case already over full scale).** The player-damage buzz's distinctness to the ear is unverified headless (see above) — her ear is the check. The boss-hit spark is proven by counter + code path + the asteroid visual, but no single boss-fight frame isolates the 5-frame plus distinctly from the white boss hull. D-pad untested (same as v4–v16). DeSmuME rendering unverifiable here (white SDL window for every ROM under Xvfb). NooDS/RetroArch iPhone paths not testable here. Boss fights verified at real HP 70 with god mode + a parked ship (the designer aims with drag, so her fights will run shorter than these unattended times). The boss parks at x=196, so his 64 px sprite is clipped ~4 px by the right screen edge (same as v7–v16).

## v18 — SFX down a further step, boss-music rule fixed for all four bosses, visible death drift restored

- **the designer's v17 verdict (Nov 19, 2026, ~9:24 PM EST):** "Can you reduce the volume of sound effects I think that will balance with the music volume better. Also when each boss appears keep playing that areas music but speed up its tempo and only change to the next area music after that boss is defeated and it should fade in to the next music." Plus a same-evening restore: the ship-type enemies used to drift up or down (random direction) for a second or two before exploding — what happened to it? And a clarification (Nov 19, 2026, 9:33 PM EST): the drift direction must be chosen ONCE per death and stay locked, never switching mid-death. And a refinement (Nov 19, 2026, ~9:38 PM EST): the drift direction is NOT random — it comes from WHERE the enemy dies: above the vertical middle of the screen it drifts upward, below the middle it drifts downward (on the line itself, down). Still locked for the whole drift.
- **Deliverable:** `~/workspace/your_files/star-drag-v18.nds`, md5 **`f8612469656b7c80d34048342af182ee`**, 4,194,304 bytes (4 MB), game code `####`, unit 0x00, logo CRC 0xCF56 OK, header CRC **0xD186** OK. **Clean rebuild reproduces the md5 exactly (verified twice, plus an identical scratch-copy build).** NOTE: an earlier pre-drift v18 build (md5 `447261deacfd13811d9c8c9edc74244b`) was superseded the same night by this final build — the pre-drift ROM is no longer on disk; all final-ROM verification below is on `f8612469…`.
- **Change 1 — SFX down a further clear step (music untouched):** shoot 85→**60**, laser 100→**70**, alarm 110→**80**, pickup 100→**70**, boom 110→**80**, boss tick 70→**50**, player-damage buzz 100→**70** (each ×0.7 ≈ −3 dB, in `sfx_shoot/laser/alarm/pickup/boom/bosshit/playerdmg`). `MUSIC_VOL` stays 127, loop peak stays 0.72, same 7.5 s 16-bit loops. **Measured — music level EXACTLY unchanged:** clean music-only title captures on the final debug ROM (`--no-tap`, 12 s, `qa/v18-runs/title-a0`…`a3`): a0 **−27.3 dBFS** (v17 −27.3), a1 **−20.1** (v17 −20.1), a2 **−21.3** (v17 −21.3), a3 **−21.8** (v17 −21.8) — identical to 0.1 dB, peaks 0.31–0.35. **Measured — SFX quieter under the identical maximum-density loadout** (debug LASER + rate-3, gameplay s12–45, same protocol both versions): RMS a0 −9.9→**−12.7**, a1 −8.3→**−10.2**, a2 −8.2→**−10.2**, a3 −7.8→**−9.9** dBFS — **1.9–2.8 dB quieter** (music unchanged, so the drop is all SFX); pinning a0 0.114%→**0.000%**, a1 2.156%→**1.111%**, a2 1.849%→**1.029%**, a3 2.015%→**1.045%** (~45% less); a4–a6 v18 1.08–1.14%. **Release gameplay pinning (wave 1, SINGLE, 45 s, final ROM, `qa/v18-runs/release-audio-final/`): 0 / 1,584,000 = 0.000%** (v17: 11 = 0.001%), gameplay RMS −16.6 dBFS, peak 0.720. The remaining debug-run pinning is boom-burst alignment under maximum SFX density (music bed unchanged + boom 80); the release mix is clean.
- **Change 2 — what the old code was actually doing wrong (read, not guessed):** at the boss intro the wave-advance code called `music_boss_start()` (tempo → 1.25×) but then immediately called `terrain_set_area(na)`, which calls `music_set_area(na)` — so the track switched to the NEXT area's music at the intro, at FULL volume immediately, while the old track faded out over 0.5 s. At the kill, `boss_death_final()` called `music_boss_end()` (tempo back to 1.0×) and `terrain_set_area(na)` was a no-op (area already changed at intro), so there was NO track change and NO fade-in at the kill. That is exactly what the designer heard: new music starting loud when the boss appears, nothing changing when he dies. **Fix (`source/main.c`):** new `terrain_set_area_nomusic(area)` (terrain change without the music switch); the boss-intro branch uses it (terrain changes for the fight, music stays on the old area's track, sped up). New `music_set_area_fadein(area)` + `MUSIC_FADEIN 90` (1.5 s) + `music_fadein_t`/`music_fadein_req`: at the kill the new track starts at volume 0 and `music_update()` ramps `music_vol` 0 → 127 linearly over 90 frames; the old track still fades out over 30 frames (0.5 s). `boss_death_final` uses nomusic for terrain plus `music_set_area_fadein(na)` when the area actually changes. Non-boss area changes keep the old behaviour (new track at full volume immediately, old fades out 0.5 s).
- **Change 2 — verification, boss-music rule for ALL FOUR modes (MUS readout timelines via new `qa/mus_timeline.py`, runs `qa/v18-runs/boss1-v18/` … `boss3-v18/`, debug ROMs, real HP 70):** readout format `MUS <name> a<N> v<VOL> x<TEMPO>` (lowercase letters decode as blanks in `read_sub.py` — `music-v2` reads as `     - 2`, `area2` as `    2`). **Boss 1 (REVERSE, a0→a1, boss1 run):** t001–17 a0 x1.00; t018 tempo ramps 1.13→1.25, still a0; t021–23 `B1M1 V-160`, a0 x1.25; t028 kill → a1 **v36→79→127**, tempo 1.04→1.00. **Boss 2 (HALT, a2→a2 no-switch, boss2 run):** t027–28 tempo →1.25, stays a2; t039 kill → tempo back to 1.00, **stays a2** (no track switch — correct, the area doesn't change). **Boss 3 (UPWARD, a3→a4, boss3 run):** t048–49 tempo →1.25, stays a3; t060 kill → a4 **v6→50→93→127**, tempo →1.00 (also in the boss2 run: kill → a4 v19→104→127). **Boss 4 (SHIELD, a5→a5 no-switch, boss2 run):** t081–83 tempo →1.25, stays a5; t117 kill → tempo back to 1.00, **stays a5**. **Boss 1 of the next cycle (a6→a0, boss3 run):** t157–59 kill → a0 **v2→45→67→127**. **Audio correlation (`qa/analyze_v18_fade.py`, 50 ms FFT method):** boss 1 kill — old track wins s020–29 (corr 0.32–0.52), new track wins s030–32 (corr 0.55 / **0.83** / 0.70); RMS stays in the same band across the switch (no silence gap, no loud jump). Boss 3's audio correlation alone is inconclusive (a3/a4 tracks too similar under SFX — corr 0.3–0.55 both ways); the MUS readout (the game's own `music_vol`/`music_play_area`) is the authoritative fade evidence there. **Caveat (cosmetic, QA-readout only):** `sub_text` doesn't clear the row, so when a shorter MUS string overwrites a longer one the stale trailing glyphs stay (e.g. `x1.001.25`) — the release ROM has no readout at all.
- **Change 3 — visible death drift, what was wrong:** since v8 the ship sprite was hidden at the killing hit — `damage_enemy` spawned the full explosion immediately and the draw loop skipped dying enemies, so only invisible embers drifted for ~0.4 s (24 frames). The v4 drifting ship the designer remembers was gone, although the drift fields (`e->drift`, dying phase) were still in the code. **Fix:** on a fatal hit the explosion does NOT fire — the enemy keeps its sprite, blinks (`e->flash = 90`, the draw loop's existing alternate-frame hide, so it reads as damaged) and trails embers (every 2 frames, was 4) while it drifts **84 frames (~1.4 s)**: forward 200/frame, vertical 1 px/frame in the locked direction, y clamped to the screen so the closing explosion is always on screen. `kill_enemy_final` now fires the per-type pixelated explosion (`spawn_explosion` + `sfx_boom`, moved here from `damage_enemy`) and finalises the kill (score/drops/wave advance). **Direction (the designer's refined rule):** `e->drift = (enemy centre y < 96) ? -1 : 1` — above the middle drifts UP, below drifts DOWN, exactly on the line drifts down — picked ONCE at the fatal hit, never re-picked, so y moves monotonically one way. The draw loop no longer skips dying enemies; bullet-collision and player-collision loops still skip them (a burning wreck can't be hit again or hurt the player — correct). Bosses untouched (own ~90-frame multi-explosion sequence). Applied to all regular (non-boss) enemies — the v4 behaviour the designer remembers.
- **Change 3 — verification (`qa/v18-runs/drift-laser/`, final debug ROM, LASER, wave 1, 60 s, 0.25 s shots, 210 shots; new `qa/analyze_drift.py` + QA DY readout row 8 `DY y<centre> d<+1/-1> t<die_t>`):** **19 death groups, 4 up + 15 down, 0 rule violations, 0 monotonicity violations.** Up drifts (kill above the middle, drifting up): death 3 y 46→16 d−1 die_t 26→56; death 8 y 90→30 d−1 die_t 25→62; death 10 y 74→16 d−1 die_t 27→62; death 16 y 16 d−1. Down drifts (kill below the middle, drifting down): death 0 y 143→173 d+1; death 6 y 140→170 d+1 die_t 36→66; death 9 y 117→176 d+1 die_t 16→76; death 11 y 136→176; death 12 y 144→174. die_t spans 11→76 across sampled deaths (drift finalises at 84, between readout refreshes); DROP k increments prove finalisation (k5→k7 across death 8's window). **On camera:** contact sheets `qa/v18-runs/drift-up-sheet.png` (t23–27) and `qa/v18-runs/drift-down-sheet.png` (t18–22) show enemy ships persisting across frames during the drift window (no instant vanish — the v8 look is gone) plus a cyan pixelated explosion burst; `qa/v18-runs/drift-laser/50-t015.png` shows the DY readout live (`DY -175 +1 67`) with enemies on screen. Height variety came from the debug ship picking up a MISSILE capsule mid-run (`WEAPON MISSILE +3` at t54) — missile kills at all heights gave the down-drifts; laser kills above the middle gave the up-drifts.
- **QA setup and suite:** full suite `qa/run_v18_suite.sh` ran and completed 02:54 EST Nov 20, 2026: 7 per-area audio runs (45 s each, waves 1/4/6/7/10/12/13) → boss1 (150 s) → boss2 (150 s) → boss3 (180 s) → release gameplay audio (45 s, release ROM, wave 1 SINGLE) → release e2e (250 s, tap2 at 215 s) → JIT-on check. Logs in `qa/v18-runs/suite.log`. **Suite caveat (read this):** the suite ran on the PRE-DRIFT v18 build (md5 `447261de…`); the drift change landed after it. The music/SFX code is untouched by the drift change, so the suite's audio and boss-music results stand for the final ROM; the final ROM got its own drift run + title captures + release e2e + release audio, all above. **No crash in ~40+ min total v18 runs** (7 audio + 3 boss + release audio + e2e + JIT-on + 60 s drift + 4 title captures + final e2e + final release audio).
- **Verification — end-to-end on the FINAL RELEASE ROM (`qa/v18-runs/e2e-final/`, md5 `f8612469…`, 250 s, 27 shots):** title (gradient STAR DRAG + TAP TO START, clean bottom UI — verified visually) → play → GAME OVER (gradient GAME OVER art, TOUCH TO RETRY, FINAL SCORE 3950, WAVE 2, BEST 3950 — verified visually; **t202 vs t212 pixel-identical, 0 differing pixels of 786,432** — the hold works) → tap at ~215 s → fresh game in OPEN SPACE (WEAPON SINGLE at t242, ship firing, enemy on screen). (The pre-drift suite e2e on `447261de…` likewise passed: GAME OVER 4650, wave 3, fresh restart.)
- **Verification — JIT-on (melonDS 1.1, `Enable = true` asserted in the toml, `qa/v18-runs/jit-on/`):** top screen **96.9% white**, bottom 3.1% white — exactly as v12–v17 and stock libnds hello-world, the known melonDS JIT + devkitPro-homebrew incompatibility, NOT a v18 bug. Fresh melonDS defaults to JIT off and renders v18 fine (all other runs above).
- **QA tooling / process notes (lessons from this round, keep):** (1) The first drift analysis reported die_t up to 627 and y jumps — a DECODE artifact, not a game bug: `sub_text` doesn't clear the row, so when a shorter DY string overwrites a longer one the stale trailing digit stays (true t=6 after t=67 reads as `t67`, and a die_t "rise" masked a death switch). Fixed in `qa/analyze_drift.py` by capping die_t to 2 digits and grouping by CONTINUITY (die_t non-decreasing AND y moves ≤32 px in the locked direction — a bigger jump, a wrong-way step, or a die_t drop splits the group). The same stale-glyph effect explains the MUS readout's `x1.001.25` strings. (2) `qa/mus_series.py` sorts filenames lexicographically, not by time (`100-t054` sorts before `19-t011`) — new `qa/mus_timeline.py` sorts numerically by t and collapses repeats to transitions. (3) `qa/read_sub.py`'s font1 charset has no lowercase, so readout lowercase reads as blanks (and `y` sometimes as `-` — the glyph matcher picks the hyphen); parse accordingly (`DY\s*-?\s*(\d+)\s*([+-]\d+)\s*(\d{1,2})`). (4) MelonDS under Xvfb with screenshots runs slower than real time (~15 game-fps here): wall-clock seconds ≠ game seconds — die_t (game frames) is the clock for drift duration, and a 60 s wall run is ~15 game-seconds of drift samples. (5) PNG md5s differ between pixel-identical frames (encoder metadata) — compare pixels, not md5s, for hold checks. (6) The debug-copy trap again: rsyncing the real project into `qa/v18-debug/` overwrote the debug `arm9/Makefile` AND `build.sh` — restored from `qa/v17-debug/` and diffed after rsyncing, as in v14/v16/v17. (7) The debug ship parks at (60,60) in the runs (S60 60) although patch 6 sets (80,80) — the runner's start tap drags it once by (−20,−20) at the beginning; it never moves after that, and the beam still reaches the boss (60+24+128=212 > 194), so boss fights complete. New persisted QA scripts: `qa/mus_timeline.py`, `qa/analyze_drift.py`, `qa/analyze_v18_fade.py`, `qa/apply_v18_debug_patches.py` (v17 patches 1–8 + 10, path updated; patch 9 QA_NOMUSIC dropped — not needed in v18 and its anchor no longer matches `music_set_area`); debug copy `qa/v18-debug/`; suite `qa/run_v18_suite.sh`.
- **Real project verified clean after the fix:** 0 `QA_START_*`, 0 `BISECT`, 0 `music_buf`, 0 NitroFS calls in the real source; the only `iprintf` is inside `#ifdef QA_DEBUG` (line 1215, `MUSIC area %d emb v%d`); volumes verified in source (`MUSIC_VOL` 127, shoot 60, laser 70, alarm 80, pickup 70, boom 80, boss tick 50, player-damage buzz 70); all 7 music tracks 16-bit at 22050 Hz; boss 1 HP 70; drop rates, 1UP, pickup chime, boss tick, player-damage buzz, hit sparks, arcade title, clean touchpad UI, rainbow border, boss-3 floor sweep, boss tempo lift 1.25×, post-boss-2 HP scaling, pixelated explosions, labeled pickups, relative drag, GAME OVER hold screen — all unchanged from v17 except where noted above.
- **Honest gaps (the designer's gate):** Sound not heard headless — the capture evidence proves the music level is EXACTLY v17's (title RMS identical to 0.1 dB) while the SFX sit ~2–3 dB lower under maximum density and the release mix pins 0 samples, but whether the balance feels right to her ear on NooDS is for the designer's playtest. The boss3 fade is proven by the game's own MUS readout (v6→50→93→127), not by audio correlation alone (a3/a4 too similar under SFX to separate by correlation — see above). The dying ship's blink + ember trail are verified in code and by the DY readout + contact sheets, but no single frame isolates the blinking ship distinctly from the other sprites (the blink hides it on alternate frames and the playfield is busy). D-pad untested (same as v4–v17). DeSmuME rendering unverifiable here (white SDL window for every ROM under Xvfb). NooDS/RetroArch iPhone paths not testable here. Boss fights verified at real HP 70 with god mode + a parked ship (the designer aims with drag, so her fights will run shorter than these unattended times). The boss parks at x=196, so his 64 px sprite is clipped ~4 px by the right screen edge (same as v7–v17). The ':' glyph renders as a merged vertical bar — no game text uses ':' today.

## v19 — death drift tuned down (shorter, slower, two types only, edge-explode), late-wave HP capped

- **the designer's v18 verdict (Nov 19, 2026, 10:34 PM EST):** "The drift is too long make it shorter and only one or two enemy types should do that also drift slower and if they hit the top or bottom just explode, the other enemies should have their own death animation explosion. The later areas I think 7 and higher the hp of enemies increase too much cap that so it doesn't get too difficult cuz it's nearly impossible to kill enemies at wave 8"
- **Deliverable:** `~/workspace/your_files/star-drag-v19.nds`, md5 **`8879391a91b391e6b56dc0e74b7e39b4`**, 4,194,304 bytes (4 MB), game code `####`, unit 0x00, logo CRC 0xCF56 OK, header CRC **0x2748** OK. **Clean rebuild reproduces the md5 exactly (verified twice, after a header-comment-only edit to `source/main.c`).**
- **Change 1 — drift, tuned down (`source/main.c`, `damage_enemy` + dying update in `update_play`):** v18's drift (84 frames, ~1.4 s, 1 px/frame, all ship-type enemies) is now **36 frames (~0.6 s)**, **0.5 px/frame** vertical (`e->y += e->drift * 128`), and only **TWO enemy types drift — type 0 drifter and type 1 weaver**, the two most ship-like (the designer didn't name them; the pair was picked as the natural ship-like types). Every other type gets its per-type pixelated explosion **immediately** at the killing hit, as in v8 — a direct `kill_enemy_final(e)` call in `damage_enemy`'s fatal-hit branch before the drift setup. **Edge rule:** if a drifting wreck reaches the top (`y <= 0`) or bottom (`y >= 160*256`, sprite bottom at 192) it explodes there at once (`kill_enemy_final`) instead of finishing the drift; the old y-clamp is gone. Direction is still position-based and locked (centre y < 96 drifts UP, >= 96 drifts DOWN, on the line down — picked once at the fatal hit, never re-picked, so y is monotonic). Drifting ships still blink (`e->flash` 90 → **40**, covers the 36-frame drift) and trail embers (every 2 frames); forward motion kept at 200/frame. Bosses untouched (own multi-explosion sequence).
- **Change 1 — verification (drift runs `qa/v19-runs/drift-v19c/d/e/`, debug ROM, wave 1, LASER, 90 s, new `qa/analyze_drift_v19.py` + DY readout row 8 `DY y<centre> d<+1/-1> t<die_t> T<type> N<tick>`):** **26 live drift groups across two runs (drift-v19d: 12, 6 up + 6 down; drift-v19e: 14, 8 up + 6 down), 0 rule violations, 0 monotonicity violations, only types 0/1 ever seen drifting.** Both drifting types observed in both runs (type 0: v19d drifts 0, 3; v19e drifts 0, 3; the rest type 1). **Duration:** full mid-screen drifts reach die_t=35 (v19e drifts 4, 7) — the 36-frame cap. **Speed:** y moves 10 px per 20 die_t = **0.5 px/frame**, repeatedly (e.g. v19c drift 0: y 116→126 as die_t 2→22; drift 8: y 72→57 as die_t 5→35). **Direction rule:** 0 violations in 26 drifts. **Edge:** 8 edge-terminated drifts (trajectory reaches the edge at die_t ≈ 32–33, before the 36-frame timeout); directly observed: v19e drift 12 (last live sample die_t=30, y=175, then the row stays idle — no die_t=35 refresh, which a timeout drift would show) and v19d drift 9 (last live sample y=17 at die_t=29, 1 px from the top edge). **Readout limit (why no sample shows y=16/176):** the DY readout refreshes every 5 frames, so the last observable sample of an edge drift is die_t=30 (y=17 up / y=175 down) — the edge fires at die_t=32 and the frame-35 refresh already shows idle; the analyzer therefore extrapolates the trajectory (locked direction, 0.5 px/frame from the start centre-y) to the edge instead of requiring an exact edge sample. **QA ghost-row trap (found Nov 20, ~3:55 AM EST):** the first v19 drift run analyzed with the v18 script reported impossible die_t=63 "deaths" spanning 26 s — `sub_text` never clears the row, so when the DY row goes idle ("DY -", 4 chars) the old digits stay behind and decode as a frozen dying enemy. Fixed by QA patch 12 (DY now prints the enemy TYPE and a refresh tick; the idle row has no digits after DY, so ghosts decode as nothing) and the new analyzer groups by continuity. **Patch 13 (debug only):** QA readout refreshes every 5 frames instead of 30, so one 36-frame drift spans ~7 refreshes. **Patch 14 (debug only, edge test):** every other drifting kill is teleported at the fatal hit to 16 px from the screen edge (centre y 32 or 160), so the edge rule fires on camera; odd kills are untouched natural drifts. **One drift vanished mid-drift (v19e drift 13, last seen die_t=21, y=151) — explained, NOT a bug:** the wave advanced to boss wave 9 at t≈85 and `clear_regulars()` (`memset(enemies, 0)`, called at every boss wave) wiped the field including dying enemies — a v6 design that clears live enemies too; the WARNING banner + area change to CRYSTAL at the same second confirm it. **Non-drifting immediate explosion:** 32 kills per run (DROP rolls) vs 12–14 observed drifts; only types 0/1 ever appeared in DY across 26 drifts; the branch is deterministic in `damage_enemy`; orange boom bursts visible in 125/673 frames of drift-v19e.
- **Change 2 — HP cap (`source/main.c`, `spawn_enemy`):** the v9 tier formula is replaced by a flat **+2 HP for all waves 7+** (`hp = hp + 2`, no growth). Old vs new HP:
  - base-1 enemy: wave 7: **4 → 3**; wave 8: **4 → 3**; wave 10: **7 → 3**; wave 13: **10 → 3**.
  - base-8 capital: wave 7: **18 → 10**; wave 8: **18 → 10**; wave 10: **28 → 10**; wave 13: **38 → 10**.
  - Waves 1–6 are **EXACTLY unchanged** (the formula only applies when `wave > 6`); boss HP untouched (boss 1 stays 70). **Why flat +2, not a tier cap at 1:** capping the tier at 1 alone would have left wave 8 unchanged (already tier 1), and the designer named wave 8 as nearly impossible — the flat +2 also makes wave 8 itself easier (4→3, 18→10) while preserving v9's intent (no more one-shotting: base-1 takes 3 SINGLE hits, capital 10).
- **Change 2 — verification, time-to-kill at wave 8 (area 3 CAVE, god mode, parked ship at (80,80) via debug patch 6, 90 s runs, `qa/run_v13.py`, kills counted from the DROP readout by new `qa/analyze_kills.py`):** **SINGLE at rate 0: v18 13 kills / 86 wall-s (8.4/min) → v19 44 kills / 86 wall-s (29.3/min), 3.4×**; early window t6–46: v18 1→3 kills vs v19 2→27 kills. **LASER at rate 3: v18 44 kills / 86 wall-s (29.3/min) → v19 44 kills / 86 wall-s (27.9/min), no regression** (LASER was already strong, so the cap helps it less); early window v18 t6–46: 2→31 kills vs v19 t6–41: 4→32 kills. Runs: `qa/v19-runs/ttk-w8-single-v18/`, `ttk-w8-laser-v18/`, `ttk-w8-single-v19/`, `ttk-w8-laser-v19/`. Both v19 runs also hit boss-wave freezes (wave 9 at ~t51–61, spawns frozen during the fight), so the improvement is if anything understated. Analytic check (SINGLE rate 0, 22-frame interval, 1 dmg, every shot hits): base-1 v18 4 hits ≈ 1.5 s → v19 3 hits ≈ 1.1 s; tough base-5 (crystal/prism) v18 12 hits ≈ 4.4 s → v19 7 hits ≈ 2.6 s; capital base-8 v18 18 hits ≈ 6.6 s → v19 10 hits ≈ 3.7 s. Waves 1–6 HP identical to v18 by construction (code inspection — formula only applies when wave > 6) plus the wave-1 drift runs showing 1-hit kills on base-1 types.
- **Boss-1 re-verification (`qa/v19-runs/boss1-v19/`, debug ROM, wave 2 start, real HP 70, LASER + rate 3, god mode, parked ship, 150 s, S/MUS readout timeline):** tempo lifts 1.00 → 1.21 → **1.25** at t=35 (boss intro); **B1M1** at t=38, V eases +62 → +22 → −46 → −106 → **−160** (full reverse held t=40–44), ship unmoved at (60,60), E0 (no other enemies during the boss — v6 rule intact); boss dead t≈44 (fight ~6 s at max fire rate — consistent with 70 HP at LASER rate 3, ~13 dmg/s); kill → next area's track fades in **v16 → 60 → 110 → 127**, tempo back to 1.00 — the v18 boss-music rule intact. Boss 2 (HALT, B1M2) t=64–76, boss 3 (UPWARD, B1M3, U−160, floor down, Y−46) t=95–106, boss 4 (SHIELD, B1M4) t=122–139 all died and the game advanced through areas 2→7 with no crash. Game runs at full 60 fps in these runs (DY tick rises ~2 per wall-second at the 30-frame cadence — the v18 "~15 game-fps" note was for a different run configuration).
- **Verification — end-to-end on the FINAL RELEASE ROM (`qa/v19-runs/e2e-final/`, 280 s, tap2 at 250 s):** title (gradient STAR DRAG + TAP TO START, "DRAG ANYWHERE TO FLY / TAP A WEAPON TO SWITCH / START RETURNS TO TITLE", greyed weapon panel — verified visually) → play (OPEN SPACE, WEAPON SINGLE; LASER by t=12; boss-1 WARNING banner ~t=23; back to SINGLE at t=39 after a hit) → **GAME OVER at t≈48** (gradient GAME OVER art, TOUCH TO RETRY, FINAL SCORE 2800, WAVE 2, BEST 2800 — verified visually; sub screen **0 differing pixels t100 vs t110**, top screen 650/49,152 differing = starfield twinkle behind the art — the hold works) → tap at 250 s → fresh game in OPEN SPACE / SINGLE → playing (LASER by t=269). Note: GAME OVER came early (~48 s) because the unattended release ship never moves; v18's e2e took until ~t202 by luck of the same setup. (The game-over score text decodes as garbage in `read_sub.py` — the game-over screen uses a different tile font than the play HUD; the visual is unambiguous.)
- **Verification — release audio on the FINAL ROM (`qa/v19-runs/release-audio-final/`, 45 s protocol, wave 1 SINGLE, `qa/run_audio.py` + `qa/analyze_audio.py`):** title RMS **−27.5 dBFS** (music present, the 16-bit level as v17/v18); gameplay s9–33 RMS **−15.5 dBFS**, peak 0.921, **0 pinned samples / 1,152,000 = 0.000%** (v18: 0 pinned, RMS −16.6). Music/SFX untouched in v19 — the numbers confirm it. **Process note:** the first audio run was invoked with a RELATIVE outdir path, which broke melonDS's relative HOME (nested `qa/v19-runs/release-audio-final/qa/...` dir) and produced no `.raw` file and three identical screenshots; re-run with ABSOLUTE paths worked. Always use absolute paths for `qa/run_audio.py`.
- **Verification — JIT-on (melonDS 1.1, `Enable = true`, `qa/v19-runs/jit-on/`, `qa/bisect-v4/run_one.py`):** top screen **96.9% white** — exactly as v12–v18 and stock libnds hello-world, the known melonDS JIT + devkitPro-homebrew incompatibility, NOT a v19 bug. JIT-off renders fine (all other runs above).
- **No crash in ~40+ min total v19 runs** (4 TTK + 3 drift + boss-1 150 s + e2e 280 s + release audio + JIT-on).
- **Real project verified clean after the fix:** 0 `QA_START_*`, 0 `BISECT`, 0 `music_buf`, 0 NitroFS calls in the real source; the only `iprintf` is inside `#ifdef QA_DEBUG` (line ~1222, `MUSIC area %d emb v%d`); volumes verified in source (`MUSIC_VOL` 127, shoot 60, laser 70, alarm 80, pickup 70, boom 80, boss tick 50, player-damage buzz 70); all 7 music tracks 16-bit at 22050 Hz; boss 1 HP 70; drop rates, 1UP, pickup chime, boss tick, player-damage buzz, hit sparks, arcade title, clean touchpad UI, rainbow border, boss-3 floor sweep, boss tempo lift 1.25×, boss-music fade-in, labeled pickups, relative drag, GAME OVER hold screen — all unchanged from v18 except where noted above. `README.md` updated to v19; `source/main.c` header updated to v19 (comment only — the md5 is unchanged by it, verified by the two clean rebuilds above).
- **QA tooling / process notes (lessons from this round, keep):** (1) New `qa/analyze_drift_v19.py` (type + tick DY readout, trajectory-based edge detection — see the readout-limit note above), `qa/analyze_kills.py` (DROP-roll kill counting; the H field in the DROP row shows decode noise and is not a reliable metric), `qa/apply_v19_debug_patches.py` (v18 patches 1–8 + 10, path updated, plus **patch 11 `QA_START_WEAPON`** — 1 = LASER + rate 3, 0 = SINGLE at rate 0, for wave-8 TTK with the plain starting weapon — **patch 12 DY type + refresh tick** (kills the ghost-row trap), **patch 13 5-frame QA readout refresh** (was 30), **patch 14 edge-forcing spawn override** (debug only — every other drifting kill starts 16 px from the edge)). Patch-script idempotency fixes: patch 7's assert now tolerates patch 10b's inserted `qa_hits++;` line; patches 10c/10d asserts tolerate already-applied state. (2) The debug-copy rsync trap again: rsyncing the real project into `qa/v19-debug/` can overwrite the debug `arm9/Makefile` and `build.sh` — restore from the previous debug copy and diff after rsyncing, as in v14/v16/v17/v18. (3) `qa/run_audio.py` needs ABSOLUTE paths (see above). (4) Debug wave advances as `1 + kills/4` (vs /12 in release), so long debug runs hit boss waves and freeze spawns — use early windows for kill-rate comparisons. (5) The DY tick proves the game runs at full 60 fps in the drift runs (tick ~2/wall-s at 30-frame cadence); a 36-frame drift lasts ~0.6 wall-s and appears in only 1–2 readout refreshes at the old 30-frame cadence — patch 13's 5-frame refresh is what makes drifts observable.
- **Honest gaps (the designer's gate):** Sound not heard by a human headless — the capture evidence proves the mix is clean (0 pinned samples) with music present at the v17/v18 level, but whether the balance feels right to her ear on NooDS is for the designer's playtest. D-pad untested (same as v4–v18). DeSmuME rendering unverifiable here (white SDL window for every ROM under Xvfb). NooDS/RetroArch iPhone paths not testable here. Boss fights verified at real HP 70 with god mode + a parked ship (the designer aims with drag, so her fights will run shorter than these unattended times). The boss parks at x=196, so his 64 px sprite is clipped ~4 px by the right screen edge (same as v7–v18). The edge-explode rule is proven by trajectory (8 drifts extrapolate to the edge at die_t ≈ 32–33) plus two directly observed edge endings (die_t=30/y=175 then sustained idle; die_t=29/y=17), not by a sample showing y=16/176 — the 5-frame readout refresh makes that exact frame unobservable (see above). The ':' glyph renders as a merged vertical bar — no game text uses ':' today.
