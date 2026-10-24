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

**Open Castle:** a one-check prologue. It can be dropped, and Fulbert the Fearful put into the item pool, unless it
stays as the game's opening.
