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
   came from v5's stale-controller bug. **Settled by Ch2 (MapTracker v12, 17:21:29):** after the end of a challenge the
   player stands in the copy of the chamber inside the challenge's `B` map (`HP3_Ch2DraconiforsB`), not InsideHub.
   Walking out of that copy triggers the next story transition. Entered normally from the castle, the chamber is
   InsideHub's own.
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

### Story beats in InsideHub, `GState010` (confirmed in play)

1. **Follow Fred to the twins' shop.** Shop intro and exit cutscenes are `1040IntroShop` / `1051ShopExit` (they also
   reposition students: `KidsToFandG_Spawn`, `KidsToChChamb_Spawn`, …).
2. **Peeves fight for the shortcut passwords.** On leaving the shop, a cutscene leads into a fight with Peeves. Winning
   it gives **the shortcut passwords**. It's set up at the end of `1051GetMaraudersMap` (`Trigger PeevesEnc1`).
   There are three `Peeves` actors in InsideHub (`Peeves0` tag `Peeves3` at (−126, 1392, 417), `peeves1` tag `Peeves`
   at (−3730, −3790, 435), `peeves2` at (−3597, 1059, 742)); a beaten Peeves saves `<LevelID><Name>_bD`. Which one
   this fight uses: *to confirm*.
3. **Take a shortcut down to the shortcut chamber** (now unlocked by the passwords).
4. **Neville's cutscene in the shortcut chamber:** he offers to guide the trio to Care of Magical Creatures.
5. **Neville leads them out through the main doors** into HP3_GroundsHub (confirmed). In `GState010` the exit is the
   `LevelLoadGrounds010` cutscene, which sets **`GState040`** (MapTracker 12:08:27: `HP3_GroundsHub  state=GState040`).
   In other states the plain `LevelLoadGrounds` is used and the state is kept.

These scenes have no `.int` cutscene file of their own (only the shop and Marauder's Map scenes do), so they're
probably scripted in the map (`HPScriptedSequence` / `CutSceneTrigger` actors).

**Open Castle:**
- The shortcut passwords are a story-granted unlock. Decision: an optional progression item, "Shortcut
  Passwords" (option `shortcut_passwords: item | start_unlocked`); see [OPEN_CASTLE.md](OPEN_CASTLE.md) §3.
  Vanilla grants them with `HPAction_GivePortShortcuts` (entries 8–21 of `PortraitPasswordManager`, keys
  `pp_8`–`pp_21`).
- The Peeves fight needs Rictusempra. Beating Peeves could be a check (`_bD` key; 3 Peeves in InsideHub).
- These story cutscenes don't play in the Open Castle base state, so nothing else is lost.

## 6. HP3_GroundsHub: first visit (`GState040`)

| | |
| --- | --- |
| Level ID | `ghub` |
| State | `GState040` (set on the way out of InsideHub) |
| Entered from | InsideHub main doors, led by Neville (`LevelLoadGrounds010`) |
| Checks | *(in progress)* |

**Optional side area: the pixie-infested well** (the doxy well side quest; the game's creatures are `Poxie`). Five
stages, run by `RoundRobin1` "PixieBattleRoundRobin" (battles) and `RoundRobin0` (exit cutscenes), saved as
`ghubRoundRobin1_iOEI` / `ghubRoundRobin0_iOEI`. Stage rewards are wizard cards: Bran the Bloodthirsty, Cyclops,
Goliath, Morholt, Hengist of Upper Barnton (teleported to the player by `HPSpawnThingy`). Confirmed: entering it is a
short fade to black that moves the player next to the well, still in HP3_GroundsHub, not a map change. In Open
Castle the well quest needs no entrance of its own: it's part of GroundsHub.

| Well stage | Reward | Status |
| --- | --- | --- |
| 1 | Bran the Bloodthirsty | **found** (first visit, `GState040`) |
| 2 | Cyclops | |
| 3 | Goliath | |
| 4 | Morholt | |
| 5 | Hengist of Upper Barnton | |

Stage 2 is available right after stage 1 (confirmed). The data backs this up for every stage: the five battles
(`PBattle1Dispatch`–`PBattle5Dispatch`, picked in turn by `RoundRobin1`) and their `PoxieManager`s carry no game-state
group or exclusion. So all five stages can be played back to back, in any story state where the well is reachable.
Logic: the five cards need only GroundsHub access (plus whatever spell the doxy fights take, presumably Rictusempra).

## 7. HP3_COMC: Care of Magical Creatures, hippogriff riding (first visit, `GState040`)

| | |
| --- | --- |
| Level ID | `comc` |
| State | `Gstate040` (written with that casing by the transition cutscene) |
| Entered from | GroundsHub, following Neville. In `GState040` this is the `ChangeLevelCOMC` cutscene (MapTracker 12:14:01). Later visits use `ChangeLevel_COMC_MINIGAME` or `ChangeLevelTrigger1`. |
| Checks | **5**, all hippogriff-riding rewards; nothing placed (no pasties, cakes, secrets or chests) |

| Track | Pass score | Reward (`HPSpawnThingy`, tag) | Status |
| --- | --- | --- | --- |
| 1 (tutorial) | 1500 | Joscelind Wadcock (`WizCard1`) | **found**, given during the win cutscene |
| 2 | 2800 | Gwenog Jones (`WizCard2`) | |
| 3 | 4000 | Cyprian Youdle (`WizCard3`) | |
| 4 | 4900 | Roderick Plumpton (`WizCard4`) | |
| 5 | 5600 | Bowman Wright (`WizCard5`) | |

Progress is saved by `HippoFlyGame` (`hfg_n` tracks completed, `hfg_h<i>`/`hfg_l<i>` scores). Win cutscenes:
`COMCWin_WizCard2`–`5`, `COMCWin_2ndTimeonTrack`; loss: `COMCTutorialLose`, `COMCLose_AllTracks`.

State note: GroundsHub also has a walk-in entrance to COMC, `ChangeLevelTrigger3`, active only in `GSTATE040`, which
sets `GState050` on the way in. The Neville cutscene keeps `040`. When `050` takes over: *(to confirm)*.

**End of the first visit (confirmed):** winning track 1 plays a cutscene; the card is awarded during it. The
cutscene then sets **`GState060`** and sends the player straight back to the castle, arriving in the shortcuts room
of InsideHub. MapTracker: `HP3_COMC state=GState060` at 12:17:29, then `HP3_InsideHub state=GState060` at 12:17:36.
This is `4076COMCExit` (COMC → InsideHub, sets `GState060`). On later visits COMC exits to GroundsHub
(`ChangeLevelTrigger0`).

So in vanilla only track 1 is played on this visit. Tracks 2–5 come on later visits.

**Later visits: course selection (confirmed, `GState060`).** Walking back from GroundsHub into the COMC loading
trigger brings up a **Hippogriff Course Selection** menu (title bar: `HP3_COMC | GState060`) instead of the
tutorial. It lists the five tracks with high score, last score and a star for each passed track. Tracks unlock in
order: after track 1 (passed, 2900, star), track 2 is selectable and 3–5 are greyed out. The menu has Play and
Quit. This matches the `ChangeLevel_COMC_MINIGAME` entrance.

Logic: track *n* needs track *n−1* passed (`hfg_n`). In Open Castle the five cards form a chain inside COMC;
nothing outside the map is needed apart from reaching it (no spells).

**Open Castle:** COMC is a separate map with its own entrance from GroundsHub; all five checks are here. The story
exit `4076COMCExit` (and its `GState060` change) must not fire in Open Castle; the base state (`090`/`100`) should
use the normal exit to GroundsHub *(verify)*.

## 8. HP3_InsideHub, `GState060`

Arrived in the shortcuts room straight from the COMC win cutscene (confirmed, 12:17:36).

## 9. HP3_GroundsHub, `GState060`: candidate entrances

- **Hagrid's hut door:** a closed wooden door with no loading zone in vanilla (GroundsHub, `GState060`, screenshot
  12:25). Candidate spot for an Open Castle entrance to a grounds story map (Adv6Paddock / BuckyExecuted are set at
  Hagrid's). Marked at **(813, −1121, 696)** (MapTracker v12, End key, 13:03:43). Nothing
  is placed within ~450 units except an ambient-sound trigger, so a new trigger volume can go at the door without
  clashing with existing actors.

## 10. HP3_InsideHub, `GState060`: candidate entrances

- **Barred doors either side of the entrance hall** (ground floor, flanking the main doors at x ≈ 1360). Marked at
  **(733, 392, 74)** and **(731, 1615, 74)** (13:06:00 and 13:06:35). They're decoration: each is a static
  `TudorDoorL_SM` door mesh with a static `DoorLockBar_SM` bar across it in a `DoorArchTudorQuad_SM` arch, plus a
  `PopupTrigger` in front (`PopupTrigger21` / `PopupTrigger7`, probably the "it's locked" hint). There's no mover and
  no trigger, so nothing in vanilla opens them.

  **Open Castle:** good reusable entrances. A spawned `ChangeLevelTrigger` (or a use trigger replacing the popup)
  can send the player to a story-only map, and the lock bar can be hidden once the matching area item is received
  (a visible "unlocked" cue). Both sit next to the grounds exit, which suits grounds story maps (e.g. Adv5Shack,
  Adv6Paddock, BuckyExecuted) or the Hogwarts Express prologue.

## 11. HP3_DungeonHUB: the dungeons (first visit, `GState060`)

| | |
| --- | --- |
| Level ID | `dung` |
| State | `GState060` (MapTracker 13:07:58); the map's own default is `GState070` |
| Entered from | InsideHub: from the entrance hall, down a staircase, into `ChangeLevelTrigger8` (225, 2968, −176), beside `Dungeon_Door_Mvr`. Confirmed open and walkable in `GState060`. |
| Leaves to | InsideHub (`ChangeLevelTrigger16`) |
| Checks (catalogue) | cards *Doxy* (placed, `WC_Doxy0` (−869, 1627, −430)) and *Bowtruckle* (`ChestSpawner3` (−1248, 4389, −359), with 4 cakes); **23 pasties and 3 cakes spawned by events** (tag `Aspawnbeans` ×17, and `Bs2`…`Bs22` ×9); no secret areas |

The 26 event-spawned snacks (`HPSpawnThingy`) are the largest group in the game: how they're triggered is
*(to confirm in play)*.

**Locked doors and portcullis** (confirmed, marked in this order):

| # | Mark | What it is |
| --- | --- | --- |
| 1 | (−481, 2805, −341) | Locked door: static `RomanesqueDoorL_SM` + `DoorLockBar_SM` in a round arch. No mover; decoration like the entrance-hall doors. Two Alohomora triggers just beside it (`PortPass_Stroulger_Trig`, `PortPass_Glover_Trig`) belong to portrait passwords, not this door. |
| 2 | (−473, 4107, −341) | Locked door, same static set (`StaticMeshActor57`/`95`). Decoration. |
| 3 | (−1251, 3822, −341) | **Portcullis, down:** `Mover30`, tag `Bexit`. It's a real mover, opened by `Trigger1` at (−1427, 4102, −291), which is *inside* the room. So it's the room's one-way exit: the room is entered another way and the portcullis lets the player out. The *Bowtruckle* card chest (`ChestSpawner3` (−1248, 4389, −359)) is in this room. |

Open Castle: doors 1 and 2 are spare entrances (like the entrance-hall pair). The portcullis room's entrance and its
requirements are *(to confirm)*; the Bowtruckle card depends on it.

**Open Castle:** in `GState060` the dungeon door is already open, so the planned "Dungeon" area unlock
(`Dungeon_Door_Mvr`) would have to close it in the base state *(check whether the door is shut in `090`/`100`)*.

## 12. HP3_InsideHub, `GState060`: second Peeves fight (by the Transfiguration classroom)

Confirmed: on the first floor, in the room next to the Transfiguration classroom, a cutscene brings Peeves in. The
scripted fight has **Ron use Carpe Retractum to pull Peeves's armour off**, and a closing cutscene drops
**4–5 pasties**.

The data says **4**: `peeves2` (−3597, 1059, 742), group `GState060`, `EventToSendWhenDefeated` `Peeves2Done`, with
four spawners `peeves2_Spwn_Pastie_01`–`04` (`HPSpawnThingy13`/`14`/`16`/`18`, around (−3570, 780, 488)).

The three Peeves fights in InsideHub, one per story state:

| Fight | Actor | State | Needs | Reward |
| --- | --- | --- | --- | --- |
| 1, outside the twins' shop | `peeves1` (−3730, −3790, 435) | `GState010` | (Rictusempra?) | **Shortcut Passwords** (`EventToSendWhenDefeated` `PortShort`) |
| 2, by Transfiguration | `peeves2` (−3597, 1059, 742) | `GState060` | **Carpe Retractum** (Ron) | 4 pasties |
| 3, entrance hall | `Peeves0` tag `Peeves3` (−126, 1392, 417); cutscenes `9225ADV4PeevesIntro` / `9227ADV4PeevesExit` | `GState100` | *(to confirm)* | 3 cakes (`Peeves3_Spwn_Cake_01`–`03`, around (435, 1000, 72)) |

**Open Castle:** each fight is filtered to one story state, so only one (or none) is present in the base state.
They'd need spawning or un-filtering to be playable; their rewards (and the `_bD` defeat flag) are candidate checks.

## 13. HP3_Ch2Draconifors: Draconifors challenge (Hermione), first visit

| | |
| --- | --- |
| Level ID | `dl` (second half `dlb`) |
| State | `GState060` (MapTracker 13:17:49) |
| Entered from | InsideHub, Transfiguration classroom: walking in plays the class cutscene (`6180TransIntro`, `GState060` only), which loads the challenge with **Hermione** as the player character (confirmed) |
| Checks | *(in progress)* |

Catalogue, both halves:
- **Cards (5):** *Hebridean Black Dragon* (`Pumpkin2` (1343, −388, −716), a pumpkin, beside secret area
  `SecretAreaMarker0`); *Common Welsh Green Dragon* (`KnightSpawner2` (−3462, 624, −139), a Depulso knight); and in
  Ch2B *Norwegian Ridgeback* (`ChestSpawner12`, by secret area `SecretAreaMarker1`), *Romanian Longhorn*
  (`ChestSpawner19`, by `SecretAreaMarker6`), *Hungarian Horntail* (`ChestSpawner13` (2657, 857, 346)).
- **Secret areas (8):** Ch2 `SecretAreaMarker0`/`2`/`3`/`4`; Ch2B `SecretAreaMarker1`/`5`/`6`/`7`.
- **Pasties/cakes:** Ch2 has 16 placed pasties and 4 placed cakes; Ch2B has 12 and 14.
- 12 challenge stars (Ch2 8, Ch2B 4), so probably ~10 shields once overlaps are removed, as in Ch1; 12 creature
  statues (Draconifors/Lapifors targets).

Found so far:

| Check | Where | Requires | Status |
| --- | --- | --- | --- |
| Pasty near the start | probably `PumpkinPasty2` (−2830, 1431, −217), the closest placed pasty to the start (~1400 units from `SmartStart0` (−4228, 1272, −144)) | **rabbit form (Lapifors)**, confirmed | **found** |
| Card *Common Welsh Green Dragon* | `KnightSpawner2` (−3462, 624, −139), suit of armour in the room after the first | **Depulso**, confirmed | **found** |
| Pasty | `PumpkinPasty1` (−2912, −672, −219); marked at (−2917, −680, −204), 18 units away | **rabbit form**, confirmed | **found** |
| Pasty | `PumpkinPasty22` (−2159, −652, −203); marked at (−2164, −661, −204) | **rabbit form**, confirmed | **found** |
| Pasty | `PumpkinPasty4` (−1535, −654, −203); marked at (−1533, −653, −204) | **rabbit form**, confirmed | **found** |
| Pasty | `PumpkinPasty13` (−1855, −1519, −203); marked at (−1848, −1511, −204) | **rabbit form**, confirmed | **found** |
| Pasty | `PumpkinPasty3` (−1533, −1464, −49); marked at (−1537, −1467, −49) | **rabbit form**, confirmed | **found** |
| Pasty | `PumpkinPasty10` (−1269, −1030, −58); marked at (−1284, −1033, −44) | **rabbit form**, confirmed | **found** |
| Pasty | `PumpkinPasty11` (433, −382, 239); marked at (416, −378, 259) | **dragon form (Draconifors)**, confirmed | **found** |
| Pasty | `PumpkinPasty20` (1349, 96, −72); marked at (1338, 104, −51) | **dragon form**, confirmed | **found** |
| Pasty | `PumpkinPasty9` (1277, −392, 252), in midair above the pumpkin pit; marked at (1288, −368, 244) | **dragon form**, confirmed | **found** |
| Pasty | `PumpkinPasty12` (1428, −400, 252), in midair; marked at (1418, −446, 234) | **dragon form**, confirmed | **found** |
| Cake | `CauldronCake8` (1342, −386, −316), above the pumpkin pit; marked at (1344, −391, −298) | **dragon form**, confirmed | **found** |
| Pasty | `PumpkinPasty18` (3364, −1503, 53); marked at (3351, −1502, 51) | **rabbit form**, confirmed | **found** |
| Pasty | `PumpkinPasty21` (3559, −1098, 343); marked at (3552, −1113, 347) | **dragon form**, confirmed | **found** |
| Pasty | `PumpkinPasty0` (4606, −1264, 416), bottom of the column; marked at (4608, −1256, 436) | **dragon form**, confirmed | **found** |
| Pasty | `PumpkinPasty15` (4608, −880, 657), middle of the column; marked at (4598, −895, 676) | **dragon form**, confirmed | **found** |
| Pasty | `PumpkinPasty17` (4607, −463, 904), top of the column; marked at (4604, −460, 924) | **dragon form**, confirmed | **found** |
| Pasty spawner (a pumpkin) | `Pumpkin6` (−6, −1084, −439); marked at (−10, −1068, −416). Pasties, class-default count (2, see below) | **Depulso**, confirmed | **found** |
| Pasty spawner (a pumpkin) | `Pumpkin5` (−255, 297, −437); marked at (−250, 299, −416). Pasties, class-default count (2, see below) | **Depulso**, confirmed | **found** |
| Pasty spawner (a pumpkin, 1 pasty) | `Pumpkin9` (944, −383, −725); marked at (944, −377, −704) | **Depulso**, confirmed | **found** |
| Pasty spawner (a pumpkin, 1 pasty) | `Pumpkin8` (1343, −795, −730); marked at (1339, −791, −706) | **Depulso**, confirmed | **found** |
| Card *Hebridean Black Dragon* | `Pumpkin2` (1343, −388, −716), the large pumpkin in the middle of the pumpkin room; marked at (1329, −387, −704). Secret area `SecretAreaMarker0` is on the same spot | **Depulso**, confirmed | **found** |
| Pasty spawner (a pumpkin, 1 pasty) | `Pumpkin7` (1783, −381, −729), east arm; marked at (1764, −382, −705) | **Depulso**, confirmed | **found** |
| Pasty spawner (a pumpkin, **2 pasties**) | `Pumpkin3` (1343, 21, −728), north arm; marked at (1360, 16, −704) | **Depulso**, confirmed | **found** |
| Pasty spawner (a pumpkin, 2 pasties) | `Pumpkin0` (3022, −596, 77); marked at (3022, −580, 95) | **Depulso**, confirmed | **found** |
| Pasty spawner (a pumpkin, 2 pasties) | `Pumpkin12` (3018, −913, 78); marked at (3025, −910, 95) | **Depulso**, confirmed | **found** |
| Pasty spawner (**edible grass**, 1 pasty) | `KnightSpawner0` (2672, −1704, 78); marked at (2691, −1704, 51) | **rabbit form** (eaten), confirmed | **found** |
| Cake chest (2 cakes) | `ChestSpawner21` (522, −383, −456); marked at (474, −391, −437), inside secret area `SecretAreaMarker2` (454, −387, −417) | **Alohomora**, confirmed | **found** |

**Softlock: the pumpkin room needs Spongify to leave** (confirmed). It's a pit (floor ≈ −725) that the player drops
into. The only way out is `SpongifyPad3` (1168, −551, −780), tag `Fspongyplat`, which bounces up to
`SpongifyTarget5` (1339, −381, −191). The pad is `bEnableOnEvent`, so something enables it first *(what: to
confirm)*. Without Spongify, a player who drops in is stuck.

Open Castle must not let that happen. Options:
1. **Logic:** treat the pit and everything in it (card *Hebridean Black Dragon*, 5 pumpkins, secret area
   `SecretAreaMarker0`) as requiring Spongify. That isn't enough on its own, since the player can still drop in
   early.
2. **Block the drop** with a blocking volume until Spongify is received.
3. **Escape hatch:** a trigger in the pit that teleports the player out (or returns them to the last save) when
   they don't have Spongify.

2 or 3 is needed, with 1 for the logic. The same check applies to every drop-down area in the game; see the
softlock list in [OPEN_CASTLE.md](OPEN_CASTLE.md).

**Still to find in the first half (placed):** cakes `CauldronCake6` (−3288, −801, −215), `CauldronCake12` (4496, −1250, −213),
`CauldronCake3` (4550, −511, −213). All 16 placed pasties found and 1 of 4 placed cakes (the start pasty
`PumpkinPasty2` is inferred).

**Class names can mislead:** the grass tufts the rabbit eats here are `KnightSpawner` actors with a different
look (`DrawType` 1) and tags `Gsecretgrass1`–`9`, in a patch around (2670–2840, −1530 to −1770). In the catalogue a
"knight" can be an armour (Depulso) or grass (rabbit). The tag tells them apart: `Gsecretgrass*` = grass.

| Grass | Tag | Drop |
| --- | --- | --- |
| `KnightSpawner0` | `Gsecretgrass2` | 1 pasty (**found**) |
| `KnightSpawner19` | `Gsecretgrass4` | 1 pasty (**found**) |
| `KnightSpawner22` | `Gsecretgrass7` | 1 pasty (**found**) |
| `KnightSpawner21` | `Gsecretgrass6` | 1 cake (**found**) |
| `KnightSpawner24`/`18`/`20`/`23`/`16` | `Gsecretgrass1`/`3`/`5`/`8`/`9` | 2 beans each |

The real suits of armour in this half: `KnightSpawner2` (card *Welsh Green*, found), `KnightSpawner1` (8 beans),
`KnightSpawner9`/`11` (6 beans), and `KnightSpawner10` (tag `EVKnight`, empty).

**Point of no return: end of the dragon section** (confirmed). Near the column of dragon pasties there's a scripted
end to the dragon form, and the player can't go back afterwards. Two placed cakes at ground level there,
`CauldronCake12` (4496, −1250, −213) and `CauldronCake3` (4550, −511, −213), were seen but missed, with a chest
between them (`ChestSpawner15` (4680, −888, −200), 22 beans only). Missing them isn't a softlock, but getting them
means replaying the challenge.

For Open Castle: there's no logic problem (they need dragon form, and a replay gives another chance), but checks
behind a one-way script should be listed so the player is warned (or the script made skippable).

**Pumpkin default:** `Pumpkin3` sets no count and gave **2 pasties** (confirmed), so the `Pumpkin` class default is 2. That also applies to `Pumpkin5` and `Pumpkin6`, and to the catalogue's other pumpkins with no count.

The pumpkin room around (1343, −388, −725) is laid out as a cross: the card pumpkin `Pumpkin2` (Hebridean Black
Dragon) sits in the middle, beside secret area `SecretAreaMarker0`, with four 1-pasty pumpkins about 400 units out:
`Pumpkin9` (west, found), `Pumpkin3` (north, (1343, 21), found), `Pumpkin8` (south, (1343, −795), found) and `Pumpkin7`
(east, (1783, −381), found).

Pumpkins, like knights and chests, are `KWGenericSpawner`s: their pasties burst out and aren't individual checks under the current plan (see the chest-check option).

The start plays `Cut_6500SP2LapIntro_Start` (`PlayerStart2`), the Lapifors introduction.

**Open Castle:** in vanilla this challenge is reached from the classroom cutscene. The plan sends the classroom door
to the exam (FA2) and uses the side-room door (`ChangeLevelTrigger10`) for the challenge.

## 14. HP3_Ch2DraconiforsB: second half of the Draconifors challenge

| | |
| --- | --- |
| Level ID | `dlb` |
| State | `GState060` |
| Entered from | HP3_Ch2Draconifors, `ChangeLevelTrigger1`, a door just past the end of the dragon section (confirmed) |
| Checks | *(in progress)* |

| Check | Where | Requires | Status |
| --- | --- | --- | --- |
| Card *Hungarian Horntail* | `ChestSpawner13` (2657, 857, 346), an Alohomora chest; marked at (2656, 809, 360) | **Alohomora** (chest) + **Depulso** (to reach it), confirmed | **found** |
| 2 pasties (bundimun) | `Bundimun6` (3413, 673, 305); marked at (3445, 664, 330) | **Rictusempra** to stun, then jump on it, confirmed | **found** |
| Pasty | `PumpkinPasty28` (3046, 1321, 294); marked at (3033, 1332, 307) | **rabbit form**, confirmed | **found** |
| Pasty | `PumpkinPasty29` (3801, 1320, 294); marked at (3809, 1309, 307) | **rabbit form**, confirmed | **found** |
| Pasty | `PumpkinPasty24` (4081, 1601, 294); marked at (4083, 1594, 307) | **rabbit form**, confirmed | **found** |
| Pasty | `PumpkinPasty5` (3808, 2643, 294); marked at (3823, 2623, 307) | **rabbit form**, confirmed | **found** |
| Pasty | `PumpkinPasty25` (3048, 2649, 294); marked at (3026, 2645, 307) | **rabbit form**, confirmed | **found** |
| Pasty | `PumpkinPasty26` (2764, 2372, 294); marked at (2771, 2387, 307) | **rabbit form**, confirmed | **found** |
| Card *Norwegian Ridgeback Dragon* | `ChestSpawner12` (4875, 4646, 312), with 12 beans, inside secret area `SecretAreaMarker1` (4945, 4649, 351); marked at (4921, 4632, 330) | **Alohomora**, confirmed | **found** |
| Card *Romanian Longhorn Dragon* | `ChestSpawner19` (3296, 5513, 645), a chest by secret area `SecretAreaMarker6`; marked at (3294, 5494, 634) | **Spongify** + **Depulso**, confirmed | **found** |
| Cake | `CauldronCake9` (3820, 4983, 327); marked at (3817, 4988, 347). A Draconifors statue is 130 units away | **dragon form**, confirmed | **found** |
| Cake chest (2 cakes) | `ChestSpawner14` (4497, 4562, 312); marked at (4506, 4608, 330) | **Alohomora**, confirmed | **found** |
| Pasty spawner (chest, 4 pasties) | `ChestSpawner26` (3550, 5514, 638); marked at (3551, 5492, 634) | **Spongify** (to reach) + **Depulso** (to open), confirmed | **found** |
| Pasty | `PumpkinPasty30` (4977, 4087, 848), high up; marked at (4982, 4087, 868) | **dragon form**, confirmed | **found** |
| Pasty | `PumpkinPasty19` (4416, 1977, 294), inside secret area `SecretAreaMarker5` (4369, 1979, 351); marked at (4381, 1985, 307) | **rabbit form**, confirmed | **found** |

**Chests near `ChestSpawner26`:** `ChestSpawner23` (3424, 5669, 645) holds a **challenge shield** (`ChallengeStar` as
its goodie), and `ChestSpawner19` (3296, 5513, 645) holds the card *Romanian Longhorn Dragon*, by secret area
`SecretAreaMarker6`. So at least one shield in this challenge comes out of a chest.

**Grass patch** (rabbit; marked centre (4518, 4282, 306)): eight tufts, `Lsecretgrass1`–`8`, in a ring around
(4515, 4340, 343).

| Grass | Tag | Drop |
| --- | --- | --- |
| `KnightSpawner31` | `Lsecretgrass7` | 1 pasty |
| `KnightSpawner27` | `Lsecretgrass3` | 1 pasty |
| `KnightSpawner29` | `Lsecretgrass5` | 1 cake |
| `KnightSpawner17`/`25`/`26`/`28`/`30` | `Lsecretgrass8`/`1`/`2`/`4`/`6` | 2 beans each |

Requirement: **rabbit form**, confirmed. *(Which drops were collected: the mark is the patch, not each tuft.)*

**Knight hall** (not visited yet), far west around (−9250 to −7240, −1540 to 513), z −960 and −417: nine suits of
armour (tags `khall1`–`6`, `k1`, `k2`, `kside1`), each with **10 beans + 2 pasties**, so 18 pasties from knights.

**Bundimuns (pasty creatures).** Stun one with Rictusempra, then jump on it, and it drops pasties. The drops are
**guaranteed**: `BundimunController.DoGoodieSpawning()` spawns exactly `GoodieCount` × `GoodieToSpawn`, and only
the direction the pickups fly is random. (Class default: 3 beans; maps override it.) There's no save flag, so a
bundimun comes back when the map reloads. When not stunned, a bundimun burrows back underground (confirmed), so it can be missed
until it surfaces again.

All 12 in the game:

| Map | Bundimun | Position | Drop |
| --- | --- | --- | --- |
| Ch2B | `Bundimun6` | (3413, 673, 305) | 2 pasties (**found**) |
| Ch2B | `Bundimun13` | (3080, 1626, 304) | 1 pasty (**marked**) |
| Ch2B | `Bundimun11` | (3069, 2331, 304) | 1 pasty (**marked**) |
| Ch2B | `Bundimun4` | (3893, 1985, 304) | 1 pasty (**marked**) |
| Ch2B | `Bundimun0` | (5316, 4360, 308) | 1 pasty (**marked**) |
| Ch2B | `Bundimun5` | (4690, 4687, 301) | 1 pasty (**marked**) |
| Ch2B | `Bundimun3` | (4475, 4953, 296) | 1 pasty |
| Ch2B | `Bundimun14` | (4633, 5289, 308) | 2 pasties (**marked**) |
| Ch2B | `Bundimun2` | (5488, 5178, 308) | 1 pasty (**marked**) |
| FA2Hermione | `Bundimun4`/`6`/`8` | around (340, −1600, 1102) | pasties, count from the class default *(check)* |

**Open Castle:** bundimuns are safe as checks. The drop is fixed, and the check can fire on the defeat itself (the
controller's `DoRictoHit` → bounce → `DoGoodieSpawning`), not on picking the pasties up. Since they respawn, a
check, once sent, stays sent. They'd fall under `pasty_checks` (or their own option), and logic needs Rictusempra.

**Open Castle:** Ch2 and Ch2B are one region, like Ch1/Ch1B (confirmed). The whole challenge needs one entrance
(the side-room door `ChangeLevelTrigger10`) and one exit back to InsideHub.

Catalogue for this half: cards *Norwegian Ridgeback* (`ChestSpawner12` (4875, 4646, 312), by secret area
`SecretAreaMarker1`), *Romanian Longhorn* (`ChestSpawner19` (3296, 5513, 645), by `SecretAreaMarker6`),
*Hungarian Horntail* (`ChestSpawner13` (2657, 857, 346)); secret areas `SecretAreaMarker1`/`5`/`6`/`7`; 12 placed
pasties and 14 placed cakes; 4 challenge stars; a bean bonus game; the end trigger.

### End of the Draconifors challenge (confirmed)

The challenge was finished, and the player came out in the challenge chamber, which is the copy inside
HP3_Ch2DraconiforsB, as the state changed to **`GState065`** (17:21:29). Trying to leave the chamber hit a cutscene
trigger, `DracLoadQuidditch` (active outside `070/075/090/100`), which loads **HP3_Quidditch** and sets
**`GState070`** (17:21:32). Still cutscene, no control.

## 15. HP3_Quidditch: cutscene only (`GState070`)

| | |
| --- | --- |
| Level ID | `quid` |
| State | `GState070` |
| Entered from | Ch2B challenge chamber, `DracLoadQuidditch` (vanilla also: from Ch1 `0700Ch1Chal_QuidDem_070` in `GState065`, Ch3B `GlaciustoQuidditch` in `GState065`) |
| Leaves to | `7010QuidDem_ALL` → GroundsDada, or `7010QuidDem_SpinFallShot` → Adv2ExpPatronum (both set `GState070`) |
| Checks | none in the catalogue |

All cutscene (confirmed): the Quidditch match and the Dementor attack. The cutscene keeps running into
**HP3_GroundsDada** (via `7010QuidDem_ALL`, confirmed), still without player control. The GroundsDada stretch is cutscene only too; it continues into **HP3_Adv2ExpPatronum**
(`7020PatronusIntro_Start`, confirmed). **Open Castle:** drop it (no checks, no
player control), as planned. *(Where the vanilla chain goes next: following.)*

## 16. HP3_Adv2ExpPatronum: Expecto Patronum adventure (`GState070`)

| | |
| --- | --- |
| Level ID | `adv2` |
| State | `GState070` (MapTracker 17:24:07) |
| Entered from | the Quidditch → GroundsDada cutscene chain, `7020PatronusIntro_Start` (confirmed; no control until here) |
| Leaves to | HP3_Adv3Library (`7090PatronusExit`) |
| Checks (catalogue) | card *Honoria Nutcombe*, spawned when the second wave is destroyed (`SecondWaveDestroyedWC`): **found** (confirmed); no placed snacks or secrets |
| Requires | **Expecto Patronum** (the level is built around it), plus Depulso for the skull shelves |

**Open Castle:** this level has a check and player control, so it stays, but vanilla only reaches it through the
Quidditch → GroundsDada cutscene chain. It needs:
- **an entrance**: the planned DADA → Expecto door (`DADA_DoorToExpecto_Mvr` / `DADA_PortToExpecto_Mvr`,
  [OPEN_CASTLE.md](OPEN_CASTLE.md) §3), or a spare door;
- **an exit back** to InsideHub instead of `7090PatronusExit` → Adv3Library;
- **Expecto Patronum as an item.** The spell is `HGame.PatronumSpell` (also used against `DementorBoss`), so it can be
  gated through the same `HasSpell` hook as the other spells.

Spawners (all Depulso targets):

| Spawner | Position | Drop | Status |
| --- | --- | --- | --- |
| `SkullShelfSpwn0` (skull shelf) | (3058, 1671, −362) | 3 pasties | **found**: Depulso, confirmed; marked at (3028, 1659, −337) |
| `SkullShelfSpwn1` | (3040, 2352, −320) | 3 pasties | **found**: Depulso, confirmed; marked at (3031, 2323, −263) |
| `SkullShelfSpwn2` | (2112, 1545, −362) | 3 cakes | **found**: Depulso, confirmed; marked at (2128, 1571, −336) |
| `TapestrySpwn6`, `ArmoireSpwn0`, `KnightSpawner0`, `ChestSpawner1` | around (2200–3130, 1100–2090) | beans | |

## 17. HP3_Adv3Library: library adventure (`GState070`)

| | |
| --- | --- |
| Level ID | `adv3` |
| State | `GState070` |
| Entered from | end of Adv2ExpPatronum: its ending cutscene (`7090PatronusExit`) carries straight into this map (confirmed) |
| Leaves to | InsideHub (`7170Library_Exit`) |
| Checks (catalogue) | cards *Falco Aesalon* (`ChestSpawner1` (−3863, −671, 281), with 5 pasties) and *Double-Ended Newt* (placed, `WC_DoubleEndedNewt0` (−8216, −7269, 1357)); 4 secret areas (`SecretAreaMarker2`/`4`/`5`/`6`, around x −8200 to −12300); a cake chest (`ChestSpawner2`, 4 cakes); one Draconifors statue |
| Requires | **Depulso** (confirmed); Draconifors (statue), Carpe Retractum (per the plan) *(to confirm)* |

| Check | Where | Requires | Status |
| --- | --- | --- | --- |
| Card *Falco Aesalon* (+ 5 pasties) | `ChestSpawner1` (−3863, −671, 281); marked at (−3815, −680, 298) | **Alohomora**, confirmed | **found** |
| Cake chest (4 cakes) | `ChestSpawner2` (−4278, −3137, 281); marked at (−4230, −3144, 298) | **Alohomora**, confirmed | **found** |

**Open Castle:** needs an entrance (the planned Library door `Library_Door_Mvr` / `Library_Port_Mvr`) and keeps its
exit to InsideHub.

## 18. HP3_InsideHub, `GState070`: back from the library

The library level ends with `7170Library_Exit`, which returns the player to InsideHub, second floor (MapTracker
17:36:58, `GState070`).

| Check / place | Where | Requires | Status |
| --- | --- | --- | --- |
| Pasty chest (10 beans + 3 pasties) after the flying-book fight | `ChestSpawner14` (−51, −1682, 632), tag `2ndFlr_BookBtlBkCs_Chest_02`, opened by `AlohomoraTrigger8`; marked at (−48, −1634, 650) | **Depulso** (flying books: `FlyingBook.vulnerableToClass=DepulsoSpell`; they're a `Poxie` subclass, but doxies themselves need Rictusempra) + **Alohomora** | **found** |
| Pasty chest (10 beans + 3 pasties) | `ChestSpawner4` (−1369, −1581, 1120); marked at (−1321, −1591, 1138) | **Lumos** (to enter the area) + **Spongify** (to reach the chest) + **Alohomora**, confirmed | **found** |
| Pasty skull (5 beans + 2 pasties) | `SkullFloatSpwn7` (−1773, −927, 894), a floating skull; marked at (−1777, −987, 842) | **Alohomora** (to reach it) + **Depulso**, confirmed | **found** |
| Pasty chest (6 beans + 2 pasties) | `ChestSpawner17` (−3653, −35, 1541), ~300 units from secret area `SecretAreaMarker3` (−3317, −35, 1591); marked at (−3610, −13, 1556) | **Alohomora**, confirmed | **found** |
| **Library exit / entrance** | Portcullis `KWMover53` (tag `Library_Port_Mvr`, (−704, −1368, 732)) in front of the double doors `KWMover52`/`KWMover13` (tag `Library_Door_Mvr`), 2nd floor; `Door3` and `PopupTrigger20` there too; marked at (−710, −1349, 778) | — | confirmed: this is where the library level comes out |

| **DADA → Expecto door** | Portcullis `KWMover27` (tag `DADA_PortToExpecto_Mvr`, (856, −1844, 1152)) in front of double doors `KWMover66`/`KWMover67` (tag `DADA_DoorToExpecto_Mvr`), in the DADA classroom, with `PopupTrigger11`; marked at (838, −1844, 1194) | — | confirmed: closed portcullis + door pair, reusable |

**Open Castle:** confirmed, the 2nd-floor Library door (portcullis + doors) is where Adv3Library exits, so it works as
the library's entrance. The "Library" area unlock in [OPEN_CASTLE.md](OPEN_CASTLE.md) §3 uses it: raise
`Library_Port_Mvr` / open `Library_Door_Mvr` once the item is received, plus a `ChangeLevelTrigger` to HP3_Adv3Library.

**Open Castle:** the DADA classroom's portcullis + doors are the "Expecto Patronum" area unlock from
[OPEN_CASTLE.md](OPEN_CASTLE.md) §3, now located: raise `DADA_PortToExpecto_Mvr` / open `DADA_DoorToExpecto_Mvr` when
the item is received, with a new `ChangeLevelTrigger` behind them to HP3_Adv2ExpPatronum. Like the Library door, it's
built the same way (portcullis + `TudorDoor1U`/`2U` pair, in the `Persistant_Movers` group).

**Spare locked door, 4th floor** (marked at (−187, 1274, 1498), 17:46:13): the same decorative set as the
entrance-hall pair, a static `TudorDoorL_SM` door with a `DoorLockBar_SM` bar in a `DoorArchTudorQuad_SM` arch, plus
`PopupTrigger15`. No mover, nothing behind it. Free for an Open Castle entrance.

**Spare locked door, 7th floor** (marked at (−84, −1081, 2154), 17:49:35): the same static set, `TudorDoorL_SM`
door + `DoorLockBar_SM` bar in a `DoorArchTudorQuad_SM` arch (`StaticMeshActor348`/`292`/`1144`). No mover. Close
to `CutSceneTrigger4` (`1080PortShort`, `GState010`), the Peeves shortcut-password scene. Free for an Open Castle
entrance.

**Spare locked door, 7th floor (east)** (marked at (1427, −960, 2154), 17:50:23): the same static door + lock-bar set
(`StaticMeshActor166`/`228`/`488`). No mover. Free for an Open Castle entrance.

**Lupin's office door (DADA classroom balcony):** the ninth lock-bar door (`StaticMeshActor227` bar at
(669, −2116, 1341), door `StaticMeshActor195` at (722, −2085, 1281)) is the office door at the top of the classroom,
behind the balcony. It's **unreachable**: the player can't get up there (confirmed; marked from the floor below at
(700, −2026, 1194)). Not usable as an entrance unless a way up is added; an Open Castle option adds a Spongify pad for this
([OPEN_CASTLE.md](OPEN_CASTLE.md), spare-door checklist).

## 19. HP3_Ch3Glacius: Glacius challenge (Harry), first visit (`GState070`)

| | |
| --- | --- |
| Level ID | `gl` (second half `glb`) |
| State | `GState070` (MapTracker 18:00:27) |
| Entered from | InsideHub *(which entrance: to confirm; in `GState070` the Charms classroom plays `7250CharmsIntro`)* |
| Requires | **Glacius for the whole map**: it's needed immediately to proceed (confirmed), so every check here is behind Glacius |

| Check | Where | Requires | Status |
| --- | --- | --- | --- |
| Card *Eargit the Ugly* (+ 15 beans) | `ChestSpawner12` (9464, −4524, 14248), by secret area `SecretAreaMarker3` (9189, −4540, 14317); marked at (9415, −4526, 14266) | Glacius + **Alohomora**, confirmed | **found** |
| Card *Ug the Unreliable* (+ 15 beans, 2 cakes) | `ChestSpawner35` (−4594, −2707, 4691), by secret area `SecretAreaMarker2` (−4895, −2715, 4745); marked at (−4636, −2684, 4706). A second chest, `ChestSpawner13`, sits almost exactly on the same spot (−4589, −2707, 4688) with no contents set *(a leftover duplicate? check whether two chests are visible)* | Glacius + **Alohomora**, confirmed | **found** |
| 3 pasties (+ 15 beans) | `KnightSpawner39` (−8631, −2496, 4580), suit of armour; marked at (−8659, −2538, 4538) | Glacius + **Depulso**, confirmed | **found** |
| 3 pasties (+ 18 beans) | `KnightSpawner38` (−8621, −3036, 4580), suit of armour; marked at (−8628, −2987, 4538) | Glacius + **Depulso**, confirmed | **found** |
| Card *Alguff the Awful* (+ 10 beans, 2 chocolate frogs) | `ChestSpawner31` (3524, −3817, 10968), by secret area `SecretAreaMarker1` (3522, −3993, 11026); marked at (3533, −3862, 10986) | Glacius + **Alohomora**, confirmed | **found** |

Catalogue, both halves:
- **Cards (5):** Ch3 has *Eargit the Ugly* (found), *Ug the Unreliable* (`ChestSpawner35` (−4594, −2707, 4691), by
  `SecretAreaMarker2`) and *Alguff the Awful* (`ChestSpawner31` (3524, −3817, 10968), by `SecretAreaMarker1`). Ch3B has
  *Urg the Unclean* (`ChestSpawner33` (−9258, −1012, 1596)) and *Gringott*, placed (`WC_Gringott0`
  (−10632, −2334, −3977)).
- **Secret areas (5):** Ch3 `1`/`2`/`3` (each with a card chest); Ch3B `0`/`4`.
- **Snacks (placed):** Ch3 has 39 pasties and 10 cakes; Ch3B has 32 and 7. The most of any challenge.
- 22 challenge stars in the data (Ch3 12, Ch3B 10).

**Open Castle:** in logic, Glacius gates the entire challenge, so the challenge entrance effectively requires
Glacius (with `split_challenge_spells` off, the Glacius item opens both). Ch3 and Ch3B will be one region, like the
other two challenges.

## 20. HP3_Ch3GlaciusB: second half of the Glacius challenge

Reached from Ch3 through `ChangeLevelTrigger0` (MapTracker 18:06:58). One region with Ch3, like the
other challenges.

| Check | Where | Requires | Status |
| --- | --- | --- | --- |
| 8 pasties + 3 cakes (+ 15 beans) | `ChestSpawner32` (−8464, 2120, 1724); marked at (−8510, 2112, 1742) | Glacius + **Alohomora**, confirmed | **found** |
| 3 pasties (+ 15 beans) | `ChestSpawner30` (−9260, 3106, 1596); marked at (−9294, 3072, 1614) | Glacius + **Alohomora**, confirmed | **found** |
| Card *Urg the Unclean* (+ 15 beans) | `ChestSpawner33` (−9258, −1012, 1596); marked at (−9295, −982, 1614) | Glacius + **Alohomora**, confirmed | **found** |
| 10 pasties + 2 cakes (+ 5 beans) | `ChestSpawner10` (−11503, 1889, 1463), ~290 units from secret area `SecretAreaMarker4` (−11459, 1606, 1648); marked at (−11488, 1843, 1482) | Glacius + **Alohomora**, confirmed | **found** |
| 5 pasties (+ 20 beans) | `ChestSpawner11` (−11540, 920, 1604); marked at (−11501, 947, 1622) | Glacius + **Alohomora**, confirmed | **found** |
| 4 pasties + 1 cake (+ 15 beans) | `BannerSpwn2` (−11762, 1253, 1819), a banner; marked from below at (−11768, 1224, 1622), 30 units across | Glacius + **Depulso** | **found** |
| 5 pasties + 2 cakes | `BannerSpwn0` (−9173, −407, 1817), a banner (flag) ~210 units up; marked from below at (−9185, −426, 1606) | Glacius + **Depulso**, confirmed | **found** |

Banners (`BannerSpwn`) are Depulso targets hung high on walls; marks from the floor sit ~200 units below them.

Nearby: secret area `SecretAreaMarker0` (−9060, 2105, 1711), about 550 units west; `ChestSpawner30` (−9260, 3106, 1596),
3 pasties + 15 beans.

### End of the Glacius challenge (confirmed)

Same pattern as the other two: after the challenge the player is in the duplicate challenge chamber inside
HP3_Ch3GlaciusB, and the state becomes **`GState075`** (18:17:26). Leaving it plays a cutscene
(`GlaciustoBuckBeakExc`, active in `070`/`075`) that loads **HP3_BuckyExecuted** (18:17:34).

## 21. HP3_BuckyExecuted: Buckbeak's execution (`GState075`)

| | |
| --- | --- |
| Level ID | `bex` |
| State | `GState075` |
| Entered from | the Ch3B challenge chamber, `GlaciustoBuckBeakExc` (confirmed). In vanilla also from Ch1/Ch1B (`0700Ch1Chal_BuckyEx`) and Ch2B (`DracLoadBuckyExec`) when the state is `075` |
| Leaves to | HP3_Adv5Shack (`8180BuckExecuted` / `8180BuckExecuted_02Temp`), with a cutscene that sets the state written as **`GSate085`** (sic: a typo for `GState085`). **Confirmed: the typo has no effect**: the player arrives in Adv5Shack still in `GState075` (MapTracker 18:19:20). |
| Checks (catalogue) | **none**: six pumpkins and a scarecrow, all beans only |

It's cutscene only: the execution cutscene runs straight on into HP3_Adv5Shack (confirmed).

**Open Castle:** no checks, so this map can be dropped (beans only). SCENES.md and OPEN_CASTLE.md listed it as having
chests with checks; they don't (beans).

## 22. HP3_Adv5Shack: the Shrieking Shack adventure (`GState075`)

| | |
| --- | --- |
| Level ID | `ADV5Shack` |
| State | **`GState075`** on arrival (the `GSate085` typo is ignored; the map's own default is `GState085`) |
| Entered from | the BuckyExecuted cutscene (`8180BuckExecuted`), confirmed |
| Leaves to | HP3_WhompingWillow (`ChangeLevelTrigger0` or `8700ADV5AtShack`), setting `GState085` |
| Checks (catalogue) | **1 card**: *Sacharissa Tugwood*, an event reward the first catalogue missed (now fixed); 5 secret areas (`SecretAreaMarker0`–`4`); 3 placed pasties and 1 placed cake; 12 snack spawners (chests, skull shelves, cauldrons, 2 "companion" chests) with about 25 pasties and 8 cakes; one Draconifors statue |
| Requires | **Draconifors** (dragon form), **Alohomora**, **Depulso**, **Spongify** (all confirmed); Glacius (per the plan) *(to confirm)* |

| Check | Where | Requires | Status |
| --- | --- | --- | --- |
| 2 pasties | `CauldronSpwn2` (−5522, −1420, −980), a cauldron; marked at (−5537, −1372, −973) | **Depulso**, confirmed | **found** |
| 2 pasties | `CauldronSpwn1` (1954, −4330, −856), a cauldron; marked at (1964, −4281, −853) | **Depulso**, confirmed | **found** |
| 4 pasties (+ 2 chocolate frogs) | `ChestCompanionSpawner0` (3240, −3773, −999), a "companion" chest with **two Alohomora padlocks** on it (`AlohomoraPadLock0`/`1`); marked at (3232, −3819, −981) | **Alohomora**, cast by **Harry and Hermione together** (one padlock each), confirmed | **found** |
| 5 pasties (+ 1 bean) | `ChestCompanionSpawner4` (4097, −2827, −999), companion chest with padlocks `AlohomoraPadLock16`/`17`; marked at (4133, −2858, −981) | **Alohomora**, Harry + Hermione together, confirmed | **found** |
| Card *Sacharissa Tugwood* | created by `SpawnThingy10` (5426, −3913, −705, tag `RmG_Win_SpawnThing_Card`, Room G) when the skeleton fight is won (dispatcher `RmG_SkeleKilled_Spwn`, which also fires 8 bean fountains); marked at (5420, −3913, −715) | **Rictusempra** (the fight), confirmed | **found** |
| 2 pasties (+ 1 chocolate frog) | `ChestSpawner7` (−4582, −2857, −899); marked at (−4590, −2898, −917) | **Alohomora**, confirmed | **found** |
| Pasty (placed) | `PumpkinPasty3` (−1761, −3577, −880); marked at (−1760, −3584, −853). A padlocked door (`AlohomoraPadLock4`/`5`) is ~315 units west | *(none stated)* | **found** |
| Cake (placed) | `CauldronCake4` (−707, −5223, −48), high up; marked at (−736, −5227, −116) | **dragon form** (Draconifors statue `DraconisforsStatue1` at (−1319, −3403, −811)), confirmed | **found** |
| 2 pasties (+ beans) | `SkullShelfSpwn2` (−2096, −2492, −1488), a skull shelf on the lower level; marked at (−2074, −2469, −1461) | **Depulso**, confirmed | **found** |
| 3 pasties (+ beans) | `ChestSpawner2` (964, −2395, −3975), at the bottom of the map (height ≈ −3975); marked at (917, −2403, −3957) | **Spongify** + **Alohomora**, confirmed | **found** |
| 3 cakes | `ChestSpawner5` (785, −3349, −2016), ~350 units from secret area `SecretAreaMarker3` (802, −2960, −1904); marked at (786, −3300, −1997) | **Spongify** + **Alohomora**, confirmed | **found** |
| Pasty (placed) | `PumpkinPasty5` (824, −2044, −1048); marked at (808, −2038, −1013) | *(none stated)* | **found** (all 3 placed pasties done) |
| Pasty (placed) | `PumpkinPasty0` (467, −2404, −2895); marked at (447, −2410, −2893) | *(none stated)* | **found** |
| 2 cakes | `ChestSpawner12` (−3235, −3979, −770), in secret area `SecretAreaMarker1` (−3310, −3971, −760); marked at (−3203, −3942, −757) | **Alohomora**, confirmed | **found** |

**Companion chests** (`ChestCompanionSpawner*`): two padlocks that two characters must open together. Here Harry
and Hermione are both in the party, since this level puts both of them in it (assumed; Ron is the injured one in
the story at this point). Logic: no extra requirement in this map, as long as the level always loads with that
party. *(Check that it does when entered from an Open Castle entrance rather than the story chain.)*

**Open Castle:** keeps its checks (secrets and snacks). It needs an entrance: a spare door, or the Whomping Willow
trigger on the grounds planned in [OPEN_CASTLE.md](OPEN_CASTLE.md) §6.

## 23. HP3_WhompingWillow: cutscene only (`GState085`)

The end of the tunnel level runs into cutscenes; the player arrives in HP3_WhompingWillow with the state now
**`GState085`** (MapTracker 18:35:55). No control (confirmed: "in cutscenes now"), and nothing in the catalogue.
**Open Castle:** drop.

## 24. HP3_DementorBattle, first visit (`GState085`)

| | |
| --- | --- |
| Level ID | `dembtl` |
| State | `GState085` (MapTracker 18:37:50, via `Entry`) |
| Entered from | WhompingWillow cutscenes |
| Leaves to | in `GState085`: Infirmary (`8730DemEncAExitPARTONE`, sets `GState090`); in `GState090`: DarkTower (`9120DemEncBExit`) |
| Checks | **none** (catalogue is empty), as expected |

**Open Castle:** no checks, so drop it (or keep it as a boss/goal candidate needing Expecto Patronum).

## 25. HP3_Infirmary: cutscene only (`GState090`)

After the Dementor battle the state becomes **`GState090`** and the player passes through HP3_Infirmary
(MapTracker 18:39:20), all cutscene, nothing in the catalogue. **Open Castle:** drop.

## 26. HP3_Adv6Paddock: freeing Buckbeak (`GState090`), **playable**

| | |
| --- | --- |
| Level ID | `adv6` |
| State | `GState090` (MapTracker 18:40:33; also the map's default) |
| Entered from | the Infirmary cutscene (`8730DemEncAExit_Infirmary`), confirmed |
| Leaves to | HP3_DementorBattle again (`9080Adv6Flyaway`), the second Dementor fight |
| Checks (catalogue) | card *Professor Armando Dippet* (`CauldronSpwn0` (−1290, 459, 452), a cauldron); 6 placed pasties and 3 placed cakes; 5 snack containers (crate, 2 chests, 2 cauldrons) with about 21 pasties and 3 cakes; no secret areas |
| Requires | **Depulso** and **Spongify** for the whole level: both are needed right at the start to proceed (confirmed). Lapifors and Draconifors statues are also present (`LapiforsStatue0`, `DraconisforsStatue0`) *(to confirm)* |

| Check | Where | Requires | Status |
| --- | --- | --- | --- |
| 4 pasties (a ring) | `PumpkinPasty7` (66, 864, 643), `PumpkinPasty6` (128, 800, 643), `PumpkinPasty0` (129, 928, 643), `PumpkinPasty4` (190, 865, 643), around (128, 864, 643); one mark in the middle at (75, 860, 609) | **dragon form**, confirmed | **found** |

Other placed snacks: pasties `PumpkinPasty5` (−583, −341, 44) and `PumpkinPasty18` (−683, −337, 53); cakes
`CauldronCake0`/`2`/`7` in a row at (1265–1417, 815, 936).

**Open Castle:** a playable level with a card, so it stays. It needs an entrance (Hagrid's hut door is the natural
spot, [OPEN_CASTLE.md](OPEN_CASTLE.md) spare doors) and an exit back to GroundsHub instead of `9080Adv6Flyaway`.
