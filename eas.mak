DEBUG = 0

!IF $(DEBUG)
LFLAGS = /NOI /PMTYPE:PM /ALIGN:2 /EXEPACK /CODEVIEW /M /BASE:0x10000 /STACK:65536 /NOD
!ELSE
LFLAGS = /NOI /PMTYPE:PM /ALIGN:2 /EXEPACK:2 /M /BASE:0x10000 /STACK:65536 /NOD /RUNFROMVDM /PACKC /PACKD
!ENDIF

.SUFFIXES:

.SUFFIXES: .c .rc .ipf

ALL: eas.EXE \
     eas.res

eas.res: eas.rc

eas.obj: eas.c

eas.exe:  \
  eas.res \
  eas.OBJ
   @REM @<<eas.@0
     $(LFLAGS)+
     eas.OBJ
     eas.exe
     nul.map
     dde4mbso.lib dll\fm3dll.lib os2386.lib
     eas.def;
<<
   LINK386.EXE @eas.@0
   RC -x2 eas.RES eas.exe

{.}.rc.res:
   RC -r .\$*.RC

{.}.c.obj:
!IF $(DEBUG)
     ICC.EXE /Kb /Ti+ /W3 /Sm /Sp4 /Ss /C /Mp /Gm+ /Gs- /G3 /O- /Q+ /Gd+ .\$*.c
!ELSE
     ICC.EXE /Gf+ /Kb /W3 /Sm /Sp4 /Ss /C /Mp /Gm+ /Gs- /O- /Q+ /G3 /Gt- /Gd+ .\$*.c
!ENDIF

