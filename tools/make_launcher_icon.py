"""Build the local DS launcher icon from imported Kestrel artwork (Pillow)."""
from pathlib import Path
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[1]

def main():
    hull = Image.open(ROOT / 'generated/original/img/ship/kestral_base.png').convert('RGBA')
    hull = hull.crop(hull.getbbox())
    hull.thumbnail((30, 21), Image.Resampling.LANCZOS)
    canvas = Image.new('RGBA', (32, 32), (9, 18, 30, 255))
    draw = ImageDraw.Draw(canvas)
    draw.rounded_rectangle((0, 0, 31, 31), radius=3, outline=(58, 116, 142))
    for x, y in [(3, 9), (28, 8), (4, 27), (27, 28)]:
        draw.point((x, y), fill=(128, 166, 179))
    # Tiny purpose-drawn letters stay legible at native launcher resolution.
    for x, rows in [(9, ['111','100','110','100','100']),
                    (14, ['111','010','010','010','010']),
                    (19, ['100','100','100','100','111'])]:
        for y, row in enumerate(rows, 3):
            for dx, bit in enumerate(row):
                if bit == '1':
                    draw.point((x + dx, y), fill=(207, 236, 242))
    canvas.alpha_composite(hull, ((32-hull.width)//2, 10+(20-hull.height)//2))
    # Reserve palette zero for DS transparency; use at most 15 visible colors.
    indexed = canvas.convert('RGB').quantize(colors=15, dither=Image.Dither.NONE)
    icon = Image.new('P', (32, 32))
    icon.putpalette([255, 0, 255] + indexed.getpalette()[:45] + [0]*720)
    pixels = indexed.get_flattened_data() if hasattr(indexed, 'get_flattened_data') else indexed.getdata()
    icon.putdata([p+1 for p in pixels])
    icon.save(ROOT / 'generated/launcher_icon.gif', optimize=False, transparency=0)
    (ROOT / 'build').mkdir(exist_ok=True)
    icon.convert('RGBA').resize((256, 256), Image.Resampling.NEAREST).save(ROOT / 'build/launcher-icon-preview.png')
    print('Generated 32x32, 16-color DS banner icon from local Kestrel art.')

if __name__ == '__main__':
    main()
