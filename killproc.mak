DEBUG = 0

!IF $(DEBUG)
LFLAGS = /NOI /PMTYPE:PM /ALIGN:2 /EXEPACK /CODEVIEW /M /BASE:0x10000 /STACK:65536 /NOD
!ELSE
LFLAGS = /NOI /PMTYPE:PM /ALIGN:2 /EXEPACK:2 /M /BASE:0x10000 /STACK:65536 /NOD /RUNFROMVDM /PACKC /PACKD
!ENDIF

.SUFFIXES:

.SUFFIXES: .c .rc .ipf

ALL: killproc.EXE \
     killproc.res

killproc.res: killproc.rc

killproc.obj: killproc.c

killproc.exe:  \
  killproc.res \
  killproc.OBJ
   @REM @<<killproc.@0
     $(LFLAGS)+
     killproc.OBJ
     killproc.exe
     nul.map
     dde4mbso.lib dll\fm3dll.lib os2386.lib
     killproc.def;
<<
   LINK386.EXE @killproc.@0
   RC -x2 killproc.RES killproc.exe

{.}.rc.res:
   RC -r .\$*.RC

{.}.c.obj:
!IF $(DEBUG)
     ICC.EXE /Kb /Ti+ /W3 /Sm /Sp4 /Ss /C /Mp /Gm+ /Gs- /G3 /O- /Q+ /Gd+ .\$*.c
!ELSE
     ICC.EXE /Gf+ /Kb /W3 /Sm /Sp4 /Ss /C /Mp /Gm+ /Gs- /O- /Q+ /G3 /Gt- /Gd+ .\$*.c
!ENDIF

