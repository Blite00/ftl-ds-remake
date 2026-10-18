#!/usr/bin/env python3
"""Import original FTL UI art for the local DS build. Requires Pillow."""
import argparse
import hashlib
import io
import json
from pathlib import Path
from PIL import Image, ImageEnhance, ImageOps
from import_ftl import PkgArchive
from convert_ship_art import c_bitmap, fit_canvas


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--archive', type=Path)
    args = parser.parse_args()
    output = Path(__file__).resolve().parents[1] / 'generated'
    archive_path = args.archive or Path(json.loads((output/'manifest.json').read_text())['archive'])
    archive = PkgArchive(archive_path)
    sources = {
        'ui_space': 'img/stars/bg_darknebula.png',
        'ui_fuel': 'img/ui_icons/icon_fuel.png',
        'ui_missile': 'img/ui_icons/icon_missiles.png',
        'ui_scrap': 'img/ui_icons/icon_scrap.png',
        'ui_medbay': 'img/icons/s_medbay.png',
    }
    header = ['/* Converted original FTL art from local owner archive. Do not redistribute. */',
              '#ifndef FTL_DS_UI_ART_H', '#define FTL_DS_UI_ART_H', '#include <stdint.h>']
    metadata = []
    total = 0
    previews = {}
    for name, path in sources.items():
        payload = archive.read(path)
        image = Image.open(io.BytesIO(payload)).convert('RGBA')
        source_size = image.size
        if name == 'ui_space':
            image = ImageOps.fit(image, (256,192), Image.Resampling.LANCZOS)
            image = ImageEnhance.Brightness(image).enhance(0.25)
            image.putalpha(255)
            conversion = 'Centered aspect-preserving crop to 256x192; RGB brightness 25%; opaque.'
        else:
            image = image.crop(image.getbbox())
            image = fit_canvas(image, (14,14))
            conversion = 'Trim transparent padding; fit centered 14x14, preserve aspect and original color.'
        header.extend(c_bitmap(name, image))
        previews[name] = image
        size = image.width*image.height*2
        total += size
        metadata.append({'array': name, 'source': path, 'source_sha256': hashlib.sha256(payload).hexdigest(),
                         'source_size': source_size, 'output_size': image.size,
                         'rgb555_bytes': size, 'conversion': conversion})
    header.extend(['#define UI_ART_TOTAL_BYTES %d' % total, '#endif', ''])
    if total > 120*1024:
        raise ValueError('UI art exceeds assigned 120 KiB budget')
    (output/'ui_art.h').write_text('\n'.join(header), encoding='utf-8')
    (output/'ui_art.json').write_text(json.dumps({'total_bytes':total,'assets':metadata},indent=2),encoding='utf-8')
    preview = previews['ui_space'].copy()
    for i,name in enumerate(('ui_fuel','ui_missile','ui_scrap','ui_medbay')):
        preview.alpha_composite(previews[name], (8+i*24,8))
    preview.save(output/'ui_art_preview.png')
    print('Imported 5 original UI assets: %d RGB555 bytes (%.2f KiB)' % (total,total/1024))


if __name__ == '__main__':
    main()
