:h1 res=92000 name=PANEL_CONFIG.Config Menu
:i1 id=aboutConfig.Config Menu

:artwork name='bitmaps\tweak.bmp' align=center.
:p.
FM/2 can be configured as you like it using the commands under this
submenu.  It is highly recommended that you step through the items in
this submenu when you first begin to use FM/2, both to familiarize
yourself with the available configuration options and to make FM/2
work the way you like it to work.
:p.
To change fonts and colors, FM/2 uses the WPS Font and Color Palettes.
The Config menu contains commands to call up these objects for you.
:p.
This submenu affects general FM/2 behavior.  Each class of container has
its own configuration menu that allows you to set the type of view,
filtering, and so on.  To get the popup menu that controls the
container's appearance, request a :link reftype=hd res=93700.context
menu:elink. while the pointer is over an empty area of the container, or
select the Views pulldown submenu.
:p.
The :hp6.:link reftype=hd res=99400.Toggle quicklists:elink.:ehp6.
command causes a dropdown listbox to appear below the toolbar (if one is
being used; below the pulldown menu, if one is being used, or titlebar
if not) and above other windows.  The listbox contains the directory
names you've assigned in the :link reftype=hd res=91500.Walk
Directories:elink. dialog.  If you pick one, a Directory Container for
that directory is opened (hold down the Shift key while clicking to
change an existing Directory Container instead). If the dropdown listbox
is already present, this command causes it to disappear.
:p.
A Drive Finder dropdown listbox also appears with the quicklist;
selecting a drive in this list causes the Drive Tree container to scroll
to show the selected drive and make it the current object.
:p.
Finally, a Setup dropdown list also appears that contains names of
setups (Drive Tree location, Directory Container locations and
associated directory names). Pick a setup name, and FM/2 reopens and
repositions the windows as they were when you saved the setup.  A
context menu requested on the setup list entry field is used to save and
delete setup names.  To add a name, enter it in the entry field of the
dropdown, request a context menu, and select :hp1.Save state as
name:ehp1.  To delete a name, put it in the entry field, request a
context menu and select :hp1.Delete state name:ehp1.
:p.
The :hp6.Autoview:ehp6. submenu controls the Autoview window.  The
default for this conditional cascade menu is the :hp6.Toggle autoview
window:ehp6. command, which causes an autoview window to appear above
the status line at the bottom of the screen.  As you move the cursor
from object to object, FM/2 displays the first few lines of file objects
in this window.  If the autoview window is already displayed, clicking
this command causes it to disappear.  You can also set the what is to
be autoviewed -- either the file's .COMMENTS EA or the start of the
file's contents (similar to the *nix HEAD program).  When .COMMENTS EAs
are being viewed, you can edit them and the changes will be saved when
you switch the focus from the Autoview window.  You can reach this
editable Autoview window with the Ctrl + Tab hotkey if you are allergic
to your mouse.
:p.
Clicking the contents Autoview window with B1 causes the file to be
viewed. Clicking with B3 (or chording with B1 and B2 simultaneously)
causes the extended attributes to be viewed.  If viewing .COMMENTS
rather than contents, you can pick :hp1.View file:ehp1. from the
context menu.
:p.
The :hp6.Toggle bottom buttons:ehp6. menu item turns off and on a row of
buttons that appear just above the status line(s).  The buttons display
the name, date, and attributes of the currently selected object, and the
filter status of the current container.  If clicked with B1, a command
is generated (rename, info, edit date/attributes and filter dialog
respectively).  If clicked with B2, a context menu appears (the same one
you get if you click B2 on the first status line).  If clicked with B3,
the sort changes for the current container:  filename, last write date,
file size and reverse sort respectively.
:p.
The :hp6.Toggle drivebar:ehp6. menu item turns off and on a bar showing
all available drives.  You can click these drive buttons to find or
switch to a drive (depending on the active window when the button is
clicked), drag objects onto the buttons, request a context menu on a
button for more commands dealing with the drive, or click B3 to open a
Directory Container for that window (or surface and activate one that
already exists).
:p.
See also&colon.
.br
:link reftype=hd res=97600.FM/2 window layout:elink.
.br
:link reftype=hd res=99400.Using quicklists:elink.
.br
:link reftype=hd res=91800.Toolbar:elink.
.br
:link reftype=hd res=90400.Associations:elink.
.br
:link reftype=hd res=90700.Commands:elink.
.br
:link reftype=hd res=94200.Edit Archiver Data:elink.
.br
:link reftype=hd res=94600.Settings notebook:elink.
.br
:link reftype=hd res=100065.Set Target directory:elink.

:h2 res=91800 name=PANEL_TOOLBAR.Toolbar
:i1 id=aboutToolbar.Toolbar

:artwork name='bitmaps\toolbar.bmp' align=center.
The :hp1.toolbar:ehp1. is a collection of buttons that invoke some of
the commands in the pulldown or popup menus.  Placing the mouse pointer
on a button and pressing and holding B2 displays brief help for the
button on the titlebar.
:p.
Some of the buttons will allow objects to be dragged onto them; for
example, you can drag objects onto the trashcan to delete them.  Note
that the hotspot of the mouse pointer itself should be over the button
before releasing, not the icon being dragged (icons are slightly offset
from the mouse pointer to give better target visibility).  Target
emphasis is provided in the form of a black outline around the button,
and the arrow pointer turns into a hand when above a toolbox icon.
:p.
This toolbar submenu is a conditional cascade menu, meaning that if you
click other than on the boxed arrow a default command is executed (the
toolbar is toggled off and on), but if you click on the boxed arrow you
get a submenu of items you can select (the other choices are Text
Toolbar, which will cause the toolbar to appear as "normal" buttons with
text on them, or Toolbar Titles, which will cause text to appear below
the toolbar bitmap buttons).
:p.
Toolbar buttons are user-configurable.  To change the toolbar, click
mouse button two (usually the right button) while the mouse pointer is
over a button to pop up a context menu.  You can get the "Load Toolbox"
dialog by clicking B2 on a blank area of the toolbar, and the "Add
Button" dialog by chording on a blank area of the toolbar.
:p.
For advanced/curious users&colon.  Information on what buttons are in the
toolbar is kept in a file named FM3TOOLS.DAT.  This file is an ASCII
(plain text) file that contains information defining the toolbar.  The
file contains comments that explain its format.
:p.
FM/2 allows you to create customized toolboxes that can be loaded as
required to provide toolboxes for specific activities.  Although FM/2
provides ways to edit the toolbar internally, it's probably easier to
load the definition files into a text editor and make the changes
manually en masse.
:p.
If you'd like to see a different toolbar layout (and try out the Load
Toolbox command), right-click on the toolbar, pick Load Toolbox from the
resultant menu, and pick a toolbox from the listbox.  Then you might try
creating your own and saving it with the Save Toolbox command.
:p.
See also&colon.
.br
:link reftype=hd res=96300.Reorder Tools:elink.
.br
:link reftype=hd res=94400.Edit Tool:elink.
.br
:link reftype=hd res=96400.Add Tool:elink.
.br
:link reftype=hd res=94000.Save Tools:elink.
.br
:link reftype=hd res=94500.Load Tools:elink.

:h3 res=96300 name=PANEL_REORDERTOOLS.Reorder Tools
:i1 id=aboutReorderTools.Reorder Tools
This dialog, accessed from the context menu of a tool button, allows you
to rearrange the order of the toolbar's tool buttons.  You take selected
items from the left listbox and Add them to the end of the right listbox
with the :hp1.Add>>:ehp1. button. When you've moved everything to the
right listbox, click :hp1.Okay:ehp1..  Click :hp1.Cancel:ehp1. if you
change your mind.
:p.
In reality, you don't need to move everything to the right listbox.  You
can move only what you want moved to the top of the list, then click
Okay.  Anything remaining in the left listbox is added to the end of
what's in the right listbox.
:p.
The :hp1.<<Remove:ehp1. button can be used to move selected items from
the right listbox to the bottom of the left listbox.

:h3 res=94400 name=PANEL_EDITTOOL.Edit Tool
:i1 id=aboutEditTool.Edit Tool

This dialog allows you to change the help and text strings and flags
associated with a button.  Edit as desired, then click Okay.  Click
Cancel if you change your mind.
:p.
The :hp1.Help:ehp1. field should contain text to remind you what the
button's command does.  For example, a button that deletes files and
directories might have help reading "Delete files and directories".
:p.
The :hp1.Text:ehp1. field should contain very brief text that will be
placed on the button (if the tool id has no bitmap associated with it).
:p.
The :hp1.Droppable:ehp1. checkbox determines if the tool can have objects
dropped on it or not.  See list below.
:p.
The :hp1.Visible:ehp1. checkbox determines if the tool is visible or not.
:p.
If the :hp1.Separator:ehp1. checkbox is checked, FM/2 puts a bit of
whitespace after the button (separates it from the next button).
:p.
If you check the :hp1.User-defined bmp:ehp1. checkbox, FM/2 will allow
you to use your own bitmap.  Bitmaps are named after the :hp1.ID:ehp1.
of the button -- for example, the bitmap file for ID 1005 would be named
"1005.bmp".  Clicking the :hp1.Edit bmp:ehp1. button will cause ICONEDIT
to be loaded with the bitmap, ready to edit.  (Note that bitmaps should
be 28 x 28.)
:p.
The :hp1.ID:ehp1. field identifies the command that is associated with
this button.  See the :link reftype=hd res=96401.Tool IDs:elink. topic.

:h3 res=96400 name=PANEL_ADDTOOL.Add Tool
:i1 id=aboutAddTool.Add Tool

This dialog is accessed from the context menu of a tool button, or if you
turn on the toolbar when there are no tools defined.
:p.
To add a tool, fill in the fields as appropriate and click Okay.  Click
Cancel if you change your mind.
:p.
The :hp1.Help:ehp1. field should contain text to remind you what the
button's command does.  For example, a button that deletes files and
directories might have help reading "Delete files and directories".
:p.
The :hp1.Text:ehp1. field should contain very brief text that will be
placed on the button (if the tool id has no bitmap associated with it).
:p.
The :hp1.Droppable:ehp1. checkbox determines if the tool can have objects
dropped on it or not.  See list below.
:p.
The :hp1.Visible:ehp1. checkbox determines if the tool is visible or not.
:p.
If the :hp1.Separator:ehp1. checkbox is checked, FM/2 puts a bit of
whitespace after the button (separates it from the next button).
:p.
If you check the :hp1.User-defined bmp:ehp1. checkbox, FM/2 will allow
you to use your own bitmap.  Bitmaps are named after the :hp1.ID:ehp1.
of the button -- for example, the bitmap file for ID 1005 would be named
"1005.bmp".  Clicking the :hp1.Edit bmp:ehp1. button will cause ICONEDIT
to be loaded with the bitmap, ready to edit.
:p.
The :hp1.ID:ehp1. field should be assigned a number that tells FM/2 what
the command associated with the button is.  See :link reftype=hd
res=96401.Tool IDs:elink. topic.

:h3 res=96401 name=PANEL_TOOLIDS.Tool IDs
:i1 id=aboutToolIDs.Tool IDs

:xmp.
ID     Command                                              Droppable?
====   =================================================    ==========
1023   View Files                                           Y
1024   Edit Files                                           Y
1026   Make Directory                                       N
1010   Object Information                                   Y
1005   Rename Files/Directories                             Y
1004   Delete Files/Directories                             Y
1006   Permanently Delete Files/Directories                 Y
1009   Set Attributes and Date/Time of Files/Directories    Y
10002  Walk Directories                                     N
10008  Select All Files                                     N
10007  Deselect All                                         N
1029   Archive Files/Directories                            Y
1030   Extract From Archives                                Y
1022   Create Objects                                       Y
1021   Create Shadow Objects                                Y
1002   Copy Files/Directories                               Y
1001   Move Files/Directories                               Y
2003   Kill Process                                         N
2004   Undelete Files                                       N
2006   Instant Command File                                 N
2007   OS/2 Command Line                                    N
5001   Filter Container                                     N
3001   Edit Associations                                    N
1048   Edit Commands                                        N
5021   Rescan                                               N
1007   Print Files                                          Y
1008   Extended Attributes                                  Y
2001   View INI Files                                       Y
1027   Save List to Clipboard                               Y
1028   Save List to File                                    Y
1011   Collect Files/Directories                            N
1060   Collect Files/Directories from list file             Y
2010   Bookshelf Viewer                                     N
1017   Open Default                                         Y
1031   Directory sizes                                      N
1132   UUDecode                                             Y
1133   Merge                                                Y
1111   Exit FM/2                                            N
4001   First Command                                        Y
4002   Second Command                                       Y
&period...
4900   First Toolbox                                        N
4901   Second Toolbox                                       N
&period...
:exmp.
Note&colon.  Other Commands can also be used; just add to 4000 as required
to get the number of the command.  For instance, Command 4 would be ID
4004.  Command buttons use the text field; others shown have bitmaps
associated with them in FM/2's resources.  Remember that if you change
your Commands around, you need to resync the tools to the Commands...
:p.
Note&colon.  Toolbox buttons on a toolbar cause the appropriate toolbox
to be loaded from QUICKTLS.DAT (one filename per line, blank lines and
lines beginning with ";" ignored).  The file named on the first line is
the First Toolbox, and so forth.  You can have up to 50 toolbox buttons
defined (total).
:p.
This list isn't complete; ask me if you'd like a command added that
doesn't appear, and I'll tell you if it can be added and what the ID is,
if so.

:h3 res=94000 name=PANEL_SAVETOOLS.Save Toolbox
:i1 id=aboutSaveTools.Save Toolbox

FM/2 allows you to save toolboxes (groups of tools) for later recall
with the :link reftype=hd res=94500.Load Toolbox:elink. command.
:p.
Type the name of the file to hold the toolbox definitions into the entry
field below the listbox, or select a file displayed in the listbox (it
will be overwritten).  FM/2 saves the toolbox definitions into the file.
:p.
The files are given an extension of &period.TLS if you don't specify an
extension.  The listbox shows files in the FM/2 directory that have the
extension .TLS (which is my way of suggesting that you follow this
convention).

:h3 res=94500 name=PANEL_LOADTOOLS.Load Toolbox
:i1 id=aboutLoadTools.Load Toolbox

FM/2 allows you to load toolboxes (groups of tools) which were saved
with the :link reftype=hd res=94000.Save Toolbox:elink. command.
:p.
Type the name of the file that holds the toolbox definitions into the
entry field below the listbox, or select a file displayed in the
listbox.  FM/2 loads the toolbox definitions from the file and updates
the toolbar.
:p.
This allows you to have groups of specific-purpose buttons which you can
load for different types of activities.  You don't have to use it, but
it's there if you would like to do so.
:p.
Clicking :hp1.Okay:ehp1. without typing in or selecting a filename
results in FM/2 reloading the default toolbox from FM3TOOLS.DAT.
:p.
If I may, here's how I'd recommend using this facility&colon.
:p.
First, save the full toolbar under a new name (perhaps ALL.TLS).  Next,
delete any buttons that you don't want in your customized toolbar.
Finally, add any Command buttons that you do want in the customized
toolbar and save the new toolbox (as <something>.TLS).  By starting with
the full toolbar, you'll spend more of your time deleting buttons you
don't want (a trivial task) and less adding buttons.

:h2 res=99980 name=PANEL_FLAGS.Drive flags
:i1 id=aboutFlags.Drive flags

Here you can set flags for various drives.  Command line switches
override these flags.  The flags are stored in FM/2's INI file and
loaded when FM/2 starts, so this is an alternative to all the
esoteric drive command line switches :hp1.except:ehp1. the
:hp2.Ignore:ehp2. switch.
:p.
:hp6.No prescan:ehp6.  Setting this causes FM/2 to :hp1.not:ehp1.
pre-scan removable drives.  You have to double-click the drive
before it's checked to see if it has any subdirectories.  This is
handy for those of you with CD carousels.
:p.
:hp6.Don't load icons:ehp6.  Prevents FM/2 from loading icons for
files and directories on this drive.  If the drive contains only
DOS programs and data files or is a very slow drive, you might
want to check this one.
:p.
:hp6.Don't load subjects:ehp6.  Prevents .SUBJECT EAs from being
automatically loaded on this drive.  Again, if your drive is slow or you
don't use .SUBJECTs with objects on this drive, you might want to check
it.
:p.
:hp6.Don't load longnames:ehp6.  Prevents .LONGNAME EAs from being
automatically loaded on this drive.  You've got the idea by now,
right?
:p.
:hp6.Slow drive:ehp6..  Check this for drives which have extremely slow
seek times (like ZIP and EZ removable hard drives).  The Autoview window
and associated messages are disabled for this drive, and the "Quick Arc
find" method is always used, whether on globally or not, which snaps up
response time. I may take other shortcuts for drives with this attribute
later (loosen error checking).  For such slow drives you may also want
to check the various :hp1.Don't load...:ehp1. flags listed above.
:p.
:hp6.Include files in tree:ehp6.  If you check this, files will be shown
as well as directories in the Drive Tree for this drive.  I have no
idea why you would ever want to check this.
:p.
You get this dialog by selecting Edit->Drive flags from a context menu
requested on a drive (root directory) in the Drive Tree or VTree window.
:p.
Note&colon.  To set the drive flags on a removable drive, like a floppy
or CD-ROM, be sure you put a disk in the drive first.  FM/2 won't let
you set drive flags on a currently invalid disk.


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

.im notebook.ipf

.im databar.ipf

