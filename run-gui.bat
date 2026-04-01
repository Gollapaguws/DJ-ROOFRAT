@echo off
echo ====================================
echo Launching DJ-ROOFRAT Graphics Mode
echo ====================================
echo.
echo The graphics window will open in a moment...
echo Look for a window titled "DJ-ROOFRAT"
echo.
echo Press ESC in the graphics window to exit
echo Or close this console window to terminate
echo.
pause
start "" "build-vs\Debug\DJ-ROOFRAT.exe" --no-audio
