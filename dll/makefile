DEBUG=0
BASE=fm3dll
BASERES=fm3res

.SUFFIXES: .c .rc .ipf

{.}.c.obj:
!IF $(DEBUG)
  ICC.EXE /Kb+ /Ti+ /W3 /Sm /Sp4 /Ss /C /Mp /Gm+ /Gs- /G3 /O- /Q+ /Ge- .\$*.c
!ELSE
  ICC.EXE /Gf+ /Gi- /Kp+ /Kx+ /Kb+ /W3 /Sm /Sp4 /Ss /C /Mp /Gm+ /Gs- /O- /Q+ /G3 /Gt- /Ge- .\$*.c
!ENDIF

!IF $(DEBUG)
LFLAGS = /NOI /ALIGN:2 /EXEPACK /CODEVIEW /M /NOE /BASE:0x20000 /SEG:1024
!ELSE
LFLAGS = /NOL /NOI /ALIGN:2 /EXEPACK:2 /PACKD /PACKC /M /NOE /BASE:0x20000 /SEG:1024
!ENDIF

OFILES= mainwnd.obj dircnrs.obj valid.obj filldir.obj saymsg.obj error.obj\
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
    @REM @<<FM3.@0
    $(LFLAGS)+
    $(LFILES)+
    $(LFILES2)+
    $(LFILES3)
    $(BASE).dll


    $(BASE).def;
<<
    link386 @FM3.@0

$(BASE).lib: $(BASE).dll $(BASE).def
    IMPLIB $(BASE).lib $(BASE).def

init.c: $(BASE).h version.h

string.c: fm3str.h version.h

internal\mkstr.exe: internal\mkstr.c version.h fm3str.h
  cd internal
  compile2 mkstr.c
  cd..

$(BASERES).obj: $(BASERES).c  version.h
  icc /C /Ss /Ge- /W3 /Kb+ /Rn /O+ $(BASERES).c

$(BASERES).res: $*.rc $*.dlg fm3dlg.h
    rc -w2 -r $*

$(BASERES).DLL: $(BASERES).RES $(BASERES).OBJ $(BASERES).DEF
    @REM @<<FM3R.@0
    $(LFLAGS)+
    $(BASERES).obj
    $(BASERES).dll


    $(BASERES).def;
<<
    link386 @FM3R.@0
    rc -x2 $(BASERES).res $(BASERES).dll
    ea /d /e:.ICON /V $(BASERES).dll

$(BASERES).lib: $(BASERES).dll $(BASERES).def
    IMPLIB $(BASERES).lib $(BASERES).def

$(BASERES).str: $(BASE).STR fm3str.h version.h
    internal\mkstr
    copy $(BASERES).str ../v
    cd..\reskit
    makekit
    cd..\dll

ipf\FM3.hlp: ipf\FM3.ipf
    cd ipf
    ipfc /CODEPAGE=850 fm3.ipf
    cd..

avv.obj:      version.h
key.obj:      version.h
misc.c:       version.h

