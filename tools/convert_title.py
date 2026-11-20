#!/usr/bin/env python3
"""Star Drag v13 — retro-arcade title / game-over art (the designer: a proper
title screen, big stylized title, starfield behind it, TAP TO START).

Renders big gradient text with PIL (DejaVu Sans Bold) and emits DS
4bpp tiles + a 32x32 tilemap + palettes, loaded onto the MAIN screen's
far starfield BG layer (bg_far, map 4) during ST_TITLE / ST_OVER.
The starfield near/mid layers keep scrolling behind it, so the title
sits over a live starfield. The map is swapped back to the saved
starfield map on start_game().

Palette 8 (title/game-over art): 0 transparent, 1 outline,
2 shadow, 3-12 gradient colours, 13 white.
Palette 9 (prompt line): 0 transparent, 1 yellow, 2 white, 3 orange.
The prompt lives on palette 9 so the code can pulse its brightness
without touching the title art.

Tile layout in main BG tile RAM base 0 (shared with the starfield
tiles 0-7): title tiles at tile 16, game-over tiles at tile 272.
Output (data/, embedded via bin2o):
  title.bin / title_map.bin / title_pal.bin
  gameover.bin / gameover_map.bin / gameover_pal.bin
Run from ~/workspace/star-drag-ds/ :  python3 tools/convert_title.py
"""
import os
from PIL import Image, ImageDraw, ImageFont

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "data")
ROOT = os.path.normpath(ROOT)
FONT = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"

# RGB15 palette entries as (r, g, b) 0..31
PAL8 = [
    (0, 0, 0),        # 0 transparent
    (4, 1, 8),        # 1 outline (near-black purple)
    (10, 3, 14),      # 2 drop shadow
    (31, 28, 6),      # 3 yellow
    (31, 22, 4),      # 4 gold
    (31, 14, 4),      # 5 orange
    (31, 8, 6),       # 6 red-orange
    (28, 4, 8),       # 7 red
    (28, 6, 22),      # 8 magenta
    (18, 8, 28),      # 9 violet
    (8, 26, 31),      # 10 cyan
    (8, 12, 31),      # 11 blue
    (14, 22, 31),     # 12 sky
    (31, 31, 31),     # 13 white
    (0, 0, 0), (0, 0, 0),
]
PAL9 = [
    (0, 0, 0),        # 0 transparent
    (31, 28, 8),      # 1 yellow
    (31, 31, 31),     # 2 white
    (31, 16, 4),      # 3 orange
] + [(0, 0, 0)] * 12


def rgb15(r, g, b):
    return (r & 31) | ((g & 31) << 5) | ((b & 31) << 10)


def lerp(a, b, t):
    return tuple(int(round(a[i] + (b[i] - a[i]) * t)) for i in range(3))


def nearest(pal, rgb):
    best, bd = 0, 1 << 30
    for i, c in enumerate(pal):
        if i == 0:
            continue
        d = sum((rgb[j] - c[j]) ** 2 for j in range(3))
        if d < bd:
            bd, best = d, i
    return best


def draw_line(img, text, y, size, stops, pal):
    """Draw one centred line: outline + shadow + vertical gradient.
    stops: list of palette colours (RGB15 0..31) from top to bottom."""
    d = ImageDraw.Draw(img)
    font = ImageFont.truetype(FONT, size)
    bbox = d.textbbox((0, 0), text, font=font)
    w, h = bbox[2] - bbox[0], bbox[3] - bbox[1]
    x0 = (256 - w) // 2 - bbox[0]
    y0 = y - bbox[1]
    # mask of the text (for the gradient fill)
    mask = Image.new("L", (256, 192), 0)
    md = ImageDraw.Draw(mask)
    # drop shadow
    md.text((x0 + 3, y0 + 4), text, font=font, fill=2)
    # outline: 8 neighbours in palette index 1
    for ox, oy in ((-2, 0), (2, 0), (0, -2), (0, 2),
                   (-2, -2), (2, -2), (-2, 2), (2, 2),
                   (-1, 0), (1, 0), (0, -1), (0, 1)):
        md.text((x0 + ox, y0 + oy), text, font=font, fill=1)
    # gradient fill over the mask's core (mask value 0 -> transparent,
    # 1 -> outline, 2 -> shadow are kept; the fill overwrites the
    # interior with gradient indices 3..12)
    mp = mask.load()
    ip = img.load()
    n = len(stops)    # fill: re-render the text per scanline with the gradient colour
    grad = Image.new("RGB", (256, 192), (0, 0, 0))
    gd = ImageDraw.Draw(grad)
    for yy in range(y0, min(192, y0 + h)):
        t = (yy - y0) / max(1, h - 1)
        seg = min(n - 2, int(t * (n - 1)))
        lt = t * (n - 1) - seg
        col = lerp(stops[seg], stops[seg + 1], lt)
        gd.line([(0, yy), (255, yy)], fill=tuple(c * 8 for c in col))
    # composite: gradient where mask is text, outline/shadow where set
    textmask = Image.new("L", (256, 192), 0)
    td = ImageDraw.Draw(textmask)
    td.text((x0, y0), text, font=font, fill=255)
    tm = textmask.load()
    gm = grad.load()
    for yy in range(192):
        for xx in range(256):
            if tm[xx, yy]:
                ip[xx, yy] = nearest(pal, tuple(v // 8 for v in gm[xx, yy]))
            elif mp[xx, yy] == 1:
                ip[xx, yy] = 1
            elif mp[xx, yy] == 2:
                ip[xx, yy] = 2


def encode_4bpp(img):
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


def build(name, lines, prompt, prompt_y, tile_base):
    """lines: [(text, y, size, stops)] on palette 8; prompt on palette 9.
    Returns (tiles, map, pal) and writes the files."""
    img = Image.new("L", (256, 192), 0)
    for text, y, size, stops in lines:
        draw_line(img, text, y, size, stops, PAL8)
    # prompt line (palette 9, yellow with white core handled by the
    # palette pulse in code — here just index 1)
    d = ImageDraw.Draw(img)
    font = ImageFont.truetype(FONT, 15)
    bbox = d.textbbox((0, 0), prompt, font=font)
    w = bbox[2] - bbox[0]
    x0 = (256 - w) // 2 - bbox[0]
    # prompt uses its own image so its pixels map to palette 9
    pimg = Image.new("L", (256, 192), 0)
    pd = ImageDraw.Draw(pimg)
    pd.text((x0, prompt_y), prompt, font=font, fill=1)
    # merge: prompt pixels overwrite (they never overlap the title)
    px, pp = img.load(), pimg.load()
    for yy in range(192):
        for xx in range(256):
            if pp[xx, yy]:
                px[xx, yy] = 0  # marker: remapped below by palette tag
    # Build tiles with dedupe keyed by (bytes, palette)
    tiles = {}
    order = []
    mapp = [0] * (32 * 32)
    for ty in range(24):
        for tx in range(32):
            pal = 9 if any(pp[tx * 8 + dx, ty * 8 + dy]
                           for dx in range(8) for dy in range(8)) else 8
            # extract this 8x8 tile's indices (prompt tiles read from
            # pimg, title tiles from img)
            tile = bytearray(32)
            for yy in range(8):
                for xx in range(8):
                    if pal == 9:
                        v = pp[tx * 8 + xx, ty * 8 + yy]
                    else:
                        v = px[tx * 8 + xx, ty * 8 + yy]
                    v = v & 15
                    off = yy * 4 + xx // 2
                    if xx % 2 == 0:
                        tile[off] = (tile[off] & 0xF0) | v
                    else:
                        tile[off] = (tile[off] & 0x0F) | (v << 4)
            if all(b == 0 for b in tile):
                mapp[ty * 32 + tx] = 0
                continue
            key = (bytes(tile), pal)
            if key not in tiles:
                tiles[key] = len(order)
                order.append(key)
            mapp[ty * 32 + tx] = (tile_base + tiles[key]) | (pal << 12)
    # rows 24..31 stay 0 (off-screen)
    tile_bytes = bytearray()
    for (tb, _pal) in order:
        tile_bytes += tb
    pal_bytes = bytearray()
    for c in PAL8 + PAL9:
        v = rgb15(*c)
        pal_bytes += bytes([v & 0xFF, (v >> 8) & 0xFF])
    open(os.path.join(ROOT, name + ".bin"), "wb").write(bytes(tile_bytes))
    open(os.path.join(ROOT, name + "_map.bin"), "wb").write(
        b"".join(int(e).to_bytes(2, "little") for e in mapp))
    open(os.path.join(ROOT, name + "_pal.bin"), "wb").write(bytes(pal_bytes))
    n = len(order)
    print(f"{name}: {n} tiles ({len(tile_bytes)} bytes), "
          f"tile base {tile_base}, end {tile_base + n}")
    assert tile_base + n <= 512, f"{name} tiles overflow VRAM"
    return n


if __name__ == "__main__":
    os.makedirs(ROOT, exist_ok=True)
    warm = [PAL8[3], PAL8[4], PAL8[5], PAL8[6], PAL8[7]]   # yellow->red
    cool = [PAL8[10], PAL8[12], PAL8[11], PAL8[8], PAL8[9]]  # cyan->violet
    n_title = build("title",
                    [("STAR", 22, 46, warm), ("DRAG", 84, 46, cool)],
                    "TAP TO START", 166, 16)
    build("gameover",
          [("GAME", 22, 46, warm), ("OVER", 84, 46, warm)],
          "TOUCH TO RETRY", 166, 16 + n_title + 8)
