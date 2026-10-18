from pathlib import Path
from PIL import Image
root = Path(__file__).resolve().parents[1]
for source in (root/'build').glob('*-v71.ppm'):
    image = Image.open(source)
    assert image.size == (256,384)
    image.resize((512,768),Image.Resampling.NEAREST).save(source.with_suffix('.png'))
print('V44 host framebuffer PNG previews written under build/.')
