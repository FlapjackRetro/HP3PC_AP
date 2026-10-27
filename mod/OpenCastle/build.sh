#!/bin/sh
# Builds HP3OpenCastle.asi (32-bit Windows DLL) with clang + lld-link, no MSVC/MinGW needed.
set -e
cd "$(dirname "$0")"
OUT=${1:-build}; mkdir -p "$OUT"
mk() { lib=$1; shift; { echo "LIBRARY $lib.dll"; echo "EXPORTS"; for s in "$@"; do echo "  $s"; done; } > "$OUT/$lib.def"
       llvm-dlltool -m i386 -k -d "$OUT/$lib.def" -l "$OUT/$lib.lib"; }
mk kernel32 GetModuleHandleW@4 GetProcAddress@8 CreateThread@24 Sleep@4 ReadProcessMemory@20 GetCurrentProcess@0 \
  DisableThreadLibraryCalls@4 CreateFileW@28 WriteFile@20 CloseHandle@4 GetLocalTime@4 WideCharToMultiByte@32 \
  VirtualProtect@16 VirtualAlloc@16 FlushInstructionCache@12 \
  GetPrivateProfileIntW@16 GetTickCount@0 ReadFile@20 GetFileSize@8 SetFilePointer@16 GetCurrentProcessId@0 WritePrivateProfileStringW@16
mk user32 wsprintfW GetAsyncKeyState@4 GetForegroundWindow@0 GetWindowThreadProcessId@8
mk shell32 SHGetFolderPathW@20
clang --target=i686-pc-windows-msvc -O2 -fno-builtin -ffreestanding -fno-stack-protector -mno-sse \
  -Wall -Wextra -Wno-unused-function -c opencastle.c -o "$OUT/opencastle.obj"
lld-link /dll /nodefaultlib /entry:DllMain@12 /subsystem:windows /machine:x86 /safeseh:no \
  "$OUT/opencastle.obj" "$OUT/kernel32.lib" "$OUT/user32.lib" "$OUT/shell32.lib" /out:"$OUT/HP3OpenCastle.asi"
echo "built $OUT/HP3OpenCastle.asi"
