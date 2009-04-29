.***********************************************************************
.*
.* $Id$
.*
.* fm/2 help - Selection features
.*
.* Copyright (c) 1993-98 M. Kimes
.* Copyright (c) 2002, 2009 Steven H.Levine
.*
.* 29 Apr 09 SHL Update formatting
.*
.***********************************************************************
.*
:h2 res=99100 name=PANEL_SELECTION.Selection
:i1 id=aboutSelection.Selection
:artwork name='..\..\bitmaps\select.bmp' align=center.
:artwork name='..\..\bitmaps\deselect.bmp' align=center.

The :color fc=default bc=palegray.Ctrl:color fc=default bc=default. +
:color fc=default bc=palegray.F8:color fc=default bc=default. accelerator calls up the Select menu for a given
container. You can also, of course, get to it with the mouse via the
:hp1.Views:ehp1. menu.
:p.
What follows is an item-by-item description of the commands in the
Select menu. Where both Select and Deselect commands are available,
only the Select command will be discussed to save space and avoid
repetition.
:p.
:hp1.Select All:ehp1. Selects all objects in a container.
:p.
:hp1.Deselect All:ehp1. has an accelerator :color fc=default bc=palegray.Ctrl:color fc=default bc=default. + :color fc=default bc=palegray.\:color fc=default bc=default..
:p.
:hp1.Select All Files:ehp1. Selects all the file objects in a
container. The accelerator key :color fc=default bc=palegray.Ctrl:color fc=default bc=default. + :color fc=default bc=palegray./:color fc=default bc=default. is assigned to this command.
:p.
:hp1.Select All Dirs:ehp1. Selects all the directory objects in a
container. The accelerator key &ShiftKey. + :color fc=default bc=palegray.Ctrl:color fc=default bc=default. + :color fc=default bc=palegray.?:color fc=default bc=default. is assigned to this
command
:p.
:hp1.Deselect All Dirs:ehp1. has an accelerator :color fc=default bc=palegray.Shift:color fc=default bc=default. + :color fc=default bc=palegray.Ctrl:color fc=default bc=default. + :color fc=default bc=palegray.|:color fc=default bc=default.
:p.
:hp1.Select Mask:ehp1. Allows you to select files that match a filemask.
The same dialog is used for this command that's used for the :link
reftype=hd res=93400.Filter:elink. command. When you first initiate
this command, FM/2 fills in a default mask built from the current
object's name. So, to quickly select all the *.BAK files in a
container, you could select one of the files, then type :color fc=default bc=palegray.Ctrl:color fc=default bc=default. + :color fc=default bc=palegray.=:color fc=default bc=default. (the
accelerator assigned to Select Mask) and press :color fc=default bc=palegray.Enter:color fc=default bc=default. to accept the
default filemask built by FM/2 (*.BAK).
:p.
:hp1.Select clipboard:ehp1. If the OS/2 clipboard contains a list of files
(such as can be created with the :hp1.Save to clipboard:ehp1. command),
you can select any files present in both the container and the list with
this command. The accelerator for this command is :color fc=default bc=palegray.Ctrl:color fc=default bc=default. + :color fc=default bc=palegray.]:color fc=default bc=default..
:p.
The accelerator for the Deselect clipboard command is :color fc=default bc=palegray.Shift:color fc=default bc=default. + :color fc=default bc=palegray.Ctrl:color fc=default bc=default. + :color fc=default bc=palegray.}:color fc=default bc=default..
:p.
:hp1.Select List:ehp1. If you've saved a list to a file (such as can be
created with the :link reftype=hd res=96000.Save to list file:elink.
command), you can select any files present in both the listfile and the
container with this command.
:p.
:hp1.Reselect:ehp1. This command causes the container to reselect the
last selected items. The accelerator is :color fc=default bc=palegray.Ctrl:color fc=default bc=default. + :color fc=default bc=palegray.'t:color fc=default bc=default..
:p.
:hp1.Invert selection:ehp1. Selects what isn't, deselects what is. In
other words, reverses the current selection set.
The accelerator is :color fc=default bc=palegray.Ctrl:color fc=default bc=default. + :color fc=default bc=palegray.;:color fc=default bc=default..
:p.
:hp2.Compare selections:ehp2.
:p.
FM/2 provides Directory Containers with special selection commands that
can be used to select file objects based on their relationship to file
objects in other open Directory Containers. These commands are extremely
powerful tools that can let you, for instance, compare two directories to
see what's different about them. See also :link reftype=hd
res=94900.Compare Directories:elink..
:p.
:hp1.Select if in all:ehp1. This command selects any file objects that
exist in all open Directory Containers.
:p.
:hp1.Select if in more than one:ehp1. Selects any file objects that
exist in more than one open Directory Containers.
:p.
:hp1.Select if in one:ehp1. Selects any file objects that exist in only
one open Directory Container.
:p.
:hp1.Select newest:ehp1. Selects the newest file objects that exist in
more than one open Directory Container.
:p.
:hp1.Select oldest:ehp1. Selects the oldest file objects that exist in
more than one open Directory Container.
:p.
:hp1.Select largest:ehp1. Selects the largest file objects that exist in
more than one open Directory Container.
:p.
:hp1.Select smallest:ehp1. Selects the smallest file objects that exist
in more than one open Directory Container.
