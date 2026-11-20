/* Star Drag v19 — a touch-and-drag HORIZONTAL space shoot'em up for DS.
 *
 * v19 changes (the designer, Nov 19, 2026):
 *  Change 1: DEATH DRIFT TUNED DOWN. the designer: the drift is too long,
 *          only one or two enemy types should do it, drift slower,
 *          and if it hits the top or bottom it should just explode.
 *          The v18 drift (84 frames, ~1.4 s, 1 px/frame, all
 *          ship-type enemies) is now: SHORTER — 36 frames (~0.6 s);
 *          SLOWER — 0.5 px/frame vertical; only TWO types drift —
 *          type 0 drifter and type 1 weaver, the two most ship-like;
 *          every other type gets its per-type pixelated explosion
 *          IMMEDIATELY at the killing hit (v8 behaviour, via a
 *          direct kill_enemy_final call in damage_enemy). EDGE RULE:
 *          if a drifting wreck reaches the top (y <= 0) or bottom
 *          (y >= 160*256) it explodes there at once instead of
 *          finishing the drift (the old y-clamp is gone). Direction
 *          is still position-based and locked (centre y < 96 drifts
 *          UP, >= 96 drifts DOWN), and drifting ships still blink
 *          (flash 40, covers the 36-frame drift) and trail embers.
 *  Change 2: LATE-WAVE HP CAPPED. the designer: from wave 7 the HP increase
 *          is too much — wave 8 is nearly impossible. The v9 tier
 *          formula grew without bound (tier = (wave-7)/3+1,
 *          hp = base*(1+tier)+tier*2: a 1-HP type was 4 HP at wave 7
 *          and 10 at wave 13, an 8-HP capital 18 at wave 7 and 38
 *          at wave 13). Now waves 7+ just add a flat +2 and NEVER
 *          grow: a 1-HP type is 3 HP and an 8-HP capital is 10 HP at
 *          every wave from 7 on. Waves 1-6 are EXACTLY unchanged and
 *          boss HP is untouched (boss 1 stays 70). Measured at wave
 *          8 (area CAVE, god mode, parked ship): SINGLE at rate 0
 *          went from 13 kills/86 s in v18 to 44 kills/86 s in v19
 *          (3.4x); LASER at rate 3 held at 44 kills/86 s.
 *
 * v18 changes (the designer, Nov 19, 2026):
 *  Change 1: SFX DOWN MORE. the designer: reduce the sound effects volume,
 *          that will balance with the music volume better. All SFX
 *          down a further clear step (~-3 dB, x0.7) from v17:
 *          shoot 85->60, laser 100->70, alarm 110->80, pickup
 *          100->70, boom 110->80, boss tick 70->50, playerdmg
 *          100->70. Music untouched (vol 127, loop peak 0.72).
 *  Change 2: BOSS MUSIC RULE. the designer: when each boss appears keep
 *          playing that area's music but speed up its tempo, and
 *          only change to the next area's music after that boss is
 *          defeated — and it should FADE IN to the next music.
 *          What the old code actually did (measured, see
 *          qa/VERIFICATION.md v18): at the boss intro it called
 *          terrain_set_area(na), which calls music_set_area(na) —
 *          so the track switched to the NEXT area's music at the
 *          intro, at FULL volume immediately, and at the kill there
 *          was no track change and no fade-in at all (the area had
 *          already changed). Fix: the boss intro now changes the
 *          terrain only (terrain_set_area_nomusic) — the current
 *          area's track keeps playing, sped up 1.25x by the
 *          existing music_boss_start lift. At the kill
 *          (boss_death_final) the music switches to the new area's
 *          track with music_set_area_fadein: it starts at volume 0
 *          and ramps to 127 over 90 frames (~1.5 s) while the old
 *          track fades out over 0.5 s and the tempo eases back to
 *          1.0x. Applies to EVERY boss (all four modes share the
 *          intro/death path). If a boss's area doesn't change
 *          (e.g. wave-6 boss), there is no switch — the same track
 *          just keeps playing at normal tempo after the kill.
 *          Non-boss area changes keep the old behaviour (new track
 *          at full volume immediately, old fades out over 0.5 s).
 *
 * v17 changes (the designer, Nov 19, 2026):
 *  Fix: MUSIC VOLUME TOO LOW IN GAME. v16's 16-bit fix worked for
 *          clarity ("Yes much clearer music great") but the music is
 *          too quiet on her phone (NooDS) — so the ~0.47x 16-bit
 *          attenuation is NOT just a melonDS quirk; the 16-bit path
 *          genuinely plays quieter on her device too. Fix: raise the
 *          music ~+9 dB while KEEPING 16-bit (never back to 8-bit):
 *          MUSIC_VOL 80 -> 127 (+4.0 dB) and shipped loop peak
 *          0.42 -> 0.72 (+4.7 dB, tools/convert_music_v17.py). That
 *          restores the music to roughly v12's perceived level.
 *          the designer confirmed the balance: "Bring down the sound
 *          effects just a tad in volume and raise the background
 *          music volume" — so the SFX come DOWN a tad across the
 *          board (shoot 100->85, laser 120->100, alarm 127->110,
 *          pickup 120->100, boom 127->110, hit 127->110, boss tick
 *          80->70) to keep mixer headroom for the louder music.
 *  Also v17 (the designer, Nov 19, 2026): new PLAYER-DAMAGE sound — a low
 *          descending buzz (280->70 Hz, 0.28 s), distinct from the
 *          boss-damage tick and the old hit sound; plays on every
 *          player hit (replaces sfx_hit in player_hit). And a HIT
 *          SPARK on multi-HP enemies: a tiny 3-particle flash at the
 *          impact point on every surviving hit (bosses included),
 *          so each hit reads visually without cluttering the screen.
 *
 * v16 changes (the designer, Nov 19, 2026):
 *  Fix: MUSIC IN-GAME STILL DISTORTED — 16-BIT SAMPLES. the designer A/B'd
 *          the v15 compare clips: the "source" clips are clean and
 *          perfect, the "in-game" clips (music-only title captures)
 *          are STILL distorted/overdriven-sounding. Diagnosis
 *          (measured, see qa/VERIFICATION.md v16 section): the
 *          distortion is the 8-BIT PCM FORMAT itself. The shipped
 *          v15 area-1 loop, measured against its own float reference
 *          chain, has a quantization-error SNR of only 28.5 dB —
 *          audible grit on sustained synth tones, exactly
 *          "distorted and overdriven", invisible to every
 *          peak/clipping measurement (peaks were 0.42, zero rail
 *          samples). The same chain in 16-bit measures 79.0 dB.
 *          Fix: the 7 tracks are now signed 16-bit PCM
 *          (SoundFormat_16Bit, tools/convert_music_v16.py) — the DS
 *          hardware and libnds support it, so clean in-game music IS
 *          possible; the game was just using 8-bit samples. 16-bit
 *          doubles the size, so the loops shorten to 7.5 s at
 *          22050 Hz (330,750 bytes/track, ARM9 ~2.53 MB of the
 *          0x27c000 cap); same windows, same rate, same shipped
 *          peak 0.42 and v12 volumes as v15 — the ONLY variable that
 *          changes is bit depth. SFX stay 8-bit (short blips).
 *  Also v16 (the designer, Nov 19, 2026): weapon bonuses HALVED — kill
 *          drops 22% -> 11% (3% 1UP, 8% weapon capsule), floater
 *          1100 -> 2200 frames (~18 s -> ~36 s) with an 8% 1UP
 *          chance; new rare 1UP pickup (type 5, gold capsule /
 *          pink heart / "1UP", shares PAL_MISSILE with gold+pink
 *          patched into slots 14/15 — all 16 palette slots were
 *          already taken), +1 life capped at 5 (else +1000 score),
 *          "1UP!" cue; new PICKUP chime (sine arp C5/E5/G5/C6,
 *          replaces the harsh square version) and BOSS-DAMAGE tick
 *          (short 330->200 Hz sine, throttled to one per 6 frames).
 *
 * v14 changes (the designer, Nov 19, 2026):
 *  Fix 1: MUSIC GARBLED / DISTORTED IN v13 — VOLUMES REVERTED, MUSIC
 *          RE-RENDERED PEAK-SAFE. the designer: "Put the music volume back to
 *          where it was... it just sounds distorted everywhere and
 *          increasing the volume did not help it just made it worse."
 *          Diagnosis (measured, see qa/VERIFICATION.md v14 section):
 *          the v13 FILES are not clipped (peaks 0.76-0.92, zero rail
 *          samples) and the music ALONE plays clean in melonDS, but
 *          v13 made the music ~9 dB hotter than v12 (loops RMS -17
 *          dBFS, channel vol 110): music peak 0.92 x 110/127 = 0.80
 *          of full scale + shooting SFX on top sums the DS 16-bit
 *          mixer over full scale — the gameplay capture proves the
 *          output pins at full scale. Louder = more clipping = worse.
 *          The v13 presence lift (+5 dB @ 1200 Hz, +3 dB @ 3 kHz)
 *          also pushed EQ'd peaks to 1.29-1.77 and quadrupled the HF
 *          share, harsh on a phone speaker. Fix: volumes back to v12
 *          (music 80, shoot 100, laser 120, hit 127, boom 127,
 *          pickup 120, alarm 127); loops re-rendered by
 *          tools/convert_music_v14.py — same best 14 s windows, a
 *          GENTLE +2 dB lift applied BEFORE the peak-capping gain,
 *          RMS -18 dBFS, peaks capped at 0.70 (-3.1 dBFS), no limiter,
 *          so the music+shoot mixer sum stays under full scale
 *          (worst case 0.96 across all areas).
 *  Fix 2: BOSS 1 NEEDS MORE HP. the designer: "the first boss needs more hp
 *          as it's killed too quickly right when the backwards flying
 *          gets so cool." Boss 1 HP 25 -> 70 (~2.8x, fight lasts
 *          ~2-3x as long). Its attack pattern, speed and bullets are
 *          UNCHANGED — still the "perfect" boss 1, only tougher.
 *          Bosses 2-4 HP unchanged.
 *
 * v13 changes (the designer, Nov 19, 2026):
 *  Fix 1: MUSIC IN AREAS 2-7 TOO QUIET TO MAKE OUT. Diagnosis
 *          (measured): v12 sliced every loop from the START (0..18 s)
 *          of its source; the first 18 s of areas 3/4/5 are dark,
 *          bass-heavy intros (centroid 235/250/286 Hz, 2.8-3.8% of
 *          energy in 1-8 kHz vs area 1's 454 Hz / 7.4%), so on a phone
 *          speaker they read as a faint murmur — and the shooting SFX
 *          (channel vol 100, raw RMS -8.4 dBFS) constantly masked the
 *          music (vol 80, RMS -17..-23 dBFS). Fix: each loop is now
 *          the BEST 14 s window of its source (loudest + brightest),
 *          with a presence lift (highshelf +5 dB @ 1200 Hz, +3 dB @
 *          3 kHz) and loudness normalization to -17 dBFS RMS
 *          (tools/convert_music_v13.py), all tracks at 22050 Hz
 *          (v12 had areas 2-7 at 16000 Hz, cutting everything above
 *          8 kHz); music channel 80 -> 110, shoot 100 -> 65, laser
 *          120 -> 85, hit 127 -> 90, boom 127 -> 100, pickup 120 ->
 *          90 (alarm stays 127). Verified by melonDS audio capture
 *          per area (see qa/VERIFICATION.md).
 *  Fix 2: BOSS 3 UPWARD FLIGHT — FLOOR FLIES DOWN TOO. The v12
 *          fight slid the floor band down only 48 px (the terrain
 *          scroll clamp), but late-area floors are up to 9 rows
 *          (72 px) tall, so a sliver stayed visible. Now the floor
 *          band is blanked once it has slid down, leaving a full
 *          starfield for the fight; after the boss dies the new
 *          area's ceiling/floor ease back in gradually (the
 *          scroll_terr_y offset eases back to 0 instead of snapping).
 *          Ship and boss positions/orientations are unchanged.
 *  Change 3: PROPER ARCADE TITLE SCREEN. Big gradient "STAR DRAG"
 *          art (tools/convert_title.py) on the far starfield layer
 *          over the live scrolling starfield, with a pulsing
 *          "TAP TO START" prompt. Replaces the old console text.
 *  Change 4: NO MORE LOG TEXT ON THE TOUCHPAD. The bottom screen
 *          is now a finished tile-text UI (tools/convert_font.py):
 *          area/weapon lines in coloured text, boss cues as a
 *          banner — no console prints anywhere in the release ROM
 *          (all iprintf is behind #ifdef QA_DEBUG, compiled out).
 *  Change 5: RAINBOW GLOWING BORDER. The selection border now
 *          cycles smoothly through the spectrum (4 s hue cycle)
 *          while its brightness breathes in and out (2 s sine,
 *          40-100%, never off) — a glow, not a blink, and not a
 *          single fixed weapon colour any more.
 *
 * v11 changes (the designer, Nov 19, 2026):
 *  Change 1: REAL UNIQUE MUSIC PER AREA, WITH FADES — and a real
 *          BOSS TEMPO LIFT. Each area now plays its OWN track file:
 *          area 1 = the original 85 s Speechma track
 *          (assets/music-v2.mp3, kept as-is, never regenerated),
 *          areas 2-7 = new Speechma tracks (assets/music-area2.mp3
 *          ... music-area7.mp3), converted by
 *          tools/convert_music_v11.py to ~36 s loops at 16000 Hz
 *          mono 8-bit (shorter loops keep the ROM at 8 MB; quality
 *          kept over length). The v10 rate/offset variations are
 *          GONE — no area plays a pitch-shifted copy of the area-1
 *          track any more. Area changes CROSSFADE (new track starts
 *          at volume 0 while the old one fades out over the same
 *          1 s — music_update, once per frame), so there is never
 *          a hard cut or gap. Boss fights LIFT THE TEMPO: the
 *          playback rate ramps up 25% at the boss intro
 *          (music_boss_start) and back to normal over ~1 s after
 *          the boss dies (music_boss_end), while the area crossfade
 *          carries the music into the next area's own track.
 *          HONEST NOTE: v10 had NO dedicated boss tempo code —
 *          the shift the designer heard at boss 1 was the wave-3 area
 *          change speeding the track up to 26000 Hz. v11 makes the
 *          boss tempo lift real, explicit, and the same for every
 *          boss. Alarm/SFX untouched (separate channels).
 *          (v18 supersedes the timing for BOSSES: the track no
 *          longer switches at the boss intro — it keeps playing,
 *          sped up, and the next area's track only starts, fading
 *          in, after the boss dies. See the v18 header note.)
 *  Change 2: GLOWING BORDER, NOT BLINKING. The v10 border pulsed
 *          full/dim every 16 frames, which read as blinking. It
 *          now follows a smooth sine over a 2 s cycle (glow_col),
 *          brightness ramping continuously between ~40% and 100%
 *          with no abrupt on/off step — the border is always
 *          visible, it breathes. Same colours, same rule: only
 *          ever on the selected weapon, uncollected stay greyed.
 *
 * v10 changes (the designer, Nov 19, 2026):
 *  Change 1: DIFFERENT MUSIC PER AREA. Each of the 7 areas now has
 *          its own music. No browser access was available in this
 *          build session, so no new AI tracks could be generated
 *          (Speechma needs a live browser); instead each area plays
 *          a DISTINCT VARIATION of the existing 85 s Speechma track
 *          (assets/music-v2.mp3 — kept as-is, never regenerated):
 *          a different loop section of the track at a different
 *          playback rate (pitch + tempo), chosen to fit the area's
 *          mood — OPEN SPACE the original, SPACE BASE faster/tense,
 *          PLANET slower/open, CAVE slow/dark, CRYSTAL fast/bright,
 *          WATER flowing, GOO slowest/eerie. The music switches
 *          cleanly at area changes (kill old loop, start new one in
 *          the same frame — no gap, no crash) and keeps looping
 *          within an area; boss fights keep the area's track, and
 *          the alarm/SFX are untouched (separate channels).
 *          ROM stays 4 MB: 6 extra tracks would not fit.
 *  Change 2: GLOWING BORDER INSTEAD OF THE STAR MARKER. The star
 *          above the selected weapon used to move with the finger
 *          while dragging, so the selection was lost. The star as a
 *          selection marker is GONE (the finger-follow reticle while
 *          dragging stays). The selected weapon is now shown by a
 *          bright COLOURED BORDER GLOWING around its panel button,
 *          in that button's own accent colour (SINGLE red, SPREAD
 *          yellow, LASER blue, MISSILE green), pulsing between full
 *          and dim brightness. Uncollected weapons stay greyed, and
 *          the border is only ever on the selected weapon.
 *
 * v9 changes (the designer, Nov 19, 2026):
 *  Fix 1: BOSS 3 VERTICAL FIGHT IS SCROLL-ONLY, like boss 1's reverse.
 *          The v7/v8 upward fight moved the ship (flew up a shaft,
 *          faced up, fired up) and parked the boss above — the designer said
 *          that is WRONG. Now the ship stays at its unchanged absolute
 *          position facing right and firing right, the boss parks on
 *          the right facing left and shoots left at the player, and
 *          ONLY the starfield / background / terrain scrolls — up
 *          (stars scroll down) for this boss, easing in from a stop
 *          exactly like boss 1's reverse and boss 2's halt, then
 *          easing back to normal horizontal flight after he dies.
 *          The one terrain change: the CEILING is OPEN during this
 *          boss's section (no ceiling band, floor/ground kept as-is)
 *          so the upward flight reads — the ceiling comes back when
 *          the next area starts after he dies. That is a background
 *          / terrain change only: the ship does NOT move up.
 *          ALL other v7 upward-mode code is removed: no ship
 *          teleport, no facing-up sprite, no vertical shots/beam,
 *          no open shaft the ship flies through. Boss 1 (reverse)
 *          and boss 2 (halt) are PERFECT and untouched; boss 4
 *          unchanged.
 *  Fix 2: ENEMIES AFTER BOSS 2 HAVE MORE HP. Past wave 6 the player
 *          was one-shotting everything and enemies were invisible.
 *          spawn_enemy now adds a flat +2 HP from wave 7 on (v19
 *          cap — the v9 tier formula grew without bound and made
 *          wave 8 nearly impossible; see spawn_enemy): small types
 *          go from 1 HP to 3, tough types proportionally more, so
 *          later enemies survive several hits and stay visible for
 *          seconds, but never get harder than that. Waves 1-6
 *          (boss 1 and level 1, as the designer approved them) are EXACTLY
 *          unchanged. Player fire rate is NOT touched — the fix is
 *          durability.
 *  Fix 3 (follow-up): the ALARM sounds for EVERY boss fight. Code
 *          inspection: spawn_boss is only ever called from the one
 *          boss-intro path, which plays sfx_alarm() x3 (frames 1,
 *          61, 121) plus "!! WARNING !!" and the mode's named cue
 *          (REVERSE! / HALT! / UPWARD! / SHIELD!) for every mode —
 *          no boss was missing it. Verified in QA by the WARNING /
 *          cue text on the sub screen for each of the four bosses
 *          (sound itself cannot be heard headless).
 *
 * v8 changes (the designer, Nov 19, 2026):
 *  Fix 1: PROPER pixelated explosions, DIFFERENT per enemy type.
 *          Enemy deaths no longer look like a ship sprite swapping:
 *          the ship vanishes at the killing hit and a chunky pixel
 *          fireball / debris burst plays instead, coloured per type
 *          (small ships = orange fireball + sparks; bugs/creatures =
 *          green/purple goo burst; crystals = shattering cyan-white
 *          shard burst; robots/metal = grey sparks + smoke; jellyfish
 *          / blobs = purple splat burst; big/tough = larger
 *          multi-stage explosion). A short drift of embers follows,
 *          then score / drops.
 *  Fix 2: BOSS DEATH = multiple consecutive explosions. When a boss
 *          is defeated he does NOT just pop once: a sequence of
 *          bursts goes off around his body, staggered over ~1.5 s
 *          (with a shake), then the final big explosion, and only
 *          then does normal flight resume. Applies to ALL bosses,
 *          including boss 1 (whose fight itself is unchanged).
 *  Fix 3: LEVEL 1 STALAGMITE BUG — root cause found. The main OAM
 *          was initialised with SpriteMapping_1D_32 (32 KB of sprite
 *          VRAM) but the game allocates ~42 KB of sprite tiles, so
 *          every sprite allocated past 32 KB (missile + fire-rate
 *          pickups, lives icons, sparks, booms, ALL terrain obstacle
 *          sprites, score digits) had its tile index wrap and
 *          rendered as some OTHER sprite's tiles. The stalactites /
 *          stalagmites aliased to enemy ship tiles — that is the
 *          "ship icons" the designer saw. Fixed by initialising the main OAM
 *          with SpriteMapping_1D_128 (the full 128 KB VRAM_B bank).
 *          This also restores the boom flames and the missile /
 *          fire-rate pickups, which were aliased the same way.
 *  Fix 4: WEAPON PICKUPS AND PANEL BUTTONS ARE IDENTIFIABLE. Every
 *          pickup now carries a big letter/icon in its own colour
 *          (SPREAD = green "S", LASER = cyan "L", MISSILES = orange
 *          "M", fire-rate = red bolt + speed chevrons — see
 *          tools/convert_v8.py). The bottom-screen buttons carry the
 *          matching letter (1 / S / L / M) in the matching colour;
 *          the selected weapon kept a star marker above its button
 *          (v10 replaces that star with a glowing coloured border —
 *          the star moved with the finger while dragging), and
 *          weapons not yet collected are drawn greyed/dimmed.
 *
 * v7 changes (the designer, Nov 18, 2026):
 *  Req 1: DIFFERENT bosses for the other levels. Boss 1 (wave 3) is
 *          PERFECT as-is — reverse-scroll fight, unchanged. Boss 2
 *          (wave 6) is the HALT boss: the scroll eases to a full stop
 *          for the battle, then speeds back up to normal after he
 *          dies. Boss 3 (wave 9) is the UPWARD boss: v9 redid him as
 *          a SCROLL-ONLY vertical flight (ship and boss keep their
 *          normal positions/orientations; only the starfield and
 *          terrain scroll up) — see v9 Fix 1 above. Boss 4 (wave 12)
 *          is the SHIELD + CHARGE boss: a
 *          periodic shield (invulnerable while up) and charge dashes
 *          across the screen. Bosses cycle 1-2-3-4 after that.
 *          Every boss has its own sprite, attack pattern, HP bar and
 *          intro cue; no other enemies during any boss fight.
 *  Req 2: PROPER death screen. Dying now plays a death animation
 *          (big explosion at the ship, ~1.5 s) and then shows a
 *          GAME OVER screen (final score, wave reached, best score)
 *          that STAYS until the player touches or presses START —
 *          it can no longer auto-restart from a finger still held
 *          down at the moment of death (the v6 bug: ST_OVER read the
 *          still-held touch as a restart tap).
 *  Follow-up: MORE enemy types with TOTALLY DIFFERENT sprites from
 *          the web — 8 new area-unique types (40 total): worms, a
 *          green alien creature, a purple jellyfish, a creeper eye,
 *          a green robot, a tentacled orb — from OpenGameArt (CC0 /
 *          CC-BY-SA, credited in README + title). See
 *          tools/convert_v7.py.
 *
 * v6 changes (the designer, Nov 18, 2026):
 *  Fix 1: NO other enemies during a boss fight. Regular spawns stop,
 *          boss minion spawns are removed, and any regular enemies /
 *          enemy bullets on screen are cleared when a boss fight
 *          starts — the player fights the boss ALONE. Spawning
 *          resumes only after the boss dies.
 *  Fix 2: the reverse effect is a SCROLL transition ONLY. The ship
 *          STAYS on the left, facing right, firing right, boss parks
 *          on the right as normal. Only the starfield + ceiling/floor
 *          / terrain scroll eases down to a stop, then accelerates in
 *          reverse — up to MAX speed, FASTER than normal flight
 *          (160% scroll speed) during the boss fight — then eases
 *          back through a stop to normal speed/direction when the
 *          boss dies. v5's ship-teleport-to-x=200 / flip / mirrored
 *          enemies-bullets-shots code is REMOVED.
 *
 * v5 changes (the designer, Nov 18, 2026):
 *  Fix 1: first boss much easier (slower/smaller spread fan, slower
 *          aimed shots, no minion spam, lower HP); later bosses ramp up.
 *  Fix 2: level 1 much easier — gentler terrain in early areas,
 *          slower enemy bullets, sparser spawns/obstacles early;
 *          difficulty ramps gradually by area/wave.
 *  Fix 3: white screen in melonDS/DeSmuME (only NooDS runs) — fresh
 *          investigation in qa/MELONDS-INVESTIGATION.md (v5 appendix):
 *          header is byte-clean vs the stock libnds hello-world
 *          (logo CRC OK, header CRC OK, ####, same ARM7); DeSmuME
 *          here behaves IDENTICALLY for hello-world and v4 (white
 *          SDL window, PC stuck at entry under its GDB stub) so no
 *          ROM-side cause was found; the decisive test is the designer
 *          loading hello-world in her cores. NO ROM change claimed
 *          to fix it.
 *  Fix 4: animated enemies — spinning, wing-flapping, pulsing and
 *          wobbling types; boss animates; player engine glow.
 *  Fix 5: coloured starfield — purple/blue/red/yellow stars.
 *  Fix 6: reverse-flight first-boss intro — alarm x3, then the
 *          ship flies in reverse (starfield reverses, enemies and
 *          boss come from the left, player fires left) until the
 *          first boss dies, then normal flight resumes.
 *
 * R-Type / Life Force style: player flies left-to-right, enemies come
 * in from the right. TOP screen: playfield. BOTTOM screen: touch
 * control panel — a clean drag area with the weapon panel at the
 * bottom. The bottom screen shows ONLY the area name and the selected
 * weapon (v4 Fix 2); score, lives and wave moved to the top screen
 * as sprite digits / mini-ship icons.
 *
 * MOVEMENT: RELATIVE drag only. The ship NEVER jumps/teleports to the
 * touch point — it moves by the same delta as the stylus drag, from
 * wherever the ship currently is. A tap moves nothing. Weapon-panel
 * taps only switch weapons and never move the ship. D-pad also works.
 *
 * WEAPONS (v3/v4): player starts with SINGLE shot only. SPREAD, LASER
 * and MISSILES are collected as power-up pickups. Getting hit loses
 * the current weapon and drops back to SINGLE. The panel only switches
 * among weapons already collected in that run.
 * FIRE RATE (v4 Fix 7): base fire is slower (deliberate, not a bullet
 * hose). Red-bolt capsules raise the fire-rate level (0-3); getting
 * hit resets it to 0.
 *
 * LASER: a long bright cyan beam with a unique zap SFX. MISSILES:
 * rocket-shaped sprites with a fading spark trail.
 *
 * ENEMIES (v4 Fix 6): 32 types with varying HP (1-8), damage flash,
 * area-unique sets (cave bats/bugs, water fish/jellyfish, crystal
 * shards, goo blobs, base drones...). Graphics: Kenney Space Shooter
 * Redux (CC0 1.0) — v4 uses far more of the pack than v3 did, plus
 * procedural terrain art.
 *
 * EXPLOSIONS (v4 Fix 3): a dying enemy keeps flying forward, drifts
 * up or down while tumbling and trailing fire/sparks, then bursts
 * into a big multi-frame explosion. v8 hid the ship at the killing
 * hit (instant explosion); v18 restores the visible drift — the ship
 * stays up, blinking and trailing embers, for ~1.4 s, then the
 * per-type pixelated explosion fires. v19 tunes it down: only the
 * drifter and weaver drift, ~0.6 s (36 frames) at 0.5 px/frame, and
 * a wreck that reaches the top or bottom of the screen explodes
 * there at once; every other type explodes at the killing hit.
 * Drift direction is NOT random: it comes from where the enemy dies
 * (centre y < 96 -> up, y >= 96 -> down), picked once at the fatal
 * hit and locked.
 *
 * MINI-BOSSES (v4 Fix 4): every 3rd wave a big 64x64 boss parks on
 * the right, attacks in patterns, and BLOCKS progression until it is
 * destroyed (no new waves, no area advance). HP bar on the top screen.
 *
 * AREAS (v4 Fix 5): 7 themed zones with SPRITE-BASED ceiling/floor
 * obstacles unique per area — pipes/wires/columns (base), trees/roots
 * (planet), stalactites/stalagmites (cave), crystal spikes/claws
 * (crystal), plants/coral/tubes (water), slime barbs/blobs (goo) —
 * at varying heights, plus a narrow wavy tunnel section. Areas change
 * every 2 waves.
 *
 * Sound: one unique AI music track per area (assets/music-v2.mp3
 * for area 1, assets/music-area2..7.mp3 for areas 2-7, Speechma),
 * crossfaded at area changes, tempo lifted 25% during boss fights
 * (see music_set_area / music_update). SFX synthesized
 * at 11025 Hz (tools/make_audio.py).
 *
 * Header: game code '####' (melonDS homebrew classification; never
 * stamp a real game code — v2's SDRG caused the RetroArch BIOS error).
 *
 * INIT ORDER (v4 Fix 1): consoleDemoInit() FIRST, then videoSetMode
 * (MAIN only — videoSetModeSub after console kills the console, found
 * in the v4 bisect), then VRAM banks / OAM / BGs. melonDS 1.1's JIT
 * recompiler does not render ANY devkitPro/libnds ROM in this VM
 * (not even the stock hello-world — verified with a checked [JIT]
 * config, FastMemory off, MaxBlockSize 1, 30 s waits), a known
 * melonDS JIT + devkitPro-homebrew issue (cf. Cool-Pizza-NDS README:
 * "turn JIT off ... JIT-enabled startup can produce a black screen").
 * The RetroArch melonDS DS core on iOS is built with JIT DISABLED by
 * default, so JIT is most likely NOT the cause of the designer's white screen
 * there — see qa/MELONDS-INVESTIGATION.md for what she should check
 * (core choice, settings, NooDS/DeSmuME core fallback). With JIT off
 * the game renders here; under JIT it is no worse than hello-world.
 */
#include <nds.h>
/* v12: filesystem.h / nitroFSInit removed — music is embedded now */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* bin2o-generated data (DATA := ../data in arm9/Makefile) */
#include "player_bin.h"
#include "player_pal_bin.h"
#include "drifter_bin.h"
#include "drifter_pal_bin.h"
#include "weaver_bin.h"
#include "weaver_pal_bin.h"
#include "diver_bin.h"
#include "diver_pal_bin.h"
#include "shooter_bin.h"
#include "shooter_pal_bin.h"
#include "asteroid_bin.h"
#include "asteroid_pal_bin.h"
#include "mine_bin.h"
#include "mine_pal_bin.h"
#include "turret_bin.h"
#include "turret_pal_bin.h"
#include "bug_bin.h"
#include "bug_pal_bin.h"
#include "alien_bin.h"
#include "alien_pal_bin.h"
#include "charger_bin.h"
#include "charger_pal_bin.h"
#include "spinner_bin.h"
#include "spinner_pal_bin.h"
#include "orbiter_bin.h"
#include "orbiter_pal_bin.h"
#include "crystal_bin.h"
#include "crystal_pal_bin.h"
#include "capital_bin.h"
#include "capital_pal_bin.h"
#include "dart_bin.h"
#include "dart_pal_bin.h"
#include "rogue_bin.h"
#include "rogue_pal_bin.h"
#include "ufo_bin.h"
#include "ufo_pal_bin.h"
#include "meteor2_bin.h"
#include "meteor2_pal_bin.h"
#include "drone_bin.h"
#include "drone_pal_bin.h"
#include "sentry_bin.h"
#include "sentry_pal_bin.h"
#include "wasp_bin.h"
#include "wasp_pal_bin.h"
#include "beetle_bin.h"
#include "beetle_pal_bin.h"
#include "bat_bin.h"
#include "bat_pal_bin.h"
#include "fish_bin.h"
#include "fish_pal_bin.h"
#include "jelly_bin.h"
#include "jelly_pal_bin.h"
#include "shard_bin.h"
#include "shard_pal_bin.h"
#include "blob_bin.h"
#include "blob_pal_bin.h"
#include "crawler_bin.h"
#include "crawler_pal_bin.h"
#include "eel_bin.h"
#include "eel_pal_bin.h"
#include "prism_bin.h"
#include "prism_pal_bin.h"
#include "spore_bin.h"
#include "spore_pal_bin.h"
#include "hunter_bin.h"
#include "hunter_pal_bin.h"
#include "boss_bin.h"
#include "boss_pal_bin.h"
#include "boss2_bin.h"
#include "boss2_pal_bin.h"
/* v7: new web-sourced sprites (see tools/convert_v7.py) */
#include "wormg_bin.h"
#include "wormr_bin.h"
#include "wormb_bin.h"
#include "alien2_bin.h"
#include "alien2b_bin.h"
#include "jelly2_bin.h"
#include "jelly2_pal_bin.h"
#include "jelly2b_bin.h"
#include "eye_bin.h"
#include "robot_bin.h"
#include "tent_bin.h"
#include "boss3_bin.h"
#include "boss4_bin.h"
#include "player_up_bin.h"
#include "laserbeam_v_bin.h"
#include "bullet_p_bin.h"
#include "bullet_p_pal_bin.h"
#include "bullet_e_bin.h"
#include "bullet_e_pal_bin.h"
#include "missile_bin.h"
#include "missile_pal_bin.h"
#include "missile2_bin.h"
#include "missile2_pal_bin.h"
#include "laserbeam_bin.h"
#include "laserbeam_pal_bin.h"
#include "pow_spread_bin.h"
#include "pow_spread_pal_bin.h"
#include "pow_laser_bin.h"
#include "pow_laser_pal_bin.h"
#include "pow_miss_bin.h"
#include "pow_miss_pal_bin.h"
#include "pow_rate_bin.h"
#include "pow_rate_pal_bin.h"
#include "pow_1up_bin.h"       /* v16: 1UP pickup sprite (shares
                                  PAL_MISSILE; gold/pink patched into
                                  missile palette indices 14/15) */
#include "player_mini_bin.h"
#include "player_mini_pal_bin.h"
#include "spark_bin.h"
#include "spark_pal_bin.h"
#include "boom0_bin.h"
#include "boom0_pal_bin.h"
#include "boom1_bin.h"
#include "boom1_pal_bin.h"
#include "boom2_bin.h"
#include "boom2_pal_bin.h"
#include "reticle_bin.h"
#include "reticle_pal_bin.h"
#include "btn_single_bin.h"
#include "btn_single_pal_bin.h"
#include "btn_spread_bin.h"
#include "btn_spread_pal_bin.h"
#include "btn_laser_bin.h"
#include "btn_laser_pal_bin.h"
#include "btn_miss_bin.h"
#include "btn_miss_pal_bin.h"
#include "terrain_base_bin.h"
#include "terrain_base_pal_bin.h"
#include "terrain_planet_bin.h"
#include "terrain_planet_pal_bin.h"
#include "terrain_cave_bin.h"
#include "terrain_cave_pal_bin.h"
#include "terrain_crystal_bin.h"
#include "terrain_crystal_pal_bin.h"
#include "terrain_water_bin.h"
#include "terrain_water_pal_bin.h"
#include "terrain_goo_bin.h"
#include "terrain_goo_pal_bin.h"
#include "obs_pipe_bin.h"
#include "obs_pipe_pal_bin.h"
#include "obs_column_bin.h"
#include "obs_column_pal_bin.h"
#include "obs_tree_bin.h"
#include "obs_tree_pal_bin.h"
#include "obs_root_bin.h"
#include "obs_root_pal_bin.h"
#include "obs_stalact_bin.h"
#include "obs_stalact_pal_bin.h"
#include "obs_stalagm_bin.h"
#include "obs_stalagm_pal_bin.h"
#include "obs_cryspike_bin.h"
#include "obs_cryspike_pal_bin.h"
#include "obs_claw_bin.h"
#include "obs_claw_pal_bin.h"
#include "obs_plant_bin.h"
#include "obs_plant_pal_bin.h"
#include "obs_tube_bin.h"
#include "obs_tube_pal_bin.h"
#include "obs_barb_bin.h"
#include "obs_barb_pal_bin.h"
#include "obs_blob_bin.h"
#include "obs_blob_pal_bin.h"
#include "shoot_raw_bin.h"
#include "boom_raw_bin.h"
#include "laser_raw_bin.h"
#include "pickup_raw_bin.h"
#include "alarm_raw_bin.h"
#include "bosshit_raw_bin.h"   /* v16: boss-damage tick */
#include "playerdmg_raw_bin.h" /* v17: player-damage sound */

/* ------------------------------------------------------------------ */
/* sprite setup                                                        */
/* ------------------------------------------------------------------ */

#define PAL_PLAYER  0
#define PAL_DRIFTER 1
#define PAL_WEAVER  2
#define PAL_DIVER   3
#define PAL_SHOOTER 4
#define PAL_ASTEROID 5
#define PAL_MINE    6
#define PAL_TURRET  7
#define PAL_BULLET_P 8
#define PAL_BULLET_E 9
#define PAL_MISSILE 10
#define PAL_BOOM    11
#define PAL_LASERBEAM 12
#define PAL_POW_SPREAD 13
#define PAL_POW_LASER  14
#define PAL_JELLY 15   /* v7: jellyfish owns the last free slot */
/* slot 15 free. pow_miss reuses PAL_MISSILE; spark reuses PAL_BOOM;
   pow_rate reuses PAL_DRIFTER; player_mini reuses PAL_PLAYER;
   obs_* reuse SHOOTER(4)/WEAVER(2)/ASTEROID(5)/DIVER(3);
   boss reuses SHOOTER, boss2 reuses WEAVER; digits reuse PAL_BOOM.
   v7: wormg/alien2/boss3 reuse WEAVER, wormr reuses DRIFTER,
   wormb/eye/tent reuse DIVER, robot/boss4 reuse SHOOTER,
   player_up reuses PLAYER, laserbeam_v reuses LASERBEAM. */
#define PAL_RETICLE 0   /* sub screen palette slot */
#define PAL_BTN_SINGLE 1
#define PAL_BTN_SPREAD 2
#define PAL_BTN_LASER  3
#define PAL_BTN_MISS   4
#define PAL_BTN_LOCKED 5   /* v8 Fix 4: grey palette for uncollected weapons */
#define PAL_BORDER     6   /* v10 Change 2: glowing selection border (sub) */

static u16 *gfx_player, *gfx_drifter, *gfx_weaver, *gfx_diver;
static u16 *gfx_shooter, *gfx_asteroid, *gfx_mine, *gfx_turret;
static u16 *gfx_bug, *gfx_alien, *gfx_charger, *gfx_spinner;
static u16 *gfx_orbiter, *gfx_crystal, *gfx_capital;
static u16 *gfx_enemy[40];
static u16 *gfx_enemy_alt[40];   /* v7: 2nd animation frame, or NULL */
static u16 *gfx_boss[4];
static u16 *gfx_player_up;
static u16 *gfx_laserbeam_v;
static u16 *gfx_bullet_p, *gfx_bullet_e, *gfx_missile, *gfx_missile2;
static u16 *gfx_laserbeam;
static u16 *gfx_pow[5];   /* v16: was [4]; +1UP pickup */
static u16 *gfx_player_mini;
static u16 *gfx_spark;
static u16 *gfx_boom[3];
static u16 *gfx_reticle;
static u16 *gfx_btn[4];
static u16 *gfx_border;   /* v10 Change 2: glowing selection border (sub) */
static u16 *gfx_obs[12];
static u16 *gfx_digits[11];   /* 0-9 + 'W' */

static void load_sprites(void) {
    /* v8 Fix 3: 1D_128, NOT 1D_32. The game allocates ~42 KB of main
       sprite tiles; 1D_32 only maps 32 KB, so every sprite past that
       (booms, sparks, pickups, lives icons, terrain obstacles, score
       digits) had its tile index wrap and rendered as another
       sprite's tiles — the stalagmites showed up as SHIP icons. */
    oamInit(&oamMain, SpriteMapping_1D_128, false);
    oamInit(&oamSub, SpriteMapping_1D_128, false);

    gfx_player   = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_drifter  = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_weaver   = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_diver    = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_shooter  = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_asteroid = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_mine     = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_turret   = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_bug      = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_alien    = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_charger  = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_spinner  = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_orbiter  = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_crystal  = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_capital  = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    /* enemy gfx table: order must match the ET_* enum below */
    gfx_enemy[0]  = gfx_drifter;  gfx_enemy[1]  = gfx_weaver;
    gfx_enemy[2]  = gfx_diver;    gfx_enemy[3]  = gfx_shooter;
    gfx_enemy[4]  = gfx_asteroid; gfx_enemy[5]  = gfx_mine;
    gfx_enemy[6]  = gfx_turret;   gfx_enemy[7]  = gfx_bug;
    gfx_enemy[8]  = gfx_alien;    gfx_enemy[9]  = gfx_charger;
    gfx_enemy[10] = gfx_spinner;  gfx_enemy[11] = gfx_orbiter;
    gfx_enemy[12] = gfx_crystal;  gfx_enemy[13] = gfx_capital;
    gfx_enemy[14] = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_enemy[15] = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_enemy[16] = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_enemy[17] = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_enemy[18] = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_enemy[19] = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_enemy[20] = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_enemy[21] = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_enemy[22] = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_enemy[23] = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_enemy[24] = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_enemy[25] = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_enemy[26] = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_enemy[27] = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_enemy[28] = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_enemy[29] = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_enemy[30] = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_enemy[31] = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    /* v7: 8 new web-sourced enemy types (indices 32-39) */
    for (int i = 32; i < 40; i++)
        gfx_enemy[i] = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    memset(gfx_enemy_alt, 0, sizeof(gfx_enemy_alt));
    gfx_enemy_alt[35] = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_enemy_alt[36] = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_boss[0]   = oamAllocateGfx(&oamMain, SpriteSize_64x64, SpriteColorFormat_16Color);
    gfx_boss[1]   = oamAllocateGfx(&oamMain, SpriteSize_64x64, SpriteColorFormat_16Color);
    gfx_boss[2]   = oamAllocateGfx(&oamMain, SpriteSize_64x64, SpriteColorFormat_16Color);
    gfx_boss[3]   = oamAllocateGfx(&oamMain, SpriteSize_64x64, SpriteColorFormat_16Color);
    gfx_player_up = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_laserbeam_v = oamAllocateGfx(&oamMain, SpriteSize_8x32, SpriteColorFormat_16Color);
    gfx_bullet_p = oamAllocateGfx(&oamMain, SpriteSize_32x8, SpriteColorFormat_16Color);
    gfx_bullet_e = oamAllocateGfx(&oamMain, SpriteSize_32x8, SpriteColorFormat_16Color);
    gfx_missile  = oamAllocateGfx(&oamMain, SpriteSize_32x8, SpriteColorFormat_16Color);
    gfx_missile2 = oamAllocateGfx(&oamMain, SpriteSize_32x8, SpriteColorFormat_16Color);
    gfx_laserbeam = oamAllocateGfx(&oamMain, SpriteSize_32x8, SpriteColorFormat_16Color);
    gfx_pow[0]   = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_pow[1]   = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_pow[2]   = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_pow[3]   = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_pow[4]   = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);   /* v16: 1UP */
    gfx_player_mini = oamAllocateGfx(&oamMain, SpriteSize_16x16, SpriteColorFormat_16Color);
    gfx_spark    = oamAllocateGfx(&oamMain, SpriteSize_16x16, SpriteColorFormat_16Color);
    gfx_boom[0]  = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_boom[1]  = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    gfx_boom[2]  = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    for (int i = 0; i < 12; i++)
        gfx_obs[i] = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
    for (int i = 0; i < 11; i++)
        gfx_digits[i] = oamAllocateGfx(&oamMain, SpriteSize_16x16, SpriteColorFormat_16Color);
    gfx_reticle  = oamAllocateGfx(&oamSub, SpriteSize_16x16, SpriteColorFormat_16Color);
    gfx_btn[0]   = oamAllocateGfx(&oamSub, SpriteSize_64x32, SpriteColorFormat_16Color);
    gfx_btn[1]   = oamAllocateGfx(&oamSub, SpriteSize_64x32, SpriteColorFormat_16Color);
    gfx_btn[2]   = oamAllocateGfx(&oamSub, SpriteSize_64x32, SpriteColorFormat_16Color);
    gfx_btn[3]   = oamAllocateGfx(&oamSub, SpriteSize_64x32, SpriteColorFormat_16Color);
    gfx_border   = oamAllocateGfx(&oamSub, SpriteSize_64x32, SpriteColorFormat_16Color);

    dmaCopy(player_bin,  gfx_player,  player_bin_size);
    dmaCopy(drifter_bin, gfx_drifter, drifter_bin_size);
    dmaCopy(weaver_bin,  gfx_weaver,  weaver_bin_size);
    dmaCopy(diver_bin,   gfx_diver,   diver_bin_size);
    dmaCopy(shooter_bin, gfx_shooter, shooter_bin_size);
    dmaCopy(asteroid_bin, gfx_asteroid, asteroid_bin_size);
    dmaCopy(mine_bin,    gfx_mine,    mine_bin_size);
    dmaCopy(turret_bin,  gfx_turret,  turret_bin_size);
    dmaCopy(bug_bin,     gfx_bug,     bug_bin_size);
    dmaCopy(alien_bin,   gfx_alien,   alien_bin_size);
    dmaCopy(charger_bin, gfx_charger, charger_bin_size);
    dmaCopy(spinner_bin, gfx_spinner, spinner_bin_size);
    dmaCopy(orbiter_bin, gfx_orbiter, orbiter_bin_size);
    dmaCopy(crystal_bin, gfx_crystal, crystal_bin_size);
    dmaCopy(capital_bin, gfx_capital, capital_bin_size);
    dmaCopy(dart_bin,    gfx_enemy[14], dart_bin_size);
    dmaCopy(rogue_bin,   gfx_enemy[15], rogue_bin_size);
    dmaCopy(ufo_bin,     gfx_enemy[16], ufo_bin_size);
    dmaCopy(meteor2_bin, gfx_enemy[17], meteor2_bin_size);
    dmaCopy(drone_bin,   gfx_enemy[18], drone_bin_size);
    dmaCopy(sentry_bin,  gfx_enemy[19], sentry_bin_size);
    dmaCopy(wasp_bin,    gfx_enemy[20], wasp_bin_size);
    dmaCopy(beetle_bin,  gfx_enemy[21], beetle_bin_size);
    dmaCopy(bat_bin,     gfx_enemy[22], bat_bin_size);
    dmaCopy(fish_bin,    gfx_enemy[23], fish_bin_size);
    dmaCopy(jelly_bin,   gfx_enemy[24], jelly_bin_size);
    dmaCopy(shard_bin,   gfx_enemy[25], shard_bin_size);
    dmaCopy(blob_bin,    gfx_enemy[26], blob_bin_size);
    dmaCopy(crawler_bin, gfx_enemy[27], crawler_bin_size);
    dmaCopy(eel_bin,     gfx_enemy[28], eel_bin_size);
    dmaCopy(prism_bin,   gfx_enemy[29], prism_bin_size);
    dmaCopy(spore_bin,   gfx_enemy[30], spore_bin_size);
    dmaCopy(hunter_bin,  gfx_enemy[31], hunter_bin_size);
    /* v7 web-sourced enemies */
    dmaCopy(wormg_bin,   gfx_enemy[32], wormg_bin_size);
    dmaCopy(wormr_bin,   gfx_enemy[33], wormr_bin_size);
    dmaCopy(wormb_bin,   gfx_enemy[34], wormb_bin_size);
    dmaCopy(alien2_bin,  gfx_enemy[35], alien2_bin_size);
    dmaCopy(alien2b_bin, gfx_enemy_alt[35], alien2b_bin_size);
    dmaCopy(jelly2_bin,  gfx_enemy[36], jelly2_bin_size);
    dmaCopy(jelly2b_bin, gfx_enemy_alt[36], jelly2b_bin_size);
    dmaCopy(eye_bin,     gfx_enemy[37], eye_bin_size);
    dmaCopy(robot_bin,   gfx_enemy[38], robot_bin_size);
    dmaCopy(tent_bin,    gfx_enemy[39], tent_bin_size);
    dmaCopy(boss_bin,    gfx_boss[0], boss_bin_size);
    dmaCopy(boss2_bin,   gfx_boss[1], boss2_bin_size);
    dmaCopy(boss3_bin,   gfx_boss[2], boss3_bin_size);
    dmaCopy(boss4_bin,   gfx_boss[3], boss4_bin_size);
    dmaCopy(player_up_bin, gfx_player_up, player_up_bin_size);
    dmaCopy(laserbeam_v_bin, gfx_laserbeam_v, laserbeam_v_bin_size);
    dmaCopy(bullet_p_bin, gfx_bullet_p, bullet_p_bin_size);
    dmaCopy(bullet_e_bin, gfx_bullet_e, bullet_e_bin_size);
    dmaCopy(missile_bin, gfx_missile, missile_bin_size);
    dmaCopy(missile2_bin, gfx_missile2, missile2_bin_size);
    dmaCopy(laserbeam_bin, gfx_laserbeam, laserbeam_bin_size);
    dmaCopy(pow_spread_bin, gfx_pow[0], pow_spread_bin_size);
    dmaCopy(pow_laser_bin,  gfx_pow[1], pow_laser_bin_size);
    dmaCopy(pow_miss_bin,   gfx_pow[2], pow_miss_bin_size);
    dmaCopy(pow_rate_bin,   gfx_pow[3], pow_rate_bin_size);
    dmaCopy(pow_1up_bin,    gfx_pow[4], pow_1up_bin_size);   /* v16: 1UP */
    dmaCopy(player_mini_bin, gfx_player_mini, player_mini_bin_size);
    dmaCopy(spark_bin,   gfx_spark,   spark_bin_size);
    dmaCopy(boom0_bin, gfx_boom[0], boom0_bin_size);
    dmaCopy(boom1_bin, gfx_boom[1], boom1_bin_size);
    dmaCopy(boom2_bin, gfx_boom[2], boom2_bin_size);
    dmaCopy(obs_pipe_bin,    gfx_obs[0], obs_pipe_bin_size);
    dmaCopy(obs_column_bin,  gfx_obs[1], obs_column_bin_size);
    dmaCopy(obs_tree_bin,    gfx_obs[2], obs_tree_bin_size);
    dmaCopy(obs_root_bin,    gfx_obs[3], obs_root_bin_size);
    dmaCopy(obs_stalact_bin, gfx_obs[4], obs_stalact_bin_size);
    dmaCopy(obs_stalagm_bin, gfx_obs[5], obs_stalagm_bin_size);
    dmaCopy(obs_cryspike_bin, gfx_obs[6], obs_cryspike_bin_size);
    dmaCopy(obs_claw_bin,    gfx_obs[7], obs_claw_bin_size);
    dmaCopy(obs_plant_bin,   gfx_obs[8], obs_plant_bin_size);
    dmaCopy(obs_tube_bin,    gfx_obs[9], obs_tube_bin_size);
    dmaCopy(obs_barb_bin,    gfx_obs[10], obs_barb_bin_size);
    dmaCopy(obs_blob_bin,    gfx_obs[11], obs_blob_bin_size);
    dmaCopy(reticle_bin, gfx_reticle, reticle_bin_size);
    dmaCopy(btn_single_bin, gfx_btn[0], btn_single_bin_size);
    dmaCopy(btn_spread_bin, gfx_btn[1], btn_spread_bin_size);
    dmaCopy(btn_laser_bin,  gfx_btn[2], btn_laser_bin_size);
    dmaCopy(btn_miss_bin,   gfx_btn[3], btn_miss_bin_size);

    dmaCopy(player_pal_bin,  &SPRITE_PALETTE[PAL_PLAYER * 16], 32);
    dmaCopy(drifter_pal_bin, &SPRITE_PALETTE[PAL_DRIFTER * 16], 32);
    dmaCopy(weaver_pal_bin,  &SPRITE_PALETTE[PAL_WEAVER * 16], 32);
    dmaCopy(diver_pal_bin,   &SPRITE_PALETTE[PAL_DIVER * 16], 32);
    dmaCopy(shooter_pal_bin, &SPRITE_PALETTE[PAL_SHOOTER * 16], 32);
    dmaCopy(asteroid_pal_bin, &SPRITE_PALETTE[PAL_ASTEROID * 16], 32);
    dmaCopy(mine_pal_bin,    &SPRITE_PALETTE[PAL_MINE * 16], 32);
    dmaCopy(turret_pal_bin,  &SPRITE_PALETTE[PAL_TURRET * 16], 32);
    dmaCopy(bullet_p_pal_bin, &SPRITE_PALETTE[PAL_BULLET_P * 16], 32);
    dmaCopy(bullet_e_pal_bin, &SPRITE_PALETTE[PAL_BULLET_E * 16], 32);
    dmaCopy(missile_pal_bin, &SPRITE_PALETTE[PAL_MISSILE * 16], 32);
    dmaCopy(boom0_pal_bin,   &SPRITE_PALETTE[PAL_BOOM * 16], 32);
    dmaCopy(laserbeam_pal_bin, &SPRITE_PALETTE[PAL_LASERBEAM * 16], 32);
    dmaCopy(pow_spread_pal_bin, &SPRITE_PALETTE[PAL_POW_SPREAD * 16], 32);
    dmaCopy(pow_laser_pal_bin,  &SPRITE_PALETTE[PAL_POW_LASER * 16], 32);
    dmaCopy(jelly2_pal_bin,     &SPRITE_PALETTE[PAL_JELLY * 16], 32);
    dmaCopy(reticle_pal_bin, &SPRITE_PALETTE_SUB[PAL_RETICLE * 16], 32);
    dmaCopy(btn_single_pal_bin, &SPRITE_PALETTE_SUB[PAL_BTN_SINGLE * 16], 32);
    dmaCopy(btn_spread_pal_bin, &SPRITE_PALETTE_SUB[PAL_BTN_SPREAD * 16], 32);
    dmaCopy(btn_laser_pal_bin,  &SPRITE_PALETTE_SUB[PAL_BTN_LASER * 16], 32);
    dmaCopy(btn_miss_pal_bin,   &SPRITE_PALETTE_SUB[PAL_BTN_MISS * 16], 32);
    /* v8 Fix 4: locked-weapon palette — flat greys, so an uncollected
       weapon's button renders dimmed (body light grey, border and
       letter dark grey, still faintly readable). */
    for (int i = 0; i < 16; i++)
        SPRITE_PALETTE_SUB[PAL_BTN_LOCKED * 16 + i] =
            (i == 0) ? RGB15(0, 0, 0) :
            (i >= 11 && i <= 14) ? RGB15(9, 9, 9) : RGB15(16, 16, 16);
}

/* 5x7 digit font + 'W' (index 10), drawn at runtime into 16x16
   sprite tiles (palette index 1 of PAL_BOOM = bright yellow-white). */
static const u8 digit_font[11][7] = {
    {0x0E,0x11,0x13,0x15,0x19,0x11,0x0E}, /* 0 */
    {0x04,0x0C,0x04,0x04,0x04,0x04,0x0E}, /* 1 */
    {0x0E,0x11,0x01,0x06,0x08,0x10,0x1F}, /* 2 */
    {0x1F,0x02,0x04,0x02,0x01,0x11,0x0E}, /* 3 */
    {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02}, /* 4 */
    {0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E}, /* 5 */
    {0x06,0x08,0x10,0x1E,0x11,0x11,0x0E}, /* 6 */
    {0x1F,0x01,0x02,0x04,0x08,0x08,0x08}, /* 7 */
    {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E}, /* 8 */
    {0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C}, /* 9 */
    {0x11,0x11,0x11,0x15,0x15,0x1B,0x11}, /* W */
};

static void build_digits(void) {
    /* 16x16 tile = 4x4 8x8 tiles, 4bpp: 128 bytes per glyph */
    for (int g = 0; g < 11; g++) {
        u8 idx[16][16];
        memset(idx, 0, sizeof(idx));
        for (int row = 0; row < 7; row++) {
            for (int col = 0; col < 5; col++) {
                if (digit_font[g][row] & (1 << (4 - col)))
                    idx[4 + row][5 + col] = 1;
            }
        }
        u8 out[128];
        int p = 0;
        for (int ty = 0; ty < 16; ty += 8)
            for (int tx = 0; tx < 16; tx += 8)
                for (int y = 0; y < 8; y++)
                    for (int x = 0; x < 8; x += 2)
                        out[p++] = idx[ty + y][tx + x] |
                                   (idx[ty + y][tx + x + 1] << 4);
        dmaCopy(out, gfx_digits[g], 128);
    }
}

/* v10 Change 2: the glowing selection border — a 64x32 sprite tile
   that is transparent except for a 2 px outline in palette index 1.
   Its colour lives in PAL_BORDER index 1 and is rewritten every
   frame (see draw_play), so one tile serves all four weapons.
   4bpp tile layout: 8x4 tiles of 8x8, low nibble = even x. */
static void build_border(void) {
    /* u16 array (naturally aligned) + CPU copy, NOT dmaCopy: QA proved
       the 1024-byte dmaCopy from the stack array never lands in
       VRAM_D (readback 0x0000, nothing renders), while a CPU halfword
       loop lands (readback 0x1111, sprite renders). The 128-byte
       digits dmaCopy works, so this is specific to this call — a
       512-halfword CPU copy once at boot costs nothing. Do NOT
       "optimise" this back to dmaCopy. */
    u16 out[512];
    memset(out, 0, sizeof(out));
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if (x < 2 || x >= 62 || y < 2 || y >= 30) {
                int tile = (y / 8) * 8 + (x / 8);
                int off = tile * 32 + (y % 8) * 4 + (x % 8) / 2;
                /* Byte off: low nibble = even x, high nibble = odd x;
                   the byte is the low byte of out[off/2] when off is
                   even, the high byte when off is odd. */
                out[off / 2] |= (u16)((x % 2 == 0 ? 1 : 0x10)
                                      << ((off % 2) * 8));
            }
        }
    }
    u16 *dst = gfx_border;
    for (int i = 0; i < 512; i++) dst[i] = out[i];
}

/* v13 Change 5: the selection border is RAINBOW now, not a single
   fixed weapon colour. The rainbow_col() function lives just above
   draw_play (it needs `frame`, declared further down): its hue
   cycles smoothly through the spectrum over a 4 s (240-frame)
   cycle while its brightness breathes in and out over a 2 s sine
   (40-100%, never off) — a glow, not a blink. The v10/v11
   border_col[] weapon colours are gone with the old glow_col();
   the border is rainbow on every weapon. Only ever on the selected
   weapon; uncollected weapons stay greyed. */

/* ------------------------------------------------------------------ */
/* starfield background: 3 parallax layers on the main screen         */
/* ------------------------------------------------------------------ */

static int bg_far, bg_mid, bg_near, bg_terrain, bgSub;
static u16 far_map_save[1024];   /* v13: far starfield map, saved so the
   title/game-over art can swap it out and start_game() restore it */
static int scroll_far, scroll_mid, scroll_near;
static int scroll_far_y, scroll_mid_y, scroll_near_y;  /* v9: vertical scroll offsets */
/* v6: scroll speed/direction ONLY. Gameplay always flies left-to-
   right (dir_fly is always +1); only the world scroll reverses.
   scroll_vel is percent of normal scroll speed: +100 normal,
   0 stopped, -160 = max-speed reverse (faster than normal flight).
   It eases toward scroll_target by 2/frame (~2 s for a full flip). */
static int dir_fly = 1;
static int scroll_vel = 100;
static int scroll_target = 100;
/* v9 Fix 1: vertical scroll velocity, % of normal scroll speed,
   eased exactly like scroll_vel (2/frame toward its target).
   -100 = background scrolls DOWN = the ship is flying UP;
   +100 = background scrolls UP = flying DOWN. Boss 3 flies UP
   (target -160, faster than normal, like boss 1's -160 reverse).
   (Sign found by QA: increasing the BG scroll Y moves content UP
   on screen — v7's +160 scrolled the stars the wrong way and
   lifted the floor band into the middle of the playfield.)
   There is NO vertical_mode any more — the ship/boss never change
   position or orientation for a vertical flight, only the world
   scrolls. */
static int vert_vel = 0;
static int vert_target = 0;

/* v13 Change 3 — VRAM LAYOUT (do not move these bases casually):
   the four main-screen maps live at bases 16-19 (0x06008000+),
   NOT at the stock 2-5. Map bases 2-5 sit inside tile base 0's
   16 KB (0x06000000-0x06003FFF), so the title/game-over art tiles
   (tile base 0, tiles 16..460) overwrote the starfield maps and
   the top screen rendered as garbage coloured blocks (caught in
   QA, Nov 19, 2026). With the maps at 16-19, all of tile base 0
   (tiles 0..511) is free for art: starfield tiles 0-7, title art
   16..230, game-over art 239..460. Terrain tiles stay at tile
   base 1 (0x06004000); its map is base 19. */
static void build_starfield(void) {
    /* v5 Fix 5: tiles 4-7 are single-pixel stars in palettes 4-7
       (purple / blue / red / yellow) for a coloured starfield. */
    static const u32 tiles[8 * 8] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0x00001000, 0, 0, 0, 0,
        0, 0, 0x00000020, 0, 0, 0, 0, 0,
        0, 0, 0x00003000, 0x00033300, 0x00003000, 0, 0, 0,
        0, 0, 0, 0x00001000, 0, 0, 0, 0,   /* purple star */
        0, 0, 0, 0x00001000, 0, 0, 0, 0,   /* blue star */
        0, 0, 0, 0x00001000, 0, 0, 0, 0,   /* red star */
        0, 0, 0, 0x00001000, 0, 0, 0, 0,   /* yellow star */
    };
    dmaCopy(tiles, BG_TILE_RAM(0), sizeof(tiles));
    BG_PALETTE[0] = RGB15(0, 0, 0);
    BG_PALETTE[1] = RGB15(31, 31, 31);
    BG_PALETTE[2] = RGB15(20, 24, 31);
    BG_PALETTE[3] = RGB15(28, 30, 31);
    BG_PALETTE[17] = RGB15(20, 20, 26);
    BG_PALETTE[18] = RGB15(14, 16, 22);
    BG_PALETTE[19] = RGB15(22, 22, 28);
    BG_PALETTE[33] = RGB15(12, 12, 16);
    BG_PALETTE[34] = RGB15(8, 9, 14);
    BG_PALETTE[35] = RGB15(14, 14, 18);
    /* v5 Fix 5: coloured star palettes (index 1 of each) */
    BG_PALETTE[64 + 1]  = RGB15(28, 16, 31);  /* purple */
    BG_PALETTE[80 + 1]  = RGB15(16, 16, 31);  /* blue */
    BG_PALETTE[96 + 1]  = RGB15(31, 12, 12);  /* red */
    BG_PALETTE[112 + 1] = RGB15(31, 28, 8);   /* yellow */

    for (int i = 0; i < 32 * 32; i++) {
        int r = rand() % 100;
        u16 t = 0;
        if (r < 5) t = 1 | (2 << 12);
        else if (r < 8) t = 2 | (2 << 12);
        else if (r < 9) t = 4 | (4 << 12);   /* purple star */
        else if (r < 10) t = 5 | (5 << 12);  /* blue star */
        BG_MAP_RAM(18)[i] = t;
    }
    for (int i = 0; i < 32 * 32; i++) {
        int r = rand() % 100;
        u16 t = 0;
        if (r < 4) t = 1 | (1 << 12);
        else if (r < 6) t = 2 | (1 << 12);
        else if (r < 7) t = 3 | (1 << 12);
        else if (r < 8) t = 6 | (6 << 12);   /* red star */
        BG_MAP_RAM(17)[i] = t;
    }
    for (int i = 0; i < 32 * 32; i++) {
        int r = rand() % 100;
        u16 t = 0;
        if (r < 2) t = 1;
        else if (r < 3) t = 3;
        else if (r < 4) t = 7 | (7 << 12);   /* yellow star */
        BG_MAP_RAM(16)[i] = t;
    }
    bg_near = bgInit(0, BgType_Text4bpp, BgSize_T_256x256, 16, 0);
    bg_mid  = bgInit(1, BgType_Text4bpp, BgSize_T_256x256, 17, 0);
    bg_far  = bgInit(2, BgType_Text4bpp, BgSize_T_256x256, 18, 0);
}

static void scroll_starfield(void) {
    /* v6 Fix 2: ease the scroll speed toward its target, through a
       stop, then accelerate the other way. Normal +100, reverse
       -160 (max speed, faster than normal flight). */
    if (scroll_vel < scroll_target) { scroll_vel += 2; if (scroll_vel > scroll_target) scroll_vel = scroll_target; }
    else if (scroll_vel > scroll_target) { scroll_vel -= 2; if (scroll_vel < scroll_target) scroll_vel = scroll_target; }
    scroll_near += 4 * scroll_vel / 100;
    scroll_mid  += 2 * scroll_vel / 100;
    scroll_far  += 1 * scroll_vel / 100;
    /* v9 Fix 1: vertical scroll eases toward vert_target exactly
       like the horizontal scroll eases toward scroll_target — it
       ramps up from a stop, holds, then eases back to 0. */
    if (vert_vel < vert_target) { vert_vel += 2; if (vert_vel > vert_target) vert_vel = vert_target; }
    else if (vert_vel > vert_target) { vert_vel -= 2; if (vert_vel < vert_target) vert_vel = vert_target; }
    scroll_near_y += 4 * vert_vel / 100;   /* vert_vel < 0 during the
    scroll_mid_y  += 2 * vert_vel / 100;      upward flight: scroll Y
    scroll_far_y  += 1 * vert_vel / 100;      falls, stars move DOWN */
    bgSetScroll(bg_near, scroll_near / 2, scroll_near_y / 2);
    bgSetScroll(bg_mid,  scroll_mid / 2, scroll_mid_y / 2);
    bgSetScroll(bg_far,  scroll_far / 2, scroll_far_y / 2);
}

/* ------------------------------------------------------------------ */
/* terrain areas: thin ceiling/floor bands on BG layer 3 (backing)    */
/* plus SPRITE obstacles unique per area (v4 Fix 5) and a narrow      */
/* wavy tunnel section (v4). Heights are in 8px tiles, 32 columns.    */
/* ------------------------------------------------------------------ */

#define NUM_AREAS 7
static const char *area_name[NUM_AREAS] = {
    "OPEN SPACE", "SPACE BASE", "PLANET", "CAVE",
    "CRYSTAL", "WATER", "GOO"
};
static const u8 *area_tile[NUM_AREAS];
static int area_top_base[NUM_AREAS] = {0, 1, 1, 2, 2, 2, 2};
static int area_bot_base[NUM_AREAS] = {0, 1, 1, 2, 2, 2, 2};

/* v12 Change 1: EMBEDDED MUSIC PER AREA (revert from v11 NitroFS).
   ROOT CAUSE of the v11 silence: v11 moved the 7 tracks into NitroFS
   (nitro/music/areaN.raw) with a chunked streaming loader. The music
   plays correctly in melonDS (verified by audio capture: bright,
   centroid ~800 Hz, real musical content — the "70 Hz drone" seen in
   a 1 s FFT peak is just the track's bass fundamental, not a defect),
   but the designer hears NO music at all on her iPhone in NooDS — only the
   shooting SFX. v10's music was EMBEDDED and she heard it fine. The
   v11 loader fails silently on NooDS (fopen returns NULL, the game
   continues without music), so the NitroFS path is not reliable on
   her device. v12 reverts to the proven v10 architecture: the 7
   unique tracks are EMBEDDED in the ARM9 binary (audio/music_aN.bin
   via bin2o) and played with soundPlaySample at full volume — no
   file I/O, no streaming, no volume-0 start. The tracks are 7.5 s
   loops at 22050 Hz in SIGNED 16-BIT PCM (tools/convert_music_v16.py:
   v16 changed the format from 8-bit — see the v16 header note — the
   8-bit quantization SNR of 28.5 dB was the distortion the designer heard in
   the v15 in-game clips; 16-bit measures 75-82 dB. Same windows as
   v15 — area 1 uses window 0, exactly v12's window; areas 2-7 use
   the loudest CLEAN window, peak <= 1.00 and squash <= 0.05%;
   peak-safe gain only, shipped peaks ~0.42, the same as v12 area-1's
   proven peak) so they fit calico's 0x27c000 ARM9 load-image cap;
   16-bit doubles the size, so 14 s loops (4.32 MB) cannot fit and
   the loops are 7.5 s (2,315,250 bytes total, ARM9 ~2.53 MB). Area
   changes crossfade: on a NORMAL (non-boss) area change the new
   track starts at full volume immediately (always audible, even if
   the fade machinery misbehaves), and the old track fades out over
   0.5 s. BOSS fights are different (v18, the designer's boss-music rule):
   when a boss appears the current area's track KEEPS playing and
   only its tempo lifts 25% via soundSetFreq (music_boss_start);
   the next area's music starts ONLY after the boss dies, fading in
   from silence over 1.5 s (music_set_area_fadein in
   boss_death_final) while the old track fades out over 0.5 s and
   the tempo eases back to 1.0x. Volumes are v17's as rebalanced in
   v18 (music 127, shoot 60, laser 70, boom 80, pickup 70, alarm
   80, boss tick 50, playerdmg 70) — v13's hotter mix (music 110 +
   loops at -17 dBFS RMS, peaks 0.92) summed the DS mixer over full
   scale and sounded garbled/distorted, and v14's louder loops
   (peaks up to 0.70) did the same in gameplay: its music-only
   title capture was clean but the gameplay capture pins at full
   scale constantly (peaks 1.00 in ~half the seconds, up to 3.83%
   of samples pinned in one second), which is what the designer heard as
   super distorted even in area 1. Sound cannot be heard headless:
   QA verifies by melonDS audio capture (correlation + spectrum +
   clipping counts vs the track, on GAMEPLAY captures, not
   title-only) — see qa/VERIFICATION.md v15. */
#include "music_a1_bin.h"
#include "music_a2_bin.h"
#include "music_a3_bin.h"
#include "music_a4_bin.h"
#include "music_a5_bin.h"
#include "music_a6_bin.h"
#include "music_a7_bin.h"
#define MUSIC_VOL 127   /* v17: 80 -> 127 (+4.0 dB). v16's 16-bit music plays ~0.47x nominal in the emulators' SPU emulation AND too quiet on the designer's phone (NooDS), so the level is raised here plus the loop peak 0.42 -> 0.72 in convert_music_v17.py (+4.7 dB) — ~+9 dB total, back to roughly v12's perceived level. SFX come down a tad in this build to keep mixer headroom. */
#define MUSIC_FADE 30          /* 0.5 s fade-out of the old track */
#define MUSIC_FADEIN 90        /* v18: 1.5 s fade-in of the new track after a boss dies (the designer's boss-music rule) */
#define MUSIC_BOSS_MUL 320     /* 1.25x tempo while a boss fights (256 = 1.0x) */
static const u8 *music_data[NUM_AREAS] = {
    music_a1_bin, music_a2_bin, music_a3_bin, music_a4_bin,
    music_a5_bin, music_a6_bin, music_a7_bin
};
static const u32 music_len[NUM_AREAS] = {
    music_a1_bin_size, music_a2_bin_size, music_a3_bin_size,
    music_a4_bin_size, music_a5_bin_size, music_a6_bin_size,
    music_a7_bin_size
};
#ifdef QA_DEBUG
static const char *music_name[NUM_AREAS] = {
    "music-v2", "area2", "area3", "area4", "area5", "area6", "area7"
};
#endif
/* native sample rate of each track file (v13: all 22050 Hz —
   v12 had areas 2-7 at 16000 Hz, which cut everything above 8 kHz
   and dulled the lead) */
static const u16 music_base_rate[NUM_AREAS] =
    {22050, 22050, 22050, 22050, 22050, 22050, 22050};
static int music_id = -1;       /* current track's sound channel */
static int music_old_id = -1;   /* previous track, fading out */
static int music_area = -1;     /* area of the track actually playing */
static int music_play_area = -1;
static int music_vol = MUSIC_VOL; /* current track volume (0..MUSIC_VOL) — ramps 0 -> MUSIC_VOL during a fade-in */
static int music_old_vol = 0;   /* volume the old track started fading from */
static int music_fade_t = 0;    /* frames left in the old-track fade-out */
static int music_fadein_t = 0;  /* v18: frames left in the new-track fade-in (0 = none) */
static int music_fadein_req = 0;/* v18: fade-in frames requested for the next music_set_area call */
static int music_mul_q = 256;   /* tempo multiplier, fixed point (256 = 1.0x) */
static int music_mul_target = 256;

static void music_apply_rate(void) {
    if (music_id < 0 || music_play_area < 0) return;
    soundSetFreq(music_id,
                 (u16)((u32)music_base_rate[music_play_area] * (u32)music_mul_q / 256));
}

static void music_set_area(int area) {
    if (area == music_area && music_id >= 0) { music_fadein_req = 0; return; }
    /* park the current track as the fading-out one */
    if (music_old_id >= 0) soundKill(music_old_id);
    music_old_id = music_id;
    music_old_vol = music_vol;
    music_fade_t = MUSIC_FADE;
    /* v18: a fade-in request (boss-death switch) starts the new track
       at volume 0 and music_update ramps it up; every other area
       change starts at FULL volume immediately — always audible,
       even if the fade machinery misbehaves on some emulator */
    music_id = soundPlaySample(music_data[area], SoundFormat_16Bit,
                               music_len[area], music_base_rate[area],
                               music_fadein_req ? 0 : MUSIC_VOL, 64, true, 0);
    music_area = area;
    music_play_area = area;
    music_vol = music_fadein_req ? 0 : MUSIC_VOL;
    if (music_fadein_req) { music_fadein_t = music_fadein_req; music_fadein_req = 0; }
    music_apply_rate();
#ifdef QA_DEBUG
    iprintf("\x1b[4;0HMUSIC area %d emb v%d   ", area, music_id);
#endif
}

/* v18: boss-death switch — the new area's track fades IN from silence
   over MUSIC_FADEIN frames instead of starting at full volume (the designer:
   after the boss dies the next area's music should fade in). */
static void music_set_area_fadein(int area) {
    music_fadein_req = MUSIC_FADEIN;
    music_set_area(area);
}

/* boss tempo lift: ramp the rate up at the intro, back down at the kill */
static void music_boss_start(void) { music_mul_target = MUSIC_BOSS_MUL; }
static void music_boss_end(void)   { music_mul_target = 256; }

/* once per frame (main loop, all states): fade out the old track
   (best effort), fade in a new track after a boss death, and run the
   tempo ramp. Volumes are computed from the fade progress (no drift);
   the tempo multiplier steps 2/frame (~0.5 s for the full 25% lift). */
static void music_update(void) {
    if (music_fade_t > 0) {
        music_fade_t--;
        if (music_old_id >= 0) {
            int ov = music_old_vol * music_fade_t / MUSIC_FADE;
            if (ov <= 0 || music_fade_t == 0) {
                soundKill(music_old_id);
                music_old_id = -1;
            } else {
                soundSetVolume(music_old_id, (u8)ov);
            }
        }
        if (music_fade_t == 0 && music_old_id >= 0) {
            soundKill(music_old_id);
            music_old_id = -1;
        }
    }
    /* v18: fade-in ramp — the new track rises 0 -> MUSIC_VOL linearly
       over MUSIC_FADEIN frames. music_vol is the live volume, so the
       QA MUS readout shows the ramp (v0, v42, v85, v127). */
    if (music_fadein_t > 0) {
        music_fadein_t--;
        music_vol = MUSIC_VOL * (MUSIC_FADEIN - music_fadein_t) / MUSIC_FADEIN;
        if (music_fadein_t == 0) music_vol = MUSIC_VOL;
        if (music_id >= 0) soundSetVolume(music_id, (u8)music_vol);
    }
    if (music_mul_q != music_mul_target) {
        if (music_mul_q < music_mul_target) {
            music_mul_q += 2;
            if (music_mul_q > music_mul_target) music_mul_q = music_mul_target;
        } else {
            music_mul_q -= 2;
            if (music_mul_q < music_mul_target) music_mul_q = music_mul_target;
        }
        music_apply_rate();
    }
}

static int terr_top[32], terr_bot[32];
static int scroll_terr, scroll_terr_y, terr_dist, terr_last_tile, cur_area;
static int tunnel_t, tunnel_next;   /* wavy tunnel state (frames) */

static void terrain_fill_col(int col, int top, int bot) {
    u16 e = 1 | (3 << 12);  /* tile 1, palette 3 */
    terr_top[col] = top;
    terr_bot[col] = bot;
    for (int row = 0; row < 24; row++) {
        u16 v = 0;
        if (row < top) v = e;
        if (row >= 24 - bot) v = e;
        BG_MAP_RAM(19)[row * 32 + col] = v;
    }
}

/* v7: heights for one terrain column, factored out so the terrain can
   be rebuilt from scratch (after the upward boss clears it). */
static void terrain_col_heights(int dist, int *top, int *bot) {
    int t = 0, b = 0;
    if (cur_area > 0) {
        if (tunnel_t > 0) {
            int c = 7 + (int)(3 * sin(dist * 0.10));
            t = c;
            b = 24 - c - 8;
        } else {
            int vr = (cur_area <= 2) ? 1 : 2;
            int vary = (dist * 7 + 3) % (2 * vr + 1) - vr;
            t = area_top_base[cur_area] + vary;
            b = area_bot_base[cur_area] - vary;
            if (t < 0) t = 0;
            if (b < 0) b = 0;
            if (t > 5) t = 5;
            if (b > 5) b = 5;
            int cap = (cur_area <= 1) ? 5 : (cur_area == 2 ? 7 : 9);
            if (t + b > cap) b = cap - t;
            if (b < 0) b = 0;
        }
    }
    *top = t;
    *bot = b;
}

static void terrain_next_col(void) {
    int top, bot;
    if (vert_target != 0) {
        /* v13 Fix 2: during the upward flight no new terrain may
           scroll in — the sky must stay empty. Fill blank columns. */
        top = 0; bot = 0;
    } else {
        terrain_col_heights(terr_dist, &top, &bot);
    }
    int col = (((scroll_terr / 8) + 31) % 32 + 32) % 32;
    terrain_fill_col(col, top, bot);
    terr_dist++;
}

/* v9 Fix 1 (clarification): the CEILING opens for the upward boss —
   every column's ceiling height goes to 0 while the floor height is
   kept exactly as-is, so the boss section has open sky above and
   normal ground below. The ship does not move; this is terrain only.
   v13 Fix 2: the floor ALSO leaves — see terrain_clear_floor():
   late-area floors are up to 9 rows (72 px) tall, so the 48 px
   scroll clamp left a sliver visible at the bottom. Once the floor
   band has slid down as far as the clamp allows it is blanked out
   of the map, leaving a full starfield for the fight. */
static void terrain_open_ceiling(void) {
    for (int c = 0; c < 32; c++) terrain_fill_col(c, 0, terr_bot[c]);
}

/* v13 Fix 2: blank the whole terrain (ceiling AND floor) out of the
   map. Called once the floor band has slid down to the clamp during
   the boss-3 upward flight. terr_bot[] is zeroed too, so the ship
   cannot collide with a floor that is no longer there. The next
   area's terrain comes back via terrain_rebuild() at boss death. */
static int terr_cleared = 0;
static void terrain_clear_floor(void) {
    for (int c = 0; c < 32; c++) terrain_fill_col(c, 0, 0);
    terr_cleared = 1;
}

/* v9 Fix 1: rebuild all 32 terrain columns from their deterministic
   heights (used when the upward boss dies, to bring the ceiling back
   for the next area). terr_dist has not advanced during the fight
   (no new columns while the horizontal scroll is stopped), so the
   rebuilt columns match the pre-fight ones. */
static void terrain_rebuild(void) {
    for (int c = 0; c < 32; c++) {
        int top, bot;
        terrain_col_heights(terr_dist + c, &top, &bot);
        terrain_fill_col(c, top, bot);
    }
    terr_dist += 32;
    terr_cleared = 0;   /* v13 Fix 2: the floor is back, terrain is live */
}

/* v18: terrain change WITHOUT the music switch — used at boss intros
   (so the current area's track keeps playing, sped up, through the
   fight) and at boss deaths (the music switch there goes through
   music_set_area_fadein instead). */
static void terrain_set_area_nomusic(int area) {
    cur_area = area;
    dmaCopy(area_tile[area], (u8 *)BG_TILE_RAM(1) + 32, 32);
    dmaCopy(terrain_base_pal_bin, &BG_PALETTE[48], 32);
}

static void terrain_set_area(int area) {
    terrain_set_area_nomusic(area);
    music_set_area(area);   /* v12: music follows the area (embedded track) */
}

static void build_terrain(void) {
    area_tile[0] = terrain_base_bin;
    area_tile[1] = terrain_base_bin;
    area_tile[2] = terrain_planet_bin;
    area_tile[3] = terrain_cave_bin;
    area_tile[4] = terrain_crystal_bin;
    area_tile[5] = terrain_water_bin;
    area_tile[6] = terrain_goo_bin;
    bg_terrain = bgInit(3, BgType_Text4bpp, BgSize_T_256x256, 19, 1);
    terrain_set_area(0);
    scroll_terr = 0;
    scroll_terr_y = 0;
    terr_dist = 0;
    terr_last_tile = 0;
    tunnel_t = 0;
    tunnel_next = 1200;
    for (int c = 0; c < 32; c++) {
        terrain_fill_col(c, 0, 0);
        terr_dist++;
    }
    terr_dist = 32;
    bgSetScroll(bg_terrain, 0, 0);
}

static void terrain_update(void) {
    /* v6 Fix 2: ceiling/floor scroll with the same eased velocity as
       the starfield — slow to a stop, then reverse (up to -160%).
       While scrolling backwards we do NOT generate new columns
       (that would overwrite the map with new heights mid-reverse);
       the already-generated columns simply scroll back. */
    scroll_terr += 2 * scroll_vel / 100;
    /* v9 Fix 1: the terrain band scrolls vertically with the
       starfield during the boss-3 upward flight — but CLAMPED.
       The terrain map is only 256 px tall, so an unbounded offset
       wraps the floor band back through the middle of the screen
       (seen in QA: a full-width band floating at ship height mid-
       fight). Clamped to -48 px the floor slides down and off the
       bottom as you fly up and stays off (negative: scroll Y falls,
       content moves down). Accumulation is gated on vert_target:
       after the fight vert_vel is still decaying toward 0, and
       letting it re-accumulate would undo boss_death_final's
       reset and strand the band mid-screen (also seen in QA).
       With vert_target == 0 the offset eases back to 0.
       v13 Fix 2: the -48 clamp only clears floors up to 48 px
       tall, but late-area floors are up to 9 rows (72 px) — a
       sliver stayed visible at the bottom. So once the band has
       slid to the clamp, the floor is blanked out of the map
       entirely (terrain_clear_floor) and the fight plays over a
       full starfield. After the fight the offset eases back to 0
       GRADUALLY (1 px/frame, ~0.8 s from -48 — no snap, and in
       step with the starfield settling), so the new area's floor
       rises back into view. */
    if (vert_target != 0) {
        scroll_terr_y += 2 * vert_vel / 100;
        if (scroll_terr_y < -48) scroll_terr_y = -48;
        if (scroll_terr_y > 48) scroll_terr_y = 48;
        if (!terr_cleared && vert_vel < 0 && scroll_terr_y <= -48)
            terrain_clear_floor();
    } else if (scroll_terr_y > 0) {
        scroll_terr_y -= 1; if (scroll_terr_y < 0) scroll_terr_y = 0;
    } else if (scroll_terr_y < 0) {
        scroll_terr_y += 1; if (scroll_terr_y > 0) scroll_terr_y = 0;
    }
    bgSetScroll(bg_terrain, scroll_terr, scroll_terr_y);
    int tile = scroll_terr >= 0 ? scroll_terr / 8
                                : -((-scroll_terr + 7) / 8);
    if (tile != terr_last_tile) {
        terr_last_tile = tile;
        if (scroll_vel > 0) terrain_next_col();
    }
    if (scroll_vel <= 0) return;   /* tunnel timer freezes in reverse */
    if (tunnel_t > 0) tunnel_t--;
    else if (cur_area > 0 && --tunnel_next <= 0) {
        tunnel_t = 480;                 /* ~8 s wavy tunnel */
        tunnel_next = 1400 + rand() % 600;
    }
}

static void terrain_at(int sx, int *top_px, int *bot_px) {
    int col = (((scroll_terr / 8) + (sx / 8)) % 32 + 32) % 32;
    *top_px = terr_top[col] * 8;
    *bot_px = terr_bot[col] * 8;
}

/* terrain obstacle sprites: per area a hanging kind and a floor kind.
   obs index into gfx_obs[]; pal is the main sprite palette slot. */
static const int obs_top_gfx[NUM_AREAS]  = {-1, 0, 3, 4, 6, 9, 10};
static const int obs_top_pal[NUM_AREAS]  = {-1, PAL_SHOOTER, PAL_ASTEROID, PAL_ASTEROID,
                                            PAL_ASTEROID, PAL_DIVER, PAL_WEAVER};
static const int obs_bot_gfx[NUM_AREAS]  = {-1, 1, 2, 5, 7, 8, 11};
static const int obs_bot_pal[NUM_AREAS]  = {-1, PAL_SHOOTER, PAL_WEAVER, PAL_ASTEROID,
                                            PAL_ASTEROID, PAL_WEAVER, PAL_WEAVER};

/* ------------------------------------------------------------------ */
/* game state                                                          */
/* ------------------------------------------------------------------ */

typedef enum { ST_TITLE, ST_PLAY, ST_DYING, ST_OVER } GameState;

/* enemy types — 40 total. gfx/pal/hp/score/speed/style/min_wave/areas/shoot */
#define ET_DRIFTER  0
#define ET_WEAVER   1
#define ET_DIVER    2
#define ET_SHOOTER  3
#define ET_ASTEROID 4
#define ET_MINE     5
#define ET_TURRET   6
#define ET_BUG      7
#define ET_ALIEN    8
#define ET_CHARGER  9
#define ET_SPINNER  10
#define ET_ORBITER  11
#define ET_CRYSTAL  12
#define ET_CAPITAL  13
#define ET_DART     14
#define ET_ROGUE    15
#define ET_UFO      16
#define ET_METEOR2  17
#define ET_DRONE    18
#define ET_SENTRY   19
#define ET_WASP     20
#define ET_BEETLE   21
#define ET_BAT      22
#define ET_FISH     23
#define ET_JELLY    24
#define ET_SHARD    25
#define ET_BLOB     26
#define ET_CRAWLER  27
#define ET_EEL      28
#define ET_PRISM    29
#define ET_SPORE    30
#define ET_HUNTER   31
/* v7: 8 new web-sourced types (40 total) — worms, alien creature,
   jellyfish, creeper eye, robot, tentacled orb */
#define ET_WORMG    32
#define ET_WORMR    33
#define ET_WORMB    34
#define ET_ALIEN2   35
#define ET_JELLY2   36
#define ET_EYE      37
#define ET_ROBOT    38
#define ET_TENT     39
#define NUM_ENEMY_TYPES 40

/* movement styles */
#define ST_DRIFT 0
#define ST_SINE  1
#define ST_DIVE  2
#define ST_HOVER 3
#define ST_TUMBLE 4
#define ST_ZIGZAG 5
#define ST_DASH  6
#define ST_ORBIT 7
#define ST_SNAKE 8
#define ST_SLOW  9

/* v5 Fix 4: enemy animation styles — 0 none, 1 spin (flip h/v),
   2 wings (flip v flaps), 3 pulse (flip + sparkle), 4 wobble,
   5 alt-frame (swap to gfx_enemy_alt[] every 15 frames — v7). */
static const int enemy_anim[NUM_ENEMY_TYPES] = {
    4, 4, 0, 0, 1, 3, 0, 2, 3, 0, 1, 1, 1, 3,
    0, 4, 3, 1, 0, 0, 2, 2, 2, 4, 3, 1, 3, 2,
    4, 1, 3, 0,
    4, 4, 4, 5, 5, 3, 0, 4   /* v7: worms wobble, alien/jelly alt-frame */
};

/* area bits */
#define A_OPEN    1
#define A_BASE    2
#define A_PLANET  4
#define A_CAVE    8
#define A_CRYSTAL 16
#define A_WATER   32
#define A_GOO     64

typedef struct {
    int pal, hp, score, speed, style, min_wave, areas, shoot;
} EnemyDef;

static const EnemyDef enemy_def[NUM_ENEMY_TYPES] = {
/* pal, hp, score, speed, style, min_wave, areas, shoot(ms) */
 {PAL_DRIFTER,  1, 100, 330, ST_DRIFT,  1, 0x7F, 0},   /* drifter */
 {PAL_WEAVER,   1, 150, 230, ST_SINE,   1, 0x7F, 0},   /* weaver */
 {PAL_DIVER,    1, 200, 260, ST_DIVE,   1, 0x7F, 0},   /* diver */
 {PAL_SHOOTER,  2, 250, 205, ST_HOVER,  2, 0x03, 110}, /* shooter */
 {PAL_ASTEROID, 4, 300, 180, ST_TUMBLE, 3, 0x0F, 0},   /* asteroid */
 {PAL_MINE,     3, 250, 128, ST_DRIFT,  3, 0x72, 0},   /* mine */
 {PAL_TURRET,   2, 300, 256, ST_HOVER,  4, 0x03, 85},  /* turret */
 {PAL_WEAVER,   2, 150, 280, ST_ZIGZAG, 2, 0x4C, 0},   /* bug */
 {PAL_DIVER,    3, 300, 200, ST_SLOW,   3, 0x50, 140}, /* alien */
 {PAL_DRIFTER,  2, 200, 200, ST_DASH,   2, 0x07, 0},   /* charger */
 {PAL_SHOOTER,  3, 250, 240, ST_TUMBLE, 3, 0x18, 0},   /* spinner */
 {PAL_SHOOTER,  4, 350, 180, ST_ORBIT,  4, 0x50, 0},   /* orbiter */
 {PAL_ASTEROID, 5, 400, 160, ST_TUMBLE, 3, 0x18, 0},   /* crystal */
 {PAL_SHOOTER,  8, 600, 120, ST_HOVER,  6, 0x03, 120}, /* capital */
 {PAL_DRIFTER,  1, 100, 420, ST_DRIFT,  1, A_OPEN, 0}, /* dart */
 {PAL_DRIFTER,  2, 200, 300, ST_SNAKE,  2, 0x05, 0},   /* rogue */
 {PAL_DIVER,    3, 300, 150, ST_HOVER,  2, A_OPEN, 130},/* ufo */
 {PAL_ASTEROID, 3, 250, 200, ST_TUMBLE, 1, A_OPEN, 0}, /* meteor2 */
 {PAL_SHOOTER,  2, 200, 260, ST_DRIFT,  2, A_BASE, 0}, /* drone */
 {PAL_SHOOTER,  3, 300, 140, ST_HOVER,  3, A_BASE, 100},/* sentry */
 {PAL_WEAVER,   1, 150, 380, ST_ZIGZAG, 2, A_PLANET, 0},/* wasp */
 {PAL_WEAVER,   3, 250, 170, ST_TUMBLE, 2, A_PLANET, 0},/* beetle */
 {PAL_WEAVER,   1, 150, 340, ST_SNAKE,  3, A_CAVE, 0}, /* bat */
 {PAL_DIVER,    2, 200, 300, ST_SINE,   2, A_WATER, 0},/* fish */
 {PAL_DIVER,    3, 250, 140, ST_SINE,   3, A_WATER, 0},/* jelly */
 {PAL_ASTEROID, 2, 250, 220, ST_TUMBLE, 3, A_CRYSTAL, 0},/* shard */
 {PAL_ASTEROID, 2, 200, 160, ST_DRIFT,  2, A_GOO, 0},  /* blob */
 {PAL_DRIFTER,  2, 200, 200, ST_ZIGZAG, 3, A_CAVE, 0}, /* crawler */
 {PAL_DIVER,    3, 300, 320, ST_SNAKE,  3, A_WATER, 0},/* eel */
 {PAL_ASTEROID, 5, 450, 140, ST_TUMBLE, 4, A_CRYSTAL, 0},/* prism */
 {PAL_ASTEROID, 1, 100, 240, ST_DRIFT,  1, 0x48, 0},   /* spore */
 {PAL_SHOOTER,  4, 350, 240, ST_DASH,   4, A_BASE, 0}, /* hunter */
 /* v7 web-sourced enemies — area-unique, totally different sprites */
 {PAL_WEAVER,   2, 200, 220, ST_SNAKE,  2, A_CAVE | A_GOO, 0},   /* worm green */
 {PAL_DRIFTER,  2, 200, 240, ST_ZIGZAG, 2, A_PLANET | A_CAVE, 0},/* worm red */
 {PAL_DIVER,    3, 250, 200, ST_SINE,   3, A_WATER, 0},           /* worm blue */
 {PAL_WEAVER,   2, 200, 260, ST_DRIFT,  1, A_OPEN | A_BASE, 0},   /* alien creature */
 {PAL_JELLY,    3, 300, 150, ST_SINE,   2, A_WATER | A_GOO, 0},   /* jellyfish */
 {PAL_DIVER,    4, 400, 140, ST_HOVER,  3, A_CRYSTAL | A_CAVE, 130},/* creeper eye */
 {PAL_SHOOTER,  4, 400, 160, ST_HOVER,  3, A_BASE, 110},          /* robot */
 {PAL_DIVER,    3, 300, 200, ST_SNAKE,  4, A_GOO | A_WATER, 0},   /* tentacled orb */
};

#define MAX_ENEMIES 20
#define MAX_PBULLETS 20
#define MAX_EBULLETS 10
#define MAX_BOOMS 10       /* v8 Fix 1: room for multi-burst explosions */
#define MAX_PICKUPS 6
#define MAX_PARTICLES 24   /* v8 Fix 1: per-type debris needs more sparks */
#define MAX_OBST 6
#define MAX_TOBST 8

#define PANEL_Y 160   /* weapon panel starts at sub-screen y=160 */

typedef enum { W_SINGLE, W_SPREAD, W_LASER, W_MISSILE } Weapon;

typedef struct {
    int active, type;
    int x, y;              /* top-left, fixed point *256 */
    int base_x, base_y, t, hp, flash;
    int shoot_t, flip_t;
    int flip_h, flip_v;
    int dying, die_t, drift;   /* v4 Fix 3: death drift */
} Enemy;

typedef struct {
    int active, x, y;      /* px */
    int vx, vy;            /* fixed *256 per frame */
    int dmg, pierce, hits, kind, life;  /* kind: 0 bullet, 1 missile, 2 beam */
} PBullet;
typedef struct { int active, x, y, vx, vy; } EBullet; /* fixed *256 */
/* v8 Fix 1: booms and particles carry a velocity (fixed *256/frame)
   and a palette slot, so one explosion can be a spreading, coloured
   burst instead of a static flame. */
typedef struct { int active, x, y, frame, t, vx, vy, pal; } Boom;
typedef struct { int active, x, y, type, t; } Pickup;   /* type: 1 spread, 2 laser, 3 missile, 4 rate */
typedef struct { int active, x, y, t, vx, vy, pal, life; } Particle;
typedef struct { int active, x, y, hp, crystal, t; } Obst;
typedef struct { int active, x, y, h, top, gfx, pal; } TObst; /* terrain obstacle sprite */
typedef struct { int active, form, x, y, hp, maxhp, t, shoot_t, fan_t, minion_t, diff;
                 /* v7: per-boss gimmick — mode 1 reverse, 2 halt, 3 upward,
                    4 shield+charge; shield/charge state for mode 4 */
                 int mode, shield, shield_t, charge_t, charging, returning;
                 /* v8 Fix 2: boss death sequence — dying counts the
                    staggered explosions; die_t frames until the final
                    blast clears the boss. */
                 int dying, die_t; } Boss;

/* v6 Fix 2: reverse SCROLL. reverse_mode = the world scroll is
   currently reversed (target -160). The ship NEVER moves/flips;
   dir_fly stays +1 and all gameplay stays left-to-right.
   v7: boss_mode is the CURRENT boss's gimmick (0 = no boss);
   intro_mode is the gimmick of the boss whose intro is running. */
static int reverse_mode = 0;
static int boss_intro = 0;
static int intro_t = 0;
static int intro_mode = 0;
static int boss_num = 0;
static int boss_flip = 0;
/* v7 Req 2: death sequence — ST_DYING plays the ship explosion
   (~1.5 s) before the GAME OVER screen; over_armed makes the
   game-over screen wait for a FRESH press (finger released first). */
static int die_t = 0;
static int ship_dead = 0;
static int over_armed = 0;
static int best_score = 0;

static GameState state = ST_TITLE;
static int score, lives, wave, kills, frame;
#ifdef QA_DEBUG
/* v16: drop-rate counters for QA (read out on sub row 6, debug only). */
static int qa_drop_rolls, qa_drop_weap, qa_drop_1up, qa_drop_float;
#endif
static int ship_x, ship_y;      /* top-left px, 32x32 sprite */
static int invuln, fire_t, spawn_t, pickup_t, obst_t, tobst_t;
static int title_t, rate_lvl;
static Weapon weapon = W_SINGLE;
static int owned[4];            /* owned[W_SINGLE] always 1 */
static Boss boss;

/* touch state — RELATIVE drag: the ship never jumps to the touch
   point. On drag start we record the ship position and the touch
   position; the ship follows by the touch DELTA only. A tap moves
   nothing. NOTE: on the release frame touchRead() returns 0,0, so
   press/release is tracked from keysHeld() and the positions
   recorded while the pen was down. */
static int touch_down;
static int drag_active, drag_is_panel, drag_moved;
static int drag_down_x, drag_down_y;
static int drag_ship_x, drag_ship_y;
static int last_touch_x, last_touch_y;

static Enemy enemies[MAX_ENEMIES];
static PBullet pbullets[MAX_PBULLETS];
static EBullet ebullets[MAX_EBULLETS];
static Boom booms[MAX_BOOMS];
static Pickup pickups[MAX_PICKUPS];
static Particle particles[MAX_PARTICLES];
static Obst obsts[MAX_OBST];
static TObst tobsts[MAX_TOBST];

static void sfx_shoot(void) {
    soundPlaySample(shoot_raw_bin, SoundFormat_8Bit, shoot_raw_bin_size,
                    11025, 60, 64, false, 0);  /* v18: 85 -> 60, SFX down a further step so the music sits on top */
}
static void sfx_laser(void) {
    soundPlaySample(laser_raw_bin, SoundFormat_8Bit, laser_raw_bin_size,
                    11025, 70, 64, false, 0);  /* v18: 100 -> 70 */
}
static void sfx_alarm(void) {
    soundPlaySample(alarm_raw_bin, SoundFormat_8Bit, alarm_raw_bin_size,
                    11025, 80, 64, false, 0);  /* v18: 110 -> 80, still the loudest SFX */
}
static void sfx_pickup(void) {
    soundPlaySample(pickup_raw_bin, SoundFormat_8Bit, pickup_raw_bin_size,
                    11025, 70, 64, false, 0);  /* v18: 100 -> 70 */
}
static void sfx_boom(void) {
    soundPlaySample(boom_raw_bin, SoundFormat_8Bit, boom_raw_bin_size,
                    11025, 80, 64, false, 0);  /* v18: 110 -> 80, the main mixer clipper, cut most */
}
/* v17: sfx_hit (old player-hit sound, 160->70 Hz square + noise) is
   retired — player_hit now plays the distinct sfx_playerdmg buzz. */
static void sfx_bosshit(void) {
    soundPlaySample(bosshit_raw_bin, SoundFormat_8Bit, bosshit_raw_bin_size,
                    11025, 50, 64, false, 0);  /* v18: 70 -> 50 */
}
/* v17: PLAYER-damage sound — low descending buzz (280->70 Hz, 0.28 s,
   sample peak 0.62), distinct from the boss tick (330->200 Hz sine,
   0.07 s) and the old hit sound. Plays in player_hit() on every hit
   including the life-losing one. At vol 100 its contribution is 0.49;
   music 0.72 + playerdmg 0.49 = 1.21 worst case if peaks align — the
   peak sits in the first 30 ms noise burst, so pinning is measured,
   not assumed (see qa/VERIFICATION.md). */
static void sfx_playerdmg(void) {
    soundPlaySample(playerdmg_raw_bin, SoundFormat_8Bit, playerdmg_raw_bin_size,
                    11025, 70, 64, false, 0);  /* v18: 100 -> 70 */
}

static void spawn_boom_pal(int cx, int cy, int pal, int vx, int vy) {
    for (int i = 0; i < MAX_BOOMS; i++) {
        if (!booms[i].active) {
            booms[i].active = 1;
            booms[i].x = cx - 16;
            booms[i].y = cy - 16;
            booms[i].frame = 0;
            booms[i].t = 0;
            booms[i].vx = vx;
            booms[i].vy = vy;
            booms[i].pal = pal;
            return;
        }
    }
}

static void spawn_boom(int cx, int cy) {
    spawn_boom_pal(cx, cy, PAL_BOOM, 0, 0);
}

static void spawn_particle2(int x, int y, int vx, int vy, int pal, int life) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) {
            particles[i].active = 1;
            particles[i].x = x;
            particles[i].y = y;
            particles[i].t = 0;
            particles[i].vx = vx;
            particles[i].vy = vy;
            particles[i].pal = pal;
            particles[i].life = life;
            return;
        }
    }
}

static void spawn_particle(int x, int y) {
    spawn_particle2(x, y, -256, 0, PAL_BOOM, 12);
}

#ifdef QA_DEBUG
static int qa_sparks = 0;   /* v17 QA: hit sparks spawned (readout row 6) */
#endif
/* v17: HIT SPARK on multi-HP enemies — a tiny 3-particle flash at the
   impact point on every surviving hit (bosses included), so each hit
   reads visually. 5 frames, a handful of pixels, PAL_BOOM's bright
   yellow-white (index 1) — no big explosion, the full explosion
   already plays on death. Pool is only 24 particles and
   spawn_particle2 silently drops when full, so rapid fire can never
   overflow it. */
static void spawn_hitspark(int x, int y) {
    spawn_particle2(x, y, -128, -64, PAL_BOOM, 5);
    spawn_particle2(x, y, 128, -64, PAL_BOOM, 5);
    spawn_particle2(x, y, 0, 96, PAL_BOOM, 5);
#ifdef QA_DEBUG
    qa_sparks++;
#endif
}

/* v8 Fix 1: explosion kinds — one palette + behaviour per enemy
   family, so deaths read as different explosions, not a ship swap.
   BK_FIRE  small ships: orange fireball + sparks (boom palette).
   BK_GOO   bugs / creatures / jellyfish: green-purple goo burst
            (jelly palette — purple/pink).
   BK_SHARD crystals: shattering cyan-white shard burst (laserbeam
            palette — bright cyan/white).
   BK_METAL robots / metal / mines: grey sparks + smoke (shooter
            palette — greys). */
#define BK_FIRE  0
#define BK_GOO   1
#define BK_SHARD 2
#define BK_METAL 3
static const int boom_pal[4] = { PAL_BOOM, PAL_JELLY, PAL_LASERBEAM, PAL_SHOOTER };
/* one kind per enemy type (indices must match the ET_* enum order in
   enemy_def): drifter/weaver/diver/shooter FIRE, asteroid/crystal/
   meteor2/shard/prism/spore SHARD, mine/turret/orbiter/drone/sentry/
   capital/hunter/robot METAL, everything organic GOO. */
static const int enemy_boom_kind[NUM_ENEMY_TYPES] = {
    BK_FIRE, BK_FIRE, BK_FIRE, BK_FIRE, BK_SHARD, BK_METAL, BK_METAL,
    BK_GOO, BK_GOO, BK_FIRE, BK_FIRE, BK_METAL, BK_SHARD, BK_METAL,
    BK_FIRE, BK_FIRE, BK_METAL, BK_SHARD, BK_METAL, BK_METAL,
    BK_GOO, BK_GOO, BK_GOO, BK_GOO, BK_GOO, BK_SHARD, BK_GOO, BK_GOO,
    BK_GOO, BK_SHARD, BK_GOO, BK_FIRE, BK_GOO,
    BK_GOO, BK_GOO, BK_GOO, BK_GOO, BK_GOO, BK_METAL, BK_GOO,
};

/* v8 Fix 1: the actual pixelated explosion — 2-3 fireball frames
   bursting out from the centre at different offsets and velocities,
   plus a spray of coloured sparks. big = tough enemies get a larger,
   multi-stage burst. v18: this now fires at the END of the visible
   death drift (kill_enemy_final), not at the killing hit — the dying
   ship was flashing and trailing embers up to this point, so the
   burst reads as the wreck finally going up, not as a ship swapping. */
static void spawn_explosion(int cx, int cy, int kind, int big) {
    int pal = boom_pal[kind];
    spawn_boom_pal(cx, cy, pal, -128, 0);
    spawn_boom_pal(cx + 10, cy - 8, pal, 96, -96);
    spawn_boom_pal(cx - 12, cy + 6, pal, 96, 96);
    if (big)
        spawn_boom_pal(cx - 4, cy - 12, pal, 0, -128);
    int n = big ? 10 : 8;
    for (int k = 0; k < n; k++) {
        int vx = (rand() % 5 - 2) * 256;
        int vy = (rand() % 5 - 2) * 256;
        spawn_particle2(cx + (rand() % 24) - 12, cy + (rand() % 24) - 12,
                        vx, vy, pal, 14 + rand() % 10);
    }
}

static void spawn_pickup(int x, int y, int type) {
    for (int i = 0; i < MAX_PICKUPS; i++) {
        if (!pickups[i].active) {
            pickups[i].active = 1;
            pickups[i].x = x;
            pickups[i].y = y;
            pickups[i].type = type;
            pickups[i].t = 0;
            return;
        }
    }
}

/* v6 Fix 1: when a boss fight starts, the player fights the boss
   ALONE — clear every regular enemy and enemy bullet on screen. */
static void clear_regulars(void) {
    memset(enemies, 0, sizeof(enemies));
    memset(ebullets, 0, sizeof(ebullets));
}

static void spawn_enemy(int type) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            Enemy *e = &enemies[i];
            e->active = 1;
            e->type = type;
            e->x = 264 * 256;  /* v6: always enter from the right */
            e->y = (16 + rand() % 140) * 256;
            e->base_x = e->x;
            e->base_y = e->y;
            e->t = 0;
            /* v9 Fix 2: enemies after boss 2 (wave > 6) have more HP —
               in v8 the player one-shot them and they were invisible.
               v19: the v9 tier formula grew WITHOUT BOUND (tier =
               (wave-7)/3+1, hp = base*(1+tier)+tier*2: a 1-HP type was
               4 HP at wave 7 and 10 HP at wave 13, an 8-HP capital 18
               at wave 7 and 38 at wave 13) and the designer found wave 8
               nearly impossible to kill through. Now waves 7+ just
               add a flat +2 and NEVER grow: a 1-HP type is 3 HP and
               an 8-HP capital is 10 HP at every wave from 7 on, so
               ordinary enemies always die in a few hits. Waves 1-6
               are EXACTLY unchanged (boss 1 / level 1 approved). */
            {
                int hp = enemy_def[type].hp;
                if (wave > 6) {
                    hp = hp + 2;
                }
                e->hp = hp;
            }
            e->flash = 0;
            e->shoot_t = 60 + rand() % 60;
            e->flip_t = 0;
            e->flip_h = e->flip_v = 0;
            e->dying = 0;
            e->die_t = 0;
            e->drift = 0;
            spawn_particle(e->x / 256 + 16, e->y / 256 + 16);  /* v5: spawn sparkle */
            return;
        }
    }
}

static void spawn_obst(void) {
    for (int i = 0; i < MAX_OBST; i++) {
        if (!obsts[i].active) {
            obsts[i].active = 1;
            obsts[i].x = 264;
            obsts[i].y = 24 + rand() % 130;
            obsts[i].hp = 3;
            obsts[i].crystal = (rand() % 2);
            obsts[i].t = 0;
            return;
        }
    }
}

static void spawn_tobst(void) {
    if (cur_area <= 0 || tunnel_t > 0) return;
    for (int i = 0; i < MAX_TOBST; i++) {
        if (!tobsts[i].active) {
            int top = (rand() % 2);
            int gfx = top ? obs_top_gfx[cur_area] : obs_bot_gfx[cur_area];
            int pal = top ? obs_top_pal[cur_area] : obs_bot_pal[cur_area];
            if (gfx < 0) return;
            tobsts[i].active = 1;
            tobsts[i].x = 264;
            tobsts[i].h = (rand() % 2) ? 32 : 64;
            tobsts[i].top = top;
            tobsts[i].y = top ? 0 : 192 - tobsts[i].h;
            tobsts[i].gfx = gfx;
            tobsts[i].pal = pal;
            return;
        }
    }
}

static void spawn_boss(void) {
    /* v6 Fix 1: boss alone — no regular enemies on screen. */
    clear_regulars();
    boss.active = 1;
    boss_num++;
    boss.diff = boss_num;   /* 1 = first boss (easy), 2+ ramp up */
    /* v7: gimmick cycles 1 reverse, 2 halt, 3 upward, 4 shield+charge.
       Boss 1 is PERFECT as-is — mode 1, reverse scroll, unchanged. */
    boss.mode = ((boss_num - 1) % 4) + 1;
    boss.form = boss.mode - 1;   /* gfx_boss[0..3]: one sprite per boss */
    boss.shield = 0;
    boss.shield_t = 150;
    boss.charge_t = 300;
    boss.charging = 0;
    boss.returning = 0;
    /* v9 Fix 1: EVERY boss enters from the right and parks on the
       right (x=196) — the upward boss no longer enters from the top
       or parks above. */
    boss.x = 264;   /* v6: boss always enters from the right */
    boss.y = 80;
#ifdef QA_DEBUG
    boss.maxhp = 40;  /* QA-only: tough enough that a scripted run can
                         * sample the whole fight (shield cycles, charge
                         * dashes) instead of one-shotting the boss;
                         * the shipped ROM uses the real HP below. */
#else
    /* v5 Fix 1 (v14: boss 1 HP 25 -> 70, ~2.8x — the designer: it died too
       fast to enjoy the reverse flight; pattern/bullets UNCHANGED,
       only HP); later bosses ramp. */
    boss.maxhp = (boss.diff == 1) ? 70 : 40 + wave * 5;
#endif
    boss.hp = boss.maxhp;
    boss.t = 0;
    boss.shoot_t = 90;
    boss.fan_t = 200;
    boss.minion_t = 240;
}

static void fire_pbullet(int x, int y, int vx, int vy, int dmg, int pierce, int kind) {
    for (int i = 0; i < MAX_PBULLETS; i++) {
        if (!pbullets[i].active) {
            pbullets[i].active = 1;
            pbullets[i].x = x;
            pbullets[i].y = y;
            pbullets[i].vx = vx;
            pbullets[i].vy = vy;
            pbullets[i].dmg = dmg;
            pbullets[i].pierce = pierce;
            pbullets[i].hits = 0;
            pbullets[i].kind = kind;
            pbullets[i].life = 0;
            return;
        }
    }
}

/* v4 Fix 7: slower base fire; rate level 0-3 cuts the interval. */
static const int weapon_base_interval[4] = {22, 26, 40, 34};

static void fire_weapon(void) {
    /* v6: shots ALWAYS fly right — the ship never reverses.
       v9 Fix 1: no upward-mode shots any more; the ship faces right
       and fires right in EVERY boss fight, including boss 3. */
    int nx = ship_x + 24, ny = ship_y + 12;
    int mult = 100 - 12 * rate_lvl;   /* rate 3 -> 64% of base interval */
    if (weapon == W_SINGLE) {
        fire_pbullet(nx, ny, 6 * 256, 0, 1, 0, 0);
        fire_t = weapon_base_interval[W_SINGLE] * mult / 100;
        sfx_shoot();
    } else if (weapon == W_SPREAD) {
        fire_pbullet(nx, ny - 6, 6 * 256, -1 * 256, 1, 0, 0);
        fire_pbullet(nx, ny,     6 * 256,  0,       1, 0, 0);
        fire_pbullet(nx, ny + 6, 6 * 256,  1 * 256, 1, 0, 0);
        fire_t = weapon_base_interval[W_SPREAD] * mult / 100;
        sfx_shoot();
    } else if (weapon == W_LASER) {
        fire_pbullet(ship_x + 24, ny, 0, 0, 3, 1, 2);
        fire_t = weapon_base_interval[W_LASER] * mult / 100;
        sfx_laser();
    } else {
        fire_pbullet(nx, ny - 8, 5 * 256, -1 * 256, 2, 0, 1);
        fire_pbullet(nx, ny + 8, 5 * 256,  1 * 256, 2, 0, 1);
        fire_t = weapon_base_interval[W_MISSILE] * mult / 100;
        sfx_shoot();
    }
}

static void fire_ebullet(int ex, int ey, int vx, int vy) {
    for (int i = 0; i < MAX_EBULLETS; i++) {
        if (!ebullets[i].active) {
            ebullets[i].active = 1;
            ebullets[i].x = ex * 256;
            ebullets[i].y = ey * 256;
            ebullets[i].vx = vx;
            ebullets[i].vy = vy;
            return;
        }
    }
}

static void clear_field(void) {
    memset(enemies, 0, sizeof(enemies));
    memset(pbullets, 0, sizeof(pbullets));
    memset(ebullets, 0, sizeof(ebullets));
    memset(booms, 0, sizeof(booms));
    memset(pickups, 0, sizeof(pickups));
    memset(particles, 0, sizeof(particles));
    memset(obsts, 0, sizeof(obsts));
    memset(tobsts, 0, sizeof(tobsts));
    memset(&boss, 0, sizeof(boss));
}

static const char *weapon_name(Weapon w) {
    return (w == W_SINGLE) ? "SINGLE" : (w == W_SPREAD) ? "SPREAD" :
           (w == W_LASER) ? "LASER" : "MISSILE";
}

/* ------------------------------------------------------------------ */
/* v13 Change 4: NO MORE LOG TEXT ON THE TOUCHPAD.                    */
/*                                                                    */
/* The bottom screen is no longer a console: it is a finished        */
/* tile-text UI on a dedicated sub BG (bgInitSub, map base 8,        */
/* tile base 0), with a pixel font rendered by                        */
/* tools/convert_font.py (font1: 8x8, font2: 16x16 per glyph) and    */
/* a navy backdrop. NOTHING is ever printed with iprintf in the      */
/* release build — every iprintf/consoleClear in this file is        */
/* behind #ifdef QA_DEBUG (grep it), so the shipped ROM can never    */
/* show log output. The bottom screen shows ONLY:                    */
/*   - AREA <name> and WEAPON <name> [+N] (rows 1-2, tile text)      */
/*   - boss cues as a transient scale-2 banner (rows 4-5)            */
/*   - title hints / game-over score lines (tile text)               */
/* The weapon panel buttons (sprites, y=160) are drawn by            */
/* draw_play/draw_title as before.                                    */
/* ------------------------------------------------------------------ */

#include "font1_bin.h"
#include "font2_bin.h"
#include "title_bin.h"
#include "title_map_bin.h"
#include "title_pal_bin.h"
#include "gameover_bin.h"
#include "gameover_map_bin.h"

/* title / game-over art: tiles in main BG tile base 0 (the far
   starfield layer reuses the same tile RAM; starfield tiles are
   0..7, title art starts at 16, game-over art at 239 — see
   tools/convert_title.py, which prints both bases). */
#define TITLE_TILE_BASE 16
#define GAMEOVER_TILE_BASE 239

/* sub font tiles: font1 glyphs at 0..45, font2 glyphs at 64..247 */
#define SUB_FONT1_TILE 0
#define SUB_FONT2_TILE 64
#define SUB_MAP_BASE 8

/* sub text palettes (BG_PALETTE_SUB): 1 cyan, 2 yellow, 3 red,
   4 green, 5 white, 6 orange — index 1 of each palette is the ink,
   index 0 is the navy backdrop. */
#define SUB_PAL_CYAN 1
#define SUB_PAL_YELLOW 2
#define SUB_PAL_RED 3
#define SUB_PAL_GREEN 4
#define SUB_PAL_WHITE 5
#define SUB_PAL_ORANGE 6

/* font charset (tools/convert_font.py): space, A-Z, 0-9, then
   + - : . ! / ' ( ) — glyph 0 is blank. */
static int font_glyph(char c) {
    if (c == ' ') return 0;
    if (c >= 'A' && c <= 'Z') return c - 'A' + 1;
    if (c >= '0' && c <= '9') return 27 + (c - '0');
    switch (c) {
        case '+': return 37; case '-': return 38; case ':': return 39;
        case '.': return 40; case '!': return 41; case '/': return 42;
        case '\'': return 43; case '(': return 44; case ')': return 45;
        default: return 0;
    }
}

static void sub_text(int col, int row, const char *s, int pal) {
    u16 *map = BG_MAP_RAM_SUB(SUB_MAP_BASE);
    for (int i = 0; s[i]; i++) {
        if (col + i >= 32) break;
        map[row * 32 + col + i] =
            (SUB_FONT1_TILE + font_glyph(s[i])) | (pal << 12);
    }
}

static void sub_text2(int col, int row, const char *s, int pal) {
    /* scale-2 text: each char is 2x2 tiles (16 px tall, 2 rows) */
    u16 *map = BG_MAP_RAM_SUB(SUB_MAP_BASE);
    for (int i = 0; s[i]; i++) {
        int g = font_glyph(s[i]);
        int t = SUB_FONT2_TILE + g * 4;
        int c0 = col + 2 * i;
        if (c0 + 1 >= 32) break;
        map[row * 32 + c0] = t | (pal << 12);
        map[row * 32 + c0 + 1] = (t + 1) | (pal << 12);
        map[(row + 1) * 32 + c0] = (t + 2) | (pal << 12);
        map[(row + 1) * 32 + c0 + 1] = (t + 3) | (pal << 12);
    }
}

static void sub_text2_center(int row, const char *s, int pal) {
    int len = 0;
    while (s[len]) len++;
    int col = (32 - 2 * len) / 2;
    if (col < 0) col = 0;
    sub_text2(col, row, s, pal);
}

static void sub_text_center(int row, const char *s, int pal) {
    int len = 0;
    while (s[len]) len++;
    int col = (32 - len) / 2;
    if (col < 0) col = 0;
    sub_text(col, row, s, pal);
}

static void sub_clear_rows(int r0, int r1) {
    u16 *map = BG_MAP_RAM_SUB(SUB_MAP_BASE);
    for (int r = r0; r <= r1; r++)
        for (int c = 0; c < 32; c++) map[r * 32 + c] = 0;
}

/* boss cue banner (sub rows 4-5, scale 2): set_cue writes it now;
   cue_t counts down in update_play, then the rows are cleared. */
static int cue_t = 0;
static void set_cue(const char *s, int pal, int frames) {
    sub_clear_rows(4, 5);
    sub_text2_center(4, s, pal);
    cue_t = frames;
}
static void clear_cue(void) {
    sub_clear_rows(4, 5);
    cue_t = 0;
}

/* v4 Fix 2: the bottom screen shows ONLY the area name and the
   selected weapon. Nothing else is ever printed during play.
   Fire-rate level (v4 Fix 7) shows as +N on the weapon line. */
static void draw_sub_info(void) {
    sub_clear_rows(0, 3);
    sub_text(1, 1, "AREA", SUB_PAL_CYAN);
    sub_text(6, 1, area_name[cur_area], SUB_PAL_WHITE);
    sub_text(1, 2, "WEAPON", SUB_PAL_CYAN);
    sub_text(8, 2, weapon_name(weapon), SUB_PAL_YELLOW);
    if (rate_lvl > 0) {
        char b[4];
        b[0] = '+'; b[1] = '0' + rate_lvl; b[2] = 0;
        int wlen = 0;
        while (weapon_name(weapon)[wlen]) wlen++;
        sub_text(8 + wlen + 1, 2, b, SUB_PAL_GREEN);
    }
}

static void start_game(void) {
    clear_field();
    score = 0;
    lives = 3;
    wave = 1;
    kills = 0;
    fire_t = 0;
    spawn_t = 40;
    pickup_t = 900;
    obst_t = 500;
    tobst_t = 200;
    invuln = 0;
    rate_lvl = 0;
    weapon = W_SINGLE;
    dir_fly = 1; reverse_mode = 0; boss_intro = 0; intro_t = 0; intro_mode = 0; boss_num = 0;
    vert_vel = 0; vert_target = 0;   /* v9 Fix 1: no vertical scroll at start */
    scroll_vel = 100; scroll_target = 100;
    ship_dead = 0; die_t = 0; over_armed = 0;
    owned[0] = 1; owned[1] = 0; owned[2] = 0; owned[3] = 0;
    touch_down = 0;
    drag_active = drag_is_panel = drag_moved = 0;
    ship_x = 16;
    ship_y = 80;
    state = ST_PLAY;
    terrain_set_area(0);
    scroll_terr = 0;
    scroll_terr_y = 0;
    terr_dist = 32;
    terr_last_tile = 0;
    tunnel_t = 0;
    tunnel_next = 1200;
    for (int c = 0; c < 32; c++) terrain_fill_col(c, 0, 0);
    /* v13 Change 3: restore the far starfield map (the title art
       replaced it) and the finished sub UI. */
    dmaCopy(far_map_save, BG_MAP_RAM(18), 2048);
    sub_clear_rows(0, 23);
    clear_cue();
    draw_sub_info();
}

static void show_title(void) {
    state = ST_TITLE;
    title_t = 0;
    clear_field();
    terrain_set_area(0);
    scroll_terr = 0;
    terr_dist = 32;
    terr_last_tile = 0;
    tunnel_t = 0;
    for (int c = 0; c < 32; c++) terrain_fill_col(c, 0, 0);
    for (int i = 0; i < 5; i++) {
        Enemy *e = &enemies[i];
        e->active = 1;
        e->type = i % 3;
        e->x = (40 + i * 55) * 256;
        e->y = (20 + i * 30) * 256;
        e->base_x = e->x;
        e->base_y = e->y;
        e->t = i * 30;
        e->hp = 99;
        e->shoot_t = 9999;
    }
    /* v13 Change 3: arcade title art on the far starfield layer
       (main screen, over the live scrolling starfield). The map is
       restored to the starfield by start_game(). */
    dmaCopy(title_map_bin, BG_MAP_RAM(18), 2048);
    bgSetScroll(bg_far, 0, 0);
    /* v13 Change 4: control hints on the sub screen in tile text */
    sub_clear_rows(0, 23);
    sub_text_center(8, "DRAG ANYWHERE TO FLY", SUB_PAL_CYAN);
    sub_text_center(10, "TAP A WEAPON TO SWITCH", SUB_PAL_YELLOW);
    sub_text_center(12, "START RETURNS TO TITLE", SUB_PAL_WHITE);
}

static void show_game_over(void) {
    /* v7 Req 2: a REAL game-over screen that STAYS. Best score is
       tracked for the session. The main loop only restarts from
       here on a FRESH press (over_armed — see main()). */
    state = ST_OVER;
    if (score > best_score) best_score = score;
    over_armed = 0;
    /* v13 Change 3: GAME OVER art on the far starfield layer */
    dmaCopy(gameover_map_bin, BG_MAP_RAM(18), 2048);
    bgSetScroll(bg_far, 0, 0);
    /* v13 Change 4: score lines on the sub screen in tile text */
    sub_clear_rows(0, 23);
    char buf[24];
    snprintf(buf, sizeof buf, "FINAL SCORE %d", score);
    sub_text2_center(6, buf, SUB_PAL_YELLOW);
    snprintf(buf, sizeof buf, "WAVE %d", wave);
    sub_text2_center(9, buf, SUB_PAL_WHITE);
    snprintf(buf, sizeof buf, "BEST %d", best_score);
    sub_text2_center(12, buf, SUB_PAL_CYAN);
}

/* finalize a dying enemy: the pixelated explosion, score, drops, wave
   advance. v18: the explosion moved HERE from damage_enemy — it fires
   at the END of the visible death drift, not at the killing hit.
   v19: for the two drifting types that is still true; every other
   type calls this directly from damage_enemy, so its explosion
   fires at the killing hit. */
static void kill_enemy_final(Enemy *e) {
    int ecx = e->x / 256 + 16, ecy = e->y / 256 + 16;
    e->active = 0;
    e->dying = 0;
    int kind = enemy_boom_kind[e->type];
    int big = (enemy_def[e->type].hp >= 4);
    spawn_explosion(ecx, ecy, kind, big);
    sfx_boom();
    score += enemy_def[e->type].score;
    kills++;
    /* v16: weapon bonuses HALVED (was 22% per kill) and a rare 1UP
       added — power-ups should feel earned, not flooded. Per kill:
       3% 1UP, 8% weapon capsule (types 1-3). Boss-death drops below
       (boss_death_final) are unchanged — boss rewards stay generous. */
#ifdef QA_DEBUG
    qa_drop_rolls++;
#endif
    {
        int r = rand() % 100;
        if (r < 3) {
            spawn_pickup(ecx, ecy, 5);   /* 1UP */
#ifdef QA_DEBUG
            qa_drop_1up++;
#endif
        } else if (r < 11) {
            spawn_pickup(ecx, ecy, 1 + rand() % 3);
#ifdef QA_DEBUG
            qa_drop_weap++;
#endif
        }
    }
    if (!boss.active) {
#ifdef QA_DEBUG
        int nw = 1 + kills / 4;   /* QA-only: faster waves */
#else
        int nw = 1 + kills / 12;
#endif
        if (nw > wave) {
            wave = wave + 1;   /* one wave at a time — never skip a boss wave */
            if (wave % 3 == 0) {
                /* v7: EVERY boss gets the intro (alarm x3 + WARNING,
                   then the mode's scroll transition) — not just the
                   first. intro_mode is the gimmick of the next boss:
                   boss_num+1 cycles 1 reverse, 2 halt, 3 upward,
                   4 shield+charge. v6 Fix 1: clear the field. */
                clear_regulars();
                boss_intro = 1;
                intro_t = 0;
                music_boss_start();   /* v11: tempo lifts 25% for the fight */
                intro_mode = ((boss_num) % 4) + 1;
                set_cue("!! WARNING !!", SUB_PAL_RED, 180);
                int na = ((wave - 1) / 2) % NUM_AREAS;
                if (na != cur_area) {
                    /* v18: the TERRAIN changes for the fight, but the
                       MUSIC stays on the current area's track — it
                       keeps playing, sped up 1.25x by music_boss_start
                       above, until the boss dies (the designer's boss-music
                       rule). The old code called terrain_set_area
                       here, which switched the track to the next
                       area at the intro, at full volume. */
                    terrain_set_area_nomusic(na);
                    draw_sub_info();
                }
            } else {
                int na = ((wave - 1) / 2) % NUM_AREAS;
                if (na != cur_area) {
                    terrain_set_area(na);
                    draw_sub_info();
                }
            }
        }
    }
}

/* damage an enemy; starts the death drift (v4 Fix 3) on a fatal hit */
static void damage_enemy(Enemy *e, int dmg) {
    if (e->dying) return;
    e->hp -= dmg;
    e->flash = 5;
    if (e->hp > 0) {
        /* v17: HIT SPARK — the shot landed on an enemy that takes
           multiple hits and survived: a tiny flash at the impact
           point (enemy centre — the bullet strikes the front edge,
           close enough at 32 px). 1-HP enemies never reach here;
           they die below and get the full explosion instead. */
        spawn_hitspark(e->x / 256 + 16, e->y / 256 + 16);
    }
    if (e->hp <= 0) {
        /* v19: only the two ship-like types drift (type 0 drifter,
           type 1 weaver — the designer, Nov 19: the drift should belong to
           one or two enemy types only). Every other type gets its
           per-type pixelated explosion IMMEDIATELY at the killing
           hit, as in v8 — kill_enemy_final fires the explosion,
           scores and drops right here, no drift at all.
           For the two drifting types the v18 visible drift is kept
           but tuned down: SHORTER (36 frames, ~0.6 s, was 84) and
           SLOWER (0.5 px/frame vertical, was 1 px/frame). The ship
           keeps its sprite, blinks (e->flash, via the draw loop) and
           trails embers while it drifts, and the per-type pixelated
           explosion fires at the END of the drift, in
           kill_enemy_final — or at once if it reaches the top or
           bottom of the screen (see update_play). Drift direction
           is NOT random: it comes from WHERE the enemy dies — above
           the vertical middle of the screen (enemy centre y < 96)
           it drifts UP, below the middle it drifts DOWN, exactly on
           the line it drifts down. The direction is picked ONCE here
           and locked: e->drift is never re-picked while dying, so y
           moves monotonically in one direction for the whole drift. */
        if (e->type != 0 && e->type != 1) {
            kill_enemy_final(e);
            return;
        }
        e->dying = 1;
        e->die_t = 0;
        int ecy = e->y / 256 + 16;
        e->drift = (ecy < 96) ? -1 : 1;
        e->flash = 40;   /* blink for the whole ~36-frame drift */
    }
}

/* v8 Fix 2: the boss's final blast — runs AFTER the staggered death
   sequence (see update_play), not at the killing hit. Restores
   normal flight for EVERY mode and hands out the boss rewards. */
static void boss_death_final(void) {
    int bcx = boss.x + 32, bcy = boss.y + 32;
    int mode = boss.mode;   /* capture before clearing */
    boss.active = 0;
    boss.dying = 0;
    music_boss_end();   /* v11: tempo ramps back to normal; the area
                           crossfade below carries the music into the
                           next area's own track if the area changes */
    spawn_boom_pal(bcx, bcy, PAL_BOOM, 0, 0);
    spawn_boom_pal(bcx + 12, bcy - 10, PAL_BOOM, 64, -64);
    spawn_boom_pal(bcx - 14, bcy + 8, PAL_BOOM, -64, 64);
    for (int k = 0; k < 10; k++)
        spawn_particle2(bcx + (rand() % 56) - 28, bcy + (rand() % 56) - 28,
                        (rand() % 5 - 2) * 256, (rand() % 5 - 2) * 256,
                        PAL_BOOM, 16);
    sfx_boom();
    score += 1500;
    spawn_t = 90;   /* v6 Fix 1: breather before spawns resume */
    /* v9 Fix 1: boss defeated — restore normal flight for EVERY
       mode. Reverse (mode 1) and halt (mode 2) ease the horizontal
       scroll back through a stop to +100; upward (mode 3) eases
       the VERTICAL scroll back to 0 the same way, and the
       horizontal scroll eases back to +100. The ship never moved.
       Upward (mode 3) also brings the ceiling back (terrain_rebuild)
       for the next area. */
    if (mode != 0) {
        reverse_mode = 0;
        vert_target = 0;
        if (mode == 3) {
            /* v13 Fix 2: the new area's terrain comes back GRADUALLY
               — its ceiling/floor ease in as scroll_terr_y eases
               back to 0. It is NOT snapped to 0 any more (the snap
               made the floor pop back in instantly). The area is
               switched BEFORE terrain_rebuild() so the rebuild draws
               the NEW area's ceiling/floor, not the old one's.
               v18: terrain only here — the music switch is in the
               block below (fade-in), once, for every mode. */
            int na3 = ((wave - 1) / 2) % NUM_AREAS;
            if (na3 != cur_area) terrain_set_area_nomusic(na3);
            terrain_rebuild();
        }
        scroll_target = 100;
        set_cue("NORMAL FLIGHT", SUB_PAL_GREEN, 180);
    }
    spawn_pickup(bcx, bcy, 1 + rand() % 3);
    spawn_pickup(bcx, bcy + 24, 4);
    int na = ((wave - 1) / 2) % NUM_AREAS;
    if (na != cur_area) {
        terrain_set_area_nomusic(na);
        draw_sub_info();
    }
    /* v18: the next area's music starts ONLY now — after the boss is
       defeated — and it FADES IN from silence over ~1.5 s (the designer's
       boss-music rule). If the boss's area didn't change (e.g. the
       wave-6 boss), na == music_area and this is a no-op: the same
       track just keeps playing at normal tempo. */
    if (na != music_area) music_set_area_fadein(na);
}

/* damage the boss; returns 1 if it died */
static int damage_boss(int dmg) {
    if (!boss.active) return 0;
    if (boss.dying) return 0;   /* v8 Fix 2: already going down */
    /* v7 mode 4: the shield blocks ALL damage while it is up — the
       boss flashes sparks instead of taking the hit. */
    if (boss.mode == 4 && boss.shield) {
        spawn_particle(boss.x + 32, boss.y + 32);
        return 0;
    }
    boss.hp -= dmg;
    if (boss.hp <= 0) {
        /* v8 Fix 2: do NOT pop him at once. Start the death
           sequence — a chain of explosions around his body over
           ~1.5 s (update_play), then boss_death_final() clears him.
           His HP bar is already empty (hp = 0). */
        boss.dying = 1;
        boss.die_t = 0;
        sfx_boom();
        return 1;
    }
    /* v16: boss-damage tick — a short tick per damaging hit, throttled
       to one per 6 frames so rapid fire stays a ticking rhythm, not
       a wall of noise. Only on real damage (shield blocks and the
       killing blow return above, so they never tick). */
    {
        static int last_tick = -100;
        if (frame - last_tick >= 6) { sfx_bosshit(); last_tick = frame; }
    }
    /* v17: HIT SPARK on the boss too — tiny flash at his front edge
       (he faces left), alongside the tick, on every damaging hit. */
    spawn_hitspark(boss.x + 6, boss.y + 32);
    return 0;
}

/* lose a life (hit by enemy / bullet / terrain / obstacle) */
static void player_hit(void) {
#ifdef QA_DEBUG
    return;   /* QA-only god mode: the ship cannot die in debug runs */
#endif
    int pcx = ship_x + 16, pcy = ship_y + 16;
    spawn_boom(pcx, pcy);
    sfx_playerdmg();   /* v17: distinct player-damage sound (was sfx_hit) */
    lives--;
    if (weapon != W_SINGLE) {   /* hit loses the current weapon */
        owned[weapon] = 0;
        weapon = W_SINGLE;
        draw_sub_info();
    }
    rate_lvl = 0;               /* hit also loses fire-rate level */
    if (lives <= 0) {
        /* v7 Req 2: do NOT jump straight to GAME OVER — play the
           death first: big explosion at the ship, ~1.5 s, THEN the
           game-over screen. The ship is hidden during the dying. */
        ship_dead = 1;
        die_t = 0;
        spawn_boom(pcx - 16, pcy - 16);
        spawn_boom(pcx + 8, pcy - 8);
        spawn_boom(pcx - 8, pcy + 8);
        for (int k = 0; k < 8; k++)
            spawn_particle(pcx + (rand() % 40) - 20, pcy + (rand() % 40) - 20);
        sfx_boom();
        state = ST_DYING;
        return;
    }
    invuln = 150;
    ship_x = 16;
    ship_y = 80;
}

/* ------------------------------------------------------------------ */
/* update                                                              */
/* ------------------------------------------------------------------ */

static void update_play(touchPosition *touch, int touching) {
    /* --- v9: boss intro — alarm x3 (sfx_alarm at intro_t 1/61/121,
           for EVERY boss — the intro is the only path to spawn_boss,
           so no boss can miss it), then the boss's own scroll
           transition. Mode 1 (boss 1, PERFECT as-is): world scroll
           eases to a stop and reverses to max speed -160. Mode 2:
           scroll eases to a full HALT (0) for the battle. Mode 3:
           horizontal scroll eases to a stop, then the world scrolls
           VERTICALLY (up flight, +160, faster than normal) — the
           ship and boss keep their normal positions/orientations.
           Mode 4: normal scroll, the boss brings his shield.
           The ship STAYS exactly where it is on the left. --- */
    if (boss_intro) {
        intro_t++;
        if (intro_t == 1 || intro_t == 61 || intro_t == 121) sfx_alarm();
        if (intro_t >= 180) {
            boss_intro = 0;
            if (intro_mode == 1) {
                reverse_mode = 1;
                scroll_target = -160;   /* max-speed reverse */
                set_cue("REVERSE!", SUB_PAL_CYAN, 180);
            } else if (intro_mode == 2) {
                scroll_target = 0;      /* full halt for the battle */
                set_cue("HALT!", SUB_PAL_YELLOW, 180);
            } else if (intro_mode == 3) {
                scroll_target = 0;      /* horizontal flight slows to a stop */
                vert_target = -160;     /* ...then you fly UP (background
                                           scrolls DOWN, faster than
                                           normal — same trick as boss 1) */
                terrain_open_ceiling(); /* ceiling OPEN in this section,
                                           floor/ground kept as-is */
                /* terrain obstacles would float in open sky once the
                   floor band slides away — clear them all; they
                   respawn after the fight. */
                for (int i = 0; i < MAX_TOBST; i++) tobsts[i].active = 0;
                /* v13 Fix 2: floating rocks too (they would hover in
                   the full starfield once the floor flies down). */
                for (int i = 0; i < MAX_OBST; i++) obsts[i].active = 0;
                set_cue("UPWARD!", SUB_PAL_GREEN, 180);
            } else {
                scroll_target = 100;    /* shield boss: normal scroll */
                set_cue("SHIELD!", SUB_PAL_ORANGE, 180);
            }
            spawn_boss();
        }
    }

    /* --- boss cue: clear the banner when its time is up --- */
    if (cue_t > 0 && --cue_t == 0) clear_cue();

    /* --- ship movement: RELATIVE drag only -------------------------- */
    if (touching) {
        if (!touch_down) {
            touch_down = 1;
            drag_down_x = touch->px;
            drag_down_y = touch->py;
            drag_ship_x = ship_x;
            drag_ship_y = ship_y;
            drag_moved = 0;
            drag_is_panel = (touch->py >= PANEL_Y);
            drag_active = !drag_is_panel;
        }
        last_touch_x = touch->px;
        last_touch_y = touch->py;
        if (drag_active) {
            int dx = touch->px - drag_down_x;
            int dy = touch->py - drag_down_y;
            if (abs(dx) > 3 || abs(dy) > 3)
                drag_moved = 1;
            if (drag_moved) {
                ship_x = drag_ship_x + dx;
                ship_y = drag_ship_y + dy;
            }
        }
    } else if (touch_down) {
        touch_down = 0;
        if (drag_is_panel && !drag_moved) {
            int btn = last_touch_x / 64;   /* 4 buttons, 64px each */
            if (btn >= 0 && btn < 4 && owned[btn]) {
                weapon = (Weapon)btn;
                draw_sub_info();
                sfx_shoot();
            }
        }
        drag_active = 0;
        drag_is_panel = 0;
        drag_moved = 0;
    }
    u16 held = keysHeld();
    if (held & KEY_LEFT)  ship_x -= 3;
    if (held & KEY_RIGHT) ship_x += 3;
    if (held & KEY_UP)    ship_y -= 3;
    if (held & KEY_DOWN)  ship_y += 3;
    if (ship_x < 0) ship_x = 0;
    if (ship_x > 224) ship_x = 224;
    if (ship_y < 8) ship_y = 8;
    if (ship_y > 160) ship_y = 160;

    /* --- auto-fire (v4 Fix 7: slower base rate, rate pickups) --- */
    if (--fire_t <= 0) fire_weapon();
    /* v5 Fix 4: player engine glow */
    if ((frame % 6) == 0) spawn_particle(ship_x - 2, ship_y + 14);

    /* --- spawn waves: enemies enter from the right.
           Frozen while a mini-boss blocks progression. --- */
    if (!boss.active && !boss_intro && --spawn_t <= 0) {
        /* v5 Fix 2: waves 1-2 spawn slower; later waves ramp up. */
        int interval = (wave <= 2) ? 115 - wave * 5 : 95 - wave * 5;
        if (interval < 35) interval = 35;
        spawn_t = interval;
        /* eligible types for this area + wave */
        int elig[NUM_ENEMY_TYPES], n = 0;
        for (int t = 0; t < NUM_ENEMY_TYPES; t++) {
            if (enemy_def[t].min_wave <= wave &&
                (enemy_def[t].areas & (1 << cur_area)))
                elig[n++] = t;
        }
        if (n > 0) spawn_enemy(elig[rand() % n]);
    }

    /* --- floating pickup (weapon or fire-rate) --- */
    if (--pickup_t <= 0) {
        /* v16: was 1100 frames (~18 s) — bonuses half as often. A
           rare 1UP rides the floater too (8% of floaters). */
        pickup_t = 2200;
        int ft = 1 + rand() % 4;
        if ((rand() % 100) < 8) ft = 5;   /* 1UP */
        spawn_pickup(264, 24 + rand() % 130, ft);
#ifdef QA_DEBUG
        qa_drop_float++;
        if (ft == 5) qa_drop_1up++;
#endif
    }

    /* --- floating rock obstacles (terrain areas only) --- */
    if (cur_area > 0 && !boss.active && !boss_intro && --obst_t <= 0) {
        obst_t = (cur_area <= 1) ? 900 : 650;   /* v5 Fix 2: fewer rocks early */
        spawn_obst();
    }

    /* --- terrain obstacle sprites (v4 Fix 5) --- */
    if (cur_area > 0 && tunnel_t <= 0 && !boss.active && !boss_intro && --tobst_t <= 0) {
        tobst_t = (cur_area <= 1) ? 90 : (cur_area == 2 ? 70 : 45);
        spawn_tobst();
    }

    /* --- player bullets --- */
    for (int i = 0; i < MAX_PBULLETS; i++) {
        PBullet *b = &pbullets[i];
        if (!b->active) continue;
        if (b->kind == 2) {   /* laser beam: follows the ship, short life.
                                 v9 Fix 1: always horizontal. */
            b->x = ship_x + 24;
            b->y = ship_y + 12;
            if (++b->life >= 14) b->active = 0;
            continue;
        }
        if (b->kind == 1) {   /* missile: home + spark trail */
            int best = -1, bestd = 1 << 30;
            for (int j = 0; j < MAX_ENEMIES; j++) {
                Enemy *e = &enemies[j];
                if (!e->active || e->dying) continue;
                int dx = e->x / 256 - b->x, dy = e->y / 256 - b->y;
                int d = dx * dx + dy * dy;
                if (d < bestd) { bestd = d; best = j; }
            }
            if (best >= 0) {
                int dy = enemies[best].y / 256 - b->y;
                if (dy > 4) b->vy += 32;
                else if (dy < -4) b->vy -= 32;
                if (b->vy > 2 * 256 + 128) b->vy = 2 * 256 + 128;
                if (b->vy < -2 * 256 - 128) b->vy = -2 * 256 - 128;
            } else if (boss.active) {
                int dy = boss.y + 32 - b->y;
                if (dy > 4) b->vy += 32;
                else if (dy < -4) b->vy -= 32;
            }
            if ((frame % 3) == 0)
                spawn_particle(b->x - 2, b->y + 2);
        }
        b->x += b->vx / 256;
        b->y += b->vy / 256;
        if (b->x > 264 || b->y < -16 || b->y > 208) b->active = 0;
    }

    /* --- enemies --- */
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy *e = &enemies[i];
        if (!e->active) continue;
        e->t++;
        if (e->flash > 0) e->flash--;
        if (e->dying) {
            /* v19: SHORTER (36 frames, ~0.6 s, was 84) and SLOWER
               (0.5 px/frame vertical, was 1 px/frame) than v18, and
               only the drifter/weaver ever reach here — every other
               type explodes at the killing hit (see damage_enemy).
               The ship sprite stays up for the whole drift (draw no
               longer skips dying enemies), blinking via e->flash and
               trailing embers, so it reads as a ship going down.
               If the wreck reaches the top (y <= 0) or bottom
               (y >= 160*256, sprite bottom at 192) of the screen it
               explodes there at once instead of finishing the drift.
               Drift lasts 36 frames: forward 200/frame, vertical
               0.5 px/frame in the locked direction picked at the
               fatal hit (never re-picked, so y is monotonic). Then
               kill_enemy_final fires the pixelated explosion and
               finalises the kill. */
            e->die_t++;
            e->x -= 200;
            e->y += e->drift * 128;
            if (e->y <= 0 || e->y >= 160 * 256) {
                kill_enemy_final(e);
                continue;
            }
            if ((e->die_t % 2) == 0)
                spawn_particle2(e->x / 256 + 16, e->y / 256 + 16, -256, 0,
                                boom_pal[enemy_boom_kind[e->type]], 12);
            if (e->die_t >= 36) kill_enemy_final(e);
            continue;
        }
        const EnemyDef *def = &enemy_def[e->type];
        int ex = e->x / 256, ey = e->y / 256;
        /* v5 Fix 6: in reverse flight enemies fly right-to-left's
           mirror — they enter from the left and move right. */
        switch (def->style) {
        case ST_DRIFT:
            e->x -= def->speed;
            e->y = e->base_y + (int)(10 * sin(e->t * 0.03)) * 256;
            break;
        case ST_SINE:
            e->x -= def->speed * 8 / 10;
            e->y = e->base_y + (int)(48 * sin(e->t * 0.05)) * 256;
            break;
        case ST_DIVE:
            if (ex > 190) {
                e->x -= def->speed;
            } else {
                int dy = (ship_y + 16) - (ey + 16);
                int step = dy > 0 ? 460 : -460;
                if (dy > -8 && dy < 8) step = 0;
                e->y += step;
                e->x -= 560;
            }
            break;
        case ST_HOVER:
            if (ex > 205) e->x -= 205;
            else e->y = e->base_y + (int)(8 * sin(e->t * 0.05)) * 256;
            break;
        case ST_TUMBLE:
            e->x -= def->speed;
            e->y = e->base_y + (int)(14 * sin(e->t * 0.02)) * 256;
            if (++e->flip_t >= 20) {
                e->flip_t = 0;
                e->flip_h = !e->flip_h;
                if (e->t % 40 == 0) e->flip_v = !e->flip_v;
            }
            break;
        case ST_ZIGZAG:
            e->x -= def->speed;
            e->y = e->base_y + (int)(30 * sin(e->t * 0.09)) * 256
                 + (((e->t / 10) % 2) ? 12 : -12) * 256;
            break;
        case ST_DASH:
            if (ex > 180) e->x -= 200;
            else if ((e->t % 120) < 50) e->x -= 720;
            else e->x -= 100;
            break;
        case ST_ORBIT:
            e->base_x -= 180;
            e->x = e->base_x + (int)(22 * cos(e->t * 0.06)) * 256;
            e->y = e->base_y + (int)(40 * sin(e->t * 0.06)) * 256;
            break;
        case ST_SNAKE:
            e->x -= def->speed;
            e->y = e->base_y + (int)(36 * sin(e->t * 0.09)) * 256;
            break;
        case ST_SLOW:
            e->x -= def->speed * 6 / 10;
            e->y = e->base_y + (int)(30 * sin(e->t * 0.06)) * 256;
            break;
        }
        /* v5 Fix 4: per-type animation — spin / wings / pulse / wobble */
        {
            int an = enemy_anim[e->type];
            if (an == 1) {
                if ((e->t % 12) == 0) e->flip_h = !e->flip_h;
                if ((e->t % 24) == 0) e->flip_v = !e->flip_v;
            } else if (an == 2) {
                if ((e->t % 8) == 0) e->flip_v = !e->flip_v;
            } else if (an == 3) {
                if ((e->t % 20) == 0) e->flip_h = !e->flip_h;
                if ((e->t % 30) == 0) spawn_particle(ex + 16, ey + 16);
            } else if (an == 4) {
                if ((e->t % 15) == 0) e->flip_h = !e->flip_h;
            } else if (an == 5) {
                /* v7: two-frame walk/pulse — the frame swap itself
                   happens in draw_play via gfx_enemy_alt[]. */
            }
        }
        /* enemy shooting — v5 Fix 2: slower bullets in waves 1-2 */
        if (def->shoot > 0) {
            if (--e->shoot_t <= 0) {
                e->shoot_t = def->shoot + rand() % 50;
                int cx = e->x / 256, cy = e->y / 256 + 12;
                int dx = (ship_x + 16) - cx, dy = (ship_y + 16) - cy;
                int len = (int)sqrt(dx * dx + dy * dy);
                if (len < 8) len = 8;
                int bspeed = (wave <= 2) ? 380 : 500;
                fire_ebullet(cx, cy, dx * bspeed / len, dy * bspeed / len);
                if (e->type == ET_CAPITAL) {
                    fire_ebullet(cx, cy, dx * bspeed / len - 120, dy * bspeed / len - 200);
                    fire_ebullet(cx, cy, dx * bspeed / len - 120, dy * bspeed / len + 200);
                }
            }
        }
        if (e->x / 256 < -40) e->active = 0;
    }

    /* --- mini-boss (v4 Fix 4, v5 Fix 1: first boss much easier,
           v7: each boss has its own gimmick) --- */
    if (boss.active && boss.dying) {
        /* v8 Fix 2: boss death sequence — a chain of explosions
           around the boss's body, staggered over ~1.5 s, with a
           shake; then the final big blast clears him. He does not
           move, shoot, or take damage while dying (damage_boss
           returns 0), and no other enemies spawn (spawn gate checks
           boss.active, which stays set until the final blast). */
        boss.die_t++;
        if ((boss.die_t % 12) == 0) {
            int bx = boss.x + (rand() % 64);
            int by = boss.y + (rand() % 64);
            spawn_boom_pal(bx, by, PAL_BOOM,
                           (rand() % 3 - 1) * 256, (rand() % 3 - 1) * 256);
            spawn_particle2(bx, by, -128, -64, PAL_BOOM, 14);
            spawn_particle2(bx, by, 128, 64, PAL_BOOM, 14);
            sfx_boom();
        }
        boss.x += (rand() % 5) - 2;   /* shake */
        boss.y += (rand() % 5) - 2;
        if (boss.die_t >= 90) boss_death_final();
    } else if (boss.active) {
        boss.t++;
        /* v9 Fix 1: mode 3 parks on the right like every other
           boss (the generic branch below) — no more descending
           from the top or weaving across a shaft. */
        if (boss.mode == 4 && boss.charging) {
            /* SHIELD boss charge: dashes left across the screen,
               then drifts back to his parking spot. */
            boss.x -= 5;
            if (boss.x <= 40) { boss.charging = 0; boss.returning = 1; }
        } else if (boss.mode == 4 && boss.returning) {
            boss.x += 3;
            if (boss.x >= 196) boss.returning = 0;
        } else {
            /* v6: boss always parks on the right (x=196), as normal. */
            if (boss.x > 196) boss.x -= 2;
            boss.y = 80 + (int)((boss.diff == 1 ? 32 : 48) * sin(boss.t * 0.02));
        }
        /* v5 Fix 4: boss animates (flips + sparkles) */
        if ((boss.t % 30) == 0) boss_flip = !boss_flip;
        if ((boss.t % 40) == 0) spawn_particle(boss.x + 32, boss.y + 32);
        /* v7 mode 4: shield cycles — up ~2 s, down ~1.5 s. While it
           is up the boss is invulnerable (see damage_boss); sparks
           ring the shield so the player can see it. */
        if (boss.mode == 4) {
            if (--boss.shield_t <= 0) {
                boss.shield = !boss.shield;
                boss.shield_t = boss.shield ? 120 : 90;
            }
            if (boss.shield && (boss.t % 10) == 0)
                spawn_particle(boss.x + (rand() % 64), boss.y + (rand() % 64));
            if (--boss.charge_t <= 0 && !boss.charging && !boss.returning) {
                boss.charge_t = 300;
                boss.charging = 1;
            }
        }
        if (--boss.shoot_t <= 0) {
            /* aimed shot: mode 1 slow/single, mode 2 slower still,
               mode 3 aims LEFT at the player like the others,
               mode 4 faster. (dx/dy are computed from the boss to
               the ship, so every mode shoots toward the player.) */
            if (boss.mode == 1)      boss.shoot_t = 170;
            else if (boss.mode == 2) boss.shoot_t = 200;
            else if (boss.mode == 3) boss.shoot_t = 150;
            else                     boss.shoot_t = 140;
            int cx = boss.x, cy = boss.y + 28;
            int dx = (ship_x + 16) - cx, dy = (ship_y + 16) - cy;
            int len = (int)sqrt(dx * dx + dy * dy);
            if (len < 8) len = 8;
            int bspeed = (boss.mode == 1) ? 380 : (boss.mode == 2) ? 320 : 450;
            fire_ebullet(cx, cy, dx * bspeed / len, dy * bspeed / len);
            if (boss.diff > 1 && boss.mode != 2) {
                fire_ebullet(cx, cy, dx * bspeed / len - 100, dy * bspeed / len - 220);
                fire_ebullet(cx, cy, dx * bspeed / len - 100, dy * bspeed / len + 220);
            }
        }
        if (--boss.fan_t <= 0) {
            /* spread fan: mode 1 = 3 slow bullets (PERFECT as-is),
               mode 2 = 2 slow bullets, mode 3 = 3 leftward bullets,
               mode 4 = 3 faster bullets. v9: no more downward fan. */
            if (boss.mode == 1)      { boss.fan_t = 320; }
            else if (boss.mode == 2) { boss.fan_t = 420; }
            else if (boss.mode == 3) { boss.fan_t = 300; }
            else                     { boss.fan_t = 260; }
            if (boss.mode == 2) {
                /* halt boss: just 2 slow bullets, wide apart */
                fire_ebullet(boss.x, boss.y + 28, -300, -120);
                fire_ebullet(boss.x, boss.y + 28, -300,  120);
            } else {
                int fk = 1;
                int fv = (boss.mode == 1) ? 140 : 180;
                int fs = (boss.mode == 1) ? 320 : 420;
                for (int k = -fk; k <= fk; k++)
                    fire_ebullet(boss.x, boss.y + 28, -fs, k * fv);
            }
        }
        /* v6 Fix 1: NO minions during a boss fight — boss alone.
           (minion_t is never reset/fired; the field is removed.) */
    }

    /* --- obstacles drift left --- */
    for (int i = 0; i < MAX_OBST; i++) {
        Obst *o = &obsts[i];
        if (!o->active) continue;
        o->t++;
        o->x -= 2;
        if (o->x < -40) o->active = 0;
    }
    for (int i = 0; i < MAX_TOBST; i++) {
        TObst *o = &tobsts[i];
        if (!o->active) continue;
        o->x -= 2;
        if (o->x < -40) o->active = 0;
    }

    /* --- pickups drift left and bob --- */
    for (int i = 0; i < MAX_PICKUPS; i++) {
        Pickup *p = &pickups[i];
        if (!p->active) continue;
        p->t++;
        p->x -= 1;
        if (p->x < -40) p->active = 0;
    }

    /* --- particles fade (v8 Fix 1: they fly on their own velocity
           and live for their own life, in their explosion's colour) --- */
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle *p = &particles[i];
        if (!p->active) continue;
        p->t++;
        p->x += p->vx / 256;
        p->y += p->vy / 256;
        if (p->t > p->life) p->active = 0;
    }

    /* --- enemy bullets --- */
    for (int i = 0; i < MAX_EBULLETS; i++) {
        EBullet *b = &ebullets[i];
        if (!b->active) continue;
        b->x += b->vx;
        b->y += b->vy;
        if (b->y / 256 > 208 || b->y / 256 < -16 ||
            b->x / 256 < -16 || b->x / 256 > 272)
            b->active = 0;
    }

    /* --- collisions: player bullets / beam vs enemies and boss --- */
    for (int i = 0; i < MAX_PBULLETS; i++) {
        PBullet *b = &pbullets[i];
        if (!b->active) continue;
        if (b->kind == 2) {   /* beam: long horizontal hit box.
                                 v9 Fix 1: no vertical beam any more. */
            for (int j = 0; j < MAX_ENEMIES; j++) {
                Enemy *e = &enemies[j];
                if (!e->active || e->dying) continue;
                int ecx = e->x / 256 + 16, ecy = e->y / 256 + 16;
                if (ecx > b->x - 10 && ecx < b->x + 128 &&
                    abs(ecy - (b->y + 4)) < 22) {
                    if (++b->hits > 2) b->active = 0;
                    damage_enemy(e, b->dmg);
                }
            }
            if (boss.active) {
                int bcx = boss.x + 32, bcy = boss.y + 32;
                if (bcx + 34 > b->x - 10 && bcx - 34 < b->x + 128 &&
                    abs(bcy - (b->y + 4)) < 34) {
                    if (++b->hits > 2) b->active = 0;
                    damage_boss(b->dmg);
                }
            }
            continue;
        }
        int bcx = b->x + 16, bcy = b->y + 4;
        int consumed = 0;
        for (int j = 0; j < MAX_ENEMIES; j++) {
            Enemy *e = &enemies[j];
            if (!e->active || e->dying) continue;
            int ecx = e->x / 256 + 16, ecy = e->y / 256 + 16;
            if (abs(bcx - ecx) < 26 && abs(bcy - ecy) < 26) {
                if (b->pierce) {
                    if (++b->hits > 3) b->active = 0;
                } else {
                    b->active = 0;
                }
                damage_enemy(e, b->dmg);
                consumed = 1;
                break;
            }
        }
        if (!consumed && boss.active) {
            int bcy2 = boss.y + 32;
            /* v7: bullet point vs the boss BOX, not just his centre —
               a bullet that spawns inside the boss's span (ship flown
               up close) still counts instead of flying harmlessly
               past his centre. */
            if (bcx > boss.x - 8 && bcx < boss.x + 72 &&
                abs(bcy - bcy2) < 34) {
                if (b->pierce) {
                    if (++b->hits > 3) b->active = 0;
                } else {
                    b->active = 0;
                }
                damage_boss(b->dmg);
                consumed = 1;
            }
        }
        /* bullets vs floating-rock obstacles */
        if (b->active && !consumed) {
            for (int j = 0; j < MAX_OBST; j++) {
                Obst *o = &obsts[j];
                if (!o->active) continue;
                if (abs(bcx - (o->x + 16)) < 26 && abs(bcy - (o->y + 16)) < 26) {
                    b->active = 0;
                    if (--o->hp <= 0) {
                        o->active = 0;
                        spawn_boom(o->x + 16, o->y + 16);
                        sfx_boom();
                        score += 50;
                    }
                    break;
                }
            }
        }
    }

    /* --- collisions: pickups vs player --- */
    {
        int pcx = ship_x + 16, pcy = ship_y + 16;
        for (int i = 0; i < MAX_PICKUPS; i++) {
            Pickup *p = &pickups[i];
            if (!p->active) continue;
            int qcx = p->x + 16, qcy = p->y + 16;
            if (abs(pcx - qcx) < 28 && abs(pcy - qcy) < 28) {
                p->active = 0;
                if (p->type == 5) {   /* v16: 1UP — extra life, max 5 */
                    if (lives < 5) {
                        lives++;
                        set_cue("1UP!", SUB_PAL_YELLOW, 150);
                    } else {
                        score += 1000;
                        set_cue("1UP! +1000", SUB_PAL_YELLOW, 150);
                    }
                    sfx_pickup();
                } else if (p->type == 4) {   /* fire-rate capsule */
                    if (rate_lvl < 3) rate_lvl++;
                    sfx_pickup();
                } else {
                    owned[p->type] = 1;
                    weapon = (Weapon)p->type;
                    sfx_pickup();
                    draw_sub_info();
                }
            }
        }
    }

    /* --- collisions: enemies / enemy bullets / terrain / obstacles --- */
    if (invuln > 0) invuln--;
    int pcx = ship_x + 16, pcy = ship_y + 16;
    int hit = 0;
    if (invuln <= 0) {
        for (int j = 0; j < MAX_ENEMIES; j++) {
            Enemy *e = &enemies[j];
            if (!e->active || e->dying) continue;
            int ecx = e->x / 256 + 16, ecy = e->y / 256 + 16;
            if (abs(pcx - ecx) < 22 && abs(pcy - ecy) < 22) {
                e->active = 0;
                spawn_boom(ecx, ecy);
                hit = 1;
                break;
            }
        }
        if (!hit && boss.active) {
            if (abs(pcx - (boss.x + 32)) < 36 && abs(pcy - (boss.y + 32)) < 36)
                hit = 1;
        }
        if (!hit) {
            for (int i = 0; i < MAX_EBULLETS; i++) {
                EBullet *b = &ebullets[i];
                if (!b->active) continue;
                int bcx = b->x / 256 + 16, bcy = b->y / 256 + 4;
                if (abs(pcx - bcx) < 18 && abs(pcy - bcy) < 18) {
                    b->active = 0;
                    hit = 1;
                    break;
                }
            }
        }
        if (!hit && cur_area > 0) {
            /* v9 Fix 1: ceiling/floor stay solid in EVERY boss fight
               — there is no open shaft any more. */
            int top_px, bot_px;
            terrain_at(pcx, &top_px, &bot_px);
            if (ship_y < top_px || ship_y + 32 > 192 - bot_px)
                hit = 1;
        }
        if (!hit) {
            for (int j = 0; j < MAX_OBST; j++) {
                Obst *o = &obsts[j];
                if (!o->active) continue;
                if (abs(pcx - (o->x + 16)) < 24 && abs(pcy - (o->y + 16)) < 24) {
                    o->active = 0;
                    spawn_boom(o->x + 16, o->y + 16);
                    hit = 1;
                    break;
                }
            }
        }
        if (!hit) {   /* terrain obstacle sprites (solid) */
            for (int j = 0; j < MAX_TOBST; j++) {
                TObst *o = &tobsts[j];
                if (!o->active) continue;
                if (pcx > o->x + 4 && pcx < o->x + 28 &&
                    pcy > o->y + 4 && pcy < o->y + o->h - 4) {
                    hit = 1;
                    break;
                }
            }
        }
    }
    if (hit) {
        player_hit();
        if (state != ST_PLAY) return;
    }

    /* --- explosions (v8 Fix 1: booms drift apart on their velocity) --- */
    for (int i = 0; i < MAX_BOOMS; i++) {
        Boom *b = &booms[i];
        if (!b->active) continue;
        b->x += b->vx / 256;
        b->y += b->vy / 256;
        if (++b->t >= 8) {
            b->t = 0;
            if (++b->frame >= 3) b->active = 0;
        }
    }

    scroll_starfield();
    /* v6 Fix 2: terrain scrolls with the same eased velocity as the
       starfield — it slows, stops, then reverses with it. */
    terrain_update();
#ifdef QA_DEBUG
    /* QA-only readout on the sub screen (rows 3 and 7, free during
       play): ship pos, horizontal scroll velocity, boss active/
       mode, VERTICAL scroll velocity, FLOOR offset, shield. v9:
       U is vert_vel (-160 = flying UP at max speed). v13: Y is
       scroll_terr_y (-48 once the floor has slid down for the
       boss-3 upward flight, 0 when the new area's floor is back).
       Drawn as tile text — v13 removed the console, so iprintf
       would write to an invisible map. */
    if ((frame % 30) == 0) {
        int ne = 0;
        for (int i = 0; i < MAX_ENEMIES; i++)
            if (enemies[i].active && !enemies[i].dying) ne++;
        char rbuf[40];
        snprintf(rbuf, sizeof rbuf, "S%d,%d V%+d E%d B%dM%d U%+d Y%+d",
                 ship_x, ship_y, scroll_vel, ne, boss.active,
                 boss.active ? boss.mode : 0, vert_vel, scroll_terr_y);
        sub_text(0, 3, rbuf, SUB_PAL_WHITE);
        /* v11: re-print the music line every readout so it survives
           area changes. Shows the track, its volume (0..127, ramping
           during a crossfade) and the tempo multiplier (1.00 normal,
           1.25 boss lift). */
        if (music_play_area >= 0) {
            snprintf(rbuf, sizeof rbuf, "MUS %s a%d v%d x%d.%02d",
                     music_name[music_play_area], music_play_area, music_vol,
                     music_mul_q / 256, (music_mul_q % 256) * 100 / 256);
            sub_text(0, 7, rbuf, SUB_PAL_WHITE);
        }
        /* v16: drop-rate readout — kills, weapon capsules, 1UPs,
           floaters. Proves the halved rates and the rare 1UP.
           v17: SP = hit sparks spawned (multi-HP enemy + boss hits). */
        snprintf(rbuf, sizeof rbuf, "DROP k%d w%d 1up%d fl%d L%d SP%d",
                 qa_drop_rolls, qa_drop_weap, qa_drop_1up, qa_drop_float,
                 lives, qa_sparks);
        sub_text(0, 6, rbuf, SUB_PAL_WHITE);
        /* v18: death-drift readout — first dying enemy's centre y,
           locked drift direction (+1 down / -1 up) and drift age in
           frames. Proves the position rule (y<96 -> up, y>=96 ->
           down) and that y moves monotonically one way per death:
           die_t rises while y steps steadily in the locked
           direction. Row 8 is otherwise unused. */
        {
            int dy_i = -1;
            for (int i = 0; i < MAX_ENEMIES; i++)
                if (enemies[i].active && enemies[i].dying) { dy_i = i; break; }
            if (dy_i >= 0) {
                Enemy *de = &enemies[dy_i];
                snprintf(rbuf, sizeof rbuf, "DY y%d d%+d t%d",
                         de->y / 256 + 16, de->drift, de->die_t);
            } else {
                snprintf(rbuf, sizeof rbuf, "DY -");
            }
            sub_text(0, 8, rbuf, SUB_PAL_WHITE);
        }
    }
#endif
}

/* ------------------------------------------------------------------ */
/* draw                                                                */
/* ------------------------------------------------------------------ */

static int oam_id;

static void draw_number(int x, int y, int value, int digits) {
    /* right-aligned, leading blanks; glyphs are 16x16 sprites */
    int div = 1;
    for (int i = 1; i < digits; i++) div *= 10;
    for (int i = 0; i < digits; i++) {
        int d = (value / div) % 10;
        int blank = (value < div && i < digits - 1);
        if (!blank && oam_id < 127) {
            oamSet(&oamMain, oam_id++, x + i * 10, y, 0, PAL_BOOM,
                   SpriteSize_16x16, SpriteColorFormat_16Color,
                   gfx_digits[d], -1, false, false, false, false, false);
        }
        div /= 10;
    }
}

/* v13 Change 5: the border colour is now rainbow_col() — a smooth
   rainbow hue cycle (4 s) combined with a breathing brightness
   (2 s sine, 40-100%, never off). The old glow_col()/border_col[]
   single-colour glow is gone. */
static u16 rainbow_col(void) {
    double a = sin(2.0 * 3.14159265358979323846 * (double)(frame % 120) / 120.0);
    int v = (int)(178.0 + 77.0 * a);   /* brightness 101..255, smooth */
    int h = ((frame % 240) * 3) / 2;   /* hue 0..359 over 4 s */
    int sec = h / 60, f = h % 60;
    int q = v * (60 - f) / 60, t = v * f / 60;
    int r, g, b;
    switch (sec) {
        case 0: r = v; g = t; b = 0; break;
        case 1: r = q; g = v; b = 0; break;
        case 2: r = 0; g = v; b = t; break;
        case 3: r = 0; g = q; b = v; break;
        case 4: r = t; g = 0; b = v; break;
        default: r = v; g = 0; b = q; break;
    }
    return RGB15(r >> 3, g >> 3, b >> 3);
}

static void draw_play(int touching, touchPosition *touch) {
    oamClear(&oamMain, 0, 127);
    oamClear(&oamSub, 0, 127);
    oam_id = 0;

    /* top HUD: score digits top-left, lives icons below, wave top-right */
    draw_number(2, 2, score, 6);
    for (int k = 0; k < lives && oam_id < 127; k++) {
        oamSet(&oamMain, oam_id++, 2 + k * 14, 20, 0, PAL_PLAYER,
               SpriteSize_16x16, SpriteColorFormat_16Color,
               gfx_player_mini, -1, false, false, false, false, false);
    }
    if (oam_id < 127) {
        oamSet(&oamMain, oam_id++, 208, 2, 0, PAL_BOOM,
               SpriteSize_16x16, SpriteColorFormat_16Color,
               gfx_digits[10], -1, false, false, false, false, false);
    }
    draw_number(222, 2, wave, 2);

    /* mini-boss HP bar: up to 5 beam segments, top centre */
    if (boss.active) {
        int segs = (boss.hp * 5 + boss.maxhp - 1) / boss.maxhp;
        for (int s = 0; s < segs && oam_id < 127; s++) {
            oamSet(&oamMain, oam_id++, 48 + s * 32, 20, 0, PAL_LASERBEAM,
                   SpriteSize_32x8, SpriteColorFormat_16Color,
                   gfx_laserbeam, -1, false, false, false, false, false);
        }
    }

    /* weapon buttons on the sub screen (always visible) + v13
       Change 5 rainbow glowing selection border. The border is a
       64x32 outline sprite drawn at OAM id 0 — same priority as
       the buttons, but the LOWER OAM index draws in front, so the
       bright outline sits on top of the button's edge. Its colour
       cycles through the spectrum while its brightness breathes
       (rainbow_col: 4 s hue cycle, 2 s sine fade, never blinking
       off). The old star marker above the button is GONE: it used
       to move with the finger while dragging. Uncollected weapons
       still draw in the grey locked palette, and the border is
       only ever on the selected weapon. */
    for (int i = 0; i < 4; i++) {
        int bpal = owned[i] ? (PAL_BTN_SINGLE + i) : PAL_BTN_LOCKED;
        oamSet(&oamSub, 1 + i, i * 64, PANEL_Y, 0, bpal,
               SpriteSize_64x32, SpriteColorFormat_16Color, gfx_btn[i],
               -1, false, false, false, false, false);
    }
    SPRITE_PALETTE_SUB[PAL_BORDER * 16 + 1] = rainbow_col();
    oamSet(&oamSub, 0, (int)weapon * 64, PANEL_Y, 0, PAL_BORDER,
           SpriteSize_64x32, SpriteColorFormat_16Color, gfx_border,
           -1, false, false, false, false, false);

    /* player (blink while invulnerable) */
    int hide_ship = (invuln > 0) && ((frame / 4) % 2 == 0);
    /* v6/v9: the ship ALWAYS faces right — only the scroll reverses
       (boss 1) or goes vertical (boss 3). It never faces up. */
    oamSet(&oamMain, oam_id++, ship_x, ship_y, 0, PAL_PLAYER, SpriteSize_32x32,
           SpriteColorFormat_16Color, gfx_player,
           -1, false, hide_ship || ship_dead,
           false, false, false);

    /* enemies (flash on hit; v18: dying ones ARE drawn again — the
       visible death drift: a dying ship keeps its sprite, blinking
       via e->flash while it drifts, so it reads as a wreck going
       down; v7 alt-frame types swap to their second frame every
       15 frames) */
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy *e = &enemies[i];
        if (!e->active || oam_id >= 127) continue;
        int hide = (e->flash > 0) && ((frame % 2) == 0);
        u16 *egfx = gfx_enemy[e->type];
        if (gfx_enemy_alt[e->type] && ((e->t / 15) % 2) == 1)
            egfx = gfx_enemy_alt[e->type];
        oamSet(&oamMain, oam_id++, e->x / 256, e->y / 256, 0,
               enemy_def[e->type].pal,
               SpriteSize_32x32, SpriteColorFormat_16Color,
               egfx, -1,
               false, hide, e->flip_h, e->flip_v, false);
    }

    /* mini-boss (64x64) — v7: one sprite + palette per boss form;
       v8 Fix 2: flashes while dying (his death sequence is running) */
    if (boss.active && oam_id < 127) {
        int bpal = (boss.form == 1 || boss.form == 2) ? PAL_WEAVER : PAL_SHOOTER;
        int hide_boss = boss.dying && ((frame % 2) == 0);
        oamSet(&oamMain, oam_id++, boss.x, boss.y, 0,
               bpal,
               SpriteSize_64x64, SpriteColorFormat_16Color,
               gfx_boss[boss.form], -1, false, hide_boss, boss_flip, false, false);
    }

    /* terrain obstacle sprites (stacked for tall ones) */
    for (int i = 0; i < MAX_TOBST; i++) {
        TObst *o = &tobsts[i];
        if (!o->active) continue;
        int nseg = o->h / 32;
        for (int s = 0; s < nseg && oam_id < 127; s++) {
            int sy = o->top ? o->y + s * 32 : o->y + s * 32;
            oamSet(&oamMain, oam_id++, o->x, sy, 0, o->pal,
                   SpriteSize_32x32, SpriteColorFormat_16Color,
                   gfx_obs[o->gfx], -1, false, false, false, false, false);
        }
    }

    /* floating-rock obstacles */
    for (int i = 0; i < MAX_OBST; i++) {
        Obst *o = &obsts[i];
        if (!o->active || oam_id >= 127) continue;
        oamSet(&oamMain, oam_id++, o->x, o->y, 0, PAL_ASTEROID,
               SpriteSize_32x32, SpriteColorFormat_16Color,
               o->crystal ? gfx_crystal : gfx_asteroid, -1,
               false, false, false, false, false);
    }

    /* pickups (bob) */
    for (int i = 0; i < MAX_PICKUPS; i++) {
        Pickup *p = &pickups[i];
        if (!p->active || oam_id >= 127) continue;
        int bob = (int)(3 * sin(p->t * 0.15));
        /* v16: type 5 (1UP) shares PAL_MISSILE — its gold/pink live
           in the patched slots 14/15, same as the missile pickup. */
        int pal = (p->type == 1) ? PAL_POW_SPREAD :
                  (p->type == 2) ? PAL_POW_LASER :
                  (p->type == 4) ? PAL_DRIFTER : PAL_MISSILE;
        oamSet(&oamMain, oam_id++, p->x, p->y + bob, 0, pal,
               SpriteSize_32x32, SpriteColorFormat_16Color,
               gfx_pow[p->type - 1], -1, false, false, false, false, false);
    }

    /* player bullets / missiles / laser beam */
    for (int i = 0; i < MAX_PBULLETS; i++) {
        PBullet *b = &pbullets[i];
        if (!b->active) continue;
        if (b->kind == 2) {   /* beam: four 32px segments = ~128px.
                                 v9 Fix 1: always horizontal. */
            for (int s = 0; s < 4 && oam_id < 127; s++) {
                oamSet(&oamMain, oam_id++, b->x + s * 28, b->y, 0, PAL_LASERBEAM,
                       SpriteSize_32x8, SpriteColorFormat_16Color,
                       gfx_laserbeam, -1, false, false, false, false, false);
            }
            continue;
        }
        if (oam_id >= 127) continue;
        oamSet(&oamMain, oam_id++, b->x, b->y, 0,
               b->kind == 1 ? PAL_MISSILE : PAL_BULLET_P,
               SpriteSize_32x8, SpriteColorFormat_16Color,
               b->kind == 1 ? gfx_missile2 : gfx_bullet_p,
               -1, false, false, false, false, false);
    }

    /* missile trail / explosion particles (v8 Fix 1: each spark is
       drawn in its explosion's palette, and hides after 8 frames) */
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle *p = &particles[i];
        if (!p->active || oam_id >= 127) continue;
        int hide = (p->t > 8);
        oamSet(&oamMain, oam_id++, p->x, p->y, 0, p->pal,
               SpriteSize_16x16, SpriteColorFormat_16Color, gfx_spark,
               -1, false, hide, false, false, false);
    }

    /* enemy bullets */
    for (int i = 0; i < MAX_EBULLETS; i++) {
        EBullet *b = &ebullets[i];
        if (!b->active || oam_id >= 127) continue;
        oamSet(&oamMain, oam_id++, b->x / 256, b->y / 256, 0, PAL_BULLET_E,
               SpriteSize_32x8, SpriteColorFormat_16Color, gfx_bullet_e,
               -1, false, false, false, false, false);
    }

    /* explosions (v8 Fix 1: each fireball is drawn in its
       explosion's palette) */
    for (int i = 0; i < MAX_BOOMS; i++) {
        Boom *b = &booms[i];
        if (!b->active || oam_id >= 127) continue;
        oamSet(&oamMain, oam_id++, b->x, b->y, 0, b->pal,
               SpriteSize_32x32, SpriteColorFormat_16Color,
               gfx_boom[b->frame], -1, false, false, false, false, false);
    }

    /* touch reticle on the sub screen (drag area only) — v10: moved
       to OAM id 5, since id 0 is now the selection border. The star
       still follows the finger while dragging (the designer: that's fine);
       it is no longer the selection marker. */
    if (touching && drag_active) {
        oamSet(&oamSub, 5, touch->px - 8, touch->py - 8, 0, PAL_RETICLE,
               SpriteSize_16x16, SpriteColorFormat_16Color, gfx_reticle,
               -1, false, false, false, false, false);
    }

    oamUpdate(&oamMain);
    oamUpdate(&oamSub);
}

static void draw_title(void) {
    oamClear(&oamMain, 0, 127);
    oamClear(&oamSub, 0, 127);
    int id = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy *e = &enemies[i];
        if (!e->active) continue;
        e->t++;
        e->x -= 200;
        if (e->x / 256 < -40) e->x = 264 * 256;
        e->y = e->base_y + (int)(12 * sin(e->t * 0.04)) * 256;
        oamSet(&oamMain, id++, e->x / 256, e->y / 256, 0,
               enemy_def[e->type].pal,
               SpriteSize_32x32, SpriteColorFormat_16Color,
               gfx_enemy[e->type], -1,
               false, false, false, false, false);
    }
    scroll_starfield();
    terrain_update();
    /* v13 Change 3: the title art lives on bg_far — keep that layer
       still while the near/mid starfields scroll behind it. */
    bgSetScroll(bg_far, 0, 0);
    /* v13 Change 3: pulse the TAP TO START prompt (palette 9,
       index 1 of the title art) so it breathes. */
    {
        double a = sin(2.0 * 3.14159265358979323846 *
                       (double)(title_t % 120) / 120.0);
        int s = (int)(190.0 + 65.0 * a);   /* 125..255 */
        BG_PALETTE[144 + 1] = RGB15(31 * s / 255, 28 * s / 255, 8 * s / 255);
    }
    /* v13 Change 4: the weapon panel, greyed, so the touchpad
       looks finished even on the title screen. */
    for (int i = 0; i < 4; i++)
        oamSet(&oamSub, 1 + i, i * 64, PANEL_Y, 0, PAL_BTN_LOCKED,
               SpriteSize_64x32, SpriteColorFormat_16Color, gfx_btn[i],
               -1, false, false, false, false, false);
    oamUpdate(&oamMain);
    oamUpdate(&oamSub);
}

/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */

int main(void) {
    srand(0xC0FFEE);
    /* v4 Fix 1: console FIRST (melonDS JIT bisect: videoSetMode
       before consoleDemoInit makes the JIT failure worse — black
       instead of white top screen — and consoleDemoInit is the
       stock libnds order). v13 Change 4: videoSetModeSub IS now
       called after consoleDemoInit — it wipes the console, which
       is exactly what we want: the release build never prints
       anything, and the sub screen is a finished tile-text UI
       instead (the v4 bisect m4 "wipes the console" finding was
       a problem only while the game still USED the console). */
    consoleDemoInit();
    videoSetMode(MODE_0_2D);
    videoSetModeSub(MODE_0_2D);
    vramSetBankA(VRAM_A_MAIN_BG);
    vramSetBankB(VRAM_B_MAIN_SPRITE);
    vramSetBankC(VRAM_C_SUB_BG);
    vramSetBankD(VRAM_D_SUB_SPRITE);

    /* v13 Change 4: sub-screen tile-text UI. BG0 is a 4bpp tile
       layer (map base 8, tile base 0) with the pixel font loaded
       into its tile RAM and a navy backdrop. */
    bgSub = bgInitSub(0, BgType_Text4bpp, BgSize_T_256x256, 8, 0);
    dmaCopy(font1_bin, BG_TILE_RAM_SUB(0), font1_bin_size);
    dmaCopy(font2_bin, (u8 *)BG_TILE_RAM_SUB(0) + 64 * 32, font2_bin_size);
    for (int p = 0; p < 7; p++) BG_PALETTE_SUB[p * 16] = RGB15(2, 3, 10);
    BG_PALETTE_SUB[1 * 16 + 1] = RGB15(10, 28, 31);  /* cyan */
    BG_PALETTE_SUB[2 * 16 + 1] = RGB15(31, 28, 8);   /* yellow */
    BG_PALETTE_SUB[3 * 16 + 1] = RGB15(31, 10, 10);  /* red */
    BG_PALETTE_SUB[4 * 16 + 1] = RGB15(12, 31, 12);  /* green */
    BG_PALETTE_SUB[5 * 16 + 1] = RGB15(31, 31, 31);  /* white */
    BG_PALETTE_SUB[6 * 16 + 1] = RGB15(31, 18, 6);   /* orange */
    sub_clear_rows(0, 23);

    load_sprites();
    build_digits();
    build_border();   /* v10 Change 2: glowing selection border tile */
    build_starfield();
    /* v13 Change 3: save the far starfield map (title/game-over
       art swaps it out) and load the title/game-over art tiles
       + palettes into main BG tile RAM (bases 16 and 239). */
    dmaCopy(BG_MAP_RAM(18), far_map_save, 2048);
    dmaCopy(title_bin, (u8 *)BG_TILE_RAM(0) + 16 * 32, title_bin_size);
    dmaCopy(gameover_bin, (u8 *)BG_TILE_RAM(0) + 239 * 32,
            gameover_bin_size);
    dmaCopy(title_pal_bin, &BG_PALETTE[128], 64);
    build_terrain();

    soundEnable();
    music_set_area(0);   /* v12: OPEN SPACE track at boot, embedded, full volume */

    show_title();

    touchPosition touch;
    while (1) {
        swiWaitForVBlank();
        scanKeys();
        touchRead(&touch);
        int touching = (keysHeld() & KEY_TOUCH) != 0;
        u16 down = keysDown();
        frame++;
        music_update();   /* v11: music crossfade + boss tempo ramp */

        if (state == ST_TITLE) {
            title_t++;
            draw_title();
            if (touching || (down & (KEY_START | KEY_A))) {
                start_game();
            }
        } else if (state == ST_PLAY) {
            update_play(&touch, touching);
            if (state == ST_PLAY) {
                draw_play(touching, &touch);
                if (down & KEY_START) {
                    show_title();
                }
            }
        } else if (state == ST_DYING) {
            /* v7 Req 2: death animation — the ship's explosion plays
               on the top screen for ~1.5 s (90 frames) before the
               GAME OVER screen. No input is accepted here. */
            die_t++;
            for (int i = 0; i < MAX_BOOMS; i++) {
                Boom *b = &booms[i];
                if (!b->active) continue;
                if (++b->t >= 8) {
                    b->t = 0;
                    if (++b->frame >= 3) b->active = 0;
                }
            }
            for (int i = 0; i < MAX_PARTICLES; i++) {
                Particle *p = &particles[i];
                if (!p->active) continue;
                if (++p->t > 12) p->active = 0;
            }
            oamClear(&oamMain, 0, 127);
            oam_id = 0;
            for (int i = 0; i < MAX_BOOMS; i++) {
                Boom *b = &booms[i];
                if (!b->active || oam_id >= 127) continue;
                oamSet(&oamMain, oam_id++, b->x, b->y, 0, PAL_BOOM,
                       SpriteSize_32x32, SpriteColorFormat_16Color,
                       gfx_boom[b->frame], -1, false, false, false, false, false);
            }
            for (int i = 0; i < MAX_PARTICLES; i++) {
                Particle *p = &particles[i];
                if (!p->active || oam_id >= 127) continue;
                oamSet(&oamMain, oam_id++, p->x, p->y, 0, PAL_BOOM,
                       SpriteSize_16x16, SpriteColorFormat_16Color, gfx_spark,
                       -1, false, false, false, false, false);
            }
            oamUpdate(&oamMain);
            if (die_t >= 90) show_game_over();
        } else { /* ST_OVER */
            /* v7 Req 2: the GAME OVER screen STAYS. It only restarts
               on a FRESH press: the player must first release the
               touch/buttons (over_armed), so a finger still held
               down from the moment of death cannot skip the screen. */
            oamClear(&oamMain, 0, 127);
            oamSet(&oamMain, 0, 112, 80, 0, PAL_BOOM,
                   SpriteSize_32x32, SpriteColorFormat_16Color,
                   gfx_boom[2], -1, false, false, false, false, false);
            oamUpdate(&oamMain);
            /* v13 Change 3: pulse the TOUCH TO RETRY prompt (same
               palette 9 index 1 as the title prompt). */
            {
                double a = sin(2.0 * 3.14159265358979323846 *
                               (double)(frame % 120) / 120.0);
                int s = (int)(190.0 + 65.0 * a);   /* 125..255 */
                BG_PALETTE[144 + 1] =
                    RGB15(31 * s / 255, 28 * s / 255, 8 * s / 255);
            }
            if (!touching && !(keysHeld() & (KEY_START | KEY_A)))
                over_armed = 1;
            if (over_armed && (touching || (down & (KEY_START | KEY_A)))) {
                start_game();
            }
        }
        bgUpdate();
    }
    return 0;
}
