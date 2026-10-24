#!/bin/sh
# Builds HP3WindowFix.asi (32-bit Windows DLL) with clang + lld-link, no MSVC/MinGW needed.
set -e
cd "$(dirname "$0")"
OUT=${1:-build}; mkdir -p "$OUT"
mk() { lib=$1; shift; { echo "LIBRARY $lib.dll"; echo "EXPORTS"; for s in "$@"; do echo "  $s"; done; } > "$OUT/$lib.def"
       llvm-dlltool -m i386 -k -d "$OUT/$lib.def" -l "$OUT/$lib.lib"; }
mk kernel32 CreateThread@24 Sleep@4 GetCurrentProcessId@0 DisableThreadLibraryCalls@4 GetFileAttributesW@4 \
  GetModuleFileNameW@12 WritePrivateProfileStringW@16 GetPrivateProfileStringW@24 GetPrivateProfileIntW@16 CreateFileW@28 WriteFile@20 \
  CloseHandle@4 GetLocalTime@4 WideCharToMultiByte@32
mk user32 EnumWindows@8 GetWindowThreadProcessId@8 IsWindowVisible@4 IsWindow@4 IsIconic@4 GetWindowRect@8 \
  GetWindowLongW@8 SetWindowPos@28 MonitorFromWindow@8 MonitorFromRect@8 GetMonitorInfoW@8 GetForegroundWindow@0 keybd_event@16 wsprintfW
mk shell32 SHGetFolderPathW@20
clang --target=i686-pc-windows-msvc -O2 -fno-builtin -ffreestanding -fno-stack-protector -mno-sse \
  -Wall -Wextra -c windowfix.c -o "$OUT/windowfix.obj"
lld-link /dll /nodefaultlib /entry:DllMain@12 /subsystem:windows /machine:x86 /safeseh:no \
  "$OUT/windowfix.obj" "$OUT/kernel32.lib" "$OUT/user32.lib" "$OUT/shell32.lib" /out:"$OUT/HP3WindowFix.asi"
echo "built $OUT/HP3WindowFix.asi"
