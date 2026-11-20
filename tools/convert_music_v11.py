#!/usr/bin/env python3
"""Convert the Star Drag v11 per-area AI music tracks to DS-playable raw PCM.

Sources:
  area 1: assets/music-v2.mp3 — the original 85 s Speechma track,
          kept as-is, never regenerated; looped here as a 36 s slice.
  areas 2-7: assets/music-area2.mp3 ... music-area7.mp3 — unique
          instrumental tracks generated on Speechma (same synthwave
          chiptune prompt style as the area-1 track, adapted per
          area), no sign-in, commercial use stated.

Output: nitro/music/areaN.raw — unsigned 8-bit PCM, mono: area 1 at
22050 Hz, areas 2-7 at 16000 Hz. Each is a ~36 s loop taken from
8 s into the source (skips any intro swell) with a 0.5 s fade-in
at the start and a 0.5 s fade-out at the end so the loop point
does not click. If a source is shorter than 44 s, the whole file
is used instead.

WHY NITROFS, NOT EMBEDDED: the ARM9 ELF has a hard 3.5 MB limit —
7 embedded tracks (4.2 MB of audio) overflow the link. The tracks
live in the ROM filesystem instead (top Makefile: NITRO_FILES :=
nitro) and main.c loads them into a double RAM buffer at area
changes. The old embedded bins (audio/music_raw.bin,
audio/music_areaN_raw.bin) are NOT used by v11 — do not recreate
them; tools/convert_music_v2.py is kept only as the v2 record.

PLACEHOLDERS: until the real Speechma tracks arrive, the slots
assets/music-area2.mp3 ... music-area7.mp3 may be COPIES of
music-v2.mp3 — the converter works the same on them, but the
resulting ROM is a TEST build only, not the v11 deliverable.

Usage: python3 tools/convert_music_v11.py [N ...]
  With no arguments, converts areas 1-7. With arguments, only those.
"""
import os
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SLICE = 36.0       # seconds of loop per area
SKIP = 8.0         # seconds skipped at the start of the source
FADE = 0.5         # fade in/out at the loop seam
RATE = {1: 22050, 2: 16000, 3: 16000, 4: 16000,
        5: 16000, 6: 16000, 7: 16000}


def probe_duration(path):
    r = subprocess.run(['ffprobe', '-v', 'error', '-show_entries',
                        'format=duration', '-of', 'csv=p=0', path],
                       capture_output=True, text=True)
    try:
        return float(r.stdout.strip())
    except ValueError:
        return 0.0


def convert(area):
    src = (os.path.join(ROOT, 'assets', 'music-v2.mp3') if area == 1
           else os.path.join(ROOT, 'assets', f'music-area{area}.mp3'))
    dst = os.path.join(ROOT, 'nitro', 'music', f'area{area}.raw')
    if not os.path.exists(src):
        print(f'area {area}: MISSING {src} — skipping')
        return None
    os.makedirs(os.path.dirname(dst), exist_ok=True)
    dur = probe_duration(src)
    if dur > SKIP + SLICE + 1:
        start, length = SKIP, SLICE
    else:
        start, length = 0.0, dur
    rate = RATE[area]
    fade_out_start = max(0.0, length - FADE)
    cmd = ['ffmpeg', '-y', '-v', 'error',
           '-ss', str(start), '-t', str(length), '-i', src,
           '-ac', '1', '-ar', str(rate),
           '-af', f'volume=0.9,afade=t=in:st=0:d={FADE},'
                  f'afade=t=out:st={fade_out_start}:d={FADE}',
           '-f', 'u8', dst]
    r = subprocess.run(cmd)
    if r.returncode != 0:
        sys.exit(r.returncode)
    n = os.path.getsize(dst)
    print(f'area {area}: {dst}: {n} bytes, {n / rate:.2f}s '
          f'at {rate} Hz mono 8-bit (source {dur:.1f}s, slice '
          f'{start:.0f}s..{start + length:.0f}s)')
    return n


if __name__ == '__main__':
    areas = [int(a) for a in sys.argv[1:]] or [1, 2, 3, 4, 5, 6, 7]
    total = 0
    for a in areas:
        n = convert(a)
        if n:
            total += n
    print(f'total nitro music: {total} bytes ({total / 1024 / 1024:.2f} MB)')
