# HP3PC_AP

An in-progress [Archipelago](https://archipelago.gg) randomizer for **Harry Potter and the Prisoner of Azkaban** (PC, 2004).

## The game

*Harry Potter and the Prisoner of Azkaban* was developed by KnowWonder and published by Electronic Arts in 2004.
It is a third-person action-adventure game that follows the third book and film. You play as Harry, Ron and
Hermione, and switch between them to use each character's abilities. The game is built around Hogwarts:
the castle and grounds act as hubs between classroom challenge levels (Carpe Retractum, Draconifors, Glacius),
adventure levels and the final exams. Collectibles include about 80 Wizard Cards, Bertie Bott's Every Flavour
Beans, Cauldron Cakes and Pumpkin Pasties (the currencies used at Fred and George's shop and the student
vendors), challenge stars and shields, secret areas, portrait passwords and the pieces of the Marauder's Map.

The PC version runs on KnowWonder's build of Unreal Engine 2, and its gameplay is almost entirely
UnrealScript. That is what makes it a good fit for a randomizer.

## Archipelago

Archipelago is a multi-game randomizer: items from one game can turn up in another player's game, and every
player works through their own world while unlocking each other's progress. Supporting HP3 means describing
its items, locations and logic as an Archipelago "world", plus a client that connects the running game to the
Archipelago server. The sister project for the previous game is
[HP2PC_AP](https://github.com/Kryen112/HP2PC_AP) (Chamber of Secrets).

## Components

The randomizer is planned for this setup:

| Component | Role |
| --- | --- |
| **Harry Potter and the Prisoner of Azkaban (PC)**, retail disc | The base game. Not included; bring your own copy. |
| **[HP3-UHDE](https://github.com/Vawlpe/HP3-UHDE)** by Vawlpe | Makes the game run on modern Windows: d3d8to9, Ultimate ASI Loader, ReShade, config tweaks, and the no-CD launcher that's needed because Windows 10/11 can't run SafeDisc. The HD texture pack is optional and isn't needed here. The ASI loader also gives us a way to load our own code. |
| **Framerate fix** | Caps the game at 60 fps. Retail HP3 runs uncapped in single player, and high frame rates are associated with softlocks (HP3-UHDE lists its FPS lock as a "Softlock Fix", still unfinished). HP3-UHDE itself only turns on VSync, so this is applied separately with [`tools/framerate_patch.py`](tools/framerate_patch.py). See [docs/SETUP.md](docs/SETUP.md). |

## Status

Research stage. Done so far:

- A decompiler for the game's script packages (all 1,422 classes in HGame, kwGame, GamePlay and Engine decompile without errors).
- A first catalogue of potential check locations across all 34 maps: [`data/hp3_location_catalog.csv`](data/hp3_location_catalog.csv).
- The Wizard Card table and the story game-state list, pulled from the game's defaults.
- Notes on how items, saves and progression work: [docs/FINDINGS.md](docs/FINDINGS.md).
- How maps, story game states and level loading work, and every map's entrances: [docs/SCENES.md](docs/SCENES.md), [`data/map_entrances.json`](data/map_entrances.json).
- The Open Castle design (checks, items, area unlocks, entrance layout, draft YAML): [docs/OPEN_CASTLE.md](docs/OPEN_CASTLE.md).
- [MapTracker](mod/MapTracker/README.md): a small plugin that shows the current map and story state in the title bar and logs them, for documenting a playthrough.

## To do

- [ ] **Client framework.** Choose and build the bridge between the game and Archipelago: most likely a native
      `.asi` plugin loaded by HP3-UHDE's ASI loader, with a script-level file-I/O channel as a fallback.
      Then the Archipelago client that talks to it (see [Bridge options](docs/FINDINGS.md#bridge-options)).
- [ ] **Fully document scenes and load triggers.** Map every level, how it's entered and left
      (`ChangeLevelTrigger`s and the hub connections), which story game state (`GSTATE000`–`GSTATE120`) gates it,
      and fill in the missing Level IDs.
- [ ] **Decide how to tweak the game for Archipelago.** For example: turning spells into items through
      `WeaponWand.HasSpell()`, handling vendors and the Fred & George shop, and whether to offer an
      **Open Castle** mode like HP2PC_AP's, with levels unlocked from the start instead of following the story.
- [ ] **Build the logic framework for checks.** Write the apworld's items, locations, regions and access rules
      from the catalogue, decide which categories become checks (cards, stars, secrets, challenge wins, chests,
      vendors), and make the goal condition.

## Repo layout

| Path | Contents |
| --- | --- |
| `docs/FINDINGS.md` | What's known so far about the engine, save keys, items, locations and hooks |
| `docs/SETUP.md` | Installing the base game, HP3-UHDE and the framerate fix |
| `docs/SCENES.md` | Maps vs. game states, how levels load, story chains, challenge/exam entrances |
| `docs/OPEN_CASTLE.md` | Open Castle design: checks, items, area unlocks, entrance plan, draft options |
| `mod/MapTracker/` | Map/state tracker plugin (C, built with clang; no game code) |
| `data/` | Generated tables: location catalogue (CSV and JSON), map entrances, wizard cards, game states |
| `tools/` | Python tools: package reader/decompiler, catalogue and entrance-graph builders, framerate patcher |

Nothing derived from the game's own code or binaries is committed. Run the tools against your own install to
regenerate the decompiled scripts locally ([docs/FINDINGS.md](docs/FINDINGS.md#tools)).

*Not affiliated with or endorsed by Warner Bros., Electronic Arts, KnowWonder or J.K. Rowling.*
