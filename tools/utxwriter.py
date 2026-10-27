#!/usr/bin/env python3
"""Write an HP3 (Unreal Engine 2, package version 129) texture package (.utx) from PNG files.

Usage: python utxwriter.py OUT.utx Name=image.png [Name=image.png ...]

Each image becomes a masked 8-bit palettized texture (TEXF_P8, bMasked), the format HP3's own sprite textures use
(e.g. SpellShapes.utx). Fully transparent pixels (alpha < 128) map to palette index 0, which the engine skips when
bMasked is set; the rest are quantized to 255 colours. Sides must be powers of two. A full mip chain is written.

The object path in game is <Package>.<Name>, e.g. HP3AP.APLogoProgression for HP3AP.utx. The package goes in the
game's textures folder, which the default Paths already search.

Layout (checked against SpellShapes.utx): header (tag, version 129, licensee 0, flags, name/export/import tables,
GUID, one generation), Palette exports (tagged properties "None", then a compact count and RGBA entries) and Texture
exports (tagged properties, then a compact mip count and per mip: an absolute offset just past the data, a compact
byte count, the bytes, USize, VSize, UBits, VBits).
"""
import os, struct, sys, uuid
from PIL import Image

TAG = 0x9E2A83C1
VERSION, LICENSEE = 129, 0
PKG_FLAGS = 1                         # PKG_AllowDownload, as the retail packages
RF_TEXTURE, RF_PALETTE = 0x000F0004, 0x00070004   # copied from SpellShapes.utx


def ci(v):
    """Unreal compact index."""
    out = bytearray(); a = abs(v)
    b0 = (0x80 if v < 0 else 0) | (a & 0x3F); a >>= 6
    if a: b0 |= 0x40
    out.append(b0)
    while a:
        b = a & 0x7F; a >>= 7
        if a: b |= 0x80
        out.append(b)
    return bytes(out)


def fstr(s):
    b = s.encode('latin-1') + b'\0'
    return ci(len(b)) + b


class Writer:
    def __init__(self):
        self.names = []; self.imports = []; self.exports = []

    def name(self, n):
        for i, x in enumerate(self.names):
            if x.lower() == n.lower(): return i
        self.names.append(n); return len(self.names) - 1

    def imp(self, cpkg, cls, outer, name):
        self.imports.append((self.name(cpkg), self.name(cls), outer, self.name(name)))
        return -len(self.imports)

    # tagged properties
    def prop(self, name, ptype, payload=b'', boolval=False):
        sizes = {1: 0, 2: 1, 4: 2, 12: 3, 16: 4}
        n = len(payload)
        info = ptype
        if ptype == 3:                                  # bool: the value is the array bit, no payload
            info |= 0x80 if boolval else 0
            return ci(self.name(name)) + bytes([info])
        if n in sizes: return ci(self.name(name)) + bytes([info | sizes[n] << 4]) + payload
        return ci(self.name(name)) + bytes([info | 5 << 4, n]) + payload

    def end_props(self):
        return ci(self.name('None'))


def quantize(img):
    """RGBA image -> (index bytes, 256 RGBA palette). Index 0 is the mask colour."""
    img = img.convert('RGBA')
    alpha = img.getchannel('A')
    rgb = Image.new('RGB', img.size, (0, 0, 0)); rgb.paste(img, mask=alpha)
    q = rgb.quantize(colors=255, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
    pal = q.getpalette()[:255 * 3]
    pal += [0] * (255 * 3 - len(pal))
    idx = bytes((0 if a < 128 else p + 1) for p, a in zip(q.tobytes(), alpha.tobytes()))
    palette = [(255, 0, 255, 0)] + [(pal[3 * i], pal[3 * i + 1], pal[3 * i + 2], 255) for i in range(255)]
    return idx, palette, q, alpha


def mips_of(img):
    """Masked mip chain: each level resized from the full-size RGBA, alpha thresholded."""
    out = []; w, h = img.size
    while True:
        out.append(img.resize((w, h), Image.LANCZOS) if (w, h) != img.size else img)
        if w == 1 and h == 1: break
        w = max(1, w // 2); h = max(1, h // 2)
    return out


def build(out_path, items):
    pkg = os.path.splitext(os.path.basename(out_path))[0]
    w = Writer()
    w.name('None')
    eng = w.imp('Core', 'Package', 0, 'Engine')
    cls_tex = w.imp('Core', 'Class', eng, 'Texture')
    cls_pal = w.imp('Core', 'Class', eng, 'Palette')
    blobs = []        # (class, name, flags, bytes builder)
    for tex_name, png in items:
        img = Image.open(png).convert('RGBA')
        W, H = img.size
        if W & (W - 1) or H & (H - 1): sys.exit(f'{png}: {W}x{H} is not a power of two')
        idx0, palette, q, alpha = quantize(img)
        levels = mips_of(img)
        # every mip uses the top level's palette (remap by nearest colour via PIL quantize with that palette)
        palimg = Image.new('P', (1, 1)); flat = []
        for r, g, b, a in palette[1:]: flat += [r, g, b]
        palimg.putpalette(flat + [0] * (768 - len(flat)))
        mipdata = [idx0]
        for lv in levels[1:]:
            a = lv.getchannel('A'); rgb = Image.new('RGB', lv.size, (0, 0, 0)); rgb.paste(lv, mask=a)
            qq = rgb.quantize(palette=palimg, dither=Image.Dither.NONE)
            mipdata.append(bytes((0 if aa < 128 else p + 1) for p, aa in zip(qq.tobytes(), a.tobytes())))
        pal_name = tex_name + 'Pal'
        pal_index = len(blobs) + 1          # export indices are 1-based, palettes are written just before
        blobs.append(('pal', pal_name, palette))
        blobs.append(('tex', tex_name, (W, H, mipdata, pal_index)))
    # export table data: serialize bodies (mip offsets need the absolute position, fixed up below)
    names_snapshot = None
    exports = []
    for kind, name, data in blobs:
        w.name(name)
    for k in ('Palette', 'bMasked', 'UBits', 'VBits', 'USize', 'VSize', 'UClamp', 'VClamp', 'Format'):
        w.name(k)
    header_size = 36 + 16 + 4 + 8
    # names
    name_bytes = b''.join(fstr(n) + struct.pack('<I', 0x00070010) for n in w.names)
    import_bytes = b''.join(ci(cp) + ci(cl) + struct.pack('<i', outer) + ci(nm) for cp, cl, outer, nm in w.imports)
    name_off = header_size
    import_off = name_off + len(name_bytes)
    data_off = import_off + len(import_bytes)
    bodies = []; pos = data_off
    for kind, name, data in blobs:
        if kind == 'pal':
            body = w.end_props() + ci(len(data)) + b''.join(bytes(c) for c in data)
        else:
            W, H, mipdata, pal_index = data
            props = (w.prop('Palette', 5, ci(pal_index)) + w.prop('bMasked', 3, boolval=True)
                     + w.prop('UBits', 1, bytes([W.bit_length() - 1])) + w.prop('VBits', 1, bytes([H.bit_length() - 1]))
                     + w.prop('USize', 2, struct.pack('<i', W)) + w.prop('VSize', 2, struct.pack('<i', H))
                     + w.prop('UClamp', 2, struct.pack('<i', W)) + w.prop('VClamp', 2, struct.pack('<i', H))
                     + w.end_props())
            body = bytearray(props + ci(len(mipdata)))
            mw, mh = W, H
            for md in mipdata:
                cnt = ci(len(md))
                after = pos + len(body) + 4 + len(cnt) + len(md)   # absolute offset just past this mip's bytes
                body += struct.pack('<i', after) + cnt + md
                body += struct.pack('<iiBB', mw, mh, max(mw.bit_length() - 1, 0), max(mh.bit_length() - 1, 0))
                mw = max(1, mw // 2); mh = max(1, mh // 2)
            body = bytes(body)
        bodies.append(body); exports.append((kind, name, pos, len(body))); pos += len(body)
    export_off = pos
    export_bytes = b''
    for kind, name, off, size in exports:
        cls = cls_pal if kind == 'pal' else cls_tex
        flags = RF_PALETTE if kind == 'pal' else RF_TEXTURE
        export_bytes += ci(cls) + ci(0) + struct.pack('<i', 0) + ci(w.name(name)) + struct.pack('<I', flags) + ci(size) + ci(off)
    header = struct.pack('<IHHIIIIIII', TAG, VERSION, LICENSEE, PKG_FLAGS, len(w.names), name_off,
                         len(exports), export_off, len(w.imports), import_off)
    header += uuid.uuid4().bytes + struct.pack('<iii', 1, len(exports), len(w.names))
    assert len(header) == header_size
    with open(out_path, 'wb') as f:
        f.write(header + name_bytes + import_bytes + b''.join(bodies) + export_bytes)
    return [f'{pkg}.{n}' for k, n, _ in blobs if k == 'tex']


if __name__ == '__main__':
    if len(sys.argv) < 3: sys.exit(__doc__)
    items = [a.split('=', 1) for a in sys.argv[2:]]
    for p in build(sys.argv[1], items): print(p)
