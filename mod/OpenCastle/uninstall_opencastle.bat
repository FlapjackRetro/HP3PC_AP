@echo off
rem Run as administrator. Removes HP3OpenCastle.asi and HP3AP.utx from the game.
set "R=C:\Program Files (x86)\EA GAMES\Harry Potter and the Prisoner of Azkaban(TM)"
set "G=%R%\system\plugins"
del "%G%\HP3OpenCastle.asi"
if exist "%G%\HP3OpenCastle.asi" goto fail
if exist "%R%\textures\HP3AP.utx" del "%R%\textures\HP3AP.utx"
if exist "%R%\textures\HP3AP.utx" goto fail
echo Removed HP3OpenCastle.asi and HP3AP.utx
goto end
:fail
echo FAILED - right-click the .bat and choose Run as administrator
:end
pause
