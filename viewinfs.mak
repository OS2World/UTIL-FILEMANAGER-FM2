DEBUG = 0

!IF $(DEBUG)
LFLAGS = /NOI /PMTYPE:PM /ALIGN:2 /EXEPACK /CODEVIEW /M /BASE:0x10000 /STACK:65536 /NOD
!ELSE
LFLAGS = /NOI /PMTYPE:PM /ALIGN:2 /EXEPACK:2 /M /BASE:0x10000 /STACK:65536 /NOD /RUNFROMVDM /PACKC /PACKD
!ENDIF

.SUFFIXES:

.SUFFIXES: .c .rc .ipf

ALL: viewinfs.EXE \
     viewinfs.res

viewinfs.res: viewinfs.rc

viewinfs.obj: viewinfs.c

viewinfs.exe:  \
  viewinfs.res \
  viewinfs.OBJ
   @REM @<<viewinfs.@0
     $(LFLAGS)+
     viewinfs.OBJ
     viewinfs.exe
     nul.map
     dde4mbso.lib dll\fm3dll.lib os2386.lib
     viewinfs.def;
<<
   LINK386.EXE @viewinfs.@0
   RC -x2 viewinfs.RES viewinfs.exe

{.}.rc.res:
   RC -r .\$*.RC

{.}.c.obj:
!IF $(DEBUG)
     ICC.EXE /Kb /Ti+ /W3 /Sm /Sp4 /Ss /C /Mp /Gm+ /Gs- /G3 /O- /Q+ /Gd+ .\$*.c
!ELSE
     ICC.EXE /Gf+ /Kb /W3 /Sm /Sp4 /Ss /C /Mp /Gm+ /Gs- /O- /Q+ /G3 /Gt- /Gd+ .\$*.c
!ENDIF

