# melonDS blank-white investigation — Star Drag v1 (Nov 18, 2026)

the designer's report (twice, Folded Letter v1 and Star Drag v1): ROM loads and plays
in NooDS, but shows only a blank white screen in melonDS and DeSmuME.
The DSi-header fix from the first investigation was already applied to v1
(unit code 0x00, DSi magic zeroed, verified by hand) — so the DSi header
is NOT the cause of the current blank. Something else differs between our
melonDS test and the designer's setup.

## What was tested (melonDS 1.1 — that IS the current stable release)

All runs under Xvfb here, screenshots pixel-checked (white% / non-black%),
melonDS 1.1 AppImage extracted to ~/workspace/emulators/melonds-bin/.

| Run | Config | Result |
|---|---|---|
| v1, DS direct-boot, JIT off | ConsoleType=0, DirectBoot=true | **Renders** (title, ship, HUD) |
| v1, DS direct-boot, JIT on | same, JIT Enable=true | **Black screens, emulator runs 60fps** — game hung before first frame |
| v1, JIT on, OpenGL renderer | UseGL=true | **Black screens** (same) |
| v1, JIT on, FastMemory off | FastMemory=false | **Black screens** (same) |
| v1, DSi console, no DSi BIOS | ConsoleType=1 | Error dialog "DSi ARM9 BIOS was not found", ROM never loads (not white) |
| v1, firmware boot, no firmware | DirectBoot=false | **Renders** (falls back to direct boot) |
| hello-world (stock libnds), JIT on | — | **Renders** (white sub-console, frame counter) |
| hello-world, JIT off | — | **Renders** |

## Bisect (minimal ROMs built from the Star Drag source, /tmp/sd-test)

- No-sound variant (soundEnable/soundPlaySample removed): still black under JIT.
  Sound is NOT the trigger.
- Console-only variant (no sprites/BG init): still black under JIT.
  Sprites/BG init is NOT the trigger.
- Minimal A (videoSetMode + vramSetBank + consoleDemoInit + VBlank loop):
  **melonDS segfaults under JIT (rc -11), reproducible**; renders under
  interpreter.
- Minimal B (consoleDemoInit only, no manual video/VRAM calls): **renders
  under JIT**, like hello-world.
- Minimal C (videoSetMode only) and D (vramSetBank only): **both segfault
  under JIT**.
- Minimal E (consoleDemoInit first, then video/VRAM calls): **segfaults
  under JIT**.
- Minimal F (A + explicit irqSet(IRQ_VBLANK)/irqEnable): still black under
  JIT. The VBlank IRQ handler is NOT the fix.
- Minimal A rebuilt with `-mthumb` (stock hello-world uses -mthumb; our
  arm9 Makefile did not): **no more segfault, but screens still black
  under JIT** (emulator runs 60fps, game hung before first frame).

## Conclusions

1. **Reproduced, in part.** With melonDS 1.1's JIT recompiler enabled
   (the out-of-the-box default), our libnds 2.x / calico builds do not
   render: ARM-mode builds segfault melonDS; Thumb builds run at 60fps
   but hang before the first frame. With JIT disabled, the identical ROM
   renders fine. The stock libnds hello-world renders under JIT, so the
   trigger is in our build's early video/IO init (any videoSetMode /
   vramSetBank call triggers it in ARM mode), not in game logic, sound,
   sprites, or the DSi header. The exact JIT codegen mechanism was not
   identified — this looks like a melonDS 1.1 JIT incompatibility with
   this toolchain build, and it needs the designer's melonDS version/settings to
   confirm on her side.
2. **NooDS works because it is interpreter-only** — consistent with the
   JIT finding. DeSmuME 0.9.11 loads the ROM (log: `ROM serial:
   NTR-####`, no DSi line) but its SDL window is white for *every* ROM
   in this VM, so DeSmuME rendering cannot be verified here at all;
   DeSmuME's dynarec may share the JIT issue.
3. **A real ROM anomaly was found and fixed for v2:** the v1 file is
   310,272 bytes but the header's device-capacity byte (0x14 = 0x02)
   declares 512 KB — melonDS logs `!! bad ROM size 310272 (expected
   524288) rounded to 524288` and pads in memory, but stricter loaders
   (DeSmuME stream mode, RetroArch cores) may not. v2's build pads the
   ROM to the declared capacity with 0xFF; the padded v1 loads in
   melonDS with no "bad ROM size" line.
4. **Mitigations applied in v2:** ARM9 built with `-mthumb` (no JIT
   segfault), ROM padded to device capacity, real game code `SDRG` and
   maker code `01` instead of `####`/0x0000 (melonDS logged `ROM entry
   not found for gamecode 589505315` for `####`), init order kept
   console-first. v2 was still verified primarily with JIT off; the JIT
   rendering question is NOT fully solved at the code level.

## What the designer should check / try (needed to close this)

- melonDS version and platform (desktop? RetroArch core on iPhone?).
- melonDS Emu settings: is the JIT recompiler enabled? (Settings →
  Emu → JIT recompiler.) Turning JIT **off** is the known workaround
  for v1 — if the game then renders, the JIT finding is confirmed on
  her side.
- Console type: DS, not DSi (DSi mode with no DSi BIOS refuses to load
  at all — error dialog, not a white screen).
- DeSmuME: which version, and does it render with CPU mode set to
  interpreter instead of dynarec?

## Nov 18, 2026 (evening) — Fix 1: the "encrypted ROM needs native BIOS" error was the game code, not the secure area (CORRECTION to the v2 note above)

the designer, on RetroArch iPhone melonDS, loading v2 got:
"The loaded rom is encrypted and needs native NDS BIOS files, but they're not loaded."
NooDS played the same ROM fine.

**Root cause (from melonDS source, `NDS_Header.h`):**

    bool IsHomebrew() { return (ARM9ROMOffset < 0x4000) || (strncmp(GameCode, "####", 4) == 0); }

v2 stamped game code `SDRG` and maker `01` (done in v2 as a supposed
mitigation, after melonDS logged `ROM entry not found for gamecode
589505315` for `####`), with ARM9 offset exactly `0x4000`. So
`IsHomebrew()` was **false**. In `NDS::ParseROM()` melonDS then builds a
`CartRetail`, which needs the native BIOS to decrypt the secure area.
The RetroArch melonDS core (melonds-ds changelog 1.1.8, Oct 18, 2024:
"Fixed encrypted NDS ROMs failing to load without any feedback; loading
one without using the native BIOS will now display an error message")
shows exactly the designer's error in that case. v1 used `####` and never got
the encrypted error (only the blank-white/JIT issue); the stock libnds
hello-world (`~/workspace/ds-diag/hello-pureds.nds`) also ships `####`,
ARM9 offset `0x4000`, maker blank, and is classified homebrew.

**v2 header facts verified by hand:** unit `0x00`, DSi magic zeroed,
game code `SDRG`, maker `01`, device capacity `0x04` (2 MB), ARM9 offset
`0x4000`, ARM9 entry `0x02004800`, first 0x800 bytes at `0x4000`
(secure area) all zero, real code starts at `0x4800`; secure-area CRC
`0x5574` (OK, multiboot). None of those was the problem — the game code
alone flipped the homebrew/retail classification.

**Fix (v3):** `tools/fix_ds_header.py` stamps game code `####` and maker
`0x00 0x00`, with a comment warning never to stamp a real game code
again. No BIOS files may be shipped or downloaded (copyrighted).
Verified in v3: `ndstool -i` shows game code `####`, maker blank, unit
`0x00`, DSi magic zero, secure-area CRC OK, header CRC OK; melonDS 1.1
log: `Inserted cart with game code: ####`, `No need to re-encrypt cart
secure area`. (The `Secure area decryption failed` line that follows is
the harmless direct-boot attempt on a zeroed secure area — the stock
libnds hello-world shows the same and boots.)

**Correction to the v2 mitigation note above:** stamping a real game
code `SDRG` did NOT help anything — it *caused* the RetroArch load
error. The `ROM entry not found for gamecode` log line for `####` is
cosmetic (melonDS just has no database entry for homebrew) and is not a
problem.

**What is still NOT proven:** the RetroArch melonDS core itself was not
testable here (only standalone melonDS 1.1). The ROM-side cause is proven
and fixed, and standalone melonDS now classifies the ROM as homebrew via
the same `IsHomebrew()` path the core uses, so the error should be gone
— but the designer should confirm on her iPhone. If the core still refuses to
load it, the next thing to check is the core's own settings (e.g. whether
it is set to use native BIOS / DSi mode instead of DS mode), not the ROM
header.

**Separate, unchanged issue:** with JIT on, melonDS still renders black
for our libnds builds (v3 re-checked Nov 18, 2026: loads, no segfault,
black screens). JIT-off renders fine. This is a rendering issue, not a
load issue, and is unrelated to Fix 1.

## Nov 18, 2026 (late evening) — v4 Fix 1: JIT bisect (m1–m10), and why JIT is probably NOT the designer's cause

the designer reported v3 (header verified plain-DS, game code `####`, no BIOS
error) as a blank WHITE screen again in melonDS (RetroArch, iPhone),
while NooDS runs it fine. v4 re-bisected the JIT question carefully,
because one earlier result in this file was WRONG.

### Correction to the table above
The row "hello-world (stock libnds), JIT on — Renders" is **wrong**.
It came from a runner that edited the wrong `Enable =` line in
melonDS.toml (the first `Enable =` in the file is under `[DLDI]`, not
`[JIT]`), so those "JIT-on" runs were actually JIT-off. Re-checked
with the `[JIT] Enable` value verified in the config melonDS actually
saved after the run: **stock libnds hello-world under JIT-on shows NO
console text** — only 1536 white px confined to y 0–5 of the bottom
crop, which is bleed from the top screen's bottom edge, not text.
Under JIT-off the same ROM shows console text (2753 white px, y 0–149).
Top screen default (uninitialised) is white; a BLACK top screen means
videoSetMode executed. Screenshot metric used throughout: bottom-screen
console text = white pixels spread over y 6–150 of the bottom crop
(0,208,256,400) of a 1024x768 window shot.

### Bisect (minimal Thumb ROMs, `qa/bisect-v4/`, melonDS 1.1 extracted binary, fresh config home per run)
- v3 JIT-off: renders title fine. v3 JIT-on: both screens black
  (top 96.9% black, bottom 100% black).
- m1 console-only: JIT-on top white (blank), no text.
- m2 videoSetMode BEFORE consoleDemoInit: JIT-on both screens BLACK
  (videoSetMode executed, then hung).
- m3 vramSetBank before console: JIT-on renders like m1 (top white, no text).
- m4 video/vram AFTER console: JIT-on top white, no text; **JIT-off m4
  shows NO console text at all — videoSetModeSub after consoleDemoInit
  kills the sub console** (re-proves the v2 lesson: console first).
- m5/m6/m7 (video-before + bgInit/oamInit/sound): black like m2.
- m8/m9 console-first full init + sound: JIT-on top white, no text.
- **m10 (consoleDemoInit FIRST, then videoSetMode MAIN only — no
  videoSetModeSub — then VRAM A/B/D, OAM, BG) is the chosen v4 order:
  JIT-off shows console text (630 white px, y 6–92); JIT-on is at
  hello-world parity (top white, no text).**
- Baremetal ROM (no libnds) is white under JIT-on AND JIT-off and never
  rendered even in old ds-diag shots — ignore it as a test case.

### External confirmation (web search, Nov 18, 2026 evening)
1. melonDS JIT + devkitPro homebrew black screen is a **known issue**:
   Cool-Pizza-NDS README (GitHub): "On melonDS Android 2.0.2, turn JIT
   off in the emulator settings before launching. JIT-enabled startup
   can produce a black screen or crash; disabling JIT resolves the
   reported startup issue."
2. The RetroArch **melonDS DS core on iOS is built with JIT DISABLED by
   default** (libretro melonds-ds CMakeLists:
   `if (IOS) set(DEFAULT_ENABLE_JIT OFF)`; App Store RetroArch has no
   JIT at all). So JIT is most likely **NOT** the cause of the designer's white
   screen on her iPhone.

### v4 conclusion — do NOT claim v4 fixes the melonDS white screen
- v4 uses the m10 init order (console first, main video only) and
  keeps the plain-DS `####` header, Thumb build, and 4 MB padding.
  Standalone melonDS 1.1 JIT-off renders v4 (title → play → game over,
  verified in `qa/v4-runs/play1`). JIT-on here still shows nothing,
  at hello-world parity — no ROM-side change in v4 can fix that from
  our side, and the stock hello-world fails identically, so this is a
  melonDS JIT + devkitPro-homebrew incompatibility, not a Star Drag bug.
- Since the RetroArch iOS core has JIT off by default, the designer's white
  screen has a different cause that is **not reproducible here**.
  Things for the designer to check on her iPhone (in that order):
  1. Use the **"melonDS DS"** core (melonds-ds), not the old
     "melonDS" core — the old core is unmaintained and behaves
     differently with homebrew.
  2. Core Options / CPU settings: make sure JIT is off (it should be
     by default on iOS) and the console is DS, not DSi.
  3. Try the **NooDS core** in RetroArch as a fallback — NooDS runs
     the ROM fine standalone, and the core is interpreter-only.
  4. Try the **DeSmuME core** as another fallback.
  5. If it still white-screens in the melonDS DS core, tell us the
     core version shown in RetroArch (Information → Core Information)
     and whether the same core renders the stock libnds hello-world —
     that tells us whether it is our ROM or the core.
- DeSmuME rendering remains unverifiable here (its SDL window is white
  for every ROM under Xvfb); NooDS is not runnable here.

---

# v5 appendix — fresh investigation, Nov 18, 2026 (~11:00–11:45 PM EST)

## Header byte-compare vs stock libnds hello-world
Built stock libnds hello-world with the same toolchain
(`~/workspace/ds-diag/hello-pureds.nds`) and byte-compared its header
against Star Drag v4: logo CRC OK, header CRC OK, game code `####`,
unit `0x00`, same ARM7 (size 0xB374, same entry 0x2380000), same ARM9
entry 0x02004800, capacity byte 0x05 = 4 MB matching file size, DSi
fields at 0x188–0x1AC non-zero in BOTH hello and v4 (ndstool template
bytes, not a differentiator). **No header difference found that could
explain a white screen.** The v5 ROM has the same header profile
(game code `####`, unit `0x00`, logo CRC 0xCF56, header CRC 0xD9D8).

## DeSmuME 0.9.11 here
Hello-world and v4 behave **identically**: white SDL window, 104% CPU,
PC stuck at ARM9 entry 0x02004800 under the GDB stub (tested via
`arm-none-eabi-gdb` and a raw RSP socket with byte-swapped register
parsing; `continue`/`vCont;c` did not advance the PC for either ROM).
The stub result is inconclusive for both, not evidence of a v4 bug —
and the white SDL window is a VM/Xvfb rendering artifact that affects
every ROM here, so DeSmuME cannot distinguish v4 from hello-world
either. No ROM-side cause found.

## melonDS 1.1 — JIT re-verified with a CORRECT config (v5)
- JIT-off: v5 renders fully (title, gameplay, game over, restart) —
  verified on the exact delivered md5 `7865ef8c…`.
- JIT-on: v5 shows **exactly the white screen the designer describes** —
  top screen white, bottom screen black, no text, no crash, no
  segfault — identical to v4 and to the stock hello-world.
  This run's JIT setting was verified, not assumed: the config file
  was read back as `[JIT] Enable = true` both before and after the
  run, because an earlier attempt had silently run JIT-off (see below).
- **Tooling trap found and fixed:** `qa/bisect-v4/run_one.py`
  toggled JIT by replacing the substring `"Enable = false\nFastMemory"`.
  melonDS had re-saved the fresh-home toml with the [JIT] keys in a
  different order (`FastMemory` now precedes `Enable`), so the replace
  silently no-opped and the "JIT-on" run actually ran JIT-off (and
  rendered the title — which briefly looked like a v5 JIT fix).
  `run_one.py` now edits the [JIT] section directly and asserts the
  toggle took. Lesson: always read the config back after a JIT run.

## Leading hypothesis (updated)
JIT is again the leading hypothesis for the designer's **melonDS** white
screen: JIT-on produces precisely her symptom (white top, black
bottom) for every devkitPro/libnds ROM here, including the stock
hello-world. The earlier reason for discounting it — "RetroArch iOS
melonDS DS core is JIT-off by default" — is only true for the App
Store build; a sideloaded RetroArch (or a toggled core option) can
have JIT on, and the designer's exact setup is unknown. **Action for the designer:**
in RetroArch, with the melonDS DS core, check Core Options → CPU
core / JIT and set it to interpreter (JIT off), console DS not DSi.
The **DeSmuME** white screen is still unexplained — DeSmuME has no
JIT on ARM — and may be a separate cause (or the same "core" being
mislabelled in her setup). If the melonDS DS core with JIT off still
white-screens, the decisive test is still: does the stock libnds
hello-world render in that same core? If hello-world also whites,
it is the core, not our ROM.
