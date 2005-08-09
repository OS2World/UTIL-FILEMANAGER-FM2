# makefile - build all fm/2 components
# $Id$

# 24 May 05 SHL Add clean and cleanobj target
# 16 Jun 05 SHL Workaround makeflags wierdness
# 18 Jul 05 SHL Add bitmap dependencies
# 20 Jul 05 SHL Add makeres support

# Environment:

#   DEBUG	0 = release build, 1 = debug build

BASE = fm3

!include makefile_pre.mk

all: dll $(BASE) mak

# Only update resources
res:
  @echo Updating resources only
  $(MAKE) /nologo /$(MAKEFLAGS) MAKERES=1

# make DLL components

dll:
  cd dll
  $(MAKE) /nologo /$(MAKEFLAGS)
  cd ..

$(BASE): $(BASE).exe $(BASE).res

$(BASE).res: $(BASE).rc  $(BASE).h bitmaps\*.bmp

$(BASE).obj: $(BASE).c $(BASE).h dll\version.h

# make EXE compenents

mak: *.mak
  !$(MAKE) /NOLOGO /$(MAKEFLAGS) /f $?

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

!include makefile_post.mk

# The end
