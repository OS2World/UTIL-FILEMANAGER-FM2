::icc /G3 /O+ /W3 /Kb /Gs+ /Ss /Sm /B"/RUNFROMVDM" /B"/STACK:65535" /B"/BASE:0x10000" /B"/EXEPACK:2" /B"/ALIGN:4" %1
set CFLAGS = /G5 /Gm+ /Gs- /Gt- /Mp /O+ /Q+ /Sm /Ss /W3 /C
set LFLAGS = /EXEPACK:2 /MAP /PMTYPE:VIO
icc %CFLAGS% /B"%LFLAGS%" %1

