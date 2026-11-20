#!/usr/bin/env python3
"""Convert the Star Drag per-area AI music tracks to DS-playable raw PCM,
EMBEDDED in the ARM9 binary (bin2o, audio/music_aN.bin) — v13.

WHY v13 (the designer, Nov 19, 2026): v12's music IS back, but areas 2-7 are
hard to make out while area 1 is fine. Diagnosis (measured Nov 19):
  - v12 sliced every loop from the START (0..18 s) of its source. The
    first 18 s of areas 3/4/5 are dark, bass-heavy intros: spectral
    centroid 235/250/286 Hz, only 2.8-3.8% of energy in 1-8 kHz, vs
    area 1's first 18 s at 454 Hz / 7.4%. On a phone speaker the low
    end barely reproduces, so those tracks read as a faint murmur.
  - The best 14 s window of each track (later in the song, when the
    lead/arpeggio is in) is 2-5 dB louder AND much brighter
    (e.g. area 4: 44 s, centroid 422 Hz, HF 7.5% vs 250 Hz / 2.8%).
  - SFX were louder than the music: shoot at channel volume 100 with
    raw RMS -8.4 dBFS vs music at volume 80 with raw RMS -17..-23 dBFS,
    firing every ~0.4 s — the shooting constantly masks the music.
    Area 1 survived because its lead is brighter; areas 2-7 did not.
FIX (this script + code changes in main.c):
  - Each loop is now the BEST 14 s window of its source (1 s steps,
    scored by RMS * (0.5 + HF fraction)), not the first 14 s.
  - A presence lift (highshelf +5 dB @ 1200 Hz, +3 dB @ 3 kHz) and
    loudness normalization to -16 dBFS RMS (peak capped at 0.92)
    match every area to the same clear level.
  - All 7 tracks at 22050 Hz (v12 had areas 2-7 at 16000 Hz, which
    cut everything above 8 kHz). 14 s loops keep the ARM9 load image
    under calico ds9.ld's lma9 cap of 0x27c000 (2,621,440 bytes):
    7 * 14 s * 22050 = 2,160,900 bytes music. (16 s loops overflowed
    the cap by 80,796 bytes at link time; the dead v1 synth loop,
    audio/music_synth_raw.bin, was also removed — preserved as
    qa/music_synth_v1_raw.bin.)
  - Code: MUSIC_VOL 80 -> 110, shoot 100 -> 65, laser 120 -> 85,
    hit 127 -> 90, boom 127 -> 100, pickup 120 -> 90 (alarm stays 127).

Sources (never regenerated): area 1 assets/music-v2.mp3 (85 s),
areas 2-7 assets/music-areaN.mp3 (70-85 s Speechma, Nov 19).
Output: audio/music_aN.bin — unsigned 8-bit PCM, mono, 22050 Hz.
Usage: python3 tools/convert_music_v13.py [N ...]
"""
import os
import subprocess
import sys

import numpy as np

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
WIN = 14.0          # seconds of loop per area
RATE = 22050
TARGET_DB = -17.0   # RMS target, measured on the shipped u8 bytes
PEAK_CAP = 0.92
FADE_IN = 0.1
FADE_OUT = 0.5


def decode(src, rate=RATE, ss=None, dur=None):
    cmd = ['ffmpeg', '-v', 'error']
    if ss is not None:
        cmd += ['-ss', str(ss)]
    if dur is not None:
        cmd += ['-t', str(dur)]
    cmd += ['-i', src, '-ac', '1', '-ar', str(rate), '-f', 'u8', '-']
    r = subprocess.run(cmd, capture_output=True)
    if r.returncode != 0:
        sys.exit(f"ffmpeg failed on {src}: {r.stderr.decode()[:300]}")
    return np.frombuffer(r.stdout, dtype=np.uint8).astype(np.float64)


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


def best_window(src):
    full = (decode(src) - 128) / 128.0
    W = int(WIN * RATE)
    best = None
    for st in range(0, len(full) - W + 1, RATE):
        w = full[st:st + W]
        rms_db, hf, cen = stats(w)
        rms = 10 ** (rms_db / 20)
        score = rms * (0.5 + hf)
        if best is None or score > best[0]:
            best = (score, st // RATE, rms_db, hf, cen)
    _, start, rms_db, hf, cen = best
    return start, rms_db, hf, cen


def decode_u8(src, af, ss, dur):
    """Decode through ffmpeg to raw u8 — the ground truth for what the
    DS plays (the f32le path measures ~3 dB hotter than the u8 bytes
    that actually ship, and acompressor crushes dynamics with no
    makeup, so both are avoided)."""
    cmd = ['ffmpeg', '-v', 'error', '-ss', str(ss), '-t', str(dur),
           '-i', src, '-ac', '1', '-ar', str(RATE),
           '-af', af, '-f', 'u8', '-']
    r = subprocess.run(cmd, capture_output=True)
    if r.returncode != 0:
        sys.exit(f"ffmpeg failed on {src}: {r.stderr.decode()[:300]}")
    return (np.frombuffer(r.stdout, dtype=np.uint8).astype(np.float64) - 128) / 128.0


def convert(area):
    src = (os.path.join(ROOT, 'assets', 'music-v2.mp3') if area == 1
           else os.path.join(ROOT, 'assets', f'music-area{area}.mp3'))
    dst = os.path.join(ROOT, 'audio', f'music_a{area}.bin')
    if not os.path.exists(src):
        print(f'area {area}: MISSING {src} — skipping')
        return None
    start, rms0, hf0, cen0 = best_window(src)
    # Pass 1: EQ only, measured on the shipped u8 bytes.
    fx = 'highshelf=f=1200:g=5,equalizer=f=3000:width_type=o:width=1.5:g=3'
    x = decode_u8(src, fx, start, WIN)
    rms_db, hf, cen = stats(x)
    peak = np.max(np.abs(x))
    gain_db = TARGET_DB - rms_db
    gain = 10 ** (gain_db / 20)
    if peak * gain > PEAK_CAP:
        gain = PEAK_CAP / peak
        gain_db = 20 * np.log10(gain)
    # Pass 2: EQ + gain + a lookahead limiter (only touches peaks
    # above the cap, unlike acompressor which crushed the whole
    # track) + loop-seam fades -> final file.
    fade_out_start = WIN - FADE_OUT
    af = (f'{fx},volume={gain:.4f},alimiter=limit={PEAK_CAP}:level=false,'
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
    print(f'area {area}: window {start}s..{start + int(WIN)}s '
          f'(was {rms0:.1f}dBFS/HF{hf0:.3f}/{cen0:.0f}Hz) -> '
          f'{n} bytes, RMS {frms:.1f}dBFS, HF {fhf:.3f}, '
          f'centroid {fcen:.0f}Hz, peak {fpeak:.2f}, gain {gain_db:+.1f}dB')
    return n


if __name__ == '__main__':
    areas = [int(a) for a in sys.argv[1:]] or [1, 2, 3, 4, 5, 6, 7]
    total = 0
    for a in areas:
        n = convert(a)
        if n:
            total += n
    print(f'total embedded music: {total} bytes ({total / 1024 / 1024:.2f} MB)')
