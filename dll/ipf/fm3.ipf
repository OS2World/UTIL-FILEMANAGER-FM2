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
within the help window).  The Contents view of a help file is something
like the Table of Contents in a book, with each major topic representing
a chapter.  Some topics have a "+" sign beside them to indicate that
there are subtopics available in a hierarchal inverted tree structure;
click the "+" and they'll be revealed.
:p.
The help sometimes has hypertext links shown in a different color, like
the words "Context menu" a few paragraphs below.  You can select these
links to switch to a different topic related to the highlighted word(s).
In this way you can browse through the help, moving from topic to topic
as you feel the need for more specific help.  The :hp1.Previous:ehp1.
button (or the Escape key) will move backwards through the hypertext
links you've followed.
:p.
To find something on a specific topic, click the :hp2.Search:ehp2.
button at the bottom of the help window and enter some text.  This might
be analogous to the index at the back of a book (although the help
manager also provides an "index" of its own, which is something like the
Contents window with the hierarchy removed -- difficult to browse, in my
opinion).
:p.
A couple of Search examples to give you the feel of the process&colon.
:p.
To find out about changing a volume label, enter "Label" as the "Search
for&colon." text, check the "All sections" checkbox, then press [Enter].
:p.
To find out how to open a new FM/2 window, enter "Open" as the "Search
for&colon." text, check the "All sections" checkbox, then press [Enter].
:p.
If you'd like a printout of any of the topics in the online help, click
the :hp2.Print:ehp2. button at the bottom of the help window.  The :link
reftype=hd res=93700.Context menu:elink. help topic would probably be a
good one to print out.  Gives you something to read in the john.
Remember that you can also use the :hp1.FM/2 Online Help:ehp1. icon in
the FM/2 folder to view the help without starting FM/2 itself.
:p.
If you're stuck in a dialog, click that dialog's Help button.  That will
usually take you directly to appropriate help.  One note of caution:
if you call up help for a dialog, be sure to dismiss the help before
dismissing the dialog.  Many of the dialogs run in threads other than
thread 1 (the main thread), and there's a long-standing OS/2 bug that
causes weird behavior if you do it the other way around to a dialog run
in a thread other than thread 1.  Trust me.
:p.
So, if you're totally new to FM/2, :hp1.how to get started?:ehp1.
First, run the :hp2.OS/2 Tutorial:ehp2. if you need help on the basics
of using a mouse in general or using one with OS/2 in particular.  Next,
read the help section titled :link reftype=hd
res=91300.Terminology:elink. so we'll understand each other a bit
better.  If you're still nervous about OS/2 conventions, try FM/2's
:link reftype=hd res=99800.Tutorial:elink. topic for real hand-holding.
Get an overview of some important FM/2 windows in the :link reftype=hd
res=97600.Window layouts:elink. topic.  Then plunge into the :link
reftype=hd res=90000.General Help:elink. topic to find out how to look
at, Move, Copy, Rename and Compare files and directories (the basics).
That will get you started, and we'll give you hints along the way about
other places you might like to look (like :link reftype=hd
res=93000.Hints:elink. &colon.-) when you're ready.  The previously
mentioned :link reftype=hd res=93700.Context menus:elink. topic will
give you an overview of all the commands available in FM/2 (and there
are a lot of commands you can use).
:p.
Since some folks are at first overwhelmed by FM/2's configurability,
take a look in the internal :link reftype=hd res=94600.Settings
notebook:elink. (accessible under the :link reftype=hd
res=92000.Config:elink. menu), specifically at the :link reftype=hd
res=99200.Quick page:elink..  There you'll see a few "standard"
configurations you can try out to perhaps get an idea of the range of
appearance and performance you can get out of FM/2 via the Settings
notebook and Config menu.
:p.
Command line help is in the :link reftype=launch object='E.EXE'
data='READ.ME'.READ.ME:elink. file that accompanied the archive,
since you should have read that before trying to start FM/2. You did,
didn't you?  
:p.
There is one thing you should keep in mind about FM/2.  FM/2 is
extremely powerful and has a lot of features, but :hp6.you don't have to
use or even know them all:ehp6..  Most people will use only a few of
FM/2's features on a regular basis (and not everyone will use the same
combination), and that's fine -- find what works best for you and
:hp2.use:ehp2. it.  If you find yourself needing some other feature,
call up the help, find it, and use it -- it'll be there whenever you
need it.  But don't feel that, somehow, by not using every little nook
and cranny of the program that you're missing out on something.  The
idea is to use what you need; pick your tools from the arsenal and get
some work done.
:p.
On the other hand, always assume that there's a way to do what you want
with FM/2 (chances are good that there is), and ask the help window to
Search for it.  If you can't find it there, drop me a line.

:h1 res=97600 name=PANEL_FM2WINDOWLAYOUT.FM/2 Window Layouts
:i1 id=aboutFM2WINDOWLAYOUT.FM/2 Window Layouts
Here's the main FM/2 window in all its glory (everything turned on)&colon.
:artwork align=left name='bitmaps\oall.bmp'.
:p.
:hp6.Note&colon.:ehp6.  You can turn optional windows and controls on
and off.  Pick the ones you like, get the others out of your way (see
the Config menu).  Surely no one uses all of them at the same time...
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
The :hp1.Toolboxes quicklist:ehp1. only appears when the
:hp1.Toolbar:ehp1. is visible (see :link reftype=hd res=99400.Using
quicklists:elink. topic).
:p.
Status bar #1 can be clicked to shift the focus to FM/2 without
activating any commands.  In addition, if the Drive tree is the
active window within FM/2's monolithic window, the Swapfile and
memory available indicators in Status bar #2 will be immediately
updated (otherwise they update about once every ten seconds).
:p.
You can get information on most parts of the window just by moving the
mouse pointer over the part of interest (unless you've turned off
bubble help in the internal Settings notebook).  For help with the
quicklists, which don't have bubble help, see :link reftype=hd
res=99400.this topic:elink..
:p.
.br
Here's a Directory Container window in more detail&colon.
:artwork align=left name='bitmaps\dircnr.bmp'.
:p.
Here's :link reftype=hd res=100000.FM/2 Lite:elink. with an explanation
of the things that are unique to it (except for the Alt-click to change
sort in Details view -- that works in FM/2, too).  The toolbar and
Autoview windows have been turned off to unclutter this illustration.
:artwork align=left name='bitmaps\fm4oall.bmp'.
:p.
Note that the current window, which would be affected by a pulldown menu
command, is surrounded by a light red border.  In this case, it's the
Drive Tree, so the Directory Container, or pane, that would change if a
directory were double-clicked in the Tree is surrounded by a darker red
border.  This color coding gives you instant feedback as to how the
commands you select will work.
:p.
.br
Here's a look at the text file viewer window -- the default text file
viewer built into FM/2 that you get when you double-click a text file in
a Directory container (you can configure a different one if you
like)&colon.
:artwork align=left name='bitmaps\newview.bmp'.
:p.
Here's how the window looks if you double-click some lines of text&colon.
:artwork align=left name='bitmaps\newview2.bmp'.
Here we've used the bookmark listbox (which we filled by double-clicking
some lines of text) to make an index for the document being read, to
enable us to move to different sections quickly.  See the :link
reftype=hd res=99300.Internal viewer:elink. topic for more information.

:h1 res=90000 name=PANEL_GENERAL.General Help
:i1 id=aboutGeneral.General Help

FM/2's main purpose is to show you what's on your file system and let you
sling what's there around.
:p.
Here we'll cover the basics.  Some familiarity with OS/2's WPS
(WorkPlace Shell) is assumed.  If you need refreshing, run the OS/2
Tutorial.  It's also assumed that you've already read the :link
reftype=hd res=93200.How to use FM/2's help:elink. and :link reftype=hd
res=91300.Terminology:elink. topics.  If you feel that you need more
in-depth help when we're through with this topic, try FM/2's :link
reftype=hd res=99800.Tutorial:elink. topic.
:p.
There are several ways to view a directory with FM/2, just as there are
with the WPS.  Icon, Name, Text and Details views all offer different
perspectives into the directory being "looked at" (see :link reftype=hd
res=91300.Terminology:elink.).  Views showing icons can use full-sized
icons or smaller "mini-icons" to save space.
:p.
Details view can show a great deal of information about file system
objects, and you can customize what is shown with the :hp2.Details
Setup:ehp2. submenu (under the :link reftype=hd res=93800.Views
menu:elink. or a Directory Container window's :link reftype=hd
res=93700.context menu:elink.).
:p.
You can also place some limits on the amount of detail that FM/2 loads
from the file system with the :link reftype=hd res=92400.Toggles:elink.
page of the :link reftype=hd res=94600.Settings notebook:elink..  This
can speed up FM/2's scanning of directories but can also make for duller
screens and less information being presented to you.  My advice to you
is to enjoy the bells and whistles OS/2 and PM provide.
:p.
Take a moment to set up the look of your Directory Container windows
to match your taste -- everyone likes something different.  Then meet
me back here and we'll talk about manipulating those objects you see...
:p.
.br
As we talk about manipulating objects, keep firmly in mind the concepts
of :link reftype=hd res=98000."current object" and "highlighted
objects":elink.. The current object is the one on which commands act (it
has the dotted outline around it).  If the current object is also
highlighted, all highlighted objects will be affected.
:p.
:artwork name='..\..\bitmaps\rename.bmp' align=center.
:hp1.Renaming file system objects&colon.:ehp1.
The simplest way to rename a file system object is to point at it with
the mouse pointer, hold down the ALT key, and click the text of its name.
OS/2 produces a mini MLE text entry field where you can type in a new
name (this is :link reftype=hd res=98200.Direct Editing:elink.).  When
finished, click the object and a rename is performed.  Note that you can
even move the object to another directory when you do this.  (Also note
that in Details view you can direct-edit the Subject field to change an
object's Subject, and the Longname field to change an object's Longname
on FAT drives.)
:p.
Using this method will not allow you to overwrite an existing file.  You
can use drag and drop (as detailed below for Move) or the menu command
:link reftype=hd res=91400.Rename:elink. or the :link reftype=hd
res=91800.toolbar:elink. to allow overwriting.
:p.
If you'd like more detail, go to the :link reftype=hd
res=99810.Rename:elink. tutorial.
.br
:artwork name='..\..\bitmaps\mover.bmp' align=center.
:hp1.Moving file system objects&colon.:ehp1.
.br
There are several ways to move a file system object.  The best and most
intuitive is :hp2.drag and drop:ehp2..  Using this method, you "grab"
the file system object by pressing and holding B2 while the mouse
pointer is over the object, then begin to move the mouse (still holding
B2).  The object's icon should begin to move with the mouse pointer.
"Drag" this icon to where you want to move it (for instance, if you want
to move a file from C&colon.\ to D&colon.\, drag the file to the Drive
Tree's D&colon.\ object).  When the object is where you want it, release
B2 and the move is done.
:p.
When dragging an object into a Directory Container, remember that to
place it into the directory into which the Directory Container "looks"
you need to drop it on container "whitespace" (a part of the container
not occupied by an object).  For convenience, the two large status
areas at the top of the container are considered whitespace.
:p.
If you get confused when dragging object(s), press the F1 key.  This
will give you some information about what you're doing.  Pressing the
Escape key will abort the drag.
:p.
Note that you can't move a file or directory onto another file (except
for archive targets), only into a directory (moving into container
whitespace in a Directory Container window is the same as moving into
the directory the Directory Container "looks" into, and a minimized
Directory Container window is "all whitespace").  Also note that the
object you grab becomes the current object, and if it's also highlighted
you'll drag all highlighted objects (you'll see visual feedback to this
effect).
:p.
You could, of course, also select "Move" from the :hp1.:link reftype=hd
res=93300.Files:elink.:ehp1. menu or a context menu, or click the Move
toolbar button, or type the accelerator key Ctrl + m (hold the control
key down and type "m").  In this case, you'll get the :link reftype=hd
res=91500.Walk Directories:elink. dialog where you can enter a target
directory.
:p.
If you'd like more detail, go to the :link reftype=hd
res=99820.Move:elink. tutorial.
.br
:artwork name='..\..\bitmaps\copier.bmp' align=center.
:hp1.Copying file system objects&colon.:ehp1.
.br
The procedure for copying file system objects is very similar to that
for moving them.  When you begin to drag the object, and until you
release it, hold down the control (Ctrl) key.  You'll notice that the
dragged icon is "ghosted" to give visual feedback that a copy, not a
move, is being performed.  Note that you can copy a file onto an archive
file as well as into a directory.  You can also "clone" a file by dropping
it into the directory where it already resides -- you'll get a rename
dialog that will allow you to change the name, creating a file exactly
like the other with a different name.
:p.
As for move above, there is a "Copy" menu item and a toolbar button,
and Ctrl + c is the accelerator key.
:p.
If you'd like more detail, go to the :link reftype=hd
res=99830.Copy:elink. tutorial.
.br
:artwork name='..\..\bitmaps\linkdrag.bmp' align=center.
:hp1.Comparing file system objects&colon.:ehp1.
There is one other type of drag and drop operation called a "link drag."
To link drag, hold down the Ctrl and Shift keys while dragging.
You'll see a "rubber band line" extend from where you grabbed the icon
to the mouse pointer as a visual cue.  Link dragging is usually used
within FM/2 to do compare operations (see also :link reftype=hd
res=99950.:hp2.Link Sets Icon:ehp2. toggle:elink.). What you drag will
be compared to what you drop it on.  Note, however, that if you drag to
a WPS object (like the desktop or other folder), OS/2's version of a
link drag is performed, which usually results in a shadow object being
created.
If you'd like more detail, go to the :link reftype=hd
res=99840.Compare:elink. tutorial.
:p.
.br
Double-clicking an object in a Directory Container window causes a
:hp1.default action:ehp1. to take place.  What that action is depends on
the type of object and how you've configured FM/2.  Briefly, the FM/2
defaults (which you can override) cause the object to be opened in the
most likely manner.  You can :link reftype=hd res=100060.read about this
in more detail here:elink. in the tutorial section.
:p.
Other commands are accessed via :link reftype=hd
res=97700.pulldown:elink. or :link reftype=hd res=93700.context
menu:elink. commands or toolbar buttons.  You can read about them by
selecting the highlighted words "context menu" in this paragraph.  FM/2
also offers several general :link reftype=hd res=92100.utilities:elink.
and many :link reftype=hd res=92000.configuration:elink. options which
you may want to explore. But you now know how to perform the file system
maintenance basics: Move, Copy, Rename and Compare.  You are now, as the
Smothers Brothers said, educated.
:p.
For more specific information on FM/2, click the :hp2.Contents:ehp2.
button at the bottom of the help window.  If you're still confused on
the basics, try the FM/2 :link reftype=hd res=99800.Tutorial:elink.
topic.  You might also want to look at the :link reftype=hd
res=98900.Opening a Directory Container:elink., :link reftype=hd
res=99400.Using quicklists:elink. and :link reftype=hd res=97600.Window
layout:elink. topics.

:h1 res=98900 name=PANEL_GENERALOPEN.Opening a Directory Container
:i1 id=aboutOpeningDirectory.Opening a Directory Container

There are several ways to open a Directory Container.
:p.
The most common is to double-click a directory in the Drive Tree.  If no
Directory Container windows are open, one is created.  If one or more
Directory Container windows are already open, hold down the Shift key
while double-clicking (see picture below) -- otherwise the default
action is to switch the most recently used Directory Container to the
new directory rather than open a new one.
:artwork name='bitmaps\open.bmp' align=center.
:p.
An alternative is to click the :hp1.Open button:ehp1. in the Drive
Tree's titlebar (see location indicated by the mouse pointer in the
snapshot below). This opens a window for the directory highlighted in
the Drive Tree.
:artwork name='bitmaps\openbutt.bmp' align=center.
:p.
Another alternative is to select the :hp1.Open->New FM/2 Window:ehp1.
command from the :hp1.Files menu:ehp1. with the Drive Tree active, or
(preferred) a context menu obtained on the directory you want to open
(remember, you get a context menu by clicking mouse button 2 on an
object).
:artwork name='bitmaps\openmenu.bmp' align=center.
:p.
Yet another alternative is to invoke the :hp1.Utilities->Open Directory
Container:ehp1. command (Ctrl + O accelerator -- remember, that "O"
needs to be uppercase, so Shift if necessary).  Using this command
invokes the :link reftype=hd res=91500. Walk Directories:elink. dialog
and lets you pick the directory to open.
:artwork name='bitmaps\openmnu2.bmp' align=center.
The :hp1.Walk:ehp1. (walking dude) :hp1.button:ehp1. near the bottom
right corner of the FM/2 window also invokes this command.  You can even
drag a file system object onto this button to open a new Directory
Container for that object's directory.
:artwork name='bitmaps\walkbutt.bmp' align=center.
:p.
Finally, if you have the Drivebar toggled on (see the :link reftype=hd
res=92000.Config menu:elink. topic) and a 3-button mouse, you can click
B3 (or hold down Ctrl while clicking B2) on one of the drive icons to
open a Directory Container for that drive (unless one already exists, in
which case it'll be surfaced and activated).
:p.
Now, if that's not enough ways to open a Directory Container, I give up --
go on back to Windoze and the Billy-mandated one way.  &colon.-)
:p.
You can also open directories from their objects in a Directory Container
via context menu, Files menu or using the Shift modifier while
double-clicking.

:h1 res=99400 name=PANEL_USERLISTS.Using quicklists
:i1 id=aboutUserLists.Using quicklists

FM/2's optional quickists (dropdown combo boxes) provide quick access
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
The first list (starting from the left) contains drives.  If you select a
drive from the list, the Drive Tree scrolls to that drive and makes it the
current object in the Drive Tree.  This can be handy when you have the tree
expanded and don't want to collapse it.  This list is readonly.
:artwork name='bitmaps\userdriv.bmp' align=center.
:p.
.br
The second list is the State list, containing the names of saved States of
the FM/2 window (what directories are open, where the windows are located,
etc.).  You can add to this list by typing a name into the entry field,
requesting a context menu on the entry field, then selecting :hp1.Save
State as name:ehp1. from the context menu.  FM/2 will save its current
State under that name for later recall.
:artwork name='bitmaps\useradds.bmp' align=center.
This can be handy if you have different setups that you use frequently,
as you can save and recall them when you need them rather than opening
containers and setting them up to point at the directories you want and
positioning them where you want each time.  For example, let's say
you're connected to a LAN and have a BBS system running.  You might want
one State set up specifically for dealing with the LAN, one set up
specifically for maintaining the BBS, and another (or few) for general
work.  Once you've set FM/2 up in each of these conditions and saved the
State under an easy-to-recall name, you can quickly move from State to
State using the State list.  You'll find this :hp2.much:ehp2. faster and
more flexible than the way DOS-think file managers do things.
:p.
To "edit" a State, first recall it, then change what you want, then
delete the State name, then save it again.
:p.
Holding down the Shift key while selecting a state name prevents FM/2
from closing windows that are already open.
:artwork name='bitmaps\userstat.bmp' align=center.
:p.
.br
The third list is the user-defined commands list.  User-defined commands
allow you to extend FM/2 with commands of your own devising.  You can
add, change and delete commands with the :hp1.Config->Edit Commands:ehp1.
menu item (or click B2 on the list).
:artwork name='bitmaps\cmdlist.bmp' align=center.
:p.
.br
The fourth list is the commonly used directory list.  You can add a
directory to this list as above; type in the name, select :hp1.Add
pathname:ehp1. from a context menu.
:artwork name='bitmaps\useraddd.bmp' align=center.
You can also drag the pathname from the entry field (you'll be dragging
the directory itself), or drag an object onto the control to add it to
the list.  For example, if you have a Directory Container open and
decide it'd be nice to have the directory name in this list, start a
drag in the large status box at the top of the Directory Container (just
to the right of the Folder button) and drop the dragged folder on this
list.
:p.
Selecting a directory name from the list results in FM/2 opening a new
Directory Container for that path (hold down the Shift key while
selecting the path to cause the current Directory Container to switch
instead).  You can reverse this action with the :hp1.Quicklist
switches:ehp1. toggle in the :link reftype=hd res=98400.Settings
Notebook:elink..  In addition, the :hp1.Recent Dirs:ehp1. toggle can be
used to cause FM/2 to automatically add temporary entries for
directories that you visit during a session.  You can add them as above
to make them permanent if you desire.
:p.
Selecting the :hp1.<New directory>:ehp1. item from this list allows you
to create a new directory (same as using the Ctrl + M accelerator key).
:p.
The commonly used directory list, in combination with the State list,
allows you to quickly place FM/2 into precisely the configuration you
need to get real work done.
:artwork name='bitmaps\userdirs.bmp' align=center.
:p.
.br
The last list (only present if the Toolbar is on) is a list of
Toolboxes. You can select one from the list.  FM/2 automatically
maintains this list for you.  Initially, the saved toolbox names are
read in from disk (the FM/2 directory), and any you subsequently save
are added to it on the fly.  The FM/2 archive includes a few Toolboxes
for you to look at and customize if you'd like.
:artwork name='bitmaps\usertool.bmp' align=center.
:p.
You can use the font and color palettes to set the colors and fonts
used in these windows.  Note that you must set the entry field and
listbox components separately.  To set the listbox component, drop
onto the button, not the entry field.
:p.
The :hp1.Interface:ehp1. (list-and-arrow) :hp1.button:ehp1. at the
bottom right corner of the FM/2 window toggles the quicklists on and off.
:artwork name='bitmaps\userbutt.bmp' align=center.

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
on selected objects by picking the programs by an assigned title from a
dynamically built submenu of FM/2's Files pulldown menu.
:p.
When commands are displayed in the submenu, visual queues are given as
to the behavior of a given command.  Commands that are checked will run
once for each selected file.  Commands that are framed will prompt the
user to edit and accept the command line before running.
:p.
FM/2 provides accelerator keys for the first twenty commands in the
submenu.  The accelerators are listed beside the command's title for
reference.  This provides a "macro key" capability.
:p.
Don't overlook the power of Commands.  This is a simple way of extending
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

.im folder.ipf

.im arclist.ipf

.im cmdline.ipf

.im walkem.ipf

.im chklist.ipf

.im editor.ipf

.im hints.ipf

.im term.ipf

.im fm4.ipf

.im errors.ipf

.im keys.ipf

.im mouse.ipf

.im getnames.ipf

:index.
:euserdoc.
