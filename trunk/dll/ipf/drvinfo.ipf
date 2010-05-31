.***********************************************************************
.*
.* $Id$
.*
.*  Drive flags and stats
.*
.* Copyright (c) 1993-98 M. Kimes
.* Copyright (c) 2006 Steven H.Levine
.*
.* 01 Mar 07 GKY Add no stats drive flag
.*
.***********************************************************************
.*
:h2 res=90900 name=PANEL_DRVINFO.Drive Info
:i1 id=aboutDriveInfo.Drive Info

:artwork name='..\..\bitmaps\info.bmp' align=center.
:p.
FM/2 will show you information about the drive from which you chose the
:hp1.Info:ehp1. command in a context menu or files menu.
:p.
For writable drives, you can change the drive's :hp1.label:ehp1. here
by changing the text in the entry field and clicking &OkayButton..
:p.
The dialog box shows you the type of file system, volume label, total
and available space on the drive (megabytes, kilobytes and units), tells
you how the drive's resources are parceled into units, gives the drive's
serial number and some :hp1.flags:ehp1..
:p.
These flags indicate special properties about the drive, like
:hp1.Removable:ehp1. (the drive allows its media to be removed and
changed) or :hp1.Not Writable:ehp1. (the drive does not allow changes
to be written to it).
:p.
Additionally, each drive has some flags that you, the user, can set, to
tell FM/2 that you want the drive treated in some special manner. For
example, you can set a "NoLoadIcons" flag to prevent FM/2 from going to
the disk to get a file system object's icon (a default is used instead),
which can speed up scanning on slow drives. You can change the drive's
editable flags using the :link reftype=hd res=99980.Edit->Drive
flags:elink. command.

:h3 res=99980 name=PANEL_FLAGS.Drive flags
:i1 id=aboutFlags.Drive flags

Here you can set flags for various drives. Command line switches
override these flags. The flags are stored in FM/2's INI file and
loaded when FM/2 starts, so this is an alternative to all the
esoteric drive command line switches :hp1.except:ehp1. the
:hp2.Ignore:ehp2. switch.
:p.
:hp6.No prescan:ehp6. Setting this causes FM/2 to :hp1.not:ehp1.
pre-scan removable drives. You have to double-click the drive
before it's checked to see if it has any subdirectories. This is
handy for those of you with CD carousels.
:p.
:hp6.Don't load icons:ehp6. Prevents FM/2 from loading icons for
files and directories on this drive. If the drive contains only
DOS programs and data files or is a very slow drive, you might
want to check this one.
:p.
:hp6.Don't load subjects:ehp6. Prevents .SUBJECT EAs from being
automatically loaded on this drive. Again, if your drive is slow or you
don't use .SUBJECTs with objects on this drive, you might want to check
it.
:p.
:hp6.Don't load longnames:ehp6. Prevents .LONGNAME EAs from being
automatically loaded on this drive. You've got the idea by now,
right?
:p.
:hp6.Slow drive:ehp6.. Check this for drives which have extremely slow
seek times (like ZIP and EZ removable hard drives). The Autoview window
and associated messages are disabled for this drive, and the "Quick Arc
find" method is always used, whether on globally or not, which snaps up
response time. I may take other shortcuts for drives with this attribute
later (loosen error checking). For such slow drives you may also want
to check the various :hp1.Don't load...:ehp1. flags listed above.
:p.
:hp6.Include files in tree:ehp6. If you check this, files will be shown
as well as directories in the Drive Tree for this drive. I have no
idea why you would ever want to check this.
:p.
:hp6.No drive stats:ehp6. If you check this, no stats about size free space
etc will be displayed on the information dialog of this drive. The drive will
not be shown when FM2 is minimized to a databar. The purpose for this flag is for
virtual drives such as netdrives, where multiple drives and/or directories can be
mounted making the stats meaningless at best.
:p.
:hp6.Write verify off:ehp6. Checking this button will turn write verify off for
this drive only. If write verify is off globally it has no effect. This was added
because some USB removable fail when write verify is on. The flag is set by default
for writable removable drives.
:p.
You get this dialog by selecting Edit->Drive flags from a context menu
requested on a drive (root directory) in the Drive Tree or VTree window.
:p.
Note&colon. To set the drive flags on a removable drive, like a floppy
or CD-ROM, be sure you put a disk in the drive first. FM/2 won't let
you set drive flags on a currently invalid disk

:h2 res=95700 name=PANEL_INFO.Object Information
:i1 id=aboutInfo.Object Information

:artwork name='..\..\bitmaps\info.bmp' align=center.
:p.
This comprehensive dialog tells you just about everything there is to
know about file system objects. If information is being displayed for
more than one object, the objects may be scrolled through in the listbox
at the top of the dialog. The dialog is accessed from :hp1.View->Info:ehp1.
of the objects context menu.
:p.
Note the object's icon is shown. It's usable -- you can get a context
menu on it, or you can drag other objects onto it to change the object's
icon.
:p.
You can edit an object's EAs or WPS Settings notebook (Properties) by
clicking the appropriate button. If you want to see inside a file,
double-click it in the listbox.
:p.
Click &OkayButton. when you're through examining the objects.


