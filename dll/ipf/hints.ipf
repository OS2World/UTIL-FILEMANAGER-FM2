.***********************************************************************
.*
.* $Id$
.*
.* fm/2 help - Hints
.*
.* Copyright (c) 1993-98 M. Kimes
.* Copyright (c) 2002-2012 Steven H.Levine
.*
.* 26 Jun 11 GKY Removed Resource.ipf which was out dated and no longer relevant.
.* 09 Oct 11 GKY Fixed line length to eliminate the need for horizontal scrolling.
.* 08 Jan 12 GKY Added drive flag tips for Netdrive drives.
.* 01 Jan 13 GKY Add section on FM/2 as a workplace shell replacement
.* 01 Jan 13 GKY Add Netdrive section to the index
.*
.***********************************************************************
.*
:h1 res=93000 name=PANEL_HINTS.Hints and troubleshooting
:i1 id=aboutHints.Hints and troubleshooting

:artwork name='bitmaps\secret.bmp' align=center.
:p.
This section contains hints about obscure functions, FM/2 "secrets" and
other rubbish.
.br
:p.
With most video display drivers, :color fc=default bc=cyan.chording:color fc=default bc=default. 
a directory in the Drive Tree (or clicking the :color fc=default bc=cyan.B3:color fc=default bc=default. 
on a three button mouse) will cause FM/2 to open a new FM/2 Directory Container window for that 
directory without further ado.
:p.
Want to open FM/2 with various different configurations see :link reftype=hd res=100125.Starting FM/2:elink.
for command line options.
:p.
Want to copy or move some files to a new directory?  Try dragging them
onto an empty area (whitespace) of the Drive Tree window. FM/2 will pop up a
dialog allowing you to specify a name for a new directory into which to
place the files.
:p.
To set the label of a drive, pick :link reftype=hd
res=90900.Files->Info:elink. (&CtrlKey. + :color fc=default bc=palegray.i:color fc=default bc=default. 
accelerator) on the root of the drive in the Drive Tree container. You'll find :hp1.Format:ehp1. and
:hp1.Chkdsk:ehp1. in that menu, too, under the :hp1.Miscellaneous:ehp1.
submenu.
:p.
Remember, when using the :link reftype=hd res=93900.internal
editor:elink. (but don't, use your favorite editor instead), FM/2 saves
the file as it appears in the MLE. If you don't want long lines wrapped
(such as when editing CONFIG.SYS), turn wrap OFF before saving.
:p.
If you want to change the fonts used in the :link reftype=hd
res=91500.Walk Directories:elink. dialog, drop a new font from the Font
Palette onto a blank area of the dialog (not a control). The new font
will be used in the directory listboxes and path entry field the next
time you use the dialog. This is sometimes necessary when the default
font for the dialog doesn't match up well to the codepage in use for
non-US users, as non-ASCII characters might show up improperly.
:p.
To invoke an OS/2 WPS association rather than an FM/2 internal
association, try Open->Default from a context menu on the file object
(F6 accelerator) or holding down the &CtrlKey.
 key while double-clicking the object. Alternatively, use <> as the command line for an :link
reftype=hd res=90400.association:elink. and it'll open the object's
default WPS view, which will run the program associated with the object
under the WPS if there is one. Follow the hypertext link in this
paragraph for detailed explanations.
:p.
If you periodically do something to the same set of files, you might
consider making a :link reftype=hd res=96000.List:elink. of the files
and :link reftype=hd res=90100.Collecting:elink. them from that list
file so you don't have to reselect them when you want to manipulate them
later. Note that FM/2 has a Reselect command under the Views->Select
menu, but it only remembers the last selection set -- using the
List/Collect method you can reselect even in another session.
:p.
Using :hp1.AV/2:ehp1. from the WPS&colon. Drag an archive file onto the
AV/2 object; this opens a view into the archive. Drag any files you
want added to the archive into the container; they're added to the
archive. To create a new archive, drag the objects to be archived onto
the :hp1.Make Archive:ehp1. object.
:p.
For advanced users&colon. You can add commands to FM/2's action bar
menu. Create a file in FM/2's directory called FM3MENU.DAT. Here's a
sample&colon.
:xmp.
;
;Items listed in this file are added to FM/2's action bar (pulldown) menu.
;First word in a line MUST be MENUITEM. Next comes ID of command (see
;FM3TOOLS.DAT). Finally, the text to display on the menu for the command.
;
;Any line beginning with a semi-colon, like this one, is a comment.
;
MENUITEM 1023 V~iew
MENUITEM 1024 ~Edit
MENUITEM 1010 I~nfo
MENUITEM 1009 ~Attrs
MENUITEM 1017 ~Open
MENUITEM 1006 ~Kill
:exmp.
:p.
If you want to set the extract directory in an :link reftype=hd
res=90200.Archive Container:elink. to the same directory as the archive
is in, start a drag from the recessed text field next to the :link
reftype=hd res=91900.Folder button:elink. and drop onto the Folder
button. If you always want the extract directory to be the same as the
directory in which the archive resides, enter * in the Ext. Path field
of the internal Settings Notebook's or if you want FM/2 to create a subdirectory
based on the archiver name leave Ext. Path blank and check the box Use archive
name as extract path in container (Ext. Path over rides this setting)
:link reftype=hd res=99940.Archivers page:elink..
:p.
You can drag files or directories onto an archive object in a Directory
Container, and FM/2 will display the :link reftype=hd
res=90300.Archive:elink. dialog to allow you to add those files to the
archive without having to first open the archive.
:p.
If a Directory Container is in Details view, and if the titles above the
columns are turned on, you can hold down :color fc=default bc=palegray.ALT:color fc=default bc=default.
 and click a title to cause the container to sort on that field (assuming the field is one of those
on which FM/2 will sort -- filename, size, EA size, and dates are all
valid). Works in Archive Containers, too.
:p.
Can't set the :hp3.default sort or view for new Directory
Containers:ehp3.?  Yes you can -- use the :link reftype=hd
res=97100.internal Settings notebook:elink. instead of the popup menus.
The popups only change the :hp1.current:ehp1. container -- the one on
which you requested the context menu. The Settings notebook determines
how new containers that you open will appear. This is a distinction
often overlooked by new users.
:p.
Old DOS hands will know this, but you can enter :hp2.PRN:ehp2. when you
want output to go to a printer rather than a disk file. You can even
specify different printers using LPT? (i.e. LPT1, LPT2, LPT3, etc.).
:p.
If nothing shows in a Directory Container, Archive Container, the
Collector or Drive Tree although you :hp1.know:ehp1. there's something
in it, check your Filter (&CtrlKey.
 + :color fc=default bc=palegray.f:color fc=default bc=default. accelerator). The Filter button for
the appropriate container will show the current filter status for the
current container (F&colon.<All> means everything is visible, <Attr>
means attributes are being used to filter, otherwise you'll see the
current mask set). Remember to look at the attributes as well as the
mask. The :hp1.All:ehp1. button in the :link reftype=hd
res=93400.Filter dialog:elink. can be used to ensure that everything is
visible.
:p.
To compare the directories of two open Directory Containers without
resorting to the Drive Tree, link-drag from the recessed status window
in one Directory Container to the recessed status window in the other.
Remember, when you drag :hp1.from:ehp1. one of those recessed areas, you
drag the directory the Directory Container is "looking" into, and when
you drop :hp1.on:ehp1. one of them, you drop into the directory the
Directory Container is "looking" into. They behave, for drag and drop
and requesting context menus, like empty container space (whitespace).
:p.
To begin a direct edit of the current file's name using the keyboard,
use &CtrlKey. + :color fc=default bc=palegray.F10
:color fc=default bc=default.. To end (complete) the direct edit, use  &CtrlKey. +  
:color fc=default bc=palegray.F11:color fc=default bc=default.. To
cancel an edit underway, use :color fc=default bc=palegray.Esc:color fc=default bc=default..
:p.
PM uses several :color fc=default bc=palegray.ALT:color fc=default bc=default. + 
:color fc=default bc=palegray.F?:color fc=default bc=default. accelerators to control frame windows. However,
this applies to the current frame window, which can be within the main
FM/2 window when run monolithically. Add &CtrlKey. to these accelerators to
affect the main window in that case. For example, &CtrlKey.  + :color fc=default bc=palegray.Alt
:color fc=default bc=default. + :color fc=default bc=palegray.F9:color fc=default bc=default. will
minimize the main FM/2 window, while :color fc=default bc=palegray.Alt:color fc=default bc=default.  + 
:color fc=default bc=palegray.F9:color fc=default bc=default.  will minimize whichever
frame window has the focus.
:p.
If you have the bottom buttons turned on in FM/2 and have a 3-button
mouse, clicking the  :color fc=default bc=cyan.B3:color fc=default bc=default. (or holding down &CtrlKey.  
while clicking :color fc=default bc=cyan.B1:color fc=default bc=default.) on them will change your sort type.
:p.
Problem with ZIP or EZ drive -- slow scanning&colon. See :link
reftype=hd res=99980.Edit->Drive flags:elink. command. The problem
is that these drives respond :hp1.very:ehp1. slowly to some commands
and requests. You can tweak the drive's flags to alleviate some of
this.
:p.
Problem with details view refreshing -- top items come up blank. This
is one of those never-fixed OS/2 bugs. Try :hp1.un-checking:ehp1. the
Immediate updates toggle in the Settings notebook. This usually works
around this bug.
:p.
To find any directory in the Drive Tree quickly, type  &CtrlKey.  +
&ShiftKey.  +  :color fc=default bc=palegray.F:color fc=default bc=default.
with the Drive Tree active, then type in the pathname of the directory
that you want to find. To find the directory of a Directory Container
in the Drive Tree, type &CtrlKey. + &ShiftKey.+ 
:color fc=default bc=palegray.F:color fc=default bc=default. with the Directory Container active.
:p.
The :link reftype=hd res=90100.Collector:elink. can search for files
based on a variety of criteria. The search function can also find
potential duplicate files for you. Potential duplicates can also be
tracked down in the :link reftype=hd res=98500.See all files:elink.
control.
:p.
You can selectively turn off FM/2's bubble help. Use the internal
Settings notebook, turn to the :hp1.Bubbles:ehp1. page, and read the
help.
:p.
"I double-clicked on an INI file and FM/2 showed it in the text viewer."
The INI file may not be a standard OS/2 INI file (Windoze programs, for
example, use *.INI files that are flat text files). FM/2 will "fail"
quietly to view such files using the INI viewer when you double-click
them since there's no reason to bother you each time you want to look at
one. If you're sure the file is a standard OS/2 INI file, use the
Utilities menu to open the INI viewer, then choose Files->Other profile
and enter the name of the file. FM/2 will then tell you the reason that
it is unable to open the file -- you probably want to know about it since
you're already in the INI viewer.
:p.
Trouble starting FM/2:  "Resource not found."  Probably mismatched DLL
and EXE files -- re-extract FM/2 from the distribution archive, being
particularly sure to overwrite :hp1.all:ehp1. old DLL and EXE files.
:p.
FM/2 crashes when attempting to view a file:  Un-check :link reftype=hd
res=92200."Check for multimedia w/ MMPM/2":elink. -- see explanation
there.

:h2 res=100010 name=PANEL_HINTSCMD.Command lines
:i1 id=aboutHintsCmd.Command lines

If you want to directly execute a self-extracting archive (or anything
else, for that matter -- I mention self-extracting archives because FM/2
will attempt to view them rather than run them when you double-click
them), press &CtrlKey. + :color fc=default bc=palegray.
F5:color fc=default bc=default.. This brings up the :link reftype=hd
res=90600.Command line:elink. dialog with the cursored file in the entry
field.
:p.
You can press the [Home] key to move the cursor to the start of the
entry field and enter the name of a program to run with the file as an
argument, or just press [Enter] to execute the file.
:p.
Alternatively, if running FM/2 monolithically, press :color fc=default bc=palegray.F5
:color fc=default bc=default. to get a
miniature command line at the bottom of the window. Enter any commands
you'd like here, including :link reftype=hd
res=100075.meta-strings:elink.. Type /HELP in the command line entry
field for brief information specific to this control.
:p.
Note that if you use the same command line time after time you will
probably want to set up a :link reftype=hd res=90700.Command:elink.
for the command line to save time. With a Command, you can even
execute a command line using an accelerator key -- can't beat that
for speed.

:h2 res=100140 name=PANEL_HINTSWPSREPLACE.FM/2 as WPS replacement
:i1 id=aboutHintswpsreplace.FM/2 as WPS replacement
:p. FM/2 makes a useful replacement for OS2's workplace shell. It allow for you to manipulate eas and the OS2.ini 
and OS2SYS.ini without the work place shell running. We recommend you setup commands and a toolbar. These
should include your favorite ini cleaners ea manipulation tools and basic utilities like pstat and rmview. 
You will also want to include pmshell so you can just start the WPS when you are ready. It is easy to setup.
Just add the following line to your config.sys file&colon.
.br
.br
SET RUNWORKPLACE=G&colon.\Fm2\fm3.exe
.br
.br
Remember to REM the following line&colon.
.br
.br
SET RUNWORKPLACE=C&colon.\OS2\PMSHELL.EXE

:h2 res=100135 name=PANEL_NETDRIVE.Netdrive drive flag suggestions
:i1 id=aboutHintnetdrive.Netdrive drive flag suggestions
:p.The Netdrive file system can slow FM/2 down to a crawl and even lock FM/2. This is particularly true of the FTP
plugin but other plugin can also cause it. We suggest the drive flags shown below be used for any Netdrive's drive.
that display this behavior. 
.br
.br
:artwork name='bitmaps\NDflags.bmp' align=center.

.im bonusp.ipf

.im fm2util.ipf

.im trouble.ipf

.im errors.ipf

.*.im resource.ipf


