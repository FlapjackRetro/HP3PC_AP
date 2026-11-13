<#
.SYNOPSIS
  Compares an HP3 install with the retail copy this project is tested against (data/retail_manifest.csv,
  documented in docs/RETAIL.md). Read-only: it changes nothing.

.PARAMETER GameDir   The game folder (default: C:\Program Files (x86)\EA GAMES\Harry Potter and the Prisoner of Azkaban(TM)).
.PARAMETER All       List every matching file too, not only the problems.
#>
param(
    [string]$GameDir = "${env:ProgramFiles(x86)}\EA GAMES\Harry Potter and the Prisoner of Azkaban(TM)",
    [switch]$All
)
$ErrorActionPreference = 'Stop'
$manifest = Join-Path $PSScriptRoot '..\data\retail_manifest.csv'
if (-not (Test-Path $manifest)) { Write-Host "no $manifest" -ForegroundColor Red; exit 1 }
if (-not (Test-Path (Join-Path $GameDir 'system\hppoa.exe'))) { Write-Host "no game at $GameDir (pass -GameDir)" -ForegroundColor Red; exit 1 }

# Engine.dll after tools/framerate_patch.py (docs/RETAIL.md)
$patched = @{
    'e4b17d38f0860951385a0e8023477a0daa827c46648fe1d26c464ab383a1c826' = 'framerate-patched (60 fps cap)'
    'c1dfe8153d8ffc02e700f9d38be860b7ad62aebf94b01fa993f3f9658228e735' = 'framerate-patched (60 fps cap + fps counter)'
}
# files the mod depends on (docs/RETAIL.md): a mismatch here matters more
$key = @('system\hppoa.exe', 'system\Core.dll', 'system\Engine.dll', 'system\D3DDrv.dll', 'system\Core.u', 'system\Engine.u',
         'system\kwGame.u', 'system\GUI.u', 'system\HGame.u', 'textures\HP3_Menu.utx', 'textures\SpellShapes.utx',
         'textures\HP_FX.utx', 'Animations\HPCharacters.ukx', 'Animations\HProps.ukx', 'StaticMeshes\HP3_Gameplay_SM.usx',
         'StaticMeshes\HP3_InteriorAccents_SM.usx', 'StaticMeshes\HP3_InterArch_SM.usx', 'sounds\soundeffects.uax')
# files setup adds, or the game and the plugins write
$added = '^(system\\(d3d8\.dll|d3d9\.dll|plugins\\.*|.*\.bak|HP3_.*\.(log|ini|txt)|.*SafeDiscShim.*\.log|.*\.log)|textures\\HP3AP\.utx)$'
# files the game or setup.bat change: different from retail on purpose
$changed = '^system\\(Default\.ini|defuser\.ini)$'

$rows = Import-Csv $manifest
$ok = 0; $bad = @(); $keyBad = @(); $missing = @(); $notes = @()
foreach ($r in $rows) {
    $f = Join-Path $GameDir $r.Path
    $isKey = $key -contains $r.Path
    if (-not (Test-Path -LiteralPath $f)) { $missing += $r.Path; continue }
    $h = (Get-FileHash -LiteralPath $f -Algorithm SHA256).Hash.ToLower()
    if ($h -eq $r.SHA256) { $ok++; if ($All) { Write-Host "ok        $($r.Path)" }; continue }
    if ($r.Path -eq 'system\Engine.dll' -and $patched.ContainsKey($h)) {
        $notes += "system\Engine.dll: $($patched[$h])"
        $bak = Join-Path $GameDir 'system\Engine.dll.retail.bak'
        if ((Test-Path $bak) -and (Get-FileHash $bak -Algorithm SHA256).Hash.ToLower() -eq $r.SHA256) { $notes += 'system\Engine.dll.retail.bak: retail' }
        $ok++; continue
    }
    if ($r.Path -match $changed) { $notes += "$($r.Path): changed (setup's ini patch, or the game's own settings)"; $ok++; continue }
    if ($isKey) { $keyBad += "$($r.Path)  ($((Get-Item -LiteralPath $f).Length) bytes, sha256 $h)" } else { $bad += $r.Path }
}
$known = @{}; foreach ($r in $rows) { $known[$r.Path.ToLower()] = 1 }
$extra = Get-ChildItem -LiteralPath $GameDir -Recurse -File | ForEach-Object { $_.FullName.Substring($GameDir.TrimEnd([char]92, [char]47).Length + 1).Replace([char]47, [char]92) } |
    Where-Object { -not $known.ContainsKey($_.ToLower()) }
$fromSetup = @($extra | Where-Object { $_ -match $added }); $unknown = @($extra | Where-Object { $_ -notmatch $added })

Write-Host "game:  $GameDir"
Write-Host "retail files matching: $ok of $($rows.Count)"
foreach ($n in $notes) { Write-Host "note      $n" -ForegroundColor Cyan }
foreach ($k in $keyBad) { Write-Host "DIFFERENT $k   <- the mod depends on this file" -ForegroundColor Red }
foreach ($b in $bad) { Write-Host "different $b" -ForegroundColor Yellow }
foreach ($m in $missing) { Write-Host "missing   $m" -ForegroundColor Yellow }
foreach ($a in $fromSetup) { Write-Host "added     $a   (setup / plugins / logs)" -ForegroundColor DarkGray }
foreach ($u in $unknown) { Write-Host "extra     $u   (not retail, not from setup)" -ForegroundColor Yellow }
Write-Host ''
if ($keyBad.Count) { Write-Host 'Your copy differs from the tested one in files the mod depends on: it may not work. Please mention these when reporting problems.' -ForegroundColor Red; exit 2 }
elseif ($bad.Count -or $missing.Count) { Write-Host 'The files the mod depends on match. Other files differ: probably fine, but untested.' -ForegroundColor Yellow; exit 0 }
else { Write-Host 'Your copy matches the tested retail copy.' -ForegroundColor Green; exit 0 }
