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
# 22 Jun 07 JBS Moved some macro-handling code to this
#               file from warpin\makefile because of some
#               differences in how Watcom handles macros.

# Environment - see makefile_pre.mk

BASE = fm3

# FM2_VER defines fm/2 WPI file name suffix
# e.g. FM2_VER=-3-5-9 results in FM2-3-5-9.wpi being built
# If FM2_VER is empty, then FM2.wpi is built
# NOTE: Start the value with '-'

!ifndef FM2_VER                  # if defined on wmake command, use it
FM2_VER=-3-6-0                   # default value
!ifdef %FM2_VER                  # if defined via env. var.
FM2_VER=$(%FM2_VER)              #     use the env. var.
!endif
!endif

# FM2UTILS_VER defines the fm2utils WPI file name suffix.
# e.g. FM2UTILS_VER=-1-0 results in FM2Utils-1.0.wpi being built
# If FM2UTILS_VER is empty, then FM2UTILS.wpi is built
# NOTE: Start the value with '-'

!ifndef FM2UTILS_VER             # if defined on wmake command, use it
FM2UTILS_VER=-1-1                # default value
!ifdef %FM2UTILS_VER             # if defined via env. var.
FM2UTILS_VER=$(%FM2UTILS_VER)    #     use the env. var.
!endif
!endif

# If BUILD_FM2UTILS = 1, build FM2UTILS*.wpi and FM2*.wpi
# Otherwise build just FM2*.wpi

!ifndef BUILD_FM2UTILS           # if defined on wmake command, use it
!ifdef %BUILD_FM2UTILS           # else if defined via env. var.
!ifneq %BUILD_FM2UTILS 1         #     if env. var. is anything but 1
BUILD_FM2UTILS=0                 #     use a value of 0
!else
BUILD_FM2UTILS=1
!endif
!else
BUILD_FM2UTILS=0                 # use default value if not defined via env. or command line
!endif
!endif

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
   $(MAKE) $(__MAKEOPTS__) FM2_VER=$(FM2_VER) FM2UTILS_VER=$(FM2UTILS_VER) BUILD_FM2UTILS=$(BUILD_FM2UTILS)
   cd ..

# makefile_post.mk contains lxlite target for $(BASE).exe
# Apply to each *.mak for other exes
lxlite:: *.mak .symbolic
   @for %f in ($<) do $(MAKE) -f %f $(__MAKEOPTS__) lxlite

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
