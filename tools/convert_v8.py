#!/usr/bin/env python3
"""Star Drag v8 sprite pass (the designer, Nov 19, 2026 — Fix 4).

Regenerates the four weapon/fire-rate pickup sprites and the four
bottom-screen weapon buttons with a big readable letter/icon on each,
so the pickup itself (and its matching button) identifies the weapon
at a glance. Also rewrites the two pickup palettes that are owned by
a single sprite (pow_spread, pow_laser), patches three unused slots
of the shared missile palette for the orange MISSILE pickup, and
rewrites the four button palettes with saturated accent colours.

Palette ownership (why this is safe):
  - PAL_POW_SPREAD (13) is used ONLY by pow_spread.
  - PAL_POW_LASER  (14) is used ONLY by pow_laser.
  - PAL_MISSILE    (10) is shared by missile / missile2 / pow_miss.
    missile uses indices 1-9, missile2 uses 3,4,7,9,10 — so indices
    11-13 are free and are patched here to orange/dark/light-orange
    for the MISSILE pickup. NOTE: re-running tools/convert_sprites.py
    regenerates missile_pal.bin from the source art and would wipe
    indices 11-13 — re-run THIS script after it.
  - PAL_DRIFTER    (1) is shared, but pow_rate only needs its existing
    entries (1 white, 8-10 red-orange, 14 dark brown), so no change.

The v7 sprites this replaces were unidentifiable (laser and missile
were the same bolt shape in different colours; buttons were plain
coloured bars with no letters).

Output: data/pow_spread.bin/.pal, data/pow_laser.bin/.pal,
data/pow_miss.bin, data/pow_rate.bin, data/missile_pal.bin (patched),
data/btn_single.bin/.pal, data/btn_spread.bin/.pal,
data/btn_laser.bin/.pal, data/btn_miss.bin/.pal.
Run from ~/workspace/star-drag-ds/ :  python3 tools/convert_v8.py
"""
import os
from PIL import Image, ImageDraw

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "data")
ROOT = os.path.normpath(ROOT)

# ---------------------------------------------------------------- 4bpp --
def encode_4bpp(img):
    """img: PIL 'P'/'L' image with palette indices 0-15. Returns DS-tiled
    4bpp bytes (8x8 tiles, low nibble = even x)."""
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

def write_pal(name, colors):
    """colors: list of (r,g,b) for indices 0..15 (padded with black)."""
    cols = list(colors) + [(0, 0, 0)] * (16 - len(colors))
    raw = bytearray()
    for (r, g, b) in cols[:16]:
        v = rgb15(r, g, b)
        raw += bytes([v & 0xFF, (v >> 8) & 0xFF])
    open(os.path.join(ROOT, name + "_pal.bin"), "wb").write(bytes(raw))
    return bytes(raw)

def read_pal(name):
    raw = open(os.path.join(ROOT, name + "_pal.bin"), "rb").read()
    cols = []
    for i in range(16):
        v = raw[i * 2] | (raw[i * 2 + 1] << 8)
        cols.append(((v & 31) * 8, ((v >> 5) & 31) * 8, ((v >> 10) & 31) * 8))
    return cols

# ------------------------------------------------------------ 5x7 font --
FONT = {
    "S": ["01110", "10001", "10000", "01110", "00001", "10001", "01110"],
    "L": ["10000", "10000", "10000", "10000", "10000", "10000", "11111"],
    "M": ["10001", "11011", "10101", "10101", "10001", "10001", "10001"],
    "1": ["00100", "01100", "00100", "00100", "00100", "00100", "01110"],
}
# lightning bolt, 6 wide x 10 tall (fire-rate icon); '1' = pixel
BOLT = [
    "001100",
    "001100",
    "011000",
    "011000",
    "111110",
    "001100",
    "001100",
    "011000",
    "011000",
    "110000",
]
# small chevron ">" 3 wide x 5 tall (speed mark on the rate pickup)
CHEV = ["100", "010", "001", "010", "100"]

def draw_glyph(img, glyph, x0, y0, scale, color, outline=None):
    """Draw a bitmap glyph (list of '0'/'1' strings) scaled up."""
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

def rounded_rect(dr, box, rad, fill, outline=None, width=1):
    dr.rounded_rectangle(box, radius=rad, fill=fill,
                         outline=outline, width=width)

# ------------------------------------------------------------- pickups --
# palette indices used by the pickup sprites:
#   CAP = capsule fill, W = white letter, DARK = outline, HI = highlight
def make_pickup(name, cap_rgb, letter, kind):
    """kind: 'letter' (5x7 font) or 'bolt' (lightning + chevrons)."""
    img = Image.new("L", (32, 32), 0)
    dr = ImageDraw.Draw(img)
    # capsule: rounded square, dark outline, bright top edge
    rounded_rect(dr, [3, 3, 28, 28], 7, fill=1, outline=3, width=2)
    dr.line([6, 5, 25, 5], fill=4, width=2)  # top highlight
    if kind == "letter":
        draw_glyph(img, FONT[letter], 8, 5, 3, 2, outline=3)
    else:  # bolt + two speed chevrons along the bottom
        draw_glyph(img, BOLT, 10, 4, 2, 2, outline=3)
        draw_glyph(img, CHEV, 7, 24, 1, 2, outline=None)
        draw_glyph(img, CHEV, 12, 24, 1, 2, outline=None)
        draw_glyph(img, CHEV, 17, 24, 1, 2, outline=None)
    open(os.path.join(ROOT, name + ".bin"), "wb").write(encode_4bpp(img))
    print("wrote", name + ".bin")

# SPREAD — green capsule, white "S"
write_pal("pow_spread", [(0, 0, 0), (0, 200, 80), (248, 248, 248),
                         (0, 90, 35), (130, 255, 170)])
make_pickup("pow_spread", (0, 200, 80), "S", "letter")
# LASER — cyan capsule, white "L"
write_pal("pow_laser", [(0, 0, 0), (0, 210, 240), (248, 248, 248),
                        (0, 100, 120), (150, 240, 255)])
make_pickup("pow_laser", (0, 210, 240), "L", "letter")
# MISSILE — orange capsule, white "M" (uses patched slots 11-13 of the
# shared missile palette: 11 orange, 12 dark, 13 light orange).
cols = read_pal("missile")
cols[11] = (248, 120, 0)
cols[12] = (120, 50, 0)
cols[13] = (255, 180, 80)
write_pal("missile", cols)
# the missile pickup sprite uses palette indices 11/1/12/13, so draw it
# with a remapped index set: CAP=11, W=1, DARK=12, HI=13.
img = Image.new("L", (32, 32), 0)
dr = ImageDraw.Draw(img)
rounded_rect(dr, [3, 3, 28, 28], 7, fill=11, outline=12, width=2)
dr.line([6, 5, 25, 5], fill=13, width=2)
draw_glyph(img, FONT["M"], 8, 5, 3, 1, outline=12)
open(os.path.join(ROOT, "pow_miss.bin"), "wb").write(encode_4bpp(img))
print("wrote pow_miss.bin (missile palette slots 11-13 patched)")
# FIRE-RATE — red capsule (drifter palette idx 8), white bolt + chevrons.
# Uses PAL_DRIFTER as before: 1 white, 8 red-orange, 14 dark brown.
img = Image.new("L", (32, 32), 0)
dr = ImageDraw.Draw(img)
rounded_rect(dr, [3, 3, 28, 28], 7, fill=8, outline=14, width=2)
dr.line([6, 5, 25, 5], fill=1, width=1)
draw_glyph(img, BOLT, 10, 4, 2, 1, outline=14)
draw_glyph(img, CHEV, 7, 24, 1, 1, outline=None)
draw_glyph(img, CHEV, 12, 24, 1, 1, outline=None)
draw_glyph(img, CHEV, 17, 24, 1, 1, outline=None)
open(os.path.join(ROOT, "pow_rate.bin"), "wb").write(encode_4bpp(img))
print("wrote pow_rate.bin")

# ------------------------------------------------------------- buttons --
# 64x32 buttons: white body, saturated accent border + big accent letter.
BTN = [
    ("btn_single", "1", [(248, 248, 248)] * 11 +
     [(248, 80, 80), (248, 0, 0), (248, 0, 0), (180, 0, 0), (0, 0, 0)]),
    ("btn_spread", "S", [(248, 248, 248)] * 11 +
     [(248, 220, 80), (248, 200, 0), (248, 200, 0), (180, 140, 0), (0, 0, 0)]),
    ("btn_laser", "L", [(248, 248, 248)] * 11 +
     [(80, 200, 248), (0, 160, 248), (0, 160, 248), (0, 110, 180), (0, 0, 0)]),
    ("btn_miss", "M", [(248, 248, 248)] * 11 +
     [(80, 248, 120), (0, 200, 60), (0, 200, 60), (0, 140, 40), (0, 0, 0)]),
]
for name, letter, pal in BTN:
    # palette: index 0 transparent, 1-10 white body, 11-14 accent, 15 black
    cols = [(0, 0, 0)] + pal
    write_pal(name, cols)
    img = Image.new("L", (64, 32), 0)
    dr = ImageDraw.Draw(img)
    rounded_rect(dr, [2, 2, 61, 29], 5, fill=1, outline=13, width=2)
    draw_glyph(img, FONT[letter], 24, 5, 3, 13, outline=None)
    open(os.path.join(ROOT, name + ".bin"), "wb").write(encode_4bpp(img))
    print("wrote", name + ".bin/.pal")

print("convert_v8 done.")
