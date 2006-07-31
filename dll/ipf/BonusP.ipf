:h2 res=100095 id='PANEL_BONUSP'.
Bonus Programs
:p.You&apos.ll note the extra objects that the Install program creates in the
FM&slash.2 Tools subfolder of the File Manager&slash.2 folder.  FM&slash.2
is modular&comma. so that you can get directly to some of its components without running the
entire ball of wax.  This may allow you to enhance the behavior of some
of your other applications in the WPS tradition.  You don&apos.t
&us.have&us. to keep these around&comma. of course &endash.&endash. FM&slash.2 itself contains
all their functionality.
:p.:link reftype=hd res=90300.Archive Viewer&slash.2&colon.:elink.  Intended for drag&endash.and&endash.drop
operation &lpar.or WPS association&rpar. with WPS objects or another
applications.  Drag an archive onto it&comma. drop it&comma.
You get an archive listing box.  FM&slash.2&apos.s
installation program sets up some associations between archive
files and this program by file extension &lpar.run INSTALL with "NOASSOC" as
an argument if you don&apos.t want them&rpar..  AV&slash.2 will try to display
whatever you give it as sensibly as it can.
.br
Filename AV2.EXE.
:p.:link reftype=hd res=95000.EA Viewer&colon.:elink. Drag a file system object onto it and it&apos.ll
show you the object&apos.s extended attributes.
.br
Filename EAS.EXE.
:p.:link reftype=hd res=95300.INI Viewer&colon.:elink. Drag an .INI file onto it and it&apos.ll show you its
contents.
.br
Filename INI.EXE.
:p.Bookshelf Viewer&colon.   Shows all .INF files in a listbox and lets
you pick the one&lpar.s&rpar. you want to view.  If you
give any command line argument&comma. the .HLP files on the
HELP path will be shown instead &lpar.the Helpfile Viewer
object calls VIEWINFS.EXE with "dummy" for an
argument&comma. for example&rpar..
.br
Filename VIEWINFS.EXE.
:p.Process Killer&colon. Lets you kill off renegade processes.  An
English version of PSTAT.EXE must be on your PATH.
.br
Filename KILLPROC.EXE.
:p.Undeleter&colon. Lets you undelete files &lpar.via interface with
UNDELETE.COM&rpar..  Drag a file system object
onto it and it&apos.ll let you undelete files for that drive.
&lpar.This only works if you have OS2's Del directories setup in Config.sys&rpar.
.br
Filename UNDEL.EXE.
:p.Visual Tree&colon. Opens a Drive Tree window &lpar.like the WPS
Drives object with more horsepower&rpar..
.br
Filename VTREE.EXE.
:p.Visual Directory&colon. Opens a Directory Container window&semi. drag a
file system object onto it and this will open its directory &lpar.like a WPS directory Folder
with more horsepower&rpar..
.br
Filename VDIR.EXE.
:p.:link reftype=hd res=90100.Collector&colon.:elink. Opens a Collector window.
.br
Filename VCOLLECT.EXE.
:p.
Two other objects&comma. "See all files" and "Seek and scan" are created which
call up the Collector and go directly to dialogs for the appropriate purpose.
:p.Global Viewer&colon. Opens a global view of a drive or drives.
.br
Filename GLOBAL.EXE.
:p.:link reftype=hd res=99000.Databar&colon.:elink. Opens a databar showing some system information.
.br
Filename DATABAR.EXE.
:p.DirSize&colon. Shows where drive usage is concentrated.
.br
Filename DIRSIZE.EXE.
:p.:link reftype=hd res=100000.FM&slash.2 Lite&colon.:elink.
 A simplified interface for "dummies."
.br
Filename FM4.EXE.
:p.FM&slash.2&apos.s install creates FM2.CMD&comma. AV2.CMD&comma.
VDIR.CMD&comma. VTREE.CMD&comma. VCOLLECT.CMD&comma.
UNDEL.CMD&comma. KILLPROC.CMD&comma.
INI.CMD&comma. EAS.CMD&comma. DIRSIZE.CMD&comma.
VIEWINFS.CMD and VIEWHELP.CMD files for you in a subdirectory "utils" below
the install directory.  You should add this directory to your PATH&eq.
statement in CONFIG.SYS &lpar.type HELP PATH at a command line for more
info&rpar..  If you prefer&comma. FM&slash.2&apos.s INSTALL builds a
SETENV.CMD in the FM&slash.2 directory which you can call to set the PATH for
FM&slash.2 in any given session without modifying CONFIG.SYS.
