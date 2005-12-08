:h1 res=99800 name=PANEL_TUTORIAL.Tutorials
:i1 id=aboutTutorial.Tutorials
:artwork name='bitmaps\tutor.bmp' align=center.
This tutorial section is provided for the complete novice. If you
already know how to use OS/2's WPS in general and have used other OS/2
PM applications, you probably don't need this tutorial. Go forth and be
productive.
:p.
This tutorial addresses the most often used commands in FM/2 -- the only
commands that most "file managers" provide. There are, of course, many
more commands available in FM/2. However, once you have the methodology
for these basic commands mastered, other commands are easy to use. 
:p.
Plesae pick a topic&colon.
:p.
:link reftype=hd res=99810.Rename:elink.
.br
:link reftype=hd res=99820.Move:elink.
.br
:link reftype=hd res=99830.Copy:elink.
.br
:link reftype=hd res=99840.Compare:elink.
.br
:link reftype=hd res=99850.View:elink.
.br
:link reftype=hd res=99860.Open:elink.
.br
:link reftype=hd res=99870.Delete:elink.
.br
:link reftype=hd res=99880.Make directory:elink.
.br
:link reftype=hd res=99890.Create archive:elink.
.br
:link reftype=hd res=99900.Extract from archive:elink.
.br
:link reftype=hd res=100060.Double-click actions:elink.
.br
:link reftype=hd res=99910.Using context menus:elink.

:h2 res=99910 name=PANEL_TCONTEXT.Context menu tutorial
:i1 id=aboutContextTutorial.Using Context menus
To request a context menu, first point the mouse arrow at an object.
Then click mouse button 2 (B2, usually the right button). A context
menu will then appear, if available.
:p.
The :hp1.Files:ehp1. menu is a "shadow" of the context menu that would
be shown if one were requested over the :hp1.current object:ehp1..
Keyboard-only users can use the :hp1.Files:ehp1. menu instead of context
menus. But stop being unproductively stubborn, get a pointing device.
&colon.-)
:p.
The :hp1.Views:ehp1. menu is a "shadow" of the context menu that would
be shown if one were requested over whitespace in the current window.
:p.
To make things a little clearer to users who haven't used OS/2's WPS
before, take a look at the following three pictures&colon.
:p.
:artwork name='bitmaps\tcontxt1.bmp' align=left.
:p.
A context menu that applies to one item. The mouse arrow still points at
the object on which the menu was requested -- that is the object that will
be affected by the command selected from the menu. You can see the dotted
outline with rounded corners around the object.
:p.
:artwork name='bitmaps\tcontxt2.bmp' align=left.
:p.
A context menu that applies to several items. The mouse arrow still points
at the object on which the menu was requested -- since it is highlighted,
all highlighted objects will be affected by the command selected from the
menu.
:p.
:artwork name='bitmaps\tcontxt3.bmp' align=left.
:p.
A context menu that applies to a container as a whole. The menu was
requested over container whitespace (an empty part of the container),
not over any of the objects that it contains.

:h2 res=99810 name=PANEL_TRENAME.Rename tutorial
:i1 id=aboutRenameTutorial.Rename Tutorial
:p.
:artwork name='..\..\bitmaps\rename.bmp' align=left.
:p.
Please select a method to learn about&colon.
:p.
:link reftype=hd res=99811.Direct editing:elink.
.br
:link reftype=hd res=99812.Drag and drop:elink.
.br
:link reftype=hd res=99813.Keystrokes:elink.

:h3 res=99811 name=PANEL_TRENAMEDIRECT.Rename by direct editing
:i1 id=aboutRenameDirectTutorial.Rename by Direct Editing
:p.
:artwork align=left name='bitmaps\direct.bmp'.
:p.
To rename a file system object, hold down the :hp1.Alt:ehp1. key and
click on the object's name with mouse button 1 (B1, usually the left
button). The entire pathname of the object appears in a framed control
known as an :hp1.MLE:ehp1.. You'll initially be located at the start of
the filename with the filename (excluding the path) highlighted (ready
to be replaced by whatever you type). The usual editing keys work
(arrows, home, end, insert, delete, etc.).
:p.
Type in the new name, then click the file system object again. FM/2
renames the object and updates the display.
:p.
You can abort the rename by pressing the :hp1.ESCape:ehp1. key.
:p.
:link reftype=hd res=99800.Return to Tutorial menu:elink.

:h3 res=99812 name=PANEL_TRENAMEDRAG.Rename by drag and drop
:i1 id=aboutRenameDragTutorial.Rename by Drag and Drop
:p.
:artwork name='bitmaps\drename.bmp' align=left.
:p.
To invoke a rename using drag and drop, "grab" a file system object with
the mouse by placing the mouse pointer on top of the object and
depressing and holding mouse button 2 (B2, usually the right button).
Still holding B2, "drag" the object to some whitespace (unoccupied space
in the Directory Container -- a blank area) and then release B2. The
:link reftype=hd res=91400.Rename dialog:elink. will then appear and you
can enter a new name for the object.
:p.
You can abort a drag and drop action by pressing the :hp1.ESCape:ehp1.
key.
:p.
:link reftype=hd res=99800.Return to Tutorial menu:elink.

:h3 res=99813 name=PANEL_TRENAMEKEY.Rename by keystrokes
:i1 id=aboutRenameKeyTutorial.Rename by Keystrokes
Place the dotted cursor on the object you want to rename (make it the
:hp1.current object:ehp1.). Press Ctrl + r, the accelerator key
for the Rename command, or pick "Rename" from the :hp1.Files menu:ehp1..
:p.
The :link reftype=hd res=91400.Rename dialog:elink. will then appear and
you can enter a new name for the object.
:p.
:link reftype=hd res=99800.Return to Tutorial menu:elink.

:h2 res=99820 name=PANEL_TMOVE.Move tutorial
:i1 id=aboutMoveTutorial.Move Tutorial
:p.
:artwork name='..\..\bitmaps\mover.bmp' align=left.
:p.
Please select a method to learn about&colon.
:p.
:link reftype=hd res=99821.Direct editing:elink.
.br
:link reftype=hd res=99822.Drag and drop:elink.
.br
:link reftype=hd res=99823.Keystrokes:elink.

:h3 res=99821 name=PANEL_TMOVEDIRECT.Move by direct editing
:i1 id=aboutMoveDirectTutorial.Move by Direct Editing
:p.
:artwork align=left name='bitmaps\direct.bmp'.
:p.
To move a file system object, hold down the :hp1.Alt:ehp1. key and
click on the object's name with mouse button 1 (B1, usually the left
button). The entire pathname of the object appears in a framed control
known as an :hp1.MLE:ehp1.. You'll initially be located at the start of
the pathname. Type in the new pathname, then click the file system
object again. FM/2 moves the object and updates the display.
:p.
You can abort the move by pressing the :hp1.ESCape:ehp1. key.
:p.
:link reftype=hd res=99800.Return to Tutorial menu:elink.

:h3 res=99822 name=PANEL_TMOVEDRAG.Move by drag and drop
:i1 id=aboutMoveDragTutorial.Move by Drag and Drop
:p.
:artwork name='bitmaps\tmove.bmp' align=left.
:p.
To move a file system object using drag and drop, "grab" a file system
object with the mouse by placing the mouse pointer on top of the object
and depressing and holding mouse button 2 (B2, usually the right
button). Still holding B2, "drag" the object to its destination (for
instance, onto a directory in the Drive Tree), and then release B2.
:p.
You can abort a drag and drop action by pressing the :hp1.ESCape:ehp1.
key.
:p.
:link reftype=hd res=99800.Return to Tutorial menu:elink.

:h3 res=99823 name=PANEL_TMOVEKEY.Move by keystrokes
:i1 id=aboutMoveKeyTutorial.Move by Keystrokes
Place the dotted cursor on the object you want to move (make it the
:hp1.current object:ehp1.). Press Ctrl + m, the accelerator key
for the Move command, or pick "Move" from the :hp1.Files menu:ehp1..
:p.
A dialog known as the :link reftype=hd res=91500.Walk Directories:elink.
dialog appears to allow you to pick or type the destination directory
for the move operation.
:link reftype=hd res=99800.Return to Tutorial menu:elink.

:h2 res=99830 name=PANEL_TCOPY.Copy tutorial
:i1 id=aboutCopyTutorial.Copy Tutorial
:p.
:artwork name='..\..\bitmaps\copier.bmp' align=left.
:p.
Please select a method to learn about&colon.
:p.
:link reftype=hd res=99832.Drag and drop:elink.
.br
:link reftype=hd res=99833.Keystrokes:elink.
.br
:link reftype=hd res=99834.Cloning:elink.

:h3 res=99832 name=PANEL_TCOPYDRAG.Copy by drag and drop
:i1 id=aboutCopyDragTutorial.Copy by Drag and Drop
:p.
:artwork name='bitmaps\tcopy.bmp' align=left.
:p.
To copy a file system object using drag and drop, press and hold the
Ctrl key, then "grab" a file system object with the mouse by placing the
mouse pointer on top of the object and depressing and holding mouse
button 2 (B2, usually the right button). Still holding B2 and Ctrl,
"drag" the object to its destination (for instance, onto a directory in
the Drive Tree), and then release B2. During a copy drag, the dragged
object is "ghosted" rather than solid as during a move drag.
:p.
You can abort a drag and drop action by pressing the :hp1.ESCape:ehp1.
key.
:p.
:link reftype=hd res=99800.Return to Tutorial menu:elink.

:h3 res=99833 name=PANEL_TCOPYKEY.Copy by keystrokes
:i1 id=aboutCopyKeyTutorial.Copy by Keystrokes
Place the dotted cursor on the object you want to rename (make it the
:hp1.current object:ehp1.). Press Ctrl + c, the accelerator key
for the Copy command, or pick "Copy" from the :hp1.Files menu:ehp1..
:p.
A dialog known as the :link reftype=hd res=91500.Walk Directories:elink.
dialog appears to allow you to pick or type the destination directory
for the copy operation.
:p.
:link reftype=hd res=99800.Return to Tutorial menu:elink.

:h3 res=99834 name=PANEL_TCLONE.Clone tutorial
:i1 id=aboutCloneTutorial.Clone Tutorial
You :hp1.clone:ehp1. a file system object by copying it into the same
directory that already holds it. The :link reftype=hd res=91400.Rename
dialog:elink. will then appear because of the naming conflict, and allow
you to specify a new name for the new, "cloned" copy of the object.
:p.
:link reftype=hd res=99800.Return to Tutorial menu:elink.

:h2 res=99840 name=PANEL_TCOMPARE.Compare tutorial
:i1 id=aboutCompareTutorial.Compare Tutorial
:p.
:artwork name='..\..\bitmaps\linkdrag.bmp' align=left.
:p.
To compare file system objects, link-drag one object onto another.
:p.
A link drag is initiated by placing the mouse cursor on the object to
be dragged, holding down the Ctrl and Shift keys, and clicking mouse
button 2 (B2, usually the right button). While still holding the keys
and B2, move the mouse to drag the object over the target object, then
release B2. During a link drag, a rubber-band line appears between the
source object and the dragged object.
:p.
You can change the compare program from FM/2's :link reftype=hd
res=94600.Internal Settings notebook:elink.. An internal default is
used if you leave the entry there blank.

:h2 res=99850 name=PANEL_TVIEW.View tutorial
:i1 id=aboutViewTutorial.View Tutorial
:p.
:artwork name='..\..\bitmaps\view.bmp' align=left.
:p.
Viewing files can be as simple as double-clicking their objects. FM/2
will attempt to do to the object what should "logically" be done, which,
in the case of text files, archives and the like, is to view them. If
you would like different actions taken, FM/2 provides :link reftype=hd
res=90400.Associations:elink. to allow you to assign special viewer
programs or actions to different file types.
:p.
You can also invoke viewing directly from the :hp1.Files:ehp1., :link
reftype=hd res=93700.context:elink. menus, or by pressing the Ctrl + v
accelerator key. In this case, no other default actions will be taken
even if one might be "obvious" -- the object will simply be viewed. If,
for example, you wanted to view an executable, this is the method you'd
need to use (the default double-click action would be to run an
executable file, which is usually what you'd want to do).
:p.
You can change the viewing program from FM/2's :link reftype=hd
res=94600.Internal Settings notebook:elink.. An internal default is
used if you leave the entry there blank.
:p.
A good viewing program to try out would be :hp1.Hyperview:ehp1. by
Michael H. Shacter.

:h2 res=99860 name=PANEL_TOPEN.Open tutorial
:i1 id=aboutOpenTutorial.Open Tutorial
"Opening" a file system object is an object-based concept. When you
open a directory, you get a view of the file system objects (files and
directories) contained within that object. When you open an executable
file, you cause it to execute. When you open a data file, you usually
cause its owning application (if known) to execute with the data file
being loaded by the application. This is known as a "default" open.
:p.
But there are often several types of "open" operation that can be
performed on an object. For instance, you can open an object's Settings
notebook. FM/2 provides some variations on WPS open themes. You can
open either a WPS folder or an FM/2 Directory Container for a directory
object, for instance (to find out more about opening an FM/2 Directory
Container for a directory object, see the :link reftype=hd
res=98900.Opening a Directory Container:elink. section).
:p.
To open an object's default WPS "view," use the F6 accelerator key, or
press the Ctrl key while double-clicking the object. The same type of
"open" will be done on the object that would be done if you directly
double-clicked the object in a WPS folder object (remember, WPS folders
are just visual representations of directories).
:p.
To open a file system object's WPS Settings notebook directly, use the
F7 accelerator key, or press the Ctrl and Shift keys while
double-clicking the object.
:p.
For directory objects, you have the choice of WPS view type to open, as
well -- Icon, Details or Tree view. You pick these from the
:hp1.Open:ehp1. cascade menu on the :hp1.Files:ehp1. menu or a context
menu requested on the object (you request a context menu on an object by
placing the mouse pointer on the object and clicking mouse button 2,
B2). A command to open an FM/2 Directory Container is also on that
cascade menu, and is the default.

:h2 res=99870 name=PANEL_TDELETE.Delete tutorial
:i1 id=aboutDeleteTutorial.Delete Tutorial
:p.
:artwork name='..\..\bitmaps\delete.bmp' align=left.
:artwork name='..\..\bitmaps\permdel.bmp' align=left.
:p.
There are two styles of deleting under FM/2 -- a "regular" delete, that
leaves the objects capable of being undeleted using OS/2's UNDELETE
command or FM/2's Undelete Utility, and a "permanent" delete that does
not leave the object recoverable. "Permanent" deletions are usually
faster than "recoverable" deletions. You can select which type you want
to be the default in FM/2's internal :link reftype=hd res=94600.Settings
notebook:elink..
:p.
Note&colon. You have to set up OS/2, using CONFIG.SYS, so that UNDELETE
is enabled to do non-permanent deletions. See :link reftype=launch
object='VIEW.EXE' data='CMDREF.INF UNDELETE'.UNDELETE:elink. in the OS/2
online help. Note&colon. FM/2 does not use the "Trashcan" that comes with 
eWorkPlace or XWorkPlace for non-permanent deletions. If you wish to use the
"Trashcan" can drag the items directly to it from FM/2.
:p.
You can delete objects in many ways&colon.
:p.
:hp1.Drag and drop:ehp1. Drag the object(s) to FM/2's toolbar Trashcan
icon, to the WPS's Shredder icon or to a Trashcan application. (You might want to pick up a
replacement for the WPS Shredder like the free :hp1.mshred:ehp1. object
written by the author of FM/2.)
:p.
:hp1.Files or context menu:ehp1. Select the :hp1.Delete:ehp1. command
or pick the type of delete from the cascade menu.
:p.
:hp1.Keyboard:ehp1. The Ctrl + d accelerator key performs a "regular"
delete, Shift + Ctrl + D (also written as just "Ctrl + D" -- accelerator
keys are case sensitive) performs a permanent delete.

:h2 res=99880 name=PANEL_TMKDIR.Make directory tutorial
:i1 id=aboutMkdirTutorial.Make Directory Tutorial
:artwork name='..\..\bitmaps\mkdir.bmp' align=left.
:p.
To create a directory, use FM/2's Make Directory command.
:p.
You can pick this command from the :hp1.Files:ehp1. menu or a
:hp1.context:ehp1. menu (under the Miscellaneous cascade menu). You can
click the button on FM/2's toolbar. Or you can use the Shift + Ctrl
+ "M" accelerator key.
:p.
FM/2 presents you with a small dialog box in which you can type the name
of the directory that you want to create -- FM/2 will try to fill in the
entry field with a guess at the first part (parent directories) of what you want, but you can
delete that if desired (:hp1.Shift + Home:ehp1., then touch the
:hp1.Del:ehp1. key to do it quickly).
:p.
The directory created can be many levels deep. FM/2 will create
intermediate subdirectories as required if they do not already exist.
For example, if you have a directory \myapps and want to create a
directory \myapps\games\mygame you can do so without first creating
\myapps\games -- just enter the full path you want and FM/2 will
create the intermediate directory \myapps\games.
:p.
You can pick :hp1.<New directory>:ehp1. from the Common directories
:link reftype=hd res=99400.quicklist:elink. to create a new directory
and open its Directory container simultaneously.

:h2 res=99890 name=PANEL_TARCHIVE.Create archive tutorial
:i1 id=aboutArchiveTutorial.Create Archive Tutorial
:artwork name='..\..\bitmaps\archive.bmp' align=left.
:p.
To create a new archive, select the file system objects you want in the
archive, then select :hp1.Archive:ehp1. from the :hp1.Files:ehp1. menu
or a :hp1.context:ehp1. menu, or click the Archive toolbar button.
:p.
A dialog then appears that lets you select the type of archive to be
created from the types defined in the :hp1.ARCHIVER.BB2:ehp1. control
file.
:p.
Once you have selected the archive type, the :link reftype=hd
res=90300.Archive:elink. dialog appears and lets you specify the name of
the archive to be created and the type of archiving action (move or copy
to archive, include subdirectories or not, recurse into subdirectories,
etc.). You can specify additional filemasks or place more obscure
options on the command line to be executed.
:p.
To add to an existing archive, you can go through the same process,
changing the name of the archive to that of the existing archive, or
simply drag objects onto the existing archive's object.
:p.
You can also add objects to an archive by viewing it (double-click on
the archive object) and then dragging the objects to be added onto the
:link reftype=hd res=90200.Archive Container:elink. window.

:h2 res=99900 name=PANEL_TEXTRACT.Extract from archive tutorial
:i1 id=aboutExtractTutorial.Extract from Archive Tutorial
:artwork name='..\..\bitmaps\extract.bmp' align=left.
:p.
To extract from an archive, first place the dotted cursor on the file
(make it the :hp1.current object:ehp1.), then select the
:hp1.Extract:ehp1. command from the :hp1.Files:ehp1. menu or a context
menu requested on the archive, or press the Ctrl + x accelerator
key.
:p.
The :link reftype=hd res=91000.Extract:elink. dialog then appears and
allows you to specify a filemask to be extracted and/or to add more
esoteric options to the command line to be executed.
:p.
You can also extract objects from an archive by viewing it (double-click
on the archive object) and then dragging objects from the :link
reftype=hd res=90200.Archive Container:elink. window to FM/2's other
windows, or by using the :hp1.Files:ehp1. or context menu and selecting
from various types of :hp1.Extract:ehp1. commands there.

:h2 res=100060 name=PANEL_DBLCLK.Double-click actions
:i1 id=aboutDoubleClick.Double-click actions

Double-clicking a file system object causes its :hp1.default action:ehp1.
to be taken. FM/2 has its own set of defaults, but you can override
those in many cases. Here's a description of the FM/2 defaults and
how to change them&colon.
:p.
If the object is a directory, the Directory Container switches to look
into that directory. If it's a file, FM/2 first checks to see if you've
assigned any :link reftype=hd res=90400.Associations:elink. that match
the filename and signature (if applicable). Next, FM/2 tries to view
the file as an :link reftype=hd res=90200.archive:elink.. If it's not
an archive, FM/2 checks to see if the file's an executable and runs it
if it is. Then INI and HLP files are checked by extensions and viewed as
such. If all else fails, FM/2 views the file using the configured or
internal viewer.
:p.
In the Drive Tree, holding down the Shift key while double-clicking
causes a new FM/2 Directory Container to be opened, and holding down
the Ctrl key while double-clicking causes a WPS folder to be opened.
:p.
In Directory Containers, holding down the Ctrl key while double-clicking
causes the default WPS open to be performed on the object (this honors
WPS associations, runs executables, opens folders in their default view,
or opens the Settings notebook for the object -- whatever
double-clicking on the object in the WPS would do). Holding down the
Shift key while double-clicking causes file objects to be directly Viewed,
bypassing the steps listed above for file objects (for directory
objects this causes a new FM/2 container to be opened).
:p.
If you'd like more detail, go to the :link reftype=hd
res=99850.View:elink. and :link reftype=hd res=99860.Open:elink.
tutorials.
:p.
You can change what FM/2 does on a double-click of a file object by
using FM/2's :link reftype=hd res=90400.Associations:elink.. This is
a very powerful tool for customizing FM/2's behavior, so when you're
ready, be sure to look it over.
