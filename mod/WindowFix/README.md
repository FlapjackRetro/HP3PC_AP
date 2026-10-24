# HP3 WindowFix

An Ultimate ASI Loader plugin that fixes two window annoyances:

- **Remembers fullscreen or windowed.** The game reads `StartupFullscreen` from `hppoa.ini` at startup, but
  Alt+Enter never saves it. The plugin writes the current mode to that key whenever it changes and once more when
  the game exits, so the game starts the way you left it.
- **Centers the window.** Whenever the game becomes windowed (at startup or after Alt+Enter) the window is moved
  to the middle of its monitor. If it's bigger than the screen, the title bar stays visible. After that it's left
  alone, so you can drag it anywhere.

If the ini says windowed and the game still starts fullscreen, the plugin presses Alt+Enter once for you.

It only uses window functions and the ini file; it never touches game memory.

## Install

Requires HP3-UHDE's `d3d9.dll` (Ultimate ASI Loader). Copy `HP3WindowFix.asi` to `<game>\system\plugins\`
(admin rights needed under `Program Files`). Delete it to uninstall.

The ini is `Documents\Harry Potter and the Prisoner of Azkaban\hppoa.ini` (or `system\hppoa.ini` if that doesn't
exist). What the plugin does is logged to `HP3_WindowFix.log` in the same Documents folder.

## Build

```
./build.sh            # needs clang, lld-link and llvm-dlltool; no MSVC or MinGW
```

No C runtime: imports are kernel32, user32 and shell32 only, and the output is about 6 KB.
