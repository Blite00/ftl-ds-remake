#!/usr/bin/env python3
"""Import original FTL window/menu art for the V54 original-style DS UI.

- ui_hex_tile: the hex texture inside img/window_base.png, darkened, as an 8x8 tile
  that panel interiors repeat.
- ui_drone_icon: the drone-parts icon from img/statusUI/top_drones_on.png (14x14).
- ui_menu_back: img/main_menus/main_base2.png (title art with the FTL logo), 256x192.
Frame borders, glow and chamfers are drawn in the original colours at DS scale.
Requires Pillow. Output stays local (generated/ftl_ui_art.h); do not redistribute.
"""
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
    archive = PkgArchive(args.archive or Path(json.loads((output / 'manifest.json').read_text())['archive']))
    header = ['/* Converted original FTL UI art from the local owner archive. Do not redistribute. */',
              '#ifndef FTL_DS_UI_FRAMES_H', '#define FTL_DS_UI_FRAMES_H', '#include <stdint.h>']
    meta = []

    def source(path):
        payload = archive.read(path)
        meta.append({'source': path, 'sha256': hashlib.sha256(payload).hexdigest()})
        return Image.open(io.BytesIO(payload)).convert('RGBA')

    window = source('img/window_base.png')
    tile = window.crop((17, 20, 49, 52)).resize((8, 8), Image.Resampling.LANCZOS).convert('RGB')
    tile = ImageEnhance.Brightness(tile).enhance(0.42)
    tile = tile.convert('RGBA')
    header += c_bitmap('ui_hex_tile', tile)
    # Resource icons: the mint icon square at the left of each original top-bar box.
    for name, path in (('ui_res_fuel', 'img/statusUI/top_fuel_on.png'), ('ui_res_missiles', 'img/statusUI/top_missiles_on.png'),
                       ('ui_res_drones', 'img/statusUI/top_drones_on.png'), ('ui_res_scrap', 'img/statusUI/top_scrap.png')):
        box = source(path)
        mint = [(x, y) for y in range(box.height) for x in range(box.width // 2)
                if box.getpixel((x, y))[3] > 200 and min(box.getpixel((x, y))[:3]) > 200]
        xs, ys = [x for x, _ in mint], [y for _, y in mint]
        square = box.crop((min(xs), min(ys), max(xs) + 1, max(ys) + 1)).convert('RGBA')
        square = square.resize((max(1, round(square.width * 15 / square.height)), 15), Image.Resampling.LANCZOS)
        # Keep the teal glyph legible at 15 px: anything darker than the mint square
        # becomes the original's deep teal.
        square.putdata([(r, g, b, a) if min(r, g, b) > 205 else (18, 72, 82, a) for r, g, b, a in square.get_flattened_data()])
        header += c_bitmap(name, square)
        icon = square
    back = source('img/main_menus/main_base2.png').convert('RGB')
    # Letterbox the 16:9 title art so both the fleet and the FTL logo stay visible.
    back = back.resize((256, 144), Image.Resampling.LANCZOS)
    frame = Image.new('RGB', (256, 192), (6, 8, 12))
    frame.paste(back, (0, 24))
    back = frame.convert('RGBA')
    header += c_bitmap('ui_menu_back', back)
    header += ['#endif', '']
    (output / 'ftl_ui_art.h').write_text('\n'.join(header), encoding='utf-8')
    (output / 'ftl_ui_art.json').write_text(json.dumps({'assets': meta}, indent=2), encoding='utf-8')
    preview = Image.new('RGBA', (256 + 40, 192), (0, 0, 0, 255))
    preview.paste(back, (0, 0))
    preview.alpha_composite(tile.resize((32, 32), Image.Resampling.NEAREST), (260, 0))
    preview.alpha_composite(icon.resize((28, 28), Image.Resampling.NEAREST), (260, 40))
    preview.resize((preview.width * 2, preview.height * 2), Image.Resampling.NEAREST).save(output / 'ftl_ui_art_preview.png')
    print('Wrote generated/ftl_ui_art.h')


if __name__ == '__main__':
    main()
