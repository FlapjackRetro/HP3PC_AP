# Logic to-do: item spawns without a definitive logic requirement

Generated 2026-10-28 from `data/hp3_location_catalog.csv` against the logic notes in [APWORLD.md](APWORLD.md).
Candidate locations: placed pasties / cakes / cards, event-spawned pasties / cakes, containers with a fixed pasty /
cake / card drop, and secret areas. Left out: random-amount containers (not locations), bean-only containers,
challenge stars (not randomized), bundimuns (exempt: their behaviour isn't changed), shops and vendors (logic = shop access + currency; this includes the five
`WC_BeatrixBloxam` cards, which are display props in Fred & George's shop), and spawns whose trigger never fires in
Open Castle (Hogwarts: the Peeves 2 / 3 rewards, `CauldronCake4`, `PumpkinPasty5`), and the five cards in the
Grounds' developer room (Bran the Bloodthirsty, Cyclops, Goliath, Hengist of Upper Barnton, Morholt), and
the Library's out-of-bounds copy of the grounds (a cutscene set: secrets 2, 4, 5, 6 and a Double-Ended Newt card), and the half of each Glacius map
that the player is never in (Ch3 and Ch3B share the middle of the level; see APWORLD.md), Glacius
`PumpkinPasty36` (exempt), Glacius B's and Draconifors B's bean bonus rooms (exempt; 21 and 30 spawns), Carpe event cake `HPSpawnThingy3` (exempt: only the first-visit doxy cutscene spawns it), four Glacius B slide pasties (`PumpkinPasty22`, `23`, `24`, `6`: exempt for now, believed reachable but
unconfirmed; see APWORLD.md), and the dropped maps
(Quidditch, WhompingWillow, BuckyExecuted, DementorBattle, Infirmary, DarkTower, GroundsDada, Final).

- **Partly known:** 51 (a region rule or an event chain covers them, but they weren't reported spot by spot).
- **Nothing recorded:** 0.

## Partly known

| Area | Actor | What | Where | Status |
| --- | --- | --- | --- | --- |
| Hermione FE | `CauldronCake15` | cake (placed) | (2511, -1222, 1725) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `CauldronCake20` | cake (placed) | (6186, 1091, 1642) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `CauldronCake21` | cake (placed) | (7070, 1091, 3348) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `CauldronCake22` | cake (placed) | (7075, -21, 3631) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `CauldronCake23` | cake (placed) | (5704, -771, 1389) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty0` | pasty (placed) | (7153, -62, 1629) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty10` | pasty (placed) | (4720, 1177, 1653) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty11` | pasty (placed) | (5073, 984, 1653) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty12` | pasty (placed) | (5598, 1089, 1832) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty13` | pasty (placed) | (5951, 1084, 1918) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty14` | pasty (placed) | (6458, 1089, 1757) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty15` | pasty (placed) | (2623, -1218, 1715) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty16` | pasty (placed) | (6989, -64, 1629) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty17` | pasty (placed) | (6986, 30, 1629) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty18` | pasty (placed) | (7069, 79, 1629) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty19` | pasty (placed) | (7151, 36, 1629) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty20` | pasty (placed) | (7072, 611, 2816) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty21` | pasty (placed) | (6103, -17, 3215) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty22` | pasty (placed) | (6987, 1132, 2836) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty23` | pasty (placed) | (7071, 1181, 2836) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty24` | pasty (placed) | (7152, 1138, 2836) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty25` | pasty (placed) | (7155, 1040, 2836) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty26` | pasty (placed) | (7070, -498, 1504) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty27` | pasty (placed) | (6660, -894, 1629) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty28` | pasty (placed) | (6303, -831, 1679) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty29` | pasty (placed) | (3168, -1216, 1510) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty30` | pasty (placed) | (5263, -386, 1679) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty31` | pasty (placed) | (4913, -322, 1679) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty32` | pasty (placed) | (4542, -961, 1679) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty33` | pasty (placed) | (2811, -1218, 1633) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty34` | pasty (placed) | (2752, -1217, 1667) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty35` | pasty (placed) | (2683, -1216, 1692) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty39` | pasty (placed) | (2241, 161, 2185) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty40` | pasty (placed) | (2498, 160, 2185) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty41` | pasty (placed) | (1984, 159, 2185) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty42` | pasty (placed) | (1824, 322, 2185) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty43` | pasty (placed) | (1821, 699, 2185) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty44` | pasty (placed) | (1986, 866, 2185) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty45` | pasty (placed) | (2239, 865, 2185) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty46` | pasty (placed) | (2501, 863, 2185) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty47` | pasty (placed) | (2642, 509, 2219) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty48` | pasty (placed) | (4371, 511, 2002) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty49` | pasty (placed) | (6991, 1038, 2836) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty5` | pasty (placed) | (2831, 509, 1755) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty50` | pasty (placed) | (7071, -114, 1629) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty51` | pasty (placed) | (5947, -898, 1679) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty54` | pasty (placed) | (7073, 988, 2836) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty6` | pasty (placed) | (3313, 522, 1907) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty7` | pasty (placed) | (3668, 511, 2013) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty8` | pasty (placed) | (4015, 510, 1827) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |
| Hermione FE | `PumpkinPasty9` | pasty (placed) | (4547, 731, 1757) | area rule only (Hermione's Final Exam Unlock + Lapifors; past the cauldron also Draconifors); extra needs not reported |

## Nothing recorded

| Area | Count |
| --- | ---: |
