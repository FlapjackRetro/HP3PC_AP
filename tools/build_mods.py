#!/usr/bin/env python3
"""Build the mod plugins (HP3OpenCastle.asi debug + release, HP3MapTracker.asi, HP3WindowFix.asi) on Windows or Linux.

Usage: python tools/build_mods.py [OUT_DIR] [--only OpenCastle,MapTracker,WindowFix]     (default OUT_DIR: build)

Needs LLVM: clang, lld-link and llvm-dlltool (Windows: `winget install LLVM.LLVM`; found on PATH or in
C:\\Program Files\\LLVM\\bin). No MSVC or MinGW: the plugins have no C runtime and link against import libraries made
from each mod's own list of Windows functions.

That list is read from the mod's build.sh (its `mk <dll> <symbol>...` lines), so build.sh stays the one place a new
import is added, and this script builds exactly what `sh build.sh` builds.

Output:
  OUT_DIR/HP3OpenCastle.asi            debug (OC_DEBUG=1: debug panel, test grid, noclip)
  OUT_DIR/release/HP3OpenCastle.asi    release (OC_DEBUG=0)
  OUT_DIR/HP3MapTracker.asi, OUT_DIR/HP3WindowFix.asi
"""
import os, re, shutil, subprocess, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
MODS = {   # mod folder: (C source, output, variants as (subfolder, extra defines))
    'OpenCastle': ('opencastle.c', 'HP3OpenCastle.asi', [('', ['-DOC_DEBUG=1']), ('release', ['-DOC_DEBUG=0'])]),
    'MapTracker': ('maptracker.c', 'HP3MapTracker.asi', [('', [])]),
    'WindowFix':  ('windowfix.c', 'HP3WindowFix.asi', [('', [])]),
}
CFLAGS = ['--target=i686-pc-windows-msvc', '-O2', '-fno-builtin', '-ffreestanding', '-fno-stack-protector', '-mno-sse',
          '-Wall', '-Wextra']
LFLAGS = ['/dll', '/nodefaultlib', '/entry:DllMain@12', '/subsystem:windows', '/machine:x86', '/safeseh:no']


def tool(name):
    exe = name + ('.exe' if os.name == 'nt' else '')
    found = shutil.which(exe)
    if not found and os.name == 'nt':
        for base in (os.environ.get('ProgramFiles', r'C:\Program Files'), os.environ.get('ProgramFiles(x86)', '')):
            cand = os.path.join(base, 'LLVM', 'bin', exe)
            if base and os.path.exists(cand):
                found = cand
                break
    if not found:
        sys.exit(f'{name} not found: install LLVM (Windows: winget install LLVM.LLVM) and try again')
    return found


def imports(build_sh):
    """{dll: [symbols]} from the `mk` lines of a build.sh (backslash continuations joined)."""
    text = open(build_sh, encoding='utf-8').read().replace('\\\n', ' ')
    libs = {}
    for line in text.splitlines():
        line = line.strip()
        if line.startswith('mk ') and not line.startswith('mk()'):
            parts = line.split()
            libs[parts[1]] = parts[2:]
    return libs


def run(cmd, cwd=None):
    r = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True)
    if r.stdout.strip():
        print(r.stdout.strip())
    if r.returncode != 0:
        sys.exit(f'failed: {" ".join(cmd)}\n{r.stderr}')
    if r.stderr.strip():
        print(r.stderr.strip())


def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    only = None
    if '--only' in sys.argv:
        only = sys.argv[sys.argv.index('--only') + 1].split(',')
        args = [a for a in args if a != sys.argv[sys.argv.index('--only') + 1]]
    out = os.path.abspath(args[0] if args else os.path.join(ROOT, 'build'))
    clang, lld, dlltool = tool('clang'), tool('lld-link'), tool('llvm-dlltool')
    for mod, (src, asi, variants) in MODS.items():
        if only and mod not in only:
            continue
        mdir = os.path.join(ROOT, 'mod', mod)
        libdir = os.path.join(out, 'lib', mod)
        os.makedirs(libdir, exist_ok=True)
        libs = []
        for dll, syms in imports(os.path.join(mdir, 'build.sh')).items():
            d = os.path.join(libdir, dll + '.def')
            with open(d, 'w') as f:
                f.write(f'LIBRARY {dll}.dll\nEXPORTS\n' + ''.join(f'  {s}\n' for s in syms))
            lib = os.path.join(libdir, dll + '.lib')
            run([dlltool, '-m', 'i386', '-k', '-d', d, '-l', lib])
            libs.append(lib)
        for sub, defines in variants:
            vdir = os.path.join(out, sub) if sub else out
            os.makedirs(vdir, exist_ok=True)
            obj = os.path.join(libdir, f'{os.path.splitext(src)[0]}{"_" + sub if sub else ""}.obj')
            run([clang] + CFLAGS + defines + ['-c', src, '-o', obj], cwd=mdir)
            target = os.path.join(vdir, asi)
            run([lld] + LFLAGS + [obj] + libs + [f'/out:{target}', f'/implib:{os.path.join(libdir, asi + ".lib")}'])
            print(f'built {os.path.relpath(target)}')


if __name__ == '__main__':
    main()
