# Harry Potter 3 PC apworld

`hp3pc/` is the Archipelago world for Harry Potter and the Prisoner of Azkaban (PC, 2004) with the Open Castle mod.
Version 0.1.0, for **Archipelago 0.6.8**. Design and logic: [docs/APWORLD.md](../docs/APWORLD.md).

It generates seeds. Playing them needs the game-side client (receiving items, sending checks), which isn't written
yet: see [docs/CLIENT_SYNC.md](../docs/CLIENT_SYNC.md).

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
