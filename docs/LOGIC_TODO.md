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
`PumpkinPasty36` (exempt), Glacius B's bean bonus room (exempt, 21 spawns), four Glacius B slide pasties (`PumpkinPasty22`, `23`, `24`, `6`: exempt for now, believed reachable but
unconfirmed; see APWORLD.md), and the dropped maps
(Quidditch, WhompingWillow, BuckyExecuted, DementorBattle, Infirmary, DarkTower, GroundsDada, Final).

- **Partly known:** 63 (a region rule or an event chain covers them, but they weren't reported spot by spot).
- **Nothing recorded:** 101.

## Partly known

| Area | Actor | What | Where | Status |
| --- | --- | --- | --- | --- |
| Carpe | `ChestSpawner11` | ChestSpawner: pastyx3, card AmarilloLestoatx1 | (1314, 4086, 1177) | Carpe region rule only; not checked spot by spot |
| Carpe | `ChestSpawner22` | ChestSpawner: card CountVladDrakulx1, cakex3 | (-3479, 3391, 1336) | Carpe region rule only; not checked spot by spot |
| Carpe | `HPSpawnThingy3` | cake (event spawn) | (1315, 2522, 1209) | Carpe region rule only; not checked spot by spot |
| Carpe | `HPSpawnThingy4` | pasty (event spawn) | (1284, 2109, 864) | Carpe region rule only; not checked spot by spot |
| Carpe | `HPSpawnThingy6` | pasty (event spawn) | (1357, 2132, 864) | Carpe region rule only; not checked spot by spot |
| Carpe | `HPSpawnThingy8` | pasty (event spawn) | (1292, 2177, 864) | Carpe region rule only; not checked spot by spot |
| Carpe | `SecretAreaMarker0` | secret area | (2992, 1048, 627) | Carpe region rule only; not checked spot by spot |
| Carpe | `SecretAreaMarker1` | secret area | (1310, 4223, 1192) | Carpe region rule only; not checked spot by spot |
| Carpe | `SecretAreaMarker2` | secret area | (-3334, 3389, 1336) | Carpe region rule only; not checked spot by spot |
| Carpe (B) | `ChestSpawner13` | ChestSpawner: card LadyCarmillaSanguinax1, pastyx6 | (-1762, 5868, 2258) | Carpe region rule only; not checked spot by spot |
| Carpe (B) | `ChestSpawner19` | ChestSpawner: pastyx4 | (2260, 7300, 1465) | Carpe region rule only; not checked spot by spot |
| Carpe (B) | `KnightSpawner11` | KnightSpawner: cakex1 | (1646, 6681, 1792) | Carpe region rule only; not checked spot by spot |
| Carpe (B) | `KnightSpawner15` | KnightSpawner: pastyx2 | (1585, 5813, 1798) | Carpe region rule only; not checked spot by spot |
| Carpe (B) | `KnightSpawner21` | KnightSpawner: card BlodwynBluddx1, pastyx5 | (1713, 5812, 1797) | Carpe region rule only; not checked spot by spot |
| Carpe (B) | `SecretAreaMarker3` | secret area | (-1769, 5968, 2288) | Carpe region rule only; not checked spot by spot |
| Carpe (B) | `SecretAreaMarker4` | secret area | (2264, 7105, 1480) | Carpe region rule only; not checked spot by spot |
| Drac/Lap | `ChestSpawner20` | ChestSpawner: cakex2 | (-1439, -386, 128) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso); spawn event `Dsecretspawner`; extra needs not reported |
| Drac/Lap | `SecretAreaMarker2` | secret area | (454, -387, -417) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso); extra needs not reported |
| Drac/Lap | `SecretAreaMarker3` | secret area | (3862, -476, -163) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso); extra needs not reported |
| Drac/Lap (B) | `CauldronCake0` | cake (placed) | (-7116, 256, -390) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors); extra needs not reported |
| Drac/Lap (B) | `CauldronCake1` | cake (placed) | (-8163, -513, -617) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors); extra needs not reported |
| Drac/Lap (B) | `CauldronCake15` | cake (placed) | (-8608, 154, -390) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors); extra needs not reported |
| Drac/Lap (B) | `CauldronCake16` | cake (placed) | (-8162, -511, -317) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors); extra needs not reported |
| Drac/Lap (B) | `CauldronCake18` | cake (placed) | (-8628, -255, -390) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors); extra needs not reported |
| Drac/Lap (B) | `CauldronCake19` | cake (placed) | (-8628, -769, -390) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors); extra needs not reported |
| Drac/Lap (B) | `CauldronCake2` | cake (placed) | (-7683, -766, -392) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors); extra needs not reported |
| Drac/Lap (B) | `CauldronCake20` | cake (placed) | (-9249, -1691, -392) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors); extra needs not reported |
| Drac/Lap (B) | `CauldronCake21` | cake (placed) | (-7687, -257, -388) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors); extra needs not reported |
| Drac/Lap (B) | `CauldronCake25` | cake (placed) | (-8162, -512, -57) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors); extra needs not reported |
| Drac/Lap (B) | `CauldronCake4` | cake (placed) | (-9248, -1039, -392) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors); extra needs not reported |
| Drac/Lap (B) | `CauldronCake7` | cake (placed) | (3709, 912, 297) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors; past the Horntail chest also Rictusempra); extra needs not reported |
| Drac/Lap (B) | `CauldronCake9` | cake (placed) | (3820, 4983, 327) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors; past the Horntail chest also Rictusempra); extra needs not reported |
| Drac/Lap (B) | `ChestSpawner0` | ChestSpawner: pastyx2 | (-8762, -257, -992) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors); spawn event `floorplate`; extra needs not reported |
| Drac/Lap (B) | `ChestSpawner1` | ChestSpawner: pastyx1, cakex1 | (-8765, -769, -992) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors); spawn event `floorplate`; extra needs not reported |
| Drac/Lap (B) | `ChestSpawner12` | ChestSpawner: card NorwegianRidgebackDragonx1 | (4875, 4646, 312) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors; past the Horntail chest also Rictusempra); extra needs not reported |
| Drac/Lap (B) | `ChestSpawner14` | ChestSpawner: cakex2 | (4497, 4562, 312) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors; past the Horntail chest also Rictusempra); extra needs not reported |
| Drac/Lap (B) | `ChestSpawner2` | ChestSpawner: pastyx1, cakex1 | (-7555, -769, -992) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors); spawn event `floorplate`; extra needs not reported |
| Drac/Lap (B) | `ChestSpawner3` | ChestSpawner: pastyx2 | (-7560, -253, -992) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors); spawn event `floorplate`; extra needs not reported |
| Drac/Lap (B) | `ChestSpawner4` | ChestSpawner: pastyx1, cakex1 | (-7991, -1019, -85) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors); spawn event `gargspawner1`; extra needs not reported |
| Drac/Lap (B) | `ChestSpawner5` | ChestSpawner: pastyx1, cakex1 | (-8330, -1019, -85) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors); spawn event `gargspawner1`; extra needs not reported |
| Drac/Lap (B) | `ChestSpawner6` | ChestSpawner: pastyx1, cakex1 | (-8329, 0, -85) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors); spawn event `gargspawner1`; extra needs not reported |
| Drac/Lap (B) | `ChestSpawner7` | ChestSpawner: pastyx1, cakex1 | (-7990, 0, -85) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors); spawn event `gargspawner1`; extra needs not reported |
| Drac/Lap (B) | `ChestSpawner8` | ChestSpawner: pastyx1, cakex1 | (-8327, -511, -85) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors); spawn event `gargspawner1`; extra needs not reported |
| Drac/Lap (B) | `ChestSpawner9` | ChestSpawner: pastyx1, cakex2 | (-7989, -511, -85) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors); spawn event `gargspawner1`; extra needs not reported |
| Drac/Lap (B) | `KnightSpawner1` | KnightSpawner: pastyx2 | (-9247, -1121, -960) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors); spawn event `kside1`; extra needs not reported |
| Drac/Lap (B) | `KnightSpawner12` | KnightSpawner: pastyx2 | (-9054, -127, -960) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors); spawn event `khall4`; extra needs not reported |
| Drac/Lap (B) | `KnightSpawner13` | KnightSpawner: pastyx2 | (-7706, -1538, -417) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors); spawn event `k2`; extra needs not reported |
| Drac/Lap (B) | `KnightSpawner14` | KnightSpawner: pastyx2 | (-7242, -126, -960) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors); spawn event `khall3`; extra needs not reported |
| Drac/Lap (B) | `KnightSpawner2` | KnightSpawner: pastyx2 | (-9054, -515, -960) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors); spawn event `khall5`; extra needs not reported |
| Drac/Lap (B) | `KnightSpawner27` | KnightSpawner: pastyx1 | (4412, 4391, 343) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors; past the Horntail chest also Rictusempra); spawn event `Lsecretgrass3`; extra needs not reported |
| Drac/Lap (B) | `KnightSpawner29` | KnightSpawner: cakex1 | (4513, 4433, 343) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors; past the Horntail chest also Rictusempra); spawn event `Lsecretgrass5`; extra needs not reported |
| Drac/Lap (B) | `KnightSpawner3` | KnightSpawner: pastyx2 | (-9080, -1283, -416) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors); spawn event `khall6`; extra needs not reported |
| Drac/Lap (B) | `KnightSpawner31` | KnightSpawner: pastyx1 | (4625, 4307, 343) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors; past the Horntail chest also Rictusempra); spawn event `Lsecretgrass7`; extra needs not reported |
| Drac/Lap (B) | `KnightSpawner6` | KnightSpawner: pastyx2 | (-7705, 513, -960) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors); spawn event `k1`; extra needs not reported |
| Drac/Lap (B) | `KnightSpawner7` | KnightSpawner: pastyx2 | (-7244, -509, -960) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors); spawn event `khall2`; extra needs not reported |
| Drac/Lap (B) | `KnightSpawner8` | KnightSpawner: pastyx2 | (-7244, -897, -960) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors); spawn event `khall1`; extra needs not reported |
| Drac/Lap (B) | `PumpkinPasty23` | pasty (placed) | (3138, 3902, 294) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors; past the Horntail chest also Rictusempra); extra needs not reported |
| Drac/Lap (B) | `PumpkinPasty30` | pasty (placed) | (4977, 4087, 848) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors; past the Horntail chest also Rictusempra); extra needs not reported |
| Drac/Lap (B) | `PumpkinPasty31` | pasty (placed) | (3711, 3903, 294) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors; past the Horntail chest also Rictusempra); extra needs not reported |
| Drac/Lap (B) | `PumpkinPasty32` | pasty (placed) | (4945, 5241, 519) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors; past the Horntail chest also Rictusempra); extra needs not reported |
| Drac/Lap (B) | `PumpkinPasty33` | pasty (placed) | (4672, 4121, 519) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors; past the Horntail chest also Rictusempra); extra needs not reported |
| Drac/Lap (B) | `SecretAreaMarker1` | secret area | (4945, 4649, 351) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors; past the Horntail chest also Rictusempra); extra needs not reported |
| Drac/Lap (B) | `SecretAreaMarker7` | secret area | (4496, 4308, 351) | area rule only (Draconifors / Lapifors Challenge Unlock + Lapifors; past the Welsh Green armor also Alohomora; past the pumpkin ring also Depulso; all of Ch2B also Draconifors; past the Horntail chest also Rictusempra); extra needs not reported |

## Nothing recorded

| Area | Count |
| --- | ---: |
| Hermione FE (`HP3_FA2Hermione`) | 62 |
| Tunnel (`HP3_Adv5Shack`) | 21 |
| Harry FE (`HP3_FA3Harry`) | 10 |
| Ron FE (`HP3_FA1Ron`) | 8 |

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

### Tunnel (`HP3_Adv5Shack`): 21

| Actor | What | Where | Tag |
| --- | --- | --- | --- |
| `CauldronCake4` | cake (placed) | (-707, -5223, -48) |  |
| `CauldronSpwn1` | CauldronSpwn: pastyx2 | (1954, -4330, -856) | `RmA_Cauldron_Caul_02` |
| `CauldronSpwn2` | CauldronSpwn: pastyx2 | (-5522, -1420, -980) | `RmA_Cauldron_02_Caul` |
| `ChestCompanionSpawner0` | ChestCompanionSpawner: pastyx4, ChocolateFrogx2 | (3240, -3773, -999) | `RmF_CompLock_Chest` |
| `ChestCompanionSpawner4` | ChestCompanionSpawner: pastyx5 | (4097, -2827, -999) | `RmG_CompLock_Chest` |
| `ChestSpawner11` | ChestSpawner: cakex1 | (-6349, 63, -992) | `RmA_Secret_01_Chest` |
| `ChestSpawner12` | ChestSpawner: cakex2 | (-3235, -3979, -770) |  |
| `ChestSpawner2` | ChestSpawner: pastyx3 | (964, -2395, -3975) | `RmE_ShaftChest_Chest` |
| `ChestSpawner5` | ChestSpawner: cakex3 | (785, -3349, -2016) |  |
| `ChestSpawner7` | ChestSpawner: ChocolateFrogx1, pastyx2 | (-4582, -2857, -899) | `RmA_skelWin_Chest` |
| `SkullShelfSpwn2` | SkullShelfSpwn: pastyx2 | (-2096, -2492, -1488) |  |
| `SkullShelfSpwn6` | SkullShelfSpwn: cakex1 | (-5520, 293, -538) | `RmA_SmlSecret_Spawn` |
| `PumpkinPasty0` | pasty (placed) | (467, -2404, -2895) |  |
| `PumpkinPasty3` | pasty (placed) | (-1761, -3577, -880) |  |
| `PumpkinPasty5` | pasty (placed) | (824, -2044, -1048) |  |
| `SecretAreaMarker0` | secret area | (-5444, 258, -928) | `RmA_SmlSecret_Scrt` |
| `SecretAreaMarker1` | secret area | (-3310, -3971, -760) |  |
| `SecretAreaMarker2` | secret area | (-718, -4791, -760) |  |
| `SecretAreaMarker3` | secret area | (802, -2960, -1904) |  |
| `SecretAreaMarker4` | secret area | (-6270, -544, -928) |  |
| `SpawnThingy10` | card SacharissaTugwood | (5426, -3913, -705) | `RmG_Win_SpawnThing_Card` |

### Harry FE (`HP3_FA3Harry`): 10

| Actor | What | Where | Tag |
| --- | --- | --- | --- |
| `CauldronCake0` | cake (placed) | (-3679, -347, 301) |  |
| `CauldronCake2` | cake (placed) | (-3955, -354, 400) |  |
| `Hpgenericspawner0` | Hpgenericspawner: pastyx6 | (-97, -231, 283) | `Beanbonanza` |
| `Hpgenericspawner1` | Hpgenericspawner: pastyx6, cakex2 | (68, -542, 206) | `Beanbonanza` |
| `Hpgenericspawner2` | Hpgenericspawner: pastyx3 | (-59, -509, 291) | `Beanbonanza` |
| `Hpgenericspawner3` | Hpgenericspawner: pastyx3 | (91, -383, 317) | `Beanbonanza` |
| `Hpgenericspawner4` | Hpgenericspawner: pastyx3 | (96, -298, 222) | `Beanbonanza` |
| `PumpkinPasty0` | pasty (placed) | (-3784, -350, 346) |  |
| `PumpkinPasty1` | pasty (placed) | (-3528, -358, 264) |  |
| `PumpkinPasty2` | pasty (placed) | (-3248, -360, 175) |  |

### Ron FE (`HP3_FA1Ron`): 8

| Actor | What | Where | Tag |
| --- | --- | --- | --- |
| `CauldronCake0` | cake (placed) | (5103, -3108, 1328) |  |
| `CauldronCake1` | cake (placed) | (5200, -3150, 1328) |  |
| `CauldronCake2` | cake (placed) | (5103, -3205, 1328) |  |
| `ChestSpawner3` | ChestSpawner: pastyx10 | (1941, -3238, 1349) |  |
| `ChestSpawner5` | ChestSpawner: pastyx4, cakexNone | (1574, -5669, 792) |  |
| `ChestSpawner9` | ChestSpawner: cakex8 | (2614, -3259, 1349) |  |
| `KnightSpawner7` | KnightSpawner: ChocolateFrogx1, pastyx3 | (716, -4561, 1401) |  |
| `TapestrySpwn1` | TapestrySpwn: pastyx4 | (440, -3816, 1563) |  |
