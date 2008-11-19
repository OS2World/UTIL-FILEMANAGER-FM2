# dll\makefile - build fm/2 dlls, help files, string tables and resource kit
# $Id$

# Copyright (c) 1993-98 M. Kimes
# Copyright (c) 2002, 2008 Steven H. Levine

# 22 May 03 SHL Correct icon dependencies
# 21 Nov 03 SHL Correct ipf dependents
# 26 Jul 04 SHL Correct CFLAGSR
# 23 May 05 SHL Drop saymsg
# 24 May 05 SHL Add clean and cleanobj targets
# 06 Jun 05 SHL Add warnings enable (WARN)
# 24 Jul 05 SHL Rework resource kit support
# 16 Apr 06 SHL Add lxlite target
# 28 Jun 06 SHL Add more .h dependencies
# 22 Jul 06 SHL Add more .h dependencies
# 29 Jul 06 SHL Reorganize icons
# 31 Jul 06 SHL Tweak dependencies
# 16 Aug 06 SHL Drop 4os2-ism
# 18 Aug 06 SHL Drop rc -x2 compression - not reliable
# 01 Sep 06 SHL fm3.hlp: add missing dependency on ..\bitmaps
# 01 Sep 06 SHL Adjust fm3str.str case
# 30 Sep 06 GKY Add del of Help file to clean
# 19 Oct 06 SHL Move all .hlp logic build to ipf\makefile too
# 12 May 07 SHL Drop obsolete macros
# 02 Jun 07 SHL Convert to OpenWatcom
# 27 Jun 07 SHL Allow DEBUG set from command line or environment
# 03 Jul 07 SHL Change DEBUG semantics to ifdef/ifndef
# 04 Jul 07 SHL Pass DEBUG settings to sub-make
# 04 Jul 07 SHL Add missing .autodepend
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
# 18 Nov 08 JBS Ticket 297: Various build improvements/corrections
# 19 Nov 08 JBS Ticket 297: Removed bldlevel calls

# Environment:

# DEBUG - not defined = release build, defined = debug build
# WARNALL - add more warnings if defined
# FORTIFY - build with FORTIFYed memory

BASE = fm3dll
BASERES = fm3res

.SUFFIXES:
.SUFFIXES: .obj .c .res .rc .ipf

CC = wcc386
LINK = wlink

!ifndef USE_RC			# if not defined on command line
!ifdef %USE_RC			# if defined in environment
USE_RC = $(%USE_RC)
!else
USE_RC = 0
!endif
!endif

!if $(USE_RC)
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

SYMS = $(BASE).sym $(BASERES).sym internal\mkstr.sym  #set a targets for building SYM files

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

!ifdef DEBUG
LFLAGS = sys os2v2_dll initinstance terminstance op quiet op verbose op cache &
	 op caseexact op implib op map debug dwarf all
!else
LFLAGS = sys os2v2_dll initinstance terminstance op quiet op verbose op cache &
	 op caseexact op implib op map
!endif

# rc Includes can be in current director or dll subdirectory
!if $(USE_RC)
RCFLAGS = -r -i dll
RCFLAGS2 = -x2
!else
# Pass 1 flags
RCFLAGS = -r -i=dll -ad
# Pass 2 flags
RCFLAGS2 =
!endif

!ifndef MAKERES
.c.obj: .autodepend
  $(CC) $(CFLAGS) $*.c
!else
  # !error "MAKERES mode"
!endif

OBJS = arccnrs.obj archive.obj assoc.obj attribs.obj autoview.obj &
       avl.obj avv.obj chklist.obj cmdline.obj codepage.obj &
       collect.obj colors.obj commafmt.obj command.obj common.obj &
       comp.obj copyf.obj datamin.obj defview.obj delims.obj dircnrs.obj &
       dirs.obj dirsize.obj draglist.obj droplist.obj eas.obj errutil.obj &
       extract.obj filldir.obj filter.obj findrec.obj flesh.obj fm2cmd.obj &
       fonts.obj fortify.obj fsopen.obj &
       getnames.obj grep.obj grep2.obj info.obj inis.obj &
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
     ipf\fm3.hlp &
     internal\mkstr.exe &
     fm3res.str

syms: $(SYMS) .symbolic

$(BASE).dll $(BASE).lib: $(OBJS) $(BASE).def $(BASE).lrf
  @echo $(LINK) @$(BASE).lrf @$(BASE).def
  $(LINK) @$(BASE).lrf @$(BASE).def
  @rem type $(BASE).lrf

$(BASE).lrf: $(__MAKEFILES__)
   @%write $^@ $(LFLAGS)
   @%append $^@ name $(BASE)
   @for %f in ($(OBJS)) do @%append $^@ file %f
!ifdef %EXCEPTQ
    @%append $^@ file exceptq.lib
!endif
  @%append $^@ library os2386.lib

!else

# MAKERES mode - build resources only

all: chkexe &
     $(BASERES).res &
     $(BASERES).dll &
     ipf\fm3.hlp &
     fm3res.str

chkexe:
  @echo Checking required EXEs
  if not exist internal\mkstr.exe internal\mkstr.exe
!ifndef DEBUG
  lxlite -c:exehdr internal\mkstr.exe >nul
! endif

!endif

# Update resources only

res:
  @echo Updating resources only
  $(MAKE) $(__MAKEOPTS__) $(DEBUG_OPT) MAKERES=1

internal\mkstr.exe: internal\mkstr.c version.h fm3str.h
  cd internal
  $(MAKE) $(__MAKEOPTS__) $(DEBUG_OPT)
  cd..

$(BASERES).obj: $(BASERES).c .autodepend
  $(CC) $(CFLAGSR) $(BASERES).c

$(BASERES).res: *.rc *.dlg fm3dll2.h fm3dlg.h $(ICONS)
  @echo.
  @echo Compiling resource: $*
  @echo.
  $(RC) $(RCFLAGS) $*
!if $(USE_RC)
  ren $*.res $*.res
!endif

!ifndef MAKERES

$(BASERES).dll $(BASERES).lib: $(BASERES).res $(BASERES).obj $(BASERES).def $(BASERES).lrf
  @echo $(LINK) @$(BASERES).lrf @$(BASERES).def
  $(LINK) @$(BASERES).lrf @$(BASERES).def
  @echo.
  @echo Compiling resource: $@
  @echo.
  $(RC) $(RCFLAGS2) $(BASERES).res $@

$(BASERES).lrf: $(__MAKEFILES__)
   @%write $^@ $(LFLAGS)
   @%append $^@ name $(BASERES)
   @for %f in ($(BASERES).obj) do @%append $^@ file %f

$(BASE).sym: $(BASE).map
   @echo Processing: $?
   -perl ..\debugtools\mapsymw.pl $?

$(BASERES).sym: $(BASERES).map
   @echo Processing: $?
   -perl ..\debugtools\mapsymw.pl $?

internal\mkstr.sym: internal\mkstr.map
   @echo Processing: $?
   cd internal
   -perl ..\..\debugtools\mapsymw.pl mkstr.map
   cd ..

!else

# MAKERES mode - build resources only

$(BASERES).dll: $(BASERES).res
  @if not exist $@ echo $@ missing
!ifndef DEBUG
  lxlite $@ /x+ /b-
  lxlite $@ /c:minstub
!endif
  @echo.
  @echo Compiling resource: $@
  @echo.
  $(RC) $(RCFLAGS2) $(BASERES).res $@
!ifndef DEBUG
  lxlite $@ /x- /b-
!endif

!endif # MAKERES mode

# For testing new code
tmp.obj: tmp.c

$(BASERES).str: $(BASE).str fm3str.h version.h
  internal\mkstr
  # ren $(BASERES).str $(BASERES).str
  copy $(BASERES).str ..\ /v

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

cleanobj: .symbolic
  cd internal
  $(MAKE) $(__MAKEOPTS__) $(DEBUG_OPT) cleanobj
  cd..
  -del *.obj

clean: .symbolic
  cd internal
  $(MAKE) $(__MAKEOPTS__) $(DEBUG_OPT) clean
  cd ..\ipf
  $(MAKE) $(__MAKEOPTS__) $(DEBUG_OPT) clean
  cd..
  -del *.dll
  -del *.lib
  -del *.lrf
  -del *.map
  -del *.obj
  -del *.res
  -del *.sym
  -del fm3res.str
