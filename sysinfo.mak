DEBUG = 0

!IF $(DEBUG)
LFLAGS = /NOI /PMTYPE:PM /ALIGN:2 /EXEPACK /CODEVIEW /M /BASE:0x10000 /STACK:65536 /NOD
!ELSE
LFLAGS = /NOI /PMTYPE:PM /ALIGN:2 /EXEPACK:2 /M /BASE:0x10000 /STACK:65536 /NOD /RUNFROMVDM /PACKC /PACKD
!ENDIF

.SUFFIXES:

.SUFFIXES: .c .rc .ipf

ALL: sysinfo.EXE \
     sysinfo.res

sysinfo.res: sysinfo.rc

sysinfo.obj: sysinfo.c

sysinfo.exe:  \
  sysinfo.res \
  sysinfo.OBJ
   @REM @<<sysinfo.@0
     $(LFLAGS)+
     sysinfo.OBJ
     sysinfo.exe
     nul.map
     dde4mbso.lib dll\fm3dll.lib os2386.lib
     sysinfo.def;
<<
   LINK386.EXE @sysinfo.@0
   RC -x2 sysinfo.RES sysinfo.exe

{.}.rc.res:
   RC -r .\$*.RC

{.}.c.obj:
!IF $(DEBUG)
     ICC.EXE /Kb /Ti+ /W3 /Sm /Sp4 /Ss /C /Mp /Gm+ /Gs- /G3 /O- /Q+ /Gd+ .\$*.c
!ELSE
     ICC.EXE /Gf+ /Kb /W3 /Sm /Sp4 /Ss /C /Mp /Gm+ /Gs- /O- /Q+ /G3 /Gt- /Gd+ .\$*.c
!ENDIF

