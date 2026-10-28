# Apworld: items and options (foundation)

Code: `apworld/hp3pc/` (`items.py`, `options.py`, `world.py`). Locations, regions and rules aren't written yet, so
seeds can't be generated; the item table and options are in place. Decisions from 2026-10-26.

## Item tiers

| Tier | Items | Count |
| --- | --- | ---: |
| progression | Wizard cards (one item per card, "*Name* Card") | 79 |
| progression | Spells: Alohomora, Depulso, Lumos, Rictusempra, Spongify, Expecto Patronum | 6 |
| progression | Challenge spells: Carpe Retractum, Draconifors, Lapifors, Glacius (only with `split_challenge_spells`) | 4 |
| progression | Level unlocks: Carpe Retractum / Draconifors / Glacius / Monster Book / Hippogriff / Doxy Challenge; Hogwarts Express, Patronus Lesson, Library, Shrieking Shack, Paddock, Bonus Room; Ron's / Hermione's / Harry's Final Exam | 15 |
| progression | Portrait Password 1–8 (`pp_0`–`pp_7`, in the order the twins sell them) | 8 |
| progression_skip_balancing | Pumpkin pasty bundles 1, 2, 3, 4, 5, 10; cauldron cake bundles 1, 2, 3, 4 | per location |
| useful | Shortcut Passwords (`pp_8`–`pp_21` at once) | 1 |
| filler | Chocolate Frog (heals on receipt; the repeatable filler), Marauder's Map | — / 1 |

Not randomized (first pass): jellybeans, challenge stars (the game's challenge shields), the Harry Potter card.

IDs: `33_000_000` + a fixed offset (spells 1–10, unlocks 101–115, passwords 200–207 and 220, currency 301–310 and
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

Portrait passwords → portraits (mapping in progress): **1 → a secret room inside the castle (InsideHub, around (−500, 500, 1530))**, **4 → Portrait B**, **6 → Portrait C**, **3 → an area in the dungeons (DungeonHUB, around (290, 900, −690))**, **5 → the "Glover" portrait room in the dungeons (`PortraitPasswordTrigger1` at (−1255, 2340, −282); room around (−1630, 2270, −359))**, **2 → Portrait F**, **7 → Portrait D**, **8 → Portrait E**.

Rules collected so far (to go into `rules.py` with the locations):

| Region | Every check there needs |
| --- | --- |
| Carpe (Ch1 + Ch1B) | Carpe Retractum Challenge Unlock (door), **Carpe Retractum, Spongify, Depulso** (2026-10-27) |
| Portrait E | **Portrait Password 8**, and to enter any part of it **Alohomora, Depulso and Spongify** (2026-10-27). Its checks: 7 knights (Depulso) and one armoire with two spawners (below) |
| Portrait Password 3 area (Dungeons, around (290, 900, −690); the "Stroulger" portrait room: chests tagged `Stroulger_Poxie_Chest`) | **Portrait Password 3 + Spongify** for every check there (2026-10-27); everything past the doxy fight also needs **Alohomora + Rictusempra**. That includes the 10 snacks the fight spawns (`HPSpawnThingy3/5/7/11/12/14/20/21/23/27`, tag `Aspawnbeans`), collected in play 2026-10-28: Password 3 + Spongify + Alohomora + Rictusempra |
| Portrait Password 5 area (Dungeons) | **Portrait Password 5 + Lumos** for every check there (2026-10-27). Possible bypass to verify: `AlohomoraTrigger8` (−496, 2669, −280), near dungeon door 1, also opens `PortPass_Glover_Mvr` |
| Portrait B | **Portrait Password 4 + Lumos, Alohomora and Depulso** for everything there (2026-10-27) |
| Portrait C | **Portrait Password 6 + Depulso and Spongify** for everything there (2026-10-27) |
| Patronum (Adv2ExpPatronum) | **Patronus Lesson Unlock + Expecto Patronum** to start (2026-10-28); all three skull shelves need **Depulso** (2026-10-28): `SkullShelfSpwn2` (3 cakes, (2112, 1545)), `SkullShelfSpwn0` (3 pasties, (3058, 1671)), `SkullShelfSpwn1` (3 pasties, (3040, 2352)); the Honoria Nutcombe card (`HPSpawnThingy3`, second boggart wave) needs nothing beyond the area rule (2026-10-28). Area complete. |
| Bonus Room (BeanBonus) | **Bonus Room Unlock**. The ground floor needs no spell for the knights and the Hufflepuff / Ravenclaw cards; its four corner floor chests need **Depulso** on a switch. Everything from the Slytherin card up (the upper levels: Gryffindor and Dumbledore cards, the six `gargspawner1` chests, knights 3 and 5, the nine placed pasties) needs **Spongify** and nothing else (all collected in play 2026-10-28). Area complete. |
| Paddock (Adv6Paddock) | **Paddock Unlock + Depulso, Spongify and Draconifors** for every check there (2026-10-28). Area complete (2026-10-28). |
| Monster Book challenge (InsideHub, MBOM) | **Monster Book Challenge Unlock + Depulso + Rictusempra** (2026-10-28): its 5 reward cards (Baba Yaga, Cordelia Misericordia, Leticia Somnolens, Malodora Grymm, Old Mother Hubbard), at (−2650, −3770, 304) |
| Fred & George's shop (InsideHub) | The five `WC_BeatrixBloxam1`–`5` cards (around (−130, −2370, 2185)) are the shop's display props, the stand-ins for the cards on sale, not pickups: **not locations** (checked in play 2026-10-28) |
| Hogwarts (InsideHub), never spawned | **Not locations** (2026-10-28): their triggers never fire in Open Castle. The Peeves 2 rewards (`HPSpawnThingy13/14/16/18`, 4 pasties), the Peeves 3 rewards (`HPSpawnThingy22/23/25`, 3 cakes), and `CauldronCake4` / `PumpkinPasty5` (no position in the map data). Hogwarts complete |
| Hippogriff (COMC) | **Hippogriff Challenge Unlock** only (2026-10-28): the 5 course cards |
| Express (Adv1Express) | **Hogwarts Express Unlock + Alohomora, Depulso, Rictusempra and Lumos** (2026-10-28). Its one check: the Fulbert the Fearful card (`HPSpawnThingy0`, tag `SpawnWC1`, (5350, 11, −55)) |
| Portrait D | **Portrait Password 7** (confirmed in play 2026-10-27) **+ Alohomora** for everything there: the portrait itself, on an upper floor of the castle at (111, 540, 2026), sits behind padlock `AlohomoraPadLock16` (101, 105, 2022) (2026-10-28); everything past the freestanding cake also needs **Rictusempra, Spongify and Depulso** (2026-10-27): the two companion chests `ChestCompanionSpawner3` (Imp card, 10 pasties, 5 cakes) and `ChestCompanionSpawner0` (10 pasties, 15 cakes) |

| Check | Extra needs | Source |
| --- | --- | --- |
| Carpe: Sir Herbert Varney Card | Alohomora (chest `ChestSpawner18`, Ch1 (2736, 1140, 608); also 5 beans) | marked in play 2026-10-27 |
| Portrait D: chest `ChestCompanionSpawner0` (10 pasties, 15 cakes; also 45 beans) | Alohomora: three padlocks on it (`AlohomoraPadLock1` / `12` / `13`), on top of the Portrait D rule. At (998, −709, 2169) | marked in play 2026-10-27 |
| Portrait D: chest `ChestCompanionSpawner3` (Imp card, 10 pasties, 5 cakes; also 45 beans) | Alohomora: three padlocks (`AlohomoraPadLock3` / `4` / `5`), on top of the Portrait D rule. At (1621, 125, 2168) | marked in play 2026-10-27 |
| Bonus Room: knight `KnightSpawner6` (2 pasties, 2 cakes; also 10 beans) | None beyond the Bonus Room Unlock (armor, no spell). At (1089, 65, −424), tag `k1`, save key `bbKnightSpawner6_lives` | marked in play 2026-10-28 |
| Bonus Room: Rowena Ravenclaw card (`WC_RowenaRavenclaw0`) | None beyond the Bonus Room Unlock (no spell). At (1747, −1731, −492), save key `bbWC_RowenaRavenclaw0_PU` | marked in play 2026-10-28 |
| Bonus Room: knight `KnightSpawner8` (2 pasties, 2 cakes; also 10 beans) | None beyond the Bonus Room Unlock (armor, no spell). At (1550, -1345, -424), tag `khall1` | marked in play 2026-10-28 |
| Bonus Room: knight `KnightSpawner7` (2 pasties, 2 cakes; also 10 beans) | None beyond the Bonus Room Unlock (armor, no spell). At (1550, -957, -424), tag `khall2` | marked in play 2026-10-28 |
| Bonus Room: knight `KnightSpawner0` (2 pasties, 2 cakes; also 10 beans) | None beyond the Bonus Room Unlock (armor, no spell). At (1552, -574, -424), tag `khall3` | marked in play 2026-10-28 |
| Bonus Room: Helga Hufflepuff card (`WC_HelgaHufflepuff0`) | None beyond the Bonus Room Unlock (no spell). At (-448, -190, -492) | marked in play 2026-10-28 |
| Bonus Room: knight `KnightSpawner4` (2 pasties, 2 cakes; also 10 beans) | None beyond the Bonus Room Unlock (armor, no spell). At (-254, -575, -424), tag `khall4` | marked in play 2026-10-28 |
| Bonus Room: knight `KnightSpawner2` (2 pasties, 2 cakes; also 10 beans) | None beyond the Bonus Room Unlock (armor, no spell). At (-254, -963, -424), tag `khall5` | marked in play 2026-10-28 |
| Bonus Room: floor chest `ChestSpawner2` (2 cakes; also 10 beans) | Depulso on a nearby switch (event `floorplate` opens the chest), within the Bonus Room Unlock. At (1245, -1217, -456), tag `floorplate` | marked in play 2026-10-28 |
| Bonus Room: floor chest `ChestSpawner3` (2 cakes; also 10 beans) | Depulso on a nearby switch (event `floorplate` opens the chest), within the Bonus Room Unlock. At (1240, -701, -456), tag `floorplate` | marked in play 2026-10-28 |
| Bonus Room: floor chest `ChestSpawner0` (2 cakes; also 10 beans) | Depulso on a nearby switch (event `floorplate` opens the chest), within the Bonus Room Unlock. At (38, -705, -456), tag `floorplate` | marked in play 2026-10-28 |
| Bonus Room: floor chest `ChestSpawner1` (2 cakes; also 10 beans) | Depulso on a nearby switch (event `floorplate` opens the chest), within the Bonus Room Unlock. At (35, -1217, -456), tag `floorplate` | marked in play 2026-10-28 |
| Bonus Room: Salazar Slytherin card (`WC_SalazarSlytherin0`) | **Spongify**, within the Bonus Room Unlock. Everything above it (the upper levels) also needs Spongify. At (638, -958, -64) | marked in play 2026-10-28 |
| Bonus Room: knight `KnightSpawner1` (2 pasties, 2 cakes; also 10 beans) | None beyond the Bonus Room Unlock (armor, no spell; ground floor, not behind Spongify). At (-447, -1569, -424), tag `kside1` | marked in play 2026-10-28 |
| Bonus Room: knight `KnightSpawner5` (2 pasties, 2 cakes; also 10 beans) | Spongify to reach it (upper level), within the Bonus Room Unlock; armor needs no spell. At (1088, -1986, 119) | marked in play 2026-10-28 |
| Paddock: placed pasty `PumpkinPasty0` (group of four) | Draconifors, within the Paddock rule (Paddock Unlock + Depulso, Spongify, Draconifors). At (129, 928, 643) | marked in play 2026-10-28 |
| Paddock: placed pasty `PumpkinPasty4` (group of four) | Draconifors, within the Paddock rule (Paddock Unlock + Depulso, Spongify, Draconifors). At (190, 865, 643) | marked in play 2026-10-28 |
| Paddock: placed pasty `PumpkinPasty6` (group of four) | Draconifors, within the Paddock rule (Paddock Unlock + Depulso, Spongify, Draconifors). At (128, 800, 643) | marked in play 2026-10-28 |
| Paddock: placed pasty `PumpkinPasty7` (group of four) | Draconifors, within the Paddock rule (Paddock Unlock + Depulso, Spongify, Draconifors). At (66, 864, 643) | marked in play 2026-10-28 |
| Paddock: placed cake `CauldronCake0` (row of three) | Draconifors, within the Paddock rule. At (1265, 815, 936) | marked in play 2026-10-28 |
| Paddock: placed cake `CauldronCake2` (row of three) | Draconifors, within the Paddock rule. At (1344, 814, 937) | marked in play 2026-10-28 |
| Paddock: placed cake `CauldronCake7` (row of three) | Draconifors, within the Paddock rule. At (1417, 815, 936) | marked in play 2026-10-28 |
| Paddock: cauldron `CauldronSpwn1` (4 pasties, 2 cakes; also 10 beans) | Depulso, within the Paddock rule. At (1385, 1265, 45) | marked in play 2026-10-28 |
| Paddock: chest `ChestSpawner1` (4 pasties; also 10 beans) | Alohomora, within the Paddock rule. At (1565, 1058, 440) | marked in play 2026-10-28 |
| Paddock: crate `CrateSpwn1` (4 pasties, 1 cake; spawn event `poxiedis`) | Alohomora to reach it, within the Paddock rule. At (-1245, -157, 554) | marked in play 2026-10-28 |
| Paddock: Professor Armando Dippet card (cauldron `CauldronSpwn0`) | Depulso, plus Alohomora to reach the area, within the Paddock rule. At (-1290, 459, 452) | marked in play 2026-10-28 |
| Paddock: chest `ChestSpawner0` (4 pasties) | Alohomora to open it, plus Rictusempra and Lapifors to reach the area, within the Paddock rule. At (-325, -214, 441) | marked in play 2026-10-28 |
| Paddock: cauldron `CauldronSpwn4` (5 pasties) | Depulso, plus Rictusempra and Lapifors to reach the area, within the Paddock rule. At (-147, -318, 1249) | marked in play 2026-10-28 |
| Paddock: placed pasty `PumpkinPasty5` (low pair, found with noclip) | Alohomora, Rictusempra and Lapifors to reach it, within the Paddock rule. At (-583, -341, 44) | marked in play 2026-10-28 |
| Paddock: placed pasty `PumpkinPasty18` (low pair, found with noclip) | Alohomora, Rictusempra and Lapifors to reach it, within the Paddock rule. At (-683, -337, 53) | marked in play 2026-10-28 |
| Monster Book challenge: Babayaga card (`WC_Babayaga0`) | **Monster Book Challenge Unlock + Depulso + Rictusempra**. At (-2656, -3812, 304), tag `MBOM_Babayaga` | marked in play 2026-10-28 |
| Monster Book challenge: CordeliaMisericordia card (`WC_CordeliaMisericordia0`) | **Monster Book Challenge Unlock + Depulso + Rictusempra**. At (-2637, -3734, 304), tag `MBOM_CordeliaMisericordia` | marked in play 2026-10-28 |
| Monster Book challenge: LeticiaSomnolens card (`WC_LeticiaSomnolens0`) | **Monster Book Challenge Unlock + Depulso + Rictusempra**. At (-2647, -3774, 304), tag `MBOM_LeticiaSomnolens` | marked in play 2026-10-28 |
| Monster Book challenge: MalodoraGrymm card (`WC_MalodoraGrymm0`) | **Monster Book Challenge Unlock + Depulso + Rictusempra**. At (-2652, -3793, 304), tag `MBOM_MalodoraGrymm` | marked in play 2026-10-28 |
| Monster Book challenge: OldMotherHubbard card (`WC_OldMotherHubbard0`) | **Monster Book Challenge Unlock + Depulso + Rictusempra**. At (-2642, -3753, 304), tag `MBOM_OldMotherHubbard` | marked in play 2026-10-28 |
| Hogwarts: chest `ChestSpawner4` (3 pasties; also 10 beans) | **Alohomora** to open, **Lumos and Spongify** to reach it. At (-1369, -1581, 1120) | marked in play 2026-10-28 |
| Hogwarts: chest `ChestSpawner14` (3 pasties; also 10 beans; tag `2ndFlr_BookBtlBkCs_Chest_02`) | **Alohomora + Depulso**. At (-51, -1682, 632), tag `2ndFlr_BookBtlBkCs_Chest_02` | marked in play 2026-10-28 |
| Hogwarts: secret area `SecretAreaMarker1` (tied to chest `ChestSpawner14`) | **Alohomora + Depulso**, same as the chest it goes with. At (-288, -1088, 760) | marked in play 2026-10-28 |
| Hogwarts: chest `ChestSpawner17` (2 pasties; also 6 beans; tag `5thFlrScrt_Chest`) | **Alohomora**. At (-3653, -35, 1541), tag `5thFlrScrt_Chest` | marked in play 2026-10-28 |
| Hogwarts: secret area `SecretAreaMarker3` (5th floor, with chest `ChestSpawner17`) | **Alohomora**, same as the chest it goes with. At (-3317, -35, 1591), tag `IH02` | marked in play 2026-10-28 |
| Hogwarts: secret area `SecretAreaMarker4` (upper floor, next to bean chest `ChestSpawner18`) | **Alohomora + Spongify**. At (482, -797, 2378) | marked in play 2026-10-28 |
| Hogwarts: secret area `SecretAreaMarker2` (tag `IH03`) | **Depulso + Spongify**. At (-672, 1004, 859), tag `IH03` | marked in play 2026-10-28 |
| Hogwarts: floating skull `SkullFloatSpwn7` (2 pasties; also 5 beans) | **Depulso + Alohomora**. At (-1773, -927, 894) | marked in play 2026-10-28 |
| Grounds: secret area `SecretAreaMarker1` | **Depulso + Spongify**. At (-846, -5083, 1893) | marked in play 2026-10-28 |
| Grounds: Kelpie card (`WC_Kelpie0`) | **Depulso + Spongify + Alohomora**. At (-832, -4344, 1808) | marked in play 2026-10-28 |
| Grounds: wardrobe `ArmoireSpwn0` (3 pasties) | **Depulso + Spongify + Alohomora**. At (-1048, -4792, 1784) | marked in play 2026-10-28 |
| Grounds: wardrobe `ArmoireSpwn1` (3 pasties) | **Depulso + Spongify + Alohomora**. At (-610, -4790, 1784) | marked in play 2026-10-28 |
| Grounds: secret area `SecretAreaMarker2` (end of the x≈450 path) | **Depulso + Spongify**. At (447, -6955, 1335) | marked in play 2026-10-28 |
| Grounds: Double-Ended Newt card (`WC_DoubleEndedNewt0`, x≈450 path) | **Depulso + Spongify**. At (456, -5285, 1325) | marked in play 2026-10-28 |
| Grounds: placed cake `CauldronCake1` (x≈450 path) | **Depulso + Spongify**. At (426, -5327, 1316) | marked in play 2026-10-28 |
| Grounds: placed cake `CauldronCake2` (x≈450 path) | **Depulso + Spongify**. At (481, -5558, 1316) | marked in play 2026-10-28 |
| Grounds: placed cake `CauldronCake3` (x≈450 path) | **Depulso + Spongify**. At (414, -5397, 1316) | marked in play 2026-10-28 |
| Grounds: placed cake `CauldronCake4` (x≈450 path) | **Depulso + Spongify**. At (458, -5467, 1316) | marked in play 2026-10-28 |
| Grounds: placed pasty `PumpkinPasty18` (x≈450 path) | **Depulso + Spongify**. At (434, -5707, 1316) | marked in play 2026-10-28 |
| Grounds: placed pasty `PumpkinPasty19` (x≈450 path) | **Depulso + Spongify**. At (441, -5832, 1316) | marked in play 2026-10-28 |
| Grounds: placed pasty `PumpkinPasty20` (x≈450 path) | **Depulso + Spongify**. At (474, -5639, 1316) | marked in play 2026-10-28 |
| Grounds: placed pasty `PumpkinPasty21` (x≈450 path) | **Depulso + Spongify**. At (466, -5873, 1316) | marked in play 2026-10-28 |
| Grounds: placed pasty `PumpkinPasty22` (x≈450 path) | **Depulso + Spongify**. At (429, -5776, 1316) | marked in play 2026-10-28 |
| Grounds: Giant Purple Toad card (`WC_GiantPurpleToad0`) | **Alohomora + Lumos + Spongify**. At (-204, -7440, 1236) | marked in play 2026-10-28 |
| Grounds: secret area `SecretAreaMarker3` (with the Giant Purple Toad card) | **Alohomora + Lumos + Spongify**. At (-228, -7437, 1256) | marked in play 2026-10-28 |
| Grounds: Phoenix card (chest `ChestSpawner0`, spawned by `WheelPuzzlePrize`) | **Depulso**. At (-3711, -7424, 1069), tag `WheelPuzzlePrize` | marked in play 2026-10-28 |
| Grounds: secret area `SecretAreaMarker5` (tag `BridgeSecrets`) | **Lumos**. At (-832, -3076, 1279), tag `BridgeSecrets` | marked in play 2026-10-28 |
| Grounds: placed cake `CauldronCake5` | Nothing (no spell). At (801, -2758, 911) | marked in play 2026-10-28 |
| Grounds: Gytrash card (`WC_Gytrash0`) | Nothing (no spell). At (960, -1596, 676) | marked in play 2026-10-28 |
| Grounds: secret area `SecretAreaMarker0` (with the Gytrash card) | Nothing (no spell). At (959, -1598, 696) | marked in play 2026-10-28 |
| Grounds: placed cake `CauldronCake0` | Nothing (no spell). At (-2426, -2738, 884) | marked in play 2026-10-28 |
| Grounds: scarecrow `Scarecrow0` (2 cakes; spawn event `CompScareCrow`) | **Depulso**. At (1126, -491, 647), tag `CompScareCrow` | marked in play 2026-10-28 |
| Grounds: pumpkin `Pumpkin3` (2 pasties; the only pasty pumpkin in the patch, the rest drop beans) | **Depulso** (Pumpkin.vulnerableToClass = DepulsoSpell). At (902, -609, 590) | marked in play 2026-10-28 |
| Portrait E: knight `KnightSpawner1` (10 pasties) | Depulso, within the Portrait E rule. At (−1643, −1069, 2132), save key `ppeKnightSpawner1_lives` | marked in play 2026-10-27 |
| Portrait E: knight `KnightSpawner2` (5 pasties; also 10 beans) | Depulso, within the Portrait E rule. At (−836, −1526, 2142), save key `ppeKnightSpawner2_lives` | marked in play 2026-10-27 |
| Portrait E: knight `KnightSpawner4` (5 cakes) | Depulso, within the Portrait E rule. At (−2108, −1537, 2133), save key `ppeKnightSpawner4_lives` | marked in play 2026-10-27 |
| Portrait E: knight `KnightSpawner5` (10 cakes, 5 pasties; also 25 beans) | Depulso, within the Portrait E rule. Upper level, at (−1932, −1177, 2741), right above the marker (2346); save key `ppeKnightSpawner5_lives` | marked in play 2026-10-27 |
| Portrait E: knights `KnightSpawner3` (5 cakes, 10 pasties; 25 beans), `KnightSpawner6` (10 cakes; 20 beans), `KnightSpawner7` (10 cakes; 20 beans) | Depulso, within the Portrait E rule. Top level (z 3078–3081), about 415 units above the marker at (−1476, −1019, 2698); save keys `ppeKnightSpawner3/6/7_lives` | marked in play 2026-10-27 |
| Portrait E: armoire `ArmoireSpwn2` (Unicorn card) | Alohomora, within the Portrait E rule. At (−1472, −2957, 1816); opening it starts a flying-book fight (19 `FlyingBook`s, Event `FCount1`) | marked in play 2026-10-27 |
| Portrait E: armoire `ArmoireSpwn0` (10 pasties; also 100 beans) | the book fight won: the same armoire, a second spawner (Tag `OpenPortcullisA`) fired when the last book falls. Spell for the books: **Depulso** (`FlyingBook.vulnerableToClass = DepulsoSpell`, from the decompiled class; already in the Portrait E rule) | marked in play 2026-10-27 |
| (not a location) Portrait F knight `KnightSpawner7` (pasties) | Depulso. Rolls 6–14 pasties per hit, 2 hits, so it stays vanilla under the no-random-amounts rule. At (−3521, −776, 1608) | marked in play 2026-10-27 |
| Portrait F: chest `ChestSpawner34` (Manticore card, 8 cakes) | Alohomora, plus Portrait Password 2. At (−3990, −580, 1680), save key `ppfChestSpawner34_lives` | marked in play 2026-10-27 |
| Portrait F: Secret Area (`SecretAreaMarker0`, (−3400, −559, 1749)) | same as the chest `ChestSpawner34`: Alohomora, plus Portrait Password 2 (found together in play) | marked in play 2026-10-27 |
| Hogwarts: chest `ChestSpawner3` (Giant Squid card, 15 pasties) | Portrait Password 1 (the room behind its portrait) + Alohomora. At (−493, 483, 1524), save key `ihubChestSpawner3_lives` | marked in play 2026-10-27 |
| Hogwarts: chest `ChestSpawner0` (15 pasties, 8 cakes) | Portrait Password 1 (same secret room) + Alohomora. At (−754, 635, 1480), save key `ihubChestSpawner0_lives` | marked in play 2026-10-27 |
| Hogwarts: 4th-floor secret Spongify column: pasty (`HPSpawnThingy2`, z 1657), pasty (`HPSpawnThingy4`, z 1742), cake (`HPSpawnThingy1`, z 1842) at about (−545, 637) | Portrait Password 1 (same secret room) + **Alohomora + Spongify**. Opening the Giant Squid chest releases them (confirmed in the map: `AlohomoraTrigger0` → `4thFlr_SecretSpong_Disp` → `Dispatcher26` → `4thFlr_SecretSpong_Spwn_01`–`03`, the `HPSpawnThingy`s; the Spongify pad `SpongifyPad6` → target `SpongifyTarget13` reaches them). No save key of their own: check detection for spawned pickups is still to design | marked in play 2026-10-27 |
| Dungeons: chest `ChestSpawner7` (2 pasties; also 10 beans) | the Password 3 area rule + Alohomora, cast twice (`Lives` 2: two openings). At (291, 764, −694), save key `dungChestSpawner7_lives` (the check fires on the first opening or when Lives reaches 0: to decide) | marked in play 2026-10-27 |
| Dungeons: chest `ChestSpawner8` (1 cake; also 10 beans) | the Password 3 area rule + Alohomora, cast twice. At (−1056, 960, −951), save key `dungChestSpawner8_lives` | marked in play 2026-10-27 |
| Dungeons: Doxy Card (placed `WC_Doxy0`) | the Password 3 area rule (Portrait Password 3 + Spongify). At (−869, 1627, −430), save key `dungWC_Doxy0_PU` | marked in play 2026-10-27 |
| Dungeons: the doxy-fight reward, every `HPSpawnThingy` tagged `Aspawnbeans`: pasties `HPSpawnThingy16` (290, 1120, −509), `25` (−739, 961, −698), `26` (−865, 1050, −744), `28`, `30`; cakes `19`, `29` (the line at x ≈ −864, y 1191–1410); plus beans | the Password 3 area rule + **Alohomora** (opening `ChestSpawner0` / `ChestSpawner9`, tags `Stroulger_Poxie_Chest`/`2`, releases the doxies) + **Rictusempra** (beat them). Map: `Counter1` (`Apoxiecounter`, 6) → `Aspawnbeans`. Event-spawned, no save key of their own: detect from the counter / event | marked in play 2026-10-27 |
| Dungeons: chest `ChestSpawner5` (4 pasties) | the Password 5 area rule (Portrait Password 5 + Lumos) + Alohomora. At (−1628, 2272, −359), save key `dungChestSpawner5_lives` | marked in play 2026-10-27 |
| Dungeons: the firecrab line of pasties, `HPSpawnThingy54`–`74` (tags `Bs2`–`Bs22`; 9 pasties in the catalogue, the rest beans) along x ≈ −2207, y 2082–4095 | the Password 5 area rule + **Rictusempra** (the firecrabs). Map: five firecrabs `Bcrab1`–`5`; crabs 2+3 → `Counter2` (`Bcrabcounter`) → `Bdoordis` → the `Bbridge…` dispatchers, crabs 4+5 → `Counter0` (`Bcrabcounter2`) → `Bbridgedis` → more; those dispatchers fire the `Bs…` spawns (and raise a bridge). Event-spawned, no save key: detect from the counters | marked in play 2026-10-27 |
| Dungeons: chest `ChestSpawner3` (Bowtruckle card, 4 cakes) | the Password 5 area rule (Portrait Password 5 + Lumos) + **Rictusempra** (past the firecrabs) + Alohomora. At (−1248, 4389, −359), save key `dungChestSpawner3_lives` | marked in play 2026-10-27 |
| Portrait B: the 12 placed cakes (`CauldronCake0`, `2`–`11`, `13`; line along y ≈ −3460, x −6 to 843, z 63; save keys `ppbCauldronCake<n>_PU`) | the Portrait B rule only | collected in play 2026-10-27 |
| Portrait B: chest `ChestSpawner12` (Billywig card; also 20 beans) | the Portrait B rule (Alohomora included). At (1177, −3457, 97), save key `ppbChestSpawner12_lives` | marked in play 2026-10-27 |
| Portrait B: 3 event pasties (`SpawnThingy0` / `2` / `3`, tag `BsecJelly`, around (−880, −3460, 224)) | the Portrait B rule (smashing the six shield knights is Depulso). Map: the knights (`3KnA`–`F`) count up `Counter0` (`CompanionKnightsSmash`, 6) → `SecretStairsBB` raises the secret stairs; stair mover `Mover3` → `ExitSecretB` … → `Dispatcher0` (`ExitSecretC`) → `MtriggerA`–`F` and `BsecJelly` (the pasties). Event-spawned, no save key: detect from the counter / dispatcher | marked in play 2026-10-27 |
| Portrait C: chest `ChestSpawner0` (4 pasties; also 30 beans) | the Portrait C rule + Alohomora. At (−3296, −1182, 1208), save key `ppcChestSpawner0_lives` | marked in play 2026-10-27 |
| Portrait C: chest `ChestSpawner1` (10 pasties; also 15 beans) | the Portrait C rule + Alohomora. At (−3296, −2015, 1208), save key `ppcChestSpawner1_lives` | marked in play 2026-10-27 |
| Portrait C: Gnome Card (placed `WC_Gnome0`, (−4000, −1597, 1850)) and the 4 cakes beside it (`HPSpawnThingy1` / `4` / `6` / `8`, tag `Cspongyplate3`, z 1611–1926) | the Portrait C rule + **Rictusempra** (the firecrabs). Map: firecrabs `Cfirecrab2` / `3` → `Counter0` (`Cfirecrabcounter`) → `Dispatcher0` (`Cspongy3dis`) → opens `Cfirecrabgate` / `Cfirecrabgate1` and spawns the cakes (`Cspongyplate3`). The card is placed (save key `ppcWC_Gnome0_PU`); the cakes are event-spawned (detect from the counter) | marked in play 2026-10-27 |
| Portrait D: Cauldron Cake | only Portrait Password 7 (opens Portrait D). Freestanding `CauldronCake4` at (897, 139, 2178), save key `ppdCauldronCake4_PU` | marked in play 2026-10-27 |

For logic tracing the challenge doors and the spells are separate unlocks in the mod and the debug panel; the
apworld couples them by default (`split_challenge_spells: false`).

## To do: repeatable event rewards (decision deferred, 2026-10-27)

Event-spawned rewards (an `HPSpawnThingy` fired by a counter or dispatcher) have no save record, so the fight and
its reward reset on every visit. Confirmed in play: re-entering Portrait C, the chests and armour stayed empty but
beating the firecrabs again spawned the 4 cakes again (the placed Gnome card stayed gone).

- Plan: the check fires once, on the first completion in a save (an `OC_Loc_<id>` flag, see
  [CLIENT_SYNC.md](CLIENT_SYNC.md)); the first reward is the check, not the vanilla spawn.
- **Open decision:** what repeat completions give. (a) nothing: the mod turns the spawns off once the check is
  taken; (b) the vanilla currency again: a farm for pasties / cakes, which softens normal shop prices (logic never
  counts on it).
- Verify per reward which ones repeat: Stroulger doxies (counter: probably), Glover firecrab line (counters:
  probably), Portrait C cakes (**confirmed**), Portrait B pasties (knights may stay smashed), 4th-floor Spongify
  column (Giant Squid chest's Alohomora trigger: may fire once), Carpe imp pasties (first-visit cutscene: once per
  save).

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

- None right now.

## Mod tooling to-do

- **Companions ignore spell locks** (seen in the Paddock doxy fight, 2026-10-28). The lock is `WeaponWand.HasSpell`, which
  only the player's targeting calls. A companion in a fight (`HPCompanionCombatController`, state `stateShootAtTarget`)
  calls `HPCharacter.castSpell(target, , target.vulnerableToClass)` directly. Each `PoxieManager` sets how many
  creatures the companions kill per wave (`CompanionNumToDestroyPerWave`). Paddock sets 2, so the player still has to
  finish each wave. But the class default is 999, and managers that don't override it let the companions clear the
  whole fight alone: the Carpe, Carpe B, Glacius and Glacius B doxy groups, the two Stroulger (Portrait Password 3)
  doxy fights, and Library `BookBattleB_CantDoThis`. The imp fights (`ImpCombatManager`) have no such setting at all.
  Doxies and imps need Rictusempra (`RictusempraSpell_Combat`); flying books need Depulso. **To do:** companions
  can't cast a locked spell either (proposed: cancel a companion's spell of a locked class when it's cast).
- ~~Noclip toggle~~: built in poc8h (F6 / debug panel), 100x faster since poc8j; works in play (found the Paddock pasties).
- ~~Live coordinates in the window title bar~~: MapTracker v13 (X, Y, Z and facing).

## Settled

- `cards_required` defaults to 40.
- Game name: **Harry Potter 3 PC**, after Harry Potter 2 PC, the first of the series in Archipelago.
- The grounds are always open: no Grounds Access item, the `GroundsBlocker` portcullises stay up.
- Traps: none designed yet. IDs 350–399 are reserved for them.
