# HP3 WindowFix

An Ultimate ASI Loader plugin that fixes two window annoyances:

- **Remembers fullscreen or windowed.** The game reads `StartupFullscreen` from `hppoa.ini` at startup, but
  Alt+Enter never saves it. The plugin writes the current mode to that key whenever it changes and once more when
  the game exits, so the game starts the way you left it.
- **Remembers where the window was.** When you move the window, its position is saved to `HP3_WindowFix.ini`.
  Whenever the game becomes windowed or resizes the window (it opens at 640×480, then switches to the configured size and its own default position), the window goes back there. Only moves made while a mouse button is held count as yours; if the game moves the window on its own, it's put back. With no saved
  position, or if it's no longer on any monitor, the window is centered instead (title bar kept on screen if the
  window is bigger than the monitor). This uses its own file because the engine rewrites `hppoa.ini` from memory
  when it exits.

If the ini says windowed and the game still starts fullscreen, the plugin presses Alt+Enter once for you.

It only uses window functions and the ini file; it never touches game memory.

## Install

Requires HP3-UHDE's `d3d9.dll` (Ultimate ASI Loader). Copy `HP3WindowFix.asi` to `<game>\system\plugins\`
(admin rights needed under `Program Files`). Delete it to uninstall.

The ini is `Documents\Harry Potter and the Prisoner of Azkaban\hppoa.ini` (or `system\hppoa.ini` if that doesn't
exist). `HP3_WindowFix.ini` (saved position) and `HP3_WindowFix.log` (what the plugin did) go in the same Documents folder.

## Build

```
./build.sh            # needs clang, lld-link and llvm-dlltool; no MSVC or MinGW
```

No C runtime: imports are kernel32, user32 and shell32 only, and the output is about 8 KB.
