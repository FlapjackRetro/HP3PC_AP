#!/usr/bin/env python3
"""Build the check-location catalogue and data tables from an HP3 install.

Usage: python build_catalog.py <game_dir> <out_dir>
  <game_dir> contains system/ and maps/. Writes:
    locations.json / hp3_location_catalog.csv  - every collectible-ish actor in every map
    wizard_cards.json                          - WizardCardManager.Cards default table
    game_states.json                           - HPWorldInfo.GameStateMasterList
"""
import os, sys, glob, json, csv, struct, collections
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ue2 import Package, load, Reader, decode_struct_array, read_tagged

CHECK_BASES = ['WizardCard', 'ChallengeStar', 'CauldronCake', 'PumpkinPasty', 'MaraudersPickup',
               'SecretAreaMarker', 'VendorItem', 'EndChallengeTrigger', 'Peeves', 'BeanBonusGame',
               'KWGenericSpawner', 'SavePoint', 'CreatureStatue', 'KWMiniGame']

def main():
    if len(sys.argv) < 3: sys.exit(__doc__)
    game, out = sys.argv[1], sys.argv[2]; os.makedirs(out, exist_ok=True)
    sysdir = os.path.join(game, 'system')
    sup, cdefaults = {}, {}
    pk = {n: Package(os.path.join(sysdir, n + '.u')) for n in ['Engine', 'GamePlay', 'kwGame', 'HGame']}
    for p in pk.values():
        for e in p.exports:
            if p.objclass(e['idx']) != 'Class': continue
            sup[e['name']] = p.objname(e['sup']) if e['sup'] else None
            if p.name in ('kwGame', 'HGame'):
                try: c = load(p, e['idx'])
                except Exception: continue
                cdefaults[e['name']] = {(n, i): v for n, i, t, v in c.get('defaults', [])}
    def isa(c, base):
        while c:
            if c == base: return True
            c = sup.get(c)
        return False
    def class_default(c, key):
        while c:
            if key in cdefaults.get(c, {}): return cdefaults[c][key]
            c = sup.get(c)
    def vec(v):
        if isinstance(v, tuple) and v[0] == 'STRUCT' and v[1] == 'Vector':
            return [round(x) for x in struct.unpack('<fff', v[2][:12])]
        return None

    # data tables from class defaults
    hg = pk['HGame']
    for e in hg.exports:
        if hg.objclass(e['idx']) != 'Class': continue
        if e['name'] == 'WizardCardManager':
            d = cdefaults['WizardCardManager']
            cards = decode_struct_array(hg, d[('Cards', 0)][1])
            sets = decode_struct_array(hg, d[('CardSets', 0)][1])
            for c in cards:
                for k, dv in (('Status', 0), ('Currency', 0), ('GameStateAvailable', 0)): c.setdefault(k, dv)
            json.dump({'statuses': ['AvailableInLevel', 'AvailableToVendors', 'AvailableInAdventureLevel',
                                    'AvailableToFnGShop', 'SpecialLastCard', 'OwnedByPlayer', 'OwnedByVendor'],
                       'currencies': ['Jellybeans', 'CauldronCakes', 'PumpkinPasties'],
                       'cards': cards, 'card_sets': sets}, open(os.path.join(out, 'wizard_cards.json'), 'w'), indent=1)
        if e['name'] == 'HPWorldInfo':
            v = cdefaults['HPWorldInfo'].get(('GameStateMasterList', 0))
            r = Reader(v[1]); n = r.ci()
            json.dump([r.fstr() for _ in range(n)], open(os.path.join(out, 'game_states.json'), 'w'), indent=1)

    def persist_key(lid, name, c, pr):
        """PersistentStore key the game writes for this actor (KWPickup.PickedUp / KWGenericSpawner.SavePersistentData),
        or None when the map turns persistence off (the actor comes back on every visit)."""
        def flag(k):
            v = pr.get(k)
            return class_default(c, (k, 0)) if v is None else v
        if isa(c, 'KWPickup'): return f"{lid}{name}_PU" if flag('bPickUpPersistent') is not False else None
        if isa(c, 'KWGenericSpawner'): return f"{lid}{name}_lives" if flag('bLivesPersistent') is not False else None
        return None

    rows = []
    for f in sorted(glob.glob(os.path.join(game, 'maps', '*.unr'))):
        p = Package(f); mp = os.path.splitext(os.path.basename(f))[0]; lid = None
        teleported = {}   # card actor Tag -> event tag of the HPSpawnThingy that hands it to the player
        for e in p.exports:
            c = p.objclass(e['idx'])
            if c == 'LevelInfo' and lid is None:   # the first LevelInfo (LevelInfo0) is the live one; later ones are leftovers
                lid = {n: v for n, i, t, v in load(p, e['idx'])['props']}.get('ID', '')
            elif isa(c, 'SpawnThingy'):   # HPSpawnThingy and plain SpawnThingy both create things on an event
                pr = {n: v for n, i, t, v in load(p, e['idx'])['props']}
                if pr.get('bTeleportThingy') and pr.get('TeleportActorTag'):
                    teleported[str(pr['TeleportActorTag']).lower()] = pr.get('Tag')
                sc = str(pr.get('SpawnClass') or '').split('.')[-1]
                sbase = next((b for b in ('WizardCard', 'PumpkinPasty', 'CauldronCake') if isa(sc, b)), None)
                if sbase:   # card or snack created on the spot when an event fires (quest reward, fight won, ...)
                    rows.append(dict(map=mp, level_id=lid or '', actor=e['name'], cls=sc, category=sbase,
                                     loc=vec(pr.get('Location')), tag=pr.get('Tag'), event=None, contents=[],
                                     persist_key=None, awarded_by=pr.get('Tag')))
        for e in p.exports:
            c = p.objclass(e['idx']); base = next((b for b in CHECK_BASES if isa(c, b)), None)
            if not base: continue
            try: o = load(p, e['idx'])
            except Exception as ex: print('skip', mp, e['name'], ex); continue
            pr = {(n if not i else f'{n}({i})'): v for n, i, t, v in o['props']}
            contents = []
            if base == 'KWGenericSpawner':
                for k in range(8):
                    key = 'GoodieToSpawn' if k == 0 else f'GoodieToSpawn({k})'
                    g = pr.get(key) or class_default(c, ('GoodieToSpawn', k))
                    if g and g != 'None':
                        nkey = 'GoodiesNumber' if k == 0 else f'GoodiesNumber({k})'
                        num = pr.get(nkey, class_default(c, ('GoodiesNumber', k)))
                        contents.append([g.split('.')[-1], num])
            random_amount = None
            if base == 'KWGenericSpawner' and contents and not any(n for _, n in contents):
                # no GoodiesNumber set: KWGenericSpawner spawns RandRange(Limits.Min, Limits.Max) goodies (Min when
                # Min >= Max), so the amount is random only when Max > Min
                lim = pr.get('Limits'); lo = hi = None
                if isinstance(lim, tuple) and lim[0] == 'STRUCT':
                    sub = {n: v for n, _, _, v in read_tagged(p, Reader(lim[2]))}
                    lo, hi = sub.get('Min', 0), sub.get('Max', 0)
                    if hi > lo: random_amount = f'{lo}-{hi - 1}'
                else:
                    random_amount = 'class default Limits'
            rows.append(dict(map=mp, level_id=lid or '', actor=e['name'], cls=c, category=base,
                             loc=vec(pr.get('Location')), tag=pr.get('Tag'), event=pr.get('Event'),
                             contents=contents,
                             persist_key=persist_key(lid or '', e['name'], c, pr),
                             awarded_by=teleported.get(str(pr.get('Tag')).lower()) if base == 'WizardCard' else None,
                             random_amount=random_amount))
    json.dump(rows, open(os.path.join(out, 'locations.json'), 'w'), indent=1)
    with open(os.path.join(out, 'hp3_location_catalog.csv'), 'w', newline='') as fh:
        w = csv.writer(fh)
        w.writerow(['map', 'level_id', 'actor', 'class', 'category', 'x', 'y', 'z', 'tag', 'event', 'contents', 'persist_key', 'awarded_by', 'random_amount'])
        for r in rows:
            w.writerow([r['map'], r['level_id'], r['actor'], r['cls'], r['category'], *(r['loc'] or ['', '', '']),
                        r['tag'] or '', r['event'] or '', ';'.join(f'{a}x{b}' for a, b in r['contents']), r['persist_key'] or '',
                        r.get('awarded_by') or '', r.get('random_amount') or ''])
    print(len(rows), 'rows;', dict(collections.Counter(r['category'] for r in rows)))

if __name__ == '__main__': main()
