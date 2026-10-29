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

- **Partly known:** 4 (a region rule or an event chain covers them, but they weren't reported spot by spot).
- **Nothing recorded:** 62.

## Partly known

| Area | Actor | What | Where | Status |
| --- | --- | --- | --- | --- |
| Ron FE | `CauldronCake0` | cake (placed) | (5103, -3108, 1328) | area rule only (Ron's Final Exam Unlock + Carpe Retractum); extra needs not reported |
| Ron FE | `CauldronCake1` | cake (placed) | (5200, -3150, 1328) | area rule only (Ron's Final Exam Unlock + Carpe Retractum); extra needs not reported |
| Ron FE | `CauldronCake2` | cake (placed) | (5103, -3205, 1328) | area rule only (Ron's Final Exam Unlock + Carpe Retractum); extra needs not reported |
| Ron FE | `ChestSpawner9` | ChestSpawner: cakex8 | (2614, -3259, 1349) | area rule only (Ron's Final Exam Unlock + Carpe Retractum); extra needs not reported |

## Nothing recorded

| Area | Count |
| --- | ---: |
| Hermione FE (`HP3_FA2Hermione`) | 62 |

### Hermione FE (`HP3_FA2Hermione`): 62

| Actor | What | Where | Tag |
| --- | --- | --- | --- |
| `CauldronCake1` | cake (placed) | (982, -2363, 1112) |  |
| `CauldronCake13` | cake (placed) | (1854, -1644, 1899) |  |
| `CauldronCake15` | cake (placed) | (2511, -1222, 1725) |  |
| `CauldronCake20` | cake (placed) | (6186, 1091, 1642) |  |
| `CauldronCake21` | cake (placed) | (7070, 1091, 3348) |  |
| `CauldronCake22` | cake (placed) | (7075, -21, 3631) |  |
| `CauldronCake23` | cake (placed) | (5704, -771, 1389) |  |
| `CauldronSpwn0` | CauldronSpwn: pastyx4 | (1269, -953, 1357) | `Bshieldplatb` |
| `PumpkinPasty0` | pasty (placed) | (7153, -62, 1629) |  |
| `PumpkinPasty10` | pasty (placed) | (4720, 1177, 1653) |  |
| `PumpkinPasty11` | pasty (placed) | (5073, 984, 1653) |  |
| `PumpkinPasty12` | pasty (placed) | (5598, 1089, 1832) |  |
| `PumpkinPasty13` | pasty (placed) | (5951, 1084, 1918) |  |
| `PumpkinPasty14` | pasty (placed) | (6458, 1089, 1757) |  |
| `PumpkinPasty15` | pasty (placed) | (2623, -1218, 1715) |  |
| `PumpkinPasty16` | pasty (placed) | (6989, -64, 1629) |  |
| `PumpkinPasty17` | pasty (placed) | (6986, 30, 1629) |  |
| `PumpkinPasty18` | pasty (placed) | (7069, 79, 1629) |  |
| `PumpkinPasty19` | pasty (placed) | (7151, 36, 1629) |  |
| `PumpkinPasty2` | pasty (placed) | (1000, -1482, 1096) |  |
| `PumpkinPasty20` | pasty (placed) | (7072, 611, 2816) |  |
| `PumpkinPasty21` | pasty (placed) | (6103, -17, 3215) |  |
| `PumpkinPasty22` | pasty (placed) | (6987, 1132, 2836) |  |
| `PumpkinPasty23` | pasty (placed) | (7071, 1181, 2836) |  |
| `PumpkinPasty24` | pasty (placed) | (7152, 1138, 2836) |  |
| `PumpkinPasty25` | pasty (placed) | (7155, 1040, 2836) |  |
| `PumpkinPasty26` | pasty (placed) | (7070, -498, 1504) |  |
| `PumpkinPasty27` | pasty (placed) | (6660, -894, 1629) |  |
| `PumpkinPasty28` | pasty (placed) | (6303, -831, 1679) |  |
| `PumpkinPasty29` | pasty (placed) | (3168, -1216, 1510) |  |
| `PumpkinPasty3` | pasty (placed) | (930, -2176, 1112) |  |
| `PumpkinPasty30` | pasty (placed) | (5263, -386, 1679) |  |
| `PumpkinPasty31` | pasty (placed) | (4913, -322, 1679) |  |
| `PumpkinPasty32` | pasty (placed) | (4542, -961, 1679) |  |
| `PumpkinPasty33` | pasty (placed) | (2811, -1218, 1633) |  |
| `PumpkinPasty34` | pasty (placed) | (2752, -1217, 1667) |  |
| `PumpkinPasty35` | pasty (placed) | (2683, -1216, 1692) |  |
| `PumpkinPasty36` | pasty (placed) | (1273, -2223, 1112) |  |
| `PumpkinPasty37` | pasty (placed) | (1852, -327, 1655) |  |
| `PumpkinPasty38` | pasty (placed) | (1856, -95, 1759) |  |
| `PumpkinPasty39` | pasty (placed) | (2241, 161, 2185) |  |
| `PumpkinPasty4` | pasty (placed) | (-418, -1301, 1093) |  |
| `PumpkinPasty40` | pasty (placed) | (2498, 160, 2185) |  |
| `PumpkinPasty41` | pasty (placed) | (1984, 159, 2185) |  |
| `PumpkinPasty42` | pasty (placed) | (1824, 322, 2185) |  |
| `PumpkinPasty43` | pasty (placed) | (1821, 699, 2185) |  |
| `PumpkinPasty44` | pasty (placed) | (1986, 866, 2185) |  |
| `PumpkinPasty45` | pasty (placed) | (2239, 865, 2185) |  |
| `PumpkinPasty46` | pasty (placed) | (2501, 863, 2185) |  |
| `PumpkinPasty47` | pasty (placed) | (2642, 509, 2219) |  |
| `PumpkinPasty48` | pasty (placed) | (4371, 511, 2002) |  |
| `PumpkinPasty49` | pasty (placed) | (6991, 1038, 2836) |  |
| `PumpkinPasty5` | pasty (placed) | (2831, 509, 1755) |  |
| `PumpkinPasty50` | pasty (placed) | (7071, -114, 1629) |  |
| `PumpkinPasty51` | pasty (placed) | (5947, -898, 1679) |  |
| `PumpkinPasty52` | pasty (placed) | (1207, -2187, 1112) |  |
| `PumpkinPasty53` | pasty (placed) | (1209, -2259, 1112) |  |
| `PumpkinPasty54` | pasty (placed) | (7073, 988, 2836) |  |
| `PumpkinPasty6` | pasty (placed) | (3313, 522, 1907) |  |
| `PumpkinPasty7` | pasty (placed) | (3668, 511, 2013) |  |
| `PumpkinPasty8` | pasty (placed) | (4015, 510, 1827) |  |
| `PumpkinPasty9` | pasty (placed) | (4547, 731, 1757) |  |
