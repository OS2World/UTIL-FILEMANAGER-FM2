# makefile

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

MAK: *.MAK
  !$(MAKE) /NOLOGO /f $?


!INCLUDE makefile_post.mk

# The end
