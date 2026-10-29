@echo off
rem Run as administrator. Removes HP3MapTracker.asi (title-bar coordinates, HP3_MapLog.txt, mark key) from the game.
rem Open Castle does not need it.
set "G=C:\Program Files (x86)\EA GAMES\Harry Potter and the Prisoner of Azkaban(TM)\system\plugins"
if exist "%G%\HP3MapTracker.asi" del "%G%\HP3MapTracker.asi"
if exist "%G%\HP3MapTracker.asi" goto fail
echo Removed HP3MapTracker.asi
goto end
:fail
echo FAILED - right-click the .bat and choose Run as administrator
:end
pause
