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
