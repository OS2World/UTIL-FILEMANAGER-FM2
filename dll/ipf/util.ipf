:h1 res=92100 name=PANEL_UTILITIES.Utilities Menu
:i1 id=aboutUtilities.Utilities Menu

FM/2 offers several utilities to make your life a little easier&colon.
:p.
:link reftype=hd res=100045.Remap drives:elink.
.br
:link reftype=hd res=90100.Collector:elink.
.br
:link reftype=hd res=91600.Seek and scan files:elink.
.br
:link reftype=hd res=98500.See all files:elink.
.br
:link reftype=hd res=94900.Compare directories:elink.
.br
:link reftype=hd res=92500.Undelete Files:elink.
.br
:link reftype=hd res=92600.Kill Processes:elink.
.br
:link reftype=hd res=92700.Instant Batch File:elink.
.br
:link reftype=hd res=92800.Command Line:elink.
.br
:link reftype=hd res=95300.INI Viewer:elink.
.br
:link reftype=hd res=94800.View Bookshelf:elink.
.br
:link reftype=hd res=94850.View Helpfiles:elink.
.* :p.
.* :hp2.System info:ehp2. shows you some of OS/2's system variables (those
.* retrieved with the DosQuerySysInfo API, if you're interested).
:p.
For convenience, you can also get to the :hp2.System Clock:ehp2. object,
:hp2.System Setup:ehp2. folder, and command line windows from this menu.

:h2 res=100045 name=PANEL_REMAP.Remap drives
:i1 id=aboutRemap.Remap drives

To remap (attach) a remote server to a local drive letter, enter the
UNC server name in the entry field at the top center of this dialog,
then select the drive letter from the left (attach) listbox to which to
attach the server.  Finally, click the :hp1.Attach:ehp1. button.
:p.
To detach a local drive letter from a remote server, select the
drive letter from the right (detach) listbox, then click the
:hp1.Detach:ehp1. button.
:p.
When you're through remapping drives, click :hp1.Done:ehp1..
:p.
According to IBM LAN Server documentation, a UNC name consists of a
double backslash, the name of the server, another backslash, and the
name of the resource&colon.  \\servername\netname
:p.
Note that FM/2 saves the UNC names you enter in the listbox below the
entry field.  You can recall these names later by clicking on them.
The :hp1.Delete:ehp1. button deletes the currently selected name from
the listbox, and the :hp1.Clear:ehp1. button removes all names from
the listbox.  Names are added automatically.  Up to 200 names can be
stored in this manner (kept on disk between sessions in a file named
RESOURCE.DAT).

.im collect.ipf

.im comp.ipf

:h2 res=92500 name=PANEL_UNDELETE.Undelete Files
:i1 id=aboutUndelete.Undelete Files

:artwork name='..\..\bitmaps\undelete.bmp' align=center.
This leads to a dialog that interfaces with UNDELETE.COM to allow you to
undelete files.  The drive that will be operated on is determined by the
highlighted object in the directory tree.  This dialog filters out files
that already exist on the disk.
:p.
The :hp1.Mask:ehp1. entry field lets you set a mask (which can include
a directory path).  You can switch drives using the dropdown listbox.
A :hp1.Subdirs:ehp1. button lets you choose whether to show files that
can be undeleted in subdirectories as well.
:p.
You can always go directly to UNDELETE.COM if you have the need for more
control.  This is provided only for convenience.

:h2 res=92600 name=PANEL_KILLPROC.Kill Processes
:i1 id=aboutKillProc.Kill Processes
:artwork name='..\..\bitmaps\killproc.bmp' align=center.
This leads to a dialog that allows you to kill most renegade processes.
If you run into a window that just won't close, or one that hides itself
but doesn't quite go away, this may let you kill the hung process.
:p.
Obviously you'll want to exercise some care here.  FM/2 will remove its
own PID (Process ID) from the list, but if you ran it from a command
line you could still kill FM/2 by killing its parent.  You can also kill
off the WPS (PMSHELL.EXE), but it should restart itself.  Some kernal
processes show up but can't be killed.
:p.
A checkbox allows you to set the Process Killer to use the undocumented
DosQProcStat API instead of parsing PSTAT.EXE's output.  While this
removes the requirement of having an English version of PSTAT.EXE, you
should be aware that the DosQProcStat can be changed by IBM without
notice, possibly causing the Process Killer to fail and/or trap.

:h2 res=92700 name=PANEL_INSTANT.Instant Batch File
:i1 id=aboutInstant.Instant Batch File
:artwork name='..\..\bitmaps\instant.bmp' align=center.
This leads to a dialog that lets you quickly hack together a batch
(command) file and run it (the currently highlighted tree directory will
be its default directory).  The command file isn't saved; think of it as
an "extended command line" which allows you to enter more than one line
at a time (for instance, when several tests must be made).

:h2 res=92800 name=PANEL_COMMANDLINE.Command Line
:i1 id=aboutCommandLine.Command Line
:artwork name='..\..\bitmaps\cmdline.bmp' align=center.
This brings up a windowed OS/2 command line.  F9 is the accelerator key
for this command.
:p.
There are also commands to bring up a windowed DOS command line and
a Win-OS/2 full screen session.

.im inis.ipf

:h2 res=94800 name=PANEL_VIEWINFS.View Bookshelf
:i1 id=aboutViewBookshelf.View Bookshelf
FM/2 presents a listbox containing all the .INF files found in the
directories listed in your BOOKSHELF environment variable (see
SET BOOKSHELF= in CONFIG.SYS).  You select the .INF file(s) you want
to view, then click the :hp1.View:ehp1. button.
:p.
If you selected more than one .INF file, FM/2 calls VIEW.EXE in such
a way that all the files are presented at once (a single contents
page appears listing the contents of all the .INF files).  Warning:
Don't select more than one filename with the same title (left column
of listbox) -- View.exe will choke if you do, and be unable to read any
of the files.
:p.
You can enter text into the entry field below the listbox, and FM/2 will
try to find the first listbox entry with matching text as you go.  If
you click :hp1.Select:ehp1. FM/2 will highlight all matching entries
(hint: empty the entry field and click Select to unhighlight
everything).  If you click :hp1.Filter:ehp1. FM/2 will remove all but
highlighted items from the listbox (:hp1.Rescan:ehp1. will refill the
listbox).
:p.
The :hp1.AddDirs:ehp1. button will copy the contents of the entry field
and add it to the directories listed in the BOOKSHELF environment variable.
It should be in the same format as the HELP environment variable uses,
fully qualified directory names separated by semi-colons.  The next time you
use the Bookshelf Viewer, FM/2 will remember this input and use it. This
is an internal addition; your CONFIG.SYS and environment are not
modified.
:p.
The :hp1.Topic:ehp1. entry field can be used to have the INF file(s)
searched on entry for a topic of interest (like typing "VIEW inffile
topic" at a command line).
:p.
When you're done with the dialog, press [ESCape] or click
:hp1.Cancel:ehp1..  Any open .INF files remain open until you close them
(hint&colon. F3 will close an .INF file).
:p.
Note&colon.  this dialog is shared by the :hp2.:link reftype=hd
res=94850.View Helpfiles:elink.:ehp2. and :hp2.View Bookshelf:ehp2.
commands. If you click on either while this dialog is up, the dialog is
simply brought to the foreground.  Close it if you want to switch
function. (This doesn't apply if you started this from the Bookshelf
Viewer object in the FM/2 WPS folder.)

:h2 res=94850 name=PANEL_VIEWHELPS.View Helpfiles
:i1 id=aboutViewHelpfiles.View Helpfiles
FM/2 presents a listbox containing all the .HLP files found in the
directories listed in your HELP environment variable (see SET HELP= in
CONFIG.SYS).  You select the .HLP file you want to view, then click
the :hp1.View:ehp1. button.
:p.
You can enter text into the entry field below the listbox, and FM/2 will
try to find the first listbox entry with matching text as you go.
:p.
The :hp1.AddDirs:ehp1. button will copy the contents of the entry field
and add it to the directories listed in the HELP environment variable.
It should be in the same format as the HELP environment variable uses,
fully qualified directory names separated by semi-colons.  The next time
you use the Bookshelf Viewer, FM/2 will remember this input and use it.
This is an internal addition; your CONFIG.SYS and environment are not
modified.
:p.
When you're done with the dialog, press [ESCape] or click
:hp1.Cancel:ehp1..  Any open .HLP file will remain open until you close
it.
:p.
Note&colon.  this dialog is shared by the :hp2.:link reftype=hd
res=94800.View Bookshelf:elink.:ehp2. and :hp2.View Helpfiles:ehp2.
commands. If you click on either while this dialog is up, the dialog is
simply brought to the foreground.  Close it if you want to switch
function. (This doesn't apply if you started this from the Helpfile
Viewer object in the FM/2 WPS folder.)

