:h2 res=90700 name=PANEL_COMMAND.Editing Commands
:i1 id=aboutCommands.Editing Commands
:artwork name='\fm3\bitmaps\command.bmp' align=center.
This dialog allows you to edit the commands that are available in the
Commands submenu.  There is always a :hp1.Do it yourself:ehp1. command
in the Commands submenu, and it is the default for the conditional
cascade. The Do it yourself command supplies the names of selected files
as the command line and allows you to enter a command to be performed on
the files in an entry field.  Note that OS/2 command lines are limited
to 1,000 characters.
:p.
To add a command to this submenu, fill in the entry fields and set the
radio buttons and checkboxes that control session type as desired (these
are explained in more detail in the help for :link reftype=hd
res=90600.Editing Commandline:elink., except for :hp1.Each:ehp1., which
means that the command will be run once for each selected file, and
:hp1.Prompt:ehp1., which means that the command will display a dialog
that allows the user to edit the command line before running), then
click :hp1.Add.:ehp1..
:p.
To delete a command, select it in the listbox, then click
:hp1.Del:ehp1..  You should be aware that the command deleted
is the one matching the entry field, specifically, the title field.
:p.
To change a command, delete it, edit the entry fields, radio buttons and
checkboxes, then add it.
:p.
The :hp1.Find:ehp1. button brings up a standard OS/2 open dialog that
you can use to point-and-click at the desired executable file.  It's
pathname will be entered into the command line entry field.
:p.
The :hp1.Environment:ehp1. MLE control lets you enter environment
strings for the program to inherit.  Generally speaking, this is only
for running DOS programs where any strings entered here are interpreted
as DOS settings. For example, :hp3.IDLE_SECONDS=5:ehp3. would adjust the
DOS setting IDLE_SECONDS to 5.  Names of DOS settings are as shown in
the Settings notebook for a DOS program.
:p.
See also&colon.
.br
:link reftype=hd res=100075.Metastrings:elink.
.br
:link reftype=hd res=95800.Reordering Commands:elink.

:h3 res=95800 name=PANEL_REORDERCOMMANDS.Reordering Commands
:i1 id=aboutReorderingCommands.Reordering Commands
This dialog, accessed from the Edit Commands dialog, allows you to
rearrange the order of Commands.  You take selected items from the left
listbox and Add them to the end of the right listbox with the
:hp1.Add>>:ehp1. button. When you've moved everything to the right
listbox, click :hp1.Okay:ehp1..  Click :hp1.Cancel:ehp1. if you change
your mind.
:p.
In reality, you don't need to move everything to the right listbox.  You
can move only what you want moved to the top of the list, then click
Okay.  Anything remaining in the left listbox is added to the end of
what's in the right listbox.
:p.
The :hp1.<<Remove:ehp1. button can be used to move selected items from
the right listbox to the bottom of the left listbox.


:h3 res=100075 name=PANEL_METASTRINGS.Metastrings
:i1 id=aboutMetastrings.Metastrings
The following "metastrings," or replaceable parameters, can be used in
command lines&colon.
:parml compact tsize=6 break=none.
:pt.%$
:pd.drive letter
:pt.%a
:pd.full pathnames
:pt.%A
:pd.full pathnames, no leading drive letters
:pt.%r
:pd.full pathnames, no quoting under any circumstances
:pt.%R
:pd.full pathnames, no leading drive letters, no quoting
:pt.%c
:pd.command processor specified in %COMSPEC%
:pt.%f
:pd.filenames, no paths
:pt.%F
:pd.filenames, no paths or extensions
:pt.%e
:pd.extensions
:pt.%p
:pd.path of execution (d&colon.\directory)
:pt.%P
:pd.path of execution (\directory)
:pt.%t
:pd.designated Target directory
:pt.%u
:pd.path of first datafile (d&colon.\directory)
:pt.%U
:pd.path of first datafile (\directory)
:pt.%d
:pd.full pathnames of all open Directory Containers
:pt.%D
:pd.full pathname of current directory in Drive Tree
:pt.%!
:pd.listfile name (first places full filenames in a list file; user
Command should delete listfile when complete, but FM/2 will clean any
left over up when FM/2 closes).  This is an advanced command -- see
EXAMPLE.CMD in the FM/2 distribution archive for a skeleton program that
you can use to do work on the filenames listed in the listfile.  You can
use %! more than once in the same command line -- the listfile will only
be built once, but the name of that listfile will be inserted each time.
:pt.%%
:pd.a percent sign
:eparml.
:p.
When you enter a metastring into a command line, the metastring is
replaced with the appropriate text.  For example, "%c /C MYCMD.CMD %a"
might become "CMD.EXE /C MYCMD.CMD d&colon.\file1 d&colon.\file2
d&colon.\file3".
