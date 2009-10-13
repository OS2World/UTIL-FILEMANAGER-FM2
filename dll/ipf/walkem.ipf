.***********************************************************************
.*
.* $Id$
.*
.* Walk dialog help
.*
.* Copyright (c) 1993-98 M. Kimes
.* Copyright (c) 2002, 2009 Steven H.Levine
.*
.* 07 Oct 09 SHL Drop 100 file limit text
.*
.***********************************************************************
.*
:h1 res=91500 name=PANEL_WALKEM.Walk Directories
:i1 id=aboutWalkem.Walk Directories

:artwork name='..\..\bitmaps\walk.bmp' align=center.
:p.
This dialog lets you pick a directory by "walking" through the
directory structure of your drives. It also lets you save and
recall user-defined directories.
:p.
On the left is a listbox containing all your drive letters. If you
select a drive, the directories on that drive fill the center listbox.
:p.
If you double-click one of these directories, any subdirectories of
that directory are displayed, as well as a special directory named ".."
which is actually the previous (parent) directory. In this manner you
can walk to any directory on any drive.
:p.
The listbox on the right of the window (:hp1.User List:ehp1.) contains
only directories that you add to it. To add a directory, click
:hp1.Add:ehp1. when the desired directory name is displayed in the
entry field at the bottom of the window. To delete a directory,
highlight it and click :hp1.Delete:ehp1..
To switch to one of these
user-defined directories, highlight it and click &OkayButton. or
double-click the directory.
To highlight the directory without switching, hold down the &CtrlKey.
and click on the directory.
:p.
You can also select directories from the Recent directories drop-down
list at the top right of the window. FM/2 adds to this list
automatically as you traverse your drives. Just drop down the list
and click the directory of choice.
To exit the drop down list without making a selection, click the drop down arrow
or Tab out of the drop down list with the Tab key.
:p.
When the desired directory is displayed in the bottom entry field of
the dialog, click &OkayButton. to exit. Click the &CancelButton. to
exit without selecting a directory.

