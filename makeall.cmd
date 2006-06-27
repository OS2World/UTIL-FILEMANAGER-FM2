:: $Id$

:: %1 only useful for standard target names
cd dll
nmake %1
if errorlevel 1 cancel
cd ..
nmake %1
if errorlevel 1 cancel
for %X in (*.mak) do nmake /f %X %1
