#!/usr/bin/env python3
"""Convert Kenney Space Shooter Redux PNGs to DS 4bpp sprite tiles + palettes.

For each sprite: scale to fit the target canvas (kept aspect, centred),
quantize opaque pixels to 15 colours (index 0 = transparent), emit:
  data/<name>.bin  - 4bpp tiles, 8x8, low nibble = left pixel, tiles row-major
  data/<name>.pal  - 16 x u16 LE RGB15 palette, entry 0 = 0x0000 (transparent)
Usage: python3 tools/convert_sprites.py
"""
import os
from PIL import Image

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PACK = os.path.join(ROOT, 'assets', 'kenney_space-shooter-redux', 'PNG')
OUT = os.path.join(ROOT, 'data')

# v2 (horizontal): ships rotated -90 deg so they face left/right along the
# flight axis; everything smaller than v1 (32px canvases). Ship/enemy
# source art points up/down in the Kenney pack; PIL rotate(-90) is
# clockwise: up -> right, down -> left.
# name, source png (relative to PACK), canvas w, canvas h, fit w, fit h, rot
SPRITES = [
    ('player',     'playerShip1_blue.png',          32, 32, 30, 24, -90),
    ('drifter',    'Enemies/enemyRed1.png',         32, 32, 28, 26, -90),
    ('weaver',     'Enemies/enemyGreen3.png',       32, 32, 28, 26, -90),
    ('diver',      'Enemies/enemyBlue4.png',        32, 32, 28, 26, -90),
    ('shooter',    'Enemies/enemyBlack2.png',       32, 32, 30, 26, -90),
    ('asteroid',   'Meteors/meteorBrown_big1.png',  32, 32, 30, 26,   0),
    ('mine',       'ufoRed.png',                    32, 32, 28, 28,   0),
    ('turret',     'ufoGreen.png',                  32, 32, 28, 28,   0),
    ('bullet_p',   'Lasers/laserBlue01.png',        32,  8, 30,  6, -90),
    ('bullet_e',   'Lasers/laserRed01.png',         32,  8, 30,  6, -90),
    ('missile',    'Lasers/laserBlue08.png',        32,  8, 30,  6, -90),
    ('boom0',      'Effects/fire04.png',            32, 32, 24, 24,   0),
    ('boom1',      'Effects/fire04.png',            32, 32, 28, 28,   0),
    ('boom2',      'Effects/fire04.png',            32, 32, 30, 30,   0),
    ('reticle',    'Power-ups/star_gold.png',       16, 16, 16, 16,   0),
    ('btn_spread', 'UI/buttonYellow.png',           64, 32, 60, 28,   0),
    ('btn_laser',  'UI/buttonBlue.png',             64, 32, 60, 28,   0),
    ('btn_miss',   'UI/buttonGreen.png',            64, 32, 60, 28,   0),
    ('btn_single', 'UI/buttonRed.png',              64, 32, 60, 28,   0),
    # v3: 7 more enemy types (14 total). pal_from = quantize against an
    # existing sprite's palette, so the new sprite REUSES that palette
    # slot (main sprite palette has only 16 slots; see main.c).
    ('bug',        'Enemies/enemyGreen2.png',       32, 32, 28, 26, -90, 'weaver'),
    ('alien',      'Enemies/enemyBlue1.png',        32, 32, 28, 26, -90, 'diver'),
    ('charger',    'Enemies/enemyRed5.png',         32, 32, 28, 26, -90, 'drifter'),
    ('spinner',    'Enemies/enemyBlack1.png',       32, 32, 28, 26, -90, 'shooter'),
    ('orbiter',    'Enemies/enemyBlack5.png',       32, 32, 28, 26, -90, 'shooter'),
    ('crystal',    'Meteors/meteorGrey_big2.png',   32, 32, 30, 26,   0, 'asteroid'),
    ('capital',    'Enemies/enemyBlack3.png',       32, 32, 30, 26, -90, 'shooter'),
    # v3 missile: a real rocket-shaped dart (enemyBlue5), not a laser bolt
    ('missile2',   'Enemies/enemyBlue5.png',        32,  8, 30,  6, -90, 'diver'),
    # v3 pickups: distinct shapes/colours per weapon
    ('pow_spread', 'Power-ups/powerupYellow_star.png', 32, 32, 28, 28, 0),
    ('pow_laser',  'Power-ups/powerupGreen_bolt.png',  32, 32, 28, 28, 0),
    ('pow_miss',   'Power-ups/powerupBlue_bolt.png',  32, 32, 28, 28, 0, 'missile'),
    # v3 trail spark (star, tinted by the boom fire palette)
    ('spark',      'Effects/star1.png',             16, 16, 14, 14,   0, 'boom0'),
    # v4: 18 more enemy types (32 total), all reusing existing palettes.
    # Area-unique sets are chosen in main.c; pal_from keeps us inside
    # the 16 main sprite palette slots.
    ('dart',       'Enemies/enemyRed2.png',         32, 32, 28, 26, -90, 'drifter'),
    ('rogue',      'playerShip3_red.png',           32, 32, 28, 26, -90, 'drifter'),
    ('ufo',        'ufoBlue.png',                   32, 32, 28, 28,   0, 'diver'),
    ('meteor2',    'Meteors/meteorBrown_med1.png',  32, 32, 28, 26,   0, 'asteroid'),
    ('drone',      'Enemies/enemyBlack4.png',       32, 32, 28, 26, -90, 'shooter'),
    ('sentry',     'ufoYellow.png',                 32, 32, 28, 28,   0, 'shooter'),
    ('wasp',       'Enemies/enemyGreen4.png',       32, 32, 28, 26, -90, 'weaver'),
    ('beetle',     'Enemies/enemyGreen1.png',       32, 32, 28, 26, -90, 'weaver'),
    ('bat',        'Enemies/enemyGreen5.png',       32, 32, 28, 26, -90, 'weaver'),
    ('fish',       'Enemies/enemyBlue3.png',        32, 32, 28, 26, -90, 'diver'),
    ('jelly',      'Enemies/enemyBlue2.png',        32, 32, 28, 26, -90, 'diver'),
    ('shard',      'Meteors/meteorGrey_med2.png',   32, 32, 28, 26,   0, 'asteroid'),
    ('blob',       'Meteors/meteorBrown_small1.png', 32, 32, 26, 24,   0, 'asteroid'),
    ('crawler',    'Enemies/enemyRed3.png',         32, 32, 28, 26, -90, 'drifter'),
    ('eel',        'Enemies/enemyRed4.png',         32, 32, 28, 26, -90, 'drifter'),
    ('prism',      'Meteors/meteorGrey_big3.png',   32, 32, 30, 26,   0, 'asteroid'),
    ('spore',      'Meteors/meteorBrown_tiny1.png', 32, 32, 22, 20,   0, 'asteroid'),
    ('hunter',     'playerShip2_orange.png',        32, 32, 28, 26, -90, 'shooter'),
    # v4 bosses: 64x64, distinct big sprites
    ('boss',       'Enemies/enemyBlack3.png',       64, 64, 60, 52, -90, 'shooter'),
    ('boss2',      'Enemies/enemyGreen2.png',       64, 64, 60, 52, -90, 'weaver'),
    # v4 fire-rate pickup (red bolt) + mini player ship (lives icons)
    ('pow_rate',   'Power-ups/powerupRed_bolt.png', 32, 32, 28, 28,   0, 'drifter'),
    ('player_mini', 'playerShip1_blue.png',         16, 16, 15, 12, -90, 'player'),
]

# v3 procedural sprites (src=None): built in code, not from a PNG.
# laserbeam: 64x8 bright cyan beam, white core, cyan edges.
# terrain_<area>: 32x32 solid block for the terrain BG, all sharing one
# 15-colour palette (slot PAL_TERRAIN) with per-area colours at fixed
# indices: 1=base, 2=metal, 3=planet, 4=cave, 5=crystal, 6=water, 7=goo,
# 8=light edge, 9=dark edge, 10=speckle.
TERRAIN_PAL = [(0, 0, 0),
    (120, 120, 130),   # 1 base metal grey
    (190, 120, 60),    # 2 planet rust
    (110, 80, 55),     # 3 cave brown
    (80, 220, 240),    # 4 crystal cyan
    (40, 110, 220),    # 5 water blue
    (90, 200, 90),     # 6 goo green
    (60, 60, 70),      # 7 dark (unused spare)
    (220, 230, 240),   # 8 light edge
    (40, 40, 50),      # 9 dark edge
    (255, 255, 255),   # 10 speckle white
    (0, 0, 0), (0, 0, 0), (0, 0, 0), (0, 0, 0), (0, 0, 0)]
TERRAIN_AREAS = {'base': 1, 'planet': 2, 'cave': 3, 'crystal': 4,
                 'water': 5, 'goo': 6}
PROCEDURAL = ['laserbeam'] + ['terrain_' + a for a in TERRAIN_AREAS]


def pal15_from_rgb(rgb_list):
    return [0] + [rgb15(r, g, b) for (r, g, b) in rgb_list[:15]]


def write_bin_pal(name, idx, pal, cw, ch):
    out = bytearray()
    for ty in range(0, ch, 8):
        for tx in range(0, cw, 8):
            for y in range(8):
                for x in range(0, 8, 2):
                    lo = idx[ty + y][tx + x]
                    hi = idx[ty + y][tx + x + 1]
                    out.append(lo | (hi << 4))
    with open(os.path.join(OUT, name + '.bin'), 'wb') as f:
        f.write(out)
    with open(os.path.join(OUT, name + '_pal.bin'), 'wb') as f:
        pal16 = (pal + [0] * 16)[:16]
        for p in pal16:
            f.write(bytes([p & 0xFF, (p >> 8) & 0xFF]))
    print(f'{name}: {cw}x{ch} tiles={len(out)}B pal={len(pal)} colours')


def make_procedural(name):
    if name == 'laserbeam':
        cw, ch = 32, 8
        # rows: 0,7 transparent; 1,6 cyan edge; 2,5 bright cyan; 3,4 white core
        pal = pal15_from_rgb([(0, 255, 255), (255, 255, 255), (0, 200, 220)])
        idx = [[0] * cw for _ in range(ch)]
        for x in range(cw):
            idx[1][x] = 1
            idx[2][x] = 3
            idx[3][x] = 2
            idx[4][x] = 2
            idx[5][x] = 3
            idx[6][x] = 1
        write_bin_pal(name, idx, pal, cw, ch)
        return
    if name.startswith('terrain_'):
        area = name[len('terrain_'):]
        ci = TERRAIN_AREAS[area]
        cw, ch = 8, 8
        pal = pal15_from_rgb(TERRAIN_PAL[1:])
        idx = [[ci] * cw for _ in range(ch)]
        for x in range(cw):
            idx[0][x] = 8          # light top edge
            idx[7][x] = 9          # dark bottom edge
        idx[3][3] = 10             # speckle
        idx[4][5] = 10
        write_bin_pal(name, idx, pal, cw, ch)
        return
    raise ValueError(name)


# v4 terrain obstacles: sprite-based, unique per area, 32x32 each.
# (name, shape, base RGB colour, pal_from)
OBSTACLES = [
    ('obs_pipe',    'pipe',       (150, 150, 160), 'shooter'),
    ('obs_column',  'column',     (170, 170, 180), 'shooter'),
    ('obs_tree',    'tree',       (60, 180, 80),   'weaver'),
    ('obs_root',    'root',       (150, 100, 50),  'asteroid'),
    ('obs_stalact', 'stalactite', (140, 120, 100), 'asteroid'),
    ('obs_stalagm', 'stalagmite', (140, 120, 100), 'asteroid'),
    ('obs_cryspike','cryspike',   (80, 220, 240),  'asteroid'),
    ('obs_claw',    'claw',       (80, 220, 240),  'asteroid'),
    ('obs_plant',   'plant',      (40, 200, 120),  'weaver'),
    ('obs_tube',    'tube',       (60, 140, 230),  'diver'),
    ('obs_barb',    'barb',       (120, 220, 90),  'weaver'),
    ('obs_blob',    'blob',       (90, 200, 90),   'weaver'),
]


def make_obstacle(name, shape, rgb, pal_from):
    from PIL import ImageDraw
    cw = ch = 32
    im = Image.new('RGBA', (cw, ch), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    r, g, b = rgb
    dark = (max(0, r - 60), max(0, g - 60), max(0, b - 60))
    lite = (min(255, r + 60), min(255, g + 60), min(255, b + 60))
    if shape == 'pipe':          # vertical metal pipe, hanging or standing
        d.rectangle([11, 0, 21, 31], fill=dark + (255,))
        d.rectangle([13, 0, 19, 31], fill=(r, g, b, 255))
        d.rectangle([9, 6, 23, 10], fill=lite + (255,))
        d.rectangle([9, 20, 23, 24], fill=lite + (255,))
    elif shape == 'column':      # stone column with caps
        d.rectangle([10, 0, 22, 31], fill=(r, g, b, 255))
        d.rectangle([7, 0, 25, 5], fill=lite + (255,))
        d.rectangle([7, 26, 25, 31], fill=lite + (255,))
        d.rectangle([12, 8, 14, 24], fill=dark + (255,))
    elif shape == 'tree':        # trunk + canopy (floor plant)
        d.rectangle([14, 16, 18, 31], fill=(120, 80, 40, 255))
        d.ellipse([4, 0, 28, 20], fill=(r, g, b, 255))
        d.ellipse([10, 4, 22, 16], fill=lite + (255,))
    elif shape == 'root':        # thick curved root hanging down
        for i, (x, y, w) in enumerate([(8, 0, 7), (12, 6, 7), (16, 12, 6),
                                       (14, 18, 6), (18, 24, 5)]):
            d.ellipse([x, y, x + w, y + 8], fill=(r, g, b, 255))
        d.ellipse([16, 28, 22, 32], fill=lite + (255,))
    elif shape == 'stalactite':  # triangle, wide at top, point at bottom
        d.polygon([(4, 0), (28, 0), (16, 31)], fill=(r, g, b, 255))
        d.polygon([(12, 0), (20, 0), (16, 18)], fill=lite + (255,))
        d.ellipse([8, 0, 12, 6], fill=dark + (255,))
        d.ellipse([20, 0, 24, 6], fill=dark + (255,))
    elif shape == 'stalagmite':  # triangle, point at top
        d.polygon([(16, 0), (4, 31), (28, 31)], fill=(r, g, b, 255))
        d.polygon([(16, 12), (12, 31), (20, 31)], fill=lite + (255,))
    elif shape == 'cryspike':    # tall crystal spike
        d.polygon([(10, 0), (22, 0), (18, 31), (14, 31)], fill=(r, g, b, 255))
        d.polygon([(14, 0), (18, 0), (16, 28)], fill=lite + (255,))
        d.polygon([(22, 4), (28, 12), (22, 20)], fill=dark + (255,))
    elif shape == 'claw':        # curved hook
        d.polygon([(6, 0), (14, 0), (12, 16), (20, 26), (16, 31),
                   (10, 22), (6, 12)], fill=(r, g, b, 255))
        d.polygon([(8, 2), (12, 2), (11, 14)], fill=lite + (255,))
    elif shape == 'plant':       # wavy fronds from the floor
        for x0 in (6, 14, 22):
            d.line([(x0, 31), (x0 - 3, 18), (x0 + 2, 8)], fill=(r, g, b, 255), width=3)
            d.ellipse([x0 - 4, 4, x0 + 4, 12], fill=lite + (255,))
    elif shape == 'tube':        # vertical tube with rings
        d.rectangle([12, 0, 20, 31], fill=(r, g, b, 255))
        for y in (4, 13, 22):
            d.ellipse([10, y, 22, y + 6], fill=lite + (255,))
    elif shape == 'barb':        # spike with barb hooks
        d.polygon([(16, 0), (10, 31), (22, 31)], fill=(r, g, b, 255))
        d.polygon([(10, 14), (16, 18), (10, 22)], fill=lite + (255,))
        d.polygon([(22, 14), (16, 18), (22, 22)], fill=lite + (255,))
    elif shape == 'blob':        # slime blob with drips
        d.ellipse([4, 8, 28, 28], fill=(r, g, b, 255))
        d.ellipse([8, 4, 18, 14], fill=lite + (255,))
        d.ellipse([10, 26, 14, 32], fill=(r, g, b, 255))
        d.ellipse([20, 26, 24, 32], fill=(r, g, b, 255))
    # quantize against the pal_from palette (same helper path as convert)
    with open(os.path.join(OUT, pal_from + '_pal.bin'), 'rb') as f:
        raw = f.read()
    pal = [0]
    pal_rgb = []
    for i in range(15):
        v = raw[(i + 1) * 2] | (raw[(i + 1) * 2 + 1] << 8)
        pal.append(v)
        pal_rgb.append(((v & 31) << 3, ((v >> 5) & 31) << 3, ((v >> 10) & 31) << 3))
    idx = [[0] * cw for _ in range(ch)]
    px = im.load()
    for y in range(ch):
        for x in range(cw):
            if px[x, y][3] >= 128:
                r2, g2, b2 = px[x, y][:3]
                best, bd = 1, 1 << 30
                for i, (pr, pg, pb) in enumerate(pal_rgb):
                    dd = (r2 - pr) ** 2 + (g2 - pg) ** 2 + (b2 - pb) ** 2
                    if dd < bd:
                        bd, best = dd, i + 1
                idx[y][x] = best
    write_bin_pal(name, idx, pal, cw, ch)


def rgb15(r, g, b):
    return (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10)


def convert(name, src, cw, ch, fw, fh, rot=0, pal_from=None):
    im = Image.open(os.path.join(PACK, src)).convert('RGBA')
    if rot:
        im = im.rotate(rot, resample=Image.BICUBIC, expand=True)
    if name.startswith('boom'):
        # crop the flame to its solid core before scaling, so it fills the
        # 32x32 explosion frame instead of sitting small in the corner
        alpha = im.split()[3]
        xs, ys = [], []
        apx = alpha.load()
        for y in range(im.height):
            for x in range(im.width):
                if apx[x, y] >= 90:
                    xs.append(x); ys.append(y)
        if xs:
            im = im.crop((max(0, min(xs) - 1), max(0, min(ys) - 1),
                          min(im.width, max(xs) + 2), min(im.height, max(ys) + 2)))
    # scale to fit (thumbnail() never enlarges, so use resize for both)
    sc = min(fw / im.width, fh / im.height)
    im = im.resize((max(1, round(im.width * sc)), max(1, round(im.height * sc))),
                   Image.LANCZOS)
    if name.startswith('boom'):
        # the flame is a thin vertical streak; composite it at 0/90/180/270
        # degrees so the explosion reads as a radial fireball, not a candle
        comp = Image.new('RGBA', (cw, ch), (0, 0, 0, 0))
        for ang in (0, 90, 180, 270):
            r = im.rotate(ang, resample=Image.BICUBIC, expand=False) \
                if ang else im
            comp.paste(r, ((cw - r.width) // 2, (ch - r.height) // 2), r)
        im = comp
    canvas = Image.new('RGBA', (cw, ch), (0, 0, 0, 0))
    canvas.paste(im, ((cw - im.width) // 2, (ch - im.height) // 2),
                 im)
    if pal_from:
        # v3: quantize against an EXISTING sprite's palette so this
        # sprite reuses that palette slot (only 16 main sprite palettes).
        # pal_from names a sprite already converted (data/<name>_pal.bin).
        with open(os.path.join(OUT, pal_from + '_pal.bin'), 'rb') as f:
            raw = f.read()
        pal = [0]
        pal_rgb = []
        for i in range(15):
            v = raw[(i + 1) * 2] | (raw[(i + 1) * 2 + 1] << 8)
            r, g, b = (v & 31) << 3, ((v >> 5) & 31) << 3, ((v >> 10) & 31) << 3
            pal.append(v)
            pal_rgb.append((r, g, b))
        idx = [[0] * cw for _ in range(ch)]
        px = canvas.load()
        for y in range(ch):
            for x in range(cw):
                if px[x, y][3] >= 128:
                    r, g, b = px[x, y][:3]
                    best, bd = 1, 1 << 30
                    for i, (pr, pg, pb) in enumerate(pal_rgb):
                        d = (r - pr) ** 2 + (g - pg) ** 2 + (b - pb) ** 2
                        if d < bd:
                            bd, best = d, i + 1
                    idx[y][x] = best
        write_bin_pal(name, idx, pal, cw, ch)
        return
    # quantize opaque pixels to 15 colours
    rgb = Image.new('RGB', (cw, ch), (0, 0, 0))
    alpha = canvas.split()[3]
    px = canvas.load()
    rgbpx = rgb.load()
    for y in range(ch):
        for x in range(cw):
            if px[x, y][3] >= 128:
                rgbpx[x, y] = px[x, y][:3]
    q = rgb.quantize(colors=15, method=Image.MEDIANCUT)
    pal_rgb = (q.getpalette() or [])[:15 * 3]
    pal_rgb = pal_rgb + [0] * (15 * 3 - len(pal_rgb))
    # index map: pixel -> 0 transparent, else q index + 1
    idx = [[0] * cw for _ in range(ch)]
    qpx = q.load()
    for y in range(ch):
        for x in range(cw):
            if px[x, y][3] >= 128:
                idx[y][x] = qpx[x, y] + 1
    # palette: 16 x RGB15, entry 0 transparent
    pal = [0]
    for i in range(15):
        r, g, b = pal_rgb[i * 3:i * 3 + 3]
        pal.append(rgb15(r, g, b))
    if name.startswith('boom'):
        # fire04 is a pale-blue exhaust flame; re-tint by luminance into
        # a white-yellow -> orange -> deep-red fireball ramp so kills read
        # as explosions on the DS screen.
        ramp = [(255, 240, 170), (255, 214, 100), (255, 190, 50),
                (255, 165, 20), (255, 140, 0), (248, 115, 0),
                (232, 90, 0), (208, 75, 0), (184, 58, 0), (160, 42, 0),
                (136, 33, 0), (112, 25, 0), (88, 16, 0), (64, 8, 0),
                (40, 0, 0)]
        pal = [0] + [rgb15(r, g, b) for (r, g, b) in ramp]
        for y in range(ch):
            for x in range(cw):
                r, g, b, a = px[x, y]
                if a >= 90:
                    lum = (r * 3 + g * 5 + b * 2) // 10
                    lvl = min(14, (lum * a // 255) * 15 // 256)
                    idx[y][x] = 1 + lvl
                else:
                    idx[y][x] = 0
    # tiles: 8x8, row-major, low nibble = left pixel
    out = bytearray()
    for ty in range(0, ch, 8):
        for tx in range(0, cw, 8):
            for y in range(8):
                for x in range(0, 8, 2):
                    lo = idx[ty + y][tx + x]
                    hi = idx[ty + y][tx + x + 1]
                    out.append(lo | (hi << 4))
    with open(os.path.join(OUT, name + '.bin'), 'wb') as f:
        f.write(out)
    with open(os.path.join(OUT, name + '_pal.bin'), 'wb') as f:
        for p in pal:
            f.write(bytes([p & 0xFF, (p >> 8) & 0xFF]))
    print(f'{name}: {cw}x{ch} tiles={len(out)}B pal={len(pal)} colours')


def main():
    os.makedirs(OUT, exist_ok=True)
    for s in SPRITES:
        convert(*s)
    for name in PROCEDURAL:
        make_procedural(name)
    for name, shape, rgb, pal_from in OBSTACLES:
        make_obstacle(name, shape, rgb, pal_from)


if __name__ == '__main__':
    main()
