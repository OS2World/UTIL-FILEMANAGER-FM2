:h2 res=95000 name=PANEL_EAS.Extended Attributes
:i1 id=aboutEAs.Extended Attributes
:artwork name='\fm3\bitmaps\ea.bmp' align=center.
This dialog allows you to view and edit text Extended Attributes (EAs)
for an object.  Binary EAs may be viewed but not edited.  You'd have to
be the sort who enjoys programming on a hex keypad to :hp2.want:ehp2. to
edit a binary EA...
:p.
There are three types of EAs that FM/2 can view and edit (the only three
types that have meaning to humans):
:p.
:hp1.ASCII EAs:ehp1. are shown in an entry field.
:p.
:hp1.Multi-value single-type EAs:ehp1. are shown in an MLE control if
the type is text.  Each line represents one 'record' of the EA.
:p.
:hp1.Multi-value mult-type EAs:ehp1. are also shown in an MLE control
if all types are text.  Each line represents one 'record' of the EA.
:p.
In general, if you don't know the purpose of an EA you shouldn't change
it.  In particular, EA names beginning with a period (i.e. .TYPE), as
these EAs are used by the WPS.  An exception is the .SUBJECT EA, for
which FM/2 provides a special context menu item.  This EA is used to
store a simple text description of an object.  FM/2's details views can
show this description and allow you to direct-edit it.
:p.
To view a particular EA, select its name in the listbox in the middle of
the dialog. The EA type will be shown on a text field toward the bottom
of the dialog, and if it's a human-editable type the appropriate control
will appear to display it; otherwise, a :link reftype=hd res=98800.hex
dump:elink. is shown in a listbox. If you edit the EA, a Change button
can be clicked to save your changes. You can delete EAs, but do so with
extreme caution and at your own risk. Otherwise, click Okay when done.
:p.
If you're viewing EAs for more than one object, you can change the
current object by scrolling the listbox containing the names of the
objects at the top of the dialog. If you want to see inside a file,
double-click it in the listbox.
:p.
See :link reftype=launch object='CMD.EXE' data='/C HELP
EAUTIL'.EAUTIL:elink. in OS/2's Command Reference for more information
on EAs and how to manipulate them.  Note that this dialog is not meant
to be a full-featured super-powerful EA editor (though it does a decent
job with text EAs and beats the pants off what comes with other file
managers -- if anything at all comes with them, that is).  You can use
:hp1.Config->Edit Commands:ehp1. to add such an external EA editor to a
list of commands that you can run on selected files, if desired.

:h3 res=95100 name=PANEL_ADDEA.Adding an Extended Attribute
:i1 id=aboutADDEA.Adding an Extended Attribute
To add an extended attribute (EA), enter its name in the top entry
field, then select a type for it from the radio buttons.  Click Okay to
create it, Cancel to abort.
:p.
OS/2 defines several Standard Extended Attributes (SEAs):
:p.
:hp1..ASSOCTABLE:ehp1. is a multi-value multi-type (MVMT) EA.  FM/2 will
not create one of these, but you can with the Association page of a
program object in the WPS.
:p.
:hp1..CLASSINFO:ehp1. is a binary attribute.  FM/2 will not create one
of these, but the WPS does automagically as required.
:p.
:hp1..ICON:ehp1. is an icon attribute.  FM/2 will not create one of these
via the EA dialog, but you can change the icon of a file system object in
other, more direct, ways with FM/2.
:p.
:hp1..CODEPAGE:ehp1. is an attribute (don't know the type).  FM/2 won't
make one.
:p.
:hp1..TYPE:ehp1. is an MVMT attribute (see a file object's Type Settings
page).
:p.
:hp1..SUBJECT:ehp1. is an ASCII attribute (see a file object's File page).
This describes the object.  FM/2 makes use of these for you.
:p.
:hp1..COMMENTS:ehp1. is an MVMT attribute (see a file object's third
File page).
:p.
:hp1..KEYPHRASES:ehp1. is an MVMT attribute (see a file object's third
File page).  OS/2 documentation describes this as an MVST, but the WPS
objects create MVMTs.
:p.
:hp1..HISTORY:ehp1. is an MVMT attribute (see a file object's third File
page).
:p.
:hp1..LONGNAME:ehp1. is an ASCII attribute that gives the full name of a
file when stored on a file system that doesn't support long filenames
(like FAT).  Sometimes you'll see them even on files stored on HPFS
drives, when invalid characters (invalid for the file system, like
a colon not used for a path separator, for example) are used.
:p.
:hp1..VERSION:ehp1. is an ASCII attribute that gives some sort of version
information.
:p.
When creating attributes of your own, you should :hp2.not:ehp2. begin
them with a period.  Try using a convention like "JOES.ATTRIBUTE"
(yourname.attribtag) to make sure it doesn't conflict with the WPS or
any apps you may run.
:p.
&period.SUBJECT, .COMMENTS and .KEYPHRASES can be modified, deleted and
added by the user without problem.  The other standard EAs are the
domain of apps and the WPS and should be left alone.  You can, of
course, create your own EAs and manipulate them with REXX or other types
of programs.

