# Open Castle: mod changes, map by map

The concrete changes behind the layout in [OPEN_CASTLE.md](OPEN_CASTLE.md) §6a, for base state **`GState090`**.
Everything is done at runtime by the mod when a map loads (spawn, retarget, hide, disable). The game's `.unr`
files are never edited or shipped.

The list was built from [`data/map_entrances.json`](../data/map_entrances.json), filtered to the level changes active
in `GState090`, plus the positions confirmed in [PLAYTHROUGH.md](PLAYTHROUGH.md). Actor names are the map's own.

**Verbs**
- **Spawn**: create a new actor (usually a `ChangeLevelTrigger`) at a position.
- **Retarget**: change an existing trigger's `NewMapName`, or a cutscene's `ChangeLevel` target, and drop its
  `SetGameState`/`bSetGameState`.
- **Suppress**: disable a trigger or cutscene trigger so the story chain never starts.
- **Gate**: keep something shut or disabled until an Archipelago item has been received.
- **Return point**: where the player appears when coming back from a story map (a spawned `PlayerStart`, or a
  teleport right after load).

## 0. Global

| # | Change | How |
| --- | --- | --- |
| G1 | **Lock the story state to `GState090`** on every level load, whatever a trigger or cutscene asks for | hook the state set (`KWGame.CurrentGameState`; `SetGameState` cutscene command; `?GameState=` on travel URLs) |
| G2 | **Spell items**: Alohomora, Depulso, Spongify, Rictusempra, Expecto Patronum, Carpe Retractum, Draconifors (+ Lapifors), Glacius | replace `WeaponWand.HasSpell()`; Lumos separately (`LumosTurnOn`) |
| G3 | **Shortcut Passwords** item | `PortraitPasswordManager.EnablePortraitShortcuts()` (entries 8–21, `pp_8`–`pp_21`); suppress the vanilla grant (Peeves fight 1 is `GState010` only, so it's absent anyway) |
| G4 | **Checks**: cards (`WizardCardManager.MarkWizardCardAsOwned`), secrets (`SecretAreaMarker` keys), placed snacks (`_PU` keys), event snacks (`SpawnThingy`/`HPSpawnThingy` spawn events), bundimun defeats, Marauder's Map pickup | hooks per [OPEN_CASTLE.md](OPEN_CASTLE.md) §2 |
| G5 | **Return points**: remember which entrance the player used, and place them there when its story map exits | store on travel; teleport after the hub loads |
| G6 | **Party for the adventures**: make sure Adv5Shack loads with Harry + Hermione (companion chests) | check which party each map sets on load *(verify)* |

## 0a. Game start

| # | Change | Detail |
| --- | --- | --- |
| S1 | New game starts in **InsideHub, shortcuts room** (where `1051GetMaraudersMap` leaves the player), not on the Hogwarts Express | `FirstLevel` / the new-game travel URL → `HP3_InsideHub?GameState=GState090`, with the start at the Marauder's Map spot (≈ (150, −870, 144)) |
| S2 | **Grant the Marauder's Map at start** unless `marauders_map_check` is on | the vanilla way is simply `Spawn(Class'MaraudersMap')` + `InventoryCarrierPawn.AddInventory` (`HPAction_GiveMap`, also the `GiveMap` console command). The `1051GetMaraudersMap` cutscene itself isn't needed: its staging (Fred, George, the shop, Peeves) is `GState010`, so it wouldn't play properly in `090` |
| S3 | Optional flavour | a short custom cutscene of the twins handing over the map, if wanted later |

## 1. HP3_InsideHub

**Keep:** `ChangeLevelTrigger8` (DungeonHUB), `ChangeLevelTrigger14` (BeanBonus), portraits
`ChangeLevelTrigger1`/`4`/`13`/`12`/`0` (PortraitPassword_B–F), `CutSceneTrigger31` `LevelLoadGrounds` (grounds,
behind the Grounds Access gate).

| # | Change | Actor(s) | Detail |
| --- | --- | --- | --- |
| I1 | Suppress | `CutSceneTrigger13` (−98, −714, 1259), `9240RonExamIntroPt1` | Ron's exam intro (would load FA1 and set `100`) |
| I2 | Suppress | `0200DADAIntro` (level-start cutscene into Ch1) | DADA intro chain |
| I3 | Retarget | `ChangeLevelTrigger3` (262, −2328, 1218), tag `ChangleLevel_CarpeChallenge` | Ch1 → **HP3_FA1Ron**. Gate: Carpe Retractum (+ exam item if split). It's normally fired by a cutscene; make it touch-activated |
| I4 | Retarget | `ChangeLevelTrigger2` and `ChangeLevelTrigger5` (−4373, 1556, 0), tag `ChangeLevel_Ch2Dracon` | Ch2 → **HP3_FA2Hermione**; drop `bSetGameState` (`GState065`). Gate: Draconifors |
| I5 | Retarget | `ChangeLevelTrigger6` (−1359, −2983, 941) | Ch3 → **HP3_FA3Harry**. Gate: Glacius |
| I6 | Gate | side room `ChangeLevelTrigger9` (971, −2049, 304) → Ch1 | Carpe Retractum (+ "Carpe Retractum Challenge" if `split_challenge_spells`) |
| I7 | Gate | `ChangeLevelTrigger10` (1309, −1722, 304) and `ChangeLevelTrigger7` (−3172, 2205, 547) → Ch2 | Draconifors (+ challenge item) |
| I8 | Gate | `ChangeLevelTrigger11` (974, −1397, 304) → Ch3 | Glacius (+ challenge item) |
| I9 | Gate + Spawn | DADA portcullis `KWMover27` (`DADA_PortToExpecto_Mvr`) + doors `KWMover66`/`67` (`DADA_DoorToExpecto_Mvr`), (856, −1844, 1152) | on the item: raise/open; spawn a `ChangeLevelTrigger` → **HP3_Adv2ExpPatronum** behind them. Return point here |
| I10 | Gate + Spawn | Library portcullis `KWMover53` (`Library_Port_Mvr`) + doors `KWMover52`/`13` (`Library_Door_Mvr`), (−704, −1368, 732) | on the item: raise/open; spawn → **HP3_Adv3Library**. Return point here (vanilla `7170Library_Exit` already lands nearby) |
| I11 | — | entrance-hall barred door south (733, 392, 74) | **unused**: GroundsDada is provisionally cutscene only. Spare |
| I12 | Gate + Spawn | entrance-hall barred door north: bar `StaticMeshActor42` (736, 1635, 91); player spot (731, 1615, 74) | on the item: hide the bar; spawn → **HP3_Adv1Express**. Return point here |
| I13 | Gate | `GroundsBlocker` portcullises `KWMover21`/`43`/`23` (≈ (1024, 880–1136, 0)) | "Grounds Access" item: open; until then shut |
| I14 | Check / start item | `MaraudersPickup0`/`1`/`2` (≈ (150, −870, 144), shortcuts room; no state group, so present in `090`) | `marauders_map_check` **off**: the map is granted at game start (below). **On**: the pickup spot is the check location and the map is an item in the pool |

## 2. HP3_GroundsHub

**Keep:** `ChangeLevelTrigger1` / `CutSceneTrigger15` (COMC course selection), `ChangeLevelTrigger2` (back to
InsideHub), the well quest (`RoundRobin1`/`0`).

| # | Change | Actor(s) | Detail |
| --- | --- | --- | --- |
| H1 | Spawn | Hagrid's hut door, player spot (813, −1121, 696) | `ChangeLevelTrigger` → **HP3_Adv6Paddock**. Gate: Paddock item. Return point here |
| H2 | Gate (goal) | `FinalDayDoor` (`Mover11`, (768, −6208, 864)) | open when the goal condition is met; it leads to **HP3_Final** |
| H3 | Verify | `ChangeLevelTrigger3` (COMC, `GState040` only, sets `050`) | inactive in `090`: no change |

## 3. HP3_DungeonHUB

| # | Change | Actor(s) | Detail |
| --- | --- | --- | --- |
| D1 | Gate + Spawn | dungeon locked door 1: bar `StaticMeshActor315` (−480, 2786, −318); player spot (−481, 2805, −341) | on the Shack item: hide the bar; spawn → **HP3_Adv5Shack**. Return point here |
| D2 | Verify | portcullis `Mover30` (`Bexit`) | the one-way exit of the Bowtruckle room; find its way in |

## 4. Challenges (Ch1/Ch1B, Ch2/Ch2B, Ch3/Ch3B)

In `GState090` each challenge already returns to InsideHub (`0700Ch1Chal_InsHubDoor`, `DracLoadInsideHub`,
`GlaciustoInsideHub`), and the story exits (Quidditch, BuckyExecuted, `GState065`/`075`) are inactive. The
**challenge-chamber copies** inside the challenge maps still link to the other challenges, though:

| # | Change | Actor(s) | Detail |
| --- | --- | --- | --- |
| C1 | Gate | Ch1 / Ch1B: `CutSceneTrigger29` (`0700Ch1Chal_DracDoor` → Ch2), `CutSceneTrigger32` (`0700Ch1Chal_GlacDoor` → Ch3); Ch1B: `ChangeLevelTrigger10` (→ Ch2), `ChangeLevelTrigger11` (→ Ch3) | same gates as I7/I8 |
| C2 | Gate | Ch2B: `ChangeLevelTrigger0` (→ Ch1), `ChangeLevelTrigger3` (→ Ch3), `ChangeLevelTrigger2` / `CutSceneTrigger27` (`6620Ch2Chal_DracDoor`, sets `065`) (→ Ch2) | same gates; drop the `065` state set |
| C3 | Gate | Ch3B: `ChangeLevelTrigger3` (→ Ch1), `ChangeLevelTrigger4` (→ Ch2), `ChangeLevelTrigger0` (→ Ch3) | same gates |
| C4 | Return point | each chamber copy's InsideHub exit | land in InsideHub's real side room (vanilla behaviour) |
| C5 | Note | Ch2 dragon section | a point of no return for two cakes (missable, not a softlock) |
| C6 | Softlock guard | Ch2 pumpkin pit (1343, −388, −725) | blocker or escape trigger without Spongify ([OPEN_CASTLE.md](OPEN_CASTLE.md) §7a) |

## 5. Story maps

| Map | # | Change | Detail |
| --- | --- | --- | --- |
| HP3_Adv1Express | A1 | Retarget | `0180ADV1DemExit` (→ GroundsDada, sets `GState000`) → **InsideHub**, return point I12; no state set |
| | A2 | Verify | the map's own start (`PlayerStart0`, `Cut_0010Intro`) works when entered from a door |
| HP3_Adv2ExpPatronum | A6 | Retarget | `CutSceneTrigger2` `7090PatronusExit` (→ Adv3Library) → **InsideHub**, return point I9 |
| HP3_Adv3Library | A7 | Keep | `CutSceneTrigger5` `7170Library_Exit` → InsideHub (return point I10) |
| HP3_Adv5Shack | A8 | Retarget | `ChangeLevelTrigger0` (`ChangeLevel_Willow2`) and `CutSceneTrigger3` `8700ADV5AtShack` (→ WhompingWillow, set `085`) → **DungeonHUB**, return point D1; no state set |
| | A9 | Verify | the map works in `090` (built for `085`; vanilla entered it in `075`) |
| HP3_Adv6Paddock | A10 | Retarget | `CutSceneTrigger12` `9080Adv6Flyaway` (→ DementorBattle) → **GroundsHub**, return point H1 |
| HP3_FA1Ron | A11 | Retarget | `CutSceneTrigger4` `RonExamExitDONE` (→ FA2) → **InsideHub**, return point at DADA (I3) |
| HP3_FA2Hermione | A12 | Retarget | `ChangeLevelTrigger1` (→ FA3) → **InsideHub**, return point at Transfiguration (I4) |
| HP3_FA3Harry | A13 | Retarget | `ChangeLevelTrigger0` (→ GroundsHub, sets `100`) → **InsideHub**, return point at Charms (I5); no state set |
| HP3_COMC | A14 | Verify | `CutSceneTrigger7` `4076COMCExit` (→ InsideHub, sets `060`) only fires after the tutorial track; otherwise suppress |

## 6. Not used (no changes needed)

Quidditch, WhompingWillow, BuckyExecuted, DementorBattle, Infirmary, DarkTower, and GroundsDada (provisionally
cutscene only; confirm after the final exams): nothing leads to them once A1, A6, A8 and A10 are retargeted. Their own exits stay as they are.

## 7. Open items before building

- GroundsDada: confirm it's cutscene only after the final exams (if it isn't, restore its entrance at I11).
- Adv5Shack in `090` (A9) and its party (G6).
- COMC tutorial exit (A14).
- How the classroom triggers I3–I5 are activated: some are cutscene-fired, so they may need to become touch triggers.
- Return-point mechanism (G5): a spawned `PlayerStart` with the right `SmartStart.PreviousLevelName`, or a teleport.
