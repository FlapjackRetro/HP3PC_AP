#!/usr/bin/env python3
"""Build apworld/hp3pc/location_table.py from the location catalogue and the logic notes.

Usage: python tools/build_apworld_locations.py            (run from the repo root)

Inputs
  data/hp3_location_catalog.csv  every collectible-ish actor in every map (tools/build_catalog.py)
  docs/APWORLD.md                the logic notes: the region rules and the per-check table ("| Check | Extra needs |"),
                                 recorded in play 2026-10-27 .. 10-29

What it does
  1. Candidates: placed / event pasties and cakes, placed / event cards, secret areas, and containers with a fixed
     (not random) pasty / cake / card drop, on the maps Open Castle keeps.
  2. Drops what the notes say isn't a location (EXCLUDE below, each with its reason).
  3. Requirements = the area's base rule (AREAS) + every spell / password named in the check's own row of the notes
     + the area's progressive rules (PROGRESSIVE) + hand fixes (FIX). Mentions are taken from the row's "Extra needs"
     cell only; the few rows that name a spell in a negative sense are fixed in FIX.
  4. Adds the checks that aren't map actors: Fred & George's 6 cards and 8 passwords, the 20 student-vendor
     purchases, the 5 doxy rounds.
  5. Writes the table with fixed location IDs. IDs are kept in data/location_ids.json: existing names keep their ID,
     new names get the next free one, so regenerating never renumbers a released location.
"""
import csv, json, os, re, sys, collections

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CAT = os.path.join(ROOT, 'data', 'hp3_location_catalog.csv')
DOC = os.path.join(ROOT, 'docs', 'APWORLD.md')
IDS = os.path.join(ROOT, 'data', 'location_ids.json')
OUT = os.path.join(ROOT, 'apworld', 'hp3pc', 'location_table.py')

DROPPED_MAPS = {'HP3_BuckyExecuted', 'HP3_GroundsDada', 'HP3_DarkTower', 'HP3_Final', 'HP3_Quidditch',
                'HP3_WhompingWillow', 'HP3_DementorBattle', 'HP3_Infirmary'}

SPELLS = ['Alohomora', 'Depulso', 'Lumos', 'Rictusempra', 'Spongify', 'Expecto Patronum', 'Carpe Retractum',
          'Draconifors', 'Lapifors', 'Glacius']

# area -> (location name prefix, region, base requirements). The doc's row prefix is the area name with " (B)" for
# the B maps; FAMILY strips it for matching.
AREAS = {
    'HP3_Adv1Express':        ('Express', 'Hogwarts Express', ['Hogwarts Express Unlock', 'Alohomora', 'Depulso', 'Rictusempra', 'Lumos']),
    'HP3_Adv2ExpPatronum':    ('Patronum', 'Patronus Lesson', ['Patronus Lesson Unlock', 'Expecto Patronum']),
    'HP3_Adv3Library':        ('Library', 'Library', ['Library Unlock', 'Depulso']),
    'HP3_Adv5Shack':          ('Tunnel', 'Shrieking Shack Tunnel', ['Shrieking Shack Unlock', 'Rictusempra']),
    'HP3_Adv6Paddock':        ('Paddock', 'Paddock', ['Paddock Unlock', 'Depulso', 'Spongify', 'Draconifors']),
    'HP3_BeanBonus':          ('Bonus Room', 'Wizard Card Bonus Room', ['Bonus Room Unlock']),
    'HP3_COMC':               ('Hippogriff', 'Hippogriff Challenge', ['Hippogriff Challenge Unlock']),
    'HP3_Ch1CarpeRetractum':  ('Carpe', 'Carpe Retractum Challenge', ['Carpe Retractum Challenge Unlock', 'Carpe Retractum', 'Spongify', 'Depulso']),
    'HP3_Ch1CarpeRetractumB': ('Carpe B', 'Carpe Retractum Challenge', ['Carpe Retractum Challenge Unlock', 'Carpe Retractum', 'Spongify', 'Depulso', 'Rictusempra', 'Alohomora']),
    'HP3_Ch2Draconifors':     ('Drac/Lap', 'Draconifors Challenge', ['Draconifors Challenge Unlock', 'Lapifors']),
    'HP3_Ch2DraconiforsB':    ('Drac/Lap B', 'Draconifors Challenge', ['Draconifors Challenge Unlock', 'Lapifors', 'Alohomora', 'Depulso', 'Draconifors']),
    'HP3_Ch3Glacius':         ('Glacius', 'Glacius Challenge', ['Glacius Challenge Unlock', 'Glacius', 'Rictusempra']),
    'HP3_Ch3GlaciusB':        ('Glacius B', 'Glacius Challenge', ['Glacius Challenge Unlock', 'Glacius', 'Rictusempra', 'Spongify']),
    'HP3_DungeonHUB':         ('Dungeons', 'Dungeons', []),
    'HP3_FA1Ron':             ('Ron FE', "Ron's Final Exam", ["Ron's Final Exam Unlock", 'Carpe Retractum']),
    'HP3_FA2Hermione':        ('Hermione FE', "Hermione's Final Exam", ["Hermione's Final Exam Unlock", 'Lapifors']),
    'HP3_FA3Harry':           ('Harry FE', "Harry's Final Exam", ["Harry's Final Exam Unlock", 'Glacius']),
    'HP3_GroundsHub':         ('Grounds', 'Grounds', []),
    'HP3_InsideHub':          ('Hogwarts', 'Hogwarts', []),
    'HP3_PortraitPassword_B': ('Portrait B', 'Portrait B', ['Portrait Password 4', 'Lumos', 'Alohomora', 'Depulso']),
    'HP3_PortraitPassword_C': ('Portrait C', 'Portrait C', ['Portrait Password 6', 'Depulso', 'Spongify']),
    'HP3_PortraitPassword_D': ('Portrait D', 'Portrait D', ['Portrait Password 7', 'Alohomora']),
    'HP3_PortraitPassword_E': ('Portrait E', 'Portrait E', ['Portrait Password 8', 'Alohomora', 'Depulso', 'Spongify']),
    'HP3_PortraitPassword_F': ('Portrait F', 'Portrait F', ['Portrait Password 2']),
}
DOC_AREA = {  # the doc's row prefix per map
    'HP3_Adv1Express': 'Express', 'HP3_Adv2ExpPatronum': 'Patronum', 'HP3_Adv3Library': 'Library',
    'HP3_Adv5Shack': 'Tunnel', 'HP3_Adv6Paddock': 'Paddock', 'HP3_BeanBonus': 'Bonus Room', 'HP3_COMC': 'Hippogriff',
    'HP3_Ch1CarpeRetractum': 'Carpe', 'HP3_Ch1CarpeRetractumB': 'Carpe', 'HP3_Ch2Draconifors': 'Draconifors',
    'HP3_Ch2DraconiforsB': 'Draconifors', 'HP3_Ch3Glacius': 'Glacius', 'HP3_Ch3GlaciusB': 'Glacius',
    'HP3_DungeonHUB': 'Dungeons', 'HP3_FA1Ron': "Ron's exam", 'HP3_FA2Hermione': "Hermione's exam",
    'HP3_FA3Harry': "Harry's exam", 'HP3_GroundsHub': 'Grounds', 'HP3_InsideHub': 'Hogwarts',
    **{f'HP3_PortraitPassword_{c}': f'Portrait {c}' for c in 'BCDEF'},
}
ROW_PREFIXES = ['Express', 'Patronum', 'Library', 'Tunnel', 'Paddock', 'Bonus Room', 'Hippogriff', 'Carpe',
                'Draconifors', 'Glacius', 'Dungeons', "Ron's exam", "Hermione's exam", "Harry's exam", 'Grounds',
                'Hogwarts', 'Portrait B', 'Portrait C', 'Portrait D', 'Portrait E', 'Portrait F', 'Monster Book challenge']


def box(x0, x1, y0, y1, z0, z1):
    return lambda c: x0 <= c['xf'] <= x1 and y0 <= c['yf'] <= y1 and z0 <= c['zf'] <= z1


# (map, actor or predicate, reason). An actor name matches exactly; a predicate gets the candidate dict.
EXCLUDE = [
    # Library: an out-of-bounds copy of the grounds (cutscene set), checked with noclip 2026-10-28
    *[('HP3_Adv3Library', a, 'out of bounds (cutscene set)') for a in
      ('SecretAreaMarker2', 'SecretAreaMarker4', 'SecretAreaMarker5', 'SecretAreaMarker6', 'WC_DoubleEndedNewt0')],
    # Carpe: only the first-visit doxy cutscene (GState000) spawns it
    ('HP3_Ch1CarpeRetractum', 'HPSpawnThingy3', 'never spawns in GState090'),
    # repeatable event rewards (2026-10-29): exempt, vanilla
    *[('HP3_Ch1CarpeRetractum', f'HPSpawnThingy{n}', 'repeatable event reward') for n in (4, 6, 8)],
    *[('HP3_PortraitPassword_C', f'HPSpawnThingy{n}', 'repeatable event reward') for n in (1, 4, 6, 8)],
    *[('HP3_PortraitPassword_B', f'SpawnThingy{n}', 'repeatable event reward') for n in (0, 2, 3)],
    *[('HP3_InsideHub', f'HPSpawnThingy{n}', 'repeatable event reward') for n in (1, 2, 4)],
    ('HP3_DungeonHUB', lambda c: c['kind'] == 'event' and c['actor'].startswith('HPSpawnThingy'),
     'repeatable event reward (Stroulger doxy fight, Glover firecrab line)'),
    # bean bonus rooms inside the B challenge maps (2026-10-29)
    ('HP3_Ch2DraconiforsB', box(-9500, -7000, -1800, 700, -1000, -40), 'bean bonus room'),
    ('HP3_Ch3GlaciusB', box(-6700, -4400, 3600, 5900, -1000, -40), 'bean bonus room'),
    # Glacius: the two maps overlap (same actor, same place); each copy is reachable in one map only
    ('HP3_Ch3Glacius', lambda c: c['overlap'] and c['zf'] < 2000, 'overlap: reachable in Glacius B'),
    ('HP3_Ch3GlaciusB', lambda c: c['overlap'] and c['zf'] >= 2000, 'overlap: reachable in Glacius'),
    ('HP3_Ch3Glacius', 'PumpkinPasty36', 'exempt'),
    *[('HP3_Ch3GlaciusB', f'PumpkinPasty{n}', 'exempt: believed reachable, unconfirmed') for n in (22, 23, 24, 6)],
    # Hogwarts: never spawned in Open Castle, shop display props, no position
    *[('HP3_InsideHub', f'HPSpawnThingy{n}', 'Peeves reward (never fires)') for n in (13, 14, 16, 18, 22, 23, 25)],
    *[('HP3_InsideHub', f'WC_BeatrixBloxam{n}', "Fred & George's display prop") for n in range(1, 6)],
    ('HP3_InsideHub', 'CauldronCake4', 'no position in the map data'),
    ('HP3_InsideHub', 'PumpkinPasty5', 'no position in the map data'),
    # Grounds: the doxy challenge's reward cards sit in a closed room and are teleported out on each round's win;
    # they become the "Doxy: Round n" checks below instead
    *[('HP3_GroundsHub', a, 'doxy round reward (see the Doxy checks)') for a in
      ('WC_BrantheBloodthirsty0', 'WC_Cyclops0', 'WC_Goliath0', 'WC_HengistofUpperBarnton0', 'WC_Morholt0')],
]

# extra requirements by position in the area (progressive rules from the region table)
def _drac_b_after_horntail(c):   # everything recorded after ChestSpawner13 (doc order), i.e. all but it
    return c['actor'] != 'ChestSpawner13'

PROGRESSIVE = [
    ('HP3_Ch2DraconiforsB', _drac_b_after_horntail, ['Rictusempra']),
    # Glacius (Ch3): Spongify for everything past (3524, -3991, 11274); before it only ChestSpawner12 + secret 3
    ('HP3_Ch3Glacius', lambda c: c['actor'] not in ('ChestSpawner12', 'SecretAreaMarker3'), ['Spongify']),
    # Hermione's exam: Draconifors for all but the start (rows say "exam rule only" there)
    # Dungeons: the portrait rooms' area rules
    ('HP3_DungeonHUB', lambda c: 'Portrait Password 3' in c['req'], ['Spongify']),
    ('HP3_DungeonHUB', lambda c: 'Portrait Password 5' in c['req'], ['Lumos']),
    # Bonus Room: the upper levels (z > 0) need Spongify
    ('HP3_BeanBonus', lambda c: c['zf'] > 0, ['Spongify']),
    # Patronum: the three skull shelves need Depulso
    ('HP3_Adv2ExpPatronum', lambda c: c['actor'].startswith('SkullShelfSpwn'), ['Depulso']),
]

# hand fixes: (map, actor) -> (remove, add)
FIX = {
    # "not behind Spongify": ground floor knight
    ('HP3_BeanBonus', 'KnightSpawner1'): (['Spongify'], []),
    # the Monster Book reward cards: the challenge's own rule (its region carries the unlock)
}

CONTAINER_NAMES = {
    'ChestSpawner': 'Chest', 'ChestCompanionSpawner': 'Companion Chest', 'KnightSpawner': 'Knight',
    'KnightShieldSpawner': 'Shield Knight', 'KnightCollapse': 'Collapsing Knight', 'ArmoireSpwn': 'Armoire',
    'BannerSpwn': 'Banner', 'CauldronSpwn': 'Cauldron', 'CrateSpwn': 'Crate', 'Hpgenericspawner': 'Spawner',
    'Pumpkin': 'Pumpkin', 'Scarecrow': 'Scarecrow', 'SkullFloatSpwn': 'Floating Skull', 'SkullShelfSpwn': 'Skull Shelf',
    'TapestrySpwn': 'Tapestry', 'TopiaryBadgerSpwn': 'Badger Topiary', 'TopiaryLionSpwn': 'Lion Topiary',
    'TopiaryRavenSpwn': 'Raven Topiary', 'TopiarySnakeSpwn': 'Snake Topiary', 'AlohomoraPadLock': 'Padlock',
}


def goodies(contents):
    out = []
    for g in re.split(r'[;,]\s*', contents):
        m = re.match(r'(.+?)x(None|\d+)$', g.strip())
        if m:
            out.append((m.group(1), None if m.group(2) == 'None' else int(m.group(2))))
    return out


def candidates():
    cat = list(csv.DictReader(open(CAT, encoding='utf-8')))
    pos = collections.Counter((r['actor'], r['x'], r['y'], r['z']) for r in cat if r['map'] in ('HP3_Ch3Glacius', 'HP3_Ch3GlaciusB'))
    res = []
    for r in cat:
        if r['map'] in DROPPED_MAPS or r['map'] not in AREAS:
            continue
        c, kind, give = r['category'], None, []
        if c in ('PumpkinPasty', 'CauldronCake'):
            kind = 'placed' if r['persist_key'] else 'event'
            give = [('pasty' if c == 'PumpkinPasty' else 'cake', 1)]
        elif c == 'WizardCard':
            kind, give = 'card', [('card', r['class'])]
        elif c == 'SecretAreaMarker':
            kind = 'secret'
        elif c == 'KWGenericSpawner':
            for n, k in goodies(r['contents']):
                if n.lower().startswith('wc_'):
                    give.append(('card', n))   # a card is always one (xNone included: KnightSpawner2 in Ch2)
                    continue
                if k is None:
                    continue   # random amount: not a location
                if n == 'PumpkinPasty':
                    give.append(('pasty', k))
                elif n == 'CauldronCake':
                    give.append(('cake', k))
            if give:
                kind = 'container'
        if not kind:
            continue
        d = dict(r, kind=kind, give=give)
        d['xf'], d['yf'], d['zf'] = (float(r[k]) if r[k] else 0.0 for k in ('x', 'y', 'z'))
        d['overlap'] = r['map'] in ('HP3_Ch3Glacius', 'HP3_Ch3GlaciusB') and pos[(r['actor'], r['x'], r['y'], r['z'])] > 1
        res.append(d)
    return res


def doc_rows():
    L = open(DOC, encoding='utf-8').read().split('\n')
    start = next(i for i, l in enumerate(L) if l.startswith('| Check | Extra needs'))
    end = next(i for i, l in enumerate(L) if l.startswith('## Repeatable'))
    rows = []
    for i in range(start + 2, end):
        l = L[i]
        if not l.startswith('| '):
            continue
        cells = [x.strip() for x in l.strip().strip('|').split('|')]
        if len(cells) < 3 or cells[0].startswith('(not a location)'):
            continue
        at = re.findall(r'At \(([-−\d.]+),\s*([-−\d.]+),\s*([-−\d.]+)\)', cells[1])
        rows.append(dict(line=i + 1, head=cells[0], needs=cells[1], acts=row_actors(cells[0] + ' ' + cells[1]),
                         at=[tuple(float(v.replace('−', '-')) for v in a) for a in at]))
    return rows


def row_actors(text):
    acts, last = set(), None
    for t in re.findall(r'`([^`]+)`', text):
        for part in re.split(r'\s*/\s*', t):
            m = re.match(r'([A-Za-z_]+?)(\d+)$', part)
            if m:
                last = m.group(1); acts.add(part)
            elif re.fullmatch(r'\d+', part) and last:
                acts.add(last + part)
    for pre, a, b in re.findall(r'`([A-Za-z_]+?)(\d+)`(?:\s*[–-]\s*|\s*to\s*)`(\d+)`', text):
        acts |= {f'{pre}{k}' for k in range(int(a), int(b) + 1)}
    for pre, rest in re.findall(r'`([A-Za-z_]+?)\d+`((?:\s*(?:,|/|and|or)?\s*`\d+`(?:\s*[–-]\s*`\d+`)?)+)', text):
        for a, b in re.findall(r'`(\d+)`\s*[–-]\s*`(\d+)`', rest):
            acts |= {f'{pre}{k}' for k in range(int(a), int(b) + 1)}
        for a in re.findall(r'`(\d+)`', rest):
            acts.add(pre + a)
    return acts


def row_area(head):
    for a in ROW_PREFIXES:
        if head.startswith(a):
            return 'Hogwarts' if a == 'Monster Book challenge' else a
    return None


def mentions(t):
    s = {x for x in SPELLS if re.search(r'\b' + x + r'\b', t)}
    if re.search(r'\bPatronum\b', t):
        s.add('Expecto Patronum')
    s |= {f'Portrait Password {n}' for n in re.findall(r'Portrait Password (\d)', t)}
    s |= {f'Portrait Password {n}' for n in re.findall(r'Password (\d) area', t)}
    if 'Monster Book Challenge Unlock' in t:
        s.add('Monster Book Challenge Unlock')
    return s


def matching_rows(c, rows):
    out = []
    for r in rows:
        if c['actor'] not in r['acts']:
            continue
        a = row_area(r['head'])
        if a is not None and a != DOC_AREA[c['map']]:
            continue
        if r['at'] and len(r['acts']) == 1 and not any(abs(c['xf'] - p[0]) < 8 and abs(c['yf'] - p[1]) < 8 for p in r['at']):
            continue
        out.append(r)
    return out


def excluded(c):
    for m, a, why in EXCLUDE:
        if m != c['map']:
            continue
        if (callable(a) and a(c)) or a == c['actor']:
            return why
    return None


def main():
    cards = {}
    src = open(os.path.join(ROOT, 'apworld', 'hp3pc', 'items.py'), encoding='utf-8').read()
    for idx, cls, name in re.findall(r'\(\s*(\d+), "([^"]+)", "([^"]+)"\)', src):
        cards[cls.lower()] = name
    rows = doc_rows()
    locs, dropped, unmatched = [], collections.Counter(), []
    for c in candidates():
        why = excluded(c)
        if why:
            dropped[why] += 1
            continue
        prefix, region, base = AREAS[c['map']]
        ms = matching_rows(c, rows)
        req = set(base)
        for r in ms:
            req |= mentions(r['needs'])
        if not ms and c['map'] not in ('HP3_Adv1Express', 'HP3_Adv2ExpPatronum', 'HP3_COMC', 'HP3_BeanBonus'):
            unmatched.append(f"{c['map']} {c['actor']}")
        c['req'] = req
        for m, pred, add in PROGRESSIVE:
            if m == c['map'] and pred(c):
                req |= set(add)
        rem, add = FIX.get((c['map'], c['actor']), ([], []))
        req = (req - set(rem)) | set(add)
        # the Monster Book reward cards sit in InsideHub but belong to the challenge
        if c['map'] == 'HP3_InsideHub' and 'Monster Book Challenge Unlock' in req:
            prefix, region = 'Monster Book', 'Monster Book Challenge'
            req.discard('Monster Book Challenge Unlock')
        # thing name
        num = re.search(r'(\d+)$', c['actor']).group(1) if re.search(r'(\d+)$', c['actor']) else ''
        card = next((g[1] for g in c['give'] if g[0] == 'card'), None)
        if c['kind'] == 'card':
            thing = f"{cards[card.lower()]} Card"
        elif c['kind'] == 'secret':
            thing = f"Secret Area {num}"
        elif c['kind'] in ('placed', 'event'):
            what = 'Pumpkin Pasty' if c['category'] == 'PumpkinPasty' else 'Cauldron Cake'
            if c['category'] in ('PumpkinPasty', 'CauldronCake') and c['actor'].startswith(('PumpkinPasty', 'CauldronCake')):
                thing = f"{what} {num}"
            else:
                thing = f"Event {what} {num}"
        else:
            base_name = CONTAINER_NAMES.get(c['class'], c['class'])
            thing = f"{base_name} {num}" + (f" ({cards[card.lower()]} Card)" if card else '')
        pasty = sum(k for g, k in c['give'] if g == 'pasty')
        cake = sum(k for g, k in c['give'] if g == 'cake')
        locs.append(dict(name=f"{prefix}: {thing}", region=region, map=c['map'], actor=c['actor'], kind=c['kind'],
                         key=c['persist_key'] or '', card=cards[card.lower()] + ' Card' if card else '',
                         pasty=pasty, cake=cake, req=sorted(req), price=None))

    # ---- checks that aren't map actors ---------------------------------------------------------------------------
    fng_cards = [('Grogan Stump', 'pasty', 25), ('Gunhilda of Gorsemoor', 'bean', 100), ('Hesper Starkey', 'bean', 125),
                 ('Mirabella Plunkett', 'pasty', 25), ('Mountain Troll', 'bean', 150), ('Streeler', 'bean', 200)]
    for n, cur, p in fng_cards:
        locs.append(dict(name=f"F&G Shop: {n} Card", region="Fred and George's Shop", map='HP3_InsideHub', actor='',
                         kind='shop', key='', card=f'{n} Card', pasty=0, cake=0, req=[], price=(cur, p)))
    for k, p in enumerate((10, 12, 12, 15, 15, 20, 25, 30)):
        locs.append(dict(name=f"F&G Shop: Portrait Password {k + 1}", region="Fred and George's Shop", map='HP3_InsideHub',
                         actor=f'pp_{k}', kind='shop', key='', card='', pasty=0, cake=0, req=[], price=('cake', p)))
    vendor = [('Beatrix Bloxam', 'bean', 25), ('Carlotta Pinkstone', 'bean', 100), ('Cliodne', 'pasty', 30),
              ('Crispin Cronk', 'bean', 100), ('Dorcas Wellbeloved', 'bean', 175), ('Dymphna Furmage', 'bean', 50),
              ('Edgar Stroulger', 'bean', 150), ('Elfrida Clagg', 'bean', 125), ('Ethelred the Ever-Ready', 'pasty', 80),
              ('Glanmore Peakes', 'pasty', 60), ('Ignatia Wildsmith', 'pasty', 10), ('Jocunda Sykes', 'bean', 200),
              ('Morgan le Fay', 'pasty', 50), ('Mungo Bonham', 'bean', 50), ('Norvel Twonk', 'bean', 125),
              ('Oswald Beamish', 'bean', 225), ('Thaddeus Thurkell', 'pasty', 70), ('Wendelin the Weird', 'bean', 75),
              ('Wilfred Elphick', 'pasty', 40), ('Zou Yen', 'pasty', 20)]
    for k, (n, cur, p) in enumerate(vendor):
        locs.append(dict(name=f"Vendor: Card Purchase {k + 1}", region='Student Vendors', map='HP3_InsideHub',
                         actor=f'vendor_{k + 1}', kind='vendor', key='', card=f'{n} Card', pasty=0, cake=0, req=[],
                         price=(cur, p)))
    for k, n in enumerate(('Bran the Bloodthirsty', 'Cyclops', 'Goliath', 'Morholt', 'Hengist of Upper Barnton')):
        locs.append(dict(name=f"Doxy: Round {k + 1} ({n} Card)", region='Doxy Challenge', map='HP3_GroundsHub',
                         actor=f'WinCard{k + 1}', kind='minigame', key='', card=f'{n} Card', pasty=0, cake=0,
                         req=['Rictusempra'], price=None))

    names = [l['name'] for l in locs]
    dup = [n for n, k in collections.Counter(names).items() if k > 1]
    if dup:
        sys.exit(f'duplicate location names: {dup}')
    ids = json.load(open(IDS)) if os.path.exists(IDS) else {}
    nxt = max(ids.values(), default=0) + 1
    for l in sorted(locs, key=lambda l: (l['region'], l['name'])):
        if l['name'] not in ids:
            ids[l['name']] = nxt; nxt += 1
        l['id'] = ids[l['name']]
    json.dump(dict(sorted(ids.items(), key=lambda t: t[1])), open(IDS, 'w'), indent=1)
    locs.sort(key=lambda l: l['id'])

    with open(OUT, 'w', encoding='utf-8') as f:
        f.write('"""Generated by tools/build_apworld_locations.py from data/hp3_location_catalog.csv and the logic notes in\n'
                'docs/APWORLD.md. Do not edit by hand: change the notes or the script\'s tables and regenerate.\n\n'
                'Fields: name, location id offset, region, map, actor, kind, save key, vanilla card, vanilla pasties, vanilla\n'
                'cakes, requirements (item names; spells resolve through rules.has_spell), shop price (currency, amount) or None.\n'
                '"""\n')
        f.write('from typing import NamedTuple, Optional\n\n\n')
        f.write('class LocationData(NamedTuple):\n    name: str\n    offset: int\n    region: str\n    map: str\n    actor: str\n'
                '    kind: str\n    save_key: str\n    card: str\n    pasties: int\n    cakes: int\n    requires: tuple\n'
                '    price: Optional[tuple]\n\n\n')
        f.write('LOCATIONS: tuple = (\n')
        for l in locs:
            f.write(f"    LocationData({l['name']!r}, {l['id']}, {l['region']!r}, {l['map']!r}, {l['actor']!r}, {l['kind']!r}, "
                    f"{l['key']!r}, {l['card']!r}, {l['pasty']}, {l['cake']}, {tuple(l['req'])!r}, {l['price']!r}),\n")
        f.write(')\n')
    print(f'{len(locs)} locations written to {os.path.relpath(OUT, ROOT)}')
    print('dropped:', dict(dropped))
    if unmatched:
        print('no logic row (area rule only):', len(unmatched)); print('  ' + '\n  '.join(unmatched))


if __name__ == '__main__':
    main()
