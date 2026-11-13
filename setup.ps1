<#
.SYNOPSIS
  HP3 PC Archipelago setup: checks your hppoa.exe, then steps 2 and 3 of docs/SETUP.md (HP3-UHDE core files, not its executable; 60 fps cap), then builds and
  installs the Open Castle plugin (HP3OpenCastle.asi) and its texture package (HP3AP.utx).

.DESCRIPTION
  Run from a checkout of this repository (setup.bat does it for you, as administrator). Each step is skipped when
  it's already done, so running it again is safe (after a `git pull`, it rebuilds and reinstalls the plugin).

  Needs:  Python 3 (python.org or `winget install Python.Python.3.12`); Pillow is installed for you if missing.
          LLVM for the build (`winget install LLVM.LLVM`; offered if missing).
          Step 1 of SETUP.md done (the retail game installed), and the game run once so its Documents folder exists.

.PARAMETER GameDir      The game folder (default: C:\Program Files (x86)\EA GAMES\Harry Potter and the Prisoner of Azkaban(TM)).
.PARAMETER DebugBuild   Install the debug build of Open Castle (debug panel, test grid, noclip) instead of the release one.
.PARAMETER MapTracker   Also install HP3MapTracker.asi (title-bar position, HP3_MapLog.txt, mark key).
.PARAMETER WindowFix    Also install HP3WindowFix.asi.
.PARAMETER TestSave     Also copy UHDE's 100% save (Save500.usa) into the Saves folder.
.PARAMETER SkipExeCheck   Leave out the hppoa.exe check (old name: SkipSafeDiscShim).
.PARAMETER SkipUHDE / SkipFramerate / SkipBuild / SkipInstall   Leave that step out.
#>
param(
    [string]$GameDir = "${env:ProgramFiles(x86)}\EA GAMES\Harry Potter and the Prisoner of Azkaban(TM)",
    [switch]$DebugBuild, [switch]$MapTracker, [switch]$WindowFix, [switch]$TestSave,
    [Alias('SkipSafeDiscShim')][switch]$SkipExeCheck, [switch]$SkipUHDE, [switch]$SkipFramerate, [switch]$SkipBuild, [switch]$SkipInstall
)
$ErrorActionPreference = 'Stop'
$Repo = $PSScriptRoot
$Sys = Join-Path $GameDir 'system'
$MyDocs = [Environment]::GetFolderPath('MyDocuments'); if (-not $MyDocs) { $MyDocs = $HOME }
$Docs = Join-Path $MyDocs 'Harry Potter and the Prisoner of Azkaban'
$Build = Join-Path $Repo 'build'
# everything setup uses is in this repository; it downloads nothing (LLVM and Pillow come from winget / pip if missing)
$UHDE = Join-Path $Repo 'thirdparty\hp3-uhde'   # HP3-UHDE's core files (Vawlpe), included in this repository
$TestedExe = '1ac9c1e37b1cd8d9c581558be8c8ebb607ef0853b66c698fa3b881dcb9b54b0d'   # the verified retail hppoa.exe (docs/RETAIL.md)
$SDShimGuid = '{001827e2-fa73-4907-81de-5596bc8e3d37}'   # SafeDiscShim's compatibility database, if installed
$APIcon = Join-Path $Repo 'thirdparty\archipelago\icon.png'   # the Archipelago logo, included (COPYING.md)

function Find-LLVM {   # LLVM's bin folder: PATH, the installer's registry key (any install folder), the usual folders
    $c = Get-Command clang.exe -CommandType Application -ErrorAction SilentlyContinue | Select-Object -First 1
    $dirs = @()
    if ($c) { $dirs += Split-Path $c.Source }
    foreach ($k in 'HKLM:\SOFTWARE\LLVM\LLVM', 'HKLM:\SOFTWARE\WOW6432Node\LLVM\LLVM', 'HKCU:\SOFTWARE\LLVM\LLVM') {
        $d = (Get-ItemProperty $k -ErrorAction SilentlyContinue).'(default)'
        if ($d) { $dirs += Join-Path $d 'bin' }
    }
    # $env:ProgramW6432 is the 64-bit Program Files even in a 32-bit PowerShell (where $env:ProgramFiles is the x86 one)
    $dirs += "$env:ProgramW6432\LLVM\bin", "$env:ProgramFiles\LLVM\bin", "${env:ProgramFiles(x86)}\LLVM\bin", "$env:LOCALAPPDATA\Programs\LLVM\bin"
    foreach ($d in $dirs) {
        if (-not $d -or -not (Test-Path (Join-Path $d 'clang.exe'))) { continue }
        $miss = @('lld-link.exe', 'llvm-dlltool.exe' | Where-Object { -not (Test-Path (Join-Path $d $_)) })
        if ($miss.Count) { Write-Host "LLVM at $d is missing $($miss -join ', ')" -ForegroundColor Yellow; continue }
        return $d
    }
    Write-Host ("no LLVM in: " + (($dirs | Where-Object { $_ } | Select-Object -Unique) -join '; ')) -ForegroundColor Yellow
    return $null
}
function Step($t) { Write-Host ''; Write-Host "== $t" -ForegroundColor Cyan }
function Fail($t) { Write-Host "FAILED: $t" -ForegroundColor Red; exit 1 }

# ---- checks ------------------------------------------------------------------------------------------------------
if (-not ($SkipUHDE -and $SkipFramerate -and $SkipInstall)) {   # these write under Program Files
    $admin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole(
        [Security.Principal.WindowsBuiltInRole]::Administrator)
    if (-not $admin) { Fail 'the game folder is under Program Files: run setup.bat (it asks for administrator), or an elevated PowerShell' }
}
if (-not (Test-Path (Join-Path $Sys 'Engine.dll'))) { Fail "no game at $GameDir (pass -GameDir)" }
if (-not (Test-Path (Join-Path $Sys 'hppoa.exe'))) { Fail "no hppoa.exe in ${Sys}: install the game first (step 1 of docs/SETUP.md)" }
Write-Host 'note: the game runs with your own hppoa.exe, from your retail disc; this setup neither provides nor changes it.'

# Python: the py launcher (py -3) or python on PATH. Run with $ErrorActionPreference relaxed so a non-zero exit or
# stderr output doesn't stop the script (Windows PowerShell turns a native command's stderr into errors).
$PyExe = $null; $PyArgs = @()
foreach ($c in @(@('py', '-3'), @('python'), @('python3'))) {
    $cmd = Get-Command $c[0] -CommandType Application -ErrorAction SilentlyContinue | Select-Object -First 1
    if (-not $cmd) { continue }
    $pre = @($c | Select-Object -Skip 1)
    $old = $ErrorActionPreference; $ErrorActionPreference = 'Continue'
    & $cmd.Source @pre -c 'import sys; sys.exit(0 if sys.version_info >= (3, 8) else 1)' 2>$null
    $ok = $LASTEXITCODE -eq 0; $ErrorActionPreference = $old
    if ($ok) { $PyExe = $cmd.Source; $PyArgs = $pre; break }
}
if (-not $PyExe) { Fail 'Python 3.8+ not found: install it (winget install Python.Python.3.12) and run this again' }
function Test-Py([string]$code) {   # exit code of a python one-liner, quietly
    $old = $ErrorActionPreference; $ErrorActionPreference = 'Continue'
    & $PyExe @PyArgs -c $code 2>$null | Out-Null
    $r = $LASTEXITCODE; $ErrorActionPreference = $old; return $r
}
function Invoke-Py {   # python with these arguments; stops the setup if it fails
    $old = $ErrorActionPreference; $ErrorActionPreference = 'Continue'
    & $PyExe @PyArgs @args
    $r = $LASTEXITCODE; $ErrorActionPreference = $old
    if ($r -ne 0) { Fail "python $($args -join ' ')" }
}
Write-Host "game:   $GameDir"
Write-Host "python: $PyExe $($PyArgs -join ' ')"

# ---- step 1: your hppoa.exe ---------------------------------------------------------------------------------------
# This project supports one executable: the verified retail hppoa.exe (docs/RETAIL.md). It is SafeDisc-protected and,
# on Windows 11, runs from the disc with nothing else installed. Some SafeDisc versions need secdrv.sys, which Windows
# 10/11 block; SafeDiscShim by RibShark (GPLv3) answers for it without a driver. It isn't needed for the verified exe,
# and setup doesn't install it (docs/SETUP.md step 1).
if (-not $SkipExeCheck) {
    Step 'Your hppoa.exe (SETUP.md step 1)'
    $exePath = Join-Path $Sys 'hppoa.exe'
    if ((Get-FileHash $exePath -Algorithm SHA256).Hash -eq $TestedExe) {
        Write-Host 'the verified retail hppoa.exe (docs/RETAIL.md). It runs from the disc on Windows 11: keep the disc in the drive.'
    } else {
        Write-Host 'This hppoa.exe is NOT the verified retail executable (docs/RETAIL.md). This project is only tested with that one,' -ForegroundColor Yellow
        Write-Host 'installed from the retail disc; run tools\verify_retail.ps1 to compare your install.' -ForegroundColor Yellow
        $exeText = [Text.Encoding]::ASCII.GetString([IO.File]::ReadAllBytes($exePath))
        $dll = Join-Path $env:SystemRoot $(if ([Environment]::Is64BitOperatingSystem) { 'SysWOW64\drvmgt.dll' } else { 'System32\drvmgt.dll' })
        $sdb = Test-Path "HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\AppCompatFlags\InstalledSDB\$SDShimGuid"
        if ($exeText.Contains('BoG_') -and -not ((Test-Path $dll) -and $sdb)) {
            Write-Host 'It is SafeDisc-protected. If the game says "Cannot locate the CD-ROM" with the disc in, see docs/SETUP.md step 1'
            Write-Host '(SafeDiscShim by RibShark).'
        }
    }
}

# ---- step 2: HP3-UHDE core files ---------------------------------------------------------------------------------
if (-not $SkipUHDE) {
    Step 'HP3-UHDE core files (SETUP.md step 2)'
    $u = $UHDE   # the files in this repository (thirdparty\hp3-uhde\README.md): no game executable among them
    foreach ($f in 'system\d3d8.dll', 'system\d3d9.dll', 'system\plugins\dxgi.asi', 'system\Default.ini.IniPatch', 'system\defuser.ini.IniPatch',
                   'MyDocuments\hppoa.ini.IniPatch', 'MyDocuments\User.ini.IniPatch') {
        if (-not (Test-Path (Join-Path $u $f))) { Fail "$f missing from $u (get the whole repository: docs/SETUP.md)" }
    }
    New-Item -ItemType Directory -Force -Path (Join-Path $Sys 'plugins') | Out-Null
    foreach ($f in 'd3d8.dll', 'd3d9.dll', 'plugins\dxgi.asi') { Copy-Item (Join-Path $u "system\$f") (Join-Path $Sys $f) -Force; Write-Host "copied $f" }

    # ini patches: the game's own files, backed up once
    $patches = @(@((Join-Path $Sys 'Default.ini'), 'system\Default.ini.IniPatch'), @((Join-Path $Sys 'defuser.ini'), 'system\defuser.ini.IniPatch'),
                 @((Join-Path $Docs 'hppoa.ini'), 'MyDocuments\hppoa.ini.IniPatch'), @((Join-Path $Docs 'User.ini'), 'MyDocuments\User.ini.IniPatch'))
    foreach ($p in $patches) {
        $ini = $p[0]
        if (-not (Test-Path $ini)) { Write-Host "skipped $ini (not there yet: start the game once, then run this again)" -ForegroundColor Yellow; continue }
        if (-not (Test-Path "$ini.bak")) { Copy-Item $ini "$ini.bak" }
        Invoke-Py (Join-Path $Repo 'tools\inipatch.py') $ini $ini (Join-Path $u $p[1])
        Write-Host "patched $ini"
    }
    if ($TestSave) {
        $saves = Join-Path $Docs 'Save'
        New-Item -ItemType Directory -Force -Path $saves | Out-Null
        Copy-Item (Join-Path $u 'MyDocuments\Save\Save500.usa') (Join-Path $saves 'Save500.usa') -Force; Write-Host 'copied Save500.usa (UHDE 100% save)'
    }
}

# ---- step 3: 60 fps cap ------------------------------------------------------------------------------------------
if (-not $SkipFramerate) {
    Step 'Framerate fix (SETUP.md step 3)'
    $eng = Join-Path $Sys 'Engine.dll'
    if (Test-Path "$eng.retail.bak") { Write-Host 'already patched (Engine.dll.retail.bak exists)' }
    else { Invoke-Py (Join-Path $Repo 'tools\framerate_patch.py') $eng }
}

# ---- build: HP3OpenCastle.asi (+ MapTracker, WindowFix) and HP3AP.utx --------------------------------------------
if (-not $SkipBuild) {
    Step 'Build the plugins and HP3AP.utx'
    if ((Test-Py 'import PIL') -ne 0) { Write-Host 'installing Pillow (for the textures)'; Invoke-Py -m pip install --user --quiet pillow }
    $llvm = Find-LLVM
    if (-not $llvm) {
        $a = Read-Host 'LLVM (clang, lld-link) is needed to build the plugins. Install it now with winget? [Y/n]'
        if ($a -notmatch '^[nN]') { winget install --id LLVM.LLVM -e --accept-package-agreements --accept-source-agreements }
        $llvm = Find-LLVM
        if (-not $llvm) { Fail 'LLVM not found (winget install LLVM.LLVM, or put its bin folder on PATH)' }
    }
    Write-Host "llvm:   $llvm"
    if (($env:Path -split ';') -notcontains $llvm) { $env:Path = "$llvm;$env:Path" }   # for tools\build_mods.py
    Invoke-Py (Join-Path $Repo 'tools\build_mods.py') $Build
    if (-not (Test-Path $APIcon)) { Fail "$APIcon missing (get the whole repository: docs/SETUP.md)" }
    Invoke-Py (Join-Path $Repo 'tools\build_hp3ap.py') $APIcon $GameDir (Join-Path $Build 'HP3AP.utx')
}

# ---- install -----------------------------------------------------------------------------------------------------
if (-not $SkipInstall) {
    Step 'Install'
    $plugins = Join-Path $Sys 'plugins'
    $oc = if ($DebugBuild) { Join-Path $Build 'HP3OpenCastle.asi' } else { Join-Path $Build 'release\HP3OpenCastle.asi' }
    if (-not (Test-Path $oc)) { Fail "$oc not built (run without -SkipBuild)" }
    Copy-Item $oc (Join-Path $plugins 'HP3OpenCastle.asi') -Force
    Write-Host ("installed HP3OpenCastle.asi (" + $(if ($DebugBuild) { 'debug' } else { 'release' }) + ')')
    Copy-Item (Join-Path $Build 'HP3AP.utx') (Join-Path $GameDir 'textures\HP3AP.utx') -Force; Write-Host 'installed HP3AP.utx'
    foreach ($m in @(@($MapTracker, 'HP3MapTracker.asi'), @($WindowFix, 'HP3WindowFix.asi'))) {
        if ($m[0]) { Copy-Item (Join-Path $Build $m[1]) (Join-Path $plugins $m[1]) -Force; Write-Host "installed $($m[1])" }
    }
}

Step 'Done'
Write-Host 'Next: the Archipelago side (apworld/README.md): put the apworld in custom_worlds, start "Harry Potter 3 PC Client".'
