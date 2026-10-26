#!/usr/bin/env python3
"""Key textures for door / area unlock items.

  python gen_key.py drawn OUT.png
      our own gold key (vector-drawn, bevelled), 128x128, transparent background.
  python gen_key.py upscale <game>/textures/HP_FX.utx OUT.png
      HP3's own plain gold key (HP_FX.Particles.Key2, 32x32) scaled 4x with Scale2x twice, cropped to fill
      the 128x128 texture. It's built from the player's own game files, so no EA art goes into this repo.
"""
import os, sys
from PIL import Image, ImageChops, ImageDraw, ImageFilter
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

S = 512


def drawn():
    def shape(grow):
        m = Image.new('L', (S, S), 0); d = ImageDraw.Draw(m)
        cx, cy, R = 372, 256, 96 + grow
        d.ellipse((cx - R, cy - R, cx + R, cy + R), fill=255)
        d.rounded_rectangle((44 - grow, 234 - grow, cx - 40, 278 + grow), radius=8, fill=255)
        d.rounded_rectangle((56 - grow, 262 - grow, 98 + grow, 352 + grow), radius=6, fill=255)
        d.rounded_rectangle((126 - grow, 262 - grow, 162 + grow, 328 + grow), radius=6, fill=255)
        hole = 34 - grow
        d.ellipse((cx - hole, cy - hole, cx + hole, cy + hole), fill=0)
        return m
    outer, inner = shape(16), shape(0)
    grad = Image.new('RGBA', (S, S)); gd = ImageDraw.Draw(grad)
    for y in range(S):
        t = y / S
        gd.line([(0, y), (S, y)], fill=(int(255 - 40 * t), int(222 - 90 * t), int(96 - 70 * t), 255))
    im = Image.new('RGBA', (S, S), (0, 0, 0, 0))
    im.paste((74, 42, 4, 255), mask=outer); im.paste(grad, mask=inner)
    hl = ImageChops.subtract(inner, ImageChops.offset(inner, 6, 6)).filter(ImageFilter.GaussianBlur(2))
    sh = ImageChops.subtract(inner, ImageChops.offset(inner, -6, -6)).filter(ImageFilter.GaussianBlur(2))
    im.paste((255, 246, 190, 255), mask=hl.point(lambda v: int(v * 0.8)))
    im.paste((150, 90, 10, 255), mask=sh.point(lambda v: int(v * 0.7)))
    im = im.resize((128, 128), Image.LANCZOS)
    im.putalpha(im.getchannel('A').point(lambda v: 255 if v > 100 else 0))
    return im


def scale2x(im):
    w, h = im.size; s = im.load(); o = Image.new('RGBA', (w * 2, h * 2)); d = o.load()
    g = lambda x, y: s[min(max(x, 0), w - 1), min(max(y, 0), h - 1)]
    for y in range(h):
        for x in range(w):
            P, A, B, C, D = g(x, y), g(x, y - 1), g(x + 1, y), g(x - 1, y), g(x, y + 1)
            d[2 * x, 2 * y] = C if (C == A and C != D and A != B) else P
            d[2 * x + 1, 2 * y] = A if (A == B and A != C and B != D) else P
            d[2 * x, 2 * y + 1] = C if (D == C and D != B and C != A) else P
            d[2 * x + 1, 2 * y + 1] = B if (B == D and B != A and D != C) else P
    return o


def upscale(hp_fx):
    from ue2 import Package, read_tagged
    p = Package(hp_fx); idx = {p.path(e['idx']): e['idx'] for e in p.exports}
    r = p.reader(idx['Particles.Key2']); props = {n: v for n, _, t, v in read_tagged(p, r)}
    r.ci(); r.i32(); cnt = r.ci(); data = r.d[r.p:r.p + cnt]; r.p += cnt; w = r.i32(); h = r.i32()
    pr = p.reader(idx[props['Palette']]); read_tagged(p, pr); pc = pr.ci()
    pal = [tuple(pr.d[pr.p + 4 * k:pr.p + 4 * k + 4]) for k in range(pc)]
    src = Image.new('RGBA', (w, h)); src.putdata([(0, 0, 0, 0) if b == 0 else pal[b] for b in data[:w * h]])
    px = src.load()
    for y in range(h):
        for x in range(w):
            r_, g_, b_, a_ = px[x, y]
            if r_ < 12 and g_ < 12 and b_ < 12: px[x, y] = (0, 0, 0, 0)
    up = scale2x(scale2x(src))
    a = up.getchannel('A').filter(ImageFilter.GaussianBlur(0.7)).point(lambda v: 255 if v > 110 else 0)
    up = up.filter(ImageFilter.SMOOTH); up.putalpha(a)
    k = up.crop(up.getchannel('A').getbbox()); sc = 120 / max(k.size)
    k = k.resize((round(k.width * sc), round(k.height * sc)), Image.NEAREST)
    out = Image.new('RGBA', (128, 128), (0, 0, 0, 0)); out.paste(k, ((128 - k.width) // 2, (128 - k.height) // 2))
    return out


if __name__ == '__main__':
    if len(sys.argv) < 3: sys.exit(__doc__)
    if sys.argv[1] == 'drawn': drawn().save(sys.argv[2])
    elif sys.argv[1] == 'upscale' and len(sys.argv) > 3: upscale(sys.argv[2]).save(sys.argv[3])
    else: sys.exit(__doc__)
