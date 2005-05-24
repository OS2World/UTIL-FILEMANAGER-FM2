# dll\makefile - build fm/2 dlls, help files, string tables and resource kit
# $Id$

# 22 May 03 SHL Correct icon dependencies
# 21 Nov 03 SHL Correct ipf dependents
# 26 Jul 04 SHL Correct CFLAGSR
# 23 May 05 SHL Drop saymsg

# Environment:

#   DEBUG	0 = release build, 1 = debug build

BASE=fm3dll
BASERES=fm3res

.SUFFIXES: .c .rc .ipf

LINK = ilink
RC = rc

!IFNDEF DEBUG
DEBUG = 0
!ENDIF

CFLAGS =  /G5 /Ge- /Gm+ /Gs- /Gt- /Mp /O- /Q+ /Sp4 /Ss /Ti+ /W3
# for fm3res only
CFLAGSR = /G5 /Ge- /Gs+ /O+ /Rn /Ss /W3

!IF $(DEBUG)
LFLAGS = /DE /ALIGN:4 /EXEPACK:2 /NOI /MAP /NOE
!ELSE
LFLAGS = /ALIGN:4 /EXEPACK /MAP /NOI /NOE
!ENDIF

.c.obj:
  $(CC) $(CFLAGS) /C $*.c

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

ALL: FM3DLL.DLL \
     FM3RES.RES \
     FM3RES.DLL \
     FM3DLL.LIB \
     FM3RES.LIB \
     ipf\FM3.HLP \
     internal\mkstr.exe \
     fm3res.str

$(BASE).dll: $(OFILES) $(OFILES2) $(OFILES3) $(BASE).def
    @REM @<<$(BASE).@0
    $(LFLAGS)
    /OUT:$@
    $(OFILES)
    $(OFILES2)
    $(OFILES3)
    $(BASE).def
<<
    @rem type $(BASE).@0
    $(LINK) @$(BASE).@0

$(BASE).lib: $(BASE).dll $(BASE).def
    IMPLIB $(BASE).lib $(BASE).def

init.c: $(BASE).h version.h

string.c: fm3str.h version.h

internal\mkstr.exe: internal\mkstr.c version.h fm3str.h
  cd internal
  compile2 mkstr.c
  cd..

$(BASERES).obj: $(BASERES).c version.h
  $(CC) $(CFLAGSR) /C $(BASERES).c

$(BASERES).res: $*.rc $*.dlg fm3dlg.h icons\*.ico
  $(RC) -r $*

$(BASERES).DLL: $(BASERES).RES $(BASERES).OBJ $(BASERES).DEF
    @REM @<<$(BASERES).@0
    $(LFLAGS)
    /OUT:$@
    $(BASERES).obj
    $(BASERES).def
<<
    type $(BASERES).@0
    $(LINK) @$(BASERES).@0
    $(RC) -x2 $(BASERES).res $(BASERES).dll
    :: SHL ea /d /e:.ICON /V $(BASERES).dll

$(BASERES).lib: $(BASERES).dll $(BASERES).def
    IMPLIB $(BASERES).lib $(BASERES).def

$(BASERES).str: $(BASE).STR fm3str.h version.h icons\*.ico
    internal\mkstr
    copy $(BASERES).str .. /v
    cd ..\reskit
    makekit
    cd..\dll

ipf\FM3.hlp: ipf\*.ipf
    cd ipf
    ipfc /CODEPAGE=850 fm3.ipf
    cd..

avv.obj:      version.h
key.obj:      version.h
misc.c:       version.h

