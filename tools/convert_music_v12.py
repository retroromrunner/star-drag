#!/usr/bin/env python3
"""Convert the Star Drag v12 per-area AI music tracks to DS-playable raw PCM,
EMBEDDED in the ARM9 binary (bin2o, audio/music_aN.bin) — v10-style.

WHY EMBEDDED (v12 root-cause fix): v11 moved the 7 tracks into NitroFS
(nitro/music/areaN.raw) with a chunked streaming loader. The music plays
correctly in melonDS (verified by audio capture: bright, centroid ~800 Hz),
but the designer hears NO music at all on her iPhone in NooDS — only the shooting
SFX. v10's music was EMBEDDED and she heard it fine. The v11 music system
fails silently on NooDS (fopen returns NULL, the game continues without
music), so the NitroFS path is not reliable on her device. v12 reverts to
the proven v10 architecture: tracks embedded in the ARM9 binary, played
with soundPlaySample at full volume (80) — no file I/O, no streaming, no
volume-0 start. The 7 unique tracks are kept, as shorter 18 s loops so
they fit the ARM9 ELF (area 1 at 22050 Hz, areas 2-7 at 16000 Hz).

Sources (same as v11, never regenerated):
  area 1: assets/music-v2.mp3 — the original 85 s Speechma track.
  areas 2-7: assets/music-area2.mp3 ... music-area7.mp3.

Output: audio/music_aN.bin — unsigned 8-bit PCM, mono. Each is an 18 s
loop taken from the START of the source (0 s, includes the bright intro)
with a short 0.1 s fade-in (avoids a click without muting the intro)
and a 0.5 s fade-out at the end so the loop point does not click.

Usage: python3 tools/convert_music_v12.py [N ...]
  With no arguments, converts areas 1-7. With arguments, only those.
"""
import os
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SLICE = 18.0       # seconds of loop per area (fits the ARM9 ELF)
SKIP = 0.0         # start at the beginning — the intro is the audible part
FADE_IN = 0.1      # short fade-in: no click, intro stays audible
FADE_OUT = 0.5     # fade-out at the loop seam
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
    dst = os.path.join(ROOT, 'audio', f'music_a{area}.bin')
    if not os.path.exists(src):
        print(f'area {area}: MISSING {src} — skipping')
        return None
    os.makedirs(os.path.dirname(dst), exist_ok=True)
    dur = probe_duration(src)
    length = min(SLICE, dur)
    rate = RATE[area]
    fade_out_start = max(0.0, length - FADE_OUT)
    cmd = ['ffmpeg', '-y', '-v', 'error',
           '-ss', str(SKIP), '-t', str(length), '-i', src,
           '-ac', '1', '-ar', str(rate),
           '-af', f'volume=0.9,afade=t=in:st=0:d={FADE_IN},'
                  f'afade=t=out:st={fade_out_start}:d={FADE_OUT}',
           '-f', 'u8', dst]
    r = subprocess.run(cmd)
    if r.returncode != 0:
        sys.exit(r.returncode)
    n = os.path.getsize(dst)
    print(f'area {area}: {dst}: {n} bytes, {n / rate:.2f}s '
          f'at {rate} Hz mono 8-bit (source {dur:.1f}s, slice '
          f'{SKIP:.0f}s..{SKIP + length:.0f}s)')
    return n


if __name__ == '__main__':
    areas = [int(a) for a in sys.argv[1:]] or [1, 2, 3, 4, 5, 6, 7]
    total = 0
    for a in areas:
        n = convert(a)
        if n:
            total += n
    print(f'total embedded music: {total} bytes ({total / 1024 / 1024:.2f} MB)')
