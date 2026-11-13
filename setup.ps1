<#
.SYNOPSIS
  HP3 PC Archipelago setup: SafeDiscShim (offered), steps 2 and 3 of docs/SETUP.md (HP3-UHDE core files, not its executable; 60 fps cap), then builds and
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
.PARAMETER ForceSafeDiscShim  Offer SafeDiscShim even for the tested retail hppoa.exe (which doesn't need it).
.PARAMETER SkipSafeDiscShim / SkipUHDE / SkipFramerate / SkipBuild / SkipInstall   Leave that step out.
#>
param(
    [string]$GameDir = "${env:ProgramFiles(x86)}\EA GAMES\Harry Potter and the Prisoner of Azkaban(TM)",
    [switch]$DebugBuild, [switch]$MapTracker, [switch]$WindowFix, [switch]$TestSave,
    [switch]$SkipSafeDiscShim, [switch]$ForceSafeDiscShim, [switch]$SkipUHDE, [switch]$SkipFramerate, [switch]$SkipBuild, [switch]$SkipInstall
)
$ErrorActionPreference = 'Stop'
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
$Repo = $PSScriptRoot
$Sys = Join-Path $GameDir 'system'
$MyDocs = [Environment]::GetFolderPath('MyDocuments'); if (-not $MyDocs) { $MyDocs = $HOME }
$Docs = Join-Path $MyDocs 'Harry Potter and the Prisoner of Azkaban'
$Build = Join-Path $Repo 'build'
$Tmp = Join-Path ([IO.Path]::GetTempPath()) 'hp3pc_ap_setup'
# pinned sources (the versions this was tested with)
$UHDE = Join-Path $Repo 'thirdparty\hp3-uhde'   # HP3-UHDE's core files (Vawlpe), included in this repository
$SDShim = '(SafeDiscShim installer; link removed)'
$SDShimSha = 'B4C355EC63047A09C20C286C083FCAD1031D47A2C17EAC0DE12B96D377EC369F'
$TestedExe = '1ac9c1e37b1cd8d9c581558be8c8ebb607ef0853b66c698fa3b881dcb9b54b0d'   # docs/RETAIL.md: runs without SafeDiscShim on Windows 11
$SDShimGuid = '{001827e2-fa73-4907-81de-5596bc8e3d37}'   # its compatibility database (SafeDiscShim.sdb)
$APIcon = '(Archipelago icon; link removed)'

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
function Get-File($url, $dest) {
    New-Item -ItemType Directory -Force -Path (Split-Path $dest) | Out-Null
    Invoke-WebRequest -UseBasicParsing -Uri $url -OutFile $dest
}

# ---- checks ------------------------------------------------------------------------------------------------------
if (-not ($SkipSafeDiscShim -and $SkipUHDE -and $SkipFramerate -and $SkipInstall)) {   # these write under Program Files / Windows
    $admin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole(
        [Security.Principal.WindowsBuiltInRole]::Administrator)
    if (-not $admin) { Fail 'the game folder is under Program Files: run setup.bat (it asks for administrator), or an elevated PowerShell' }
}
if (-not (Test-Path (Join-Path $Sys 'Engine.dll'))) { Fail "no game at $GameDir (pass -GameDir)" }
if (-not (Test-Path (Join-Path $Sys 'hppoa.exe'))) { Fail "no hppoa.exe in ${Sys}: install the game first (step 1 of docs/SETUP.md)" }
Write-Host 'note: the game runs with your own hppoa.exe; this setup neither provides nor changes it. The retail disc''s'
Write-Host '      SafeDisc protection may need SafeDiscShim on Windows 10/11 (offered below); keep the disc in the drive.'

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

# ---- step 1b: SafeDiscShim -----------------------------------------------------------------------------------------
# The retail hppoa.exe is SafeDisc-protected; some SafeDisc versions need secdrv.sys, which Windows 10/11 block. SafeDiscShim (RibShark,
# GPLv3) answers those requests itself: no driver, no change to the executable, and the original disc is still needed.
# It installs drvmgt.dll into SysWOW64 and a compatibility database (sdbinst) that loads it into SafeDisc games.
# It only hooks process creation and the secdrv requests, so Ultimate ASI Loader and our plugins load as before.
if (-not $SkipSafeDiscShim) {
    Step 'SafeDiscShim (SETUP.md step 1)'
    $dll = Join-Path $env:SystemRoot $(if ([Environment]::Is64BitOperatingSystem) { 'SysWOW64\drvmgt.dll' } else { 'System32\drvmgt.dll' })
    $sdb = Test-Path "HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\AppCompatFlags\InstalledSDB\$SDShimGuid"
    $exeText = [Text.Encoding]::ASCII.GetString([IO.File]::ReadAllBytes((Join-Path $Sys 'hppoa.exe')))
    if ((Test-Path $dll) -and $sdb) { Write-Host 'already installed' }
    elseif (-not $exeText.Contains('BoG_')) { Write-Host 'not needed: this hppoa.exe has no SafeDisc' }
    elseif (-not $ForceSafeDiscShim -and (Get-FileHash (Join-Path $Sys 'hppoa.exe') -Algorithm SHA256).Hash -eq $TestedExe) {
        Write-Host 'not needed: this is the tested retail hppoa.exe (docs/RETAIL.md), which runs on Windows 11 without it.'
        Write-Host '  Keep the disc in the drive. If the game says "Cannot locate the CD-ROM" with the disc in, run setup with -ForceSafeDiscShim.'
    }
    else {
        Write-Host 'Your hppoa.exe is SafeDisc-protected. Windows 10/11 block the driver SafeDisc may ask for (secdrv.sys).'
        Write-Host 'SafeDiscShim answers for it, with no driver; the original disc is still required.'
        Write-Host '  - The game reached the main menu with the disc in (SETUP.md step 1.4)? You don''t need it: answer n.'
        Write-Host '  - It said "Cannot locate the CD-ROM" with the disc in, or you haven''t tried yet? Answer Y.'
        Write-Host 'SafeDiscShim by RibShark (v0.9.0, GPLv3)'
        $a = Read-Host 'Install SafeDiscShim now? [Y/n]'
        if ($a -match '^(n|no)$') { Write-Host 'skipped. If the game later says "Cannot locate the CD-ROM" with the disc in, run setup again and answer Y.' -ForegroundColor Yellow }
        else {
            $exe = Join-Path $Tmp 'SafeDiscShim_Setup_0.9.0.exe'
            Get-File $SDShim $exe
            if ((Get-FileHash -Algorithm SHA256 $exe).Hash -ne $SDShimSha) { Fail "$exe doesn't match the pinned SafeDiscShim 0.9.0 installer" }
            Write-Host 'running SafeDiscShim_Setup_0.9.0.exe'
            $p = Start-Process -FilePath $exe -ArgumentList '/VERYSILENT', '/SUPPRESSMSGBOXES', '/NORESTART' -Wait -PassThru
            if ($p.ExitCode -ne 0) { Fail "SafeDiscShim installer exited with $($p.ExitCode)" }
            if (-not (Test-Path $dll)) { Fail "SafeDiscShim installed, but $dll isn't there" }
            Write-Host 'installed SafeDiscShim'
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
    $icon = Join-Path $Tmp 'icon.png'
    Get-File $APIcon $icon
    Invoke-Py (Join-Path $Repo 'tools\build_hp3ap.py') $icon $GameDir (Join-Path $Build 'HP3AP.utx')
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
