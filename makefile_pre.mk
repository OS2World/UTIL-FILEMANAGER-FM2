# makefile_pre.mk - common makefile prefix settings for all makefiles
# $Id$

# Copyright (c) 1993-98 M. Kimes
# Copyright (c) 2002, 2011 Steven H. Levine

# 03 Jan 08 SHL Switch to wrc.exe default; support USE_RC from environment
# 23 Jan 08 JBS Add support for building SYM files (Ticket 226)
# 27 May 08 SHL Add WARNALL and FORTIFY support
# 22 Jul 08 SHL Pass FORTIFY to subordinate makefiles
# 06 Oct 08 SHL Pass DEBUG in CFLAGS; clean up USE_RC usage
# 18 Nov 08 JBS Ticket 297: Various build improvements/corrections
# 12 Jul 09 GKY Allow FM/2 to load in high memory call exehdr /hi
# 13 Apr 10 SHL Drop HIMEM support
# 21 Jun 11 GKY Add exceptq .xqs support
# 2011-07-01 SHL sort map

# Environment: see dll\makefile

# DEBUG - not defined = release build, defined = debug build
# WARNALL - add more warnings if defined
# FORTIFY - build with FORTIFYed memory
# USE_RC - build with rc.exe if defined, other build with wrc.exe

CC = wcc386
LINK = wlink

!ifndef USE_RC			# if not defined on command line
!ifdef %USE_RC			# if defined in environment
USE_RC = $(%USE_RC)
!endif
!endif

!ifdef USE_RC
RC = rc -n
!else
RC = wrc -q
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

!ifdef %WARNALL			# if defined in environment
WARNALL = $(%WARNALL)		# use value from environment
!endif

!ifdef FORTIFY				# if defined on wmake command line
FORTIFY_OPT = FORTIFY=$(FORTIFY)	# set in case needed by sub-make
!else
!ifdef %FORTIFY                 	# if defined in environment
FORTIFY = $(%FORTIFY)           	# use value from environment
FORTIFY_OPT = FORTIFY=$(FORTIFY)	# set in case needed by sub-make
!endif
!endif

SYMS = $(BASE).sym              #set a target for building SYM files

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
# -we		treat warnings as errors
# -wx		max warnings
# -zfp		disable fs use
# -zgp		disable gs use
# -zp4		align 4
# -zq		quiet

# -wx excludes these
# See GenCOptions() in openwatcom\bld\cc\c\coptions.c
# -wce130	possible loss of precision
# -wcd=303	no reference to formal parameter
# -wcd=307	obsolete non-prototype declarator
# -wcd=308	unprototyped function called
# -wcd=309	unprototyped function called indirectly

# We always compile with debug info to avoid needing a full rebuild just to debug
CFLAGS = -bt=os2 -mf -bm -d2 -olirs   -s -j -wx -zfp -zgp -zp4 -zq -hd

!ifdef WARNALL
CFLAGS += -wce=118 -wce=130 -wce=303 -wce=307 -wce=308 -wce=309
!else
CFLAGS += -we
!endif

!ifdef FORTIFY
CFLAGS += -dFORTIFY
!endif

LFLAGS = sys os2v2_pm op quiet op verbose op cache op caseexact op map sort global
!ifdef DEBUG
CFLAGS += -d$DEBUG_OPT
LFLAGS += debug dwarf all
!endif

# rc Includes can be in current director or dll subdirectory
!ifdef USE_RC
RCFLAGS = -r -i dll
RCFLAGS2 = -x2
!else
# Pass 1 flags
RCFLAGS = -r -i=dll -ad
# Pass 2 flags
RCFLAGS2 =-ad
!endif

.SUFFIXES:
.SUFFIXES: .obj .c .res .rc .ipf .sym .map .xqs

!ifdef USE_RC
.rc.res:
   @echo.
   @echo Compiling resource: $*.rc
   @echo.
   $(RC) $(RCFLAGS) $*.rc
   ren $*.res $*.res
!else
.rc.res: .AUTODEPEND
   @echo.
   @echo Compiling resource: $*.rc
   @echo.
  $(RC) $(RCFLAGS) $*.rc
!endif

.c.obj: .AUTODEPEND
  $(CC) $(CFLAGS) $*.c

# The end
