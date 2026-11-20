#!/usr/bin/env python3
"""Star Drag v7: convert NEW web-sourced sprites (not Kenney ships) to DS
4bpp tiles + palettes, reusing tools/convert_sprites.py's convert().

Sources (all free, credited in README.md and the title screen):
- alien2/alien2b: "alienSpriteSheet" by rrodi411, CC0, lpc.opengameart.org
  (green alien creature, 2 walk frames, facing right)
- jelly2/jelly2b: "Jellyfish" by StendhalGame, CC-BY-SA 4.0, opengameart.org
  (purple jellyfish, 2 pulse frames) — own palette slot (PAL_JELLY=15)
- wormg/wormr/wormb: "32x32 Worms Pack", CC-BY-SA 4.0, opengameart.org
- eye: "creeperEyeBlue" (CC0, opengameart.org, "creepy-thing" by ??? —
  see README); boss3 uses creeperEyesGreen (same pack, CC0)
- robot / boss4: "Green Robot Enemy" (CC0, opengameart.org), background
  chroma-keyed out before slicing
- tent: PROCEDURAL tentacled orb drawn here (same style as v4 obstacles)
- player_up: player ship facing UP (Kenney art, rot=0) for the v7
  upward-boss mode; laserbeam_v: vertical laser beam (procedural)
Usage: python3 tools/convert_v7.py
"""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import convert_sprites as cs
from PIL import Image, ImageDraw

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
WEB = os.path.join(ROOT, 'assets', 'web-sprites')

# (name, src, cw, ch, fw, fh, rot, pal_from) — pal_from=None = own palette
WEB_SPRITES = [
    ('wormg',   'wormg.png',   32, 32, 28, 26, 0, 'weaver'),
    ('wormr',   'wormr.png',   32, 32, 28, 26, 0, 'drifter'),
    ('wormb',   'wormb.png',   32, 32, 28, 26, 0, 'diver'),
    ('alien2',  'alien2.png',  32, 32, 28, 26, 0, 'weaver'),
    ('alien2b', 'alien2b.png', 32, 32, 28, 26, 0, 'weaver'),
    ('jelly2',  'jelly2.png',  32, 32, 28, 26, 0, None),
    ('jelly2b', 'jelly2b.png', 32, 32, 28, 26, 0, 'jelly2'),
    ('eye',     'eye.png',     32, 32, 28, 26, 0, 'diver'),
    ('robot',   'robot.png',   32, 32, 28, 26, 0, 'shooter'),
    ('boss3',   'boss3.png',   64, 64, 60, 52, 0, 'weaver'),
    ('boss4',   'boss4.png',   64, 64, 60, 52, 0, 'shooter'),
]


def make_tent():
    """Procedural tentacled orb (purple, like a floating sea creature)."""
    cw = ch = 32
    im = Image.new('RGBA', (cw, ch), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    # tentacles first (behind the orb)
    for x0, dx in ((8, -4), (13, -1), (18, 2), (23, 5)):
        d.line([(x0, 18), (x0 + dx, 31)], fill=(150, 80, 200, 255), width=3)
    d.ellipse([6, 4, 26, 24], fill=(170, 100, 220, 255))
    d.ellipse([10, 7, 22, 19], fill=(220, 160, 255, 255))
    d.ellipse([11, 9, 15, 13], fill=(40, 0, 60, 255))
    d.ellipse([18, 9, 22, 13], fill=(40, 0, 60, 255))
    im.save(os.path.join(WEB, '_tent.png'))
    cs.PACK = WEB
    cs.convert('tent', '_tent.png', 32, 32, 30, 30, 0, 'diver')


def make_laserbeam_v():
    """Vertical laser beam: 8x32, same cyan/white palette as laserbeam."""
    cw, ch = 8, 32
    pal = cs.pal15_from_rgb([(0, 255, 255), (255, 255, 255), (0, 200, 220)])
    idx = [[0] * cw for _ in range(ch)]
    for y in range(ch):
        idx[y][1] = 1
        idx[y][2] = 3
        idx[y][3] = 2
        idx[y][4] = 2
        idx[y][5] = 3
        idx[y][6] = 1
    cs.write_bin_pal('laserbeam_v', idx, pal, cw, ch)


def main():
    os.makedirs(cs.OUT, exist_ok=True)
    # web sprites: point convert() at the web-sprites folder
    cs.PACK = WEB
    for s in WEB_SPRITES:
        cs.convert(*s)
    make_tent()
    make_laserbeam_v()
    # player_up: Kenney ship art, facing UP (rot=0 — the art points up)
    cs.PACK = cs.os.path.join(ROOT, 'assets', 'kenney_space-shooter-redux', 'PNG')
    cs.convert('player_up', 'playerShip1_blue.png', 32, 32, 30, 24, 0, 'player')


if __name__ == '__main__':
    main()
