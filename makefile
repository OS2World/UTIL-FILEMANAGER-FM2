# makefile - build fm/2
# $Id$

# Revisions:	21 Nov 03 SHL - Comments

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

!INCLUDE makefile_post.mk

# The end
