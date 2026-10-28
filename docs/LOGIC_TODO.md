# Logic to-do: item spawns without a definitive logic requirement

Generated 2026-10-28 from `data/hp3_location_catalog.csv` against the logic notes in [APWORLD.md](APWORLD.md).
Candidate locations: placed pasties / cakes / cards, event-spawned pasties / cakes, containers with a fixed pasty /
cake / card drop, and secret areas. Left out: random-amount containers (not locations), bean-only containers,
challenge stars (not randomized), shops and vendors (logic = shop access + currency; this includes the five
`WC_BeatrixBloxam` cards, which are display props in Fred & George's shop), and spawns whose trigger never fires in
Open Castle (Hogwarts: the Peeves 2 / 3 rewards, `CauldronCake4`, `PumpkinPasty5`), and the five cards in the
Grounds' developer room (Bran the Bloodthirsty, Cyclops, Goliath, Hengist of Upper Barnton, Morholt), and the dropped maps
(Quidditch, WhompingWillow, BuckyExecuted, DementorBattle, Infirmary, DarkTower, GroundsDada, Final).

- **Partly known:** 16 (a region rule or an event chain covers them, but they weren't reported spot by spot).
- **Nothing recorded:** 323.

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

## Nothing recorded

| Area | Count |
| --- | ---: |
| Glacius (B) (`HP3_Ch3GlaciusB`) | 63 |
| Hermione FE (`HP3_FA2Hermione`) | 62 |
| Drac/Lap (B) (`HP3_Ch2DraconiforsB`) | 58 |
| Glacius (`HP3_Ch3Glacius`) | 57 |
| Drac/Lap (`HP3_Ch2Draconifors`) | 37 |
| Tunnel (`HP3_Adv5Shack`) | 21 |
| Harry FE (`HP3_FA3Harry`) | 10 |
| Ron FE (`HP3_FA1Ron`) | 8 |
| Library (`HP3_Adv3Library`) | 7 |

### Glacius (B) (`HP3_Ch3GlaciusB`): 63

| Actor | What | Where | Tag |
| --- | --- | --- | --- |
| `CauldronCake0` | cake (placed) | (-5031, 4991, -388) |  |
| `CauldronCake16` | cake (placed) | (-5972, 4479, -390) |  |
| `CauldronCake17` | cake (placed) | (-5503, 4737, -618) |  |
| `CauldronCake18` | cake (placed) | (-5504, 4737, -314) |  |
| `CauldronCake19` | cake (placed) | (-5972, 4993, -390) |  |
| `CauldronCake2` | cake (placed) | (-5027, 4482, -392) |  |
| `CauldronCake4` | cake (placed) | (-5505, 4737, -57) |  |
| `BannerSpwn0` | BannerSpwn: pastyx5, cakex2 | (-9173, -407, 1817) |  |
| `BannerSpwn2` | BannerSpwn: pastyx4, cakex1 | (-11762, 1253, 1819) |  |
| `ChestSpawner0` | ChestSpawner: pastyx1 | (-6106, 4991, -992) | `floorplate` |
| `ChestSpawner1` | ChestSpawner: pastyx1 | (-6109, 4479, -992) | `floorplate` |
| `ChestSpawner10` | ChestSpawner: pastyx10, cakex2 | (-11503, 1889, 1463) |  |
| `ChestSpawner11` | ChestSpawner: pastyx5 | (-11540, 920, 1604) |  |
| `ChestSpawner2` | ChestSpawner: pastyx1 | (-4899, 4479, -992) | `floorplate` |
| `ChestSpawner3` | ChestSpawner: pastyx1 | (-4904, 4995, -992) | `floorplate` |
| `ChestSpawner30` | ChestSpawner: pastyx3 | (-9260, 3106, 1596) | `RmJ_SpongTrig_Chest_01` |
| `ChestSpawner32` | ChestSpawner: pastyx8, cakex3 | (-8464, 2120, 1724) |  |
| `ChestSpawner33` | ChestSpawner: card UrgtheUncleanx1 | (-9258, -1012, 1596) | `RmJ_Rm2_PushDown_Chest` |
| `ChestSpawner7` | ChestSpawner: pastyx1, cakex2 | (-5334, 5248, -85) | `gargspawner1` |
| `KnightSpawner1` | KnightSpawner: pastyx2 | (-6591, 4127, -960) | `kside1` |
| `KnightSpawner12` | KnightSpawner: pastyx2 | (-6398, 5121, -960) | `khall4` |
| `KnightSpawner13` | KnightSpawner: pastyx2 | (-5056, 3710, -417) | `k2` |
| `KnightSpawner14` | KnightSpawner: pastyx2 | (-4592, 5122, -960) | `khall3` |
| `KnightSpawner2` | KnightSpawner: pastyx2 | (-6398, 4733, -960) | `khall5` |
| `KnightSpawner3` | KnightSpawner: pastyx2 | (-6430, 3965, -416) | `khall6` |
| `KnightSpawner6` | KnightSpawner: pastyx2 | (-5055, 5761, -960) | `k1` |
| `KnightSpawner7` | KnightSpawner: pastyx2 | (-4594, 4739, -960) | `khall2` |
| `KnightSpawner8` | KnightSpawner: pastyx2 | (-4594, 4351, -960) | `khall1` |
| `PumpkinPasty1` | pasty (placed) | (-11367, -4770, -4829) |  |
| `PumpkinPasty10` | pasty (placed) | (-11148, 4374, 552) |  |
| `PumpkinPasty11` | pasty (placed) | (-11368, -1688, 3700) |  |
| `PumpkinPasty12` | pasty (placed) | (-11240, -1617, 3666) |  |
| `PumpkinPasty14` | pasty (placed) | (-11485, -1797, 3743) |  |
| `PumpkinPasty15` | pasty (placed) | (-11568, -1888, 3778) |  |
| `PumpkinPasty16` | pasty (placed) | (-11905, -1115, -3219) |  |
| `PumpkinPasty17` | pasty (placed) | (-11824, -4966, -4973) |  |
| `PumpkinPasty18` | pasty (placed) | (-11629, -4966, -4922) |  |
| `PumpkinPasty19` | pasty (placed) | (-11368, -4962, -4829) |  |
| `PumpkinPasty20` | pasty (placed) | (-11627, -4773, -4922) |  |
| `PumpkinPasty21` | pasty (placed) | (-11900, -1117, -3165) |  |
| `PumpkinPasty22` | pasty (placed) | (-10706, 4094, 437) |  |
| `PumpkinPasty23` | pasty (placed) | (-11858, 4420, -240) |  |
| `PumpkinPasty24` | pasty (placed) | (-12027, 4574, -147) |  |
| `PumpkinPasty33` | pasty (placed) | (-11533, 7087, -328) |  |
| `PumpkinPasty34` | pasty (placed) | (-9323, 6782, -834) |  |
| `PumpkinPasty35` | pasty (placed) | (-11914, 3038, 1218) |  |
| `PumpkinPasty36` | pasty (placed) | (-3024, 856, 8327) |  |
| `PumpkinPasty38` | pasty (placed) | (-11070, -1547, 3609) |  |
| `PumpkinPasty45` | pasty (placed) | (-11913, 3104, 1218) |  |
| `PumpkinPasty46` | pasty (placed) | (-11909, 3176, 1178) |  |
| `PumpkinPasty47` | pasty (placed) | (-11910, 3230, 1138) |  |
| `PumpkinPasty48` | pasty (placed) | (-11904, -1113, -3262) |  |
| `PumpkinPasty49` | pasty (placed) | (-11908, -1057, -3227) |  |
| `PumpkinPasty5` | pasty (placed) | (-11822, -4773, -4973) |  |
| `PumpkinPasty50` | pasty (placed) | (-11909, -1059, -3184) |  |
| `PumpkinPasty51` | pasty (placed) | (-11904, -1061, -3131) |  |
| `PumpkinPasty6` | pasty (placed) | (-11940, 4810, -635) |  |
| `PumpkinPasty7` | pasty (placed) | (-9370, 6019, -1022) |  |
| `PumpkinPasty8` | pasty (placed) | (-11908, 3287, 1081) |  |
| `PumpkinPasty9` | pasty (placed) | (-11657, 4253, 642) |  |
| `SecretAreaMarker0` | secret area | (-9060, 2105, 1711) |  |
| `SecretAreaMarker4` | secret area | (-11459, 1606, 1648) |  |
| `WC_Gringott0` | card Gringott | (-10632, -2334, -3977) |  |

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

### Drac/Lap (B) (`HP3_Ch2DraconiforsB`): 58

| Actor | What | Where | Tag |
| --- | --- | --- | --- |
| `CauldronCake0` | cake (placed) | (-7116, 256, -390) |  |
| `CauldronCake1` | cake (placed) | (-8163, -513, -617) |  |
| `CauldronCake15` | cake (placed) | (-8608, 154, -390) |  |
| `CauldronCake16` | cake (placed) | (-8162, -511, -317) |  |
| `CauldronCake18` | cake (placed) | (-8628, -255, -390) |  |
| `CauldronCake19` | cake (placed) | (-8628, -769, -390) |  |
| `CauldronCake2` | cake (placed) | (-7683, -766, -392) |  |
| `CauldronCake20` | cake (placed) | (-9249, -1691, -392) |  |
| `CauldronCake21` | cake (placed) | (-7687, -257, -388) |  |
| `CauldronCake25` | cake (placed) | (-8162, -512, -57) |  |
| `CauldronCake4` | cake (placed) | (-9248, -1039, -392) |  |
| `CauldronCake5` | cake (placed) | (4277, 1986, 420) |  |
| `CauldronCake7` | cake (placed) | (3709, 912, 297) |  |
| `CauldronCake9` | cake (placed) | (3820, 4983, 327) |  |
| `ChestSpawner0` | ChestSpawner: pastyx2 | (-8762, -257, -992) | `floorplate` |
| `ChestSpawner1` | ChestSpawner: pastyx1, cakex1 | (-8765, -769, -992) | `floorplate` |
| `ChestSpawner12` | ChestSpawner: card NorwegianRidgebackDragonx1 | (4875, 4646, 312) |  |
| `ChestSpawner13` | ChestSpawner: card HungarianHorntailDragonx1 | (2657, 857, 346) |  |
| `ChestSpawner14` | ChestSpawner: cakex2 | (4497, 4562, 312) |  |
| `ChestSpawner19` | ChestSpawner: card RomanianLonghornDragonx1 | (3296, 5513, 645) | `pwizspawner` |
| `ChestSpawner2` | ChestSpawner: pastyx1, cakex1 | (-7555, -769, -992) | `floorplate` |
| `ChestSpawner26` | ChestSpawner: pastyx4 | (3550, 5514, 638) | `ppastyspawner` |
| `ChestSpawner28` | ChestSpawner: cakex2 | (3426, 5033, 784) | `Ksecretspawner` |
| `ChestSpawner3` | ChestSpawner: pastyx2 | (-7560, -253, -992) | `floorplate` |
| `ChestSpawner4` | ChestSpawner: pastyx1, cakex1 | (-7991, -1019, -85) | `gargspawner1` |
| `ChestSpawner5` | ChestSpawner: pastyx1, cakex1 | (-8330, -1019, -85) | `gargspawner1` |
| `ChestSpawner6` | ChestSpawner: pastyx1, cakex1 | (-8329, 0, -85) | `gargspawner1` |
| `ChestSpawner7` | ChestSpawner: pastyx1, cakex1 | (-7990, 0, -85) | `gargspawner1` |
| `ChestSpawner8` | ChestSpawner: pastyx1, cakex1 | (-8327, -511, -85) | `gargspawner1` |
| `ChestSpawner9` | ChestSpawner: pastyx1, cakex2 | (-7989, -511, -85) | `gargspawner1` |
| `KnightSpawner1` | KnightSpawner: pastyx2 | (-9247, -1121, -960) | `kside1` |
| `KnightSpawner12` | KnightSpawner: pastyx2 | (-9054, -127, -960) | `khall4` |
| `KnightSpawner13` | KnightSpawner: pastyx2 | (-7706, -1538, -417) | `k2` |
| `KnightSpawner14` | KnightSpawner: pastyx2 | (-7242, -126, -960) | `khall3` |
| `KnightSpawner2` | KnightSpawner: pastyx2 | (-9054, -515, -960) | `khall5` |
| `KnightSpawner27` | KnightSpawner: pastyx1 | (4412, 4391, 343) | `Lsecretgrass3` |
| `KnightSpawner29` | KnightSpawner: cakex1 | (4513, 4433, 343) | `Lsecretgrass5` |
| `KnightSpawner3` | KnightSpawner: pastyx2 | (-9080, -1283, -416) | `khall6` |
| `KnightSpawner31` | KnightSpawner: pastyx1 | (4625, 4307, 343) | `Lsecretgrass7` |
| `KnightSpawner6` | KnightSpawner: pastyx2 | (-7705, 513, -960) | `k1` |
| `KnightSpawner7` | KnightSpawner: pastyx2 | (-7244, -509, -960) | `khall2` |
| `KnightSpawner8` | KnightSpawner: pastyx2 | (-7244, -897, -960) | `khall1` |
| `PumpkinPasty19` | pasty (placed) | (4416, 1977, 294) |  |
| `PumpkinPasty23` | pasty (placed) | (3138, 3902, 294) |  |
| `PumpkinPasty24` | pasty (placed) | (4081, 1601, 294) |  |
| `PumpkinPasty25` | pasty (placed) | (3048, 2649, 294) |  |
| `PumpkinPasty26` | pasty (placed) | (2764, 2372, 294) |  |
| `PumpkinPasty28` | pasty (placed) | (3046, 1321, 294) |  |
| `PumpkinPasty29` | pasty (placed) | (3801, 1320, 294) |  |
| `PumpkinPasty30` | pasty (placed) | (4977, 4087, 848) |  |
| `PumpkinPasty31` | pasty (placed) | (3711, 3903, 294) |  |
| `PumpkinPasty32` | pasty (placed) | (4945, 5241, 519) |  |
| `PumpkinPasty33` | pasty (placed) | (4672, 4121, 519) |  |
| `PumpkinPasty5` | pasty (placed) | (3808, 2643, 294) |  |
| `SecretAreaMarker1` | secret area | (4945, 4649, 351) |  |
| `SecretAreaMarker5` | secret area | (4369, 1979, 351) |  |
| `SecretAreaMarker6` | secret area | (3424, 5046, 605) |  |
| `SecretAreaMarker7` | secret area | (4496, 4308, 351) |  |

### Glacius (`HP3_Ch3Glacius`): 57

| Actor | What | Where | Tag |
| --- | --- | --- | --- |
| `CauldronCake10` | cake (placed) | (-2138, -2600, 5871) |  |
| `CauldronCake11` | cake (placed) | (-1920, -414, 7777) |  |
| `CauldronCake12` | cake (placed) | (-1712, -2599, 6015) |  |
| `CauldronCake13` | cake (placed) | (-11809, -1653, 3711) |  |
| `CauldronCake14` | cake (placed) | (-11995, -1596, 3655) |  |
| `CauldronCake15` | cake (placed) | (-12976, -671, 3342) |  |
| `CauldronCake3` | cake (placed) | (-2157, -312, 7854) |  |
| `CauldronCake5` | cake (placed) | (-12979, -817, 3330) |  |
| `CauldronCake6` | cake (placed) | (-12912, -509, 3320) |  |
| `CauldronCake8` | cake (placed) | (-1918, -2601, 5939) |  |
| `ChestSpawner12` | ChestSpawner: card EargittheUglyx1 | (9464, -4524, 14248) |  |
| `ChestSpawner31` | ChestSpawner: card AlgufftheAwfulx1, ChocolateFrogx2 | (3524, -3817, 10968) |  |
| `ChestSpawner35` | ChestSpawner: card UgtheUnreliablex1, cakex2 | (-4594, -2707, 4691) | `RmG_Secret_Chest_ActualSpawner` |
| `KnightSpawner38` | KnightSpawner: pastyx3 | (-8621, -3036, 4580) | `RmH_KnightsSecret_Knight_03` |
| `KnightSpawner39` | KnightSpawner: pastyx3 | (-8631, -2496, 4580) | `RmH_KnightsSecret_Knight_02` |
| `PumpkinPasty1` | pasty (placed) | (-11367, -4770, -4829) |  |
| `PumpkinPasty10` | pasty (placed) | (-11148, 4374, 552) |  |
| `PumpkinPasty11` | pasty (placed) | (-11368, -1688, 3700) |  |
| `PumpkinPasty12` | pasty (placed) | (-11240, -1617, 3666) |  |
| `PumpkinPasty13` | pasty (placed) | (-10266, -1097, 3412) |  |
| `PumpkinPasty14` | pasty (placed) | (-11485, -1797, 3743) |  |
| `PumpkinPasty15` | pasty (placed) | (-11568, -1888, 3778) |  |
| `PumpkinPasty17` | pasty (placed) | (-11824, -4966, -4973) |  |
| `PumpkinPasty18` | pasty (placed) | (-11629, -4966, -4922) |  |
| `PumpkinPasty19` | pasty (placed) | (-11368, -4962, -4829) |  |
| `PumpkinPasty20` | pasty (placed) | (-11627, -4773, -4922) |  |
| `PumpkinPasty22` | pasty (placed) | (-10706, 4094, 437) |  |
| `PumpkinPasty23` | pasty (placed) | (-11858, 4420, -240) |  |
| `PumpkinPasty24` | pasty (placed) | (-12027, 4574, -147) |  |
| `PumpkinPasty25` | pasty (placed) | (-1868, -244, 8922) |  |
| `PumpkinPasty26` | pasty (placed) | (-2138, -2785, 5867) |  |
| `PumpkinPasty27` | pasty (placed) | (-1711, -2784, 6011) |  |
| `PumpkinPasty28` | pasty (placed) | (-1918, -2786, 5935) |  |
| `PumpkinPasty29` | pasty (placed) | (-1912, -233, 7853) |  |
| `PumpkinPasty30` | pasty (placed) | (-1657, -612, 7762) |  |
| `PumpkinPasty31` | pasty (placed) | (-1733, -420, 7795) |  |
| `PumpkinPasty32` | pasty (placed) | (-1863, -500, 9013) |  |
| `PumpkinPasty33` | pasty (placed) | (-11533, 7087, -328) |  |
| `PumpkinPasty34` | pasty (placed) | (-9323, 6782, -834) |  |
| `PumpkinPasty35` | pasty (placed) | (-11914, 3038, 1218) |  |
| `PumpkinPasty36` | pasty (placed) | (-3024, 856, 8327) |  |
| `PumpkinPasty37` | pasty (placed) | (1247, -4512, 11157) |  |
| `PumpkinPasty38` | pasty (placed) | (-11070, -1547, 3609) |  |
| `PumpkinPasty41` | pasty (placed) | (-10606, -1500, 3511) |  |
| `PumpkinPasty42` | pasty (placed) | (-10471, -1461, 3511) |  |
| `PumpkinPasty43` | pasty (placed) | (-10351, -1358, 3511) |  |
| `PumpkinPasty44` | pasty (placed) | (-10295, -1239, 3478) |  |
| `PumpkinPasty45` | pasty (placed) | (-11913, 3104, 1218) |  |
| `PumpkinPasty49` | pasty (placed) | (-11908, -1057, -3227) |  |
| `PumpkinPasty5` | pasty (placed) | (-11822, -4773, -4973) |  |
| `PumpkinPasty50` | pasty (placed) | (-11909, -1059, -3184) |  |
| `PumpkinPasty6` | pasty (placed) | (-11940, 4810, -635) |  |
| `PumpkinPasty7` | pasty (placed) | (-9370, 6019, -1022) |  |
| `PumpkinPasty9` | pasty (placed) | (-11657, 4253, 642) |  |
| `SecretAreaMarker1` | secret area | (3522, -3993, 11026) |  |
| `SecretAreaMarker2` | secret area | (-4895, -2715, 4745) |  |
| `SecretAreaMarker3` | secret area | (9189, -4540, 14317) |  |

### Drac/Lap (`HP3_Ch2Draconifors`): 37

| Actor | What | Where | Tag |
| --- | --- | --- | --- |
| `CauldronCake12` | cake (placed) | (4496, -1250, -213) |  |
| `CauldronCake3` | cake (placed) | (4550, -511, -213) |  |
| `CauldronCake6` | cake (placed) | (-3288, -801, -215) |  |
| `CauldronCake8` | cake (placed) | (1342, -386, -316) |  |
| `ChestSpawner20` | ChestSpawner: cakex2 | (-1439, -386, 128) | `Dsecretspawner` |
| `ChestSpawner21` | ChestSpawner: cakex2 | (522, -383, -456) |  |
| `KnightSpawner0` | KnightSpawner: pastyx1 | (2672, -1704, 78) | `Gsecretgrass2` |
| `KnightSpawner19` | KnightSpawner: pastyx1 | (2684, -1572, 78) | `Gsecretgrass4` |
| `KnightSpawner2` | KnightSpawner: card CommonWelshGreenDragonxNone | (-3462, 624, -139) |  |
| `KnightSpawner21` | KnightSpawner: cakex1 | (2818, -1562, 78) | `Gsecretgrass6` |
| `KnightSpawner22` | KnightSpawner: pastyx1 | (2838, -1635, 78) | `Gsecretgrass7` |
| `Pumpkin0` | Pumpkin: pastyx2 | (3022, -596, 77) |  |
| `Pumpkin12` | Pumpkin: pastyx2 | (3018, -913, 78) |  |
| `Pumpkin2` | Pumpkin: card HebrideanBlackDragonx1 | (1343, -388, -716) |  |
| `Pumpkin7` | Pumpkin: pastyx1 | (1783, -381, -729) |  |
| `Pumpkin8` | Pumpkin: pastyx1 | (1343, -795, -730) |  |
| `Pumpkin9` | Pumpkin: pastyx1 | (944, -383, -725) |  |
| `PumpkinPasty0` | pasty (placed) | (4606, -1264, 416) |  |
| `PumpkinPasty1` | pasty (placed) | (-2912, -672, -219) |  |
| `PumpkinPasty10` | pasty (placed) | (-1269, -1030, -58) |  |
| `PumpkinPasty11` | pasty (placed) | (433, -382, 239) |  |
| `PumpkinPasty12` | pasty (placed) | (1428, -400, 252) |  |
| `PumpkinPasty13` | pasty (placed) | (-1855, -1519, -203) |  |
| `PumpkinPasty15` | pasty (placed) | (4608, -880, 657) |  |
| `PumpkinPasty17` | pasty (placed) | (4607, -463, 904) |  |
| `PumpkinPasty18` | pasty (placed) | (3364, -1503, 53) |  |
| `PumpkinPasty2` | pasty (placed) | (-2830, 1431, -217) |  |
| `PumpkinPasty20` | pasty (placed) | (1349, 96, -72) |  |
| `PumpkinPasty21` | pasty (placed) | (3559, -1098, 343) |  |
| `PumpkinPasty22` | pasty (placed) | (-2159, -652, -203) |  |
| `PumpkinPasty3` | pasty (placed) | (-1533, -1464, -49) |  |
| `PumpkinPasty4` | pasty (placed) | (-1535, -654, -203) |  |
| `PumpkinPasty9` | pasty (placed) | (1277, -392, 252) |  |
| `SecretAreaMarker0` | secret area | (1348, -390, -685) |  |
| `SecretAreaMarker2` | secret area | (454, -387, -417) |  |
| `SecretAreaMarker3` | secret area | (3862, -476, -163) |  |
| `SecretAreaMarker4` | secret area | (2752, -1778, 95) |  |


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

### Library (`HP3_Adv3Library`): 7

| Actor | What | Where | Tag |
| --- | --- | --- | --- |
| `ChestSpawner1` | ChestSpawner: card FalcoAesalonx1, pastyx5 | (-3863, -671, 281) |  |
| `ChestSpawner2` | ChestSpawner: cakex4 | (-4278, -3137, 281) |  |
| `SecretAreaMarker2` | secret area | (-8225, -8939, 1367) |  |
| `SecretAreaMarker4` | secret area | (-12316, -9596, 1762) | `WheelPuzzlePrize` |
| `SecretAreaMarker5` | secret area | (-9504, -5620, 1311) | `BridgeSecrets` |
| `SecretAreaMarker6` | secret area | (-10597, -8007, 1651) | `BeanPathOne` |
| `WC_DoubleEndedNewt0` | card DoubleEndedNewt | (-8216, -7269, 1357) |  |
