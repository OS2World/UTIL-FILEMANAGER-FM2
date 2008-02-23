# makefile - build all fm/2 components
# $Id$

# Copyright (c) 1993-98 M. Kimes
# Copyright (c) 2002, 2008 Steven H. Levine

# 24 May 05 SHL Add clean and cleanobj target
# 16 Jun 05 SHL Workaround makeflags wierdness
# 18 Jul 05 SHL Add bitmap dependencies
# 20 Jul 05 SHL Add makeres support
# 16 Apr 06 SHL Add lxlite target
# 31 Jul 06 SHL Tweak dependencies
# 26 Aug 06 SHL Add rest of lxlite support
# 14 Jun 07 SHL Convert to OpenWatcom
# 22 Jun 07 JBS Moved some macro-handling code to this
#               file from warpin\makefile because of some
#               differences in how Watcom handles macros.
# 04 Jul 07 SHL Pass DEBUG settings to sub-make
# 21 Jan 08 SHL Add *.lrf to clean target
# 22 Feb 08 JBS Suppress lxlite processing when DEBUG=1

# Environment - see makefile_pre.mk

BASE = fm3

# Pass values for FM2_VER, FM2UTILS_VER and BUILD_FM2UTILS which
# have been set on the command line, if any, on to the
# warpin\makefile using the WARPIN_OPTS macro.

!ifdef FM2_VER                  # if defined on wmake command, pass it
WARPIN_OPTS = FM2_VER=$(FM2_VER)
!endif

!ifdef FM2UTILS_VER             # if defined on wmake command, pass it
WARPIN_OPTS = $(WARPIN_OPTS) FM2UTILS_VER=$(FM2UTILS_VER)
!endif

!ifdef BUILD_FM2UTILS           # if defined on wmake command, pass it
WARPIN_OPTS = $(WARPIN_OPTS) BUILD_FM2UTILS=$(BUILD_FM2UTILS)
!endif

!include makefile_pre.mk


all: dll $(BASE) allexe .symbolic

dist: all lxlite wpi .symbolic

# Only update resources
res: .symbolic
  @echo Updating resources only
  $(MAKE) $(__MAKEOPTS__) $(DEBUG_OPT) MAKERES=1

# make DLL components

dll: .symbolic
  cd dll
  $(MAKE) $(__MAKEOPTS__) $(DEBUG_OPT)
  cd ..

$(BASE): $(BASE).exe $(BASE).res .symbolic

$(BASE).res: $(BASE).rc  icons\$(BASE).ico bitmaps\*.bmp

$(BASE).obj: $(BASE).c dll\version.h

# make EXE compenents

allexe: *.mak .symbolic
   @for %f in ($<) do $(MAKE) -f %f $(__MAKEOPTS__) $(DEBUG_OPT)

wpi: .symbolic
   cd warpin
   $(MAKE) $(__MAKEOPTS__) $(DEBUG_OPT) $(WARPIN_OPTS)
   cd ..

# makefile_post.mk contains lxlite target for $(BASE).exe
# Apply to each *.mak for other exes
lxlite:: *.mak .symbolic
!ifdef DEBUG
!  ifeq DEBUG 0
     @for %f in ($<) do $(MAKE) -f %f $(__MAKEOPTS__) $(DEBUG_OPT) lxlite
!  endif
!else
     @for %f in ($<) do $(MAKE) -f %f $(__MAKEOPTS__) $(DEBUG_OPT) lxlite
!endif

# Apply to dlls
lxlite:: .symbolic
  cd dll
!ifdef DEBUG
!  ifeq DEBUG 0
     $(MAKE) $(__MAKEOPTS__) $(DEBUG_OPT) lxlite
!  endif
!else
  $(MAKE) $(__MAKEOPTS__) $(DEBUG_OPT) lxlite
!endif
  cd ..

cleanobj: .symbolic
  cd dll
  $(MAKE) $(__MAKEOPTS__) $(DEBUG_OPT) cleanobj
  cd ..
  -del *.obj

clean:: .symbolic
  cd dll
  $(MAKE) $(__MAKEOPTS__) $(DEBUG_OPT) clean
  cd ..
  -del *.exe
  -del *.lrf
  -del *.map
  -del *.obj
  -del *.res
  -del fm3res.str

distclean: clean .symbolic
  cd warpin
  $(MAKE) $(__MAKEOPTS__) $(DEBUG_OPT) distclean
  cd ..

!include makefile_post.mk

# The end
