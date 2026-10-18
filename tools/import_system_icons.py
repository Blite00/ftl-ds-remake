#!/usr/bin/env python3
"""Import the original FTL room system icons (img/icons/s_*_overlay) as DS tint masks.

The original draws one white overlay silhouette per system room and tints it by
state (green powered, orange damaged, red destroyed, grey unpowered, blue ion).
Each icon is stored as a 4-bit coverage mask so the DS renderer can apply the
same tints at two sizes. Requires Pillow. Output stays local; do not redistribute.
"""
import argparse
import hashlib
import io
import json
from pathlib import Path
from PIL import Image
from import_ftl import PkgArchive

# Order matches enum SystemIcon in source/render.c.
ICONS = [
    ('shields', 'img/icons/s_shields_overlay.png'),
    ('engines', 'img/icons/s_engines_overlay.png'),
    ('weapons', 'img/icons/s_weapons_overlay.png'),
    ('oxygen', 'img/icons/s_oxygen_overlay.png'),
    ('medbay', 'img/icons/s_medbay_overlay.png'),
    ('pilot', 'img/icons/s_pilot_overlay.png'),
    ('doors', 'img/icons/s_doors_overlay2.png'),
    ('sensors', 'img/icons/s_sensors_overlay.png'),
    ('drones', 'img/icons/s_drones_overlay.png'),
    ('teleporter', 'img/icons/s_teleporter_overlay.png'),
    ('artillery', 'img/icons/s_artillery.png'),
    ('cloaking', 'img/icons/s_cloaking_overlay.png'),
]
SIZES = (12, 7)


def mask(image, size):
    image = image.crop(image.getbbox())
    scale = min(size / image.width, size / image.height)
    target = (max(1, round(image.width * scale)), max(1, round(image.height * scale)))
    image = image.resize(target, Image.Resampling.LANCZOS)
    canvas = Image.new('RGBA', (size, size))
    canvas.alpha_composite(image, ((size - image.width) // 2, (size - image.height) // 2))
    values = []
    for r, g, b, a in canvas.get_flattened_data():
        lum = max(r, g, b)
        v = a * lum // 255
        values.append(min(15, (v + 8) // 16))
    return values


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--archive', type=Path)
    args = parser.parse_args()
    output = Path(__file__).resolve().parents[1] / 'generated'
    archive_path = args.archive or Path(json.loads((output / 'manifest.json').read_text())['archive'])
    archive = PkgArchive(archive_path)
    header = ['/* Converted original FTL system icons from the local owner archive. Do not redistribute. */',
              '#ifndef FTL_DS_SYSTEM_ICONS_H', '#define FTL_DS_SYSTEM_ICONS_H', '#include <stdint.h>',
              '#define SYSTEM_ICON_COUNT %d' % len(ICONS)]
    metadata = []
    preview = Image.new('RGBA', (len(ICONS) * 16, 32), (20, 30, 36, 255))
    for size in SIZES:
        header.append('#define SYSTEM_ICON_%d %d' % (size, size))
        header.append('static const uint8_t system_icons_%d[SYSTEM_ICON_COUNT][%d] = {' % (size, size * size))
        for i, (name, path) in enumerate(ICONS):
            payload = archive.read(path)
            image = Image.open(io.BytesIO(payload)).convert('RGBA')
            values = mask(image, size)
            header.append('    {' + ','.join(str(v) for v in values) + '}, /* %s */' % name)
            if size == SIZES[0]:
                metadata.append({'icon': name, 'source': path, 'source_sha256': hashlib.sha256(payload).hexdigest(),
                                 'source_size': image.size, 'sizes': list(SIZES)})
            for j, v in enumerate(values):
                if v:
                    preview.putpixel((i * 16 + j % size + 2, (0 if size == 12 else 18) + j // size + 2),
                                     (100, 255, 100, 255) if v > 7 else (60, 150, 60, 255))
        header.append('};')
    header.extend(['#endif', ''])
    (output / 'system_icons.h').write_text('\n'.join(header), encoding='utf-8')
    (output / 'system_icons.json').write_text(json.dumps({'conversion': 'Trim, aspect-fit to 12x12 and 7x7, '
                                                          '4-bit coverage = alpha x luminance; tinted at runtime.',
                                                          'icons': metadata}, indent=2), encoding='utf-8')
    preview.resize((preview.width * 4, preview.height * 4), Image.Resampling.NEAREST).save(output / 'system_icons_preview.png')
    print('Wrote generated/system_icons.h (%d icons)' % len(ICONS))


if __name__ == '__main__':
    main()
