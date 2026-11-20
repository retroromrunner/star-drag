#!/usr/bin/env python3
"""Convert the Star Drag per-area AI music tracks to DS-playable raw PCM,
EMBEDDED in the ARM9 binary (bin2o, audio/music_aN.bin) — v14.

WHY v14 (the designer, Nov 19, 2026): v13's music sounds GARBLED / DISTORTED
in the game, and turning the volume up made it worse. Diagnosis
(measured Nov 19, see qa/VERIFICATION.md v14 section):
  - The v13 FILES are not digitally clipped (peaks 0.76-0.92, zero
    samples at the rails) and the music ALONE plays clean in melonDS
    (title capture: peak 0.85, zero pinned samples).
  - The distortion is in the MIX + the processing. v13 made the music
    ~9 dB hotter than v12 (loops RMS -17 dBFS vs -23.6 for area 1,
    channel volume 110 vs 80): a music peak 0.92 x 110/127 = 0.80 of
    full scale, plus shooting SFX on top, sums the DS 16-bit mixer
    over full scale — the melonDS gameplay capture proves the summed
    output pins at full scale (2063 pinned samples, 1599 flat-top
    samples in 37 s). Louder volume = more clipping = worse, exactly
    as the designer reported.
  - The v13 presence lift (+5 dB @ 1200 Hz, +3 dB @ 3 kHz) pushed the
    EQ'd signal's peaks to 1.29-1.77 (up to +4.9 dB OVER full scale)
    before gain reduction, and quadrupled the HF energy share
    (area 1: 0.307 of energy in 1-8 kHz vs 0.074 in v12) — a much
    harsher, brighter sound on a phone speaker, which reads as
    garbled/distorted even where it is not digitally clipping.
  - The source MP3s for areas 2 and 6 are themselves mastered hot
    (decode peaks 1.19 / 1.13, inter-sample overs) — noted, not fixed
    at the source; the converter keeps peaks safely below full scale.
FIX (this script + code changes in main.c):
  - Volumes REVERTED to v12 (music channel 80, shoot 100, laser 120,
    hit 127, boom 127, pickup 120, alarm 127) — the designer's explicit ask.
  - Loops re-rendered PEAK-SAFE: the BEST 14 s window of each source
    is KEPT (same windows as v13 — they are genuinely the clearest
    sections), but the aggressive presence lift is GONE (v13: +5 dB @
    1200 Hz, +3 dB @ 3 kHz, which pushed EQ'd peaks to 1.29-1.77 and
    quadrupled the HF share). A GENTLE +2 dB highshelf @ 1200 Hz is
    kept — applied BEFORE the peak-capping gain, so it can never clip
    (it measurably brightens the dark areas: area 3 centroid
    239 -> 306 Hz, area 4 412 -> 529 Hz). Gain is the MINIMUM of
    (RMS target -18 dBFS) and (peak cap 0.70 = -3.1 dBFS), so no
    sample can come within 3 dB of full scale and the DS mixer sum
    music+shoot stays under full scale (worst case
    0.70 x 80/127 + 0.656 x 100/127 = 0.44 + 0.52 = 0.96). No limiter
    (gain reduction handles peaks — no squashing).
  - All 7 tracks stay at 22050 Hz, 14 s (ARM9 load cap 0x27c000).

Sources (never regenerated): area 1 assets/music-v2.mp3 (85 s),
areas 2-7 assets/music-areaN.mp3 (70-85 s Speechma, Nov 19).
Output: audio/music_aN.bin — unsigned 8-bit PCM, mono, 22050 Hz.
Usage: python3 tools/convert_music_v14.py [N ...]
"""
import os
import subprocess
import sys

import numpy as np

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
WIN = 14.0          # seconds of loop per area
RATE = 22050
TARGET_DB = -18.0   # RMS target, measured on the shipped u8 bytes
PEAK_CAP = 0.70     # -3.1 dBFS: no sample within 3 dB of full scale
FADE_IN = 0.1
FADE_OUT = 0.5
# Best (loudest + brightest) window per area, from convert_music_v13.py.
BEST_WIN = {1: 65, 2: 52, 3: 58, 4: 47, 5: 54, 6: 5, 7: 34}


def stats(x):
    """x: float array in [-1, 1]. Returns (rms_db, hf_frac, centroid)."""
    if len(x) == 0:
        return -99.0, 0.0, 0.0
    rms = np.sqrt(np.mean(x ** 2))
    X = np.abs(np.fft.rfft(x * np.hanning(len(x)))) ** 2
    f = np.fft.rfftfreq(len(x), 1 / RATE)
    tot = X.sum() + 1e-12
    hf = X[(f >= 1000) & (f < 8000)].sum() / tot
    cen = (f * X).sum() / tot
    return 20 * np.log10(rms + 1e-9), hf, cen


def convert(area):
    src = (os.path.join(ROOT, 'assets', 'music-v2.mp3') if area == 1
           else os.path.join(ROOT, 'assets', f'music-area{area}.mp3'))
    dst = os.path.join(ROOT, 'audio', f'music_a{area}.bin')
    if not os.path.exists(src):
        print(f'area {area}: MISSING {src} — skipping')
        return None
    start = BEST_WIN[area]
    # Pass 1: decode the window with the GENTLE presence lift only
    # (+2 dB highshelf @ 1200 Hz — applied BEFORE the peak-capping
    # gain, so it can never clip) and measure it on the shipped u8
    # bytes — the ground truth for what the DS plays. No limiter.
    r = subprocess.run(['ffmpeg', '-v', 'error', '-ss', str(start), '-t', str(WIN),
                        '-i', src, '-ac', '1', '-ar', str(RATE),
                        '-af', 'highshelf=f=1200:g=2', '-f', 'u8', '-'],
                       capture_output=True)
    if r.returncode != 0:
        sys.exit(f"ffmpeg failed on {src}: {r.stderr.decode()[:300]}")
    x = (np.frombuffer(r.stdout, dtype=np.uint8).astype(np.float64) - 128) / 128.0
    rms_db, hf, cen = stats(x)
    peak = np.max(np.abs(x))
    gain_db = TARGET_DB - rms_db
    gain = 10 ** (gain_db / 20)
    capped = False
    if peak * gain > PEAK_CAP:
        gain = PEAK_CAP / peak
        gain_db = 20 * np.log10(gain)
        capped = True
    # Pass 2: gentle lift + gain + loop-seam fades only. No limiter —
    # the gain itself keeps every sample >= 3 dB below full scale.
    fade_out_start = WIN - FADE_OUT
    af = (f'highshelf=f=1200:g=2,volume={gain:.4f},'
          f'afade=t=in:st=0:d={FADE_IN},'
          f'afade=t=out:st={fade_out_start}:d={FADE_OUT}')
    cmd = ['ffmpeg', '-y', '-v', 'error', '-ss', str(start), '-t', str(WIN),
           '-i', src, '-ac', '1', '-ar', str(RATE),
           '-af', af, '-f', 'u8', dst]
    r = subprocess.run(cmd)
    if r.returncode != 0:
        sys.exit(r.returncode)
    n = os.path.getsize(dst)
    y = (np.fromfile(dst, dtype=np.uint8).astype(np.float64) - 128) / 128.0
    frms, fhf, fcen = stats(y)
    fpeak = np.max(np.abs(y))
    nrail = int(np.sum(np.abs(y) >= 0.9765625))
    print(f'area {area}: window {start}s..{start + int(WIN)}s '
          f'(raw {rms_db:.1f}dBFS/HF{hf:.3f}/{cen:.0f}Hz/peak {peak:.2f}) -> '
          f'{n} bytes, RMS {frms:.1f}dBFS, HF {fhf:.3f}, '
          f'centroid {fcen:.0f}Hz, peak {fpeak:.2f}, gain {gain_db:+.1f}dB'
          f'{" [PEAK-CAPPED]" if capped else ""}, rail samples {nrail}')
    assert fpeak <= PEAK_CAP + 0.01, f"area {area}: peak {fpeak} over cap!"
    assert nrail == 0, f"area {area}: {nrail} rail samples!"
    return n


if __name__ == '__main__':
    areas = [int(a) for a in sys.argv[1:]] or [1, 2, 3, 4, 5, 6, 7]
    total = 0
    for a in areas:
        n = convert(a)
        if n:
            total += n
    print(f'total embedded music: {total} bytes ({total / 1024 / 1024:.2f} MB)')
