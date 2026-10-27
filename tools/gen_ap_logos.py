#!/usr/bin/env python3
"""Draw the three Archipelago item tiers from the Archipelago logo (data/icon.png in ArchipelagoMW/Archipelago,
MIT licence) and write them as PNGs:

  APLogoProgression.png   logo + yellow up arrow   (progression items)
  APLogoUseful.png        plain logo               (useful / normal items)
  APLogoFiller.png        grayscale logo           (filler)

Usage: python gen_ap_logos.py <Archipelago>/data/icon.png OUT_DIR [size=128]
Then: python utxwriter.py HP3AP.utx APLogoProgression=OUT_DIR/APLogoProgression.png ...
"""
import os, sys
from PIL import Image, ImageDraw, ImageOps


def arrow(size):
    """Yellow up arrow with a dark outline, drawn at 4x and scaled down for smooth edges."""
    S = size * 4; im = Image.new('RGBA', (S, S), (0, 0, 0, 0)); d = ImageDraw.Draw(im)
    cx = S / 2
    pts = [(cx, S * 0.10), (S * 0.86, S * 0.52), (S * 0.64, S * 0.52), (S * 0.64, S * 0.90),
           (S * 0.36, S * 0.90), (S * 0.36, S * 0.52), (S * 0.14, S * 0.52)]
    d.polygon(pts, fill=(40, 30, 0, 255))
    inner = [(x + (cx - x) * 0.16, y + (S * 0.56 - y) * 0.16) for x, y in pts]
    d.polygon(inner, fill=(255, 214, 0, 255))
    return im.resize((size, size), Image.LANCZOS)


def main():
    if len(sys.argv) < 3: sys.exit(__doc__)
    src, out = sys.argv[1], sys.argv[2]; size = int(sys.argv[3]) if len(sys.argv) > 3 else 128
    os.makedirs(out, exist_ok=True)
    logo = Image.open(src).convert('RGBA').resize((size, size), Image.LANCZOS)
    logo.save(os.path.join(out, 'APLogoUseful.png'))
    prog = logo.copy(); a = arrow(int(size * 0.62))
    prog.alpha_composite(a, ((size - a.width) // 2, (size - a.height) // 2))
    prog.save(os.path.join(out, 'APLogoProgression.png'))
    gray = ImageOps.grayscale(logo.convert('RGB')).convert('RGBA'); gray.putalpha(logo.getchannel('A'))
    gray.save(os.path.join(out, 'APLogoFiller.png'))
    print('wrote', out)


if __name__ == '__main__':
    main()
