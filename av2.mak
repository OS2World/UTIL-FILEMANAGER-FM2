DEBUG = 0

!IF $(DEBUG)
LFLAGS = /NOI /PMTYPE:PM /ALIGN:2 /EXEPACK /CODEVIEW /M /BASE:0x10000 /STACK:65536 /NOD
!ELSE
LFLAGS = /NOI /PMTYPE:PM /ALIGN:2 /EXEPACK:2 /M /BASE:0x10000 /STACK:65536 /NOD /RUNFROMVDM /PACKC /PACKD
!ENDIF

.SUFFIXES:

.SUFFIXES: .c .rc .ipf

ALL: av2.EXE \
     av2.res

av2.res: av2.rc

av2.obj: av2.c \
    dll\version.h

av2.exe:  \
  av2.res \
  av2.OBJ
   @REM @<<av2.@0
     $(LFLAGS)+
     av2.OBJ
     av2.exe
     nul.map
     dde4mbso.lib dll\fm3dll.lib os2386.lib
     av2.def;
<<
   LINK386.EXE @av2.@0
   RC -x2 av2.RES av2.exe

{.}.rc.res:
   RC -r .\$*.RC

{.}.c.obj:
!IF $(DEBUG)
     ICC.EXE /Kb /Ti+ /W3 /Sm /Sp4 /Ss /C /Mp /Gm+ /Gs- /G3 /O- /Q+ /Gd+ .\$*.c
!ELSE
     ICC.EXE /Gf+ /Kb /W3 /Sm /Sp4 /Ss /C /Mp /Gm+ /Gs- /O- /Q+ /G3 /Gt- /Gd+ .\$*.c
!ENDIF

