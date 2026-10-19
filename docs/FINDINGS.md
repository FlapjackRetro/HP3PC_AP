# Findings

What's known so far from static analysis of the US retail build (`Version.txt`: `HPPOA 3 544`). Nothing here
has been tested in-game unless it says so.

## Engine and packages

- KnowWonder's build of Unreal Engine 2. Package file version **129**, licensee version 0.
- Script packages in `system/`: `Core`, `Engine`, `GamePlay`, **`kwGame`** (KnowWonder's framework: pawns, pickups,
  save store, cutscenes, triggers) and **`HGame`** (the HP3 game code, 675 classes). The installer copies a
  separate `kwGame.u` from the disc root (`common_filelist.txt`) into `system/`, apparently a late patch.
- `HGame` and `kwGame` ship with their source stripped but full bytecode. `tools/decompile.py` turns it into
  goto-style pseudo-UnrealScript. All 1,422 classes in HGame, kwGame, GamePlay and Engine decompile without
  errors. `Engine.u` and `Core.u` still contain their original `.uc` source, which was used to check the
  decompiler's output.
- No `ucc.exe` or `UnrealEd.exe` ships with the game, so there's no stock way to compile new script.
  `Editor.u`/`Editor.dll` are present. [Jack's Harry Potter 3 Level Editor](https://www.moddb.com/games/harry-potter-and-the-prisoner-of-azkaban/downloads/jacks-harry-potter-3-level-editor)
  on ModDB might provide one (not yet checked).
- `IpDrv.dll` ships but **`IpDrv.u` doesn't**, so script can't use `TcpLink` as-is. (HP2PC_AP depends on M212's
  engine, which restores UE1-style networking. HP3 has no equivalent.)

## Story progress

`KWGame.CurrentGameState` holds one of the 16 entries in `HPWorldInfo.GameStateMasterList`
([`data/game_states.json`](../data/game_states.json)): `GSTATE000` … `GSTATE120`. `GetNumericGameState()` is the
last three digits. Vendors and wizard-card availability check it (`GameStateAvailable`). Maps set it through
`KWAction_SetGameState` scripted actions.

## Save system

Everything persistent goes through `kwGame.PersistentStore`, a string key/value store
(`SaveInt/SaveBool/SaveString/SaveFloat(key, value)`, `Restore*`). Actors write themselves in
`SavePersistentData` and read back in `RestorePersistentData`. Keys found so far:

| What | Key | Written by |
| --- | --- | --- |
| Any pickup collected | `<LevelID><ActorName>_PU` (+ `_x/_y/_z` when `bRememberLoc`) | `KWPickup` |
| Wizard card status | `wc_<index>` → `EWizardCardStatus` (5 = owned) | `WizardCardManager` |
| Challenge stars | `<LevelID><j>` per level, from `FoundStatus[9]`, up to 10 each | `ChallengeStarManager` |
| Secret area found | `<LevelID><ActorName>` (bool), plus `SM_*` counters | `SecretAreaMarker`, `SecretManager` |
| Portrait password owned | `pp_<n>` | `PortraitPasswordManager` |
| Challenge won | `<LevelID><Name>_bAlreadyWon` | `EndChallengeTrigger` |
| Peeves defeated | `<LevelID><Name>_bD` | `Peeves` |
| Vendor stock | `NumCakesForSale`, `NumPastiesForSale`, `<LevelID><Name>` (card class) | `VendorManager`, `VI_WizardCard` |
| Spawner lives, movers, triggers | `<LevelID><Name>_lives`, `_KeyNum`, `_bTOO`, … | `KWGenericSpawner`, `KWMover`, `KWTrigger` |

`<LevelID>` is `LevelInfo.ID`, a KnowWonder addition. Several maps have no `ID` set (Adv1Express, the three
`Ch*` challenges and their `B` halves, Final, PortraitPassword_F); how their keys come out needs checking in-game.

## Items (what Archipelago could give)

| Item | How the game grants it |
| --- | --- |
| Wizard cards (80) | `HGame(Level.Game).WizardCardManager.MarkWizardCardAsOwned(class'WC_X')`. The table is in [`data/wizard_cards.json`](../data/wizard_cards.json): 49 found in levels, 20 sold by vendors, 6 in the Fred & George shop, 4 in adventure levels and 1 special last card. 10 card sets. |
| Beans / Cauldron Cakes / Pumpkin Pasties | `KWPawn.AddToInventoryCollection(class'JellybeanCollection' / 'CauldronCakeCollection' / 'PumpkinPastyCollection', n)` |
| Challenge stars/shields | `ChallengeStarManager.FoundStatus[level].Found[j]` |
| Portrait passwords | `PortraitPasswordManager.PortraitPasswords[n].Owned` |
| Marauder's Map | `MaraudersMap` inventory, added to `Level.InventoryCarrierPawn` |
| Spells | **Not owned items in the retail game.** `WeaponWand.HasSpell()` returns true for every spell except a few per character (Harry: no Draconifors/Lapifors/Carpe Retractum; Ron: no Draconifors/Lapifors; Hermione: no Carpe Retractum). Spells are effectively gated by level design. To make them Archipelago items, `HasSpell()` would need an override. |

The game already has debug commands that grant these (`HPHeroController`): `GiveAllWC`, `GiveAllButFinalWC`,
`GiveBeans N`, `GiveCakes N`, `GivePasties N`, `GiveCurrency N`, `GiveMap`, `GiveAllShields`, `GiveAllPP`,
`GiveAll`, `BeatBoss`, `summonStars N`. Whether the console can be opened in retail hasn't been checked yet.

## Locations

[`data/hp3_location_catalog.csv`](../data/hp3_location_catalog.csv) (976 rows, all 34 maps) lists every actor that could
be a check, with its map, Level ID, actor name, class, position, tag/event, chest contents and the `_PU` save key for pickups:

| Category | Count | Notes |
| --- | --- | --- |
| Chests and other spawners (`KWGenericSpawner`) | 486 | Contents from `GoodieToSpawn[8]`/`GoodiesNumber[8]`. 23 hold a specific wizard card and 1 a challenge star; most hold beans, cakes or pasties |
| Pumpkin Pasties / Cauldron Cakes (placed) | 180 / 71 | Probably filler |
| Challenge stars | 63 | Almost all in the classroom challenges and final exams |
| Save books | 45 | |
| Secret areas | 41 | |
| Wizard cards (placed) | 29 | Including five in the Bean Bonus room |
| Vendor items | 22 | All in InsideHub: cards, cakes, pasties, chocolate frogs, portrait passwords, the final card |
| Creature statues | 18 | Draconifors/Lapifors spell targets |
| Challenge completions | 7 | `EndChallengeTrigger` |
| Pixie games, bean bonus games, Peeves fights, Marauder's Map pieces | 5 / 3 / 3 / 3 | |

## Bridge options

How the running game could talk to an Archipelago client:

1. **Native `.asi` plugin (most likely).** HP3-UHDE's `d3d9.dll` is Ultimate ASI Loader, so any DLL in `system\plugins\`
   loads into the game. `Core.dll`/`Engine.dll` export the full C++ API (`StaticFindObject`, `UObject::ProcessEvent`,
   property lookup, …), so a plugin can call the script functions listed above and talk to the client over a socket.
2. **Script mod + files.** `Core.Object` has KnowWonder natives `LoadStringArray(array<string>, Filename)` (2001) and
   `SaveStringArray(array<string>, Filename)` (2002). There's also `GetIniEntry`, `Get/SetPropertyText` and `ConsoleCommand`.
   A script mod could exchange data with the client through files. This needs a script compiler (see above), and it's
   untested whether `LoadStringArray` writes the array back to the caller.
3. **Build `IpDrv.u`** against the shipped `IpDrv.dll` to bring `TcpLink` back to script. Same compiler problem.

## Tools

All in `tools/`, Python 3 with no dependencies. Run them against your own install. Their output for game code stays local (`.gitignore`).

| Script | Does |
| --- | --- |
| `ue2.py` | Package reader (names/imports/exports, tagged properties, UField/UStruct/UFunction/UState/UClass) and bytecode decompiler |
| `decompile.py <game>/system <out>` | Decompile HGame/kwGame/GamePlay/Engine and extract the embedded Core/Engine source |
| `build_catalog.py <game> <out>` | Regenerate everything in `data/` |
| `framerate_patch.py <Engine.dll>` | 60 fps cap (see [SETUP.md](SETUP.md)) |
| `inipatch.py` | Apply UHDE's `.IniPatch` files |
| `pe.py` | Small PE export-table helper used to find the framerate patch site |
