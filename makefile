# makefile - build fm/2
# $Id$

# 21 Nov 03 SHL Comments
# 24 May 05 SHL Add clean and cleanobj target

# Environment:

#   DEBUG	0 = release build, 1 = debug build

BASE = fm3

!INCLUDE makefile_pre.mk

ALL: DLL $(BASE) MAK

DLL:
  cd dll
  $(MAKE) /nologo $(MAKEFLAGS)
  cd ..

$(BASE): $(BASE).EXE \
     $(BASE).res

$(BASE).res: $(BASE).rc \
     $(BASE).h

$(BASE).obj: $(BASE).c \
     $(BASE).h dll\version.h

MAK: *.mak
  !$(MAKE) /NOLOGO /f $?

cleanobj:
  cd dll
  $(MAKE) /nologo $(MAKEFLAGS) cleanobj
  cd ..
  -del *.obj

clean:
  cd dll
  $(MAKE) /nologo $(MAKEFLAGS) clean
  cd ..
  -del *.exe
  -del *.map
  -del *.obj
  -del *.res
  -del fm3res.str

!INCLUDE makefile_post.mk

# The end
