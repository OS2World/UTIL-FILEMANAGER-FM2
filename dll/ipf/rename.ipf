:h2 res=91400 name=PANEL_RENAME.Renaming
:i1 id=aboutRenaming.Renaming
:artwork name='..\..\bitmaps\rename.bmp' align=center.
When you rename a file sytem object other than by :link reftype=hd
res=98200.Direct Editing:elink., or a naming conflict arises, you get
the Rename dialog.  Note that not all the controls discussed appear for
simple rename procedures -- some are applicable only when copying or
moving files.
:p.
The dialog displays the :hp1.Source filename:ehp1. and proposed
:hp1.Target filename:ehp1., along with information about both objects.
:p.
Below these two fields is an outlined text box that attempts to give you
directions on what to do (usually to change the target filename -- when
renaming, the target name is initially the same as the source name). If
the target file exists, you will be told whether the source is larger or
smaller, older or newer.  You will be told whether the source and target
are files or directories.  Sometimes you may see a warning "cannot
access source."  This means that FM/2 could not open the source file in
a test, and may indicate that the file is already in use and thus OS/2
may not allow it to be renamed, moved or copied.  You can use this
information display to make decisions more easily.
:p.
After entering the new target name, click :hp1.Okay:ehp1..  If the new
target name exists, the display will be updated to reflect the new
information.  You can, at that point, enter a new name to avoid the
conflict or click :hp1.Overwrite:ehp1. (which will destroy the old
target file, keeping in mind that you can't overwrite a file with
itself).  :hp1.Skip:ehp1. can be used to skip one file when you are renaming
several in one action; nothing will be done for that file.  Click
:hp1.Cancel:ehp1. if you change your mind and want to abort the whole
thing.
:p.
The :hp1.Rename Existing:ehp1. button allows you to rename (move) the
existing file system object which is causing a naming conflict.  For
example, if you are trying to copy a file to "C&colon.\MYFILE" and a
"C&colon.\MYFILE" already exists, you could change the target name as
desired, click :hp1.Rename Existing:ehp1. to rename (move) the existing
file out of the way, then click :hp1.Okay:ehp1. to continue the copy
operation.
:p.
The :hp1.Overwrite if target older or same:ehp1., :hp1. Overwrite if
target newer:ehp1. and :hp1.Don't ask again:ehp1. checkboxes can be used
in combination to avoid seeing this screen again for naming conflicts.
Files which have existing targets that do not match an overwrite
characteristic will be automatically skipped.  For example, if you don't
check either of the Overwrite... checkboxes, all existing files will be
skipped, but if you check the ...older checkbox, all existing files
older than the source file will be overwritten, while existing files
newer than the source file will be skipped.  Exception:  You will always
be prompted if you try to overwrite a directory with a file.
:p.
Wildcard characters '?' and '*' can be used in the target name --
behavior is OS/2 standard.
