DEBUG = 0

!IF $(DEBUG)
LFLAGS = /NOI /PMTYPE:PM /ALIGN:2 /EXEPACK /CODEVIEW /M /BASE:0x10000 /STACK:65536 /NOD
!ELSE
LFLAGS = /NOI /PMTYPE:PM /ALIGN:2 /EXEPACK:2 /M /BASE:0x10000 /STACK:65536 /NOD /RUNFROMVDM /PACKC /PACKD
!ENDIF

.SUFFIXES:

.SUFFIXES: .c .rc .ipf

ALL: vdir.EXE \
     vdir.res

vdir.res: vdir.rc

vdir.obj: vdir.c

vdir.exe:  \
  vdir.res \
  vdir.OBJ
   @REM @<<vdir.@0
     $(LFLAGS)+
     vdir.OBJ
     vdir.exe
     nul.map
     dde4mbso.lib dll\fm3dll.lib os2386.lib
     vdir.def;
<<
   LINK386.EXE @vdir.@0
   RC -x2 vdir.RES vdir.exe

{.}.rc.res:
   RC -r .\$*.RC

{.}.c.obj:
!IF $(DEBUG)
     ICC.EXE /Kb /Ti+ /W3 /Sm /Sp4 /Ss /C /Mp /Gm+ /Gs- /G3 /O- /Q+ /Gd+ .\$*.c
!ELSE
     ICC.EXE /Gf+ /Kb /W3 /Sm /Sp4 /Ss /C /Mp /Gm+ /Gs- /O- /Q+ /G3 /Gt- /Gd+ .\$*.c
!ENDIF

