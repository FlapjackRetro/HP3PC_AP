# HP3-UHDE core files

These are the core files of **HP3:UHDE (Harry Potter 3: Ultimate HD Edition) by Vawlpe**, included here unchanged so
that setup doesn't need to fetch them from anywhere. HP3:UHDE makes the PC version of *Harry Potter and the Prisoner of
Azkaban* run well on modern Windows. This project uses only these core files: no game executable, and not the HD
texture pack.

`setup.bat` (step 2 of [docs/SETUP.md](../../docs/SETUP.md)) copies them into the game and applies the ini patches.

| File | What it is | By | License |
| --- | --- | --- | --- |
| `system/d3d8.dll` | d3d8to9: turns the game's Direct3D 8 calls into Direct3D 9 | crosire (Patrick Mours) | BSD 2-Clause, [`licenses/d3d8to9-BSD-2-Clause.md`](licenses/d3d8to9-BSD-2-Clause.md) |
| `system/d3d9.dll` | Ultimate ASI Loader (with DirectX Windower): loads `plugins\*.asi`, which is how this project's plugins load | ThirteenAG | MIT, [`licenses/Ultimate-ASI-Loader-MIT.txt`](licenses/Ultimate-ASI-Loader-MIT.txt) |
| `system/plugins/dxgi.asi` | ReShade 6.3.3 (32-bit): post-processing | crosire (Patrick Mours) | BSD 3-Clause, [`licenses/ReShade-BSD-3-Clause.md`](licenses/ReShade-BSD-3-Clause.md); ReShade's own third-party components are under their own licenses |
| `system/Default.ini.IniPatch`, `system/defuser.ini.IniPatch` | Settings for the game's `system\Default.ini` and `defuser.ini` | Vawlpe | GPL-3.0, [`licenses/HP3-UHDE-GPL-3.0.md`](licenses/HP3-UHDE-GPL-3.0.md) |
| `MyDocuments/hppoa.ini.IniPatch`, `MyDocuments/User.ini.IniPatch` | Settings for `Documents\Harry Potter and the Prisoner of Azkaban\hppoa.ini` and `User.ini`: 1920x1080, maximum texture detail, VSync at 60 Hz, FOV 100.39 | Vawlpe | GPL-3.0 |
| `MyDocuments/Save/Save500.usa` | HP3:UHDE's 100% save (`setup.bat -TestSave`) | Vawlpe | GPL-3.0 |

HP3:UHDE is Copyright (C) 2025 Vawlpe, licensed under the GNU General Public License, version 3 or (at your option)
any later version; the full text is in [`licenses/HP3-UHDE-GPL-3.0.md`](licenses/HP3-UHDE-GPL-3.0.md). The three
binaries are the unmodified releases of their authors' projects, redistributed under the licenses above, whose
notices are reproduced in [`licenses/`](licenses/).

HP3:UHDE also credits FCH823 and Wastelander121 for TFCInstaller, the tool it uses to install its texture pack.
This project doesn't use the texture pack or TFCInstaller.

## Checksums

| File | Bytes | SHA-256 |
| --- | ---: | --- |
| `system/d3d8.dll` | 123,904 | `1e0096ed7a5c8e19f7840cc8117bc40899300632da077ce66f74ebcd250a5038` |
| `system/d3d9.dll` | 1,699,840 | `29fee0aa46414864ad1f65b2689b470bfb2cf2f09a94e2afd271e68f2e16743b` |
| `system/plugins/dxgi.asi` | 3,691,520 | `94ddadbae44ca4a7be5b797753a8323d985948b0480c383d22ef402c00e031f2` |
| `system/Default.ini.IniPatch` | 566 | `1dc570025f2211fbc0edf4585bc2983044055e0a1d12733eb7101578b08e39ad` |
| `system/defuser.ini.IniPatch` | 44 | `19243581ae3d3695d0b34b83528c3c298299760e3facef582d30b7af5b883fe9` |
| `MyDocuments/hppoa.ini.IniPatch` | 566 | `1dc570025f2211fbc0edf4585bc2983044055e0a1d12733eb7101578b08e39ad` |
| `MyDocuments/User.ini.IniPatch` | 44 | `19243581ae3d3695d0b34b83528c3c298299760e3facef582d30b7af5b883fe9` |
| `MyDocuments/Save/Save500.usa` | 8,852,595 | `19f4dc48fd169119d59943cb313a64e3113b190c1a0d426f73aed7647e277727` |
