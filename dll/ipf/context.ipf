:h1 res=93700 name=PANEL_CONTEXT.Context menus
:i1 id=aboutContext.Context menus
:link reftype=hd res=97800.Context menus:elink. (also called popup
menus) are used extensively in FM/2, just as they are in the WPS.
:p.
Context menus are requested by placing the mouse pointer over a desired
object in a container (or over container whitespace) and clicking mouse
button two (B2, usually the right button).
:p.
Commands that affect the container as a whole are found in context menus
requested over whitespace (any empty area of the container).  Commands
that affect the objects within the container are requested over the
object of interest.  If an object is :link reftype=hd
res=98000.highlighted:elink. when a context menu is requested, the
commands will usually affect all highlighted objects; otherwise, any
commands will affect only the object over which the menu was requested
(you'll see visual feedback to this effect).
:p.
FM/2's :link reftype=hd res=93300.Files menu:elink. shows the same menu
that would be obtained if you requested a context menu over the current
object.  FM/2's :link reftype=hd res=93800.Views menu:elink. shows the
same menu that would be obtained if you requested a context menu over
the current window's whitespace.  :hp9.Note&colon.:ehp9.  FM/2 Lite's
pulldown menus are simplified, but the context menus have all the
commands available, so the Files and Views menus aren't exact matches
as in FM/2.
:p.
When a menu command leads to a dialog, the command name is followed by
dots (i.e. "Attributes...").  In cases where commands have :link
reftype=hd res=100005.accelerator key equivalents:elink., the :link
reftype=hd res=97700.accelerator key:elink. is listed after the command
(i.e. "Info...    Ctrl + i").
:p.
:hp2.See also&colon.:ehp2.
.br
:link reftype=hd res=93705.Context menus affecting objects:elink.
.br
:link reftype=hd res=93710.Context menus affecting containers:elink.
.br
:link reftype=hd res=93715.Context menus affecting Drive Tree container:elink.

:h2 res=93705 name=PANEL_CONTEXTOBJECT.Context menus affecting objects
:i1 id=aboutContextObject.Context menus affecting objects

(Note that not all commands are available for all objects on all drives;
CD-ROM drives obviously wouldn't allow Delete and Move commands, for
instance, since they're read-only drives.)
:p.
These context menus are shadows of the :link reftype=hd
res=93300.Files:elink. pulldown menu.
:artwork name='..\..\bitmaps\view.bmp' align=center.
The :hp6.View:ehp6. conditional cascade submenu allows you to view the
current object.  Since this is a conditional cascade submenu, you can
click on the button to get a list of choices or click elsewhere to get
a default viewing action (noted below).
:p.
:hp6.Autoview:ehp6. views objects through the :link reftype=hd
res=92200.configured viewer:elink..  This is the default for the
Files->View conditional cascade submenu when a file is the current
object.  When you select Autoview, FM/2 guesses whether the file is text
or binary data and views it accordingly.
:p.
:hp6.as text:ehp6. causes FM/2 to view the current file object as text,
using the :link reftype=hd res=97000.configured text viewer:elink. or
the internal if none is configured.
:p.
:hp6.as binary:ehp6. causes FM/2 to view the current file object as
binary data, using the :link reftype=hd res=97000.configured binary
viewer:elink. or the internal if none is configured.  Binary data is
usually viewed as a :link reftype=hd res=98800.hex dump:elink..
:artwork name='..\..\bitmaps\info.bmp' align=center.
:hp6.Information:ehp6. brings up a dialog telling you everything you
ever wanted to know about file system objects but were afraid to ask.
If you select this from a drive object (root directory) in the Drive
Tree container, you get a ":link reftype=hd res=90900.drive
information:elink." dialog that lets you change the drive's label.
Otherwise, you get a :link reftype=hd res=95700.comprehensive
dialog:elink. that, besides showing you all the pertinent information
about the object, shows the object's icon. That icon is important.  You
can get a context menu on it, and you can drag other objects onto it to
change the object's icon.  If the object is an icon or pointer file
(*.ICO or *.PTR) you can use it to :link reftype=hd res=95500.change a
system pointer:elink. (for instance, you could change OS/2's usual arrow
pointer to a finger).  You can draw pointers yourself using OS/2's
ICONEDIT program, or find them pre-drawn in several collections of icons
and pointers freely available on BBSes.  This is the default command for
the Files->View conditional cascade submenu when the current object is a
directory.
:artwork name='..\..\bitmaps\playmm.bmp' align=center.
:hp6.Multimedia:ehp6. attempts to play the objects via MMPM/2 using
FM2PLAY.EXE (from the FM/2 Utilities collection, distributed separately
in FM2UTILS.ZIP).  Obviously, the objects must be multimedia objects
(sound files, movies, midi files, etc.) for this to have the desired
effect, and MMPM/2 must be installed in your system.  Note:  for this
command to work properly with large numbers of files (where the length
of the resultant command line would exceed the OS/2 command line length
limit of 1000 characters), you must have a version of FM2PLAY.EXE that
supports the /# command line switch. To test your version of
FM2PLAY.EXE, run FM2PLAY /? and see if /# is listed as a supported
switch.  If not, upgrade to a new version of the FM/2 Utilities.
:p.
:hp6.Update objects:ehp6. updates objects by refreshing the information
FM/2 has on them from disk to make sure it's current (an alternative to
rescan for special situations).
:p.
:hp6.Hide objects:ehp6. hides objects (removes them from view in the
container) until you rescan, use the :link reftype=hd res=93400.Filter
dialog:elink. or switch directories.
:p.
:artwork name='..\..\bitmaps\edit.bmp' align=center.
The :hp6.Edit:ehp6. conditional cascade submenu allows you to edit the
current object.  Since this is a conditional cascade submenu, you can
click on the button to get a list of choices or click elsewhere to get a
default editing action (noted below).
:p.
:hp6.Autoedit:ehp6. edits objects through the :link reftype=hd
res=92200.configured editor:elink..  This is the default for the
Files->Edit conditional cascade submenu when a file is the current
object.  When you select Autoedit, FM/2 guesses whether the file is text
or binary data and edits it accordingly.
:p.
:hp6.as text:ehp6. causes FM/2 to edit the current file object as text,
using the :link reftype=hd res=97000.configured text editor:elink. or
the internal if none is configured.
:p.
:hp6.as binary:ehp6. causes FM/2 to edit the current file object as
binary data, using the :link reftype=hd res=97000.configured binary
editor:elink..  Binary data is usually edited as a :link reftype=hd
res=98800.hex dump:elink..  No default binary editor is provided at
this time, but that may change.
:artwork name='..\..\bitmaps\ea.bmp' align=center.
:hp6.:link reftype=hd res=95000.Extended Attributes:elink.:ehp6. allows
you to view an object's extended attributes (EAs) and to edit and add
text attributes.
:artwork name='..\..\bitmaps\attrlist.bmp' align=center.
:hp6.:link reftype=hd res=95900.Attributes:elink.:ehp6. leads to a
dialog that sets objects' attributes and (optionally) date/time. This is
the default for the Files->Edit conditional cascade submenu when a
directory is the current object.
:p.
:hp6.Subject:ehp6. allows you to give an object a description.  This
makes use of the same EA (.SUBJECT) that the WPS uses for object
descriptions -- you can see and edit it on the File page of an object's
Settings notebook.
:artwork name='..\..\bitmaps\rename.bmp' align=center.
:hp6.:link reftype=hd res=91400.Rename:elink.:ehp6. allows you to rename
objects.  You are notified of conflicts as they occur.  An easier method
for renaming one object is to point at its text, hold down the ALT key,
and click mouse button one; however, this command allows you to use
wildcards when renaming if you desire.
:artwork name='..\..\bitmaps\delete.bmp' align=center.
:hp6.Delete:ehp6. deletes objects.  If the :link reftype=hd
res=92400.Confirm Delete:elink. toggle is on or one or more directories
are among the selected objects, you get a dialog showing the selected
objects and asking you to confirm that you really meant what you said.
In that dialog you have a chance to remove some of the objects.  If you
have Undelete enabled for the drive on which the objects reside, they
may be recoverable.
:artwork name='..\..\bitmaps\permdel.bmp' align=center.
:hp6.Permanent Delete:ehp6. deletes objects as above, but they will not
be recoverable (which may make the deletion faster).  It should be noted
that when deleting directory objects, the file objects within the
directory can never be recovered, but deleting all the file objects
inside a directory (rather than the directory itself) allows things to
be recovered :hp1.if:ehp1. you use the Delete command above rather than
this Permanent Delete command :hp1.and:ehp1. have Undelete enabled (type
:link reftype=launch object='CMD.EXE' data='/C HELP UNDELETE'.HELP
UNDELETE:elink. at a command line for more information on enabling
Undelete).
:artwork name='..\..\bitmaps\print.bmp' align=center.
:link reftype=hd res=99985.:hp6.Print:ehp6.:elink. prints text files.
It'd be a good idea to have a printer to which to print, and have
configured it first, before trying to use this.  If using the standard
WPS, be sure you have a printer object configured for the device you've
told FM/2 to use, or you may wind up with FM/2's printing thread blocked
for eternity awaiting access to a nonexistent or inaccessible device.
Actually, if using the standard WPS, it's recommended that you simply
drag files to the printer object and drop them instead of using this
command.
:artwork name='..\..\bitmaps\mover.bmp' align=center.
:artwork name='..\..\bitmaps\copier.bmp' align=center.
:hp6.Move and Copy:ehp6. move or copy objects.  :hp2.:link reftype=hd
res=90000.Drag and drop:elink. is recommended over using the menu
commands for moving and copying.:ehp2. When using the menu commands, the
:link reftype=hd res=91500.Walk Directories:elink. dialog appears to allow
you to select a target directory.
:p.
:hp6.Copy and rename:ehp6. and :hp6.Move and rename:ehp6. allow you to
change the names of the destination files as you copy and move using
the standard rename dialog.  You can also copy or move with wildcarding,
like you can from the command line (COPY thisfile.txt *.bak) by using
wildcards in the filename portion of the destination.
:p.
:hp6.Copy and preserve:ehp6. and :hp6.Move and preserve:ehp6. are only
available in the Collector and See all files windows.  These commands
copy or move the selected files but preserve the directory relationship
of the files.  The effect of this can be non-obvious, so use with care.
:p.
Let's say you select three files&colon. G&colon.\FOO\BAR\DUDE,
G&colon.\FOO\BAR\WOW\DUDE and G&colon.\FOO\BAR\RUFF\DUDE.  If you select
:hp1.Copy and preserve:ehp1. and pick a destination directory of
H&colon.\HERE, the resultant files will be H&colon.\HERE\DUDE,
H&colon.\HERE\WOW\DUDE and H&colon.\HERE\RUFF\DUDE.
:p.
Note that drives are not considered when preserving directory
relationships, so if one of our three files above resided on drive
F&colon., the results would be the same.
:p.
:hp6.:link reftype=hd res=99995.Merge:elink.:ehp6. lets you merge
several files together into a single file (you get to set the order of
the files to be merged and the name of the file to which they're
merged).
:p.
:hp6.WPS Copy:ehp6. and :hp6.WPS Move:ehp6. work like their standard
counterparts, except that WPS techniques are used.  There is more
overhead using this method, so only use it when you need it -- for
example, when moving a directory containing a program suite to
maintain the link between program objects and the program executables
in the directory.

:artwork name='..\..\bitmaps\shadow.bmp' align=center.
:artwork name='..\..\bitmaps\object.bmp' align=center.
:hp6.:link reftype=hd res=93600.Shadow:elink.:ehp6. builds WPS shadow
objects on your desktop (or other selected folder) for selected
object(s).  You can also create :hp6.Real Objects:ehp6. (except for
directories, for which you can only create shadow objects).  Both these
options (where applicable) are in a conditional cascade submenu called
:hp6.Create Objects:ehp6., with Shadows as the default command.
:artwork name='..\..\bitmaps\opend.bmp' align=center.
The :hp6.Open:ehp6. conditional cascade submenu allows you to open an
object's Settings notebook, open directories as WPS Folders, or open a
new FM/2 Directory Container window for directories (the default for
directories).  Note that when WPS Folders are opened, they come up in
the background.  This is an OS/2 bug, and IBM has been notified. Opening
a file's Default view will honor any OS/2 associations that you have
setup.  Remember that F6 or Ctrl + double-click opens an object's
default WPS view, and Ctrl + Shift + double-click opens an object's
WPS Settings notebook.
:artwork name='..\..\bitmaps\archive.bmp' align=center.
:hp6.:link reftype=hd res=90300.Archive:elink.:ehp6. allows you to build
an archive containing the selected object(s).
:artwork name='..\..\bitmaps\extract.bmp' align=center.
:hp6.:link reftype=hd res=91000.Extract:elink.:ehp6. allows you to
extract files from selected archives.
:p.
:hp6.UUDecode:ehp6. decodes files that were encoded with UUEncode, a
common protocol on the Internet.  Files created by UUDecoding are
appended if they already exist.
:artwork name='..\..\bitmaps\saveclip.bmp' align=center.
:hp6.Save to clipboard:ehp6. allows you to save selected objects to the
clipboard as a text list, one per line.  This is a good way to transfer
selections of files to other programs; for instance, you might copy a
list of files to the clipboard and feed it to a terminal program to send
the files over a modem or network.
:artwork name='..\..\bitmaps\savelist.bmp' align=center.
:hp6.:link reftype=hd res=96000.Save to list file:elink.:ehp6. lets you
save selected objects as a list to a text file.  Lists can include file
sizes, subjects, etc.
:p.
:artwork name='..\..\bitmaps\collect.bmp' align=center.
:hp6.Collect File(s):ehp6. calls up the :link reftype=hd
res=90100.Collector:elink. and places the selected files and directories
into it.  You can also open the Collector and drag things into it.
:p.
:hp6.Collect List in file(s):ehp6. collects the files listed inside the
selected files (see :hp6.Save to list file:ehp6. above).  The filename
should be the first item on each line of the list.  If spaces are
contained in the filenames, enclose the filenames in "quote marks."
Filenames must be full pathnames (d&colon.\path\filename).  Directories as
well as files can be Collected.
:p.
:hp6.Quick Tree:ehp6. appears in Directory Containers.  You can use this
to quickly select a subdirectory into which to switch the Directory
Container.  Obviously, if there are no subdirectories to select from,
FM/2 will ignore this command except to tell you.

:h2 res=93710 name=PANEL_CONTEXTCNR.Context menus affecting containers
:i1 id=aboutContextCnr.Context menus affecting containers

These context menus are shadows of the :link reftype=hd
res=93800.Views:elink. pulldown menu.
:p.
:hp6.Icon:ehp6. switches the container to Icon view; the object's name
appears below its icon.
:p.
:hp6.Name:ehp6. switches the container to Name view; the object's name
appears beside its icon.
:p.
:hp6.Text:ehp6. switches the container to Text view.  Text view is the
fastest view for a container to maintain, but provides the least
information on the objects it contains.
:p.
:hp6.Details:ehp6. switches the container to Details view.  Details view
shows a great deal of information on the objects it contains, including
file sizes, dates, and times, but it is the slowest view for a container
to maintain.
:p.
:hp6.Mini Icons:ehp6. is a toggle controlling whether icons are shown
full size or in miniature in views that show icons.
:p.
The :hp6.Details Setup:ehp6. submenu allows you to control what is shown
in a Details view.  Each possible field in the details view for the drive
type is shown.  If the field is checked, FM/2 will show it.  If not, it
won't.  Set the current view to Details view and you can see the changes
as they occur.  Note that for Directory Containers, the context menu
items change the current container only.  Use the internal :link
reftype=hd res=94600.Settings notebook:elink. to change the default for
subsequently created containers.

:artwork name='..\..\bitmaps\rescan.bmp' align=center.
:hp6.Rescan:ehp6. rescans the directory associated with a container, or
the drive from the current object down in a tree container.  FM/2 tries
very hard to keep all its windows up to date, but things outside FM/2
can cause changes that FM/2 cannot know about automatically.  This
command will ensure that your display is current.
:p.
The :hp6.Sort submenu:ehp6. allows you to control how objects are sorted
based on several criteria.  You can also tell FM/2 to always display
directories ahead of or behind files.  Note that :hp1.Last access
date:ehp1. and :hp1.Creation date:ehp1. are only meaningful for HPFS
file systems; FAT file systems do not track this information.  The
difference between :hp1.Pathname:ehp1. and :hp1.Filename:ehp1. is only
apparent in the Collector.  With the former, the entire pathname of the
object is used to sort.  With the latter, only the filename portion is
used to sort.  FM/2 maintains separate sort criteria for Drive Tree,
Collector, Directory Container and Archive Container windows.  See
:link reftype=hd res=97200.Directory Container sort page:elink. and
:link reftype=hd res=97400.Collector Container sort page:elink..  Note
that Ctrl + F7 will call up the Sort menu for a given container.  The
internal :link reftype=hd res=94600.Settings notebook:elink. can be
used to set the default for subsequently opened Directory Containers --
the context menu item sets the sort for _this_ container only.
:p.
:hp6.Resort:ehp6. resorts items in a container.  If you have more than
one Directory or Archive Container window open, selecting a new sort
type only causes the container in which you requested the context menu
to resort itself (although the change will affect all future rescans,
resorts and insertions in that type of container).  This command lets
you resort a container so that the new sort type is reflected in the
display.
:artwork name='..\..\bitmaps\filter.bmp' align=center.
:hp6.:link reftype=hd res=93400.Filter:elink.:ehp6. leads to a dialog
that lets you set filemasks and attributes for objects to include in the
container's display.  Note:  For Directory Containers, this sets the
default for _this_ container. Use the internal :link reftype=hd
res=94600.Settings notebook:elink. to change the defaults for
subsequently created containers.
:p.
:hp6.Parent:ehp6. moves directory containers to the previous (parent)
directory.
:p.
:hp6.Previous Directory:ehp6. returns the container to the last
directory. This is sort of like a one-step "undo" when you switch
a container to look at a different directory.
:artwork name='..\..\bitmaps\walk.bmp' align=center.
:hp6.:link reftype=hd res=91500.Walk Directories:elink.:ehp6. leads to a
dialog that lets you walk through your directory structures, or recall
user-defined directories.
:p.
:hp6.Show all files:ehp6. is a command available on directory objects
and in the container menu of Directory Containers.  It invokes the
:link reftype=hd res=98500.See all files:elink. window and shows all
the files in the directory and all its subdirectories.
:artwork name='..\..\bitmaps\select.bmp' align=center.
:artwork name='..\..\bitmaps\deselect.bmp' align=center.
The :hp6.:link reftype=hd res=99100.Select:elink.:ehp6. submenu gives
you many ways to highlight and unhighlight objects in a container.  This
lets you quickly build sophisticated selection sets of objects upon
which you can perform tasks.
:p.
You can select (or deselect) all files, all directories or all objects.
You can also select (or deselect) everything matching a mask string
which can contain wildcards, select (or deselect) filenames stored in
the clipboard or a list file and invert the current selections
(highlight what isn't, unhighlight what is).
:p.
Finally, FM/2 offers a full set of :hp6.Compare Selection:ehp6. tools
that let you select and deselect files based on how they compare to
unfiltered files in all other open Directory Containers (available only
in Directory Containers).  To give you an idea how this might be
helpful, imagine that you just hit the [Enter] key in the middle of
typing a copy command, when you were reaching for the backslash key to
complete a path. Before you realize what's happening and can hit Ctrl-C,
you copied fifty files from a data directory to the root directory of
your boot drive (you shoulda used FM/2! &colon.-).  Now you want to get rid of
them, but you don't want to pick each one.  Open the data directory and
the root, choose "Select if in all," then delete the selected files in
the root directory.  You're done.
:p.
These powerful selection tools are where a file manager really outshines
command line file management, so be sure to take a look at them.  Note
that Ctrl + F8 will call up the Select menu for a given container.

:h2 res=93715 name=PANEL_CONTEXTTREE.Context menus affecting Drive Tree container
:i1 id=aboutContextTree.Context menus affecting Drive Tree container

:hp6.Expand:ehp6. expands the tree from the point where the context menu
was requested to the bottom of the branch.  This isn't the same as clicking
the [+] symbol as it expands :hp1.all:ehp1. branches.
:p.
:hp6.Collapse:ehp6. collapses the tree from the point where the context
menu was requested to the bottom of the branch.  This isn't the same as
clicking the [-] symbol as it collapses :hp1.all:ehp1. branches.
:p.
:hp6.Optimize:ehp6. runs a .CMD file with the name <Filesystem>OPT.CMD,
giving the drive to optimize as a command line argument.  Therefore, for
a FAT drive C&colon. "FATOPT.CMD C&colon." would be run (through the
command interpreter defined in %COMSPEC% or CMD.EXE if none is defined)
and for an HPFS drive D&colon. "HPFSOPT.CMD D&colon.".  CMD files are
supplied with FM/2 that call utility programs from FM2UTILS.ZIP (a
separate collection of free-for-the-using utilities, sometimes named
FM2UTL.ZIP).  You may modify these command files as required for your
system, even to call other programs than those supplied.  Always be sure
to check the disk before trying to optimize it, and (in the case of the
FAT optimizer) it's a good idea to back up first.  You shouldn't run the
FAT optimizer on compressed drives -- use the utilities that came with
your compression program instead.
:p.
:hp6.Check Disk:ehp6. runs PMCHKDSK.EXE on the selected drive.  This
tests the drive and can correct some deficiencies.  This is available
only in context menus requested on drives (root directories).  Note
that OS/2 cannot correct defects on disks that are in use by the
system or programs (including FM/2).
:p.
:hp6.Format Disk:ehp6. runs PMFORMAT.EXE on the selected drive.
:hp8.Formatting a disk will destroy any information already on the
disk.:ehp8. This is available only in context menus requested on drives
(root directories).
:artwork name='..\..\bitmaps\mkdir.bmp' align=center.
:hp6.Make Directory:ehp6. allows you to create new directories.  The
name of the directory where you requested the context menu is filled in
as a starting point for convenience.  Directories may be created many
levels deep in one pass.
:p.
:hp6.:link reftype=hd res=95200.Sizes:elink.:ehp6. brings up a dialog
showing how many bytes are in the selected directory and its
subdirectories.
:p.
:hp6.Eject:ehp6. ejects removable media from drives (for instance,
opens the door of a CD ROM drive).
:p.
:hp6.Lock:ehp6. locks a removable drive.
:p.
:hp6.Unlock:ehp6. unlocks a removable drive.
:p.
:hp6.Partitions:ehp6. calls up :link reftype=launch object='CMD.EXE'
data='/C HELP FDISKPM'.FDISKPM.EXE:elink. to allow you to modify the
partitions on your hard drives.  :hp8.Extreme caution should be
exercised; read the help!:ehp8.
:p.
The :hp6.Drives:ehp6. submenu lets you select a root directory and the
Drive Tree will scroll to show that directory, and make it the current
object.

.im merge.ipf

:h2 res=96000 name=PANEL_SAVETOLIST.Save list to file
:i1 id=aboutSaveToList.Save list to file
:artwork name='..\..\bitmaps\savelist.bmp' align=center.

This command allows you to save the list of selected files to a disk file
(or to a printer; enter PRN for the file name to which to save the list).
:p.
The :hp1.Save as:ehp1. entry field contains the name of the file to
which the list will be saved.  The :hp1.Find:ehp1. button calls up a
standard OS/2 open dialog to let you point and click at a file.  If the
file exists, it will be appended.  :hp6.Hint&colon.:ehp6.  You can enter
:hp1.PRN:ehp1. as the filename to print the list.
:p.
The listbox below this contains patterns you've saved in the past (use
the :hp1.Add:ehp1. button to add the current pattern (the one in the
entry field), and the :hp1.Del:ehp1. button to remove the currently
highlighted pattern from the listbox).  You can select one of the
patterns in the listbox to avoid retyping it (as you highlight a pattern
in the listbox it'll appear in the entry field). The patterns are saved
in a file named PATTERNS.DAT, one per line.
:p.
The :hp1.Pattern:ehp1. entry field contains a pattern that will be used
to format the list.  Metastrings may be used to cause parts of a file
description to be written where desired (see below).  Also note that the
pattern is run through a :link reftype=hd res=99500.C-style escape
encoder:elink., so that \x1b would be interpreted as an ESCAPE
character, \r\n as a carriage return and linefeed "newline," and \\ is
required to get a single '\' character.
:p.
When everything's set as you want it, click :hp1.Okay:ehp1. to save the
list.  Click :hp1.Cancel:ehp1. if you change your mind.
:p.
Metastrings and their meanings (note&colon. these are different from those
used in command lines)&colon.
:parml compact tsize=6 break=none.
:pt.%s
:pd.subject (description)
:pt.%S
:pd.subject padded to 40 chars
:pt.%z
:pd.file size
:pt.%Z
:pd.file size padded to 13 chars
:pt.%e
:pd.EA size
:pt.%E
:pd.EA size padded to 5 chars
:pt.%d
:pd.last write date
:pt.%t
:pd.last write time
:pt.%l
:pd.longname
:pt.%L
:pd.longname padded to 40 chars
:pt.%f
:pd.filename (no path)
:pt.%F
:pd.filename (no path) padded to 13 chars
:pt.%p
:pd.full pathname
:pt.%P
:pd.directory only (no file)
:pt.%$
:pd.drive letter
:pt.%%
:pd.percent sign
:eparml.
:p.
Note that you can manipulate list files from REXX.  An EXAMPLE.CMD is
included in the FM/2 archive to show you how it's done.  REXX scripts
written in this manner can be effectively used as Commands.
.br

.im rename.ipf

.im filter.ipf

.im drvinfo.ipf

.im attribs.ipf

.im printer.ipf

.im shadow.ipf

.im archive.ipf

.im eas.ipf

.im dirsize.ipf

.im seticon.ipf

.im objcnr.ipf

.im select.ipf
