:h2 res=90400 name=PANEL_ASSOC.Associations
:i1 id=aboutAssociations.Associations
:artwork name='..\..\bitmaps\assoc.bmp' align=center.
(Note&colon.  if you're wondering why FM/2 has Associations separate
from OS/2's, see the :link reftype=hd res=90401.Why separate
Associations?:elink. topic.  Also note that you can :link reftype=hd
res=90402."fall through" to OS/2's associations:elink. if desired.)
:p.
:hp1.Associations:ehp1. are programs that are run when files matching
specified filemasks (and optional file signatures) are selected
(double-clicked).
:p.
You can use this facility to cause editors specific to different
datafile types to start when the datafile is selected.  For instance, if
you associate "*&period.ICO" with "ICONEDIT.EXE %a" the icon editor will
be started with the selected icon file when you double-click an icon. A
special association command line, "<>", causes FM/2 to open the default
WPS view of an object.  :hp8.Warning&colon.:ehp8.  Do not use this on a
file associated with AV/2 as you will cause an endless loop as AV/2
constantly starts itself.
:p.
Signatures provide a mechanism to further test a matching file to
determine that it is the proper type.  For example, all OS/2 &period.INF
(information) files have the string "HSP" at position 0 of the file. By
using signature "HSP" at offset 0 for the filemask "*&period.INF" and
assigning the command line "VIEW.EXE %a" to the association, any OS/2
&period.INF file will be read using VIEW when selected, but non-OS/2
files that have an &period.INF extension will not match this association.
The signature entry field supports :link reftype=hd res=99500.C-style
escaping.:elink.
:p.
To add an association, fill in the entry fields and set the radio
buttons and checkboxes that control session type as desired (these are
explained in more detail in the help for :link reftype=hd
res=90600.Editing Commandline:elink. except for :hp1.Prompt:ehp1.,
which causes a dialog to appear that allows editing the command line
before it is run), then click :hp1.Add.:ehp1..
:p.
To delete an association, select it in the listbox, then click
:hp1.Del:ehp1..  You should be aware that the association deleted is
the one matching the entry fields, specifically the mask, signature
and offset fields.
:p.
To change an association, delete it, edit the entry fields, radio
buttons and checkboxes, then add it.
:p.
The :hp1.Top:ehp1. button moves the highlighted Association to the top
of the listbox.
:p.
The :hp1.Find:ehp1. button brings up a standard OS/2 open dialog that
you can use to point-and-click at the desired executable file.  It's
pathname will be entered into the command line entry field.
:p.
The :hp1.Close:ehp1. checkbox will cause FM/2 to close after executing
this association.  Please be sure that's the behavior you really want
before checking this button.
:p.
The :hp1.Environment:ehp1. MLE control lets you enter environment strings
for the program to inherit.  Generally speaking, this is only for running
DOS programs as any strings entered here are interpreted as DOS settings.
For example, :hp3.IDLE_SECONDS=5:ehp3. would adjust the DOS setting
IDLE_SECONDS to 5.  Names of DOS settings are as shown in the Settings
notebook for a DOS program.
:p.
See also :link reftype=hd res=100075.Metastrings:elink..
:p.
The command that gets you to this dialog is Config->Edit Associations.


:h3 res=90401 name=PANEL_WHYASSOC.Why separate Associations?
:i1 id=aboutWhyAssociations.Why separate Associations?
Under OS/2's WPS you can set up associations so that double-clicking a
datafile object invokes a program.  Usually you would use this to start
the program that edits the file; for example, if you double-clicked a
graphics file you'd want your graphics editing program to be started.
:p.
However, while using a file manager, you would probably prefer to have
a simple viewer started to look at the file quickly so you can decide
what to do with the file.  Therefore, FM/2 allows you to set up
Associations for files that are different than those that OS/2 uses.
This lets you invoke viewers in FM/2 via double-click, but invoke
editors in OS/2 via double-click.
:p.
Moreover, FM/2's Associations can be a bit smarter than OS/2's.  You can
associate a signature with a filemask and only files that match both
will be viewed with the associated executable.  You can even use these
signatures to invoke OS/2's own associations, providing added assurance
that the right executable is being invoked.  For example, if you have
a plain text file with the extension ".INF," OS/2 would try to view it
with VIEW.EXE, which wouldn't know what to do with it.  If you make an
FM/2 Association and give "HSP" as the signature at offset 0 of the file,
only OS/2 INFs will be passed to VIEW.EXE.
:p.
Finally, don't forget that you can :link reftype=hd res=90402.access
OS/2's associations:elink. from FM/2, so you've really lost nothing
and stand to gain considerably.

:h3 res=90402 name=PANEL_OS2ASSOC.Accessing OS/2's associations
:i1 id=aboutOS2Associations.Accessing OS/2's associations
You can access OS/2's associations for a file system object at any time
by pressing the F6 key or holding down Ctrl while double-clicking the
object.  This causes a default WPS open to be performed on the object.
:p.
If you want to access OS/2's associations by an unaugmented double-click,
enter "<>" as the executable in the FM/2 Association definition.  This
causes FM/2 to do a default WPS open on the object.  Note that if you
wanted :hp2.all:ehp2. objects to use OS/2's associations, you'd need only
one FM/2 Assocation (filemask "*", executable "<>").
