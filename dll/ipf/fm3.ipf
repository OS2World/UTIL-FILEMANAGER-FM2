:userdoc.
:title.File Manager/2 Help File
:docprof toc=123456.
:body.
:ctrldef.
:ctrl ctrlid=new1 controls='ESC CONTENTS INDEX SEARCH FORWARD BACK' coverpage.
:ectrldef.

.im footnote.ipf

:h1 res=93200 name=PANEL_HOWTOUSEHELP.How to use FM/2's help (Read me first!)
:i1 id=aboutHowToUseHelp.How to use FM/2's help (Read me first!)
:artwork name='bitmaps\fm3.bmp' align=center.
The best way to use FM/2's help, which is basically a hypertext
document, is to look at the :hp2.Contents:ehp2. (which you'll get if you
press Ctrl + F1 at the main window -- it may help you to maximize it
within the help window). The Contents view of a help file is something
like the Table of Contents in a book, with each major topic representing
a chapter. Some topics have a "+" sign beside them to indicate that
there are subtopics available in a hierarchal inverted tree structure;
click the "+" and they'll be revealed.
:p.
The help sometimes has hypertext links shown in a different color, like
the words "Context menu" a few paragraphs below. You can select these
links to switch to a different topic related to the highlighted word(s).
In this way you can browse through the help, moving from topic to topic
as you feel the need for more specific help. The :hp1.Previous:ehp1. button 
 (IBM's view.exe)or the :hp1.Back:ehp1. button (NewView)
(or the Escape key) will move backwards through the hypertext
links you've followed. Please note that in "NewView" the :hp1.Previous:ehp1.
and  :hp1.Next:ehp1. buttons navigate up and down the table of contents
while the :hp1.Back:ehp1. and :hp1.Forward:ehp1. buttons navigate hyperlinks
:p.
To find something on a specific topic, click the :hp2.Search:ehp2.
button at the bottom of the help window and enter some text (IBM's view.exe)  
or use the search tab (NewView). This might
be analogous to the index at the back of a book (although the help
manager also provides an "index" of its own, which is something like the
Contents window with the hierarchy removed -- difficult to browse, in my
opinion).
:p.
A couple of Search examples to give you the feel of the process&colon.
:p.
To find out about changing a volume label, enter "Label" as the "Search
for&colon." text, check the "All sections" checkbox, if present, then press [Enter].
:p.
To find out how to open a new FM/2 window, enter "Open" as the "Search
for&colon." text, check the "All sections" checkbox, if present, then press [Enter].
:p.
If you'd like a printout of any of the topics in the online help, click
the :hp2.Print:ehp2. button at the bottom ("IBM's" View.exe) or top (NewView.exe) of the help 
window. The :link reftype=hd res=93700.Context menu:elink. help topic would probably be a
good one to print out. Gives you something to read in the john.
Remember that you can also use the :hp1.FM/2 Online Help:ehp1. icon in
the FM/2 folder to view the help without starting FM/2 itself.
:p.
If you're stuck in a dialog, click that dialog's :hp2.Help button:ehp2. That should
take you directly to appropriate help. One note of caution:
if you call up help for a dialog, be sure to dismiss the help before
dismissing the dialog. Many of the dialogs run in threads other than
thread 1 (the main thread), and there's a long-standing OS/2 bug that
causes weird behavior if you close a dialog run
in a thread other than thread 1 after before closing its help file. Trust me.
:p.
So, if you're totally new to FM/2, :hp1.here is how to get started.:ehp1.
First, read the help section titled :link reftype=hd
res=91300.Terminology:elink. so we'll understand each other a bit
better. If you're still nervous about OS/2 conventions, try FM/2's
:link reftype=hd res=99800.Tutorial:elink. section for real hand-holding.
Get an overview of some important FM/2 windows in the :link reftype=hd
res=97600.Window layouts:elink. section. Then plunge into the :link
reftype=hd res=90000.General Help:elink. section to find out how to look
at, Move, Copy, Rename and Compare files and directories (the basics).
That will get you started, and we'll give you hints along the way about
other places you might like to look (like :link reftype=hd
res=93000.Hints:elink. &colon.-) when you're ready. The previously
mentioned :link reftype=hd res=93700.Context menus:elink. topic will
give you an overview of all the commands available in FM/2 (and there
are a lot of commands you can use).
:p.
Since some folks are at first overwhelmed by FM/2's configurability,
take a look in the internal :link reftype=hd res=94600.Settings
notebook:elink. (accessible under the :link reftype=hd
res=92000.Config:elink. menu), specifically at the :link reftype=hd
res=99200.Quick page:elink.. There you'll see a few "standard"
configurations you can try out to get an idea of the range of
appearance and performance you can obtain from FM/2 via the Settings
notebook and Config menu.
:p.
Command line help is in the :link reftype=launch object='E.EXE'
data='\FM2\README'.README:elink. file that accompanied the archive,
since you should have read that before trying to start FM/2. You did,
didn't you? Note&colon. This link only works if FM/2 is installed 
to a directory named FM2 on the boot drive. If not go to your FM/2 
install directory and double-click B1 on the file "README".
:p.
There is one thing you should keep in mind about FM/2. FM/2 is
extremely powerful and has a lot of features, but :hp6.you don't have to
use, understand or even know about them all:ehp6.. Most people will use only a few of
FM/2's features on a regular basis (and not everyone will use the same
combination), and that's fine -- find what works best for you and
:hp2.use:ehp2. it. If you find yourself needing some other feature,
call up the help, find it, and use it -- it'll be there whenever you
need it. But don't feel that, somehow, by not using every little nook
and cranny of the program that you're missing out on something. The
idea is to use what you need; pick your tools from the arsenal and get
some work done.
:p.
On the other hand, always assume that there's a way to do what you want
with FM/2 (chances are good that there is), and ask the help window to
Search for it. If you can't find it there, drop me a line at steve53@earthlink.net
:h1 res=90000 name=PANEL_GENERAL.General Help
:i1 id=aboutGeneral.General Help

FM/2's main purpose is to show you what's on your file system and let you
sling what's there around.
:p.
Here we'll cover the basics. Some familiarity with OS/2's WPS
(WorkPlace Shell) is assumed. If you need refreshing, review the :link 
reftype=launch object='View.EXE' data='OS2UG.INF'.OS/2 Desktop Guide:elink. 
It's also assumed that you've already read the :link
reftype=hd res=93200.How to use FM/2's help:elink. and :link reftype=hd
res=91300.Terminology:elink. sections. If you feel that you need more
in-depth help when we're through with this topic, try FM/2's :link 
reftype=hd res=99800.Tutorial:elink. section. Also review the FM/2 default 
:link reftype=hd res=100040.Mouse actions:elink. and a list of FM/2 :link reftype=hd  
res=100005.Accelerator (shortcut) Keys:elink.
:p.
There are several ways to view a directory with FM/2, just as there are
with the WPS. Icon, Name, Text and Details views all offer different
perspectives into the directory being "looked at" (see :link reftype=hd
res=91300.Terminology:elink.). Views showing icons can use full-sized
icons or smaller "mini-icons" to save space.
:p.
Details view can show a great deal of information about file system
objects, and you can customize what is shown with the :hp2.Details
Setup:ehp2. submenu (:link reftype=hd res=93800.Views
menu:elink. or a Directory Container:link reftype=hd
res=93700.context menu:elink.).
:p.
You can also place some limits on the amount of detail that FM/2 provides
from the file system on the :link reftype=hd res=92400.Toggles:elink.
page of the :link reftype=hd res=94600.Settings notebook:elink.. This
can speed up FM/2's scanning of directories but can also make for duller
screens with less information being presented to you. My advice to you
is enjoy the bells and whistles OS/2 and PM provide.
:p.
Take a moment to set up the look of your Directory Container windows
to match your taste -- everyone likes something different. Then meet
me back here and we'll talk about manipulating those objects you see...
:p.
.br
As we talk about manipulating objects, keep firmly in mind the concepts
of :link reftype=hd res=98000."current object" and "highlighted
objects":elink.. The current object is the one on which commands act (it
has the dotted outline around it). If the current object is also
highlighted, all highlighted objects will be affected.
:p.
:artwork name='..\..\bitmaps\rename.bmp' align=center.
:p.
:hp1.Renaming file system objects&colon.:ehp1.
The simplest way to rename a file system object is to point at it with
the mouse pointer, hold down the ALT key, and click the text of its name.
FM/2 produces a mini MLE text entry field where you can type in a new
name (this is :link reftype=hd res=98200.Direct Editing:elink.). When
finished, click the object and a rename is performed. Note that you can
even move the object to another directory when you do this (Also note
that in Details view you can direct-edit the Subject field to change an
object's Subject, and the Longname field to change an object's Longname
on FAT drives.).
:p.
Using this method will not allow you to overwrite an existing file. You
can use drag and drop (as detailed below for Move), the menu command
:link reftype=hd res=91400.Rename:elink. or the :link reftype=hd
res=91800.toolbar:elink. to facilitate overwriting.
:p.
If you'd like more detail, go to the :link reftype=hd
res=99810.Rename:elink. tutorial.
.br
:artwork name='..\..\bitmaps\mover.bmp' align=center.
:p.
:hp1.Moving file system objects&colon.:ehp1.
.br
There are several ways to move a file system object. The most
intuitive is :hp2.drag and drop:ehp2.. Using this method, you "grab"
the file system object by pressing and holding B2 while the mouse
pointer is over the object, then begin to move the mouse (still holding
B2). The object's icon should begin to move with the mouse pointer.
"Drag" this icon to where you want to move it (for instance, if you want
to move a file from C&colon.\ to D&colon.\, drag the file to the Drive
Tree's D&colon.\ object). When the object is where you want it, release
B2 and the move is done.
:p.
When dragging an object into a Directory Container, remember that to
place it into the directory into which the Directory Container "looks"
you need to drop it on container "whitespace" (a part of the container
not occupied by an object). For convenience, the two large status
areas at the top of the container are considered whitespace.
:p.
If you get confused when dragging object(s), press the F1 key. This
will give you some information about what you're doing. Pressing the
Escape key will abort the drag.
:p.
Note that you can't move a file or directory onto another file (except
for archive targets), only into a directory (moving into container
whitespace in a Directory Container window is the same as moving into
the directory the Directory Container "looks" into, and a minimized
Directory Container window is "all whitespace"). Also note that the
object you grab becomes the current object, and if it's also highlighted
you'll drag all highlighted objects (you'll see visual feedback to this
effect).
:p.
You could, of course, also select "Move" from the :hp1.:link reftype=hd
res=93300.Files:elink.:ehp1. menu, a context menu, click the Move
toolbar button, or type the accelerator key Ctrl + m (hold the control
key down and type "m"). In this case, you'll get the :link reftype=hd
res=91500.Walk Directories:elink. dialog where you can enter a target
directory.
:p.
If you'd like more detail, go to the :link reftype=hd
res=99820.Move:elink. tutorial.
.br
:artwork name='..\..\bitmaps\copier.bmp' align=center.
:p.
:hp1.Copying file system objects&colon.:ehp1.
.br
The procedure for copying file system objects is very similar to that
for moving them. When you begin to drag the object, and until you
release it, hold down the control (Ctrl) key. You'll notice that the
dragged icon is "ghosted" to give visual feedback that a copy, not a
move, is being performed. Note that you can copy a file onto an archive
file as well as into a directory. You can also "clone" a file by dropping
it into the directory where it already resides -- you'll get a rename
dialog that will allow you to change the name, creating a file exactly
like the other with a different name.
:p.
As for move above, there is a "Copy" menu item, a toolbar button,
and Ctrl + c is the accelerator key.
:p.
If you'd like more detail, go to the :link reftype=hd
res=99830.Copy:elink. tutorial.
.br
:artwork name='bitmaps\linkdrag.bmp' align=center.
.br
:hp1.Comparing file system objects&colon.:ehp1.
.br
There is one other type of drag and drop operation called a "link drag."
To link drag, hold down the Ctrl and Shift keys while dragging.
You'll see a "rubber band line" extend from where you grabbed the icon
to the mouse pointer as a visual cue. Link dragging is usually used
within FM/2 to do compare operations (see also :link reftype=hd
res=99950.:hp2.Link Sets Icon:ehp2. toggle:elink.). What you drag will
be compared to what you drop it on. Note, however, that if you drag to
a WPS object (like the desktop or other folder), OS/2's version of a
link drag is performed, which usually results in the creation of an object shadow
If you'd like more detail, go to the :link reftype=hd
res=99840.Compare:elink. tutorial.
:p.
Double-clicking an object in a Directory Container window causes its
:hp1.default action:ehp1. to occur. What that action is depends on
the type of object and how you've configured FM/2. Briefly, the FM/2
defaults (which you can override) cause the object to be opened in the
most likely manner. You can :link reftype=hd res=100060.read about this
in more detail here:elink. in the tutorial section.
:p.
Other commands are accessed via :link reftype=hd
res=97700.pulldown:elink.,  :link reftype=hd res=93700.context
menu:elink. commands, toolbar buttons or accelerator keys. You can read about them by
selecting the highlighted words "context menu" in this paragraph. FM/2
also offers several general :link reftype=hd res=92100.utilities:elink.
and many :link reftype=hd res=92000.configuration:elink. options which
you may want to explore. But you now know how to perform the file system
maintenance basics: Move, Copy, Rename and Compare. You are now, as the
Smothers Brothers said, educated.
:p.
For more specific information on FM/2, click the :hp2.Contents:ehp2.
button at the bottom of the help window. If you're still confused on
the basics, try the FM/2 :link reftype=hd res=99800.Tutorial:elink.
topic. You might also want to look at the :link reftype=hd
res=98900.Opening a Directory Container:elink., :link reftype=hd
res=99400.Using quicklists:elink. and :link reftype=hd res=97600.Window
layout:elink. sections.

.im mouse.ipf

.im keys.ipf

:h1 res=97600 name=PANEL_FM2WINDOWLAYOUT.FM/2 Window Layouts
:i1 id=aboutFM2WINDOWLAYOUT.FM/2 Window Layouts
Here's the main FM/2 window in all its glory (everything turned on)&colon.
:artwork align=left name='bitmaps\oall.bmp'.
:p.
:hp6.Note&colon.:ehp6. You can turn optional windows and controls on
and off. Pick the ones you like, get the others out of your way (see
the Config menu). Surely no one uses all of them at the same time...
The autoview window, bottom buttons, quicklists, toolbar, drive buttons,
status line #2, and even the pulldown menu can all be turned on and off
as desired.
:p.
:hp2.Miscellaneous notes&colon.:ehp2.
:p.
Minimized Directory Containers can be used as drag and drop targets.
:p.
The :hp1.Name:ehp1., :hp1.Date/Time:ehp1., and :hp1.Attributes:ehp1.
bottom buttons display information about the current object as well as
activate commands when clicked.
:p.
The :hp1."Toolboxes" quicklist:ehp1. only appears when the
:link reftype=hd res=91800.Toolbar:elink. is visible (see :link reftype=hd res=99400.Using
quicklists:elink. section).
:p.
Status line #1 can be clicked to shift the focus to FM/2 without
activating any commands. In addition, if the Drive tree is the
active window within FM/2's monolithic window, the Swapfile and
memory available indicators in Status line #2 will be continuously
updated (otherwise they update about every ten seconds).
:p.
You can get information on most areas of the window just by moving the
mouse pointer over the area of interest (unless you've turned off
bubble help in the internal Settings notebook). For help with the
quicklists, which don't have bubble help, see :link reftype=hd
res=99400.Using quicklists:elink..
:p.
.br
Here's a Directory Container window in more detail&colon. See
:link reftype=hd res=98900.Opening a Directory Container:elink. and  
:link reftype=hd res=91900.Folder Button:elink. for more information
:p.
:artwork align=left name='bitmaps\dircnr.bmp'.
:p.
Here's :link reftype=hd res=100000.FM/2 Lite:elink. with an explanation
of the things that are unique to it (Note&colon. Alt-click to change
sort in Details view also works in FM/2). The 
Autoview window has been turned off to unclutter this illustration.
:p.
:artwork align=left name='bitmaps\fm4oall.bmp'.
:p.
Note that the active window, the window pulldown menu commands will
effect, is surrounded by a light red border. When the Drive Tree is active as  
shown above the Directory Container that would change if a
directory were double-clicked in the Tree is surrounded by a fine darker red
border. This color coding gives you instant feedback as to what the
commands you select will effect.
:p.
.br
Here's a look at the default text file
viewer built into FM/2. It is opened when you double-click a text file in
a Directory container (you can configure a different one if you
like)&colon.
.br
:artwork align=left name='bitmaps\newview.bmp'.
:p.
Here's how the window looks if you double-click some lines of text&colon.
.br
:artwork align=left name='bitmaps\newview2.bmp'.
.br
Here we're using the bookmark listbox (which is filled by double-clicking
some lines of text) to make an index for the document, 
enabling us to move from section to section quickly. See the :link
reftype=hd res=99300.Internal viewer:elink. section for more information.

See also&colon.
.br
:link reftype=hd res=99000.Databar:elink.

:h2 res=98900 name=PANEL_GENERALOPEN.Opening a Directory Container
:i1 id=aboutOpeningDirectory.Opening a Directory Container

There are several ways to open a Directory Container.
:p.
The most common is to double-click a directory in the Drive Tree. If no
Directory Container windows are open, one is created. If one or more
Directory Container windows are already open, hold down the Shift key
while double-clicking (see picture below) -- otherwise the default
action is to switch the most recently used Directory Container to the
new directory rather than open a new one.
:p.
:artwork name='bitmaps\open.bmp' align=center.
:p.
An alternative is to click the :hp1.Open button:ehp1. in the Drive
Tree's titlebar (see location indicated by the mouse pointer in the
picture below). This opens a container for the directory highlighted in
the Drive Tree.
:p.
:artwork name='bitmaps\openbutt.bmp' align=center.
:p.
Other alternatives are to select the :hp1.Open->New FM/2 Window:ehp1.
command from the :hp1.Files menu:ehp1. with the Drive Tree active, 
(preferred) a context menu obtained on the directory you want to open
(remember, you get a context menu by clicking mouse button 2 on an
object) or the accelerator key Ctrl + o.
:p.
:artwork name='bitmaps\openmenu.bmp' align=center.
:p.
Yet another alternative is to invoke the :hp1.Utilities->Open Directory
Container:ehp1. command (Ctrl + O accelerator -- remember, that "O"
needs to be uppercase, so Shift if necessary). Using this command
invokes the :link reftype=hd res=91500. Walk Directories:elink. dialog
and lets you pick the directory to open.
:p.
:artwork name='bitmaps\openmnu2.bmp' align=center.
:p.
The :hp1.Walk:ehp1. (walking dude) :hp1.button:ehp1. near the bottom
right corner of the FM/2 window also invokes this command. You can even
drag a file system object onto this button to open a new Directory
Container for that object's directory.
:p.
:artwork name='bitmaps\walkbutt.bmp' align=center.
:p.
Finally, if you have the Drivebar on (see the :link reftype=hd
res=92000.Config menu:elink. topic) and a 3-button mouse, you can click
B3 (or hold down Ctrl while clicking B2) on one of the drive icons to
open a Directory Container for that drive (unless one already exists, in
which case it'll be surfaced and activated).
:p.
Now, if that's not enough ways to open a Directory Container, I give up --
go on back to Windoze and the Billy-mandated one way. &colon.-)
:p.
You can also open directories from their objects in a Directory Container
via context menu, accelerator key, Files menu or using the Shift modifier while
double-clicking.

:h2 res=91800 name=PANEL_TOOLBAR.Toolbar
:i1 id=aboutToolbar.Toolbar

:artwork name='bitmaps\toolbar.bmp' align=center.
:p.
The :hp1.toolbar:ehp1. is a collection of buttons that invoke some of
the commands in the pulldown or popup menus. Placing the mouse pointer
on a button and pressing and holding B2 displays brief help for the
button on the titlebar.
:p.
Some of the buttons will allow objects to be dragged onto them; for
example, you can drag objects onto the trashcan to delete them. Note
that the hotspot of the mouse pointer itself should be over the button
before releasing, not the icon being dragged (icons are slightly offset
from the mouse pointer to give better target visibility). Target
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
Toolbar buttons are user-configurable. To change the toolbar, click
mouse button two (usually the right button) while the mouse pointer is
over a button to pop up a context menu. You can get the "Load Toolbox"
dialog by clicking B2 on a blank area of the toolbar, and the "Add
Button" dialog by chording on a blank area of the toolbar.
:p.
For advanced/curious users&colon. Information on what buttons are in the
toolbar is kept in a file named FM3TOOLS.DAT. This file is an ASCII
(plain text) file that contains information defining the toolbar. The
file contains comments that explain its format.
:p.
FM/2 allows you to create customized toolboxes that can be loaded as
required to provide toolboxes for specific activities. Although FM/2
provides ways to edit the toolbar internally, it's probably easier to
load the definition files into a text editor and make the changes
manually en masse.
:p.
If you'd like to see a different toolbar layout (and try out the Load
Toolbox command), right-click on the toolbar, pick Load Toolbox from the
resultant menu, and pick a toolbox from the listbox. Then you might try
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
to rearrange the order of the toolbar's tool buttons. You take selected
items from the left listbox and Add them to the end of the right listbox
with the :hp1.Add>>:ehp1. button. When you've moved everything to the
right listbox, click :hp1.Okay:ehp1.. Click :hp1.Cancel:ehp1. if you
change your mind.
:p.
In reality, you don't need to move everything to the right listbox. You
can move only what you want moved to the top of the list, then click
Okay. Anything remaining in the left listbox is added to the end of
what's in the right listbox.
:p.
The :hp1.<<Remove:ehp1. button can be used to move selected items from
the right listbox to the bottom of the left listbox.

:h3 res=94400 name=PANEL_EDITTOOL.Edit Tool
:i1 id=aboutEditTool.Edit Tool

This dialog allows you to change the help and text strings and flags
associated with a button. Edit as desired, then click Okay. Click
Cancel if you change your mind.
:p.
The :hp1.Help:ehp1. field should contain text to remind you what the
button's command does. For example, a button that deletes files and
directories might have help reading "Delete files and directories".
:p.
The :hp1.Text:ehp1. field should contain very brief text that will be
placed on the button (if the tool id has no bitmap associated with it).
:p.
The :hp1.Droppable:ehp1. checkbox determines if the tool can have objects
dropped on it or not. See list below.
:p.
The :hp1.Visible:ehp1. checkbox determines if the tool is visible or not.
:p.
If the :hp1.Separator:ehp1. checkbox is checked, FM/2 puts a bit of
whitespace after the button (separates it from the next button).
:p.
If you check the :hp1.User-defined bmp:ehp1. checkbox, FM/2 will allow
you to use your own bitmap. Bitmaps are named after the :hp1.ID:ehp1.
of the button -- for example, the bitmap file for ID 1005 would be named
"1005.bmp". Clicking the :hp1.Edit bmp:ehp1. button will cause ICONEDIT
to be loaded with the bitmap, ready to edit. (Note that bitmaps should
be 28 x 28.)
:p.
The :hp1.ID:ehp1. field identifies the command that is associated with
this button. See the :link reftype=hd res=96401.Tool IDs:elink. topic.

:h3 res=96400 name=PANEL_ADDTOOL.Add Tool
:i1 id=aboutAddTool.Add Tool

This dialog is accessed from the context menu of a tool button, or if you
turn on the toolbar when there are no tools defined.
:p.
To add a tool, fill in the fields as appropriate and click Okay. Click
Cancel if you change your mind.
:p.
The :hp1.Help:ehp1. field should contain text to remind you what the
button's command does. For example, a button that deletes files and
directories might have help reading "Delete files and directories".
:p.
The :hp1.Text:ehp1. field should contain very brief text that will be
placed on the button (if the tool id has no bitmap associated with it).
:p.
The :hp1.Droppable:ehp1. checkbox determines if the tool can have objects
dropped on it or not. See list below.
:p.
The :hp1.Visible:ehp1. checkbox determines if the tool is visible or not.
:p.
If the :hp1.Separator:ehp1. checkbox is checked, FM/2 puts a bit of
whitespace after the button (separates it from the next button).
:p.
If you check the :hp1.User-defined bmp:ehp1. checkbox, FM/2 will allow
you to use your own bitmap. Bitmaps are named after the :hp1.ID:ehp1.
of the button -- for example, the bitmap file for ID 1005 would be named
"1005.bmp". Clicking the :hp1.Edit bmp:ehp1. button will cause ICONEDIT
to be loaded with the bitmap, ready to edit.
:p.
The :hp1.ID:ehp1. field should be assigned a number that tells FM/2 what
the command associated with the button is. See :link reftype=hd
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
Note&colon. Other Commands can also be used; just add to 4000 as required
to get the number of the command. For instance, Command 4 would be ID
4004. Command buttons use the text field; others shown have bitmaps
associated with them in FM/2's resources. Remember that if you change
your Commands around, you need to resync the tools to the Commands...
:p.
Note&colon. Toolbox buttons on a toolbar cause the appropriate toolbox
to be loaded from QUICKTLS.DAT (one filename per line, blank lines and
lines beginning with ";" ignored). The file named on the first line is
the First Toolbox, and so forth. You can have up to 50 toolbox buttons
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
will be overwritten). FM/2 saves the toolbox definitions into the file.
:p.
The files are given an extension of &period.TLS if you don't specify an
extension. The listbox shows files in the FM/2 directory that have the
extension .TLS (which is my way of suggesting that you follow this
convention).

:h3 res=94500 name=PANEL_LOADTOOLS.Load Toolbox
:i1 id=aboutLoadTools.Load Toolbox

FM/2 allows you to load toolboxes (groups of tools) which were saved
with the :link reftype=hd res=94000.Save Toolbox:elink. command.
:p.
Type the name of the file that holds the toolbox definitions into the
entry field below the listbox, or select a file displayed in the
listbox. FM/2 loads the toolbox definitions from the file and updates
the toolbar.
:p.
This allows you to have groups of specific-purpose buttons which you can
load for different types of activities. You don't have to use it, but
it's there if you would like to do so.
:p.
Clicking :hp1.Okay:ehp1. without typing in or selecting a filename
results in FM/2 reloading the default toolbox from FM3TOOLS.DAT.
:p.
If I may, here's how I'd recommend using this facility&colon.
:p.
First, save the full toolbar under a new name (perhaps ALL.TLS). Next,
delete any buttons that you don't want in your customized toolbar.
Finally, add any Command buttons that you do want in the customized
toolbar and save the new toolbox (as <something>.TLS). By starting with
the full toolbar, you'll spend more of your time deleting buttons you
don't want (a trivial task) and less adding buttons.

:h2 res=99400 name=PANEL_USERLISTS.Using quicklists
:i1 id=aboutUserLists.Using quicklists

FM/2's optional quicklists (dropdown combo boxes) provide quick access
to&colon.
:ul compact.
:li.Drives in the Drive Tree.
:li.Saved states.
:li.User-defined commands
:li.Often used directories you've configured (see :link reftype=hd
res=91500.Walk Directories dialog:elink.).
:li.Toolboxes (if the Toolbar's on).
:eul.
:artwork name='bitmaps\userlist.bmp' align=center.
:p.
.br
You turn the quicklists on and off using the :hp1.Config->Toggle
quicklists:ehp1. command (F8 accelerator).
:p.
The first list (starting from the left) contains drives. If you select a
drive from the list, the Drive Tree scrolls to that drive and makes it the
current object in the Drive Tree. This can be handy when you have the tree
expanded and don't want to collapse it. This list is readonly.
:p.
:artwork name='bitmaps\userdriv.bmp' align=center.
:p.
.br
The second list is the State list, containing the names of saved States of
the FM/2 window (what directories are open, where the windows are located,
etc.). You can add to this list by typing a name into the entry field,
requesting a context menu on the entry field, then selecting :hp1.Save
State as name:ehp1. from the context menu. FM/2 will save its current
State under that name for later recall.
:p.
:artwork name='bitmaps\useradds.bmp' align=center.
:p.
This can be handy if you have different setups that you use frequently,
as you can save and recall them when you need them rather than opening
containers and setting them up to point at the directories you want and
positioning them where you want each time. For example, let's say
you're connected to a LAN and have a BBS system running. You might want
one State set up specifically for dealing with the LAN, one set up
specifically for maintaining the BBS, and another (or a few) for general
work. Once you've set FM/2 up in each of these conditions and saved the
State under an easy-to-recall name, you can quickly move from State to
State using the State list. You'll find this :hp2.much:ehp2. faster and
more flexible than the way DOS-think file managers do things.
:p.
To "edit" a State, first recall it, then change what you want, then
delete the State name, then save it again.
:p.
Holding down the Shift key while selecting a state name prevents FM/2
from closing windows that are already open.
:p.
:artwork name='bitmaps\userstat.bmp' align=center.
:p.
.br
The third list is the user-defined commands list. User-defined commands
allow you to extend FM/2 with commands of your own devising. You can
add, change and delete commands with the :hp1.Config->Edit Commands:ehp1.
menu item (or click B2 on the list).
:p.
:artwork name='bitmaps\cmdlist.bmp' align=center.
:p.
.br
The fourth list is the commonly used directory list. You can add a
directory to this list as above; type in the name and select :hp1.Add
pathname:ehp1. from a context menu.
:p.
:artwork name='bitmaps\useraddd.bmp' align=center.
:p.
You can also drag the pathname from the entry field (you'll be dragging
the directory itself), or drag an object onto the field to add it to
the list. For example, if you have a Directory Container open and
decide it'd be nice to have the directory name in this list, start a
drag in the large status box at the top of the Directory Container (just
to the right of the Folder button) and drop the dragged folder on this
list.
:p.
Selecting a directory name from the list results in FM/2 opening a new
Directory Container for that path (hold down the Shift key while
selecting the path to cause the current Directory Container to switch
instead). You can reverse this action with the :hp1.Quicklist
switches:ehp1. toggle in the :link reftype=hd res=98400.Settings
Notebook:elink.. In addition, the :hp1.Recent Dirs:ehp1. toggle can be
used to cause FM/2 to automatically add temporary entries for
directories that you visit during a session. You can add them as above
to make them permanent if you desire.
:p.
Selecting the :hp1.<New directory>:ehp1. item from this list allows you
to create a new directory (same as using the Ctrl + M accelerator key).
:p.
The commonly used directory list, in combination with the State list,
allows you to quickly place FM/2 into precisely the configuration you
need to get real work done.
:p.
:artwork name='bitmaps\userdirs.bmp' align=center.
:p.
.br
The last list (only present if the Toolbar is on) is a list of
Toolboxes. You can select one from the list. FM/2 automatically
maintains this list for you. Initially, the saved toolbox names are
read from disk (the FM/2 directory), and any you subsequently save
are added to it on the fly. The FM/2 archive includes a few Toolboxes
for you to look at and customize if you'd like.
:p.
:artwork name='bitmaps\usertool.bmp' align=center.
:p.
You can use the font and color palettes to set the colors and fonts
used in these windows. Note that you must set the entry field and
listbox components separately. To set the listbox component, drop
onto the button, not the entry field.
:p.
The :hp1.Interface:ehp1. (list-and-arrow) :hp1.button:ehp1. at the
bottom right corner of the FM/2 window toggles the quicklists on and off.
:p.
:artwork name='bitmaps\userbutt.bmp' align=center.

.im folder.ipf

.im databar.ipf

.im tutor.ipf

.im expert.ipf

:h1 res=93300 name=PANEL_FILESMENU.Files Menu
:i1 id=aboutFilesMenu.Files Menu
The Files pulldown menu displays the same menu that would be obtained as
a :link reftype=hd res=93700.context (popup) menu:elink. over the
current object in the current window. If you select the Files menu when
a window that doesn't use it is active, you'll see "n/a" (not
applicable).
:p.
Generally speaking, it's best to leave the Files menu for folks who either
don't have a mouse or don't really understand how OS/2 works, and instead
work from the :link reftype=hd res=93705.context menu:elink.s.
:p.
See also :link reftype=hd res=92900.Commands submenu:elink..

:h2 res=92900 name=PANEL_COMMANDOVER.Commands submenu
:i1 id=aboutCommandsOver.Commands submenu

:link reftype=hd res=90700.Commands:elink. are programs that can be run
on selected objects by picking the program's title from a
dynamically built submenu of FM/2's Files menu.
:p.
When commands are displayed in the submenu, visual queues are given as
to the behavior of a given command. Commands that are checked will run
once for each selected file. Commands that are framed will prompt the
user to edit and accept the command line before running.
:p.
FM/2 provides accelerator keys for the first twenty commands in the
submenu. The accelerators are listed beside the command's title for
reference. This provides a "macro key" capability.
:p.
Don't overlook the power of Commands. This is a simple way of extending
FM/2 to do things that it can't do on its own, to automate things, and
to merge those old command line utilities with a PM selection shell (FM/2).

:h1 res=93800 name=PANEL_VIEWSMENU.Views Menu
:i1 id=aboutViewsMenu.Views Menu
The Views pulldown menu displays the same menu that would be obtained as
a context (popup) menu for the current window (but not for any objects
in the window -- in other words, a :link reftype=hd res=93700.context
menu:elink. requested over whitespace). If you select the Views menu
when a window that doesn't use it is active, you'll see "n/a" (not
applicable).
:p.
Generally speaking, it's best to leave the Views menu for folks who
either don't have a mouse or don't really understand how OS/2 works, and
instead work from the :link reftype=hd res=93710.context:elink. :link
reftype=hd res=93715.menu:elink.s.

.im util.ipf

.im config.ipf

.im window.ipf

.im context.ipf

.im arclist.ipf

.im cmdline.ipf

.im walkem.ipf

.im getnames.ipf

.im chklist.ipf

.im editor.ipf

.im hints.ipf

.im term.ipf

.im fm4.ipf

.im errors.ipf

:index.
:euserdoc.
