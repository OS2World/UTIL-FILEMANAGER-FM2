DEBUG = 0

!IF $(DEBUG)
LFLAGS = /NOI /PMTYPE:PM /ALIGN:2 /EXEPACK /CODEVIEW /M /BASE:0x10000 /STACK:65536 /NOD
!ELSE
LFLAGS = /NOI /PMTYPE:PM /ALIGN:2 /EXEPACK:2 /M /BASE:0x10000 /STACK:65536 /NOD /RUNFROMVDM /PACKC /PACKD
!ENDIF

.SUFFIXES: .c .rc .ipf

ALL: fm4.EXE \
     fm4.res

fm4.res: fm4.rc \
     fm4.h

fm4.obj: fm4.c \
     fm4.h dll\version.h

fm4.exe:  \
  fm4.res \
  fm4.OBJ
   @REM @<<fm4.@0
     $(LFLAGS)+
     fm4.OBJ
     fm4.exe
     nul.map
     dde4mbso.lib dll\fm3dll.lib os2386.lib
     fm4.def;
<<
   LINK386.EXE @fm4.@0
   RC -x2 fm4.RES fm4.exe

{.}.rc.res:
   RC -r .\$*.RC

{.}.c.obj:
!IF $(DEBUG)
     ICC.EXE /Kb /Ti+ /W3 /Sm /Sp4 /Ss /C /Mp /Gm+ /Gs- /G3 /O- /Q+ /Gd+ .\$*.c
!ELSE
     ICC.EXE /Gf+ /Kb /W3 /Sm /Sp4 /Ss /C /Mp /Gm+ /Gs- /O+ /Q+ /G3 /Gt- /Gd+ .\$*.c
!ENDIF

