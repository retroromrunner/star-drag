#!/usr/bin/env python3
"""Convert the Star Drag per-area AI music tracks to DS-playable raw PCM,
EMBEDDED in the ARM9 binary (bin2o, audio/music_aN.bin) — v17.

WHY v17 (the designer, Nov 19, 2026): the 16-bit fix WORKED — "Yes much
clearer music great, but music volume too low in game fix that."
So the 16-bit path genuinely plays quieter on her phone (NooDS), not
just in melonDS: v16's title capture is ~25 dB quieter than v15's
(peak 0.117 / RMS -36.4 dBFS vs 0.62 / -11.8), i.e. the 16-bit music
plays at ~0.47x nominal in the emulators' SPU emulation while 8-bit
SFX play at full level. v12's 8-bit music (title RMS -13.5..-10.7
dBFS) was the level she called fine. Fix: raise the music level
~+9 dB WITHOUT going back to 8-bit — channel volume 80 -> 127
(+4.0 dB, in source/main.c MUSIC_VOL) plus shipped peak 0.42 -> 0.72
(+4.7 dB, here). That restores the music to roughly v12's perceived
level on her device. The v13 lesson still binds: music + SFX summing
over full scale = clipping, so the SFX come DOWN a tad in this build
too (see main.c) to keep headroom. Everything else is v16's recipe:
same windows, same 7.5 s loops at 22050 Hz, same s16-from-numpy path,
same fades. The ONLY changes vs v16 are the peak target here and the
channel/SFX volumes in main.c.

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
  - Target shipped peak 0.72 (v17: +4.7 dB over v16's 0.42)
    — the ONLY variable that changes vs v15 is bit depth.
  - The s16 bytes are written from numpy, NOT from ffmpeg's s16le
    output, so no format-path rescaling can sneak in (v15 found
    ffmpeg's MP3-decoded u8 path scales by 1/sqrt(2) = 0.707 vs the
    float chain). ffmpeg is used only to decode/resample to f32le;
    gain + fades + quantization happen here and are asserted below.

Sources: area 1 assets/music-v2.mp3 (85 s); areas 2-4 the v2
Speechma tracks; areas 5-7 assets/music-area{5,6,7}.mp3.
Output: audio/music_aN.bin — signed 16-bit PCM, mono, 22050 Hz.
Usage: python3 tools/convert_music_v17.py [N ...]
"""
import os
import subprocess
import sys

import numpy as np

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
WIN = 7.5           # seconds of loop per area (16-bit size budget)
RATE = 22050
GAIN_MAX = 1.6      # v17: quiet windows need gain > 1 to reach peak 0.72 (area 1 raw peak ~0.47 -> gain 1.52); the shipped-peak assert below is the guard, not this cap
PEAK_TARGET = 0.72  # v17: +4.7 dB over v16's 0.42 — music back to v12's level
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
    # inside a fade — that is the fade doing its job.
    assert 0.68 <= fpeak <= PEAK_TARGET + 0.005, \
        f"area {area}: shipped peak {fpeak:.2f} outside [0.68, 0.725]!"
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
