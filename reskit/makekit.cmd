@echo off
:: $Id$
:: Make reskit.zip

echo on
zip -uj9 fm2rskt compile.cmd reskit.txt
cd ..\dll
zip -uj9 ..\reskit\fm2rskt fm3dll.str fm3str.h fm3res.rc fm3res.dlg fm3dlg.h fm3dll2.h fm3hlp.h internal\mkstr.exe
zip -u9 ..\reskit\fm2rskt icons\*
cd ..\reskit
