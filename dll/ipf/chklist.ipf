:h1 res=95600 name=PANEL_CHECKLIST.Checking Lists
:i1 id=aboutCheckList.Checking Lists
On occasion you may be asked to check a list of objects. You'll be
presented with a list of highlighted objects in a listbox. To remove an
object from the list, unhighlight it (:hp6.hint:ehp6.&colon. hold down
the Ctrl key while clicking to unhighlight a single item). When you've
got the list the way you want it, click Okay. Click Cancel to abort the
action.
:p.
Since this dialog may appear for more than one reason, additional
information is provided in a multiline text field below the listbox.

:h1 res=98700 name=PANEL_DROPLIST.Drag and drop dialog
:i1 id=aboutDropList.Drag and drop dialog
If you have the "Drag&amp.Drop Dialog" toggle on, this dialog appears
when you drop files onto an FM/2 Directory Container or object within
one, or onto a directory object in the Drive Tree, or onto an object in
the Collector. You can then pick the action to be performed from the
buttons below the listbox. You can remove an object from the listbox if
you've changed your mind about including it in the action (hint: hold
down the Ctrl key while clicking to unhighlight a single item).
:p.
The :hp1.+Rename:ehp1. toggle causes :hp1.Copy:ehp1. and :hp1.Move:ehp1.
operations to allow you to change the name of the destination using the
standard :link reftype=hd res=91400.Rename dialog:elink..
:p.
The :hp1.Shadow:ehp1. toggle causes :hp1.Object:ehp1. to create Shadow
objects instead of "real" objects. Directory objects are always created
as Shadow objects.
:p.
Note that creating Shadows in directories not under the Desktop folder
probably isn't desirable.
:p.
The :hp1.Launch:ehp1. button causes FM/2 to bring up the Execute dialog
to run the target of the drop with the dragged objects as arguments.
