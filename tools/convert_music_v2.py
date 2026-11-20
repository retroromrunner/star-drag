#!/usr/bin/env python3
"""Convert the Star Drag v2 AI music track to DS-playable raw PCM.

Source: assets/music-v2.mp3 — 85 s instrumental, generated on Speechma
with the R-Type / Life Force synthwave chiptune prompt, no sign-in,
commercial use stated. (The earlier sites musiccreator.ai, onlymusic.ai
and freebeat.ai all failed on login / human-check — do not retry them.)

Output: audio/music_raw.bin — unsigned 8-bit PCM, mono, 22050 Hz, the
same format the v1 synth loop used (see tools/make_audio.py), but at
22050 Hz instead of 11025 so the produced track keeps its highs.
main.c plays it at 22050 Hz, looped. SFX stay at 11025 Hz.

A 0.13 s fade-out at the end keeps the 85 s loop point from clicking.
The v1 synthesized loop is preserved as audio/music_synth_v1_raw.bin.

Usage: python3 tools/convert_music_v2.py
"""
import os
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, 'assets', 'music-v2.mp3')
DST = os.path.join(ROOT, 'audio', 'music_raw.bin')

cmd = ['ffmpeg', '-y', '-v', 'error', '-i', SRC,
       '-ac', '1', '-ar', '22050',
       '-af', 'volume=0.9,afade=t=out:st=84.9:d=0.13',
       '-f', 'u8', DST]
r = subprocess.run(cmd)
if r.returncode != 0:
    sys.exit(r.returncode)
n = os.path.getsize(DST)
print(f'{DST}: {n} bytes, {n / 22050:.2f}s at 22050 Hz mono 8-bit')
