# makefile_pre.mk - common makefile prefix settings for all makefiles
# $Id$

# 01 Sep 06 SHL Adjust .res case
# 02 Jun 07 SHL Convert to OpenWatcom
# 27 Jun 07 SHL Use same CFLAGS for all builds
# 27 Jun 07 SHL Allow DEBUG set from command line or environment
# 03 Jul 07 SHL Change DEBUG semantics to ifdef/ifndef
# 04 Jul 07 SHL Pass DEBUG settings to sub-make

CC = wcc386
LINK = wlink

# fixme use use wrc when wrc fixed (v1.7 maybe)
# wrc 1.6 is broken - does not copy resident name table

!ifndef USE_WRC
USE_WRC = 0
!endif

!if $(USE_WRC)
RC = wrc
!else
RC = rc
!endif

# Keep this code in sync with dll\makefile
!ifdef DEBUG                  	# if defined on wmake command line
DEBUG_OPT = DEBUG=$(DEBUG)	# set in case needed by sub-make
!else
!ifdef %DEBUG                  	# if defined in environment
DEBUG = $(%DEBUG)              	# use value from environment
DEBUG_OPT = DEBUG=$(DEBUG)	# set in case needed by sub-make
!endif
!endif

# Some flags are order dependent - see OpenWatcom docs
# -bc		console app
# -bd		build target is a Dynamic Link Library (DLL) (see bd)
# -bg		gui app with WinMain entry point
# -bm		multithread libs
# -bt=os2	target
# -d2		full debug
# -d3		full debug w/unref
# -hd		dwarf
# -j		signed char
# -mf		flat
# -olinars	optimze loops, inline, e(n)able fp recip, relax (a)lias, reordering, space
# -s		disable stack checks
# -sg		generate calls to grow the stack
# -st		touch stack through SS first
# -wcd14	no reference to symbol
# -wcd726	no reference to formal parameter
# -wx		max warnings
# -zfp		disable fs use
# -zgp		disable gs use
# -zp4		align 4
# -zq		quiet

# We always compile with debug info to avoid needed a full rebuild just to debug
CFLAGS = -bt=os2 -mf -bm -d2 -olirs   -s -j -wx -zfp -zgp -zq -hd

LFLAGS = sys os2v2_pm op quiet op verbose op cache op caseexact op map
!ifdef DEBUG
LFLAGS += debug dwarf all
!endif

# rc Includes can be in current director or dll subdirectory
!if $(USE_WRC)
# Pass 1 flags
RCFLAGS = -r -i=dll -ad
# Pass 2 flags
RCFLAGS2 =-ad
!else
RCFLAGS = -r -i dll
RCFLAGS2 = -x2
!endif

.SUFFIXES:
.SUFFIXES: .obj .c .res .rc .ipf

!if $(USE_WRC)
.rc.res: .AUTODEPEND
  $(RC) $(RCFLAGS) $*.rc
!else
.rc.res:
   $(RC) $(RCFLAGS) $*.rc
   ren $*.res $*.res
!endif

.c.obj: .AUTODEPEND
  $(CC) $(CFLAGS) $*.c

# The end
