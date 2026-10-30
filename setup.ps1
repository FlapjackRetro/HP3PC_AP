<#
.SYNOPSIS
  HP3 PC Archipelago setup: steps 2 and 3 of docs/SETUP.md (HP3-UHDE core files, 60 fps cap), then builds and
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
.PARAMETER SkipUHDE / SkipFramerate / SkipBuild / SkipInstall   Leave that step out.
#>
param(
    [string]$GameDir = "${env:ProgramFiles(x86)}\EA GAMES\Harry Potter and the Prisoner of Azkaban(TM)",
    [switch]$DebugBuild, [switch]$MapTracker, [switch]$WindowFix, [switch]$TestSave,
    [switch]$SkipUHDE, [switch]$SkipFramerate, [switch]$SkipBuild, [switch]$SkipInstall
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
$UHDE = 'https://raw.githubusercontent.com/Vawlpe/HP3-UHDE/05906d8a0e71d76d676b2b15dd09620a70e83be1'
$APIcon = 'https://raw.githubusercontent.com/ArchipelagoMW/Archipelago/0.6.8/data/icon.png'

function Step($t) { Write-Host ''; Write-Host "== $t" -ForegroundColor Cyan }
function Fail($t) { Write-Host "FAILED: $t" -ForegroundColor Red; exit 1 }
function Get-File($url, $dest) {
    New-Item -ItemType Directory -Force -Path (Split-Path $dest) | Out-Null
    Invoke-WebRequest -UseBasicParsing -Uri $url -OutFile $dest
}

# ---- checks ------------------------------------------------------------------------------------------------------
if (-not ($SkipUHDE -and $SkipFramerate -and $SkipInstall)) {   # these write under Program Files
    $admin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole(
        [Security.Principal.WindowsBuiltInRole]::Administrator)
    if (-not $admin) { Fail 'the game folder is under Program Files: run setup.bat (it asks for administrator), or an elevated PowerShell' }
}
if (-not (Test-Path (Join-Path $Sys 'Engine.dll'))) { Fail "no game at $GameDir (pass -GameDir)" }

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

# ---- step 2: HP3-UHDE core files ---------------------------------------------------------------------------------
if (-not $SkipUHDE) {
    Step 'HP3-UHDE core files (SETUP.md step 2)'
    $u = Join-Path $Tmp 'uhde'
    foreach ($f in 'hppoa.exe', 'd3d8.dll', 'd3d9.dll', 'plugins/dxgi.asi', 'Default.ini.IniPatch', 'defuser.ini.IniPatch') {
        Get-File "$UHDE/Mod/Game/system/$f" (Join-Path $u $f)
    }
    $md = 'Mod/System/MyDocuments/Harry%20Potter%20and%20the%20Prisoner%20of%20Azkaban'
    Get-File "$UHDE/$md/hppoa.ini.IniPatch" (Join-Path $u 'hppoa.ini.IniPatch')
    Get-File "$UHDE/$md/User.ini.IniPatch" (Join-Path $u 'User.ini.IniPatch')
    if ($TestSave) { Get-File "$UHDE/$md/Save/Save500.usa" (Join-Path $u 'Save500.usa') }

    $exe = Join-Path $Sys 'hppoa.exe'
    if (-not (Test-Path "$exe.retail.bak")) { Copy-Item $exe "$exe.retail.bak"; Write-Host 'backed up the retail hppoa.exe (hppoa.exe.retail.bak)' }
    New-Item -ItemType Directory -Force -Path (Join-Path $Sys 'plugins') | Out-Null
    foreach ($f in 'hppoa.exe', 'd3d8.dll', 'd3d9.dll', 'plugins\dxgi.asi') { Copy-Item (Join-Path $u $f) (Join-Path $Sys $f) -Force; Write-Host "copied $f" }

    # ini patches: the game's own files, backed up once
    $patches = @(@((Join-Path $Sys 'Default.ini'), 'Default.ini.IniPatch'), @((Join-Path $Sys 'defuser.ini'), 'defuser.ini.IniPatch'),
                 @((Join-Path $Docs 'hppoa.ini'), 'hppoa.ini.IniPatch'), @((Join-Path $Docs 'User.ini'), 'User.ini.IniPatch'))
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
        Copy-Item (Join-Path $u 'Save500.usa') (Join-Path $saves 'Save500.usa') -Force; Write-Host 'copied Save500.usa (UHDE 100% save)'
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
    $clang = Get-Command clang -ErrorAction SilentlyContinue
    if (-not $clang -and -not (Test-Path "$env:ProgramFiles\LLVM\bin\clang.exe")) {
        $a = Read-Host 'LLVM (clang, lld-link) is needed to build the plugins. Install it now with winget? [Y/n]'
        if ($a -notmatch '^[nN]') { winget install --id LLVM.LLVM -e --accept-package-agreements --accept-source-agreements }
        if (-not (Test-Path "$env:ProgramFiles\LLVM\bin\clang.exe")) { Fail 'LLVM not installed (winget install LLVM.LLVM)' }
    }
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
