:h2 res=95900 name=PANEL_ATTRIBSLIST.Attributes
:i1 id=aboutAttribsList.Attributes
:artwork name='..\..\bitmaps\attrlist.bmp' align=center.
This dialog allows you to set the attributes and (optionally) date and
time of all objects in a selected group from one popup dialog.  The
dialog presents you with spinboxes to change the date and time (defaults
to current time and date) and checkboxes to set the attributes
(ReadOnly, System, Hidden and Archived).  You can also modify the
selected list of objects by pushing the :hp1.Select:ehp1. button. If you
only want to change the objects' attributes and not their date and time,
uncheck the :hp1.Use Date/Time:ehp1. checkbox.
:p.
The initial state of the checkboxes and date/time spinbuttons has no
relationship to the actual state of the objects being manipulated if
more than one object was selected and acted upon by the command.  This
command affects all selected objects at once.
:p.
The attribute checkboxes are "3-state" checkboxes.  This type of checkbox
can have three different meanings:  reset attribute (cleared box), set
attribute (checked box), or ignore attribute (greyed box, "indeterminate
state," meaning leave this attribute unchanged).

:artwork name='bitmaps\3state.bmp' align=center.

Note that checking the :hp1.Use Date/Time checkbox:ehp1., setting the
date/time to the current date/time, and leaving the attribute checkboxes
greyed results in a "touch" of the file system objects selected for the
command (works like the *nix Touch command -- if you don't know what
that is, don't worry too much).  You can use the :hp1.Leave all
attrs:ehp1. button to grey all the attribute checkboxes at once.  If
all attribute checkboxes are already greyed, clicking this button causes
the attributes and date/time of the first item in the listbox to be
assigned to the controls of the dialog.
:p.
If you want to see inside a file, double-click it in the listbox.
:p.
Click :hp1.Okay:ehp1. when done, or :hp1.Cancel:ehp1. to abort.
