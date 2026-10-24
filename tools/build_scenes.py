#!/usr/bin/env python3
"""Build the map entrance graph: every way each map can be loaded.

Usage: python build_scenes.py <game_dir> <out_dir>
Sources:
  * level-change actors placed in maps (ChangeLevelTrigger.NewMapName), with their
    game-state window (Group tags / ExcludeFromGameStates) and any NewGameState they set
  * cutscene scripts (system/Cutscenes/*.int) containing `ChangeLevel <map>`, joined to the
    map actors whose Event fires that cutscene
  * hppoa.ini / Default.ini FirstLevel (the new-game map)
Writes <out_dir>/map_entrances.json.
"""
import os, sys, re, glob, json, collections
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ue2 import Package, load, Reader

GS = re.compile(r'GSTATE\d{3}', re.I)
SKIP = {'Package', 'Class', 'Function', 'Model', 'Polys', 'TextBuffer', 'Level', 'Texture', 'StaticMesh', 'Sound', 'Palette', 'Shader'}

def state_window(pr):
    g = pr.get('Group') if isinstance(pr.get('Group'), str) else ''
    only = sorted(set(s.upper() for s in GS.findall(g)))
    ex = pr.get('ExcludeFromGameStates'); excl = []
    if isinstance(ex, tuple):
        r = Reader(ex[1]); n = r.ci(); excl = [s.upper() for s in (r.fstr() for _ in range(n)) if s]
    return only, excl

def main():
    if len(sys.argv) < 3: sys.exit(__doc__)
    game, out = sys.argv[1], sys.argv[2]; os.makedirs(out, exist_ok=True)
    cs = {}
    for f in glob.glob(os.path.join(game, 'system', 'Cutscenes', '*.int')):
        t = open(f, encoding='latin1').read().replace('\r', '')
        dests = [d.split('?')[0].replace('.unr', '') for d in re.findall(r'=\s*ChangeLevel\s+(\S+)', t, re.I)]
        if dests:
            cs[os.path.splitext(os.path.basename(f))[0].lower()] = dict(
                name=os.path.splitext(os.path.basename(f))[0], dest=dests,
                sets_state=re.findall(r'=\s*SetGameState\s+(\S+)', t, re.I))
    entrances = collections.defaultdict(list); maps = []
    for f in sorted(glob.glob(os.path.join(game, 'maps', '*.unr'))):
        p = Package(f); mp = os.path.splitext(os.path.basename(f))[0]; maps.append(mp)
        for e in p.exports:
            c = p.objclass(e['idx'])
            if c in SKIP: continue
            try: o = load(p, e['idx'])
            except Exception: continue
            pr = {(n if not i else f'{n}({i})'): v for n, i, t, v in o.get('props', [])}
            only, excl = state_window(pr)
            dest = pr.get('NewMapName') if isinstance(pr.get('NewMapName'), str) else None
            if dest:
                d = dest.replace('.unr', '')
                if d.lower() != mp.lower():
                    entrances[d.lower()].append(dict(dest=d, source=mp, via='exit', actor=e['name'], tag=pr.get('Tag'),
                        only_in=only, not_in=excl,
                        sets_state=[pr['NewGameState']] if pr.get('NewGameState') and pr.get('bSetGameState', True) else []))
            for k, v in pr.items():
                if isinstance(v, str) and k.startswith(('Event', 'OutEvents', 'EndEvent', 'CardWinEvent', 'LoseEvent', 'LevelExitEvent')) and v.lower() in cs:
                    for d in cs[v.lower()]['dest']:
                        if d.lower() == mp.lower(): continue
                        entrances[d.lower()].append(dict(dest=d, source=mp, via='cutscene', cutscene=cs[v.lower()]['name'],
                            actor=e['name'], actor_class=c, only_in=only, not_in=excl, sets_state=cs[v.lower()]['sets_state']))
    for ini in ('system/Default.ini',):
        path = os.path.join(game, ini)
        if os.path.exists(path):
            m = re.search(r'^FirstLevel=(\S+)', open(path, encoding='latin1').read(), re.M)
            if m:
                d = m.group(1).replace('.unr', '')
                entrances[d.lower()].append(dict(dest=d, source='(new game)', via='FirstLevel', actor=ini, only_in=[], not_in=[], sets_state=[]))
    # cutscenes that change level but aren't fired by a map Event (chained or level-start)
    fired = {x['cutscene'].lower() for v in entrances.values() for x in v if x['via'] == 'cutscene'}
    unfired = []
    raw = {m: open(os.path.join(game, 'maps', m + '.unr'), 'rb').read().lower() for m in maps}
    for k, v in sorted(cs.items()):
        if k in fired: continue
        refs = [m for m in maps if k.encode() in raw[m]]
        unfired.append(dict(cutscene=v['name'], dest=v['dest'], sets_state=v['sets_state'], referenced_by=refs))
        for d in v['dest']:
            for m in refs:
                if m.lower() != d.lower():
                    entrances[d.lower()].append(dict(dest=d, source=m, via='cutscene (chained / level start)', cutscene=v['name'],
                                                     actor=None, only_in=[], not_in=[], sets_state=v['sets_state']))
    result = {m: entrances.get(m.lower(), []) for m in maps}
    json.dump(dict(maps=result, unattributed_cutscenes=unfired), open(os.path.join(out, 'map_entrances.json'), 'w'), indent=1)
    for m in maps:
        print(f'{m:26s} {len(result[m])} entrances')

if __name__ == '__main__': main()
