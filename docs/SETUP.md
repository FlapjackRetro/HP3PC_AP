# Setup

This is the setup that's been tested on Windows: retail US disc (v1.0), HP3-UHDE core files and the 60 fps patch.
It boots to the main menu, starts a new game and plays normally, windowed or fullscreen (Alt+Enter toggles).

## 1. Install the base game

1. Run `setup.exe` from the disc (or a copy of the disc contents) and enter your CD key.
2. The default path is `C:\Program Files (x86)\EA GAMES\Harry Potter and the Prisoner of Azkaban(TM)\`.
   Skip the bundled DirectX 8.1/9.0 installer.
3. Settings and saves live in `Documents\Harry Potter and the Prisoner of Azkaban\` (`hppoa.ini`, `User.ini`, `Save\SaveN.usa`).

Writing to `Program Files` needs admin rights, so the copy steps below need an elevated prompt.

## 2. HP3-UHDE (core files)

HP3-UHDE hasn't published a release, and its README tells you to use TFCInstaller (Nexus Mods) to install it.
TFCInstaller is only needed for the HD texture pack, which this project doesn't need. The core files can be
copied by hand from a checkout of [Vawlpe/HP3-UHDE](https://github.com/Vawlpe/HP3-UHDE):

| From `Mod/Game/system/` | What it is |
| --- | --- |
| `hppoa.exe` | No-CD launcher (Deviance), needed because Windows 10/11 won't run SafeDisc. Back up the retail `hppoa.exe` first. |
| `d3d8.dll` | crosire's d3d8to9 |
| `d3d9.dll` | ThirteenAG's Ultimate ASI Loader (with DirectX Windower embedded). Loads `plugins\*.asi`. |
| `plugins/dxgi.asi` | ReShade 6.3.3 (32-bit) |
| `Default.ini.IniPatch`, `defuser.ini.IniPatch` | Apply to `system\Default.ini` / `defuser.ini` |

Also apply `Mod/System/MyDocuments/.../hppoa.ini.IniPatch` and `User.ini.IniPatch` to the files in Documents.
They set 1920x1080, maximum texture detail, VSync with a 60 Hz refresh rate, and FOV 100.39.
[`tools/inipatch.py`](../tools/inipatch.py) applies `.IniPatch` files and keeps the line endings intact:

```
python tools/inipatch.py hppoa.ini hppoa.ini path/to/hppoa.ini.IniPatch
```

`Save500.usa` in the same folder is UHDE's 100% save. It's handy for testing and doesn't overwrite your own slots.

## 3. Framerate fix (60 fps cap)

```
python tools/framerate_patch.py "C:\Program Files (x86)\EA GAMES\Harry Potter and the Prisoner of Azkaban(TM)\system\Engine.dll"
```

The script only accepts the retail `Engine.dll` and writes `Engine.dll.retail.bak` next to it.

**What it changes:** `UGameEngine::GetMaxTickRate()` ends by returning a float constant for standalone play.
In retail that constant is `0.0` (no cap). The patch points the instruction at an existing `60.0` constant
(file offset `0x118CA3`: `2C 20 65` → `54 55 67`, i.e. `fld [0x1065202C]` → `fld [0x10675554]`).

This is the same fix as the community "framerate mod" `Engine.dll` that circulates on Google Drive. That file also
changes `FStats::Render` so the `stat fps` overlay always shows. Pass `--with-fps-counter` to get a byte-identical copy of it.
