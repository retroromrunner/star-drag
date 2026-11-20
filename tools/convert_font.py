#!/usr/bin/env python3
"""Star Drag v13 — UI font tiles (the designer: no more white-on-black log text
on the touchpad).

Renders a 5x7-ish pixel font from DejaVu Sans Bold with PIL and emits
DS 4bpp tiles for the sub-screen BG text UI (area/weapon lines, boss
cues, game-over score lines, title hints) and a scale-2 version for
big banner text. Glyph pixels are palette index 1 (ink); index 0 is
transparent (shows the navy backdrop).

Charset (index 0 is the blank glyph):
  " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789+-:.!/'()"
Output (data/, embedded via bin2o):
  font1.bin — scale 1: 46 glyphs x 1 tile  (8x8 px each)
  font2.bin — scale 2: 46 glyphs x 4 tiles (16x16 px each, row-major:
              top-left, top-right, bottom-left, bottom-right)
The C side maps a char to its glyph index with font_glyph().
Run from ~/workspace/star-drag-ds/ :  python3 tools/convert_font.py
"""
import os
from PIL import Image, ImageDraw, ImageFont

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "data")
ROOT = os.path.normpath(ROOT)

CHARSET = " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789+-:.!/'()"
FONT_PATH = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"


def encode_4bpp(img):
    """img: PIL 'L' image, values 0/1 (index). DS-tiled 4bpp bytes."""
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


def render_glyph(ch, size, canvas):
    """Render one glyph into a canvas x canvas 'L' image (0/1).
    Renders at 3x and downsamples by taking the MAX ink in each 3x3
    block. Rendering directly at 7 px loses thin strokes: PIL's
    antialiasing thins a 1 px vertical bar to nothing, so '+' came
    out as '-' (caught in QA, Nov 19, 2026)."""
    F = 3
    img = Image.new("L", (canvas * F, canvas * F), 0)
    d = ImageDraw.Draw(img)
    font = ImageFont.truetype(FONT_PATH, size * F)
    # centre the glyph in the 3x canvas
    bbox = d.textbbox((0, 0), ch, font=font)
    w, h = bbox[2] - bbox[0], bbox[3] - bbox[1]
    d.text(((canvas * F - w) // 2 - bbox[0], (canvas * F - h) // 2 - bbox[1]),
           ch, font=font, fill=255)
    out = Image.new("L", (canvas, canvas), 0)
    op = out.load()
    ip = img.load()
    for y in range(canvas):
        for x in range(canvas):
            ink = any(ip[x * F + dx, y * F + dy] > 0
                      for dy in range(F) for dx in range(F))
            op[x, y] = 1 if ink else 0
    return out


def build(scale, size, canvas, out_name):
    """One tileset, glyph by glyph: scale 1 emits 1 tile per glyph,
    scale 2 emits 4 tiles per glyph (top-left, top-right, bottom-left,
    bottom-right) so the C side can address glyph g at tiles 4*g..4*g+3."""
    data = bytearray()
    for ch in CHARSET:
        if ch == " ":
            g = Image.new("L", (canvas, canvas), 0)
        else:
            g = render_glyph(ch, size, canvas)
        data += encode_4bpp(g)
    open(os.path.join(ROOT, out_name), "wb").write(bytes(data))
    per = 1 if scale == 1 else 4
    print(f"{out_name}: {len(data)} bytes, {len(CHARSET)} glyphs, "
          f"{per} tile(s) each, {canvas}x{canvas} px")


if __name__ == "__main__":
    os.makedirs(ROOT, exist_ok=True)
    build(1, 7, 8, "font1.bin")     # 8x8 glyphs for the sub UI
    build(2, 14, 16, "font2.bin")   # 16x16 glyphs for banner text
