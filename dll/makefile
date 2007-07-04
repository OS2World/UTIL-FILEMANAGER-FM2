# dll\makefile - build fm/2 dlls, help files, string tables and resource kit
# $Id$

# Copyright (c) 1993-98 M. Kimes
# Copyright (c) 2002, 2007 Steven H. Levine

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

# Environment:

#   DEBUG - not defined = release build, defined = debug build

BASE = fm3dll
BASERES = fm3res

.SUFFIXES:
.SUFFIXES: .obj .c .res .rc .ipf

CC = wcc386
LINK = wlink

# fixme for wrc to build working .res
# fixme for wrc to not clobber bldlevel strings

!ifndef USE_WRC
USE_WRC = 0
!endif

!if $(USE_WRC)
RC = wrc
!else
RC = rc
!endif

# Keep this code in sync with makefile_pre.mk
!ifndef DEBUG                  # if not defined on wmake command line
!ifdef %DEBUG                  # if defined in environment
DEBUG = $(%DEBUG)              # use environment setting
!endif
!endif

# Some flags are order dependent - see OpenWatcom docs
# -bc		console app
# -bd		build target is a Dynamic Link Library (DLL)
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

!ifdef DEBUG
CFLAGS =   -bt=os2 -mf -bd -bm -d2 -olirs   -s -sg -j -wx -zfp -zgp -zq -hd
!else
CFLAGS =   -bt=os2 -mf -bd -bm -d1 -olirs   -s -sg -j -wx -zfp -zgp -zq -hd
!endif

# for fm3res only
CFLAGSR = -bt=os2 -mf -bd -bm     -olirs   -s -j -wx -zfp -zgp

!ifdef DEBUG
LFLAGS = sys os2v2_dll initinstance terminstance op quiet op verbose op cache &
	 op caseexact op implib op map debug dwarf all
!else
LFLAGS = sys os2v2_dll initinstance terminstance op quiet op verbose op cache &
	 op caseexact op implib op map
!endif

# rc Includes can be in current director or dll subdirectory
# fixme for wrc to build working .res
# fixme for wrc to not clobber bldlevel strings
!if $(USE_WRC)
# Pass 1 flags
RCFLAGS = -r -i=dll -ad
# Pass 2 flags
RCFLAGS2 =-ad
!else
RCFLAGS = -r -i dll
RCFLAGS2 = -x2
!endif

!ifndef MAKERES
.c.obj:
  $(CC) $(CFLAGS) $*.c
!else
  # !error "MAKERES mode"
!endif

OBJS = arccnrs.obj archive.obj assoc.obj attribs.obj autoview.obj &
       avl.obj avv.obj chklist.obj cmdline.obj codepage.obj &
       collect.obj colors.obj commafmt.obj command.obj common.obj &
       comp.obj copyf.obj datamin.obj defview.obj delims.obj dircnrs.obj &
       dirs.obj dirsize.obj draglist.obj droplist.obj eas.obj error.obj &
       extract.obj filldir.obj filter.obj findrec.obj flesh.obj fm2cmd.obj &
       fonts.obj fsopen.obj getnames.obj grep.obj grep2.obj info.obj inis.obj &
       init.obj input.obj instant.obj key.obj killproc.obj literal.obj &
       loadbmp.obj mainwnd.obj mainwnd2.obj makelist.obj menu.obj misc.obj &
       mkdir.obj mle.obj newview.obj notebook.obj notify.obj objcnr.obj &
       objwin.obj presparm.obj printer.obj remap.obj rename.obj saveclip.obj &
       seeall.obj select.obj seticon.obj shadow.obj sortcnr.obj srchpath.obj &
       string.obj strips.obj stristr.obj subj.obj sysinfo.obj systemf.obj &
       timer.obj tools.obj treecnr.obj undel.obj update.obj uudecode.obj &
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

$(BASE).dll $(BASE).lib: $(OBJS) $(BASE).def $(BASE).lrf
  @echo $(LINK) @$(BASE).lrf @$(BASE).def
  $(LINK) @$(BASE).lrf @$(BASE).def
  @rem type $(BASE).lrf
  bldlevel $@

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
  lxlite -c:exehdr internal\mkstr.exe >nul

!endif

# Update resources only

res:
  @echo Updating resources only
  $(MAKE) $(__MAKEOPTS__) MAKERES=1

internal\mkstr.exe: internal\mkstr.c version.h fm3str.h
  cd internal
  $(MAKE) $(__MAKEOPTS__)
  cd..

$(BASERES).obj: $(BASERES).c
  # $(CC) $(CFLAGSR) /C $(BASERES).c
  $(CC) $(CFLAGSR) $(BASERES).c

$(BASERES).res: *.rc *.dlg fm3dll2.h fm3dlg.h $(ICONS)
  $(RC) $(RCFLAGS) $*
!if ! $(USE_WRC)
  ren $*.res $*.res
!endif

!ifndef MAKERES

$(BASERES).dll $(BASERES).lib: $(BASERES).res $(BASERES).obj $(BASERES).def $(BASERES).lrf
  @echo $(LINK) @$(BASERES).lrf @$(BASERES).def
  $(LINK) @$(BASERES).lrf @$(BASERES).def
  $(RC) $(RCLAGS2) $(BASERES).res $@
  bldlevel $@

$(BASERES).lrf: $(__MAKEFILES__)
   @%write $^@ $(LFLAGS)
   @%append $^@ name $(BASERES)
   @for %f in ($(BASERES).obj) do @%append $^@ file %f

!else

# MAKERES mode - build resources only

$(BASERES).dll: $(BASERES).res
  @if not exist $@ echo $@ missing
  lxlite $@ /x+ /b-
  lxlite $@ /c:minstub
  $(RC) $(RCFLAGS2) $(BASERES).res $@
  lxlite $@ /x- /b-
  bldlevel $@

!endif

$(BASERES).str: $(BASE).str fm3str.h version.h
  internal\mkstr
  # ren $(BASERES).str $(BASERES).str
  copy $(BASERES).str ..\ /v

ipf: ipf\fm3.hlp .symbolic

ipf\fm3.hlp:
  cd ipf
  $(MAKE) $(__MAKEOPTS__) fm3.hlp
  cd..

# Run for each dependent
lxlite: $(BASE).dll $(BASERES).dll .symbolic
# !lxlite /x- /b- $?
   @for %f in ($(BASE).dll $(BASERES).dll) do !lxlite /x- /b- %f

cleanobj: .symbolic
  cd internal
  $(MAKE) $(__MAKEOPTS__) cleanobj
  cd..
  -del *.obj

clean: .symbolic
  cd internal
  $(MAKE) $(__MAKEOPTS__) clean
  cd ..\ipf
  $(MAKE) $(__MAKEOPTS__) clean
  cd..
  -del *.dll
  -del *.lib
  -del *.map
  -del *.obj
  -del *.res
  -del fm3res.str
