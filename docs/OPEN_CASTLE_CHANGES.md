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
| I1 | Suppress | `CutSceneTrigger13` (−98, −714, 1259), `9240RonExamIntroPt1` | Ron's exam intro (would load FA1 and set `100`). Unavoidable in the 3rd-floor corridor before DADA (confirmed in testing). **Done in poc6k**: its Event is set to None on map load |
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

### poc4: softlock escape

- **RETURN TO HUB**: the pause menu's X opens `HGame.HPPageExit` ("Exit game and return to Main Menu?"). poc4 adds a
  third button under YES / NO by reusing the dialog's invisible focus `decoy` button (`hubmenu.h`).
  - The first click arms it ("CLICK AGAIN TO RETURN TO HUB"), because the decoy holds the initial focus and a
    stray Enter could otherwise press it.
  - The second click closes the menus with the main menu's own `ExitMenu` (unpause, stop the menu music) and
    travels to InsideHub. The player arrives in the entrance hall (the GroundsHub SmartStart).
  - The button only appears if YES's `OnClick` delegate looks exactly as expected; otherwise the log says
    "button not added" and the dialog stays vanilla.

### poc4e test, poc5: door gates

- poc4e: RETURN TO HUB works every time. Leftovers work end to end:
  - InsideHub `ChestSpawner4` left 1 item, and it was thrown out again on return.
  - The paddock saved notes for 6 spawners (44 items).
- poc5: a door's loading trigger only works while the door is unlocked, and its portcullis follows that:
  - The DADA (`DADA_PortToExpecto_Mvr`) and Library (`Library_Port_Mvr`) portcullises are `TriggerToggle` `KWMover`s,
    the same ones the vanilla cutscenes trigger (`7200TaskedCharms`).
  - The mod calls `Trigger` on the mover whenever its `KeyNum` disagrees with the door's state.
  - The test switch is `Documents\Harry Potter and the Prisoner of Azkaban\HP3_OpenCastle.ini`, `[Doors]` with
    Paddock / Express / DADA / Library / Shack set to 1 or 0. It is read every 2 s. The randomizer's items will
    drive the same switch.

### poc6l: challenges, second New Game

- **Second New Game** (from the title screen without restarting) used to start the vanilla Express opening,
  because the travel came from the last loaded map rather than HP_Preamble. Now any travel to `HP3_Adv1Express`
  with `GameState=GSTATE000` counts as New Game. Our 7th-floor door sends no GameState.
- **Reward check for the other two challenges.** Unlike COMC, neither has the track-1 slip:
  - Doxy (GroundsHub, 5 `PixieGame`s): the well's companion spell (`CompanionSpellTrigger10` →
    `PixieBattleRoundRobin`, persistent) runs rounds 1–5 in order. Each round's win teleports its placed card:
    `WinCard1`–`5` → Bran the Bloodthirsty, Cyclops, Goliath, Morholt, Hengist of Upper Barnton.
  - Monster Book (InsideHub): `CompanionSpellTrigger3` → `MBRobin` (persistent) → `MBDis1`–`5`. Each round's win
    teleports its card, `MBWC1`–`5` → Babayaga, Malodora Grymm, Leticia Somnolens, Old Mother Hubbard, and Cordelia
    Misericordia (round 5's comes from cutscene `7240MBOMWin_05Exit`).
  - No GState groups are involved, so the rewards are the same in `GState090`.
- **Gating.** `[Doors]` keys `MonsterBook` / `Hippogriff` / `Doxy`:
  - Monster Book: the room's door is three movers tagged `MBOMDoor` (portcullis `KWMover19` and doors
    `KWMover37`/`6`). They're `TriggerToggle` movers that `7200TaskedCharms` opens, so they aren't static. They are
    raised or lowered like the other portcullises. The start trigger is switched off too, as a backup.
  - Hippogriff and doxy have no door, so their start triggers are switched off while locked:
    - GroundsHub `CutSceneTrigger15` (`ChangeLevel_COMC_MINIGAME`, the walk-in to the course selection);
    - `CompanionSpellTrigger10` (the doxy well).
  - A locked challenge shows no sign yet. One option: float the drawn key sprite at the entrance while it's locked.
- **Hippogriff path (poc6l test):** with the walk-in trigger off, the path past the COMC arch still leads out of
  bounds. poc6m adds a blocker (`blocker.h`) at the arch (GroundsHub `StaticMeshActor260`, `COMCgate_SM` at
  (−1868, 992, 576)) while `Hippogriff` is locked:
  - the castle portcullis mesh (`Portcullis_SM`, ×2.0, so 256 × 408 units) on an inert `Effects` actor;
  - a blocking cylinder (radius 135, half-height 210), set through `SetCollisionSize` + `SetCollision`.
  - It's removed when the key unlocks, and spawned again on each visit while locked.

### poc7: item pickups (test grid)

The Delete grid is now a set of working pickups (`aptest.h`): each item falls to the floor (PHYS_Falling,
bCollideWorld, a collision cylinder) and is collected by walking into it. What each one does is the same switch
the randomizer client will drive, in `HP3_OpenCastle.ini`:

| Pickup | Effect |
| --- | --- |
| 10 spell glyphs | `[Spells] <spell>=1` |
| drawn key | `[Doors] Paddock=1` (test) |
| hippogriff / doxy / Monster Book | `[Doors] Hippogriff` / `Doxy` / `MonsterBook` `=1` |
| Marauder's Map | `[Items] MaraudersMap=1` → `HPHeroController.GiveMap()` (on every map load while set) |
| password scroll (`BlueBandScroll_SM`) | `[Items] ShortcutPasswords=1` → `EnablePortraitShortcuts()` |
| Archipelago logos | nothing |
| pasty, cake, Harry Potter card | the game's own pickup classes |

New mechanisms (`unlocks.h`):
- **Spell gating.** `WeaponWand.HasSpell` is turned into a native function at runtime (iNative + Func on its
  UFunction; Core.dll's CallFunction calls Func whenever iNative ≠ 0). It returns the vanilla answer unless the
  spell is locked in `[Spells]` (default 1 = has it). The spell cursor won't target, and ChooseSpell won't pick, a
  locked spell. Covers LumosSpell (gargoyles). Not covered yet: companion-cast spells (`CompanionSpellTrigger`).
- **Shortcut passwords are locked by default** (`[Items] ShortcutPasswords`, default 0): on every map load,
  entries 8–21 (but 18, vanilla's free one) are cleared until the item is set. Earlier builds unlocked them all.
- **Marauder's Map** (`[Items] MaraudersMap`, default 0).

### poc7c: unlock state lives in the save

- `HP3_OpenCastle.ini` is now only the **starting** state (stand-in for slot data / starting inventory). Pickups no
  longer write to it.
- What's found is kept in the game's PersistentStore as `OC_<Section>_<Key>` = `1` (e.g. `OC_Spells_Alohomora`,
  `OC_Doors_Paddock`, `OC_Items_ShortcutPasswords`). The store travels between levels and is written into save
  games, so a New Game starts with nothing found and loading a save brings back that save's unlocks.
- Effective state = ini OR save. Read on every settled map load (log: `save flags (found): ...`).
- The randomizer client will write received items to the same keys.

### poc7d: crash fix (GPF in RenderLevel on a pickup)

- poc7c tested: found unlocks survive save / quit / load, and a New Game starts with none (`save flags (found): none`).
- Crash: picking up the Rictusempra glyph gave a GPF in `RenderLevel`. The mod's queued game-thread jobs ran on
  whatever ProcessEvent came next, including events fired while the frame is drawn (HUD / canvas). Destroying the
  picked-up actor (or spawning the sparkle) mid-render can leave the renderer with a dead actor.
- Fix: jobs that spawn, destroy or change actors (pickups, grid, leftovers, gates, blockers, Map / shortcut grants)
  run only from `PlayerTick` / `Tick` events, i.e. while the level ticks. Travel and the menu patch are unchanged.

### poc8: debug panel (test only, not for release)

- **Delete** opens an in-game panel with a checkbox for every unlock; **backslash** (`\`) spawns the old pickup grid (poc8a; Insert in poc8).
  Arrows move, Enter / Space toggles, Delete closes. The game pauses while it's open (`[Debug] PanelPause=0` to
  leave it running).
- Rows: the 10 spells; Marauder's Map; Shortcut Passwords; the 8 level / challenge unlocks wired so far (Paddock,
  Express, DADA, Library, Shack, Monster Book, Hippogriff, Doxy); Portrait Password 1–8 (entries `pp_0`–`pp_7`,
  in sale order, which portrait each opens is still to map); placeholders for the Carpe Retractum / Draconifors /
  Glacius challenge doors (not wired). Actions: +10 pasties, +10 cakes, +100 beans, untick everything found.
- Toggling writes the save flag (`OC_*`, now `1` or `0`). Items forced on by the ini start state show `(start)` and
  can't be unticked from the panel.
- New: unticking the Marauder's Map takes it out of the inventory; portrait passwords 1–8 follow their flags
  (`[Items] Portrait1`–`8` in the ini, `OC_Items_Portrait<n>` in the save).
- Drawn from the HUD's `PostRender` with Engine.dll's `UCanvas::DrawTile` / `WrappedPrintf`. Fallback if this
  doesn't work out: a separate Win32 window with checkboxes.

### poc8b: Wizard Card bonus room on the entrance hall south door

- HP3_BeanBonus (5 cards: Dumbledore and the four founders) is decoupled from the 74-card Harry Potter card: the
  entrance hall's south door (733, 392, 74; lock bar `StaticMeshActor50`) leads there, `[Doors] BonusRoom`, item
  Bonus Room Unlock (drawn key; a second key on the test grid, a row in the debug panel).
- Vanilla teleport off: InsideHub `ChangeLevelTrigger14` (Tag `WCBonusRoom_Trig`, fired by `VI_FinalWizardCard0`'s
  `GotCardEvent` → `Dispatcher10`) gets Tag None. The SUPPRESS table can now switch a Tag as well as an Event.
- Return: the room's exit (`ChangeLevelTrigger1` → HP3_InsideHub) lands at the south door (entrance-hall
  SmartStart + RelocatePawn).
- MAXGATES 8 → 16 (nine doors now).

### poc8c: everything starts locked

- The ini no longer sets unlocks: `[Doors]`, `[Spells]` and `[Items]` are ignored. Every spell, door and item starts
  locked; only the save (`OC_*` flags: pickups, the debug panel, later received items) turns things on. The ini
  keeps `[Shop] Cheap` and `[Debug] PanelPause`.
- New Game: the moment the New Game travel is seen, the cached found flags are dropped (so the previous game's
  spells don't work for the ~3 s before the new, empty save is read).
- The debug panel's "(start)" marker can't appear any more.

### poc8d: challenge doors in the side room

- The side room's three portcullises are gated like the other doors: `ChalChamb_CR_Port` (KWMover31),
  `ChalChamb_Drac_Port` (KWMover32), `ChalChamb_Gla_Port` (KWMover33). They're the reverse of DADA / Library:
  key 0 = up (z 392), key 1 = down (136 lower). In `GState090` Carpe's starts up, the other two down. Door keys
  `ChCarpe`, `ChDracLap`, `ChGlacius`; all locked by default; rows in the debug panel.
- Their loading triggers are switched off (no collision) while locked: `ChangeLevelTrigger9` (Ch1),
  `ChangeLevelTrigger10` and the chamber's second Draconifors trigger `ChangeLevelTrigger7` (Ch2),
  `ChangeLevelTrigger11` (Ch3). `DOORS` gained `inv` and `trig` fields for this.
- The B maps (Ch1B, Ch2B, Ch3B) each have a copy of the chamber with triggers into all three challenges. Rather than
  gate each copy, the travel hook refuses any travel into a locked challenge's map (`HP3_Ch1CarpeRetractum` /
  `HP3_Ch2Draconifors` / `HP3_Ch3Glacius`) and sends the player to InsideHub instead.
- Spells and challenge access stay separate unlocks for now (logic tracing).
- Doors now start locked from the first frame (the worker used to assume unlocked until the first poll).

### poc8e: first-visit and replay cutscenes both played (Ch1 softlock)

- Reported: in HP3_Ch1CarpeRetractum the imp portrait fell and sprang back; after the imps, the trapdoor over the
  Spongify pad (`Mover23`, `CRTrapdoorZ1`) opened and shut at once, and the pad couldn't be reached (softlock).
- Cause: Ch1 pairs each scripted moment with a first-visit cutscene trigger (`bPersistent`, fires once, e.g.
  `6033Ch1GnomeGetsUp`, `6035Ch1HoleQC`) and a replay one on the same Tag (`…_02`, `ExcludeFromGameStates` GState000).
  Vanilla plays Ch1 first in `GState000` (only the first-visit one) and later visits have it already fired. Open
  Castle enters in `GState090` with nothing fired, so both play, and every `Trigger` in them runs twice: the portrait
  (a toggle mover) falls and returns, the trapdoor opens and closes.
- Fix (`dedupe_cutscenes`): on each new map, for two CutSceneTriggers with the same Tag where one's Event is the
  other's plus `_02`, the first-visit one loses its Event. In the retail maps that's four pairs, all in Ch1:
  `6033Ch1GnomeGetsUp`, `6035Ch1HoleQC`, `6040Ch1PixiesQC`, `6045Ch1BeatPixiesQC`. The cost: the first-visit
  dialogue lines in those scenes.

### poc8f: keep the first-visit cutscene, drop the replay (pasties back)

- poc8e switched off the first-visit trigger, but the replay cutscenes aren't just shorter: `6035Ch1HoleQC_02`
  leaves out `Trigger SpawnPastiesImp` (→ `Dispatcher14` → the `SpawnPasties3` spawners), so the three pasties
  by the Spongify pad never appeared (marked in play at (1275, 2247, 866)).
- Now the other way round: while the first-visit trigger is still active (it hasn't fired in this save;
  `KWTrigger` saves `bInitiallyActive` as `<LevelID><Name>_bTOO`), the **replay** trigger loses its Event, so the
  first visit plays like vanilla's, rewards and dialogue included. Once it has fired, nothing is touched and later
  visits play the replay, as in vanilla.

### poc8g: final exam doors

- Three new level unlocks (door keys `ExamRon`, `ExamHermione`, `ExamHarry`; debug-panel rows; apworld items
  Ron's / Hermione's / Harry's Final Exam Unlock, offsets 113–115). Each raises its classroom portcullis and switches
  its loading trigger on; all start locked.

| Exam | Portcullis (key 1 = up) | Loading trigger (vanilla, retargeted) | Voice line switched off |
| --- | --- | --- | --- |
| Ron (FA1) | `DADA_PortToCarpe_Mvr` (KWMover64; starts up in `GState090`) | touch `Trigger58` → `ChangeLevelTrigger3` (`HP3_Ch1CarpeRetractum` → FA1) | `PopupTrigger10` (`PC_HRY_CHALCH1COMMENTS_6`) |
| Hermione (FA2) | `ToDrac_Prt_Mvr` (KWMover50; starts down) | `ChangeLevelTrigger7` (`HP3_Ch2Draconifors.unr` → FA2, by player position) | `PopupTrigger17` (`PC_HRY_DOORCOMMENTS_34`) |
| Harry (FA3) | `Glacius_Port_Mvr` (KWMover56; starts down) | `ChangeLevelTrigger6` (`HP3_Ch3Glacius` → FA3) | `PopupTrigger13` (`PC_RON_DOORCOMMENTS_40`) |

- Correction to poc8d: `ChangeLevelTrigger7` isn't a second side-room Draconifors door, it's the Transfiguration exam
  door (the "ToDrac" door below the classroom; the in-classroom triggers `Trigger47`/`59` exist only in `GState060`).
  Its URL is the same as the side room's (`HP3_Ch2Draconifors.unr`), so the worker flags when the player stands at it
  (`gAtTransfigDoor`, within 250 of (−3171, 2204)) and rules.h sends that travel to FA2. Before this it went to the
  Draconifors challenge.
- The travel gate now looks at the destination after Open Castle's own rewrite, so a classroom door is checked
  against its exam (not the challenge whose map name the vanilla trigger carries).
- Return points: each exam drops the player in front of its classroom door (entrance-hall SmartStart, then
  RelocatePawn to where the voice-line trigger stood), not in the side room.
- Voice lines: a `SILENCE` table switches those touch PopupTriggers off (collision off) on every map load.

### poc8h: noclip (test tool)

- **F6** (or the debug panel's new "Noclip" row under ACTIONS) toggles noclip, to reach spawns that can't be found on
  foot while finishing the logic. Test-only, like the panel: remove from release builds.
- While on: move with the game's own movement keys (camera-relative; WASD / arrows as a fallback), **E / Page Up**
  up, **Q / Page Down** down, **Shift** x4 speed. Collision is off, so nothing is picked up and no trigger fires:
  turn it off above a floor next to the item, and the player falls onto it.
- How (`noclip.h`): right after the player controller's `PlayerTick`, the pawn gets collision off
  (`AActor::SetCollision`, the previous flags kept for turning it back on), `bCollideWorld` cleared and `PHYS_None`
  (`AActor::setPhysics`); each frame it is moved with `ULevel::FarMoveActor` (no encroachment check), and Velocity /
  Acceleration are zeroed. Off: the saved collision flags, `bCollideWorld` and `PHYS_Falling`. A level load turns it
  off for the old pawn and on again for the new one.
- MapTracker v13 (separate plugin) shows the live position in the title bar: `X … Y … Z … facing …`, 4 times a second.

### poc8i: faster noclip

- Noclip speed x10: 7000 units a second, 28000 with Shift (poc8h's 700 was a walk). Tested in play: noclip turns on
  and the game's own forward axis drives it (`aForward` 24000 at full stick).
- `install_opencastle.bat` also copies `HP3MapTracker.asi` when it's next to it (the title-bar coordinates didn't show
  because MapTracker v12 was still installed).

### poc8j: noclip x10 again

- 70000 units a second, 280000 with Shift.
