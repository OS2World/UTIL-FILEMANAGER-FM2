zip -uj9 fm2rskt compile.cmd
cd ..\dll
zip -uj9 ..\reskit\fm2rskt fm3dll.str fm3str.h fm3res.rc fm3res.dlg fm3dlg.h fm3dll2.h internal\mkstr.exe
zip -u9 ..\reskit\fm2rskt icons\*
cd ..\reskit
