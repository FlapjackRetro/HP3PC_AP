# Open Castle plan

Working design for the randomizer's main mode. Instead of following the story, Hogwarts starts in one fixed story
state, every area with checks gets an entrance, and progression comes from Archipelago items. Background is in
[SCENES.md](SCENES.md) (maps, game states, entrances) and [FINDINGS.md](FINDINGS.md) (save keys, items).

Status: plan. Everything marked *(verify)* needs an in-game check, which is what the
[MapTracker](../mod/MapTracker/README.md) playthrough is for.

## 1. Base state

Open Castle locks the game to a single story state. **`GState090` or `GState100`** is the natural choice:

- all three side-room challenge doors are active, and all three challenges send you back to InsideHub, in both;
- the Monster Book intro trigger is tagged `GState070/090/100`, so it can start in either;
- `100` is post-exams, and its GroundsHub level start opens `FinalDayDoor`. That could be useful as the goal gate,
  or a problem if it opens too early. *(verify)*

Which actors (students, popups, Peeves, doors) are present or missing in each candidate state is what the playthrough
should settle.

## 2. Checks

Every check below already has a stable save key or a single function the mod can hook.

| Category | YAML | Count | Detection |
| --- | --- | --- | --- |
| Wizard cards | always | 80 | `WizardCardManager.MarkWizardCardAsOwned(class)` catches every source: placed, chest, quest reward, vendor, Fred & George. The check is named after the card class. |
| Secret areas | always | 41 | `SecretAreaMarker` → save key `<LevelID><ActorName>` |
| Pumpkin Pasties (placed) | `pasty_checks` | 180 | `KWPickup` → `<LevelID><ActorName>_PU` |
| Cauldron Cakes (placed) | `cake_checks` | 71 | same |
| Marauder's Map | `marauders_map_check` | 1 | Vanilla (confirmed in play): the `1051GetMaraudersMap` cutscene in the shortcuts room on first arrival at Hogwarts (`GState010`); Harry picks up a `MaraudersPickup` (InsideHub ~(150, −870, 144)). `1005TaskedShop` also has a `GiveMap`. Neither plays in Open Castle, so the pickup needs a new trigger or a placed map. |
| Beans | no | — | Currency only |

Notes:
- Pasties and cakes that fly out of chests aren't tracked one by one, so only placed ones count. Chest openings
  could be a later option (the 486 spawners have `bLivesPersistent`/`_lives`).
- Card sources in the catalogue: 19 placed, 23 in chests, 17 quest/event rewards (see §5). The rest are sold by
  vendors (20), Fred & George (6) and adventure-level vendors (4), plus 1 special last card. Several cards are placed
  in more than one map (Beatrix Bloxam ×5, Double-Ended Newt ×3), which is another reason to key checks by card
  class, not by actor.
- Other check candidates for later: challenge stars (63) and challenge/exam wins (7, `_bAlreadyWon`), side-quest
  stages (15, §5), portrait passwords (5).

## 3. Items

### Spells

| Spell | What it gates | Hook |
| --- | --- | --- |
| Alohomora | 228 chests and padlocks, ~80 spell triggers | `WeaponWand.HasSpell()` |
| Depulso | 239 knights, armoires, tapestries and similar; ~100 triggers | `WeaponWand.HasSpell()` |
| Spongify | 97 Spongify pads (traversal, many secrets) | `WeaponWand.HasSpell()` |
| Lumos | Wand light: reveals or hides actors with `LumosType` (hidden platforms and walls) | Block `WeaponWand.LumosTurnOn()` / the Lumos toggle. Lumos isn't a projectile spell, so `HasSpell` won't cover it. |
| Rictusempra | Peeves fights (`RictusempraSpell_Combat`), ~14 triggers | `HasSpell()`, or make it a starting spell |

In the original game `HasSpell()` is true for everything except per-character exclusions
(Harry: no Draconifors/Lapifors/Carpe Retractum; Ron: no Draconifors/Lapifors; Hermione: no Carpe Retractum),
so spells are only gated by level design. Making them items means replacing that one function.

### Story spells: `split_challenge_spells`

Carpe Retractum (Ron), Draconifors and Lapifors (Hermione), Glacius (Harry).

- **Off:** one item per spell. Receiving it unlocks the spell and its challenge door.
- **On:** two separate items, "<Spell>" and "<Challenge> Access". Harder progression.

These spells are used beyond their own challenge, so logic must require them there too:

| Spell | Also needed in |
| --- | --- |
| Draconifors (fire) | Adv3Library, Adv5Shack, Adv6Paddock, FA2 |
| Draconifors / Lapifors statues | Adv3Library, Adv5Shack, Adv6Paddock, FA2 |
| Glacius | Adv5Shack, FA3 |
| Carpe Retractum | Adv3Library, FA1 |

### Area unlocks: `area_unlocks`

Depends on Open Castle working without heavy changes to Hogwarts. Candidate items, using doors that already exist
(InsideHub door + portcullis pairs that only cutscenes open):

| Item | Door(s) | Leads to |
| --- | --- | --- |
| Grounds Access | `GroundsBlocker` ×3 portcullises | GroundsHub |
| Dungeon | `Dungeon_Door_Mvr` | DungeonHUB |
| Library | `Library_Door_Mvr` / `Library_Port_Mvr` | Adv3Library *(verify)* |
| Expecto Patronum | `DADA_DoorToExpecto_Mvr` / `DADA_PortToExpecto_Mvr` | Adv2ExpPatronum *(verify)* |
| Monster Book Room | `MBOMDoor` ×3 | Monster Book side quest |
| Carpe Retractum / Draconifors / Glacius Challenge | `ChalChamb_CR/Drac/Gla_Door` + portcullises (side room) | Ch1 / Ch2 / Ch3 |
| Ron's / Hermione's / Harry's Exam | `DADA_DoorToCarpe`, `Transfig_Door`/`ToDrac_Door`, `2nd_Flr_CharmsDoor`/`Glacius_Door` | FA1 / FA2 / FA3 |
| Portrait Passwords B–F | built-in password system, `pp_<n>` | PortraitPassword_B–F |
| Grounds adventures | new trigger volumes at landmarks (Whomping Willow, Hagrid's hut, viaduct) | Adv5Shack, Adv6Paddock, BuckyExecuted *(verify)* |
| Final Day | `FinalDayDoor` (GroundsHub) | HP3_Final, as the goal gate |
| (spare) | Entrance-hall barred doors at (733, 392, 74) and (731, 1615, 74): static door + lock-bar meshes with a `PopupTrigger`, no mover | any story map; see [PLAYTHROUGH.md](PLAYTHROUGH.md) §10 |
| (spare) | Hagrid's hut door, GroundsHub (813, −1121, 696): no trigger nearby | a grounds story map (§9) |

That's about 20 items, enough to cover every story-only map with checks. The castle side has plenty of doors. The
grounds have no door with a loading zone (GroundsHub's only exits are COMC and the castle), but Hagrid's hut door
is a natural place for a new trigger.

### Shortcut Passwords: `shortcut_passwords`

In vanilla, beating Peeves outside the twins' shop (`GState010`) gives the shortcut passwords. They open the castle's
shortcut passages (the shortcut chamber network). Optional progression item:

- **`item`:** "Shortcut Passwords" is a progression item. Until it's received, the shortcuts stay locked and logic
  routes around them.
- **`start_unlocked`:** unlocked from the start.

Mechanism: `HGame.PortraitPasswordManager` keeps 22 password entries (`PortraitPasswords[i].Owned`, saved as
`pp_<i>`). Entries 0–7 are the portrait passwords (vendor-bought, for PortraitPassword_B–F). Entries 8–21 are the
14 shortcut passwords. Vanilla grants them all at once with the scripted action `HPAction_GivePortShortcuts`, which
calls `EnablePortraitShortcuts()` and sets entries 8+ to owned. The mod grants the item the same way (or writes
`pp_8`–`pp_21`), and suppresses the vanilla grant from the Peeves fight. A split option (one item per shortcut or
per area) is possible later, since each entry is separate.

### Filler

Beans, cakes and pasties as currency; chocolate frogs; possibly traps later.

## 4. Hogwarts entrance layout

```
InsideHub ground floor, side room off the shortcuts room
   ├─ door → Ch1 Carpe Retractum   (ChangeLevelTrigger9)
   ├─ door → Ch2 Draconifors       (ChangeLevelTrigger10)
   └─ door → Ch3 Glacius           (ChangeLevelTrigger11)

Classrooms (decoupled from the story chain)
   DADA            → FA1Ron        (Carpe Retractum exam; 15 Retractum targets)
   Transfiguration → FA2Hermione   (Draconifors/Lapifors exam; statues and grass walls)
   Charms          → FA3Harry      (Glacius exam; Salamander set-up)
```

Changes needed:
1. **Classroom doors → exams.** Spawn or retarget a `ChangeLevelTrigger` at each classroom door and suppress the
   story cutscenes there (`0200DADAIntro`, `6180TransIntro`, `7250CharmsIntro`).
2. **Break the exam chain.** Send these back to InsideHub:
   - FA1 exit cutscene `RonExamExitDONE` (vanilla → FA2)
   - FA2 `ChangeLevelTrigger1` (vanilla → FA3)
   - FA3 `ChangeLevelTrigger0` (vanilla → GroundsHub, sets `GState100`)

   Lose/replay paths reload the same exam and can stay. Each exam keeps its own win flag and 5 stars.
3. **Story-only adventures** get an entrance (door or new trigger) plus a return trigger, since their vanilla exits
   lead to the next story map.
4. **Arrival state.** Story maps were built for one state (Adv6Paddock `090`, Adv5Shack/Willow `085`). The entrance
   probably needs to set that state (`NewGameState`) and restore the Open Castle state on the way out *(verify)*.
5. **Spawn point.** Arrival `PlayerStart`s are filtered by state and by `SmartStart.PreviousLevelName`. A new
   hub → Adv5Shack link has no matching SmartStart, so check which start is used, or spawn one *(verify)*.
6. **Implementation.** Spawn and retarget triggers at runtime from the mod when a hub loads, so EA's `.unr` files
   are never edited or redistributed.

## 5. Story-granted items and side quests

These run on map wiring plus a saved counter, not dedicated code:

| Quest | Map | Stage counter | Stage rewards (check names) |
| --- | --- | --- | --- |
| Pixie-infested well ("doxy well"; the game's class is `Poxie`) | GroundsHub | `RoundRobin1` "PixieBattleRoundRobin" (battles) / `RoundRobin0` (exit cutscenes) → `ghubRoundRobin1_iOEI` / `ghubRoundRobin0_iOEI` | Bran the Bloodthirsty, Cyclops, Goliath, Morholt, Hengist of Upper Barnton |
| Monster Book of Monsters | InsideHub | `RoundRobin2` "MBRobin" → `ihubRoundRobin2_iOEI` | Baba Yaga, Malodora Grymm, Leticia Somnolens, Old Mother Hubbard, Cordelia Misericordia |
| Hippogriff riding | COMC | `HippoFlyGame`: `hfg_n` (tracks completed), `hfg_h<i>` / `hfg_l<i>` scores; a track passes at `PassScore` 1500 / 2800 / 4000 / 4900 / 5600 | Joscelind Wadcock (tutorial), Gwenog Jones, Cyprian Youdle, Roderick Plumpton, Bowman Wright |
| Marauder's Map | InsideHub | `MaraudersMap` inventory (the item itself) | the map |

Other event-spawned cards: Fulbert the Fearful (Adv1Express, `SpawnWC1`) and Honoria Nutcombe (Adv2, `SecondWaveDestroyedWC`).

The well and Monster Book cards are placed out of reach ("Green Room DON'T HIDE") and teleported to the player by
`HPSpawnThingy` actors. The hippogriff cards are created by class. The catalogue marks all 17 with `awarded_by`.

## 6. Story-only maps in Open Castle

| Map | Has checks | Plan |
| --- | --- | --- |
| Adv1Express (Hogwarts Express, prologue) | 1: Fulbert the Fearful card (confirmed, [PLAYTHROUGH.md](PLAYTHROUGH.md)) | drop it and put the card in the pool, or keep it as the opening (no hub door exists) |
| GroundsDada | yes | entrance needed |
| Adv2ExpPatronum | yes | DADA → Expecto door *(verify)* |
| Adv3Library | yes | Library door *(verify)* |
| BuckyExecuted | yes (chests) | grounds trigger, or grant its items |
| Adv5Shack | yes | Whomping Willow trigger on the grounds |
| Adv6Paddock (freeing Buckbeak) | yes | grounds trigger |
| FA1 / FA2 / FA3 | yes | classroom doors (§4) |
| Quidditch, WhompingWillow, DementorBattle, Infirmary, DarkTower | no | skip; candidates for boss checks or the goal |
| Final | 1 chest | goal |

Rule from the plan: **maps that are cutscene-only with no player interaction are removed.** Any items they would
grant (in the categories in §2) go into the Open Castle item pool instead.

## 7. Draft YAML options

```yaml
Harry Potter 3 PC:
  goal: final_day              # open FinalDayDoor / reach HP3_Final (TBD)
  pasty_checks: true
  cake_checks: true
  marauders_map_check: true
  spell_items: true            # Alohomora, Depulso, Lumos, Spongify (+ Rictusempra?)
  starting_spells: []          # spells precollected
  split_challenge_spells: false
  area_unlocks: true
  shortcut_passwords: item      # item | start_unlocked
```

## 8. To verify in the playthrough

- Which base state (`090` vs `100`) keeps the most of Hogwarts usable, and what each state hides.
- Every locked door: present? blocking? what's behind it? (log with F11 in MapTracker)
- Whether the Library and DADA → Expecto doors physically lead toward Adv3/Adv2.
- Where grounds triggers could go for Adv5Shack, Adv6Paddock and BuckyExecuted.
- Which story maps are pure cutscene (no player control).
- Arrival spawn points and state requirements for each story map.
