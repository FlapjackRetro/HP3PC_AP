#!/usr/bin/env python3
"""Apply the 60 fps cap to your own HP3 system/Engine.dll.

Usage: python framerate_patch.py "<game>/system/Engine.dll" [--with-fps-counter]

Retail UGameEngine::GetMaxTickRate() returns 0.0 (uncapped) for single-player.
This repoints that load to an existing 60.0f constant, i.e. caps the game at 60 fps.
--with-fps-counter additionally applies the FStats::Render tweaks from the community
Google Drive patch, which force the 'stat fps' overlay on.
Only the retail Engine.dll (sha256 73bbe178...) is accepted. A backup is written next to it.
"""
import hashlib, shutil, sys

RETAIL_SHA256 = '73bbe178769b51429062c94ca0a209da3bd6e40d020588e8a4ca401427ecc34a'
CAP = [(0x118CA3, bytes.fromhex('2c2065'), bytes.fromhex('545567'))]  # fld [0x1065202C]=0.0 -> [0x10675554]=60.0
COUNTER = [
    (0x21A64D, bytes.fromhex('0f84da010000'), b'\x90' * 6),
    (0x21A81F, bytes.fromhex('506a005256ff918c00000083c410'), b'\x90' * 14),
    (0x45144A, b'a', b'\x00'),
    (0x451460, b' ', b'\x00'),
]

def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    if not args: sys.exit(__doc__)
    path = args[0]; data = bytearray(open(path, 'rb').read())
    if hashlib.sha256(data).hexdigest() != RETAIL_SHA256:
        sys.exit('Engine.dll is not the retail build (already patched?). Nothing changed.')
    patches = CAP + (COUNTER if '--with-fps-counter' in sys.argv else [])
    for off, old, new in patches:
        assert data[off:off + len(old)] == old, hex(off)
        data[off:off + len(new)] = new
    shutil.copy2(path, path + '.retail.bak')
    open(path, 'wb').write(data)
    print(f'Patched {len(patches)} site(s); backup at {path}.retail.bak')

if __name__ == '__main__': main()
