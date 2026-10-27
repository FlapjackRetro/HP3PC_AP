# Apworld: items and options (foundation)

Code: `apworld/hp3pc/` (`items.py`, `options.py`, `world.py`). Locations, regions and rules aren't written yet, so
seeds can't be generated; the item table and options are in place. Decisions from 2026-10-26.

## Item tiers

| Tier | Items | Count |
| --- | --- | ---: |
| progression | Wizard cards (one item per card, "*Name* Card") | 79 |
| progression | Spells: Alohomora, Depulso, Lumos, Rictusempra, Spongify, Expecto Patronum | 6 |
| progression | Challenge spells: Carpe Retractum, Draconifors, Lapifors, Glacius (only with `split_challenge_spells`) | 4 |
| progression | Level unlocks: Carpe Retractum / Draconifors / Glacius / Monster Book / Hippogriff / Doxy Challenge; Hogwarts Express, Patronus Lesson, Library, Shrieking Shack, Paddock, Bonus Room | 12 |
| progression | Portrait Password 1–8 (`pp_0`–`pp_7`, in the order the twins sell them) | 8 |
| progression_skip_balancing | Pumpkin pasty bundles 1, 2, 3, 4, 5, 10; cauldron cake bundles 1, 2, 3, 4 | per location |
| useful | Shortcut Passwords (`pp_8`–`pp_21` at once) | 1 |
| filler | Chocolate Frog (heals on receipt; the repeatable filler), Marauder's Map | — / 1 |

Not randomized (first pass): jellybeans, challenge stars (the game's challenge shields), the Harry Potter card.

IDs: `33_000_000` + a fixed offset (spells 1–10, unlocks 101–112, passwords 200–207 and 220, currency 301–310 and
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
- The Harry Potter card check stays vanilla: Fred & George give it once 74 cards are found (all but Harry Potter
  and the five in the Wizard Card bonus room). In Open Castle it no longer opens the room (Bonus Room Unlock does).

## Other options

| Option | Values | |
| --- | --- | --- |
| `shop_prices` | normal / cheap | cheap = 50% rounded up ([SHOP.md](SHOP.md)) |
| `shortcut_passwords` | item / start_unlocked | default item (in the pool, useful) |

Slot data: all six options.

## Location naming (draft, 2026-10-27)

`<Area>: <Thing>`; things that repeat in an area are numbered in a fixed order (`Glacius: Pumpkin Pasty 17`,
`Grounds: Chest 4`); a card is named by the card (`Hippogriff: Joscelind Wadcock Card`). Location names are what
Archipelago shows as "found X at …"; item names stay plain.

| Prefix | Maps / checks |
| --- | --- |
| Express | Adv1Express |
| Tunnel | Adv5Shack |
| Paddock | Adv6Paddock |
| Hippogriff | COMC (5 course cards) |
| Doxy | the 5 doxy rounds (GroundsHub well) |
| MonsterBook | the 5 Monster Book rounds (InsideHub) |
| Harry FE / Hermione FE / Ron FE | FA3Harry / FA2Hermione / FA1Ron |
| Carpe | Ch1 + Ch1B |
| Drac/Lap | Ch2 + Ch2B |
| Glacius | Ch3 + Ch3B |
| Patronum | Adv2ExpPatronum |
| Hogwarts | InsideHub (not the shop or the Monster Book rounds) |
| Dungeons | DungeonHUB |
| Grounds | GroundsHub (not the doxy rounds) |
| Library | Adv3Library |
| Portrait B–F | PortraitPassword_B–F (letters, so they don't read as Portrait Password 1–8) |
| F&G Shop | Fred & George's 6 cards and 8 passwords |
| Vendor | student vendors: one shared, ordered pool (see below) |
| (open) | HP3_BeanBonus, the Wizard Card bonus room: 5 placed cards (Dumbledore and the four founders); its own level unlock since poc8b |

Wizard Card bonus room (HP3_BeanBonus), confirmed in the map data: getting the Harry Potter card from Fred & George
(`VI_FinalWizardCard0`, 7th floor) fires `GotCardEvent` = `WCBonusRoom_Trig_Disp` → `Dispatcher10` → after 3 s
`WCBonusRoom_Trig` = InsideHub `ChangeLevelTrigger14` (triggered, not walked into) → HP3_BeanBonus. Its exit,
`ChangeLevelTrigger1`, goes back to HP3_InsideHub. The Harry Potter card itself unlocks at
`WizardCardManager.IsFinalCardAvailable()`: found ≥ `Cards.Length − 6` = **74**, i.e. every card except Harry Potter
and the five that are in the bonus room. (Earlier notes said 79: wrong.)

**Open Castle decouples the room from the cards (2026-10-27):** the room is behind the entrance hall's south door
(lock bar `StaticMeshActor50`, door key `BonusRoom`), unlocked by the **Bonus Room Unlock** item (a drawn key).
The vanilla teleport is switched off (`ChangeLevelTrigger14` loses its Tag `WCBonusRoom_Trig`), so getting the
Harry Potter card no longer moves the player. The room's exit returns to the south door.

Student vendors: every student vendor sells `WizardCardManager.GetAvailableVendorCard()`, the first card in the
table with status `AvailableToVendors` whose game state has been reached; buying one marks it owned. So the pool is
shared and sold in a fixed order, whichever student you buy from (20 cards, plus the four adventure-level cards
(`AvailableInAdventureLevel`) that drop into the pool once the game state passes theirs, which in Open Castle's
`GState090` is at once).

## Logic notes

Portrait passwords → portraits (mapping in progress): **7 → Portrait D**, **8 → Portrait E**.

Rules collected so far (to go into `rules.py` with the locations):

| Region | Every check there needs |
| --- | --- |
| Carpe (Ch1 + Ch1B) | Carpe Retractum Challenge Unlock (door), **Carpe Retractum, Spongify, Depulso** (2026-10-27) |
| Portrait E | **Portrait Password 8**, and to enter any part of it **Alohomora, Depulso and Spongify** (2026-10-27). Its checks are 7 knights and 2 armoires (Depulso anyway), one armoire with the Unicorn card |
| Portrait D | **Portrait Password 7** (confirmed in play 2026-10-27); everything past the freestanding cake also needs **Rictusempra, Spongify and Depulso** (2026-10-27): the two companion chests `ChestCompanionSpawner3` (Imp card, 10 pasties, 5 cakes) and `ChestCompanionSpawner0` (10 pasties, 15 cakes) |

| Check | Extra needs | Source |
| --- | --- | --- |
| Carpe: Sir Herbert Varney Card | Alohomora (chest `ChestSpawner18`, Ch1 (2736, 1140, 608); also 5 beans) | marked in play 2026-10-27 |
| Portrait D: chest `ChestCompanionSpawner0` (10 pasties, 15 cakes; also 45 beans) | Alohomora: three padlocks on it (`AlohomoraPadLock1` / `12` / `13`), on top of the Portrait D rule. At (998, −709, 2169) | marked in play 2026-10-27 |
| Portrait D: chest `ChestCompanionSpawner3` (Imp card, 10 pasties, 5 cakes; also 45 beans) | Alohomora: three padlocks (`AlohomoraPadLock3` / `4` / `5`), on top of the Portrait D rule. At (1621, 125, 2168) | marked in play 2026-10-27 |
| Portrait E: knight `KnightSpawner1` (10 pasties) | Depulso, within the Portrait E rule. At (−1643, −1069, 2132), save key `ppeKnightSpawner1_lives` | marked in play 2026-10-27 |
| Portrait E: knight `KnightSpawner2` (5 pasties; also 10 beans) | Depulso, within the Portrait E rule. At (−836, −1526, 2142), save key `ppeKnightSpawner2_lives` | marked in play 2026-10-27 |
| Portrait E: knight `KnightSpawner4` (5 cakes) | Depulso, within the Portrait E rule. At (−2108, −1537, 2133), save key `ppeKnightSpawner4_lives` | marked in play 2026-10-27 |
| Portrait E: knight `KnightSpawner5` (10 cakes, 5 pasties; also 25 beans) | Depulso, within the Portrait E rule. Upper level, at (−1932, −1177, 2741), right above the marker (2346); save key `ppeKnightSpawner5_lives` | marked in play 2026-10-27 |
| Portrait D: Cauldron Cake | only Portrait Password 7 (opens Portrait D). Freestanding `CauldronCake4` at (897, 139, 2178), save key `ppdCauldronCake4_PU` | marked in play 2026-10-27 |

For logic tracing the challenge doors and the spells are separate unlocks in the mod and the debug panel; the
apworld couples them by default (`split_challenge_spells: false`).

## Client sync

How received items reach the game and how saves are reconciled with the server: [CLIENT_SYNC.md](CLIENT_SYNC.md).

## Mod work this implies

- **Bean bonus room:** challenge stars stay vanilla, so finding all of a challenge's shields still starts the
  challenge's bean bonus round (`BeanBonusGame` inside the Ch1B / Ch2B / Ch3B map, not the separate HP3_BeanBonus
  map). Allow it, and when it ends (`…BeanBonus…EXIT` → the chamber copy in the B map) send the player to InsideHub
  instead of the vanilla next map.
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
