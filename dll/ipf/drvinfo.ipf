:h2 res=90900 name=PANEL_DRVINFO.Drive Info
:i1 id=aboutDriveInfo.Drive Info

:artwork name='..\..\bitmaps\info.bmp' align=center.
FM/2 will show you information about the drive from which you chose the
:hp1.Info:ehp1. command in a context menu.
:p.
For writeable drives, you can change the drive's :hp1.label:ehp1. here
by changing the text in the entry field and clicking :hp1.Okay:ehp1..
:p.
The dialog box shows you the type of file system, volume label, total
and available sizes of the drive (megabytes, kilobytes and units), tells
you how the drive's resources are parceled into units, gives the drive's
serial number and some :hp1.flags:ehp1..
:p.
These flags indicate special properties about the drive, like
:hp1.Removable:ehp1. (the drive allows its media to be removed and
changed) or :hp1.Not Writeable:ehp1. (the drive does not allow changes
to be written to it).
:p.
Additionally, each drive has some flags that you, the user, can set, to
tell FM/2 that you want the drive treated in some special manner. For
example, you can set a "NoLoadIcons" flag to prevent FM/2 from going to
the disk to get a file system object's icon (a default is used instead),
which can speed up scanning on slow drives. You can change the drive's
editable flags using the :link reftype=hd res=99980.Edit->Drive
flags:elink. command.

:h2 res=95700 name=PANEL_INFO.Object Information
:i1 id=aboutInfo.Object Information

:artwork name='..\..\bitmaps\info.bmp' align=center.
This comprehensive dialog tells you just about everything there is to
know about file system objects.  If information is being displayed for
more than one object, the objects may be scrolled through in the listbox
at the top of the dialog.
:p.
Note the object's icon is shown.  It's useable -- you can get a context
menu on it, or you can drag other objects onto it to change the object's
icon.
:p.
You can edit an object's EAs or WPS Settings notebook (Properties) by
clicking the appropriate button.  If you want to see inside a file,
double-click it in the listbox.
:p.
Click :hp1.Okay:ehp1. when you're through examining the objects.
