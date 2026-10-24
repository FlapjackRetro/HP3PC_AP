# HP3 MapTracker

A tiny Ultimate ASI Loader plugin for documenting the game. It's read-only and never changes game memory.

- **Title bar:** `HP3 | <map> | <GState> | End = mark`, reapplied every second, because the game rewrites its own title (visible when windowed).
- **Log:** every change is appended to `Documents\Harry Potter and the Prisoner of Azkaban\HP3_MapLog.txt`:
  ```
  2026-10-24 09:02:11  MAP HP3_InsideHub  state=GState010  (id=ihub package=Save100)
  2026-10-24 09:05:40  MARK  map=HP3_InsideHub  state=GState010  pos=(-2412, -1100, 768)
  ```
- **End** (also F11, Scroll Lock or Pause; game focused): writes a `MARK` line with the player's position, for locked
  doors and points of interest. Keys are caught with a low-level keyboard hook, because the game reads the keyboard
  through DirectInput. The log notes when the hook starts receiving keys, which F-keys arrive, and presses ignored
  because another window had focus.
- Once the player exists, a `ready: offsets ...` line reports the property offsets it found. A `-1` there means
  that field (state or position) can't be shown; the map name still works.

## Install

Requires HP3-UHDE's `d3d9.dll` (Ultimate ASI Loader). Copy `HP3MapTracker.asi` to `<game>\system\plugins\`
(admin rights needed under `Program Files`). Delete it to uninstall.

## How it works

A background thread polls about once a second. It reads Core.dll's exported `UObject::GObjObjects` and
`FName::Names` and looks for:
- the live `PlayerController`: the viewport's `Player.Actor`. Controllers from earlier levels linger until garbage
  collection, still holding their pawns, so picking one by "has a pawn" can return a stale level;
- its `Level` (the map's `LevelInfo`) → `ID`, mapped back to the map file name (`ihub` → `HP3_InsideHub`). The
  package name isn't used directly, because a map loaded from a save is named after the save (`Save100`), and
  `HP_Preamble` stays loaded alongside it;
- `Level.Game.CurrentGameState` → the state;
- `Pawn.Location` → the F11 position.

Struct offsets were taken from disassembly of Core.dll's exported accessors: `UObject` Outer +0x18, Name +0x20,
Class +0x24; `UStruct` SuperField +0x2C; `UProperty` Offset +0x4C; `FNameEntry` name +0x0C. Property offsets are
looked up by name at runtime. Every read goes through `ReadProcessMemory` on its own process, so a stale pointer
during a level load fails instead of crashing.

## Build

```
./build.sh            # needs clang, lld-link and llvm-dlltool; no MSVC or MinGW
test/harness.c        # host-side test against a fake engine memory layout:
gcc -O1 -w -fshort-wchar -U_FORTIFY_SOURCE -o test/harness test/harness.c && ./test/harness
```

There's no C runtime: imports are kernel32, user32 and shell32 only, and the output is about 14 KB.
