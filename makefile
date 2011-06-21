# makefile - build all fm/2 components
# $Id$

# Copyright (c) 1993-98 M. Kimes
# Copyright (c) 2002, 2010 Steven H. Levine

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
# 08 Jul 08 SHL Avoid extra work for wmake -a dist
# 22 Jul 08 SHL Change from dll\dllsyms to dll\syms target for consistency
# 22 Jul 08 SHL Pass FORTIFY options to subordinate makefiles
# 25 Oct 08 SHL Sanitize DEBUG usage
# 18 Nov 08 JBS Ticket 297: Various build improvements/corrections
# 14 Dec 08 SHL Build fm3.sym
# 12 Jul 09 GKY Allow FM/2 to load in high memory call exehdr /hi
# 13 Apr 10 SHL Drop HIMEM support
# 21 Jun 11 GKY Add exceptq .xqs support
# 21 Jun 11 GKY Make high memory builds the default (resources only for exes)

# Environment - see makefile_pre.mk and dll\makefile

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

syms: fm3.sym exesyms dllsyms .symbolic

dist: all syms lxlite wpi .symbolic

disth: $(BASE) allexe syms lxlite wpi .symbolic

# Only update resources
res: .symbolic
  @echo Updating resources only
  $(MAKE) -h $(__MAKEOPTS__) $(DEBUG_OPT) MAKERES=1

# make DLL components

dll: .symbolic
  cd dll
  $(MAKE) -h $(__MAKEOPTS__) $(DEBUG_OPT) $(FORTIFY_OPT)
  cd ..

dllsyms: .symbolic
  cd dll
  $(MAKE) -h $(__MAKEOPTS__) $(DEBUG_OPT) $(FORTIFY_OPT) syms
  cd ..

$(BASE): $(BASE).exe $(BASE).res .symbolic

$(BASE).res: $(BASE).rc  icons\$(BASE).ico bitmaps\*.bmp  .autodepend

$(BASE).obj: $(BASE).c dll\version.h .autodepend

# make EXE compenents

allexe: *.mak .symbolic
  @for %f in ($<) do $(MAKE) -h -f %f $(__MAKEOPTS__) $(DEBUG_OPT) $(FORTIFY_OPT)

# make SYM files

exesyms: *.mak .symbolic
  @for %f in ($<) do $(MAKE) -h -f %f $(__MAKEOPTS__) $(DEBUG_OPT) $(FORTIFY_OPT) sym

# make WPI files

wpi: .symbolic
  cd warpin
  $(MAKE) -h $(__MAKEOPTS__) $(DEBUG_OPT) $(WARPIN_OPTS)
  cd ..

lxlite:: lxlitedll lxliteexe .symbolic

# makefile_post.mk contains lxlite target for $(BASE).exe
# Apply to each *.mak for other exes
lxliteexe: *.mak .symbolic
!ifndef DEBUG
  @for %f in ($<) do $(MAKE) -h -f %f $(__MAKEOPTS__) $(DEBUG_OPT) $(FORTIFY_OPT) lxlite
!endif

# Apply to dlls
lxlitedll: .symbolic
!ifndef DEBUG
  cd dll
  $(MAKE) -h lxlite
#  $(MAKE) -h $(__MAKEOPTS__) $(DEBUG_OPT) $(FORTIFY_OPT) lxlite
  cd ..
!endif

cleanobj: .symbolic
  cd dll
  $(MAKE) -h $(__MAKEOPTS__) $(DEBUG_OPT) cleanobj
  cd ..
  -del *.obj

clean:: .symbolic
  cd dll
  $(MAKE) -h $(__MAKEOPTS__) $(DEBUG_OPT) clean
  cd ..
  -del *.exe
  -del *.lrf
  -del *.map
  -del *.obj
  -del *.res
  -del *.sym
  -del *.xqs

distclean: clean .symbolic
  cd warpin
  $(MAKE) -h $(__MAKEOPTS__) $(DEBUG_OPT) distclean
  cd ..

!include makefile_post.mk

# The end
