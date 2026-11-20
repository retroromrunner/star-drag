#!/usr/bin/env python3
"""Star Drag v16 — 1UP pickup sprite (the designer, Nov 19, 2026).

Gold capsule with a hot-pink heart and white "1UP" text, clearly
identifiable like the other labelled pickups (S/L/M/bolt).

Palette: all 16 main-sprite palette slots are taken (slot 15 is
PAL_JELLY since v7), so — like v8's missile pickup — the 1UP shares
PAL_MISSILE and patches its two free indices:
  missile palette 14 -> gold (248,200,0)   [capsule]
  missile palette 15 -> hot pink (255,60,180) [heart]
Reused indices: 1 white (text), 12 dark brown (outline),
13 light orange (highlight). Missile sprites use 0-10 and the
missile pickup uses 11/1/12/13, so 14/15 are free.

WARNING (same as v8): re-running tools/convert_sprites.py
regenerates missile_pal.bin from the source art and WIPES indices
11-15 — re-run tools/convert_v8.py AND this script after it.

Output: data/pow_1up.bin, data/missile_pal.bin (patched 14/15)
Run from ~/workspace/star-drag-ds/ :  python3 tools/convert_v16_1up.py
"""
import os
from PIL import Image, ImageDraw

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "data")
ROOT = os.path.normpath(ROOT)


def encode_4bpp(img):
    """img: PIL 'L' image with palette indices 0-15. DS-tiled 4bpp bytes
    (8x8 tiles, low nibble = even x)."""
    w, h = img.size
    px = img.load()
    data = bytearray((w * h) // 2)
    for y in range(h):
        for x in range(w):
            idx = px[x, y] & 15
            tile = (y // 8) * (w // 8) + (x // 8)
            off = tile * 32 + (y % 8) * 4 + (x % 8) // 2
            if x % 2 == 0:
                data[off] = (data[off] & 0xF0) | idx
            else:
                data[off] = (data[off] & 0x0F) | (idx << 4)
    return bytes(data)


def rgb15(r, g, b):
    return (min(r, 248) // 8) | ((min(g, 248) // 8) << 5) | ((min(b, 248) // 8) << 10)


def draw_glyph(img, glyph, x0, y0, scale, color, outline=None):
    px = img.load()
    h = len(glyph)
    w = len(glyph[0])
    if outline is not None:
        for dy in range(h):
            for dx in range(w):
                if glyph[dy][dx] != "1":
                    continue
                for ox, oy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    for sy in range(scale):
                        for sx in range(scale):
                            px[x0 + (dx * scale + sx) + ox,
                               y0 + (dy * scale + sy) + oy] = outline
    for dy in range(h):
        for dx in range(w):
            if glyph[dy][dx] != "1":
                continue
            for sy in range(scale):
                for sx in range(scale):
                    px[x0 + dx * scale + sx, y0 + dy * scale + sy] = color


# 5x7 font for "1UP" (same style as the v8 pickup letters)
FONT = {
    "1": ["00100", "01100", "00100", "00100", "00100", "00100", "01110"],
    "U": ["10001", "10001", "10001", "10001", "10001", "10001", "01110"],
    "P": ["11110", "10001", "10001", "11110", "10000", "10000", "10000"],
}
# heart, 9 wide x 8 tall
HEART = [
    "011001100",
    "111111111",
    "111111111",
    "111111111",
    "011111110",
    "001111100",
    "000111000",
    "000010000",
]

# patch the shared missile palette: 14 gold, 15 hot pink
pal_path = os.path.join(ROOT, "missile_pal.bin")
raw = bytearray(open(pal_path, "rb").read())
for idx, (r, g, b) in {14: (248, 200, 0), 15: (255, 60, 180)}.items():
    v = rgb15(r, g, b)
    raw[idx * 2] = v & 0xFF
    raw[idx * 2 + 1] = (v >> 8) & 0xFF
open(pal_path, "wb").write(bytes(raw))
print("patched missile_pal.bin indices 14 (gold), 15 (hot pink)")

# sprite indices in the missile palette: CAP=14 gold, HEART=15 pink,
# DARK=12, HI=13, TEXT=1 white
img = Image.new("L", (32, 32), 0)
dr = ImageDraw.Draw(img)
dr.rounded_rectangle([3, 3, 28, 28], radius=7, fill=14, outline=12, width=2)
dr.line([6, 5, 25, 5], fill=13, width=2)
draw_glyph(img, HEART, 11, 6, 1, 15, outline=12)
draw_glyph(img, FONT["1"], 7, 20, 1, 1, outline=12)
draw_glyph(img, FONT["U"], 13, 20, 1, 1, outline=12)
draw_glyph(img, FONT["P"], 19, 20, 1, 1, outline=12)
open(os.path.join(ROOT, "pow_1up.bin"), "wb").write(encode_4bpp(img))
print("wrote pow_1up.bin")
