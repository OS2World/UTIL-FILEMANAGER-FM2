.***********************************************************************
.*
.* $Id$
.*
.* Context Menu Usage
.*
.* Copyright (c) 1993-98 M. Kimes
.* Copyright (c) 2006-07 Steven H.Levine
.*
.* 01 Sep 06 GKY ADD new partition disks information
.* 03 Mar 07 GKY Update that file systems other than HPFS support long names
.* 29 Feb 08 GKY Document unhide menu item
.* 06 Jul 08 GKY Update delete/undelete to include option of using the XWP trashcan
.*
.***********************************************************************
.*
:h1 res=93700 name=PANEL_CONTEXT.Context Menus
:i1 id=aboutContext.Context Menus
:link reftype=hd res=97800.Context menus:elink. (also called popup
menus) are used extensively in FM/2, just as they are in the WPS.
:p.
Context menus are requested by placing the mouse pointer over a desired
object in a container (or over container whitespace) and clicking mouse
button two (:color fc=default bc=cyan.B2:color fc=default bc=default., usually the right button).
:p.
Commands that affect the container as a whole are found in context menus
requested over whitespace (any empty area of the container). Commands
that affect the objects within the container are requested over the
object of interest. If an object is :link reftype=hd
res=98000.highlighted:elink. when a context menu is requested, the
commands will usually affect all highlighted objects; otherwise, any
commands will affect only the object over which the menu was requested
(you'll see visual feedback to this effect).
:p.
FM/2's :link reftype=hd res=93300.Files menu:elink. shows the same menu
that would be obtained if you requested a context menu over the current
object. FM/2's :link reftype=hd res=93800.Views menu:elink. shows the
same menu that would be obtained if you requested a context menu over
the current window's whitespace. :hp9.Note&colon.:ehp9. FM/2 Lite's
pulldown menus are simplified, but the context menus have all the
commands available, so the Files and Views menus aren't exact matches
as in FM/2.
:p.
When a menu command leads to a dialog, the command name is followed by
dots (i.e. "Attributes..."). In cases where commands have :link
reftype=hd res=100005.accelerator key equivalents:elink., the :link
reftype=hd res=97700.accelerator key:elink. is listed after the command
(i.e. "Info...   :color fc=default bc=palegray.Ctrl:color fc=default bc=default. +  :color fc=default bc=palegray.i:color fc=default bc=default.").
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

Commands are listed in the order they appear on the file object context
menu. Some commands apply to file, directory and drive objects. Others apply to
only one or two types of objects. Items not appearing on the file object menus
are inserted at the relative point they appear in the directory and/or drive menu.
Small icons appear with each entry to indicate which types of objects file :artwork runin name='\bitmaps\file.bmp'.
directory :artwork runin name='\bitmaps\fldr.bmp'. and/or drive :artwork runin name='\bitmaps\drive.bmp'. they are associated with.
(Note that not all commands are available for all objects on all drives;
CD-ROM drives obviously wouldn't allow Delete and Move commands, for
instance, since they're read-only drives. In this case the command(s) are grayed out)
:p.
These context menus are shadows of the :link reftype=hd
res=93300.Files:elink. pulldown menu.
:p.
:artwork name='..\..\bitmaps\rename.bmp' align=center.
:p.
:hp6.:link reftype=hd res=91400.Rename:elink.:ehp6. allows you to rename
objects. You are notified of conflicts as they occur. An easier method
for renaming one object is to point at its text, hold down the :color fc=default bc=palegray.Alt:color fc=default bc=default. key,
and click mouse button one; however, this command allows you to use
wildcards when renaming if you desire.
:artwork runin name='\bitmaps\file.bmp'. :artwork runin name='\bitmaps\fldr.bmp'.
:p.
:artwork name='..\..\bitmaps\copier.bmp' align=center. :artwork name='..\..\bitmaps\mover.bmp' align=center.
:p.
:hp6.Copy and Move:ehp6. copy or move objects. :hp2.:link reftype=hd
res=90000.Drag and drop:elink. is recommended over using the menu
commands for moving and copying.:ehp2. When using the menu commands, the
:link reftype=hd res=91500.Walk Directories:elink. dialog appears to allow
you to select a target directory.
:artwork runin name='\bitmaps\file.bmp'. :artwork runin name='\bitmaps\fldr.bmp'.
:p.
:hp6.Copy and rename:ehp6. and :hp6.Move and rename:ehp6. allow you to
change the names of the destination files as you copy and move using
the standard rename dialog. You can also copy or move with wildcarding,
like you can from the command line (COPY thisfile.txt *.bak) by using
wildcards in the filename portion of the destination.
:artwork runin name='\bitmaps\file.bmp'. :artwork runin name='\bitmaps\fldr.bmp'.
:p.
:hp6.Copy and preserve:ehp6. and :hp6.Move and preserve:ehp6. are only
available in the Collector and See all files windows. These commands
copy or move the selected files but preserve the directory relationship
of the files. The effect of this can be non-obvious, so use with care.
:artwork runin name='\bitmaps\file.bmp'. :artwork runin name='\bitmaps\fldr.bmp'.
:p.
Let's say you select three files&colon. G&colon.\FOO\BAR\DUDE,
G&colon.\FOO\BAR\WOW\DUDE and G&colon.\FOO\BAR\RUFF\DUDE. If you select
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
merged). :artwork runin name='\bitmaps\file.bmp'. :artwork runin name='\bitmaps\fldr.bmp'.
:p.
:hp6.WPS Copy:ehp6. and :hp6.WPS Move:ehp6. work like their standard
counterparts, except that WPS techniques are used. There is more
overhead using this method, so only use it when you need it -- for
example, when moving a directory containing a program suite to
maintain the link between program objects and the program executables
in the directory. :artwork runin name='\bitmaps\file.bmp'. :artwork runin name='\bitmaps\fldr.bmp'.
:p.
:artwork name='..\..\bitmaps\view.bmp' align=center.
:p.
The :hp6.View:ehp6. conditional cascade submenu allows you to view the
current object. Since this is a conditional cascade submenu, you can
click on the button to get a list of choices or click elsewhere to get
a default viewing action (noted below).
:artwork runin name='\bitmaps\file.bmp'. :artwork runin name='\bitmaps\fldr.bmp'. :artwork runin name='\bitmaps\drive.bmp'.
:p.
:hp6.Autoview:ehp6. views objects through the :link reftype=hd
res=92200.configured viewer:elink.. This is the default for the
Files->View conditional cascade submenu when a file is the current
object. When you select Autoview, FM/2 guesses whether the file is text
or binary data and views it accordingly.
:artwork runin name='\bitmaps\file.bmp'.
:p.
:hp6.as text:ehp6. causes FM/2 to view the current file object as text,
using the :link reftype=hd res=97000.configured text viewer:elink. or
the internal if none is configured.
:artwork runin name='\bitmaps\file.bmp'.
:p.
:hp6.as binary:ehp6. causes FM/2 to view the current file object as
binary data, using the :link reftype=hd res=97000.configured binary
viewer:elink. or the internal if none is configured. Binary data is
usually viewed as a :link reftype=hd res=98800.hex dump:elink..
:artwork runin name='\bitmaps\file.bmp'.
:hp6.as archive:ehp6. causes FM/2 to open the file in the :link reftype=hd
res=90200.Archive Container:elink. :artwork runin name='\bitmaps\file.bmp'.
(only works for archive file types where the underlying program (i.e. Infozip for .zip files)
is defined in archiver.bb2 and the program is in the system path.)
:artwork runin name='\bitmaps\file.bmp'.
:p.
:artwork name='..\..\bitmaps\info.bmp' align=center.
:p.
:hp6.Information:ehp6. brings up a dialog telling you everything you
ever wanted to know about file system objects but were afraid to ask.
If you select this from a drive object (root directory) in the Drive
Tree container, you get a ":link reftype=hd res=90900.drive
information:elink." dialog that lets you change the drive's label.
Otherwise, you get a :link reftype=hd res=95700.comprehensive
dialog:elink. that, besides showing you all the pertinent information
about the object, shows the object's icon. That icon is important. You
can get a context menu on it, and you can drag other objects onto it to
change the object's icon. If the object is an icon or pointer file
(*.ICO or *.PTR) you can use it to :link reftype=hd res=95500.change a
system pointer:elink. (for instance, you could change OS/2's usual arrow
pointer to a finger). You can draw pointers yourself using OS/2's
ICONEDIT program, or find them pre-drawn in several collections of icons
and pointers freely available on the internet. This is the default command for
the Files->View conditional cascade submenu when the current object is a
directory. (Note: Info is the first item on the drives menu)
:artwork runin name='\bitmaps\file.bmp'. :artwork runin name='\bitmaps\drive.bmp'.
:p.
:hp6.Quick Tree:ehp6. appears in Directory Containers. You can use this
to quickly select a subdirectory into which to switch the Directory
Container. Obviously, if there are no subdirectories to select from,
FM/2 will ignore this command except to tell you.
:artwork runin name='\bitmaps\fldr.bmp'.
:p.
:hp6.Update objects:ehp6. updates objects by refreshing the information
FM/2 has on them from disk to make sure it's current (an alternative to
rescan for special situations).
:artwork runin name='\bitmaps\file.bmp'. :artwork runin name='\bitmaps\fldr.bmp'. :artwork runin name='\bitmaps\drive.bmp'.
:p.
:hp6.Hide objects:ehp6. hides objects (removes them from view in the
container) until you rescan, use the :link reftype=hd res=93400.Filter
dialog:elink. or switch directories or use Unhide (see below).
:artwork runin name='\bitmaps\file.bmp'. :artwork runin name='\bitmaps\fldr.bmp'.
:p.
:hp6.Unhide objects:ehp6. unhides objects (restores them from view in the container)
Unhide doesn't unhide items that are filtered by the current mask or attribute filters
:artwork runin name='\bitmaps\file.bmp'. :artwork runin name='\bitmaps\fldr.bmp'.
:p.
:artwork name='..\..\bitmaps\playmm.bmp' align=center.
:p.
:hp6.Multimedia:ehp6. attempts to play the objects via MMPM/2 using
FM2PLAY.EXE (from the :link reftype=hd res=100090.FM/2 Utilities collection:elink., distributed separately
in FM2UTILS.ZIP). Obviously, the objects must be multimedia objects
(sound files, movies, midi files, etc.) for this to have the desired
effect, and MMPM/2 must be installed in your system. Note:  for this
command to work properly with large numbers of files (where the length
of the resultant command line would exceed the OS/2 command line length
limit of 1000 characters), you must have a version of FM2PLAY.EXE that
supports the /# command line switch. To test your version of
FM2PLAY.EXE, run FM2PLAY /? and see if /# is listed as a supported
switch. If not, upgrade to a new version of the FM/2 Utilities.

:artwork runin name='\bitmaps\file.bmp'.
:p.
:artwork name='..\..\bitmaps\edit.bmp' align=center.
:p.
The :hp6.Edit:ehp6. conditional cascade submenu allows you to edit the
current object. Since this is a conditional cascade submenu, you can
click on the button to get a list of choices or click elsewhere to get a
default editing action (noted below).
:artwork runin name='\bitmaps\file.bmp'. :artwork runin name='\bitmaps\fldr.bmp'. :artwork runin name='\bitmaps\drive.bmp'.
:p.
:hp6.Autoedit:ehp6. edits objects through the :link reftype=hd
res=92200.configured editor:elink.. This is the default for the
Files->Edit conditional cascade submenu when a file is the current
object. When you select Autoedit, FM/2 guesses whether the file is text
or binary data and edits it accordingly.
:artwork runin name='\bitmaps\file.bmp'.
:p.
:hp6.as text:ehp6. causes FM/2 to edit the current file object as text,
using the :link reftype=hd res=97000.configured text editor:elink. or
the internal if none is configured.
:artwork runin name='\bitmaps\file.bmp'.
:p.
:hp6.as binary:ehp6. causes FM/2 to edit the current file object as
binary data, using the :link reftype=hd res=97000.configured binary
editor:elink.. Binary data is usually edited as a :link reftype=hd
res=98800.hex dump:elink.. No default binary editor is provided at
this time, but that may change.
:artwork runin name='\bitmaps\file.bmp'.
:p.
Edit :link reftype=hd res=99980.Drive flags:elink. is found on the drives context
menu. :artwork runin name='\bitmaps\drive.bmp'.
:p.
:artwork name='..\..\bitmaps\attrlist.bmp' align=center.
:p.
:hp6.:link reftype=hd res=95900.Attributes:elink.:ehp6. leads to a
dialog that sets objects' attributes and (optionally) date/time. This is
the default for the Files->Edit conditional cascade submenu when a
directory is the current object.
:artwork runin name='\bitmaps\file.bmp'. :artwork runin name='\bitmaps\fldr.bmp'. :artwork runin name='\bitmaps\drive.bmp'.
:p.
:artwork name='..\..\bitmaps\ea.bmp' align=center.
:p.
:hp6.:link reftype=hd res=95000.Extended Attributes:elink.:ehp6. allows
you to view an object's extended attributes (EAs) and to edit and add
text attributes.
:artwork runin name='\bitmaps\file.bmp'. :artwork runin name='\bitmaps\fldr.bmp'. :artwork runin name='\bitmaps\drive.bmp'.
:p.
:hp6.Subject:ehp6. allows you to give an object a description. This
makes use of the same EA (.SUBJECT) that the WPS uses for object
descriptions -- you can see and edit it on the File page of an object's
Settings notebook.
:artwork runin name='\bitmaps\file.bmp'. :artwork runin name='\bitmaps\fldr.bmp'. :artwork runin name='\bitmaps\drive.bmp'.
:p.
:artwork name='..\..\bitmaps\print.bmp' align=center.
:p.
:link reftype=hd res=99985.:hp6.Print:ehp6.:elink. prints text files.
It'd be a good idea to have a printer to which to print, and have
configured it first, before trying to use this. If using the standard
WPS, be sure you have a printer object configured for the device you've
told FM/2 to use, or you may wind up with FM/2's printing thread blocked
for eternity awaiting access to a nonexistent or inaccessible device.
Actually, if using the standard WPS, it's recommended that you simply
drag files to the printer object and drop them instead of using this
command. :artwork runin name='\bitmaps\file.bmp'.
:p.
:artwork name='..\..\bitmaps\opend.bmp' align=center.
:p.
The :hp6.Open:ehp6. conditional cascade submenu allows you to open the
current object. Since this is a conditional cascade submenu, you can
click on the button to get a list of choices or click elsewhere to get a
default open action for files is to either run them (executable files) or open them
based on their file association. :artwork runin name='\bitmaps\file.bmp'.
:artwork runin name='\bitmaps\fldr.bmp'. :artwork runin name='\bitmaps\drive.bmp'.
:p.
The :hp6.Settings notebook:ehp6. option opens the object's WPS properties notebooks.
:artwork runin name='\bitmaps\file.bmp'. :artwork runin name='\bitmaps\fldr.bmp'. :artwork runin name='\bitmaps\drive.bmp'.
:p.
Open a directory or drive as a WPS Folder in either :hp6.icon, details or tree:ehp6. view
:artwork runin name='\bitmaps\fldr.bmp'. :artwork runin name='\bitmaps\drive.bmp'.
:p.
Open a new FM/2 window (container; the default for directories and drives).
:artwork runin name='\bitmaps\fldr.bmp'. :artwork runin name='\bitmaps\drive.bmp'.
:p.
Opening a file's Default view will honor any OS/2 associations that you have
setup. Remember that :color fc=default bc=palegray.F6:color fc=default bc=default. or :color fc=default bc=palegray.Ctrl:color fc=default bc=default. + double-click opens an object's
default WPS view, and :color fc=default bc=palegray.Ctrl:color fc=default bc=default. + :color fc=default bc=palegray.Shift:color fc=default bc=default. + double-click opens an object's
WPS Settings notebook.
:p.
:artwork name='..\..\bitmaps\object.bmp' align=center.
:artwork name='..\..\bitmaps\shadow.bmp' align=center.
:p.
Next is a conditional cascade submenu called :hp6.Create Objects:ehp6.,
with Shadows as the default command. Create shadows appears as a
a top level menu item on the drives object menu.
:artwork runin name='\bitmaps\file.bmp'. :artwork runin name='\bitmaps\drive.bmp'.
:p.
:hp6.:link reftype=hd res=93600.Shadow:elink.:ehp6. builds WPS shadow
objects on your desktop or :hp6.Shadows in folders:ehp6. for selected
object(s). :artwork runin name='\bitmaps\file.bmp'. :artwork runin name='\bitmaps\drive.bmp'.
:p.
You can also create :hp6.Real Objects:ehp6. for files
:artwork runin name='\bitmaps\file.bmp'.
:p.
The save lists to clipboard/file submenu allows you to save lists to the clipboard or a file
:p.
:artwork name='..\..\bitmaps\saveclip.bmp' align=center.
:p.
:hp6.Save/Append to clipboard:ehp6. allows you to save selected objects to the
clipboard as a text list, one per line. This is a good way to transfer
selections of files to other programs; for instance, you might copy a
list of files to the clipboard and feed it to a terminal program to send
the files over a modem or network. You can save/append the full path names or
just the filenames.
:artwork runin name='\bitmaps\file.bmp'. :artwork runin name='\bitmaps\fldr.bmp'. :artwork runin name='\bitmaps\drive.bmp'.
:p.
:artwork name='..\..\bitmaps\savelist.bmp' align=center.
:p.
:hp6.:link reftype=hd res=96000.Save to list file:elink.:ehp6. lets you
save selected objects as a list to a text file. Lists can include file
sizes, subjects, etc.
:artwork runin name='\bitmaps\file.bmp'. :artwork runin name='\bitmaps\fldr.bmp'. :artwork runin name='\bitmaps\drive.bmp'.
:p.
:artwork name='..\..\bitmaps\collect.bmp' align=center.
:p.
:hp6.Collect File(s):ehp6. calls up the :link reftype=hd
res=90100.Collector:elink. and places the selected files and directories
into it. You can also open the Collector and drag things into it.
:artwork runin name='\bitmaps\file.bmp'. :artwork runin name='\bitmaps\fldr.bmp'. :artwork runin name='\bitmaps\drive.bmp'.
:p.
:hp6.Collect List in file(s):ehp6. collects the files listed inside the
selected files (see :hp6.Save to list file:ehp6. above). The filename
should be the first item on each line of the list. If spaces are
contained in the filenames, enclose the filenames in "quote marks."
Filenames must be full pathnames (d&colon.\path\filename). Directories as
well as files can be Collected. :artwork runin name='\bitmaps\file.bmp'.
:p.
:artwork name='..\..\bitmaps\archive.bmp' align=center.
:p.
:hp6.:link reftype=hd res=90300.Archive:elink.:ehp6. allows you to build
an archive containing the selected object(s).
:artwork runin name='\bitmaps\file.bmp'. :artwork runin name='\bitmaps\fldr.bmp'. :artwork runin name='\bitmaps\drive.bmp'.
:p.
:artwork name='..\..\bitmaps\extract.bmp' align=center.
:p.
:hp6.:link reftype=hd res=91000.Extract:elink.:ehp6. allows you to
extract files from selected archives. :artwork runin name='\bitmaps\file.bmp'.
:p.
:hp6.UUDecode:ehp6. decodes files that were encoded with UUEncode, a
common protocol on the Internet. Files created by UUDecoding are
appended if they already exist. :artwork runin name='\bitmaps\file.bmp'.
:p.
:artwork name='..\..\bitmaps\delete.bmp' align=center.
:p.
:hp6.Delete:ehp6. deletes objects. If the :link reftype=hd
res=99950.Confirm Delete:elink. toggle is on or if one or more directories
are among the selected objects, you get a dialog showing the selected
objects and asking you to confirm that you really meant what you said.
In that dialog you have a chance to remove some of the objects. If you
have OS/2's Undelete enabled for the drive on which the objects reside, they
may be recoverable. You also have the option of having the files moved to the
Xworkplaces/Eworkplace trahshcan from which they may be restorable.:link reftype=hd
res=99950.Delete = move to trashcan:elink.
:artwork runin name='\bitmaps\file.bmp'. :artwork runin name='\bitmaps\fldr.bmp'.
:p.
:artwork name='..\..\bitmaps\permdel.bmp' align=center.
:p.
:hp6.Permanent Delete:ehp6. deletes objects as above, but they will not
be recoverable (which may make the deletion faster). It should be noted
that when deleting directory objects, the file objects within the
directory can never be recovered, but deleting all the file objects
inside a directory (rather than the directory itself) allows things to
be recovered :hp1.if:ehp1. you use the Delete command above rather than
this Permanent Delete command :hp1.and:ehp1. have Undelete enabled (type
:link reftype=launch object='CMD.EXE' data='/C HELP UNDELETE'.HELP
UNDELETE:elink. at a command line for more information on enabling
Undelete). :artwork runin name='\bitmaps\file.bmp'. :artwork runin name='\bitmaps\fldr.bmp'.
:p.
:artwork name='..\..\bitmaps\rescan.bmp' align=center.
:p.
:hp6.Rescan:ehp6. rescans the selected drive. FM/2 tries
very hard to keep all its windows up to date, but things outside FM/2
can cause changes that FM/2 cannot know about automatically. This
command will ensure that your display is current.
:artwork runin name='\bitmaps\drive.bmp'.
:p.
:hp6.Refresh removable media:ehp6. calls LVM.EXE to find new drives and then
rescans all the drives in the tree container. This item will not appear if
LVM.EXE isn't found in your PATH.
:artwork runin name='\bitmaps\drive.bmp'.
:p.
:hp6.Expand:ehp6. expands the tree from the point where the context menu
was requested to the bottom of the branch. This isn't the same as clicking
the [+] symbol as it expands :hp1.all:ehp1. branches.
:artwork runin name='\bitmaps\drive.bmp'.
:p.
:hp6.Collapse:ehp6. collapses the tree from the point where the context
menu was requested to the bottom of the branch. This isn't the same as
clicking the [-] symbol as it collapses :hp1.all:ehp1. branches.
:artwork runin name='\bitmaps\drive.bmp'.
:p.
:artwork name='..\..\bitmaps\mkdir.bmp' align=center.
:p.
:hp6.Make Directory:ehp6. allows you to create new directories. The
name of the directory where you requested the context menu is filled in
as a starting point for convenience. Directories may be created many
levels deep in one pass.
:artwork runin name='\bitmaps\fldr.bmp'. :artwork runin name='\bitmaps\drive.bmp'.
:p.
The Miscellaneous cascade menu appears on directory and drive context menus
The following appear on this cascade.
:p.
:hp6.:link reftype=hd res=95200.Sizes:elink.:ehp6. brings up a dialog
showing how many bytes are in the selected directory and its
subdirectories. :artwork runin name='\bitmaps\fldr.bmp'. :artwork runin name='\bitmaps\drive.bmp'.
:p.
:hp6.Show all files:ehp6. is a command available on drive and directory objects
and in the container menu of Directory Containers. It invokes the
:link reftype=hd res=98500.See all files:elink. window and shows all
the files in the directory and all its subdirectories.
:artwork runin name='\bitmaps\fldr.bmp'. :artwork runin name='\bitmaps\drive.bmp'.
:p.
:artwork name='..\..\bitmaps\find.bmp' align=center.
:p.
:hp6.Seek and scan files:ehp6. is a command available on drive and directory objects
and in the container menu of Directory Containers. It invokes the
:link reftype=hd res=91600.Seek and scan files:elink. opens a dialog for specifying
the search parameters the drive or directy this is selected from is inserted as the
root for the search.
:artwork runin name='\bitmaps\fldr.bmp'. :artwork runin name='\bitmaps\drive.bmp'.
:p.
:hp6.:link reftype=hd res=92500.Undelete Files:elink.:ehp6.
:artwork runin name='\bitmaps\fldr.bmp'. :artwork runin name='\bitmaps\drive.bmp'.
:p.
:hp6.Check Disk:ehp6. runs PMCHKDSK.EXE on the selected drive. This
tests the drive and can correct some deficiencies. This is available
only in context menus requested on drives (root directories). Note
that OS/2 cannot correct defects on disks that are in use by the
system or programs (including FM/2). :artwork runin name='\bitmaps\drive.bmp'.
:p.
:hp6.Format Disk:ehp6. runs PMFORMAT.EXE on the selected drive.
:hp8.Formatting a disk will destroy any information already on the
disk.:ehp8. This is available only in context menus requested on drives
(root directories). :artwork runin name='\bitmaps\drive.bmp'.
:p.
:hp6.Partition Disks:ehp6. has four choices for partitioning tools.
They are minilvm, DFSee, LVMGUI and FDISKPM.  We check for these tools
at start up and gray out any that aren't found in your PATH. Any of these
will allow you to modify the partitions on your hard drives.
:hp8.Extreme caution should be exercised; read the help!:ehp8.
:p.
Minilvm is a partitioning tool which is avalable with eCS and is probably
the easiest to use for LVM type system. DFSee is a share ware tool with
a much wider range of features but must be obtained from Hobbes and installed
in the system path to work. LVMGUI is the IBM provided GUI tool for
Disk management. It is a JAVA based program and earily versions only work with JAVA 1.1.8.
FDISKPM is the partition management tool for preLVM systems and should not be used
on LVM based sytems. :artwork runin name='\bitmaps\drive.bmp'.
:p.
:hp6.Optimize:ehp6. runs a .CMD file with the name <Filesystem>OPT.CMD,
giving the drive to optimize as a command line argument. Therefore, for
a FAT drive C&colon. "FATOPT.CMD C&colon." would be run (through the
command interpreter defined in %COMSPEC% or CMD.EXE if none is defined)
for an HPFS drive D&colon. "HPFSOPT.CMD D&colon." and for an JFS drive E&colon. "JFSOPT.CMD
E&colon.". CMD files are supplied with FM/2 that call utility programs from
the :link reftype=hd res=100090.FM/2 Utilities collection:elink. (a
separate collection of free-for-the-using utilities, sometimes named
FM2UTILS.ZIP) or for JFS defragfs.exe supplied with OS2. You may modify these command files as required for your
system, even to call other programs than those supplied. Always be sure
to check the disk before trying to optimize it, and (in the case of the
FAT optimizer) it's a good idea to back up first. You shouldn't run the
FAT optimizer on compressed drives -- use the utilities that came with
your compression program instead. :artwork runin name='\bitmaps\drive.bmp'.
:p.
:hp6.Detach:ehp6. detaches a network drive.
:artwork runin name='\bitmaps\drive.bmp'.
:p.
:hp6.Eject:ehp6. ejects removable media from drives (for instance,
opens the door of a CD ROM drive).
:artwork runin name='\bitmaps\drive.bmp'.
:p.
:hp6.Lock:ehp6. locks a removable drive.
:artwork runin name='\bitmaps\drive.bmp'.
:p.
:hp6.Unlock:ehp6. unlocks a removable drive.
:artwork runin name='\bitmaps\drive.bmp'.
:p.
:hp6.:link reftype=hd res=90900.Drive Info:elink.:ehp6. is the first menu item on the drives
context menu. :artwork runin name='\bitmaps\drive.bmp'.

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
:hp6.Text:ehp6. switches the container to Text view. Text view is the
fastest view for a container to maintain, but provides the least
information on the objects it contains.
:p.
:hp6.Details:ehp6. switches the container to Details view. Details view
shows a great deal of information on the objects it contains, including
file sizes, dates, and times, but it is the slowest view for a container
to maintain.
:p.
The :hp6.Details Setup:ehp6. submenu allows you to control what is shown
in a Details view. Each possible field in the details view for the drive
type is shown. If the field is checked, FM/2 will show it. If not, it
won't. Set the current view to Details view and you can see the changes
as they occur. Note that for Directory Containers, the context menu
items change the current container only. Use the internal :link
reftype=hd res=94600.Settings notebook:elink. to change the default for
subsequently created containers.
:p.
:hp6.Mini Icons:ehp6. is a toggle controlling whether icons are shown
full size or in miniature in views that show icons.
:p.
:artwork name='..\..\bitmaps\filter.bmp' align=center.
:p.
:hp6.:link reftype=hd res=93400.Filter:elink.:ehp6. leads to a dialog
that lets you set filemasks and attributes for objects to include in the
container's display. Note:  For Directory Containers, this sets the
default for _this_ container. Use the internal :link reftype=hd
res=94600.Settings notebook:elink. to change the defaults for
subsequently created containers.
:p.
:hp6.Resort:ehp6. resorts items in a container. If you have more than
one Directory or Archive Container window open, selecting a new sort
type only causes the container in which you requested the context menu
to resort itself (although the change will affect all future rescans,
resorts and insertions in that type of container). This command lets
you resort a container so that the new sort type is reflected in the
display.
:p.
:artwork name='..\..\bitmaps\rescan.bmp' align=center.
:p.
:hp6.Rescan:ehp6. rescans the directory associated with a directory container,
or all the drives in the tree container. FM/2 tries
very hard to keep all its windows up to date, but things outside FM/2
can cause changes that FM/2 cannot know about automatically. This
command will ensure that your display is current.
:p.
:hp6.Refresh removable media:ehp6. calls LVM.EXE to find new drives and then
rescans all the drives in the tree container. This item will not appear if
LVM.EXE isn't found in your PATH.
:p.
:hp6.Parent:ehp6. moves directory containers to the previous (parent)
directory.
:p.
:hp6.Previous Directory:ehp6. returns the container to the last
directory. This is sort of like a one-step "undo" when you switch
a container to look at a different directory.
:p.
:artwork name='..\..\bitmaps\walk.bmp' align=center.
:p.
:hp6.:link reftype=hd res=91500.Walk Directories:elink.:ehp6. leads to a
dialog that lets you walk through your directory structures, or recall
user-defined directories.
:p.
:hp6.Show all files:ehp6. is a command available on directory objects
and in the container menu of Directory Containers. It invokes the
:link reftype=hd res=98500.See all files:elink. window and shows all
the files in the directory and all its subdirectories. (on submenu of miscellaneous)
:p.
:hp6.Find in drive tree:ehp6. hilites the directory the directory container is
looking into on the drive tree. (on submenu of miscellaneous)
:p.
:artwork name='..\..\bitmaps\select.bmp' align=center.
:p.
:artwork name='..\..\bitmaps\deselect.bmp' align=center.
:p.
The :hp6.:link reftype=hd res=99100.Select:elink.:ehp6. submenu gives
you many ways to highlight and unhighlight objects in a container. This
lets you quickly build sophisticated selection sets of objects upon
which you can perform tasks.
:p.
You can select (or deselect) all files, all directories or all objects.
You can also select (or deselect) everything matching a mask string
which can contain wildcards, select (or deselect) filenames stored in
the clipboard or a list file and invert the current selections
(highlight what isn't, unhighlight what is).
:p.
The :hp6.Sort submenu:ehp6. allows you to control how objects are sorted
based on several criteria. You can also tell FM/2 to always display
directories ahead of or behind files. Note that :hp1.Last access
date:ehp1. and :hp1.Creation date:ehp1. are not tracked by the FAT file system
The difference between :hp1.Pathname:ehp1. and :hp1.Filename:ehp1. is only
apparent in the Collector. With the former, the entire pathname of the
object is used to sort. With the latter, only the filename portion is
used to sort. FM/2 maintains separate sort criteria for Drive Tree,
Collector, Directory Container and Archive Container windows. See
:link reftype=hd res=97200.Directory Container sort page:elink. and
:link reftype=hd res=97400.Collector Container sort page:elink.. Note
that Ctrl + F7 will call up the Sort menu for a given container. The
internal :link reftype=hd res=94600.Settings notebook:elink. can be
used to set the default for subsequently opened Directory Containers --
the context menu item sets the sort for _this_ container only.
:p.
Finally, FM/2 offers a full set of :hp6.Compare Selection:ehp6. tools
that let you select and deselect files based on how they compare to
unfiltered files in all other open Directory Containers (available only
in Directory Containers). To give you an idea how this might be
helpful, imagine that you just hit the :color fc=default bc=palegray.Enter:color fc=default bc=default. key in the middle of
typing a copy command, when you were reaching for the backslash key to
complete a path. Before you realize what's happening and can hit :color fc=default bc=palegray.Ctrl:color fc=default bc=default.+:color fc=default bc=palegray.C:color fc=default bc=default.,
you copied fifty files from a data directory to the root directory of
your boot drive (you shoulda used FM/2! &colon.-). Now you want to get rid of
them, but you don't want to pick each one. Open the data directory and
the root, choose "Select if in all," then delete the selected files in
the root directory. You're done.
:p.
These powerful selection tools are where a file manager really outshines
command line file management, so be sure to take a look at them. Note
that :color fc=default bc=palegray.Ctrl:color fc=default bc=default. + :color fc=default bc=palegray.F8:color fc=default bc=default. will call up the Select menu for a given container.
:p.
:hp6.:link reftype=hd res=100065.Set Target directory:elink.:ehp6. and
:hp6.:link reftype=hd res=94600.Settings notebook:elink.:ehp6. are also on this menu.
:p.
:h2 res=93715 name=PANEL_CONTEXTTREE.Context menus affecting Drive Tree container
:i1 id=aboutContextTree.Context menus affecting Drive Tree container

This menu includes many of the choices found on the
:link reftype=hd res=93710.Context menus affecting containers:elink.
This menu lacks the :hp6.view:ehp6. options except for :hp6.mini icons:ehp6. and doesn't include
:hp6.select:ehp6., :hp6.filter:ehp6. or the :hp6.miscellaneous submenu:ehp6. items.
The additional items it contains are described below&colon.
:p.
:hp6.Toggle icons:ehp6. turns the icons on and off in the Drive Tree container.
:p.
:hp6.Open Directory Container:ehp6. is used for:link reftype=hd res=98900.Opening a Directory Container:elink.
:p.
To remap (attach) a remote server to a local drive letter, enter the
UNC server name in the entry field at the top center of this dialog,
then select the drive letter from the left (attach) listbox to which to
attach the server. Finally, click the :hp1.Attach:ehp1. button.
:p.
To detach a local drive letter from a remote server, select the
drive letter from the right (detach) listbox, then click the
:hp1.Detach:ehp1. button.
:p.
When you're through remapping drives, click :hp1.Done:ehp1..
:p.
According to IBM LAN Server documentation, a UNC name consists of a
double backslash, the name of the server, another backslash, and the
name of the resource&colon. \\servername\netname
:p.
Note that FM/2 saves the UNC names you enter in the listbox below the
entry field. You can recall these names later by clicking on them.
The :hp1.Delete:ehp1. button deletes the currently selected name from
the listbox, and the :hp1.Clear:ehp1. button removes all names from
the listbox. Names are added automatically. Up to 200 names can be
stored in this manner (kept on disk between sessions in a file named
RESOURCE.DAT).
:p.
:hp6.Partition Disks:ehp6. has up to four choices for partitioning tools.
They are minilvm, DFSee, LVMGUI and FDISKPM. We gray out any that are not
found in your PATH. Any of these will allow you to modify the partitions
on your hard drives. :hp8.Extreme caution should be exercised; read the help!:ehp8.
:p.
Minilvm is a partitioning tool which is avalable with eCS and is probably
the easiest to use for LVM type system. DFSee is a share ware tool with
a much wider range of features but must be obtained from Hobbes and installed
in the system path to work. LVMGUI is the IBM provided GUI tool for
Disk management. It is a JAVA based program and earily versions only work with JAVA 1.1.8.
FDISKPM is the partition management tool for preLVM systems and should not be used
on LVM based sytems.
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
which the list will be saved. The :hp1.Find:ehp1. button calls up a
standard OS/2 open dialog to let you point and click at a file. If the
file exists, it will be appended. :hp6.Hint&colon.:ehp6. You can enter
:hp1.PRN:ehp1. as the filename to print the list.
:p.
The listbox below this contains patterns you've saved in the past (use
the :hp1.Add:ehp1. button to add the current pattern (the one in the
entry field), and the :hp1.Del:ehp1. button to remove the currently
highlighted pattern from the listbox). You can select one of the
patterns in the listbox to avoid retyping it (as you highlight a pattern
in the listbox it'll appear in the entry field). The patterns are saved
in a file named PATTERNS.DAT, one per line.
:p.
The :hp1.Pattern:ehp1. entry field contains a pattern that will be used
to format the list. Metastrings may be used to cause parts of a file
description to be written where desired (see below). Also note that the
pattern is run through a :link reftype=hd res=99500.C-style escape
encoder:elink., so that \x1b would be interpreted as an ESCAPE
character, \r\n as a carriage return and linefeed "newline," and \\ is
required to get a single '\' character.
:p.
When everything's set as you want it, click :hp1.Okay:ehp1. to save the
list. Click :hp1.Cancel:ehp1. if you change your mind.
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
Note that you can manipulate list files from REXX. An :link reftype=hd res=100080.
EXAMPLE.CMD:elink. is included in the FM/2 archive to show you how it's done. REXX scripts
written in this manner can be effectively used as Commands.
:p.

.im example.ipf

:h2 res=92500 name=PANEL_UNDELETE.Undelete Files
:i1 id=aboutUndelete.Undelete Files

:artwork name='..\..\bitmaps\undelete.bmp' align=center.
:p.
If you have :link reftype=hd res=99950.Delete = move to trashcan:elink. enabled undelete
will open the trahscan to facilitate resoring files. You will need to
rescan the directory containers in order to see the restored files. Otherwise,
this leads to a dialog that interfaces with UNDELETE.COM to allow you to
undelete files. The drive that will be operated on is determined by the
highlighted object in the directory tree. This dialog filters out files
that already exist on the disk. This only works if OS/2's del directories
have been designated in config.sys. It doesn't interact with "Trashcans"
:p.
The :hp1.Mask:ehp1. entry field lets you set a mask (which can include
a directory path). You can switch drives using the dropdown listbox.
A :hp1.Subdirs:ehp1. button lets you choose whether to show files that
can be undeleted in subdirectories as well.
:p.
You can always go directly to UNDELETE.COM if you have the need for more
control. This is provided only for convenience.
:artwork runin name='\bitmaps\fldr.bmp'. :artwork runin name='\bitmaps\drive.bmp'.
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
