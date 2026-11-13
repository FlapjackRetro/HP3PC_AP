# Setup

This is the setup that's been tested on Windows: retail US disc (v1.0), HP3-UHDE core files and the 60 fps patch.
To check that your copy matches the tested one, run `tools\verify_retail.ps1` (file list and hashes: [RETAIL.md](RETAIL.md)).
It boots to the main menu, starts a new game and plays normally, windowed or fullscreen (Alt+Enter toggles).

## Quick setup

First get the whole repository, not just the script: `setup.bat` runs the repository's own tools and builds the
mod from its source. Either clone it (`git clone https://github.com/FlapjackRetro/HP3PC_AP.git`) or download it
from GitHub (**Code** > **Download ZIP**) and unzip it. Run `setup.bat` from that folder.

After step 1 (including its start-up test), `setup.bat` (in the repository root) installs SafeDiscShim if your copy needs it (see step 1) and does steps 2 to 4 for you: it downloads the HP3-UHDE core
files (not its executable) and applies their ini patches, patches the framerate, builds `HP3OpenCastle.asi` and `HP3AP.utx`, and
installs them. It asks for administrator rights (the game is under `Program Files`). Running it again is safe;
after a `git pull` it rebuilds and reinstalls the mod.

You need:
- **Python 3.8+** ([python.org](https://www.python.org/downloads/) or `winget install Python.Python.3.12`).
  Pillow is installed for you if it's missing.
- **LLVM** for the build (`winget install LLVM.LLVM`; the script offers to install it).
- The game **started once** with the disc in (step 1.4), so its Documents folder and ini files exist.
  Otherwise those two ini patches are skipped; run `setup.bat` again afterwards. If it stopped at "Cannot locate
  the CD-ROM" instead, run `setup.bat` anyway: it installs SafeDiscShim, then start the game and run `setup.bat` again.

Options (`setup.bat -DebugBuild -MapTracker`, or `setup.ps1 -?` for the full list): `-GameDir` for another
install folder, `-DebugBuild` for the debug build of the mod, `-MapTracker` / `-WindowFix` for the extra plugins,
`-TestSave` for UHDE's 100% save, and `-SkipSafeDiscShim` (or `-ForceSafeDiscShim`), `-SkipUHDE`, `-SkipFramerate`, `-SkipBuild`, `-SkipInstall` to leave a step out.
For a `-GameDir` with spaces, run `setup.ps1` from an elevated PowerShell instead of `setup.bat`.

Then do step 5. The sections below are the manual steps that `setup.bat` automates.

## 1. Install the base game

1. Run `setup.exe` from the disc and enter your CD key.
2. The default path is `C:\Program Files (x86)\EA GAMES\Harry Potter and the Prisoner of Azkaban(TM)\`.
   Skip the bundled DirectX 8.1/9.0 installer.
3. Settings and saves live in `Documents\Harry Potter and the Prisoner of Azkaban\` (`hppoa.ini`, `User.ini`, `Save\SaveN.usa`).
4. **Start the game once, with the disc in the drive, before running `setup.bat`.** Use the Start menu shortcut,
   not the installer's "play now". This creates the Documents folder that setup patches. It is also the test for
   whether you need SafeDiscShim (below), done while nothing has been modded yet.

This project doesn't provide or modify `hppoa.exe`: it uses the one the disc installed. Keep the disc in the drive
whenever you play. A copied or mounted image of the disc (ISO) doesn't pass the disc check.

### Does your copy need SafeDiscShim?

Look at what the game did in step 4, with the disc in the drive:

| What you saw | What it means | What to do |
| --- | --- | --- |
| The main menu | Your copy's protection runs on this PC without help | Nothing. Answer **n** when `setup.bat` offers SafeDiscShim, or pass `-SkipSafeDiscShim` |
| **"Cannot locate the CD-ROM"** (*Please insert the correct CD-ROM, select OK and restart application*), with the disc in | The protection needs the `secdrv.sys` driver, which Windows 10/11 block | Install SafeDiscShim: answer **Y** when `setup.bat` offers it (or install it yourself, below), then start the game again with the disc in |
| Nothing at all, or a crash before any window | Probably not the disc check | See Troubleshooting below |

If you saw "Cannot locate the CD-ROM" with the disc **ejected**, that's the disc check working as intended. Put the
disc in and try again.

You can also check whether your `hppoa.exe` has SafeDisc at all. `setup.bat` does this for you and skips the
SafeDiscShim step when it doesn't, or when it's the tested retail exe (pass `-ForceSafeDiscShim` to be offered it anyway). By hand, in PowerShell:

```powershell
$b = [IO.File]::ReadAllBytes("C:\Program Files (x86)\EA GAMES\Harry Potter and the Prisoner of Azkaban(TM)\system\hppoa.exe")
[Text.Encoding]::ASCII.GetString($b).Contains('BoG_')   # True: SafeDisc 2 or later
```

### Why: SafeDisc and `secdrv.sys`

The retail disc is protected by SafeDisc, which has two separate parts:

1. **The disc check.** The game's code ships encrypted. At start-up SafeDisc reads the disc, looks for a physical
   fingerprint pressed into it (which copying the disc doesn't reproduce), and only with the right disc can it decrypt
   the game. This part needs no driver. It's why the disc must always be in the drive.
2. **`secdrv.sys`, a kernel driver.** It is there to stop crackers. SafeDisc asks it whether a kernel debugger is
   running (CPU debug registers, the interrupt table) and runs a handshake with it. For years Windows shipped this
   driver itself. It had a security hole that let any program run code in the Windows kernel, so Windows 10
   disabled it and Windows 11 blocks it.

Whether a game fails without the driver depends on the SafeDisc version and on how much that version relies on the
driver's answers. **The tested copy ([RETAIL.md](RETAIL.md)) doesn't need it:** the retail US v1.0 disc reaches the
main menu on Windows 11 (build 26200) with no `secdrv.sys` and no SafeDiscShim, and refuses to start with the disc
ejected, as it should. `setup.bat` recognises that `hppoa.exe` and skips the SafeDiscShim step for it. A version that relies on them can't get them on Windows 10/11 and gives up with the same "Cannot
locate the CD-ROM" message as a missing disc.

### SafeDiscShim

**[SafeDiscShim](https://github.com/RibShark/SafeDiscShim)** (RibShark, GPLv3) gives SafeDisc the driver's answers
without installing any driver. It doesn't skip the disc check and changes no files: **the original disc is still
required.** `setup.bat` offers the pinned v0.9.0 installer and checks its hash. You can also install it yourself
from its Releases page.

- **What it installs:** `drvmgt.dll` in `Windows\SysWOW64`, and a Windows compatibility database (via `sdbinst`)
  that loads it into SafeDisc's start-up helper. The helper then restarts the game with the shim loaded, with the
  same executable, folder and command line.
- **It doesn't affect HP3-UHDE or this mod.** It only watches process creation and the `secdrv` requests, and
  passes every other device request through. Ultimate ASI Loader (`d3d9.dll`) loads later, when the game starts its
  renderer, and loads `plugins\*.asi` as usual. Our plugins only patch `Core.dll` and `Engine.dll`, never `hppoa.exe`.
- **How to tell it worked:** `hppoa.exe_SafeDiscShim.log` appears in the game's `system\` folder (beta builds log
  by default). Set the environment variable `SAFEDISCSHIM_LOGLEVEL=none` to turn logging off.
- **Removing it:** uninstall it from Windows' installed apps.

Other routes: a Windows XP virtual machine runs the retail disc as is. Re-enabling `secdrv.sys` itself reopens the
security hole it was blocked for, and isn't recommended.

### Troubleshooting

- **"Cannot locate the CD-ROM" with the disc in and SafeDiscShim installed.**
  - Check that `hppoa.exe_SafeDiscShim.log` exists. If it doesn't, the shim didn't load: reinstall it.
  - If the log exists, check whether setup's changes are the cause by starting the unmodded game:
    1. In `system\`, rename `Engine.dll` to `Engine.dll.patched`, then `Engine.dll.retail.bak` to `Engine.dll`.
    2. Move `d3d8.dll`, `d3d9.dll` and `plugins\` out of `system\` for now.
    3. Start the game with the disc in.
  - If the unmodded game starts, put the files back one at a time to find which one SafeDisc objects to, and
    report it in Issues.

Writing to `Program Files` needs admin rights, so the copy steps below need an elevated prompt.

## 2. HP3-UHDE (core files)

HP3-UHDE hasn't published a release, and its README tells you to use TFCInstaller (Nexus Mods) to install it.
TFCInstaller is only needed for the HD texture pack, which this project doesn't need. The core files can be
copied by hand from a checkout of [FlapjackRetro/HP3-UHDE-noexe](https://github.com/FlapjackRetro/HP3-UHDE-noexe), the
no-exe edition of Vawlpe's HP3-UHDE (no game executable in any commit). Keep the game's own executable (see step 1).

| From `Mod/Game/system/` | What it is |
| --- | --- |
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
