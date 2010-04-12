.***********************************************************************
.*
.* $Id$
.*
.* fm/2 help - Configuration dialogs usage
.*
.* Copyright (c) 1993-98 M. Kimes
.* Copyright (c) 2002, 2009 Steven H.Levine
.*
.* 29 Apr 09 SHL Update formatting
.*
.***********************************************************************
.*
:h1 res=92000 name=PANEL_CONFIG.Config Menu
:i1 id=aboutConfig.Config Menu

:artwork name='bitmaps\tweak.bmp' align=center.
:p.
FM/2 can be configured as you like it using the commands under this
menu. It is highly recommended that you experiment with the items in
this menu when you first begin to use FM/2, both to familiarize
yourself with the available configuration options and to make FM/2
work the way you like it to work.
:p.
This menu affects general FM/2 behavior. Each class of container has
its own configuration menu that allows you to set the type of view,
filtering, and so on. To get the popup menu that controls the
container's appearance, request a :link reftype=hd res=93700.context
menu:elink. while the pointer is over an empty area of the container, or
select the Views pulldown submenu.
:p.
The :link reftype=hd res=94600.Settings notebook:elink. is accessed from
this menu. You can use the submenu to access the specific page you want
otherwise it opens to the last page you accessed.
:p.
The :link reftype=hd res=100065.Set Target directory:elink. is the next menu
item and can also be accessed using &CtrlKey.+:color fc=default bc=palegray.T:color fc=default bc=default..
:p.
Five menu items to toggle various FM/2 functions on and off are next.
The first two :link reftype=hd res=91800.Toolbar:elink. and :hp6.Autoview:ehp6.
have submenus.
:p.
The :hp6.Toolbar:ehp6. submenu default is to toggle the toolbar on and off
The other two items allow you to make the toolbar text only or you can
add titles under the toolbar icons. Additional functions to edit the toolbar
can be found on the context menu obtained by clicking :color fc=default bc=cyan.B2:color fc=default bc=default. over a tool icon
on the toolbar (see :link reftype=hd res=91800.Toolbar:elink. for more information)
You can switch between toolbars using the dialog that appears if you click :color fc=default bc=cyan.B2:color fc=default bc=default. over
an area of "white space" on the toolbar or by selecting load toolbar from the previously
discussed context menu.
:p.
The :hp6.Autoview:ehp6. submenu controls the Autoview window. The
default for this conditional cascade menu is the :hp6.Toggle autoview
window:ehp6. command, which causes an autoview window to appear above
the status line at the bottom of the screen. As you move the cursor
from object to object, FM/2 displays the first few lines of file objects
in this window. If the autoview window is already displayed, clicking
this command causes it to disappear. You can also set the what is to
be autoviewed -- either the file's .COMMENTS EA or the start of the
file's contents (similar to the *nix HEAD program). When .COMMENTS EAs
are being viewed, you can edit them and the changes will be saved when
you switch the focus from the Autoview window. You can reach this
editable Autoview window with the  &CtrlKey. +  :color fc=default bc=palegray.Tab:color fc=default bc=default. hotkey if you are allergic
to your mouse.
:p.
Clicking the contents Autoview window with :color fc=default bc=cyan.B1:color fc=default bc=default. causes the file to be
viewed. Clicking with :color fc=default bc=cyan.B3:color fc=default bc=default. (or  :color fc=default bc=cyan.chording:color fc=default bc=default. with :color fc=default bc=cyan.B1:color fc=default bc=default. and :color fc=default bc=cyan.B2:color fc=default bc=default. simultaneously)
causes the extended attributes to be viewed. If viewing .COMMENTS
rather than contents, you can pick :hp1.View file:ehp1. from the
context menu.
:p.
The :hp6.:link reftype=hd res=99400.Toggle quicklists:elink.:ehp6.
command causes a set of dropdown listboxes to appear below the toolbar
and above other windows. The listboxes include&colon.
.br
A Drive Finder dropdown listbox.
.br
A States dropdown listbox (Fm/2 configurations you have saved with specific name).
.br
A Commands dropdown listbox (External command that have been added to FM/2).
.br
A Directory dropdown listbox (Directory names you've assigned in the
:link reftype=hd res=91500.Walk Directories:elink. dialog).
.br
A Toolbars dropdown listbox also appears if the :hp6.Toolbar:ehp6. is on.
:p.
The :hp6.Toggle bottom buttons:ehp6. menu item turns off and on a row of
buttons that appear just above the status line(s). The buttons display
the name, date, and attributes of the currently selected object, and the
filter status of the current container. If clicked with :color fc=default bc=cyan.B1:color fc=default bc=default., a command
is generated (rename, info, edit date/attributes and filter dialog
respectively). If clicked with :color fc=default bc=cyan.B2:color fc=default bc=default., a context menu appears (the same one
you get if you click :color fc=default bc=cyan.B2:color fc=default bc=default. on the first status line). If clicked with :color fc=default bc=cyan.B3:color fc=default bc=default.,
the sort changes for the current container:  filename, last write date,
file size and reverse sort respectively.
:p.
The :hp6.Toggle drivebar:ehp6. menu item turns off and on a bar showing
all available drives. You can click these drive buttons to find or
switch to a drive (depending on the active window when the button is
clicked), drag objects onto the buttons, request a context menu on a
button for more commands dealing with the drive, or click :color fc=default bc=cyan.B3:color fc=default bc=default. to open a
Directory Container for that window (or surface and activate one that
already exists).
:p.
The menu also provides access to the dialogs for editing
:link reftype=hd res=90400.Associations:elink.,
:link reftype=hd res=90700.Commands:elink. and :hp6.Archivers:ehp6.
(:link reftype=hd res=94200.Edit Archiver Data:elink.)
:p.
To change fonts and colors, FM/2 uses the WPS Font and Color Palettes.
The Config menu contains commands to call up these objects for you.
:p.

:p.
See also&colon.
.br
:link reftype=hd res=97600.FM/2 window layout:elink.
.br
.im notebook.ipf

:h2 res=100065 name=PANEL_TARGETDIR.Set Target directory
:i1 id=aboutTargetDir.Set Target directory

You can set a default Target directory with the :hp6.Set Target
directory:ehp6. command, which FM/2 will use whenever you use the menu
or accelerator key commands to move or copy file system objects.
Otherwise, FM/2 will attempt to intelligently guess what you'd like the
target directory to be each time based on open Directory Containers or
the currently selected directory in the Drive Tree.
:p.
If you have a Target directory already set and wish to clear it,
select this command, click &CancelButton. at the :link reftype=hd res=91500.Walk
Directories:elink. dialog that appears, and answer Yes to the question
subsequently asked.
:p.
See also the :link reftype=hd res=99950.General page:elink. of the
internal Settings notebook.

.im assoc.ipf

.im command.ipf

:h2 res=94200 name=PANEL_AD_FRAME.Editing Archivers
:i1 id=aboutEditArc.Editing Archivers
:p.
This dialog box, which you can reach from the Config menu, allows you to manage your
:link reftype=hd res=100015.archivers:elink..
:p.
On the left side is a list of the currently defined archivers for FM/2. On the right side are a series of buttons&colon.
:dl compact.
:dt.:hp2.Up:ehp2.:dd.Moves the highlighted archiver up in the archive list. (See note below.)
:dt.:hp2.Down:ehp2.:dd.Moves the highlighted archiver down in the archive list. (See note below.)
:dt.:hp2.Add:ehp2.:dd.Add brings up a blank archiver details box so you can define a new archiver type.
:dt.:hp2.Delete:ehp2.:dd.Deletes the archiver description of the highlighted archiver.
:dt.:hp2.Revert:ehp2.:dd.Revert undoes all changes made during the current session. It will not undo changes once the new ARCHIVER.BB2 has been written.
:edl.
:note.When FM/2 determines which archiver to use on a file, they are checked in order of this listing.
:p.
The specifics of which archivers FM/2 can use and how FM/2 will use them are stored in a file named
:link reftype=hd res=100130.ARCHIVER&period.BB2:elink..
This file is located in the FM/2 installation directory and may be edited manually with a text editor.
:p.
Alternatively FM/2 also provides a GUI which can be used to edit a specific archiver. Simply select
the archiver you wish to edit and select the
:hp2.Edit:ehp2. button at the bottom.
:p.
When the
:hp2.Close:ehp2. button
is selected a dialog asking you if you wish to rewrite
ARCHIVER.BB2 will appear if changes have been made.

:h3 res=94300 name=PANEL_ARCFLDS.Archiver Details Fields
:i1 id=aboutArcFlds.Archiver Details Fields
:p.
:hp2.
.ce Archiver Details: General Notes
:ehp2.
:note.In order for an archiver to work properly, make sure
:ol compact.
:li.the program(s) is/are in a directory named in your PATH. (Alternatively, full path names can be used.)
:li.the DLL's the programs need, if any, are in the LIBPATH.
:li.the names of the programs are entered correctly.
:li.the command syntax is correct and does not prompt for further input.
:li.the start and end of list strings are correct.
:li.the signature and signature position, if any, are correct.
:eol.
:p.
In the event that an error occurs when you attempt to list an archive,
you'll be given an
opportunity to use this dialog to fix the entry. In this case, you will
see the listbox at the right of the dialog filled with the listing of
the archive that your archiver made. You can highlight a line and click
the << button next to the Start List or End List fields to move the line
to that entry field (these are the most common mistakes, and FM/2 cannot
find any files if the Start List string is wrong). You can double-click
on a listbox line to have FM/2 "parse" it into the Fld# text boxes for
you, to make it easier to judge field positions for sizes, dates, and
filenames. The filename field in particular is extremely important. If
it's too high, FM/2 finds no files. If it's "in range" but wrong, FM/2 gets
the wrong fields for filenames.
:p.
FM/2 can't do everything for you. You may still need to refer to your
archiver's documentation and/or run it
to get the help on its command syntax, the format used when listing the
contents of archives and/or its signature info.
:p.
When you've completed editing the archiver's details, click &OkayButton.. FM/2
will ask you if you want to rewrite ARCHIVER.BB2. If you don't
rewrite ARCHIVER.BB2, changes are good only
for the current session (handy for testing).
:p.
One generation of backup for ARCHIVER.BB2 is kept. It is named Archiver.BAK.
:p.
:hp2.
.ce Archiver Details: Fields
:ehp2.
:p.
:hp2.ID:ehp2. This field contains an ID for the archiver; something for
human consumption. It's a good idea to include the version number of the
archiver for reference. An example might be "LHArc 2.11".
:p.
:hp2.Add:ehp2. This field should contain the command that creates
and adds files to an archive. An example might be "PKZIP.EXE -a" (NOTE:
This example assumes the file is on your PATH (see PATH= in CONFIG.SYS).
If it's not, you'd need to give a full pathname, like
"C&colon.\UTILS\PKZIP.EXE -a".) Note that commands should include the
extension so that FM/2 can check them without guessing. Above this
field is a button labeled "See." If clicked, the program named in this
field will be run (you'll be given the opportunity to add arguments to
the command line) in another window. This lets you check command syntax
and archiver version, as well as assuring that FM/2 can find your archiver
executables.
:p.
:hp2.Move:ehp2. This field should contain the command that moves files
to the archive (adds then deletes the file). An example might be
"ARC.EXE mwn".
:p.
:hp2.Extension:ehp2. This field contains the extension normally
associated with files created by this archiver. An example might be
"ZOO" for files created by the Zoo archiver.
:p.
:hp2.Extract:ehp2. This field contains the command that extracts files
from the archive. This command should not delete the files from the
archive when it extracts them, and *must* be present for FM/2 to show you
a member of the archive (commands other than Extract and List may be
left blank if necessary). An example might be "PKUNZIP.EXE -o". Note
the "-o" option given; this tells PKUNZIP to automatically overwrite any
existing files (FM/2 will check to see if any of the files exist and warn
you if so). It's important to always include your archiver's "don't
stop for user input" option; some things occur as detached processes and
you can't interact with them; the program would be hung, which is
uncool. Above this field is a button labeled "See." If clicked, the
program named in this field will be run (you'll be given the opportunity
to add arguments to the command line) in another window. This lets you
check command syntax and archiver version.
:p.
:hp2.Extract w/Dirs:ehp2. This field contains the command that extracts
files from the archive and places them into directories embedded in the
archive. An example might be "LH.EXE x /o /s".
:p.
:hp2.Signature:ehp2. This field contains the signature for the archive
type. There is usually a byte or few in a particular place in any
archive that indicates that it is, indeed, an archive of that type. FM/2
uses these signatures to "sniff out" which archiver is used to
manipulate the archive. Since these signatures sometimes contain
characters which are "unprintable," you can use \x<hexnum> to represent
any "strange" characters. A side effect of this is that two backslashes
are required to represent a single backslash ("\\" == "\"). See
:link reftype=hd res=99500.C-style encoding:elink. for more information.
:p.
To determine what an archiver's signature is, either ask the archiver's
author or check several different archives of the type for one or more
bytes present in each at the same location, usually near the beginning
of the file.
:p.
This field must be entered and valid for FM/2 to detect this type of
archive (see also Sig(nature) Pos(ition)).
:p.
:hp2.List:ehp2. This field contains the command to list the archive's
contents. This command *must* be present and correct for FM/2 to work
properly with this type of archive. An example might be "ZOO.EXE v".
:p.
:hp2.Test:ehp2. This field contains the command to test the archive's
integrity. An example might be "PKUNZIP.EXE -t".
:p.
:hp2.Add with paths:ehp2. This field contains the command to add files
to the archiver with their paths (i.e. \FM3\FM3.EXE instead of just
FM3.EXE). This can be omitted if the archiver doesn't support the
command. An example might be "ZIP.EXE" (Zip defaults to adding paths).
:p.
:hp2.Move with paths:ehp2. As above, but moves the files instead of
just adding them. An example might be "PKZIP -m -P".
:p.
:hp2.Add and recurse:ehp2. Adds files to the archive, with paths, and
recurses into subdirectories. An example might be "LH a /s".
:p.
:hp2.Delete:ehp2. This field contains the command to delete files from
the archive. An example might be "LH.EXE /o /d".
:p.
:hp2.Sig(nature) Pos(ition):ehp2. This field contains a number
indicating how many bytes into the file the signature is located. If
this number is negative, FM/2 looks from the end of the file instead of
the beginning.
:p.
:hp2.F(ile)Name Pos(ition):ehp2. This field tells FM/2 which field on
the line of an archive listing is the file name.
Archive listing fields are numbered from 0.
The file name position item consists of 4 subfields separated by commas.
The first is the field number.
The second is a flag which is set to 1 to indicate that the file
name is the last field on the line.
This allows unquoted archive member names to contain spaces
The third is a flag which is set to 1 to indicate that the file name starts
at the second character of the field.
This allows files that are surrounded by unusual bracketing characters.
The fourth is a flag which is set to 1 to indicate the the file name
stands alone at the first field in the listing line and
the file details are on the next line.
:p.
To understand what "field on the line of an archive listing" means,
think of a text line as being broken up into tokens, or words, separated
by spaces. These tokens, or words, are fields. Therefore,
:p.
I like Ike.
:p.
contains three fields. Field 0 is "I", field 1 is "like", and field 3
is "Ike."  Think of it like this&colon.
.br
:xmp.
 +--------------------+
 |  0   |  1   |  2   | Field Numbers
 +------+------+------+
 |  I   | like | Ike  | Field Contents
 +------+------+------+
:exmp.
:p.
:hp2.OldS(i)z(e) Pos(ition):ehp2. This field tells FM/2 which field on
the line of an archive listing is the old (uncompressed) size of the
file. If this isn't available or you don't care about it, you can enter
a -1 to disable detection of this field entirely.
:p.
:hp2.NewS(i)z(e) Pos(ition):ehp2. This field tells FM/2 which field on
the line of an archive listing is the new (compressed) size of the file.
If this isn't available or you don't care about it, you can enter a -1
to disable detection of this field entirely.
:p.
:hp2.Date Pos(ition):ehp2. This field tells FM/2 which field on
the line of an archive listing is the date/time string. If this
isn't available or if you don't care about it, you can enter a -1 to
disable detection of this field entirely. You can optionally follow this
number with a comma and another number that indicates the type of the
date from any of the formats in the following list&colon.
.br
:xmp.
 0 = Other (or no date/time if numdateflds is 0)
 1 = 02-08-96  23&colon.55&colon.32 mm-dd-yy hh&colon.mm&colon.ss
 2 = 8 Feb 96  23&colon.55&colon.32 dd Mmm yy hh&colon.mm&colon.ss
 3 = 8 Feb 96  11&colon.55p   dd Mmm yy hh&colon.mmA
 4 = 96-02-08  23&colon.55&colon.32 yy-mm-dd hh&colon.mm&colon.ss
 5 = 31-02-98  23&colon.55    dd-mm-yy hh&colon.mm
:exmp.
:p.
Dash (-) and slash (/) separators are both supported.
:p.
Both 2 digit and 4 digit years are supported.
:p.
2 digit years slide about 1980.
:p.
:hp2.NumDateF(ie)lds:ehp2. This field tells FM/2 how many fields comprise
the time/datestring.
:p.
:hp2.Start-of-list:ehp2. The line that comes just before the list of
files in the archiver listing (see example below). You can use the
:hp2.<<:ehp2. button to insert a selected line directly from the listbox
into this field.
:p.
:hp2.End-of-list:ehp2. The line that comes just after the list of
files in the archiver listing (see example below). You can use the
:hp2.<<:ehp2. button to insert a selected line directly from the listbox
into this field.
:note.A few archivers, like TAR, do not have Start-of-list and/or End-of-list strings.
For these archivers use the string&colon.
.br
:lm margin=5.:color fc=blue.None:color fc=default.:lm margin=1.
.br
for the Start-of-list and/or End-of-list strings.
:p.
Here's an example of a listing of a ZIP file. You may
need to widen the help windows for this to look right...)&colon.
:xmp.
 Length   Method    Size  Ratio   Date   Time   CRC-32    Name
--------  ------  ------- -----   ----   ----   ------    ----     <-- Start-of-list
    2201  Defl&colon.N      978  56%  09-13-08 02&colon.54  f85e9c1b  fm3.c
     405  Defl&colon.N      283  30%  11-22-08 23&colon.47  718f2fa2  fm3.def
  192744  Defl&colon.N    61541  68%  12-02-08 04&colon.15  2b72b37b  fm3.exe
     176  Defl&colon.N      129  27%  11-30-08 21&colon.59  5db66032  FM3.INI
     171  Defl&colon.N      122  29%  12-02-08 04&colon.15  3d2bc616  fm3.lrf
   14680  Defl&colon.N     2366  84%  12-02-08 04&colon.15  a9fee027  fm3.map
    3276  Defl&colon.N     1951  40%  11-30-08 15&colon.15  35c6c7ac  fm3.obj
    6237  Defl&colon.N     1553  75%  11-18-08 05&colon.43  6ed9c658  fm3.rc
  198905  Defl&colon.N    56365  72%  11-22-08 23&colon.05  1e982a85  fm3.res
--------          -------  ---                            -------  <-- End-of-list
  418795           125288  70%                            9 files
:exmp.
This listing "parsed" into FM/2 archiver details fields&colon.
:table cols='15 10 50' rules=vert frame=rules.
:row.
:c.FM/2 Archiver detail field
:c.Value
:c.Notes
:row.
:c.FName Pos
:c.7,1,0,0
:c.7&colon. The filename starts in field 7 (start counting from 0!)
:row.
:c.
:c.
:c.1&colon. If there are spaces in the filename, it extends to the end of the line
:row.
:c.
:c.
:c.0&colon. No special bracketing characters
:row.
:c.
:c.
:c.0&colon. The listing does not use two lines per file.
:row.
:c.OldSz Pos
:c.0
:c.The uncompressed size of the file is in field 0.
:row.
:c.NumDateFlds
:c.2
:c.The date and time consume 2 fields.
:row.
:c.Date Pos
:c.4,1
:c.The date/time data starts in field 2 and the format is mm-dd-yy hh&colon.ss.
:row.
:c.NewSz Pos
:c.2
:c.The compressed size of the file is in field 2.
:etable.

:h4 res=100130 name=PANEL_ARCBB2.ARCHIVER.BB2 Structure
:i1 id=aboutArcBB2.ArchiverBB2 Structure
:p.
ARCHIVER.BB2 is a text file and can be viewd or edited with any text editor. It contains are three types of lines&colon.
:ol compact.
:li.The first line in the file is the number of lines per archiver definition in the ARCHIVER.BB2 file.
It is very important; do not change it.
It allows modifications to the file format to be transparent to older programs.
:li.Comment lines.
:ul compact.
:li.Comment lines are optional but recommended.
:li.A semicolon in column 1 marks a comment line.
:li.Comment lines may appear anywhere :hp3.except:ehp3. within the lines of an archiver definition entry.
:li.Comments are ignored and are used to provide human-readable notes and/or "white space" for visually separating blocks of lines.
:eul.
:li.Lines which provide a definitions for archivers.
:ol compact.
:li.Each archiver is defined by a set of :hp2.consecutive:ehp2. lines.
:li.The number of these lines is set by the first line of the file. Currently it is 21.
:li.Each line is described below.
:ol compact.
:li.archiver id  string for human consumption (e.g. ARC, LHARC, PKZIP)
:li.normal extension for archives without period  (e.g. ZIP, ARC, LZH)
:li.offset into file to signature (0-based, leave blank if no signature)
:li.list command
:li.extract command
:li.extract with directories command
:li.test archive command
:li.add/create command
:li.add/create with paths command
:li.add/create and recurse command
:li.move command
:li.move with paths command
:li.delete command
:li.signature (case sensitive, leading spaces count!)
:li.startlist string
:li.endlist string
:li.old size position (0-based, -1 = not available)
:li.new size position  (0-based, -1 = not available)
:li.file date/time, specified by two comma-separated numbers&colon.
:ol compact.
:li.position (0-based, -1 = not available)
:li.type code&colon.
:dl compact.
:dt.0:dd.No date in the data
:dt.1:dd.mm-dd-yy hh&colon.mm&colon.ss    (e.g. 02-31-98  23&colon.55&colon.32)
:dt.2:dd.dd Mmm yy hh&colon.mm&colon.ss   (e.g. 31 Feb 98 23&colon.55&colon.32)
:dt.3:dd.dd Mmm yy hh&colon.mmA     (e.g. 31 Feb 98  11&colon.55p)
:dt.4:dd.yy-mm-dd hh&colon.mm&colon.ss    (e.g. 98-02-31 23&colon.55&colon.32)
:dt.5:dd.dd-mm-yy hh&colon.mm       (e.g. 31-02-98  23&colon.55)
:edl.
:eol.
:li.number of elements/fields in dates (e.g. "03 June 92" would be 3)
:li.file name, specified by up to four comma-separated numbers (only the first number is required)&colon.
:ol compact.
:li.0-based position number of the file name (required; -1 = last pos on the line). See ZOO entry for an example of -1 in file name position
:li.1 = name starts at filename-position (previous number on this line) and extends to the end of the line (0 = otherwise; n/a w/ position = -1), See LH entry for an example of this
:li.1 = the file name starts at the second character of the field (0 otherwise).
:li.1 = two lines for each file listed with file name on first line (0 otherwise). See RAR 2.00 entry for an example of this
:eol.
:eol.
:eol.
:eol.
:p.
Additional notes:
:ul compact.
:li.All archiver command strings should specify a command which does :hp2.not:ehp2. prompt for further input!
:li.Archiver definition entries that contain numeric values may have trailing comments.
:li.Archiver definition lines that contain strings do not support trailing comments.
:li.Blank lines are ignored except within a 21 line archiver definition entry.
:li.A blank line within a definition entry will be treated as either an empty string
or the number 0, depending on what content is expected for the definition line.
:eul.
:p.
For additional information see&colon.
:ul compact.
:li.Help for :link reftype=hd res=94300.Archiver Details:elink.
:li.The ARCHIVER.TMP file, located in the TMPLATES subdirectory of where FM/2 is installed. (This is the default version
of ARCHIVER.BB2 which is distributed with FM/2.)
:eul.

