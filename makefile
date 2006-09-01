# makefile - build all fm/2 components
# $Id$

# Copyright (c) 1993-98 M. Kimes
# Copyright (c) 2002, 2006 Steven H. Levine

# 24 May 05 SHL Add clean and cleanobj target
# 16 Jun 05 SHL Workaround makeflags wierdness
# 18 Jul 05 SHL Add bitmap dependencies
# 20 Jul 05 SHL Add makeres support
# 16 Apr 06 SHL Add lxlite target
# 31 Jul 06 SHL Tweak dependencies
# 26 Aug 06 SHL Add rest of lxlite support

# Environment:

# DEBUG	0 = release build, 1 = debug build

BASE = fm3

!include makefile_pre.mk

all: dll $(BASE) allexe

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

$(BASE).res: $(BASE).rc  $(BASE).h icons\$(BASE).ico bitmaps\*.bmp

$(BASE).obj: $(BASE).c $(BASE).h dll\version.h

# make EXE compenents

allexe: *.mak
  !$(MAKE) /NOLOGO /f $? /$(MAKEFLAGS)

# makefile_post.mk contains lxlite target for $(BASE).exe
# Apply to each *.mak for other exes
lxlite:: *.mak
  !$(MAKE) /NOLOGO /f $? /$(MAKEFLAGS) lxlite

# Apply to dlls
lxlite::
  cd dll
  $(MAKE) /nologo /$(MAKEFLAGS) lxlite
  cd ..

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
