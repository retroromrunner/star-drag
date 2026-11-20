#!/usr/bin/env python3
"""Synthesize Star Drag audio as raw unsigned 8-bit PCM (11025 Hz mono).

  NOTE (v2): the music loop is no longer used — v2 plays the AI track
  (assets/music-v2.mp3, Speechma) via tools/convert_music_v2.py, and
  audio/music_raw.bin is the converted track. The synth loop below is
  kept as a fallback (preserved as audio/music_synth_v1_raw.bin).
  music.raw - 8 s loop, 120 BPM, Am-F-C-G: square bass (8ths) + square
              arpeggio (16ths) + noise hat (8ths). Seam crossfaded.
  shoot.raw - 0.09 s square blip, 990 -> 520 Hz sweep.
  boom.raw  - 0.45 s noise burst + 90 Hz thump, exponential decay.
  hit.raw   - 0.18 s square 160 -> 70 Hz + noise.
Output: audio/*.raw, played by libnds soundPlaySample (SoundFormat_8Bit).
"""
import math
import os
import random

RATE = 11025
OUT = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                   'audio')


def sq(phase):
    return 1.0 if (phase % 1.0) < 0.5 else -1.0


def note_freq(midi):
    return 440.0 * 2.0 ** ((midi - 69) / 12.0)


def write(name, samples):
    # samples: floats in [-1, 1] -> unsigned 8-bit
    data = bytes(max(0, min(255, int(128 + 120 * s))) for s in samples)
    with open(os.path.join(OUT, name.replace('.raw', '_raw.bin')), 'wb') as f:
        f.write(data)
    print(name, len(data), 'bytes', f'{len(data) / RATE:.2f}s')


def music():
    secs = 8.0
    n = int(RATE * secs)
    out = [0.0] * n
    # Am F C G, one bar (2 s) each. MIDI: A2=45 F2=41 C3=48 G2=43
    roots = [45, 41, 48, 43]
    chords = [[57, 60, 64], [53, 57, 60], [55, 60, 64], [55, 59, 62]]
    rng = random.Random(7)
    noise = [rng.uniform(-1, 1) for _ in range(n)]
    for i in range(n):
        t = i / RATE
        bar = int(t / 2.0) % 4
        # bass: 8th notes (0.25 s), root, square, medium
        bt = (t % 0.25)
        bass = sq(t * note_freq(roots[bar])) * (1.0 - bt * 2.5)
        # arpeggio: 16th notes (0.125 s), cycle chord tones + octave
        at = (t % 0.125)
        step = int(t / 0.125) % 6
        chord = chords[bar]
        arp_note = chord[step % 3] + (12 if step >= 3 else 0)
        arp = sq(t * note_freq(arp_note)) * 0.45 * (1.0 - at * 4.0)
        # hat: short noise tick on 8ths
        ht = (t % 0.25)
        hat = noise[i] * 0.25 if ht < 0.02 else 0.0
        out[i] = bass * 0.5 + arp * 0.5 + hat
    # crossfade the seam: last 0.1 s blends into the first 0.1 s
    fade = int(RATE * 0.1)
    for i in range(fade):
        a = i / fade
        out[n - fade + i] = out[n - fade + i] * (1 - a) + out[i] * a
    peak = max(abs(s) for s in out) or 1.0
    # v2: write the synth loop under its own name so re-running this
    # script never overwrites the AI track in audio/music_raw.bin.
    write('music_synth.raw', [s / peak * 0.9 for s in out])


def sweep(f0, f1, dur, vol, kind='square'):
    n = int(RATE * dur)
    out = []
    phase = 0.0
    for i in range(n):
        t = i / RATE
        f = f0 + (f1 - f0) * (i / n)
        phase += f / RATE
        env = 1.0 - i / n
        v = sq(phase) if kind == 'square' else math.sin(2 * math.pi * phase)
        out.append(v * env * vol)
    return out


def noise_burst(dur, vol, thump=0.0):
    n = int(RATE * dur)
    rng = random.Random(3)
    out = []
    for i in range(n):
        env = math.exp(-6.0 * i / n)
        v = rng.uniform(-1, 1) * env * vol
        if thump:
            v += math.sin(2 * math.pi * 90 * i / RATE) * env * thump
        out.append(v)
    return out


def laser():
    # v3: unique laser zap — square sweep 1500 -> 180 Hz with a fast
    # tremolo, 0.28 s. Nothing like the regular 990 -> 520 Hz pew.
    n = int(RATE * 0.28)
    out = []
    phase = 0.0
    for i in range(n):
        t = i / RATE
        f = 1500 + (180 - 1500) * (i / n)
        phase += f / RATE
        env = (1.0 - i / n) ** 1.5
        trem = 0.6 + 0.4 * math.sin(2 * math.pi * 55 * t)
        out.append(sq(phase) * env * trem * 0.8)
    return out


def pickup():
    # v16: pleasant chime arp — SINE notes C5/E5/G5/C6 (not square: the
    # v3 square version was a harsh zap). Soft 2nd harmonic, fast
    # exponential decay, 0.055 s per note, peak ~0.55. v17: played at
    # vol 100 (was 120); worst case music 0.72 + pickup 0.43 = 1.15
    # if peaks align — measured pinning is the real check.
    out = []
    for freq in (523.25, 659.25, 783.99, 1046.50):
        m = int(RATE * 0.055)
        phase = 0.0
        for i in range(m):
            phase += freq / RATE
            env = math.exp(-5.0 * i / m)
            v = math.sin(2 * math.pi * phase) + 0.3 * math.sin(4 * math.pi * phase)
            out.append(v * env * 0.42)
    return out


def bosshit():
    # v16: boss-damage tick — short sine sweep 330 -> 200 Hz, 0.07 s,
    # peak ~0.45. Distinct from hit.raw (160->70 Hz square + noise,
    # 0.18 s): higher, shorter, cleaner — a tick you feel per hit.
    # Throttled to one per 6 frames in damage_boss() so rapid fire
    # stays a rhythm, not a wall of noise. Played at vol 80.
    n = int(RATE * 0.07)
    out = []
    phase = 0.0
    for i in range(n):
        f = 330 + (200 - 330) * (i / n)
        phase += f / RATE
        env = math.exp(-8.0 * i / n)
        out.append(math.sin(2 * math.pi * phase) * env * 0.45)
    return out


def playerdmg():
    # v17: PLAYER-damage sound — "you got hit". Distinct from the
    # boss-damage tick (330->200 Hz sine, 0.07 s, clean) and from the
    # old hit.raw (160->70 Hz square + noise, 0.18 s, which was the
    # player-hit sound until v17): a LOW descending buzz — sine
    # 280 -> 70 Hz plus a 2nd harmonic and a short noise burst at the
    # attack, 0.28 s, peak ~0.65. Longer, lower and rougher than the
    # tick, so it reads as damage to the player, not a hit on the
    # boss. Played at vol 100 (v17's lowered-SFX rebalance).
    n = int(RATE * 0.28)
    rng = random.Random(11)
    out = []
    phase = 0.0
    for i in range(n):
        f = 280 + (70 - 280) * (i / n)
        phase += f / RATE
        env = math.exp(-4.0 * i / n)
        v = math.sin(2 * math.pi * phase) \
            + 0.5 * math.sin(4 * math.pi * phase)
        if i < int(RATE * 0.03):
            v += rng.uniform(-1, 1) * 0.6
        out.append(v * env * 0.42)
    return out




def alarm():
    # v5: boss warning alarm — three alternating 880/440 Hz square
    # beeps, 0.48 s total. Played 3x during the boss intro.
    out = []
    for freq in (880, 440, 880):
        m = int(RATE * 0.16)
        phase = 0.0
        for i in range(m):
            phase += freq / RATE
            out.append(sq(phase) * 0.7 * (1.0 - i / m))
    return out

def main():
    os.makedirs(OUT, exist_ok=True)
    music()
    write('shoot.raw', sweep(990, 520, 0.09, 0.7))
    write('boom.raw', noise_burst(0.45, 0.9, 0.6))
    write('hit.raw', [a + b for a, b in zip(
        sweep(160, 70, 0.18, 0.6), noise_burst(0.18, 0.35))])
    write('laser.raw', laser())
    write('pickup.raw', pickup())
    write('bosshit.raw', bosshit())
    write('playerdmg.raw', playerdmg())
    write('alarm.raw', alarm())


if __name__ == '__main__':
    main()
