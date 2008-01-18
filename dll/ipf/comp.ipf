.***********************************************************************
.*
.* $Id$
.*
.* Compare directories help
.*
.* Copyright (c) 1993-98 M. Kimes
.* Copyright (c) 2008 Steven H.Levine
.*
.* 18 Jan 08 SHL Update hide not selected documentation
.*
.***********************************************************************
.*
:h2 res=94900 name=PANEL_COMPARE.Compare Directories
:i1 id=aboutCompare.Compare Directories
:artwork name='bitmaps\linkdrag.bmp' align=center.
:p.
The :hp1.Compare Directories:ehp1. dialog shows you a comparison
breakdown of two directories. You can select files here and
:link reftype=hd res=90100.Collect:elink. them for later processing,
or drag them elsewhere.
:p.
The dialog presents two containers side-by-side. Vertical scrolling of
the containers is synced so that scrolling one scrolls both. The files
are listed so that they are in alphabetical order, with gaps in the
containers where a file exists in one but not the other. This provides
easy, at-a-glance comparison of the directories' contents.
:p.
It is possible to change the directory of a container by direct editing
of the container title. You can change both directories simultaneously
by clicking the :hp6.Dirs:ehp6. button. You can include all the files
in all subdirectories by checking the :hp6.Include subdirs:ehp6. button.
:p.
This dialog is reached by :hp1.link-dropping:ehp1. (:color fc=default bc=palegray.Ctrl:color fc=default bc=default.
 + :color fc=default bc=palegray.Shift:color fc=default bc=default. held
down while dragging and dropping) one directory object onto another
directory object, or by selecting :hp1.Utilities->Compare
directories:ehp1. from the main window's pulldown menu.
:p.
You can get a context (popup) menu in the title area of each container.
From it you can control which columns are displayed in the containers,
and save "snapshots" and, in the right container, reload those snapshots.
Snapshot files are compatible with :hp1.PMDirMatch:ehp1. (see below).
Snapshots can be used to see what changed in a directory from one point
in time to another (for instance, to see what installing a program might
have changed in your system directories). You can also
:hp1.Filter:ehp1. what shows in the containers.
:p.
The row of :hp1.Actions&colon.:ehp1. pushbuttons allow you to
:hp6.Delete:ehp6. files from either directory and to :hp6.Move:ehp6. or
:hp6.Copy:ehp6. files from one directory to another.
:p.
You can double-click a file to view it, or request a context menu on it to
compare it to its counterpart in the other directory.
To use an external compare utility, define it on the
:link reftype=hd res=98400.Compare Settings:elink.notebook page.
:p.
Several selection/deselection techniques are available via pushbuttons
to enable you to select files based on their comparison. The pushbuttons
operate on both containers. You can also select and deselect files in
the containers manually with the mouse or keyboard.
:hp2.Hint&colon.:ehp2. mouse selection of multiple files works slightly differently when you hold down :color fc=default bc=palegray.Ctrl:color fc=default bc=default. while clicking.
:p.
:hp2.The selection and deselection buttons are&colon.:ehp2.
:p.
:hp6.Same size:ehp6. All items which match name and size exactly.
:p.
:hp6.Size/time:ehp6. All items which match name, size and date exactly.
:p.
:hp6.Content:ehp6. All items which match name and content exactly.
:p.
:hp6.Both:ehp6. All items which have matching names in both containers.
:p.
:hp6.One:ehp6. All items which are present in only one of the
containers using only the name as criteria.
:p.
:hp6.Newer:ehp6. All items which have a counterpart in the other
container and are the newer of the pair.
:p.
:hp6.Older:ehp6. All items which have a counterpart in the other
container and are the older of the pair.
:p.
:hp6.Smaller:ehp6. All items which have a counterpart in the other
container and are the smaller of the pair.
:p.
:hp6.Larger:ehp6. All items which have a counterpart in the other
container and are the larger of the pair.
:p.
:hp6.All:ehp6. All items in both containers.
:p.
:hp6.Invert:ehp6. Inverts selection state of all items in both containers.
:p.
:hp2.The other buttons in the dialog are&colon.:ehp2.
:p.
:hp6.Actions:ehp6. Provides directional copy and move along with directory specific delete.
:p.
:hp6.Filter:ehp6. opens the filter dialog so you can restrict the files viewed to a certain mask (i.e. *.sys).
:p.
:hp6.Okay and Cancel:ehp6. Both close the dialog.
:p.
:hp6.Collect:ehp6. Copies the selected files to the collector.
:p.
:hp6.Dirs:ehp6. Opens the select directories dialog.
:p.
:hp6.Include subdirectories:ehp6. Includes files from all subdirectories in both directory trees for comparison.(this is a toggle which change the display automatically)
:p.
:hp6.Hide not selected:ehp6. Includes selected files only on the display.
The display is updated immediately when the button is clicked.
This is a 3 state checkbox.
If one or more files are deselected when the box is checked,
the check mark will change to half-tone to indicate that this.
Click the checkbox again to update the display.
:p.
See also :link reftype=hd res=99100.Selection:elink..
:p.
The general procedure for synchronizing two directories (assuming, of
course, that's what you're wanting to do) is to first select all files
that have no matching files in the other directory (click the
:hp1.One:ehp1. button on the left side of the dialog). Copy these files
to the other directory. Now deselect all files (click the
:hp1.All:ehp1. button on the right side of the dialog) and select all
newer files (click the :hp1.Newer:ehp1. button on the left side of the
dialog). Copy these files to the other directory. The directories
should now match exactly -- deselect all files and select identical
files (click the :hp1.Identical:ehp1. button), and all the files should
be selected (to confirm).
:p.
:hp2.Note&colon.:ehp2. You can use multiple combination of the select/deselect buttons however
they will select you all the files as if an "or" operation not as an "and" operation.
This means that clicking larger then clicking newer will select all the larger files and then select
all the newer files not just the newer files among the already selected files.
You can use DeMorgan's rules to get the effect of "and" operation.
DeMorgan's rule says A and B is the equivalent to not (not A or not B).
:p.
If you're looking directory matching features not included in FM/2, you
might find them in the programs available on Hobbes
http&colon.//hobbes.nmsu.edu/cgi-bin/h-search?key=directory+compare&amp.pushbutton=Search
You can easily setup FM/2 to use one of these programs automatically using the :link
reftype=hd res=98400.Compare page:elink. of the internal Settings notebook.

:h3 res=91550 name=PANEL_WALKEM2.Select two directories
:i1 id=aboutWalkem2.Select two directories
:p.
This dialog lets you pick two directories by "walking" through the
directory structure of your drives. See also :link reftype=hd
res=91500.Walk Directories:elink..
:p.
When the desired directories are displayed in the entry fields of
the dialog, click :hp1.Okay:ehp1. to exit. Click :hp1.Cancel:ehp1. to
exit without selecting directories.

