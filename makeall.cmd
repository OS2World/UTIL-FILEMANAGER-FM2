cd dll
nmake
if errorlevel 1 cancel
cd ..
nmake
if errorlevel 1 cancel
for %d in (*.mak) do nmake /f %d
