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
this menu.
:p.
The :link reftype=hd res=100065.Set Target directory:elink. is the next menu
item and can also be accessed using :color fc=default bc=palegray.Ctrl:color fc=default bc=default.+:color fc=default bc=palegray.T:color fc=default bc=default..
:p.
Five menu items to toggle various FM/2 functions on and off are next.
The first two :link reftype=hd res=91800.Toolbar:elink. and :hp6.Autoview:ehp6.
have submenus.
:p.
The:hp6.Toolbar:ehp6. submenu default is to toggle the toolbar on and off
The other two items allow you to make the toolbar text only or you can
add titles under the toolbar icons. Additional functions to edit the toolbar
can be found on the context menu obtained by clicking :color fc=default bc=cyan.B2:color fc=default bc=default. over a tool icon
on the toolbar (see :link reftype=hd res=91800.Toolbar:elink. for more information)
You can switch between toolbars using the dialog that appears if you click :color fc=default bc=cyan.B2:color fc=default bc=default. over
an area of "white space" on the toolbar or by selecting load toolbox from the previously
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
editable Autoview window with the  :color fc=default bc=palegray.Ctrl:color fc=default bc=default. +  :color fc=default bc=palegray.Tab:color fc=default bc=default. hotkey if you are allergic
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
,br
A States dropdown listbox (Fm/2 configurations you have saved with specific name).
.br
A Commands dropdown listbox (External command that have been added to FM/2).
.br
A Directory dropdown listbox (Directory names you've assigned in the
:link reftype=hd res=91500.Walk Directories:elink. dialog).
.br
A Toolboxesdropdown listbox also appears if the :hp6.Toolbar:ehp6. is on.
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
select this command, click Cancel at the :link reftype=hd res=91500.Walk
Directories:elink. dialog that appears, and answer Yes to the question
subsequently asked.
:p.
See also the :link reftype=hd res=99950.General page:elink. of the
internal Settings notebook.

.im assoc.ipf

.im command.ipf

:h2 res=94200 name=PANEL_AD_FRAME.Editing Archiver Details
:i1 id=aboutEditArc.Editing Archiver Details
:p.
This dialog box, which you can reach from the Config menu, allows you to
edit the details of an :link reftype=hd res=100015.archiver:elink.. Entry
boxes are present for all the twenty one fields represented in
ARCHIVER.BB2 (the text file that contains control information about your
archivers which FM/2 uses to interface with the archivers). It's
probably easier for most people to edit ARCHIVER.BB2 directly with a
text editor.
:p.
:hp1.NOTE&colon.:ehp1. The simplest method to ensure that your archivers
work properly with FM/2 is to make sure they're in a directory named in
your PATH= statement, and check the names of the files to make sure they
match what's on your system (i.e. UNZIP.EXE in both ARCHIVER.BB2 and on
your hard disk, not UNZIP.EXE in one and UNZIP32.EXE in another).
:p.
In the event that you attempt to list an archive and FM/2 feels you've
probably bungled the entry in ARCHIVER.BB2, you'll be given an
opportunity to use this dialog to fix the entry. In this case, you'll
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
You may still need to refer to your archiver's documentation, or run it
to get the help on its command syntax. FM/2 can't do everything for you,
but it holds your hand as best it can.
:p.
Refer to the ARCHIVER.BB2 file that came with FM/2 for additional
information and an example.
:p.
When you've completed editing the archiver's details, click Okay. FM/2
will ask you if you want to rewrite ARCHIVER.BB2 (be sure you save the
original copy for its complete notes; FM/2 will back it up one version to
ARCHIVER.BAK). If you don't rewrite ARCHIVER.BB2, changes are good only
for the current session (handy for testing).
:p.
You can also get to this dialog box from Select Files' Config submenu.
:p.
See also&colon.
:p.
:link reftype=hd res=94300.Archiver Details Fields:elink.

:h3 res=94300 name=PANEL_ARCFLDS.Archiver Details Fields
:i1 id=aboutArcFlds.Archiver Details Fields
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
All subfields must be present and correct for FM/2 to get the right
file names from the archive listing.
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
the line of an archive listing is where the time/datestring is. If this
isn't available or you don't care about it, you can enter a -1 to
disable detection of this field entirely. You can optionally follow this
number with a comma and another number that indicates the type of the
date from any of the formats in the following list&colon.
.br
:xmp.
 1. 02-08-96  23&colon.55&colon.32
 2. 8 Feb 96  23&colon.55&colon.32
 3. 8 Feb 96  11&colon.55p
 4. 96-02-08  23&colon.55&colon.32
 5. 31-02-98  23&colon.55
:exmp.
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
:p.
Here's an example of an ARC listing (5.12mpl, command "ARC l"; you may
need to widen the help windows for this to look right...):
:p.
:xmp.
Name           Length     Date
============  ========  =========    <--this line is start-of-list
MAKEFILE           374  28 Nov 89
QSORT.C          14279  29 Nov 89
QSORT.EXE        24629  29 Nov 89
STUFF.H            371  29 Nov 89
        ====  ========               <--this line is end-of-list
Total      4     39653
:exmp.
:p.
Note the filename is in position 0, old length in position 1, and the
date starts in position 2, with 3 parts, and there's no new length
field (so it'd be -1). Compare that to the archiver entry for
ARC 5.12mpl in the "stock" ARCHIVER.BB2 and you should get a feel for
what all those fields mean.

