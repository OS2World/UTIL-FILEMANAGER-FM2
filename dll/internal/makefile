# makefile - build makestr.exe
# $Id$

BASE=mkstr

!include ..\..\makefile_pre.mk

CFLAGS = /G5 /Gm+ /Gs- /Gt- /Mp /O+ /Q+ /Sm /Ss /W3
LFLAGS = /EXEPACK:2 /MAP /PMTYPE:VIO

all: $(BASE).exe

$(BASE).obj: $(BASE).c ..\version.h ..\fm3str.h

$(BASE).exe: $(BASE).obj
  @REM @<<$(BASE).lrf
  $(LFLAGS)
  $(BASE).obj
  os2386.lib
<<
  type $(BASE).lrf
  $(LINK) @$(BASE).lrf

clean:
  -del $(BASE).exe
  -del $(BASE).lrf
  -del $(BASE).map
  -del $(BASE).obj

# The end
