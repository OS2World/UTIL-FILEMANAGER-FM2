# makefile - build all fm/2 components
# $Id$

# Copyright (c) 1993-98 M. Kimes
# Copyright (c) 2002, 2007 Steven H. Levine

# 24 May 05 SHL Add clean and cleanobj target
# 16 Jun 05 SHL Workaround makeflags wierdness
# 18 Jul 05 SHL Add bitmap dependencies
# 20 Jul 05 SHL Add makeres support
# 16 Apr 06 SHL Add lxlite target
# 31 Jul 06 SHL Tweak dependencies
# 26 Aug 06 SHL Add rest of lxlite support
# 14 Jun 07 SHL Convert to OpenWatcom

# Environment:

# DEBUG	0 = release build, 1 = debug build

BASE = fm3

!include makefile_pre.mk

all: dll $(BASE) allexe .symbolic

dist: all lxlite wpi .symbolic

# Only update resources
res: .symbolic
  @echo Updating resources only
  $(MAKE) $(__MAKEOPTS__) MAKERES=1

# make DLL components

dll: .symbolic
  cd dll
  $(MAKE) $(__MAKEOPTS__)
  cd ..

$(BASE): $(BASE).exe $(BASE).res .symbolic

$(BASE).res: $(BASE).rc  icons\$(BASE).ico bitmaps\*.bmp

$(BASE).obj: $(BASE).c dll\version.h

# make EXE compenents

allexe: *.mak .symbolic
   @for %f in ($<) do $(MAKE) -f %f $(__MAKEOPTS__)

wpi: .symbolic
   cd warpin
   $(MAKE) $(__MAKEOPTS__)
   cd ..

# makefile_post.mk contains lxlite target for $(BASE).exe
# Apply to each *.mak for other exes
lxlite:: *.mak .symbolic
  !$(MAKE) -f $? $(__MAKEOPTS__) lxlite

# Apply to dlls
lxlite:: .symbolic
  cd dll
  $(MAKE) $(__MAKEOPTS__) lxlite
  cd ..

cleanobj: .symbolic
  cd dll
  $(MAKE) $(__MAKEOPTS__) cleanobj
  cd ..
  -del *.obj

clean:: .symbolic
  cd dll
  $(MAKE) $(__MAKEOPTS__) clean
  cd ..
  -del *.exe
  -del *.map
  -del *.obj
  -del *.res
  -del fm3res.str

distclean: clean .symbolic
  cd warpin
  $(MAKE) $(__MAKEOPTS__) distclean
  cd ..

!include makefile_post.mk

# The end
