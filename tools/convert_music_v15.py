#!/usr/bin/env python3
"""Convert the Star Drag per-area AI music tracks to DS-playable raw PCM,
EMBEDDED in the ARM9 binary (bin2o, audio/music_aN.bin) — v15.

WHY v15 (the designer, Nov 19, 2026): v14's music is STILL distorted in the
game — even area 1, which was fine in v12 and whose source is clean.
Diagnosis (measured Nov 19, see qa/VERIFICATION.md v15 section):
  1. GAMEPLAY MIXER CLIPPING — the real in-game distortion. v14's
     music-ONLY title capture is clean (peak 0.49, zero pinned
     samples), but the GAMEPLAY capture (music + shooting SFX) hits
     full scale constantly: peaks 1.00 in ~half the seconds, up to
     3.83% of samples pinned in a single second (v14 audio-a0 run).
     The DS hardware mixer sums the channels: music peak 0.65 x
     80/127 = 0.41 plus shoot 0.52 / laser 0.69 / hit 0.88 / boom
     1.00 (boom alone pins at vol 127). v12's area-1 loop was ~3.5 dB
     quieter (RMS -21.4 dBFS, peak 0.60) so the sum stayed under full
     scale far more often — the designer called v12 area 1 fine. The v14
     compare "in-game" clips were TITLE captures (music only), so
     they never showed the gameplay mix she actually hears — that is
     why melonDS title evidence said "clean" while the game sounded
     distorted. v15 verifies on GAMEPLAY captures, not title-only.
  2. BAD SOURCE WINDOWS for areas 2-4: the v13/v14 "loudest window"
     pick landed on genuinely overdriven sections of the OLD tracks
     (OLD area-2 window 52-66s: decode peak 1.19 — clips AT the
     source — RMS -10.5 dBFS, 0.41% of samples within 1 dB of full
     scale; OLD area-3 window 58-72s: a dark bass murmur, centroid
     239 Hz). Fixed by three fresh clean Speechma generations
     (assets/music-area{2,3,4}-*-v2.mp3, prompts asked for a clean
     mix; decode peaks 0.75 / 1.10 / 1.11, no squashed windows).
FIX (this script + no code changes — volumes stay at v12):
  - Back to the v12 recipe: plain resample, NO EQ lift, NO RMS
    normalization, NO loudest-window-blind pick. 14 s loops, all at
    22050 Hz, 0.1 s fade-in, 0.5 s fade-out at the loop seam.
  - Area 1 uses window 0 — EXACTLY v12's window (0-14 s of
    music-v2.mp3), the one proven clean on the designer's device.
  - Areas 2-7 use the loudest CLEAN window: peak <= 1.00 and
    squash (samples within 1 dB of full scale) <= 0.05% — unlike
    v13/v14's distortion-blind loudest pick, which chose a window
    with peak 1.19. Windows below were measured with
    /tmp/v15-window-scan.py; the script re-checks them at runtime.
  - Peak-safe gain ONLY: gain = min(0.9, 0.60 / window_peak), so
    every shipped loop peaks at or below 0.60 in the float chain —
    and, measured on the shipped u8 bytes, at ~0.42, exactly v12
    area-1's shipped peak (0.43). NOTE: ffmpeg's MP3-decoded path
    scales the u8 output by 1/sqrt(2) = 0.707 vs the float chain
    (measured Nov 19: least-squares k = 0.707, corr 0.9993, same
    filter chain to f32 vs u8) — so the f32-side cap of 0.60 lands
    at 0.42 in the bytes the DS actually plays. Do NOT "fix" this
    by raising the gain: 0.42 IS the v12-proven value. Worst-case
    mixer sum music+shoot is then 0.42 x 80/127 + 0.656 x 100/127
    = 0.26 + 0.52 = 0.78, under full scale with margin. No
    limiter, no EQ.
  - Volumes unchanged (music 80, shoot 100, laser 120, hit 127,
    boom 127, pickup 120, alarm 127).

Sources: area 1 assets/music-v2.mp3 (85 s); areas 2-4 the v2
Speechma tracks; areas 5-7 assets/music-area{5,6,7}.mp3.
Output: audio/music_aN.bin — unsigned 8-bit PCM, mono, 22050 Hz.
Usage: python3 tools/convert_music_v15.py [N ...]
"""
import os
import subprocess
import sys

import numpy as np

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
WIN = 14.0          # seconds of loop per area
RATE = 22050
GAIN_MAX = 0.9      # v12's gain
PEAK_CAP = 0.60     # v12 area-1's shipped peak — the proven-clean value
FADE_IN = 0.1
FADE_OUT = 0.5
# Window starts (seconds). Area 1 = 0 (v12 exact). Areas 2-7 = loudest
# window with decode peak <= 1.00 and squash <= 0.05% (measured):
#   a2 win42 RMS -13.7 peak 0.75 squash 0.00 | a3 win56 -12.2 / 0.99 / 0.02
#   a4 win56 -14.4 / 0.82 / 0.00 | a5 win56 -13.4 / 0.88 / 0.00
#   a6 win42 -14.1 / 0.88 / 0.00 | a7 win28 -13.0 / 0.94 / 0.00
WIN_START = {1: 0, 2: 42, 3: 56, 4: 56, 5: 56, 6: 42, 7: 28}
SRC = {1: "music-v2.mp3",
       2: "music-area2-spacebase-v2.mp3",
       3: "music-area3-planet-v2.mp3",
       4: "music-area4-cave-v2.mp3",
       5: "music-area5.mp3",
       6: "music-area6.mp3",
       7: "music-area7.mp3"}


def stats(x):
    """x: float array in [-1, 1]. Returns (rms_db, peak, squash%, hf, cen)."""
    if len(x) == 0:
        return -99.0, 0.0, 0.0, 0.0, 0.0
    rms = np.sqrt(np.mean(x ** 2))
    X = np.abs(np.fft.rfft(x * np.hanning(len(x)))) ** 2
    f = np.fft.rfftfreq(len(x), 1 / RATE)
    tot = X.sum() + 1e-12
    hf = X[(f >= 1000) & (f < 8000)].sum() / tot
    cen = (f * X).sum() / tot
    return (20 * np.log10(rms + 1e-9), np.max(np.abs(x)),
            np.mean(np.abs(x) > 0.891) * 100, hf, cen)


def convert(area):
    src = os.path.join(ROOT, "assets", SRC[area])
    dst = os.path.join(ROOT, "audio", f"music_a{area}.bin")
    if not os.path.exists(src):
        print(f"area {area}: MISSING {src} — skipping")
        return None
    start = WIN_START[area]
    # Pass 1: decode the window, measure it, check it is clean.
    r = subprocess.run(["ffmpeg", "-v", "error", "-ss", str(start),
                        "-t", str(WIN), "-i", src, "-ac", "1",
                        "-ar", str(RATE), "-f", "f32le", "-"],
                       capture_output=True)
    if r.returncode != 0:
        sys.exit(f"ffmpeg failed on {src}: {r.stderr.decode()[:300]}")
    x = np.frombuffer(r.stdout, dtype=np.float32).astype(np.float64)
    rms_db, peak, squash, hf, cen = stats(x)
    assert peak <= 1.00, f"area {area}: window peak {peak:.2f} over 1.00!"
    assert squash <= 0.05, f"area {area}: window squash {squash:.2f}% over 0.05%!"
    # Peak-safe gain only: v12's 0.9, capped so the shipped peak is
    # at most 0.60 (v12 area-1's proven value). No EQ, no RMS target.
    gain = min(GAIN_MAX, PEAK_CAP / peak)
    gain_db = 20 * np.log10(gain)
    # Pass 2: gain + loop-seam fades only.
    fade_out_start = WIN - FADE_OUT
    af = (f"volume={gain:.4f},"
          f"afade=t=in:st=0:d={FADE_IN},"
          f"afade=t=out:st={fade_out_start}:d={FADE_OUT}")
    cmd = ["ffmpeg", "-y", "-v", "error", "-ss", str(start), "-t", str(WIN),
           "-i", src, "-ac", "1", "-ar", str(RATE),
           "-af", af, "-f", "u8", dst]
    r = subprocess.run(cmd)
    if r.returncode != 0:
        sys.exit(r.returncode)
    n = os.path.getsize(dst)
    y = (np.fromfile(dst, dtype=np.uint8).astype(np.float64) - 128) / 128.0
    frms, fpeak, fsquash, fhf, fcen = stats(y)
    nrail = int(np.sum(np.abs(y) >= 0.9765625))
    print(f"area {area}: window {start}s..{start + int(WIN)}s "
          f"(raw {rms_db:.1f}dBFS/peak {peak:.2f}/squash {squash:.2f}%/"
          f"HF{hf:.3f}/{cen:.0f}Hz) -> {n} bytes, RMS {frms:.1f}dBFS, "
          f"peak {fpeak:.2f}, HF {fhf:.3f}, centroid {fcen:.0f}Hz, "
          f"gain {gain_db:+.1f}dB, rail samples {nrail}")
    assert fpeak <= PEAK_CAP + 0.01, f"area {area}: peak {fpeak} over cap!"
    assert nrail == 0, f"area {area}: {nrail} rail samples!"
    return n


if __name__ == "__main__":
    areas = [int(a) for a in sys.argv[1:]] or [1, 2, 3, 4, 5, 6, 7]
    total = 0
    for a in areas:
        n = convert(a)
        if n:
            total += n
    print(f"total embedded music: {total} bytes ({total / 1024 / 1024:.2f} MB)")
