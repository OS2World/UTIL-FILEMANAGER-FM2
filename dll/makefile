# dll\makefile - build fm/2 dlls, help files, string tables and resource kit
# $Id$

# 22 May 03 SHL Correct icon dependencies
# 21 Nov 03 SHL Correct ipf dependents
# 26 Jul 04 SHL Correct CFLAGSR
# 23 May 05 SHL Drop saymsg
# 24 May 05 SHL Add clean and cleanobj targets
# 06 Jun 05 SHL Add warnings enable (WARN)
# 24 Jul 05 SHL Rework resource kit support
# 16 Apr 06 SHL Add lxlite target

# Warning - missing many *.h dependencies

# Environment:

#   DEBUG	0 = release build, 1 = debug build
#   WARN	define for verbose compile time warnings

BASE=fm3dll
BASERES=fm3res

.SUFFIXES: .c .rc .ipf

LINK = ilink
RC = rc

!ifndef DEBUG
DEBUG = 0
!endif

# Define WARN for more warnings

!ifndef WARN
CFLAGS =  /G5 /Ge- /Gm+ /Gs- /Gt- /Mp /O- /Q+ /Sp4 /Ss /Ti+ /W3
!ELSE
# /Wcnd- suppresses EDC0425
# /Wuni is easily confused by if statements
# /Word is too easily confused by PM macros
# CFLAGSW = /Wcmp /Wcnd- /Wcns /Wcnv- /Weff /Wext- /Wini /Wobs /Wpor /Wpro /Wrea /Wret /Wtrd- /Wuni- /Wuse
CFLAGSW = /Wcmp /Wcnd- /Wcns /Wcnv- /Weff /Wext- /Wini /Wobs /Wpor /Wpro /Wrea /Wret /Wtrd- /Wuni- /Wuse
CFLAGS =  /G5 /Ge- /Gm+ /Gs- /Gt- /Mp /O- /Q+ /Sp4 /Ss /Ti+ /W3 $(CFLAGSW)
!endif

# for fm3res only
CFLAGSR = /G5 /Ge- /Gs+ /O+ /Rn /Ss /W3

!IF $(DEBUG)
LFLAGS = /DE /ALIGN:4 /EXEPACK:2 /NOI /MAP /NOE
!ELSE
LFLAGS = /ALIGN:4 /EXEPACK /MAP /NOI /NOE
!endif

!ifndef MAKERES
.c.obj:
  $(CC) $(CFLAGS) /C $*.c
!else
  # !error "MAKERES mode"
!endif

OFILES= mainwnd.obj dircnrs.obj valid.obj filldir.obj error.obj\
	treecnr.obj presparm.obj misc.obj init.obj copyf.obj strips.obj\
	flesh.obj dirs.obj srchpath.obj avl.obj literal.obj stristr.obj\
	mkdir.obj avv.obj systemf.obj cmdline.obj chklist.obj makelist.obj\
	inis.obj eas.obj getnames.obj subj.obj dirsize.obj input.obj\
	select.obj fonts.obj codepage.obj mle.obj viewer.obj saveclip.obj
OFILES2=walkem.obj archive.obj extract.obj filter.obj assoc.obj draglist.obj\
	droplist.obj shadow.obj arccnrs.obj printer.obj attribs.obj rename.obj\
	comp.obj findrec.obj update.obj info.obj fsopen.obj seticon.obj\
	objcnr.obj tools.obj sortcnr.obj collect.obj grep.obj command.obj\
	killproc.obj undel.obj instant.obj objwin.obj sysinfo.obj key.obj\
	notify.obj winlist.obj viewinf.obj defview.obj
OFILES3=notebook.obj commafmt.obj seeall.obj autoview.obj delims.obj\
	menu.obj worker.obj datamin.obj fm2cmd.obj newview.obj colors.obj\
	uudecode.obj loadbmp.obj mainwnd2.obj remap.obj timer.obj grep2.obj\
	common.obj string.obj

LFILES=$(OFILES:.obj=)
LFILES2=$(OFILES2:.obj=)
LFILES3=$(OFILES3:.obj=)

!ifndef MAKERES

all: $(BASE).dll \
     $(BASERES).res \
     $(BASERES).dll \
     $(BASE).lib \
     $(BASERES).lib \
     ipf\fm3.hlp \
     internal\mkstr.exe \
     fm3res.str

$(BASE).dll: $(OFILES) $(OFILES2) $(OFILES3) $(BASE).def
  @$(LINK) @<<$(BASE).lrf
  $(LFLAGS)
  /LI
  /OUT:$@
  $(OFILES)
  $(OFILES2)
  $(OFILES3)
  $(BASE).def
<<
  @rem type $(BASE).lrf
  bldlevel $@

$(BASE).lib: $(BASE).dll $(BASE).def
  implib $(BASE).lib $(BASE).def

!else

# Just resources

all: chkexe \
     $(BASERES).res \
     $(BASERES).dll \
     ipf\fm3.hlp \
     fm3res.str

chkexe:
  @echo Checking required EXEs
  if not exist internal\mkstr.exe internal\mkstr.exe
  lxlite -c:exehdr internal\mkstr.exe >nul

!endif

# Update resources only

res:
  @echo Updating resources only
  $(MAKE) /nologo /$(MAKEFLAGS) MAKERES=1

init.c: $(BASE).h version.h

string.c: fm3str.h version.h

internal\mkstr.exe: internal\mkstr.c version.h fm3str.h
  cd internal
  $(MAKE) /nologo /$(MAKEFLAGS)
  cd..

$(BASERES).obj: $(BASERES).c version.h
  $(CC) $(CFLAGSR) /C $(BASERES).c

$(BASERES).res: *.rc *.dlg fm3dll2.h fm3dlg.h icons\*.ico icons\*.ptr
  $(RC) -r $*

!ifndef MAKERES

$(BASERES).dll: $(BASERES).res $(BASERES).obj $(BASERES).def
  @REM @<<$(BASERES).@0
  $(LFLAGS)
  /OUT:$@
  $(BASERES).obj
  $(BASERES).def
<<
  type $(BASERES).@0
  $(LINK) @$(BASERES).@0
  $(RC) -x2 $(BASERES).res $@
  :: SHL ea /d /e:.ICON /V $@
  bldlevel $@

!else

$(BASERES).dll: $(BASERES).res
  @if not exist $@ echo $@ missing
  lxlite $@ /x+ /b-
  lxlite $@ /c:minstub
  $(RC) -x2 $(BASERES).res $@
  lxlite $@ /x- /b-
  bldlevel $@

!endif

$(BASERES).lib: $(BASERES).dll $(BASERES).def
  implib $(BASERES).lib $(BASERES).def

$(BASERES).str: $(BASE).str fm3str.h version.h icons\*.ico
  internal\mkstr
  copy $(BASERES).str .. /v

ipf\FM3.hlp: ipf\*.ipf ipf\bitmaps\*.bmp
  cd ipf
  ipfc /CODEPAGE=850 fm3.ipf
  ren fm3.HLP fm3.hlp
  cd..

avv.obj:      version.h
key.obj:      version.h
misc.c:       version.h

lxlite: $(BASE).dll $(BASERES).dll
  !lxlite /x- /b- $?

cleanobj:
  cd internal
  $(MAKE) /nologo /$(MAKEFLAGS) cleanobj
  cd..
  -del *.obj

clean:
  cd internal
  $(MAKE) /nologo /$(MAKEFLAGS) clean
  cd..
  -del *.dll
  -del *.lib
  -del *.map
  -del *.obj
  -del *.res
  -del fm3res.str


