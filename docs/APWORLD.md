# Apworld: items and options (foundation)

Code: `apworld/hp3pc/` (`items.py`, `options.py`, `world.py`). Locations, regions and rules aren't written yet, so
seeds can't be generated; the item table and options are in place. Decisions from 2026-10-26.

## Item tiers

| Tier | Items | Count |
| --- | --- | ---: |
| progression | Wizard cards (one item per card, "*Name* Card") | 79 |
| progression | Spells: Alohomora, Depulso, Lumos, Rictusempra, Spongify, Expecto Patronum | 6 |
| progression | Challenge spells: Carpe Retractum, Draconifors, Lapifors, Glacius (only with `split_challenge_spells`) | 4 |
| progression | Level unlocks: Carpe Retractum / Draconifors / Glacius / Monster Book / Hippogriff / Doxy Challenge; Hogwarts Express, Patronus Lesson, Library, Shrieking Shack, Paddock | 11 |
| progression | Portrait Password 1–8 (`pp_0`–`pp_7`, in the order the twins sell them) | 8 |
| progression_skip_balancing | Pumpkin pasty bundles 1, 2, 3, 4, 5, 10; cauldron cake bundles 1, 2, 3, 4 | per location |
| useful | Shortcut Passwords (`pp_8`–`pp_21` at once) | 1 |
| filler | Chocolate Frog (heals on receipt; the repeatable filler), Marauder's Map | — / 1 |

Not randomized (first pass): jellybeans, challenge stars (the game's challenge shields), the Harry Potter card.

IDs: `33_000_000` + a fixed offset (spells 1–10, unlocks 101–111, passwords 200–207 and 220, currency 301–310 and
321–324, filler 340–341, traps 350–399 (reserved), cards 1000 + index in `WizardCardManager.Cards`). Offsets never change once released.

### Currency

- **Pasties and cakes are progression_skip_balancing.** Shop checks are logic-gated on the total received (currency
  is spent, so e.g. all eight portrait passwords need 139 cakes at normal prices, 71 at cheap).
- **Bundles match vanilla drops.** A location's item is the bundle it held in vanilla (a single placed pasty → 
  "Pumpkin Pasty", a chest with 4 → "4 Pumpkin Pasties"). Seen in the catalogue: pasties 1–5 and 10, cakes 1–4.
- **Safety net, not logic:** Fred & George's pasties (30 beans) and cakes (50 beans), the bean bonus room, and the
  rolled-amount containers (left vanilla) all give currency that logic doesn't count.
- Jellybeans aren't randomized. Bean-priced cards (4 of the twins' 6, all student-vendor cards) are assumed
  affordable.

### Spells (`spell_mode`)

- **vanilla** (default): start with Rictusempra, Alohomora, Lumos, Depulso, Spongify. Expecto Patronum and the
  challenge spells are in the pool.
- **hard**: start with nothing. The early game rests on the few checks reachable with no spells (e.g. the cauldron
  cake near Hagrid's hut); a first Alohomora or Depulso opens up a lot. Needs a sphere-1 count from the location data
  before release, and the softlock list (OPEN_CASTLE.md §7a) matters more here.
- Expecto Patronum: progression. In Open Castle only Adv2ExpPatronum uses it (the Dementor maps are dropped).
- Character restrictions don't need logic: each challenge's loading point loads the right character (vanilla).

### Challenge unlocks (`split_challenge_spells`)

- **Off** (default): the challenge unlock also grants its spell(s): Carpe Retractum Challenge → Carpe Retractum;
  Draconifors Challenge → Draconifors **and** Lapifors; Glacius Challenge → Glacius. The four spells aren't in the
  pool.
- **On**: the four spells are separate items, in addition to the three challenge unlocks.

## Goal (`goal`, `cards_required`)

- `final_day_cards`: the Final Day door (GroundsHub `FinalDayDoor`) opens with `cards_required` cards (1–79,
  default 40). Victory is the event "Final Day Door Opened".
- Ideas to workshop: N cards **and** pass the three final exams; card sets (the game has 10: WZ 15, WI 15, QU 5,
  VP 5, HG 5, GI 5, GB 5, DR 5, BE 15, BO 5).
- The Harry Potter card check stays vanilla: Fred & George give it once the other 79 are found.

## Other options

| Option | Values | |
| --- | --- | --- |
| `shop_prices` | normal / cheap | cheap = 50% rounded up ([SHOP.md](SHOP.md)) |
| `shortcut_passwords` | item / start_unlocked | default item (in the pool, useful) |

Slot data: all six options.

## Client sync

How received items reach the game and how saves are reconciled with the server: [CLIENT_SYNC.md](CLIENT_SYNC.md).

## Mod work this implies

- **Bean bonus room:** challenge stars stay vanilla, so finding all of a challenge's stars still sends the player to
  HP3_BeanBonus. Allow it, and when the bonus game ends, travel to InsideHub instead of the vanilla next map.
- Receiving a challenge unlock (split off) must grant the spell(s) as well as open the door.
- Spells as items: replace `WeaponWand.HasSpell()`; Lumos needs its own hook (`LumosTurnOn`).
- Shortcut Passwords item: the mod currently unlocks all shortcuts at load; that becomes the `start_unlocked` path.

## Open questions

- Which portrait each of Portrait Password 1–8 opens (PortraitPassword_B–F plus in-hub rooms): next session.

## Settled

- `cards_required` defaults to 40.
- Game name: **Harry Potter 3 PC**, after Harry Potter 2 PC, the first of the series in Archipelago.
- The grounds are always open: no Grounds Access item, the `GroundsBlocker` portcullises stay up.
- Traps: none designed yet. IDs 350–399 are reserved for them.
