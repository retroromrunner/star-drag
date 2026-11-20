#!/usr/bin/env python3
"""Convert the Star Drag per-area AI music tracks to DS-playable raw PCM,
EMBEDDED in the ARM9 binary (bin2o, audio/music_aN.bin) — v16.

WHY v16 (the designer, Nov 19, 2026): she A/B'd the v15 compare clips — the
"source" clips are clean and perfect, the "in-game" clips (music-only
title captures) are STILL distorted/overdriven-sounding. Diagnosis
(measured Nov 19, see qa/VERIFICATION.md v16 section + /tmp/v16-diagnose.py):
the distortion is the 8-BIT PCM FORMAT itself. The shipped v15 area-1
loop, measured against its own float reference chain, has a
quantization-error SNR of only 28.5 dB — audible grit/harshness on
sustained synth tones, exactly "distorted and overdriven", and no
peak/clipping measurement can see it (peaks were 0.42, zero rail
samples). The same chain quantized to 16-bit measures 79.0 dB — 50 dB
cleaner. The distortion is baked into the bytes the DS plays, on ANY
emulator or hardware; it is NOT inherent to playing music in-game —
the DS hardware and libnds support 16-bit samples (SoundFormat_16Bit),
which is the fix. Answer to the designer: no, clean in-game music is possible;
the game was just using 8-bit samples.

FORMAT CHANGE (this script + one line in source/main.c):
  - Output is now signed 16-bit little-endian PCM (s16le), mono, 22050 Hz.
  - 16-bit DOUBLES the size, and the ARM9 load cap is 0x27c000 =
    2,621,440 bytes (v15 ARM9 binary: 2,377,132, headroom 227,924;
    non-music ARM9: 216,232). So the loops shorten: 7.5 s at 22050 Hz
    16-bit = 330,750 bytes/track, 2,315,250 total, ARM9 ~2,531,482,
    headroom ~73,574. Rate stays 22050 Hz (v12/v15 value) so the
    frequency response and tempo-lift math are unchanged; loop length
    is what gives. 14 s at 16-bit would be 4,321,800 bytes — impossible.
  - Same windows as v15 (area 1 = 0, exactly v12's window; areas 2-7
    loudest CLEAN window), same 0.1 s fade-in / 0.5 s fade-out.
  - Same target shipped peak 0.42 as v15 (mixer headroom proven there)
    — the ONLY variable that changes vs v15 is bit depth.
  - The s16 bytes are written from numpy, NOT from ffmpeg's s16le
    output, so no format-path rescaling can sneak in (v15 found
    ffmpeg's MP3-decoded u8 path scales by 1/sqrt(2) = 0.707 vs the
    float chain). ffmpeg is used only to decode/resample to f32le;
    gain + fades + quantization happen here and are asserted below.

Sources: area 1 assets/music-v2.mp3 (85 s); areas 2-4 the v2
Speechma tracks; areas 5-7 assets/music-area{5,6,7}.mp3.
Output: audio/music_aN.bin — signed 16-bit PCM, mono, 22050 Hz.
Usage: python3 tools/convert_music_v16.py [N ...]
"""
import os
import subprocess
import sys

import numpy as np

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
WIN = 7.5           # seconds of loop per area (16-bit size budget)
RATE = 22050
GAIN_MAX = 0.9      # v12's gain
PEAK_TARGET = 0.42  # v15's proven shipped peak — keep loudness identical
FADE_IN = 0.1
FADE_OUT = 0.5
# Window starts (seconds) — identical to v15:
#   a1 win0 (v12 exact) | a2 win42 | a3 win56 | a4 win56
#   a5 win56 | a6 win42 | a7 win28
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
    # Decode the window to float, measure it, check it is clean.
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
    # Gain for the v15 shipped peak (0.42), then fades — in numpy.
    gain = min(GAIN_MAX, PEAK_TARGET / peak)
    y = x * gain
    n = len(y)
    fi = int(FADE_IN * RATE)
    y[:fi] *= np.linspace(0, 1, fi)
    fo = int(FADE_OUT * RATE)
    y[n - fo:] *= np.linspace(1, 0, fo)
    # Quantize to s16 (round-to-nearest, clip) and write the bytes.
    s16 = np.clip(np.round(y * 32767), -32768, 32767).astype(np.int16)
    s16.tofile(dst)
    nbytes = os.path.getsize(dst)
    assert nbytes == int(WIN * RATE) * 2, f"area {area}: size {nbytes}!"
    # Verify from the shipped bytes: peak, RMS, quantization SNR.
    yb = np.fromfile(dst, dtype=np.int16).astype(np.float64) / 32768.0
    frms, fpeak, fsquash, fhf, fcen = stats(yb)
    err = yb - y
    qsnr = 20 * np.log10(np.sqrt(np.mean(y ** 2)) /
                         (np.sqrt(np.mean(err ** 2)) + 1e-12))
    nrail = int(np.sum(np.abs(yb) >= 32767 / 32768.0))
    print(f"area {area}: window {start}s..{start + WIN}s "
          f"(raw {rms_db:.1f}dBFS/peak {peak:.2f}/squash {squash:.2f}%/"
          f"HF{hf:.3f}/{cen:.0f}Hz) -> {nbytes} bytes, RMS {frms:.1f}dBFS, "
          f"peak {fpeak:.2f}, HF {fhf:.3f}, centroid {fcen:.0f}Hz, "
          f"gain {20 * np.log10(gain):+.1f}dB, rail {nrail}, "
          f"quant SNR {qsnr:.1f}dB")
    # Peak may sit a touch under target when the loudest sample falls
    # inside a fade (area 6: 0.40) — that is the fade doing its job.
    assert 0.38 <= fpeak <= PEAK_TARGET + 0.005, \
        f"area {area}: shipped peak {fpeak:.2f} outside [0.38, 0.425]!"
    assert nrail == 0, f"area {area}: {nrail} rail samples!"
    assert qsnr >= 70.0, f"area {area}: quant SNR {qsnr:.1f}dB < 70!"
    return nbytes


if __name__ == "__main__":
    areas = [int(a) for a in sys.argv[1:]] or [1, 2, 3, 4, 5, 6, 7]
    total = 0
    for a in areas:
        n = convert(a)
        if n:
            total += n
    print(f"total embedded music: {total} bytes ({total / 1024 / 1024:.2f} MB)")
