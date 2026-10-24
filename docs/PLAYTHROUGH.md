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
| Checks | *(in progress)* |
