# Harry Potter 3 PC apworld

`hp3pc/` is the Archipelago world for Harry Potter and the Prisoner of Azkaban (PC, 2004) with the Open Castle mod.
Version 0.2.0, for **Archipelago 0.6.8**. Design and logic: [docs/APWORLD.md](../docs/APWORLD.md).

It generates seeds and carries the client ("Harry Potter 3 PC Client" in the Archipelago launcher). The client talks to
the server and, on 127.0.0.1:42780, to the Open Castle mod (HP3OpenCastle.asi poc9a or later), which applies items and
reports checks. Protocol and save handling: [docs/CLIENT_SYNC.md](../docs/CLIENT_SYNC.md).

## Play

1. Install the Open Castle mod (`install_opencastle.bat`).
2. Put `hp3pc.apworld` in Archipelago's `custom_worlds`, generate locally, and host the result (for example with Archipelago's MultiServer).
3. Open the Archipelago launcher, start **Harry Potter 3 PC Client**, connect to the room with your slot name.
4. Start the game, New Game (or a save bound to this room). The client log says "Game connected".

## Install (to generate)

Put `hp3pc.apworld` in Archipelago's `custom_worlds` folder, then put a player YAML (start from
`Harry Potter 3 PC.yaml`) in `Players` and run Generate.

## Regenerate the location table

The locations and their rules come from the catalogue and the logic notes:

```sh
python tools/build_apworld_locations.py
```

It rewrites `hp3pc/location_table.py`. Location IDs live in `data/location_ids.json`: a name keeps its ID forever,
new names get the next free one. Don't renumber after a release.

## Test and build (from an Archipelago 0.6.8 source checkout)

```sh
ln -s /path/to/repo/apworld/hp3pc worlds/hp3pc
python -m unittest discover -s worlds/hp3pc/test -t .
python Launcher.py "Build APWorlds" -- "Harry Potter 3 PC"     # -> build/apworlds/hp3pc.apworld
```
