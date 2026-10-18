#!/usr/bin/env python3
"""Import original FTL star-map art (img/map/*) for the DS beacon and sector maps.

Zone backgrounds become dimmed opaque RGB555 panels; icons keep a 4-bit alpha
channel so the DS renderer can blend their glow over the map. Requires Pillow.
Output stays local (generated/map_art.h); do not redistribute.
"""
import argparse
import hashlib
import io
import json
from pathlib import Path
from PIL import Image, ImageEnhance, ImageOps
from import_ftl import PkgArchive

ZONE_SIZE = (256, 112)
ICONS = [  # name, source, box size (aspect-fit after trimming transparent padding)
    ('map_beacon', 'img/map/map_icon_diamond_yellow.png', 15),
    ('map_visited', 'img/map/map_icon_diamond_blue.png', 15),
    ('map_beacon_small', 'img/map/map_icon_diamond_yellow.png', 11),
    ('map_visited_small', 'img/map/map_icon_diamond_blue.png', 11),
    ('map_rebel', 'img/map/map_icon_triangle_red.png', 11),
    ('map_hazard', 'img/map/map_icon_hazard.png', 11),
    ('map_distress', 'img/map/map_icon_warning.png', 11),
    ('map_ship', 'img/map/map_icon_ship.png', 17),
    ('map_ship_small', 'img/map/map_icon_ship.png', 11),
    ('map_boss', 'img/map/map_icon_boss.png', 19),
    ('map_boss_small', 'img/map/map_icon_boss.png', 13),
    ('map_civilian', 'img/map/sector_legend_civilian.png', 9),
    ('map_hostile', 'img/map/sector_legend_hostile.png', 9),
    ('map_nebula_dot', 'img/map/sector_legend_nebula.png', 9),
    ('map_nebula', 'img/map/nebula_0.png', 48),
    ('map_nebula_b', 'img/map/nebula_2.png', 40),
]


def rgb555(r, g, b):
    return 0x8000 | (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10)


def emit(name, values, ctype, width):
    rows = [' '+','.join(values[i:i+width])+',' for i in range(0, len(values), width)]
    return ['static const %s %s[%d] = {' % (ctype, name, len(values))] + rows + ['};']


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--archive', type=Path)
    args = parser.parse_args()
    output = Path(__file__).resolve().parents[1] / 'generated'
    archive_path = args.archive or Path(json.loads((output / 'manifest.json').read_text())['archive'])
    archive = PkgArchive(archive_path)
    header = ['/* Converted original FTL star-map art from the local owner archive. Do not redistribute. */',
              '#ifndef FTL_DS_MAP_ART_H', '#define FTL_DS_MAP_ART_H', '#include <stdint.h>']
    meta, total, preview = [], 0, Image.new('RGBA', (256 * 3, 112 + 64), (0, 0, 0, 255))
    header.append('#define MAP_ZONE_COUNT 3')
    header.append('#define MAP_ZONE_WIDTH %d' % ZONE_SIZE[0])
    header.append('#define MAP_ZONE_HEIGHT %d' % ZONE_SIZE[1])
    zones = []
    for z in range(3):
        path = 'img/map/zone_%d.png' % (z + 1)
        payload = archive.read(path)
        image = Image.open(io.BytesIO(payload)).convert('RGB')
        image = ImageOps.fit(image, ZONE_SIZE, Image.Resampling.LANCZOS)
        image = ImageEnhance.Brightness(image).enhance(0.45)
        preview.paste(image, (z * 256, 0))
        values = ['0x%04x' % rgb555(*px) for px in image.get_flattened_data()]
        header += emit('map_zone_%d' % z, values, 'uint16_t', 16)
        zones.append('map_zone_%d' % z)
        total += len(values) * 2
        meta.append({'array': 'map_zone_%d' % z, 'source': path, 'sha256': hashlib.sha256(payload).hexdigest(),
                     'conversion': 'centre crop to 256x112, brightness 45%, opaque RGB555'})
    header.append('static const uint16_t *const map_zones[MAP_ZONE_COUNT] = {%s};' % ','.join(zones))
    x = 0
    for name, path, size in ICONS:
        payload = archive.read(path)
        image = Image.open(io.BytesIO(payload)).convert('RGBA')
        image = image.crop(image.getbbox())
        scale = min(size / image.width, size / image.height)
        image = image.resize((max(1, round(image.width * scale)), max(1, round(image.height * scale))), Image.Resampling.LANCZOS)
        colors, alpha = [], []
        for r, g, b, a in image.get_flattened_data():
            colors.append('0x%04x' % (rgb555(r, g, b) if a else 0))
            alpha.append(str(min(15, (a + 8) // 17)))
        header.append('#define %s_WIDTH %d' % (name, image.width))
        header.append('#define %s_HEIGHT %d' % (name, image.height))
        header += emit(name, colors, 'uint16_t', 16)
        header += emit(name + '_alpha', alpha, 'uint8_t', 32)
        total += len(colors) * 3
        preview.alpha_composite(image, (x, 120))
        x += image.width + 4
        meta.append({'array': name, 'source': path, 'sha256': hashlib.sha256(payload).hexdigest(),
                     'output_size': image.size, 'conversion': 'trim, aspect-fit, RGB555 + 4-bit alpha'})
    header += ['#define MAP_ART_TOTAL_BYTES %d' % total, '#endif', '']
    if total > 220 * 1024:
        raise ValueError('Map art exceeds assigned 220 KiB budget')
    (output / 'map_art.h').write_text('\n'.join(header), encoding='utf-8')
    (output / 'map_art.json').write_text(json.dumps({'total_bytes': total, 'assets': meta}, indent=2), encoding='utf-8')
    preview.resize((preview.width * 2, preview.height * 2), Image.Resampling.NEAREST).save(output / 'map_art_preview.png')
    print('Wrote generated/map_art.h (%d bytes)' % total)


if __name__ == '__main__':
    main()
