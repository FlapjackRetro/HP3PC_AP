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
| S2 | **Marauder's Map as a pickup in front of the player** (no cutscene) | The visible map is a prop actor already in InsideHub: **`MarauderMap0`** (class `HGame.MarauderMap`, an `HPCharacter` with skeletal mesh `HProps.MarauderMap`), tag `Map`, parked out of sight at (−3386, −3585, 307) with no state group, so it's present in `090`. The `1005TaskedShop` cutscene shows it with `Teleport TaskedShopMap` and puts it back with `Teleport MapHideyHole`. The inventory item is separate (`MaraudersMap extends Inventory`), and `MaraudersPickup` is only a sparkle (`GameFX`). The mod: (1) moves `MarauderMap0` to just in front of the start spot, plus a `MaraudersPickup` sparkle; (2) watches the player's distance; (3) on touch, moves the prop back to its hiding spot, removes the sparkle, and either grants the map exactly as `HPAction_GiveMap` does (option **off**) or sends the check (option **on**; the map arrives as an item) |
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
| I12 | Gate + Spawn | 7th-floor locked door (east): bar `StaticMeshActor228` (1447, −956, 2171); player spot (1427, −960, 2154) | on the Express item: hide the bar; spawn → **HP3_Adv1Express**. Return point here |
| I13 | Gate | `GroundsBlocker` portcullises `KWMover21`/`43`/`23` (≈ (1024, 880–1136, 0)) | "Grounds Access" item: open; until then shut |
| I14 | Check / start item | the same pickup (S2) | see S2 |
| I15 | Gate + Spawn | entrance-hall barred door north: bar `StaticMeshActor42` (736, 1635, 91); player spot (731, 1615, 74) | on the Paddock item: hide the bar; spawn → **HP3_Adv6Paddock**. Return point here |

## 2. HP3_GroundsHub

**Keep:** `ChangeLevelTrigger1` / `CutSceneTrigger15` (COMC course selection), `ChangeLevelTrigger2` (back to
InsideHub), the well quest (`RoundRobin1`/`0`).

| # | Change | Actor(s) | Detail |
| --- | --- | --- | --- |
| H1 | — | Hagrid's hut door, player spot (813, −1121, 696) | **unused**: Adv6Paddock moved to the entrance hall (I15). Reserved for an outdoor map |
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
| HP3_Adv6Paddock | A10 | Retarget | `CutSceneTrigger12` `9080Adv6Flyaway` (→ DementorBattle) → **InsideHub**, return point I15 |
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

## 8. Test results

### poc1 (2026-10-26)

- Both hooks install. New Game → InsideHub in `GState090` works; the `090` intro cutscene ("Ron's exam is next")
  still plays at the start. It can stay for now.
- Entrance-hall north door → Adv6Paddock works. The paddock exit (→ DementorBattle) is rerouted to InsideHub, but
  the player lands in the **GState holding room** (around (−3650, −3700, 326)). That room holds every
  `GStateNNN` `PlayerStart` (the `090` one is at (−2424, −3597, 368)), plus parked cutscene props such as the
  hag's vendor cards. Those cards are props, not checks.
  - Why: `KWHeroController.TravelPostAccept` moves the player to the `SmartStart` whose `PreviousLevelName`
    matches `KWPawn.PreviousLevelName` (the map just left). InsideHub has SmartStarts only for GroundsHub,
    DungeonHUB, the three challenge B maps, BeanBonus and PP_B–F. Without a match, the player stays at the
    state's `PlayerStart`, and in vanilla a cutscene then moves them.
  - Fix (poc2, G5): during that one call, hand the pawn a nearby SmartStart's name, so Ron and Hermione follow too.
    Then `RelocatePawn` to the Open Castle door where there's no SmartStart close by.
- Side chamber: only the Carpe Retractum portcullis is raised in `090`; Draconifors and Glacius stay down (C1–C6).
- Shortcut portraits: "Looks like it's locked". The passwords (`pp_8`–`pp_21`) are only granted in `010`.
  poc2 calls `PortraitPasswordManager.EnablePortraitShortcuts` on every map load (option `start_unlocked`).

### Re-entering a story map: do checks stay collected?

Yes. The vanilla save system already handles it; nothing new is needed for the paddock.
- On every level change with `bItems=true`, `LevelInfo.ServerTravel` calls `SavePersistentData` on every actor.
  This covers vanilla `ChangeLevelTrigger`s and cutscenes (`KWPawn.LoadLevel`) as well as Open Castle's door
  travel. The values go into the `PersistentStore`, which travels with the player and is written into save games.
  Keys are `<Level.ID><ActorName><suffix>`, so every map keeps its own state.
- `KWPickup` (pasties, cakes, cards placed in the map) writes `_PU` when picked up and destroys itself on reload.
  It is on by default (`bPickUpPersistent=True`).
- `KWGenericSpawner` (chests, cauldrons, crates, statues) saves `_lives`. A spent spawner goes straight to
  `stateEnd` on reload (`bLivesPersistent=True` by default).
- The catalog's `persist_key` column now gives the real key, or blank where the map switches persistence off.
  Exceptions with persistence off (these reset on every visit):
  - every `AlohomoraPadLock` in Adv1, Adv6, Ch1, Ch2B, FA1, FA2, DarkTower and Final. They lock again, which is
    harmless because none of them is a check;
  - the jellybeans and `CauldronCake`s in the Ch2B/Ch3B chamber copies, and the jellybeans in Ch1B (farmable);
  - Ch2 `KnightSpawner10` and Ch3B `TapestrySpwn0`.
- **Gap:** goodies a spawner has already thrown out are dynamic actors, so nothing records them. Fixed in poc3
  (`mod/OpenCastle/leftovers.h`):
  - The mod links every new pickup to the nearest spawner opened this visit whose `GoodieToSpawn` list holds the
    pickup's class.
  - When the `PersistentStore` travels, it writes the indices still on the ground to `<id><spawner>_OCleft`.
    Items that faded or fell out of the world count as still on the ground.
  - On the next visit it reads that note in `PostPersistentDataRestored`, clears it, and spawns those goodies
    again above the spawner.
  - The randomizer should still send a chest's check as soon as the chest opens (its `_lives` reaches 0).
  - Known hole: after loading a save in the middle of a level, the drops already on the ground aren't linked
    (no `PostBeginPlay`), so they aren't tracked until the next level load.
- Event rewards (`HPSpawnThingy` rows, which have no `persist_key`) depend on whether their trigger, fight or
  Peeves persists. They need checking map by map; none are in the paddock.
