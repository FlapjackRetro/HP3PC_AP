# Third-party assets

## Archipelago logo (`data/icon.png` of Archipelago, used by `tools/gen_ap_logos.py`)
- Source: the Archipelago icon, `data/icon.png` in https://github.com/ArchipelagoMW/Archipelago (tag 0.6.8),
  by the Archipelago project. It isn't committed here: `setup.ps1` downloads it, or you pass your own copy.
- License: the original Archipelago logo is available under the **MIT** license. The newer logos are derivative
  works of it and are available under **CC BY-NC 4.0** (Creative Commons Attribution-NonCommercial 4.0
  International). The version was not stated by the source; 4.0, the latest, is assumed. MIT is compatible with
  this, since attribution is honored either way.
- Attribution: logo © the Archipelago project contributors. `tools/gen_ap_logos.py` scales it and draws three
  item tiers from it (an up arrow for progression, the plain logo for useful, grayscale for filler), which
  `tools/build_hp3ap.py` packs into the textures `APLogoProgression`, `APLogoUseful` and `APLogoFiller` of the
  locally built `HP3AP.utx`.
- **Non-commercial:** because of the CC BY-NC terms, this asset and anything built from it (notably the generated
  `HP3AP.utx`) may not be used or distributed commercially.

## HP3-UHDE core files (`thirdparty/hp3-uhde/`)
- Source: the core files of HP3:UHDE (Harry Potter 3: Ultimate HD Edition) by Vawlpe, included unchanged. No game
  executable is among them, and the HD texture pack isn't included.
- License: HP3:UHDE is Copyright (C) 2025 Vawlpe, under the **GNU GPL v3** (or later). It bundles three binaries
  under their own licenses: d3d8to9 (`d3d8.dll`, crosire / Patrick Mours, **BSD 2-Clause**), Ultimate ASI Loader
  (`d3d9.dll`, ThirteenAG, **MIT**) and ReShade 6.3.3 (`plugins/dxgi.asi`, crosire / Patrick Mours, **BSD 3-Clause**).
  The full license texts are in `thirdparty/hp3-uhde/licenses/`, and the credits and checksums in
  `thirdparty/hp3-uhde/README.md`.

The rest of this repository's own code and documentation is not covered by this file. The game itself
(*Harry Potter and the Prisoner of Azkaban*, PC, 2004, Electronic Arts / KnowWonder) is not included and remains
its owners' property; nothing derived from its code, binaries or assets is committed, and `HP3AP.utx`'s other
textures are built from your own install.
