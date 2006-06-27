:h2 res=94900 name=PANEL_COMPARE.Compare Directories
:i1 id=aboutCompare.Compare Directories

:artwork name='bitmaps\linkdrag.bmp' align=center.
:p.
The :hp1.Compare Directories:ehp1. dialog shows you a comparison
breakdown of two directories. You can highlight files here and
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
:hp6.Copy:ehp6. files from one directory to another. You can
double-click a file to view it, or request a context menu on it to
compare it to its counterpart in the other directory.
:p.
Several selection/deselection techniques are available via pushbuttons
to enable you to select files based on their comparison. The pushbuttons
operate on both containers. You can also select and deselect files in
the containers manually with the mouse and/or keyboard.
:hp2.Hint&colon.:ehp2. mouse selection works slightly differently when
you hold down the Ctrl key while clicking.
:p.
:hp2.Brief description of selection/deselection button options&colon.:ehp2.
:p.
:hp6.Same:ehp6. All items which match name and size exactly.
:p.
:hp6.Identical:ehp6. All items which match name, size and date exactly.
:p.
:hp6.Both:ehp6. All items which are present in both containers (only
name used as criteria).
:p.
:hp6.One:ehp6. All items which are present in only one of the
containers (only name used as criteria).
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
If you're looking directory matching features not included in FM/2, you
might find them in the programs available on Hobbes
http&colon.//hobbes.nmsu.edu/cgi-bin/h-search?key=directory+compare&amp.pushbutton=Search
You can easily setup FM/2 to use one of these programs automatically using the :link
reftype=hd res=98400.Compare page:elink. of the internal Settings notebook.

:h3 res=91550 name=PANEL_WALKEM2.Select two directories
:i1 id=aboutWalkem2.Select two directories

This dialog lets you pick two directories by "walking" through the
directory structure of your drives. See also :link reftype=hd
res=91500.Walk Directories:elink..
:p.
When the desired directories are displayed in the entry fields of
the dialog, click :hp1.Okay:ehp1. to exit. Click :hp1.Cancel:ehp1. to
exit without selecting directories.

