.***********************************************************************
.*
.* $Id$
.*
.* fm/2 help
.*
.* Copyright (c) 1993-98 M. Kimes
.* Copyright (c) 2006, 2007 Steven H.Levine
.*
.* 30 Dec 06 GKY Corrected statement re copy/move "toggle" in OS/2
.* 03 Mar 07 GKY Update that file systems other than HPFS support long names
.* 20 Mar 07 DG  Add description for Mommy make it stop
.* 13 Aug 07 SHL Tweak scanning page
.* 06 Jul 08 GKY Update delete/undelete to include the option of using the XWP trashcan
.* 20 Jul 08 JBS Ticket 114: Support user-selectable env. strings in Tree container.
.* 08 Mar 09 GKY Added option to turn alert and/or error beeps off
.* 12 Mar 09 SHL Update container search description
.* 13 Jul 09 SHL Rework scanning page help text
.* 13 Dec 09 GKY Fixed separate paramenters.
.* 13 Dec 09 GKY Updated Quick page "Default" to match current defaults; added Gregg's way
.*               option to Quick page. 
.* 04 Aug 12 GKY Changes to allow copy and move over readonly files with a warning dialog; also added a warning dialog
.*               for delete of readonly files
.* 09 Feb 14 GKY Fix separate parameters. Moved to general page renamed separate settings
.*               for apps.
.*
.***********************************************************************
.*
:h2 res=94600 name=PANEL_SETTINGS.Settings notebook
:i1 id=aboutSettings.Settings Notebook
FM/2's internal Settings notebook lets you fiddle with some of the
toggles and variables used to control how FM/2 behaves. This is called
up via the :hp1.:link reftype=hd res=92000.Config:elink.->Settings
notebook...:ehp1. pulldown menu item.
:p.
Some container types have their own individual settings, usually available
via a popup menu requested on container whitespace (or the Views pulldown
menu).
:p.
Note that settings changes generally don't take effect until you close
the notebook by clicking the &OkayButton. button. The
:hp1.Undo:ehp1. button will restore things to the way they were before
you started fiddling about. The :hp1.Quick:ehp1. page is a notable
exception.
:p.
See also&colon.
.br
:link reftype=hd res=99930.Director Container page:elink.
.br
:link reftype=hd res=97100.Directory Container views page:elink.
.br
:link reftype=hd res=97200.Directory Container sort page:elink.
.br
:link reftype=hd res=97300.Collector Container views page:elink.
.br
:link reftype=hd res=97400.Collector Container sort page:elink.
.br
:link reftype=hd res=92400.Drive Tree page:elink.
.br
:link reftype=hd res=99990.Drive Tree sort/views page:elink.
.br
:link reftype=hd res=99940.Archive page:elink.
.br
:link reftype=hd res=92200.Viewers page:elink.
.br
:link reftype=hd res=92250.Viewers2 page:elink.
.br
:link reftype=hd res=98400.Compare page:elink.
.br
:link reftype=hd res=97000.Monolithic FM/2 page:elink.
.br
:link reftype=hd res=99950.General page:elink.
.br
:link reftype=hd res=99960.Scanning page:elink.
.br
:link reftype=hd res=100070.Bubble help page:elink.
.br
:link reftype=hd res=99200.Quick Configurations page:elink.

:h3 res=99930 name=PANEL_DCPAGE.Directory Container page
:i1 id=aboutDCP.Directory Container page

:artwork name='bitmaps\dir.bmp' align=left.
:p.
This page contains controls that affect Directory Containers.
:p.
If :hp6.Unhilite after action:ehp6. is on, highlighted objects in the
container are unhighlighted after you perform some command on them.
:p.
Normally FM/2 updates the container as things change; for instance, if
you're deleting several files, the container is updated after each file
is deleted. If you uncheck :hp6.Immediate updates:ehp6., FM/2 updates
the container after an atomic action completes (i.e. after all files
moved by one operation have been moved). This speeds up processing
but allows the container to be temporarily "out of sync" with the real
state of affairs.
:p.
The :hp6.Look in directory:ehp6. toggle controls whether or not FM/2
looks inside an open Directory Container when deciding what default to
place in the Walk Directories dialog for a copy or move command invoked
via keyboard, menu or toolbar button. If this is checked, FM/2 looks at
the current item in the Directory Container and, if it's a directory
object, uses that path. If not checked, FM/2 uses the directory into
which the Directory Container is "looking."
:p.
The :hp6.Min dir on open:ehp6. toggle, if checked, causes FM/2 to
minimize a Directory Container window if you open another Directory
Container window from one of its objects. When you close the new
Directory Container window the old window is restored.
:p.
The :hp6.Selected Always:ehp6. toggle (NOT RECOMMENDED!) causes FM/2 to
use selected object(s) (rather than keying on the current object).
:p.
The :hp6.No extended container search:ehp6. toggle, if checked, prevents FM/2
from performing extended searching in containers -- instead, the
standard OS/2 method is used, where the cursor moves to the nearest
object starting with the depressed letter and the object is selected.
Note that you override the configured value of this feature by
pressing the &ShiftKey.key along with the letter.
Extended container searching allows you to enter multiple characters which
are searched for at the start of filenames. The record is made the current (cursored) object.
The leading backslash can be omitted from directory names where there is no ambiguity.
If the record matches all the letters typed exactly, it is also selected.
A pause of more than about two seconds in typing resets the search string. 
The Escape key clears the search string.
Backspace deletes the last character from the search string.
Space toggles the object selection unless it matches a space in the object name.
:hp1.Warning:ehp1.&colon. this doesn't work right on
some versions of Warp, where the OS/2 CM_SEARCHSTRING container
message's behavior is buggy. It's not dangerous, it just doesn't
produce the expected results.
:p.
The :hp6.Multiple select cnrs:ehp6. and :hp6.Extended select cnrs:ehp6.
checkboxes control the type of selection in some new containers (existing
containers are not affected). Collector, Directory Container and Archive
Containers created after these settings are changed will have the type of
selection desired (note:  unhighlighting both results in a single select
container -- probably not desirable). The default is Extended select.
:p.
The :hp6.Stay in Tree View:ehp6. switch, if checked, causes Directory
Containers to remain in Tree View (if already in Tree View, of course)
when you switch the Directory Container to look into a different
directory. Otherwise, FM/2 switches the Directory Container back to
the last used view before you put it into Tree View.

The :hp6.No folder menu:ehp6. switch, if checked, causes a :color fc=default bc=cyan.B2:color fc=default bc=default. click on the
folder icon to immediately load the previous directory in the Directory Container.
Otherwise it results in the folder menu appearing.

:h3 res=97100 name=PANEL_DCPAGEVIEWS.Directory Container views page
:i1 id=aboutDCPV.Directory Container views page

:artwork name='bitmaps\dir.bmp' align=left.
:p.
This settings page lets you set the type of view that will be used in
new Directory Container windows. Directory Containers that are already
open won't be affected.
:p.
:hp6.Icon:ehp6. In Icon view, the object's name appears below its icon.
:p.
:hp6.Name:ehp6. In Name view, the object's name appears beside its icon.
:p.
:hp6.Text:ehp6. Text view is the fastest view for a container to
maintain, but provides the least information on the objects it contains.
:p.
:hp6.Detail:ehp6. Details view shows a great deal of information on the
objects it contains, including file sizes, dates, and times, but it is
the slowest view for a container control to maintain.
:p.
:hp6.Mini Icons:ehp6. is a toggle controlling whether icons are shown
full size or in miniature in views that show icons.
:p.
The :hp6.Subject Column Settings:ehp6. section allows you to set the width of
of the subject column which can be quite wide if you have any long subjects
in a directory. The :hp6.Subject Width:ehp6. spin button will show 0 if the
:hp6.Subject Max Width:ehp6. button (which sets the column to the width of
the longest subject in the directory) is selected. In addition you can move the
subject column to the left side of the slide bar by selecting :hp6.Subject
Left:ehp6.

:p.
The :hp6.Field Titles:ehp6. group (analogous to the Views->Details Setup
submenu) allows you to control what is shown in a Details view. Each
possible field in the details view is shown. If the field is checked,
FM/2 will show it. If not, it won't.
:p.
The :hp6.Filter:ehp6. entry field shows the current default filter mask
for Directory Containers. Moving to this field will bring up a dialog
that lets you set a new filter. The filter will be used for subsequently
created Directory Containers.

:h3 res=97200 name=PANEL_DCPAGESORT.Directory Container sort page
:i1 id=aboutDCPS.Directory Container sort page

:artwork name='bitmaps\dir.bmp' align=left.
:p.
This page lets you set the type of sort for Directory Containers.
Directory Containers that are already open won't be affected immediately
(see :hp1.Resort:ehp1. context menu command), but will use the new sort
type the next time they're resorted or rescanned.
:p.
You can also tell FM/2 to always display directories ahead of or behind
files. Note that :hp1.Last access date:ehp1. and :hp1.Creation
date:ehp1. are not tracked on FAT file systems. The difference between
:hp1.Pathname:ehp1. and :hp1.Filename:ehp1. is only apparent in the
Collector. With the former, the entire pathname of the object is used
to sort. With the latter, only the filename portion is used to sort.
FM/2 maintains separate sort criteria for Drive Tree, Collector,
Directory Container and Archive Container windows. This page affects
only Directory Container sorting. The Collector has its own page; use a
context menu or View pulldown menu for other container types (like the
Drive Tree).
:p.
:hp7.Note&colon.:ehp7. This page controls how new Directory Containers
are created (already open containers are not affected), and the context
menu in Directory Containers affects only the Directory Container from
which the menu was invoked (new containers won't inherit the setting).

:h3 res=97300 name=PANEL_COLPAGEVIEWS.Collector views page
:i1 id=aboutCOLV.Collector views page

:artwork name='bitmaps\collect.bmp' align=left.
:p.
This settings page lets you set the type of view that will be used in
new Collector windows. If the Collector is already open it won't be
affected unless you close and reopen it.
:p.
:hp6.Icon:ehp6. In Icon view, the object's name appears below its icon.
:p.
:hp6.Name:ehp6. In Name view, the object's name appears beside its icon.
:p.
:hp6.Text:ehp6. Text view is the fastest view for a container to
maintain, but provides the least information on the objects it contains.
:p.
:hp6.Detail:ehp6. Details view shows a great deal of information on the
objects it contains, including file sizes, dates, and times, but it is
the slowest view for a container control to maintain.
:p.
:hp6.Mini Icons:ehp6. is a toggle controlling whether icons are shown
full size or in miniature in views that show icons.
:p.
The :hp6.Field Titles:ehp6. group (analogous to the Views->Details Setup
submenu) allows you to control what is shown in a Details view. Each
possible field in the details view is shown. If the field is checked,
FM/2 will show it. If not, it won't.
:p.
The :hp6.Filter:ehp6. entry field shows the current filter mask for the
Collector. Moving to this field will bring up a dialog that lets you set
a new filter. The filter will be used for subsequent invocations of the
Collector.

:h3 res=97400 name=PANEL_COLPAGESORT.Collector sort page
:i1 id=aboutCOLS.Collector sort page

:artwork name='bitmaps\collect.bmp' align=left.
:p.
This page lets you set the type of sort for the Collector. If the
Collector is already open it won't be affected immediately, (see
:hp1.Resort:ehp1. context menu command), but will use the new sort type
the next time it's resorted or rescanned.
:p.
You can also tell FM/2 to always display directories ahead of or behind
files. Note that :hp1.Last access date:ehp1. and :hp1.Creation
date:ehp1. are not tracked on FAT file systems. The difference between
:hp1.Pathname:ehp1. and :hp1.Filename:ehp1. is only apparent in the
Collector. With the former, the entire pathname of the object is used
to sort. With the latter, only the filename portion is used to sort.
FM/2 maintains separate sort criteria for Drive Tree, Collector,
Directory Container and Archive Container windows. This page affects
only :link reftype=hd res=90100.Collector:elink. sorting.

:h3 res=99940 name=PANEL_ARCPAGE.Archive page
:i1 id=aboutARCP.Archive page

:artwork name='bitmaps\archive.bmp' align=left.
:p.
This page contains controls relating to archivers and Archive
Containers.
:p.
The :hp6.Quick arc find:ehp6. toggle, if checked, causes FM/2 to check
only files with extensions that match those configured in ARCHIVER.BB2
as potential archives during a drag and drop operation. If you're
dragging over files on a floppy or network directory, this can speed
things up a bit.
:p.
The :hp6.Folder after extract:ehp6. toggle, if on, causes FM/2 to create
a folder to hold objects extracted from an archive via an Archive
Container. You'll be given an opportunity to abort the folder creation
or to decide where the folder will be located and what it'll be named.
:p.
You can set a :hp6.Default archiver:ehp6. so that, when you're archiving
files and directories, this archiver is the default for the :hp1.Select
Archiver:ehp1. dialog.
:p.
If the :hp6.Show archiver activity:ehp6. toggle is checked, FM/2 runs
archiver windows in the foreground. Normally it runs them in the
background, minimized, so you'd have to use the task list to pull them
to the foreground if you want to see them.
:p.
The :hp6.Use archive name as extract path in container:ehp6.allows
you to have FM/2 create an extract path based on the archive name.
The path is made as a subdirectory to the directory the archive
is in. If a directory is listed in Ext. Path it over rides this setting.
The setting only effects the arc container. The extract dialog has a
separate setting for this function.
:p.
The :hp6.A/Virus:ehp6. field allow you to enter the command line
(PATH/ANTIVIRUS.EXE parameters) for your antivirus software
This must be done to enable virus checking of archives from FM/2.
NVCC.EXE &percent.p /C is one possible command line option
if you are using Norman Antivirus.
:p.
The :hp6.Ext. Path:ehp6. field, if anything is in it, gives the default
directory in which to place extracted files. Otherwise, FM/2 takes a
wild, hairy guess about where you want the extracted files to go
(although it'll let you override manually). You can enter "*" as the
extract path and FM/2 will use the directory in which the archive
resides. The :link reftype=hd res=99970.Find:elink. button can be used
to find a directory and fill this field in automatically.


:h3 res=92400 name=PANEL_TREEPAGE.Tree page
:i1 id=aboutTREEP.Tree page

:artwork name='bitmaps\tree.bmp' align=left.
:p.
This page contains controls that affect the Drive Tree.
:p.
The :hp6.Follow Drive Tree:ehp6. toggle causes FM/2 to "follow" the
current selected directory in the Drive Tree (when you move the cursor
in the tree, the directory container changes to show the files in that
directory without you having to press [Enter] or double-click the
directory).
:p.
The :hp6.Double-click Opens:ehp6. toggle, if on, causes FM/2 to always
open a new Directory Container window when a Drive Tree directory is
double clicked. It's probably not useful to have both this toggle and
the Follow Drive Tree toggle on simultaneously.
:p.
The :hp6.Treetop on expand:ehp6. toggle, when on, causes a directory
that is being expanded in the Drive Tree to be moved to the top of the
container.
:p.
If you check :hp6.VTree->WPS folder:ehp6., when running VTree a
double-click on a directory will open a WPS folder instead of an
FM/2 Directory Container.
:p.
:hp6.Switch on focus change:ehp6. tells FM/2 to find and make current in
the Drive Tree the directory of a Directory Container when you give the
Directory Container the focus (make it the active window).
:p.
:hp6.Switch on directory change:ehp6. causes the Drive Tree to find and
make current the directory that a Directory Container has just changed
to look into.
:p.
:hp6.Collapse before switch:ehp6. makes the Drive Tree first collapse
all expanded branches in the tree before switching because of one of the
two above switch events.
This option will significantly slow down switching
if you a large number of directories in the Drive Tree.
:p.
:hp6.Expand curr. after switch:ehp6. causes the Drive Tree to expand
the branch of the directory to which it just switched because of one of
the two above switch events.
:p.
The :hp6.Show env. vars in Tree:ehp6. toggle determines whether FM/2
shows a few environment variable lists (like PATH, DPATH and LIBPATH)
in the Drive Tree. Use the entry field to specify a semicolon-separated
list of the desired environment variables. (e.g. PATH;LIBPATH;DPATH).
Note that the pseudo-variable LIBPATH is also supported.
:p.
:hp6.Show in tree beside drive letter (radiobuttons):ehp6. This
setting allows you to list either the file system type, the drive label or nothing 
beside the drive letter in the Drive Tree container. The status line at the bottom 
left lists the information you choose not to list in the tree in addition
to the drive's serial number and the amount of free space. When split status line
is selected on the :link reftype=hd res=97000."Monolithic":elink. notebook page, 
the total number of drives is moved to the end of the line and maybe truncated
depending on window size and screen resolution.The total number of drives 
preceeds the drive description for all drives if the status lines aren't split.

:h3 res=99990 name=PANEL_TREESORTPAGE.Tree sort/views page
:i1 id=aboutTREESP.Tree sort/views page

:artwork name='bitmaps\tree.bmp' align=left.
:p.
This page contains controls that affect Drive Tree sorting and view type.
:p.
The Drive Tree will be updated with these settings when you close the
Settings notebook.


:h3 res=92200 name=PANEL_VIEWPAGE.Viewers page
:i1 id=aboutVIEWP.Viewers page

:artwork name='bitmaps\viewer.bmp' align=left.
:p.
This page contains controls relating to viewers.
:p.
:hp6.Fast internal viewer:ehp6. causes FM/2 to use the faster (non-MLE)
internal viewer (unless you have an external viewing program defined).
Without this checked, FM/2 will use an internal MLE-based viewer/editor
which can be considerably slower, but does allow the option of editing
files. It is strongly suggested that you check this option and use an
external editor rather than the internal editor, since editors are
rather complex programs in their own right, and the internal version is
quite minimal.
:p.
The :hp6.Check for multimedia w/ MMPM/2:ehp6. toggle can be unchecked to
cause FM/2 to :hp1.not:ehp1. use the MMPM/2 interface to determine
whether or not a file to be viewed is a multimedia type. The reason for
this toggle is to avoid crashing FM/2 if you've installed a buggy MMPM/2
I/O procedure. Some desktop "enhancement" programs install such buggy
IOProcs. If you can't get it fixed, at least you can work around it
(sigh). You can manually put back some of this functionality using
FM/2's :link reftype=hd res=90400.Associations:elink. feature. If you have
installed Russel O'Connor's MP3 IOProcs you should uncheck this feature
because it results in many non-multimedia programs being identified as
multimedia and some multimedia not being identified as such. Use the
:link reftype=hd res=90400.Associations:elink. instead.
:p.
The :hp6.Guess view type:ehp6. toggle, if on, causes FM/2 to guess at the
type of the file being viewed (text or binary) and display it accordingly.
:p.
The :hp6.Run Viewer as child session:ehp6. toggle causes FM/2 to run the
:hp1.Viewer:ehp1. program (see below) as a child session. The main result
of this is that the Viewer will close when FM/2 closes.
:p.
You can fill in the names of programs to run to view WWW (web --
http&colon.//) or FTP (ftp&colon.//) internet components when they're
encountered in text in the internal viewers here. In the faster
(non-MLE) internal viewer, double-clicking the line containing the
component causes it to be viewed (you're given a choice of component if
there's more than one on the line). For WWW components, the prefacing
"http&colon.//" is included. For FTP components, the prefacing
"ftp&colon.//" is :hp1.not:ehp1. included. In the MLE-based internal
viewer/editor, you highlight the desired text and choose to view the
component from a context menu. The command line that you enter here is
automatically appended with a space and then the component descriptor
from the text.
:p.
FM/2 uses one of two internal viewers or an internal MLE-based editor if
you have no text viewer, binary viewer or text editor configured here.
It's recommended that you fill these fields in with whatever editors you
like rather than use the internals. The :link reftype=hd
res=99970.Find:elink. button can be used to find a program and fill
these fields in automatically (see :link reftype=hd
res=99970.examples:elink. at that topic). Don't forget that you can
cause FM/2 to automatically use many different types of viewing programs
based on the type of file using the :link reftype=hd
res=90400.Associations:elink. feature.

:h4 res=99970 name=PANEL_FIND.Find button
:i1 id=aboutFINDB.Find button

:artwork name='bitmaps\find.bmp' align=left.
:p.
The :hp1.Find:ehp1. button can be clicked to bring up a dialog that will
let you point-and-click on a file or directory name that will be
imported to the current entry field. For example, if you're in the
:hp6.Editor:ehp6. entry field and click Find, you'll get a standard OS/2
open dialog which you may use to find your editor executable.
:p.
Examples:
:p.
Editor:  "EPM.EXE %a"
.br
or       "AE.EXE %a"
.br
or       "VS.EXE %a"
.br
or       "CMD.EXE /C START /C /FS EMACS.EXE %a"
.br
Viewer:  "HVPM.EXE /K %a" (Hyperview PM, excellent viewer)
.br
or       "LSTPM.EXE %a"
.br
or       "LIST2.EXE %a"
.br
Compare: "COMP.COM %a"
.br
or       "CONTRAST.EXE %a"
.br
or       "GFC.EXE %a"
.br
or       "CMD.EXE /C MYCOMP.CMD %a"
.br
DirCmp:  "PMDMATCH.EXE" (PMDirMatch, excellent directory comparitor)
.br
Virus:   "OS2SCAN.EXE %p /SUB /A"
:p.
See also :link reftype=hd res=100075.Metastrings:elink..

:h3 res=92250 name=PANEL_VIEWPAGE2.Viewers2 page
:i1 id=aboutVIEWP2.Viewers2 page

:artwork name='bitmaps\viewer.bmp' align=left.
:p.
This page contains controls relating to the internal viewers web access
features.
:p.
:hp6.Use WPS default for run Http/Ftp:ehp6. If selected FM/2 will use the
program and work directory found in the OS2 INI key WPURLDEFAULTSETTINGS.
If these boxes are check they over ride the programs listed for Run
Ftp/Http below them.
:p.
:hp6.Use LIBPATHSTRICT for run Http/Ftp/Mail:ehp6. If selected FM/2 will
set LIBPATHSTRICT to true before running the associaed program. This is
particularly useful when running Mozilla based applications. Note you need
to fill in the working directory if not using the defaults for this to work
correctly.
:p.
:hp6.Don't use mailto wrapper:ehp6. If selected FM/2 will pass the bare email
address to the run mail application. This is needed if using startpmm for mailto
support in PMMail. www.hobbes.nmsu.edu/cgi-bin/h-search?key=startpmm&amp.pushbutton=Search
:p.
You can fill in the names of programs to run to view WWW (web --
http&colon.//), FTP (ftp&colon.//) or mail (@) internet components
when they're encountered in text in the internal viewers here. In the faster
(non-MLE) internal viewer, double-clicking the line containing the
component causes it to be viewed (you're given a choice of component if
there's more than one on the line). For WWW components, the prefacing
"http&colon.//" is included. For FTP components, the prefacing
"ftp&colon.//" is :hp1.not:ehp1. included. Mailto&colon. is prepended to the
the email address before being placed on the command line In the MLE-based internal
viewer/editor, you highlight the desired text and choose to view the
component from a context menu. The command line that you enter here is
automatically appended with a space and then the component descriptor
from the text.
:p.
You can fill in the name of a working directory for any or all of the "run" functions.
If a working directory is provided the program will be executed from that directory.

:h3 res=98400 name=PANEL_COMPPAGE.Compare page
:i1 id=aboutCOMPP.Compare page

:artwork name='bitmaps\linkdrag.bmp' align=left.
This page contains controls related to comparing objects.
:p.
The :hp6.Compare:ehp6. field gives a program that FM/2 will run when
you compare two files (or a file and a directory, which should compare
the file to a file of the same name in the directory, if possible).
:p.
If the :hp6.Dir Cmp:ehp6. field is filled in, FM/2 runs that when
directories are to be compared. FM/2 fills in the name of the two
directories after the text entered in this field -- no "%a" or other
metastring is required.
:p.
The :link reftype=hd res=99970.Find:elink. button can be used to find a
program and fill these fields in automatically (see examples at that
topic).


:h3 res=97000 name=PANEL_MONOPAGE.Monolithic FM/2 page
:i1 id=aboutMONOP.Monolithic FM/2 page

:artwork name='bitmaps\monolith.bmp' align=left.
:p.
This page contains controls that affect FM/2 when run as a monolithic
program (a Drive Tree and none or more Directory Containers contained
within a larger main window, including optional Toolbar, Quicklists,
etc.).
:p.
The :hp6.Viewer outside:ehp6., :hp6.INI viewer outside:ehp6.,
:hp6.Collector outside:ehp6. and :hp6.Arcboxes outside:ehp6. toggles, if
checked, cause FM/2 to open windows of the corresponding kinds outside
the main FM/2 monolithic frame window.
:p.
The :hp6.Quicklist switches:ehp6. toggle reverses the operation of the
user directory quicklist control -- if you click an item in the dropdown
list without holding down &ShiftKey., the last used Directory Container
switches, if you hold down &ShiftKey. while clicking, a new Directory
Container is opened.
:p.
The :hp6.Recent Dirs:ehp6. toggle is a 3-position toggle. If checked,
FM/2 places temporary entries for directories "visited" during a session
to the user directory quicklist and Walk Directories dialog. If grayed,
FM/2 also places temporary entries for any directory chosen using the
:link reftype=hd res=91500.Walk Directories:elink. dialog.
:p.
If you check the :hp6.Free Drive Tree:ehp6. toggle, FM/2 will allow you
to move the Drive Tree Container. Otherwise it "pins" it in the upper
left corner of the client window.
:p.
Normally FM/2 leaves a space below the Drive Tree just the right size for
a minimized window. Checking the :hp6.No space under Tree:ehp6. toggle
causes FM/2 to start without this space below the Drive Tree.
:p.
The :hp6.Save state of dir windows:ehp6. toggle, if checked, causes FM/2
to "remember" the directory windows that are open when you close FM/2 so
that it can open them again when you restart FM/2. If you turn this on,
be sure to play with the Free Tree and Autotile toggles under the Windows
menu to achieve the desired effect (everyone will want a different
effect). You'll probably want to omit any directories from the command
line if you turn on this toggle.
:p.
If you turn on :hp6.Autotile:ehp6. (it's on by default), FM/2 will
retile the windows in many cases to try to keep things neat.
:p.
If :hp6.Split Status:ehp6. is checked, you get two status lines at the
bottom of the FM/2 window instead of one. The left status line typically
contains information about the current container, the right about the
current object in the container. Cool.
:p.
The :hp6.Start minimized:ehp6. and :hp6.Start maximized:ehp6. toggles
cause FM/2 to assume the indicated state after startup.
:p.
The :hp6.Minimize to databar:ehp6. toggle, if on, causes FM/2 to
minimize to a small, bar-shaped window (the :link reftype=hd
res=99000.databar:elink.) showing some system information. The
databar can be run separately if desired -- look in the FM/2 folder.
:p.
The :hp6.Tile horizontal:ehp6. toggle, if on, causes FM/2 to favor
tiling windows so they're taller than wide. I suggest you think twice
before doing this; displaying files is basically a horizontal operation,
not vertical (at least in countries that read left to right or right to
left). In other words, using this toggle will cause windows to be
biased to display side-by-side rather than one above another, which
means that the horizontal reading space is limited, resulting in more
scrolling to see the information at the right side of the window in
Details view.
:p.
The :hp6.Animate:ehp6. toggle, when on, causes internal FM/2 windows to
be animated when they open and close (if you have animation turned on in
the WPS System object, of course). Silly.
:p.
The :hp6.Blue/yellow LEDs:ehp6. toggle, when checked, causes FM/2 to
display the thread LED as blue (off) and yellow (on) LEDs rather than
the default red (off) and green (on) LEDs. The change will not show up
on the application window until you close and then reopen FM/2. The LED
blinks during background processing indicating that the background process
is still active.
:p.
The :hp6.Show Target in Drivebar:ehp6. toggle, if on, tells FM/2 to
display the Target directory name, if any, at the right side of the
Drivebar (the background window that holds the Drive buttons -- see
the :link reftype=hd res=97600.Window layouts:elink. topic).

:h3 res=99950 name=PANEL_GENERALPAGE.General page
:i1 id=aboutGENERALP.General page

:artwork name='bitmaps\general.bmp' align=left.
:p.
This page contains controls that didn't seem to fit anywhere else.
:p.
:hp6.Confirm delete:ehp6. controls whether FM/2 will ask you to confirm
the deletion of files (deleting directories or hidden/system files
always requires confirmation). It is recommended that you leave this
option on; FM/2 will let you confirm all files on which you're acting
from one dialog, so it has minimal impact and provides a safety net.
:p.
:hp6.Warn if file is readonly:ehp6. If selected a warning dialog will
appear when you attempt to delete a readonly file or copy/move a file 
over a readony file. Otherwise the file will be deleted/overwritten which 
is what FM/2 always did for delete but not for copy/move which failed with 
an error message.
:p.
The :hp6.Separate settings for Apps:ehp6. toggle tells FM/2 whether to use
separate parameters for many of the mini-apps (like FM/2 lite, VDIR and VTREE). 
If you select it with FM/2 running it applies to all the mini-apps. However,
if you select it with a mini-app running it will only effect that app. If
checked, you'll have to set up the effected applications separately, but
configurations can be different than that in the monolithic FM/2
application itself. If you don't understand that, let's try this&colon.
if you run the Visual Tree program a lot, you'll probably want to check
this. Otherwise, you won't.
:p.
The :hp6.Verify disk writes:ehp6. toggle turns system-level write
verification on and off. This is like typing :link reftype=launch
object='CMD.EXE' data='/C HELP VERIFY'.VERIFY ON:elink. or VERIFY OFF at
a command line. Write verify can be turned off on a per drive basis
using the drive's drive flags. This feature was added since some USB
removables fail to work properly with write verify on.
:p.
The :hp6.Don't move my mouse!:ehp6. toggle keeps FM/2 from moving your
mouse (to place it in the center of a popup menu or over the &OkayButton. button
in some dialogs). Some people like the help, others don't. Take your
pick.
:p.
:hp6.Link Sets Icon:ehp6. changes the action of a link-drag. If this
toggle is set, a link drag causes FM/2 to try to set the icon of the
target to the icon of the first dropped object (if the first dropped
object has no .ICON EA and is not an icon file, the target's icon is
reset. Note that OS/2 sometimes buffers this info and an icon change
may not show up immediately). If not set, a link drag causes FM/2 to do
a compare of the target with the dropped objects.
:p.
If :hp6.Default action Copy:ehp6. is checked, FM/2's windows perform a
copy rather than move by default (note the highlighting on the mouse
pointer as your cue). Instead of pressing the &CtrlKey. key to change a drag
from a move to a copy, you'll need to press the &ShiftKey. key to change a
drag from a copy to a move (don't ask me why as this is simply the standard OS/2 behavior.)
I remind you that pressing :color fc=default bc=palegray.F1:color fc=default bc=default.
when you have a target in an FM/2 window
will display some help on what the drag command would have performed.
:p.
Someone didn't like the bar graphs on the drivespace controls of the
:link reftype=hd res=99000.databar:elink., so checking :hp6.Boring
databar:ehp6. will cause them to be displayed as drab old text, if you
prefer it that way.
:p.
FM/2 usually performs copy and move actions at the lowest "normal"
priority available. If you'd like FM/2 to use a true "idle" priority,
check the :hp6.Idle Copy:ehp6. toggle. Warning&colon. if a DOS program
is running, even in the background, idle priority threads slow
:hp2.way:ehp2. down. :hp3.Performance at true idle with DOS programs
running may not be acceptable.:ehp3.
:p.
The :hp6.Drag&amp.drop dialog:ehp6. toggle, if checked, causes FM/2 to
bring up a :link reftype=hd res=98700.dialog:elink. allowing you to
select the action a drag and drop should perform.
:p.
:hp6.Default delete perm.:ehp6. controls which type of deletion is the
default for the Delete context submenu. If this is checked, the default
is permanent (unrecoverable) deletion.
:p.
:hp6.Mouse pointer (3-state):ehp6. This button is a 3 state button (toggle)
which makes the mouse pointer operate in one of 3 ways. If unselected,
FM/2 will use a finger (the default). The "x" is a transparent pointer, and
the "=" is the operating system default pointer.
:p.
:hp6.Delete = move to trashcan:ehp6. controls what happens to files when
they are deleted. The default (not Checked) will keep the same behavior
FM/2 has always had. It will move the files to the OS/2 Deldir if it is
specified in config.sys. Otherwise it will simply delete the files
permanently. If this is selected and you have the trashcan from Xworkplace
or Eworkplace installed the files will be moved to the trashcan for possible
later restore. Move to trashcan is only active for local hard drive (this is 
a design limitation of the xworkplace trashcan). The result will be a permanent 
delete for all other drive types. Also be aware that deleted files are still 
retained on the drive they were deleted from. The result can be full drive type 
errors. If you are deleting to free up drive space you must either empty the 
trashscan or use :hp6.Permanent Delete:ehp6. which deletes the files directly 
bypassing the trashcan.
:p.
If the :hp6.Confirm target:ehp6. checkbox is checked, as it is by
default, FM/2 allows you to confirm the target directory to be used when
you elect to move or copy file system objects using menu or accelerator
key commands. Otherwise, the operation is performed without
intervention by you, moving or copying the objects to the :hp6.:link
reftype=hd res=100065.Target directory:elink.:ehp6. (if one is set, of
course; otherwise, you're prompted anyway).
:p.
If the :hp6.Alert beep off:ehp6. checkbox is checked, FM/2 will not beep to
alert the user that something (e.g. a file search using Seek and Scan) has
completed.
:p.
If the :hp6.No beep on error:ehp6. checkbox is checked, FM/2 will post an error
dialog but will not beep to alert the user that something has failed.
:p.
The :hp6.Set command line length:ehp6. spin button allow you to limit the
number of characters you can place in a "command" that will ultimately be
passed to your command shell. This setting is internal to FM/2 and does not
effect the actual limit of your command shell. The minimum is 299 which is
the CMD.EXE limit. The Default is 1023 which is the unexpanded limit for
4OS2.EXE. We believe that some UNIX based shells (eg BASH) have much
higher limits so the maximum is 32K. (The 1023 was also the hard
coded limit in many place in FM/2).

:h3 res=99960 name=PANEL_SCANPAGE.Scanning page
:i1 id=aboutSCANP.Scanning page

:artwork name='bitmaps\scan.bmp' align=left.
:p.
This page contains controls related to how FM/2 scans your drives.
:p.
:hp6.Uppercase names:ehp6. and :hp6.Lowercase names:ehp6. control how
FM/2 pretreats filenames before inserting them into the container. The
default is not to change the case of the filenames at all. Changing
these toggles will have an effect on the next rescan.
:p.
If :hp6.Load Subjects:ehp6. is checked, FM/2 loads object descriptions
from their standard WPS .SUBJECT EAs during scans. If you change the
state of this toggle, you'll need to rescan to get the change to show up
in FM/2's containers. Note that only the Details view shows Subjects.
Subjects may be direct-edited when showing in the container. You can
also pick :hp1.Subject:ehp1. from a context menu, whether Subjects are
being loaded during scans or not, to view and optionally change the
object description. You can turn this off to increase scanning speed.
You can also adjust this on a drive-by-drive basis from the command line
(see the :link reftype=launch object='E.EXE'
data='\FM2\README'.README:elink. that came with FM/2).
:p.
If :hp6.Load Longnames:ehp6. is checked, FM/2 loads the .LONGNAME
extended attribute for drives that don't have native support for long file names
(ie FAT). This attribute usually contains
a long name for objects that should be restored if the object is moved
to an HPFS/JFS drive. As for Subject, Longnames are only shown in the
Details view. You can turn this off to marginally increase scanning
speed. You can also adjust this on a drive-by-drive basis from the
command line (see the :link reftype=launch object='E.EXE'
data='\FM2\README'.README:elink. that came with FM/2).
:p.
If :hp6.Load file icons:ehp6. and/or :hp6.Load directory icons:ehp6. are
checked, FM/2 will load the icons of objects from the file system;
otherwise it uses defaults. Although turning these off can speed up
scanning, it makes for boring containers. This is PM, folks, enjoy the
bells and whistles! You can also adjust this on a drive-by-drive basis
from the command line (see the :link reftype=launch object='E.EXE'
data='\FM2\README'.README:elink. that came with FM/2) so that you can, for
instance, skip reading in icons from very slow drives (CD or floppy, for
example) and/or from disks containing only DOS programs (DOS programs
don't normally have special icons associated with them, anyway).
:p.
:hp6.Notes:ehp6.&colon. some very slow drives (like EZ and ZIP drives)
:hp1.will:ehp1. benefit from turning off some of the automatic
information gathering above. Experiment and adjust to taste. See also
the :link reftype=hd res=99980.Drive flags:elink. topic.
:p.
A few remote FSDs have bugs in their file finding functions, preventing
a find for more than one file at a time from working correctly. Symptoms
range from invalid data returned to trapping of the requesting
application. While you'd think that these bugs would be fixed (and they
are), new versions seem to reintroduce them again. Therefore, FM/2
provides the :hp6.Remote find bug:ehp6. toggle. If checked, FM/2 will
only ask for one file at a time from remote drives. This is slower, but
at least usable. In the meantime, ask you system administrator to
upgrade the LAN software -- most of these bugs have been fixed in newer
versions.
:p.
If you turn this toggle off (it's on by default) and FM/2 starts
behaving strangely, turn it back on. If FM/2 traps and you can't get
to the settings page, disconnect from the network, start FM/2, then
turn on the toggle. If that's not possible for some reason, use
FM/2's INI editor to view FM3.INI and delete the "RemoteBug" keyword
from the INI (which will cause FM/2 to revert to the default setting).
:p.
If the :hp6.Don't scan removables:ehp6. is checked, FM/2 won't attempt to
find subdirectories on removable drives until you double-click the drive
in the Drive Tree, so you won't see a [+] sign beside removable drives
even if they do contain subdirectories until you double-click the drive.
Floppy drives A&colon. and B&colon. receive this treatment without this
toggle; it's for other removable drives, like CD-ROM drives. This was
added to allow folks with CD-ROM carousels to avoid having each CD
loaded and scanned automatically at FM/2 startup.
:p.
The :hp6.Find count:ehp6. spin button controls how many files FM/2 searches
for in one system call. The higher this number, the faster FM/2 works
(with properly operating FSDs that support "finding" more than one file
at a time -- unfortunately, the FSDs that can benefit the most from this,
network FSDs, are the ones most often broken), but the more memory is
temporarily consumed as a container is filled. If you habitually work
with directories containing large numbers of files and have sufficient
memory, boosting this may be a good idea. On the other hand, if you are
extremely limited in memory (less than 16 megs), reducing this might be
the thing to do. The DosFind buffer limits the number of files it can find
at a time to between 1500-2000. The only advantage of a larger number is it
reduces the number of times the find stops to insert records into the container
The range is 256 to 4096, with 256 being the default.
:p.
The :hp6.Recurse scan at startup:ehp6. section allows you to choose the drive types
that will have a full recursive scan when the Drive Tree container is opened at startup.
This is the same
scan that occurs when you press the plus sign by the drive for the first time.
The advantage of enabling the scan is the tree expand is quicker.
The disadvantage is that startup is slower.
The installation default is to enable the scan only for local drives.
Slow drives and nonwrite drives are only
scanned if they match the drive type(s) you have selected (i.e. if you have a slow virtual drive
you must select both virtual drives and slow drives for it to be scanned on startup).
:p.
The :hp6.Rescan tree on media eject:ehp6. section allows you to choose the drive types
which trigger a rescan of the Drive Tree container when the drive media is ejected. This keeps
the container in better sync with reality.
The installation default enables the option for removables and disables it for
CD/DVDs and floppies.
:p.
:h3 res=100070 name=PANEL_BUBBLEPAGE.Bubble help page
:i1 id=aboutBUBBLEP.Bubble help page

:artwork name='bitmaps\bubble.bmp' align=left.
:p.
This page controls where FM/2 shows bubble help.
:p.
:hp6.Toolbar help:ehp6. determines whether FM/2 shows bubble help when
the mouse pointer passes over toolbar buttons.
:p.
:hp6.Drivebar help:ehp6. controls whether FM/2 shows drive freespace
when the mouse pointer passes over drivebar buttons. FM/2 will not show
freespace for floppy drives A&colon. or B&colon., for CD-ROM drives, or
for drives marked as Slow in the :link reftype=hd res=99980.Drive
Flags:elink. dialog.
:p.
:hp6.Other help:ehp6. enables all the other bubble help in FM/2.


:h3 res=99200 name=PANEL_QUICKCFGS.Quick configuration page
:i1 id=aboutQuickCfgs.Quick configuration page

:artwork name='bitmaps\flash.bmp' align=left.
:p.
This page allows you to select from a few pre-configured setups for FM/2
in general. You'll see this page automatically the first time you run
FM/2. Pick something that looks close to what you want without worrying
too much about it  -- you can always change it later. The idea is to
give you some quick choices about the general way you'd like FM/2 to
look and behave, and allow you to tweak specifics later as you get some
experience with the program. Please note that none of these do anything to 
strings (i.e. the viewer program paths, antivirus path, etc) you may have
already entered.
:p.
:hp6.Default:ehp6. restores FM/2 to its default state, for the most
part.
:p.
:hp6.Max user interface:ehp6. activates most of FM/2's bells and
whistles -- quicklists, toolbar, autoview window, etc.
:p.
:hp6.Min user interface:ehp6. turns off FM/2's toolbar, menus, etc.
You'll have to use the mouse and work from context menus and accelerator
keys alone, but you'll have the maximum amount of free space within the
main FM/2 window. :hp3.Hint&colon.:ehp3. The System Menu contains the
command to unhide the pulldown menu, if that's going too far for you.
:p.
:hp6.Max info, pretty:ehp6. puts FM/2's Directory Containers into their
maximum information state as attractively as possible.
:p.
:hp6.Max info, plain:ehp6. puts FM/2's Directory Containers into their
maximum information state without caring about how pretty it looks.
Consequently, more filenames fit in a container at once.
:p.
:hp6.Max filenames:ehp6. puts FM/2's Directory Containers into a state
that allows the most filenames per container. Rather unattractive.
:p.
:hp6.Max speed:ehp6. turns off some automatic information gathering to
make FM/2 faster, but displays get a bit "dumber."
:p.
:hp6.1.x emulation:ehp6. sets up FM/2 3.x to behave somewhat like FM/2
1.x, with two Directory containers one above the other.
:p.
:hp6.DOS-think:ehp6. sets up FM/2 to look something like an older
DOS file manager, with two Directory containers side-by-side.
:p.
:hp6.Windoze-think:ehp6. sets up FM/2 to look something like a
Windows file manager, with one Directory container.
:p.
:hp6.Hector's way:ehp6. sets FM/2 up the way the author likes it. Your
mileage may vary.
:p.
:hp6.Gregg's way:ehp6. sets FM/2 up the way the one of the current maintainers 
likes it. Your mileage may vary but it give you a starting point based on
a long time user's settings. I only use FM/2 (except when testing) so you 
may want to avoid this if you generally use the other miniapps (vtree, FM/2 lite, etc).
:p.
You might also want to take a look at the :link reftype=hd
res=100000."FM/2 Lite":elink. object in the FM/2 folder.
