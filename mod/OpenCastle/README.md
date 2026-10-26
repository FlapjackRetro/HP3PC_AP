# HP3 Open Castle: test build (proof of concept)

An Ultimate ASI Loader plugin that tries out the Open Castle approach from
[docs/OPEN_CASTLE.md](../../docs/OPEN_CASTLE.md) and [docs/OPEN_CASTLE_CHANGES.md](../../docs/OPEN_CASTLE_CHANGES.md).
It changes how levels connect; it doesn't randomize anything yet.

## What it does

- **Story state locked to `GState090`.** Every level change gets `?GameState=GState090`.
- **New Game starts in Hogwarts** (InsideHub) instead of on the Hogwarts Express.
- **Classroom doors lead to the final exams:** DADA → Ron's exam, Transfiguration → Hermione's, Charms → Harry's.
  The side-room doors still lead to the three challenges.
- **Story maps return to the castle:** the exam chain is broken (each exam returns to InsideHub), and the exits of the
  Hogwarts Express, Expecto Patronum, the Shrieking Shack tunnel and Buckbeak's paddock lead back to a hub. Travel
  into a dropped cutscene map (Quidditch, Whomping Willow, Buckbeak's execution, Dementor battle, Infirmary, Dark
  Tower, GroundsDada) goes to InsideHub instead.
- **Door entrances** (all open in this build; walk up to the door):

  | Door | Leads to |
  | --- | --- |
  | Entrance hall, north locked door | Buckbeak's paddock (Adv6Paddock) |
  | 7th floor, east locked door | Hogwarts Express (Adv1Express) |
  | DADA classroom, portcullis | Expecto Patronum (Adv2ExpPatronum) |
  | 2nd floor, Library portcullis | Library (Adv3Library) |
  | Dungeon, locked door 1 | Shrieking Shack tunnel (Adv5Shack) |

  A door only works after you've been at least ~300 units away from it since arriving on the map, so you don't get
  sent straight back in.

Not in this build: items and gating (every door is open), return points (you arrive at the map's normal start),
spell items, the Marauder's Map pickup.

## Log

`Documents\Harry Potter and the Prisoner of Azkaban\HP3_OpenCastle.log`: whether both hooks were installed, every
travel URL (with the rewrite and the reason), map changes and door use.

## Install

Requires HP3-UHDE's `d3d9.dll` (Ultimate ASI Loader). Copy `HP3OpenCastle.asi` to `<game>\system\plugins\` (admin
rights under `Program Files`). Delete it to go back to the vanilla game. **Use a new save slot**: New Game starts the
Open Castle file, and saves made with it are in `GState090`.

## How it works

- `FURL::FURL(FURL*, const TCHAR*, ETravelType)` in Engine.dll is reached through an incremental-link thunk
  (`jmp rel32`); the plugin points that jump at its own function, which rewrites the URL text (rules in `rules.h`)
  and calls the original. Every level change ends here: `ChangeLevelTrigger` / cutscene `ChangeLevel` →
  `KWPawn.LoadLevel` → `LevelInfo.ServerTravel` → `KWGame.ProcessServerTravel` (appends `?GameState=`) →
  `LevelInfo.NextURL` → the engine builds an `FURL` from it.
- `UObject::ProcessEvent` in Core.dll gets a 5-byte detour (its prologue `55 8B EC 6A FF` is checked first). It only
  checks a flag: when the worker thread has queued a door travel, it calls `LevelInfo.ServerTravel(url, true)` on the
  game thread (found with `UObject::FindFunction`).
- If the bytes at either site aren't what was verified on the retail build, that hook isn't installed and the log
  says so.

## Build and test

```
./build.sh
gcc -w -fshort-wchar -o test/rules_test test/rules_test.c && ./test/rules_test   # travel rules, on the host
```
