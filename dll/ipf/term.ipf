.***********************************************************************
.*
.* $Id$
.*
.* Term definitions
.*
.* Copyright (c) 1993-98 M. Kimes
.* Copyright (c) 2006 Steven H.Levine
.*
.* 10 Sep 06 GKY Sync with F1 help changes
.*
.***********************************************************************
.*
:h1 res=91300 name=PANEL_TERMINOLOGY.Terminology
:i1 id=aboutTerminology.Terminology
Some definitions&colon.
:p.
:hp1.GUI:ehp1. = Graphical User Interface
:p.
:hp1.WPS:ehp1. = WorkPlace Shell, OS/2's GUI
:p.
:hp1.PM:ehp1. = Presentation Manager, OS/2's graphical supersystem
:p.
:hp1.:color fc=default bc=cyan.B1:color fc=default bc=default.:ehp1. = Mouse button 1, usually the left button
:p.
:hp1.:color fc=default bc=cyan.B2:color fc=default bc=default.:ehp1. = Mouse button 2, usually the right button
:p.
:hp1.:color fc=default bc=cyan.B3:color fc=default bc=default.:ehp1. = Mouse button 3, usually the middle button. Not all mice
have three buttons. OS/2 will let you redefine the buttons using the
WPS' Mouse object in the System Setup folder.
:p.
:hp1.:color fc=default bc=cyan.Chord:color fc=default bc=default.:ehp1. = Pressing :color fc=default bc=cyan.B1:color fc=default bc=default. and :color fc=default bc=cyan.B2:color fc=default bc=default. simultaneously
:p.
:hp1.:link reftype=hd res=97800.Context menu:elink.:ehp1. = a popup menu
obtained directly on an object of interest by clicking :color fc=default bc=cyan.B2:color fc=default bc=default. while the
mouse pointer is over the object. Context menus usually have options
tailored for the specific object.
:p.
:hp1.:link reftype=hd res=97600.Pulldown menu:elink.:ehp1. = the action
bar menu just under the title bar of a window. If you don't know what a
title bar is, run the OS/2 tutorial.
:p.
:hp1.:link reftype=hd res=97600.System menu:elink.:ehp1. = the icon at
the top left of most PM main windows. You can close a window by
double-clicking the System menu icon with :color fc=default bc=cyan.B1:color fc=default bc=default.. Click once and you get a
menu (called, oddly enough, the system menu).
:p.
:hp1.:link reftype=hd res=97700.Conditional cascade menu:elink.:ehp1. =
a submenu with the right-pointing link arrow contained in a button on
the menu item. When the button is clicked, the submenu displays
(cascades) as with a normal submenu. When the menu item itself is
clicked, a default action from the submenu is activated. You can see
which submenu item is the default as it has a checkmark by it. A
conditional cascade menu generally gives you a default option for a
command and several related commands; for instance, in an
:link reftype=hd res=90200.Archive Container:elink., the Extract conditional
cascade menu defaults to simply extracting the selected objects, but
clicking the arrow button reveals several extract options that can be
selected instead. FM/2 uses conditional cascade menus to keep the menus
organized in such a way that often used commands are immediately
available but many more commands are still accessible.
:p.
:hp1.:link reftype=hd res=97600.Toolbar:elink.:ehp1. = an array of
buttons, usually with pictures on them, that you can click with your
mouse to cause commands to be activated -- a sort of menu for
illiterates. The FM/2 toolbar can be turned on and off, can be changed
from icon to text buttons and back again, and can display text below
the icon buttons. Brief help appears on the title bar when the mouse
passes over a toolbar button.
:p.
:hp1.Toolbox:ehp1. = a collection of buttons in a toolbar. FM/2 allows
you to customize and save toolboxes which you can load into the toolbar
as desired.
:p.
:hp1.:link reftype=hd res=99000.Databar:elink.:ehp1. = a window to which
you can minimize FM/2 to or which can be run separately, that
shows occasionally updated information about your system, such as free
drive space, threads/processes, time/date, etc.
:p.
:hp1.Drag and drop:ehp1. = an intuitive way of manipulating objects --
this is more-or-less how you manipulate objects in the real world. If
you don't know how to drag and drop, you :hp2.really:ehp2. should run
the OS/2 Tutorial. Drag and drop is a superior method for manipulating
objects, as opposed to keyboard commands, because you choose both the
command and the target in one operation. We have limited the number of objects
per drag because large drags can overflow the draginfo buffer
causing the corruption of shared memory and forcing a reboot.
:p.
:hp1.Dialog:ehp1. = a specialized input or informational window that's
transient. You use it, then it goes away. Dialogs generally have their
own specialized help available via a Help button.
:p.
:hp1.:link reftype=hd res=98200.Direct Editing:elink.:ehp1. = a method
of changing the text of an object, as when you change the name of an
object on the WPS by pointing at it, holding down the :color fc=default bc=palegray.Alt:color fc=default bc=default.
 key and clicking the text with :color fc=default bc=cyan.B1:color fc=default bc=default.. FM/2 supports direct editing of file system
object names as well as Subject and Longname fields in Details view.
:p.
:hp1.Default action:ehp1. = what happens when you double-click an
object in a container (or put the cursor on it with the :color fc=default bc=palegray.arrow:color fc=default bc=default. keys
and press :color fc=default bc=palegray.Enter:color fc=default bc=default.)  The default action may sometimes be modified by
the Shift state of the keyboard (if you hold down :color fc=default bc=palegray.Shift:color fc=default bc=default. or :color fc=default bc=palegray.Ctrl:color fc=default bc=default.
while double-clicking or pressing :color fc=default bc=palegray.Enter:color fc=default bc=default.). The
:link reftype=hd res=90000.General Help:elink. topic explains default actions in detail.
:p.
:hp1.:link reftype=hd res=97700.Accelerator keys:elink.:ehp1. = key
combinations that allow you to quickly give a program a command without
going through menus or toolbars. For example, FM/2's accelerator key to
get help is :color fc=default bc=palegray.F1:color fc=default bc=default.. Note that accelerators are case sensitive, so
that :color fc=default bc=palegray.Ctrl:color fc=default bc=default. + :color fc=default bc=palegray.m:color fc=default bc=default. isn't the same as :color fc=default bc=palegray.Ctrl:color fc=default bc=default. + :color fc=default bc=palegray.M:color fc=default bc=default. -- you'll need to hold down
the :color fc=default bc=palegray.Shift:color fc=default bc=default. key or set :color fc=default bc=palegray.Caps Lock:color fc=default bc=default. to get the latter.
:p.
:hp1.:link reftype=hd res=97600.Drive Tree:elink.:ehp1. = the special
window that's always open in FM/2 displaying your drives in "tree"
format. If subdirectories are available, there will be a "+" sign to
the left of the drive which you can click to show the subdirectories.
Note that floppy drives (A&colon. and B&colon.) aren't checked for
subdirectories until you access them. Double-clicking a drive or
directory in the Drive Tree opens a Directory Container or switches the
current Directory Container to "look" into that directory (unless a
Directory Container "looking" into that directory already exists, in
which case it's brought to the foreground).
:p.
:hp1.:link reftype=hd res=97600.Directory Container:elink.:ehp1. = a
special window that "looks" into a particular directory and shows you
what's in it.
:p.
:hp1.:link reftype=hd res=90200.Archive Container:elink.:ehp1. = another
special window that "looks" into an archive file and shows you what's in
it.
:p.
:hp1.:link reftype=hd res=90100.Collector:elink.:ehp1. = yet another
special window that serves as a temporary storage place for file system
objects you place into it. Objects in the Collector are a little like
WPS Shadows in that they take up no additional space on your drives --
they just represent the objects so you can manipulate them.
:p.
:hp1.:link reftype=hd res=99400.Quicklists:elink.:ehp1. = optional
dropdown combo boxes at the top of the FM/2 window (below the toolbar,
if it's on) that give quick access to several often used things. Also
known as :hp1.user lists:ehp1..
:p.
:hp1.:link reftype=hd res=93400.Filter:elink.:ehp1. = what you do when
you selectively remove some of the file system objects from a container
by giving filemasks and/or attribute masks to "filter" what's displayed.
Filtering affects only what shows; the files and directories remain on
the drive.
:p.
:hp1.Mask:ehp1. = a filemask that can contain wildcard characters (* and
?) and select one or more files. In FM/2, filemasks can usually contain
multiple masks separated by semicolons. See :hp1.Filter:ehp1. link
above for more information.
:p.
:hp1.:link reftype=hd res=98000.Current object:elink.:ehp1. = the object
upon which commands will act (also called the :hp1.cursored
object:ehp1.). The current object in a container is indicated by a
dotted outline around the object. The current object may or may not
also be highlighted. There can only be one current object in a
container.
:p.
:hp1.:link reftype=hd res=98000.Highlighted objects:elink.:ehp1. =
objects in a container which are indicated by a different color (usually
darker) background. If the current object is also highlighted, commands
affect all highlighted objects. Highlighted objects are sometimes
referred to as :hp1.selected objects:ehp1.. Note that in the Drive Tree
there is always one highlighted object (which will also be the current
object), but in other containers there can be many or no highlighted
objects, and, depending on the selection type you use (see :link
reftype=hd res=97000.Windows toggle page:elink.), the current object may
not be a highlighted object. Thus understanding the distinction between
current and highlighted objects is important.
:p.
:hp1.Autoview window:ehp1. = an optional window (:hp1.Config->Toggle
Autoview Window:ehp1.) that shows you the first few lines of current
file objects.
:p.
:hp1.State:ehp1. = the state of the FM/2 main window at any given time
-- what directories are open, where their Directory Containers are
positioned, etc.
:p.
:hp1.Tree view:ehp1. = a container view similar to an inverted tree,
with roots at the top and leaves at the bottom. The FM/2 Drive Tree
is an example of this sort of view, as is the default view of an
OS/2 WPS Drive object.
:p.
:hp1.Icon view:ehp1. = a container view showing the object's name below
the object's icon. This is the default view for a WPS folder.
:p.
:hp1.Name view:ehp1. = a container view showing the object's name beside
the object's icon.
:p.
:hp1.Text view:ehp1. = a container view showing only the object's name.
Text view shows more objects for a given space than any other view, but
the least amount of information.
:p.
:hp1.Details view:ehp1. = a container view showing full object
information in rows. A detail container is split into two sides with
one vertical scrollbar serving both sides, and two separate horizontal
scrollbars. Details view shows more information on the objects within
it than any other view; it also displays fewer objects for a given space.
:p.
:hp1.MLE:ehp1. = Multi Line Edit control. This is something like a text
editor. They can (and do) come in all sizes.
:p.
:hp1.Container whitespace:ehp1. = an empty part of a container (a part
without an object on it).
:p.
:hp1.Monolithic application:ehp1. = FM/2 as run from the FM/2 program
object, with the big window containing a Drive tree and Directory
containers within it. You can also run parts of FM/2 separately.
:p.
See also these pictures&colon.
.br
:link reftype=hd res=97600.FM/2 Window Layout:elink.
.br
:link reftype=hd res=98000.Current object &amp. selected objects:elink.
.br
:link reftype=hd res=97700.Cascade menus &amp. accelerator keys:elink.
.br
:link reftype=hd res=97800.Context and sub menus:elink.
.br
:link reftype=hd res=97900.Drag and drop:elink.
.br
:link reftype=hd res=98200.Direct editing:elink.

:h2 res=98000 name=PANEL_CURRENTOBJ.Current object &amp. selected objects
:i1 id=aboutCURRENTOBJ.Current object &amp. selected objects
:artwork align=left name='bitmaps\current.bmp'.
:p.
The current object is the one with the dotted outline; the mouse pointer
is pointing at it. The selected objects are those with the darker
outline. If the current object is also selected, commands affect all
selected objects. Otherwise, commands affect only the current object.

:h2 res=97700 name=PANEL_CASCADEMENU.Cascade menus &amp. accelerator keys
:i1 id=aboutCASCADEMENU.Cascade menus &amp. accelerator keys
FM/2 with a cascade menu off a pulldown menu open&colon.
:p.
:artwork align=left name='bitmaps\cascade.bmp'.
:p.
Notice the checked default action "FM/2 Window" on the Open cascade menu
-- this is what would be executed if you clicked Open instead of on the
arrow button. Clicking the button opens the cascade menu. Standard
submenus have arrows that are not buttons; clicking anywhere on one of
thse items opens the submenu.
:p.
Notice that listed beside "FM/2 Window" is the :link reftype=hd
res=100005.accelerator key:elink. :color fc=default bc=palegray.Ctrl:color fc=default bc=default.
 + :color fc=default bc=palegray.o:color fc=default bc=default.. This is the command to open
a new FM/2 window, and is the fastest way to input a command.
:p.
Here's a picture of a cascade menu and a submenu&colon. note the
difference in appearance between the Miscellaneous cascade menu and the
Select submenu.
:p.
:artwork align=left name='bitmaps\submenu.bmp'.
:p.
Finally, note that the :link reftype=hd res=93300.Files:elink. pulldown
menu is a "ghost" for a context menu requested on a file/directory
object. The :link reftype=hd res=93800.Views:elink. pulldown is a
"ghost" for a context menu requested over container whitespace. Since
you can select the container and/or item while requesting a context menu
in one smooth motion with a mouse, context menus are faster than
pulldowns. The "ghosts" are mainly for people without a pointing device
available.

:h2 res=97800 name=PANEL_CONTEXTSUBMENU.Context and sub menus
:i1 id=aboutCONTEXTSUBMENU.Context and sub menus
FM/2 with a sub menu off a context menu open&colon.
:p.
:artwork align=left name='bitmaps\context.bmp'.
:p.
Here FM/2 is displaying a :link reftype=hd res=93700.context menu:elink.
that was requested over whitespace in the Drive Tree. This is the same
thing you'd get if you'd clicked the :link reftype=hd
res=93800.Views:elink. pulldown menu with the Drive Tree active
(titlebar highlighted). The :hp1.Sort:ehp1. submenu is open.

:h2 res=97900 name=PANEL_DRAGDROP.Drag and drop
:i1 id=aboutDRAGDROP.Drag and drop
FM/2 with a drag and drop operation underway&colon.
:p.
:artwork align=left name='bitmaps\dragdrop.bmp'.
:p.
Several files are being moved from a Directory Container to a directory
in the Drive Tree. Notice that in this picture several Directory
Containers are open with different views (Details, Text and Name view)
and that mini-icons are used in the Drive Tree container.

:h2 res=98200 name=PANEL_DIRECTEDIT.Direct editing
:i1 id=aboutDIRECTEDIT.Direct editing
:artwork align=left name='bitmaps\direct.bmp'.
:p.
An example of direct editing. Press and hold the :color fc=default bc=palegray.Alt:color fc=default bc=default. key while clicking
the filename in the container with :color fc=default bc=cyan.B1:color fc=default bc=default.. A new name can then be entered
into the small MLE by the object's icon (where the name usually shows),
and another click of  :color fc=default bc=cyan.B1:color fc=default bc=default. will accept the input, while :color fc=default bc=palegray.Esc:color fc=default bc=default. will abort
the operation.
:p.
Note that the entire pathname of the object is presented for editing --
this allows you to move the object as you rename it, if desired. The
filename portion of the pathname is initially highlighted for you, so
there's no extra work for a simple file rename. FM/2 also tries to
enlarge the MLE created to make things easier. Standard OS/2 editing
keys work as expected.
