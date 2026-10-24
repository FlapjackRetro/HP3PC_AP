# Vanilla playthrough log

Map-by-map notes from a casual playthrough, recorded with [MapTracker](../mod/MapTracker/README.md). Each entry is
confirmed in game; the check counts are cross-checked against [`data/locations.json`](../data/locations.json).
These notes decide which maps Open Castle keeps ([OPEN_CASTLE.md](OPEN_CASTLE.md) §6).

## 1. HP3_Adv1Express: prologue on the Hogwarts Express

| | |
| --- | --- |
| Level ID | `adv1` |
| State | `GSTATE000` (new game: `FirstLevel=HP3_Adv1Express.unr`) |
| Role | Prologue: the opening level on the Hogwarts Express |
| Checks | **1**: wizard card *Fulbert the Fearful* |

The card isn't placed in the level. It's spawned by event `SpawnWC1` (`HPSpawnThingy0`), so it counts as an event
reward (`awarded_by` in the catalogue).

Not checks: six bean spawners (`Hpgenericspawner`, Jellybeans only), three Alohomora padlocks
(`PadlockB`/`C`/`H`), and the save book (`SaveBook4`).

**Ending:** a scripted cutscene in which a Dementor boards the train and Ron defends Harry. It flows straight
into the next cutscene, `0180ADV1DemExit`, which loads HP3_GroundsDada (`ChangeLevel HP3_GroundsDADA`) and sets
`GState000`. From there `0200DADAIntro` plays on arrival and loads HP3_Ch1CarpeRetractum, so from the player's
side it's one cutscene running from the train into the challenge.

**Open Castle:** a one-check prologue. It can be dropped, and Fulbert the Fearful put into the item pool, unless it
stays as the game's opening.

## 2. HP3_GroundsDada, first visit (passed through in a cutscene)

| | |
| --- | --- |
| Level ID | `GroundDADA` |
| State | `GState000` |
| Role | The Defence Against the Dark Arts classroom; on this visit only the `0200DADAIntro` cutscene plays |
| Checks on this visit | **0**: there's no player control. MapTracker logged the map for about a second before Ch1 loaded. |

The map does contain checks (catalogue: wizard card *Double-Ended Newt*, and 3 secret areas: `BeanPathOne`,
`WheelPuzzlePrize`, `SecretAreaMarker2`). They're reached on a later, playable visit: the Patronus chain enters
from Quidditch (`7010QuidDem_ALL`, `GState070`) *(to confirm)*.

**Open Castle:** this visit is cutscene-only and can be skipped. The map itself stays, for the later visit.

## 3. HP3_Ch1CarpeRetractum: Carpe Retractum challenge (first visit)

| | |
| --- | --- |
| Level ID | `cr` |
| State | `GState000` on arrival |
| Role | Ron's Carpe Retractum challenge, the first playable level after the prologue |
| Requires | Carpe Retractum, **Spongify**, **Rictusempra**, **Alohomora** (confirmed) |
| Checks | *(in progress)*; one wizard card found so far |

Spell actors in the map: 21 Retractum targets, 2 Spongify pads, 4 Alohomora triggers and 2 padlocks, 17 other
spell triggers. Its second half, HP3_Ch1CarpeRetractumB, has 11 Spongify pads, Lumos sparkles and a Depulso knight.

Catalogue checks (first half):

| Check | Where | Note |
| --- | --- | --- |
| Card *Amarillo Lestoat* | `ChestSpawner11` (1314, 4086, 1177) | chest beside secret area `SecretAreaMarker1` |
| Card *Sir Herbert Varney* | `ChestSpawner18` (2736, 1140, 608) | chest beside secret area `SecretAreaMarker0` |
| Card *Count Vlad Drakul* | `ChestSpawner22` (−3479, 3391, 1336) | chest beside secret area `SecretAreaMarker2` |
| 3 secret areas | `SecretAreaMarker0`–`2` | each with its card chest |
| Challenge win | `EndChallengeTrigger0` | candidate check |
| 9 challenge stars | | candidate checks |

All three cards are in chests inside secret areas.

**Pumpkin pasties:** 3, confirmed. They aren't placed in the level. They appear on top of a Spongify pad
(`SpongifyPad3`, around (1313, 2141, 812)) once the imps are beaten by throwing their wizard crackers back at them.
`ImpCombatManager` sends its done-event, and three `HPSpawnThingy` spawners fire (`SpawnPasties1`/`2`/`3`, actors
`HPSpawnThingy8`/`4`/`6`). The chain is imp fight → `ImpCombatManager1` → `CRDoorTrigger_CS`, and the
`SpawnPastiesImp` dispatcher fires the three spawners.

**Doxy room** (right after that Spongify pad; group `RoomD_E_F`): requires **Rictusempra**, confirmed. Two waves of
three doxies (`Poxie`, the game's name for doxies). Wave 1 (`poxiegroup1`) counts down `poxtowercounter1`, which
starts wave 2 (`poxiegroup2`). Wave 2 counts down `poxtowercounter2` (3), which fires `cutpoxdoor`, the
`6045Ch1BeatPixiesQC` cutscene. The reward is **one Cauldron Cake**, confirmed, from spawner `SpawnCauldronCake`
(`HPSpawnThingy3`, (1315, 2522, 1209)).

**Alohomora** is used in this half, confirmed. Its targets in the map:
- Room C: `AlohomoraTrigger5` (3101, 391, 702), event `SecretZ1aa`, and `AlohomoraTrigger2` (3232, 519, 671),
  event `Z1flipDis`. These look like the way into secret area `SecretAreaMarker0` and the *Sir Herbert Varney* chest.
- Rooms K–M (fire crabs): padlocks `ALpadlock25` (−3232, 4482, 972) and `ALpadlock2` (−2233, 5393, 762), which
  start the fire-crab fights (`FireCrabroomfirstDis`, `CVActivateFCs`).

*(which one was required for progress: to confirm)*

Logic so far for these rewards: imp pasties need Carpe Retractum to reach the imps *(verify)*; the doxy cake needs
Spongify (to get in) and Rictusempra.

Spawned snacks have no placed actor, so they get no `_PU` save key. As checks they'd be detected by the spawn event
(or by the pickup of a spawned `PumpkinPasty`). The catalogue now lists them with `awarded_by` = the spawner's
tag: 32 pasties and 12 cakes across all maps.

## 4. HP3_Ch1CarpeRetractumB: second half of the Carpe Retractum challenge

| | |
| --- | --- |
| Level ID | `crb` |
| State | `GState000` |
| Entered from | HP3_Ch1CarpeRetractum, `ChangeLevelTrigger1`, partway through the challenge (confirmed) |
| Leaves to | InsideHub, through the end-of-challenge cutscenes (`0700Ch1Chal_InsHubDoor`, or `_010` in `GState000`, which sets `GState010`) |
| Checks | *(in progress)* |

| Check | Where | Status |
| --- | --- | --- |
| Card *Lady Carmilla Sanguina* | `ChestSpawner13` (−1762, 5868, 2258), with 25 beans and 6 pasties (pasties confirmed); beside secret area `SecretAreaMarker3` (−1769, 5968, 2288) | **found** (in a chest) |
| Card *Blodwyn Bludd* | `KnightSpawner21` (1713, 5812, 1797), Depulso knight, with 5 pasties | **found**, with the 5 pasties |
| Secret areas | `SecretAreaMarker3`, `SecretAreaMarker4` (2264, 7105, 1480) | |
| Cauldron cake | `KnightSpawner11` (1646, 6681, 1792), Depulso knight, with 20 beans | **found** |
| Cauldron cake | `BannerSpwn1` (−1137, 4734, 1429), a banner | |
| 2 pasties | `KnightSpawner15` (1585, 5813, 1798), Depulso knight, with 15 beans | **found** |
| 4 pasties | `ChestSpawner19` (2260, 7300, 1465), beside secret area `SecretAreaMarker4` | |
| pasties | `ChestSpawner12` (1662, 5275, 1737), tag `EVChest`, probably the carousel's reward *(verify)* | |

**Requires** Depulso as well (confirmed, knights).

**Knight carousel** (room O–P, around (1648, 5812)): what looks like one suit of armour is four knights on a
rotating platform (`ElevatorMover0`–`3`, tags `EV1`–`EV4`, 5 keys each). Each Depulso hit turns the next knight to
the front, so Depulso can be cast again (confirmed). Knights and drops:

| Knight | Tag | Drops |
| --- | --- | --- |
| `KnightSpawner23` | `EVKnight` / `EV1` | class default (beans) |
| `KnightSpawner15` | `EVK2` / `EV2` | 15 beans, 2 pasties (found) |
| `KnightSpawner16` | `EVK3` / `EV3` | class default (beans) |
| `KnightSpawner21` | `EVK4` / `EV4` | card *Blodwyn Bludd*, 5 pasties (found) |

`AlohomoraTrigger4` (`AlohEVtrigger` → round robin `EVRR`) is next to it, so Alohomora may be part of the
mechanism *(verify)*. For Open Castle the carousel is one location group needing Depulso.

Chest pasties come out as a burst, not as placed pickups, so under the current plan they're filler, not checks.

**Open Castle:** Ch1 and Ch1B are one region. The two halves are connected only to each other, so the whole
challenge needs a single entrance (the side-room door) and a single exit back to InsideHub. Its logic combines both
halves. Catalogue for this half: cards *Blodwyn Bludd* (`KnightSpawner21`, a Depulso knight) and *Lady Carmilla
Sanguina* (`ChestSpawner13`), secret areas `SecretAreaMarker3`/`4`, 4 challenge stars, and a bean bonus game.
Spell actors: 11 Spongify pads, 9 Retractum targets, Lumos sparkles, 2 Alohomora triggers.

### End of the challenge: bean bonus room (optional)

**Challenge shields:** the challenge has **10** (confirmed). The bean bonus room is only offered if all ten were
collected; otherwise the challenge ends without it (confirmed). In the data:
- first half: `CRS01`–`CRS07`;
- second half: `CRBS02`–`CRBS04`.

`CRBS01` sits at the same spot as `CRS07` (the area appears in both maps), and two untagged stars
(`ChallengeStar4`, and `ChallengeStar6` at the end trigger) seem to be unused or duplicates, which explains the
catalogue's 13.

The bean bonus room isn't the separate HP3_BeanBonus map (that one is entered from InsideHub,
`ChangeLevelTrigger14`). It's built into the challenge: `BeanBonusGame0` in HP3_Ch1CarpeRetractumB at
(5972, 3560, −1004), with a 120-second timer. Ch2B and Ch3B each have their own.

Sequence:
1. `EndChallengeTrigger0` (4737, 7075, 2209): all shields → `0697Ch1FinalStarALL`, then the bean bonus; otherwise
   `0696Ch1FinalStrFail`; replays play `0700Ch1ChalChReplay`.
2. Bean bonus round, 120 s (only with all 10 shields).
3. `0698BeanBonusRonEXIT` → `Trigger ChalChamber_CS`: the player comes out in the **challenge chamber**, the side
   room with the three challenge entrances. Confirmed with MapTracker v7: the chamber and the shortcuts room it
   opens onto are both part of **HP3_InsideHub** (its `ChangeLevelTrigger9`/`10`/`11`). Ch1B also contains a copy of
   the chamber (group `ChallengeChamber`, same three triggers). The earlier log line showing Ch1B in `GState010`
   came from v5's stale-controller bug, so it doesn't say which copy the bonus exit uses *(to confirm)*.
4. Leaving the chamber leads into InsideHub (next entry).

**Open Castle:** the bean bonus is beans only (no check). Candidate checks: the challenge win (`EndChallengeTrigger`,
`_bAlreadyWon`) and all 10 shields.

## 5. HP3_InsideHub: arrival at Hogwarts (`GState010`)

Confirmed: trying to leave the challenge chamber plays a cutscene in the shortcuts room. One of the twins (Fred or
George) produces the **Marauder's Map**, and Harry picks it up in the cutscene. The player then has control in
InsideHub, following the twin up to the twins' shop.

From the data:
- the cutscene is `1051GetMaraudersMap` (Fred and George lines, then Ron and Peeves, then `Trigger PeevesEnc1`);
- the map is a pickup: `MaraudersPickup0`/`1`/`2` in InsideHub around (150, −870, 144);
- `1005TaskedShop` also contains a `GiveMap`; which one actually grants the map in vanilla is still to confirm.

**Open Castle:** the Marauder's Map check (option `marauders_map_check`) can be detected on picking up
`MaraudersPickup`, or on the map entering the inventory. The cutscene is story-only, so in Open Castle the pickup
needs a new trigger, or the map is simply placed at that spot.
