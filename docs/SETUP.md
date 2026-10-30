# Setup

This is the setup that's been tested on Windows: retail US disc (v1.0), HP3-UHDE core files and the 60 fps patch.
It boots to the main menu, starts a new game and plays normally, windowed or fullscreen (Alt+Enter toggles).

## Quick setup

After step 1, `setup.bat` (in the repository root) does steps 2 to 4 for you: it downloads the HP3-UHDE core
files and applies their ini patches, patches the framerate, builds `HP3OpenCastle.asi` and `HP3AP.utx`, and
installs them. It asks for administrator rights (the game is under `Program Files`). Running it again is safe;
after a `git pull` it rebuilds and reinstalls the mod.

You need:
- **Python 3.8+** ([python.org](https://www.python.org/downloads/) or `winget install Python.Python.3.12`).
  Pillow is installed for you if it's missing.
- **LLVM** for the build (`winget install LLVM.LLVM`; the script offers to install it).
- The game **started once** (to the main menu is enough), so its Documents folder and ini files exist.
  Otherwise those two ini patches are skipped; run `setup.bat` again afterwards.

Options (`setup.bat -DebugBuild -MapTracker`, or `setup.ps1 -?` for the full list): `-GameDir` for another
install folder, `-DebugBuild` for the debug build of the mod, `-MapTracker` / `-WindowFix` for the extra plugins,
`-TestSave` for UHDE's 100% save, and `-SkipUHDE`, `-SkipFramerate`, `-SkipBuild`, `-SkipInstall` to leave a step out.
For a `-GameDir` with spaces, run `setup.ps1` from an elevated PowerShell instead of `setup.bat`.

Then do step 5. The sections below are the manual steps that `setup.bat` automates.

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

Also apply `Mod/System/MyDocuments/Harry Potter and the Prisoner of Azkaban/hppoa.ini.IniPatch` and `User.ini.IniPatch`
to the files in Documents (they exist after the game has been started once).
They set 1920x1080, maximum texture detail, VSync with a 60 Hz refresh rate, and FOV 100.39.
[`tools/inipatch.py`](../tools/inipatch.py) applies `.IniPatch` files and keeps the line endings intact. Its arguments
are the input ini, the output ini (the same file to patch in place) and one or more patches:

```
python tools/inipatch.py hppoa.ini hppoa.ini path/to/hppoa.ini.IniPatch
```

`Save/Save500.usa` in the same folder is UHDE's 100% save. It's handy for testing and doesn't overwrite your own slots.

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

## 4. Open Castle mod

Build the plugin and its textures (needs LLVM and Pillow, see Quick setup), then copy them into the game:

```
python tools/build_mods.py build
python tools/build_hp3ap.py path/to/Archipelago/data/icon.png "<game folder>" build/HP3AP.utx
```

| Built file | Goes to |
| --- | --- |
| `build/release/HP3OpenCastle.asi` (or `build/HP3OpenCastle.asi`, the debug build) | `system\plugins\` |
| `build/HP3AP.utx` (Archipelago item textures) | `textures\` |
| `build/HP3MapTracker.asi`, `build/HP3WindowFix.asi` (optional) | `system\plugins\` |

`mod/OpenCastle/install_opencastle.bat` copies a prebuilt plugin instead.

## 5. Archipelago

See [apworld/README.md](../apworld/README.md): put `hp3pc.apworld` in Archipelago's `custom_worlds`, start
**Harry Potter 3 PC Client** from the launcher, connect, then start the game.
