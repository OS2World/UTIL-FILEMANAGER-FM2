# dll\makefile - build fm/2 dlls, help files, string tables and resource kit
# $Id$

# Copyright (c) 1993-98 M. Kimes
# Copyright (c) 2002, 2012 Steven H. Levine

# 22 May 03 SHL Correct icon dependencies
# 12 May 07 SHL Drop obsolete macros
# 02 Jun 07 SHL Convert to OpenWatcom
# 27 Jun 07 SHL Allow DEBUG set from command line or environment
# 03 Jul 07 SHL Change DEBUG semantics to ifdef/ifndef
# 04 Jul 07 SHL Pass DEBUG settings to sub-make
# 04 Jul 07 SHL Add missing .AUTODEPEND
# 06 Aug 07 SHL Tweak CFLAGS for DEBUG builds
# 26 Aug 07 SHL Clean *.lrf
# 05 Sep 07 SHL Correct USE_WRC logic
# 22 Sep 07 SHL Switch to 4 byte packing (-zp4)
# 03 Jan 08 SHL Prepare for final, implending switch to wrc.exe
# 03 Jan 08 SHL Switch to wrc.exe default; support USE_RC from environment
# 05 Jan 08 SHL Sync with .c renames
# 22 Feb 08 JBS Suppress lxlite processing when DEBUG=1
# 01 Mar 08 SHL Add missing dependencies.  Add WARNALL support
# 05 May 08 SHL Add FORTIFY support
# 26 May 08 SHL Update WARNALL to warn about more
# 22 Jul 08 SHL Change from dllsyms to syms target for consistency
# 06 Oct 08 SHL Add missing -dDEBUG= to CFLAGS; rework lxlite suppress logic
# 25 Oct 08 SHL Rework dependencies to avoid target dependent build failures
# 18 Nov 08 JBS Ticket 297: Various build improvements/corrections
# 19 Nov 08 JBS Ticket 297: Removed bldlevel calls
# 21 Nov 08 JBS Ticket 297: Added support for copyright.c and copyright.h
# 08 Dec 08 SHL Ticket 307: Add process dump support
# 10 Dec 08 SHL Ticket 26: Add exception handler and exceptq support
# 14 Dec 08 SHL Drop copyright.c support - it's gone
# 18 Dec 08 SHL Avoid extra fm3res.str builds
# 04 Feb 09 SHL Drop mkstr and fm3*.str now that we use STRINGTABLE
# 12 Jul 09 GKY Allow FM/2 to load in high memory call exehdr /hi
# 30 Jul 09 SHL Avoid attempting to attach resources to fm3res.map
# 13 Apr 10 SHL Drop HIMEM support
# 09 Feb 11 SHL Add exceptq .xqs support
# 21 Jun 11 GKY Make high memory builds the default
# 01 Jul 11 SHL sort map
# 04 Jul 11 GKY Make xqs files an explicit target so they will be rebuild if lost somehow.
# 04 Jul 11 GKY Add eFTE backup files (*~) to clean target
# 13 Aug 11 GKY Build xqs and sym files as part of dll and all targets
# 25 Jan 12 SHL Allow NOHIGHMEM set from enviroment
# 18 Feb 14 JBS Ticket #516: Run exehgr to set high-memory only if something has changed.

# Environment: see makefile_pre.mk and makefile_post.mk

# DEBUG - debug build if defined, release build if not defined
# WARNALL - add more warnings if defined, standard warnings if not defined
# FORTIFY - enable FORTIFY heap checking if defined, omit FORITY support if not defined
# USE_RC - use rc.exe if defined, use wrc.exe if not defined
# NOHIGHMEM - support high memory use (i.e exehdir /highmem)

BASE = fm3dll
BASERES = fm3res

.SUFFIXES:
.SUFFIXES: .obj .c .res .rc .ipf

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

# Keep this code in sync with makefile_pre.mk
!ifdef DEBUG			# if defined on wmake command line
DEBUG_OPT = DEBUG=$(DEBUG)	# set in case needed by sub-make
!else
!ifdef %DEBUG			# if defined in environment
DEBUG = $(%DEBUG)		# use value from environment
DEBUG_OPT = DEBUG=$(DEBUG)	# set in case needed by sub-make
!endif
!endif

!ifdef %WARNALL			# if defined in environment
WARNALL = $(%WARNALL)		# use value from environment
!endif

!ifdef %FORTIFY			# if defined in environment
FORTIFY = $(%FORTIFY)		# use value from environment
!endif

# Keep this code in sync with makefile_post.mk
!ifndef NOHIGHMEM
!ifdef %NOHIGHMEM
NOHIGHMEM=$(%NOHIGHMEM)
!endif
!endif

SYMS = $(BASE).sym $(BASERES).sym $(BASE).xqs $(BASERES).xqs # targets for building SYM files

# Some flags are order dependent - see OpenWatcom docs
# -bc		console app
# -bd		build target is a Dynamic Link Library (DLL)
# -bg		gui app with WinMain entry point
# -bm		multithread libs
# -bt=os2	target
# -d1		line number debugging information
# -d2		full symbolic debugging information
# -d3		full debug w/unref
# -hd		dwarf
# -j		signed char
# -mf		flat
# -of		generate traceable stack frames
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

!ifdef DEBUG
CFLAGS =  -bt=os2 -mf -bd -bm -hd -d2 -olirs -s -sg -j -wx -zfp -zgp -zp4 -zq -d$DEBUG_OPT
CFLAGS =  -bt=os2 -mf -bd -bm -hd -d2        -s -sg -j -wx -zfp -zgp -zp4 -zq -d$DEBUG_OPT
!else
CFLAGS =  -bt=os2 -mf -bd -bm -hd -d1 -olirs -s -sg -j -wx -zfp -zgp -zp4 -zq
!endif

# 26 May 08 SHL Was reversed
!ifdef WARNALL
CFLAGS += -wce=118 -wce=130 -wce=303 -wce=307 -wce=308 -wce=309
!else
CFLAGS += -we
!endif

!ifdef FORTIFY
CFLAGS += -dFORTIFY
!endif

# for fm3res only
CFLAGSR = -bt=os2 -mf -bd -bm         -olirs -s     -j -we -wx -zfp -zgp -zp4 -zq

LFLAGS = sys os2v2_dll initinstance terminstance op quiet op verbose op cache &
	 op caseexact op implib op map sort global

!ifdef DEBUG
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
RCFLAGS2 =
!endif

!ifndef MAKERES
.c.obj: .AUTODEPEND
  $(CC) $(CFLAGS) $*.c
!else
  # !error "MAKERES mode"
!endif

OBJS = arccnrs.obj archive.obj assoc.obj attribs.obj autoview.obj &
       avl.obj avv.obj chklist.obj cmdline.obj codepage.obj &
       collect.obj colors.obj command.obj common.obj &
       comp.obj copyf.obj &
       datamin.obj defview.obj delims.obj dircnrs.obj &
       dirs.obj dirsize.obj draglist.obj droplist.obj dumputil.obj &
       eas.obj errutil.obj excputil.obj extract.obj &
       filldir.obj filter.obj findrec.obj flesh.obj fm2cmd.obj &
       fonts.obj fortify.obj &
       getnames.obj grep.obj grep2.obj i18nutil.obj info.obj inis.obj &
       init.obj input.obj instant.obj key.obj killproc.obj literal.obj &
       loadbmp.obj mainwnd.obj mainwnd2.obj makelist.obj menu.obj misc.obj &
       mkdir.obj mle.obj newview.obj notebook.obj notify.obj objcnr.obj &
       objwin.obj &
       pathutil.obj &
       presparm.obj printer.obj remap.obj rename.obj saveclip.obj &
       seeall.obj select.obj seticon.obj shadow.obj sortcnr.obj srchpath.obj &
       strutil.obj strips.obj stristr.obj subj.obj sysinfo.obj systemf.obj &
       timer.obj tmrsvcs.obj tools.obj treecnr.obj &
       undel.obj update.obj uudecode.obj &
       valid.obj viewer.obj viewinf.obj walkem.obj winlist.obj worker.obj &
       wrappers.obj

ICONS = icons\*.ico ..\icons\*.ico  icons\*.ptr

!ifndef MAKERES

all: &
     $(BASE).dll &
     $(BASERES).res &
     $(BASERES).dll &
     $(SYMS) &
     ipf\fm3.hlp

syms: $(SYMS) .symbolic

$(BASE).dll $(BASE).lib $(BASE).map: $(OBJS) $(BASE).def $(BASE).lrf
  @echo Linking $(BASE).dll
  $(LINK) @$(BASE).lrf @$(BASE).def
  @rem type $(BASE).lrf
  !exehdr /hi $(BASE).dll

$(BASE).lrf: $(__MAKEFILES__)
   @%write $^@ $(LFLAGS)
   @%append $^@ name $(BASE)
   @for %f in ($(OBJS)) do @%append $^@ file %f
  @%append $^@ library os2386.lib

!else # MAKERES defined

# MAKERES mode - build resources only

all: $(BASERES).res &
     $(BASERES).dll &
     ipf\fm3.hlp

!endif # MAKERES

# Update resources only

res:
  @echo Updating resources only
  $(MAKE) $(__MAKEOPTS__) $(DEBUG_OPT) MAKERES=1

$(BASERES).obj: $(BASERES).c .AUTODEPEND
  $(CC) $(CFLAGSR) $(BASERES).c

$(BASERES).res: *.rc *.dlg fm3dll2.h fm3dlg.h copyright.h $(ICONS)
  @echo.
  @echo Compiling resource: $*
  @echo.
  $(RC) $(RCFLAGS) $*
!ifdef USE_RC
  ren $*.res $*.res
!endif

!ifndef MAKERES

$(BASERES).dll $(BASERES).lib $(BASERES).map: $(BASERES).res $(BASERES).obj $(BASERES).def $(BASERES).lrf
  @echo Linking $(BASERES).dll
  $(LINK) @$(BASERES).lrf @$(BASERES).def
  @echo.
  @echo Attaching resources to $(BASERES).dll
  @echo.
  $(RC) $(RCFLAGS2) $(BASERES).res $(BASERES).dll
  !exehdr /hi $(BASERES).dll

$(BASERES).lrf: $(__MAKEFILES__)
   @%write $^@ $(LFLAGS)
   @%append $^@ name $(BASERES)
   @for %f in ($(BASERES).obj) do @%append $^@ file %f

$(BASE).sym: $(BASE).map .explicit
   @echo Processing: $?
   -perl ..\debugtools\mapsymw.pl $?

$(BASE).xqs: $(BASE).map .explicit
   @echo Processing: $?
   -mapxqs $?

$(BASERES).sym: $(BASERES).map .explicit
   @echo Processing: $?
   -perl ..\debugtools\mapsymw.pl $?

$(BASERES).xqs: $(BASERES).map .explicit
   @echo Processing: $?
   -mapxqs $?

!else # MAKERES defined

# MAKERES mode - build resources only

$(BASERES).dll: $(BASERES).res
  @if not exist $@ echo $@ missing
!ifndef DEBUG
  lxlite $@ /x+ /b-
  lxlite $@ /c:minstub
!endif
  @echo.
  @echo Attaching resources to $@
  @echo.
  $(RC) $(RCFLAGS2) $(BASERES).res $@
!ifndef DEBUG
  lxlite $@ /x- /b-
!endif

!endif # MAKERES

# For testing new code
tmp.obj: tmp.c

ipf: ipf\fm3.hlp .symbolic

ipf\fm3.hlp: ipf\*.ipf ipf\bitmaps\*.bmp ipf\makefile
  cd ipf
  $(MAKE) $(__MAKEOPTS__) $(DEBUG_OPT) fm3.hlp
  cd..

# Run for each dependent
lxlite: $(BASE).dll $(BASERES).dll .symbolic
# !lxlite /x- /b- $?
!ifndef DEBUG
  @for %f in ($(BASE).dll $(BASERES).dll) do !lxlite /x- /b- %f
!endif

!ifndef NOHIGHMEM
# Run for each dependent
loadhigh: $(BASE).dll $(BASERES).dll .symbolic
  @for %f in ($(BASE).dll $(BASERES).dll) do !exehdr /hi %f
!endif

cleanobj: .symbolic
  -del *.obj

clean: .symbolic
  cd ipf
  $(MAKE) $(__MAKEOPTS__) $(DEBUG_OPT) clean
  cd..
  -del *.dll
  -del *.lib
  -del *.lrf
  -del *.map
  -del *.obj
  -del *.res
  -del *.sym
  -del *.xqs
  -del *~

