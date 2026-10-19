#!/usr/bin/env python3
"""Decompile HP3's script packages into readable pseudo-UnrealScript.

Usage: python decompile.py <game>/system <out_dir> [Package ...]
Default packages: HGame kwGame GamePlay Engine. Natives are resolved from every package listed
plus Core. Also extracts the original .uc source still embedded in Engine.u/Core.u to
<out_dir>/_embedded_source/. Output stays local: it is EA/Epic code, so it is git-ignored.
"""
import os, sys, traceback
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ue2 import Package, NATIVES, load, dump_class, Reader

def extract_embedded(pkg, outdir):
    os.makedirs(outdir, exist_ok=True); n = 0
    for e in pkg.exports:
        if pkg.objclass(e['idx']) != 'TextBuffer' or e['size'] == 0: continue
        r = pkg.reader(e['idx'])
        if pkg.names[r.ci()] != 'None': continue
        r.i32(); r.i32(); txt = r.fstr()
        if txt.strip():
            open(os.path.join(outdir, f"{pkg.path(e['pkg'])}.{e['name']}.uc"), 'w').write(txt); n += 1
    return n

def main():
    if len(sys.argv) < 3: sys.exit(__doc__)
    sysdir, out = sys.argv[1], sys.argv[2]
    names = sys.argv[3:] or ['HGame', 'kwGame', 'GamePlay', 'Engine']
    pk = {n: Package(os.path.join(sysdir, n + '.u')) for n in dict.fromkeys(['Core', 'Engine'] + names)}
    for p in pk.values(): NATIVES.add_pkg(p)
    for n in names:
        p = pk[n]; od = os.path.join(out, n); os.makedirs(od, exist_ok=True); ok = bad = 0
        for e in p.exports:
            if p.objclass(e['idx']) != 'Class': continue
            try:
                open(os.path.join(od, e['name'] + '.uc'), 'w').write(dump_class(p, e['idx'])); ok += 1
            except Exception:
                bad += 1; open(os.path.join(od, e['name'] + '.ERR'), 'w').write(traceback.format_exc())
        print(f'{n}: {ok} classes, {bad} errors')
    for n in ('Core', 'Engine'):
        print(f'{n}: {extract_embedded(pk[n], os.path.join(out, "_embedded_source", n))} embedded source files')

if __name__ == '__main__': main()
