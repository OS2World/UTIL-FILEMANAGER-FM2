.***********************************************************************
.*
.* $Id$
.*
.* fm/2 help - Container filtering
.*
.* Copyright (c) 1993-98 M. Kimes
.* Copyright (c) 2002, 2009 Steven H.Levine
.*
.* 29 Apr 09 SHL Update formatting
.*
.***********************************************************************
.*
:h2 res=93400 name=PANEL_FILTER.Filter container
:i1 id=aboutFilter.Filter container
:artwork name='..\..\bitmaps\filter.bmp' align=center.
:p.
This dialog allows you to filter what's shown in a container. A
filemask or filemasks can be used to filter, and so can file attributes
(except for archive listings where attributes are not applicable). In
addition, you can set attributes that _must_ be present on the objects
to be shown (for instance, if you check :hp1.Hidden:ehp1. in the
"Must-have Attribs" group, only objects with their hidden attribute set
will show up). You can specify whether FM/2 should always show
directories whether they match the mask(s) or not by checking the
:hp1.Always show directories:ehp1. checkbox to keep them visible.
:p.
:hp2.To be sure that everything in a container is displayed, clear any
filemask, check all attributes in the Attribs groupbox and clear
all attributes in the Must-have Attribs groupbox.:ehp2. You can click
the :hp1.All:ehp1. button to set the filter to show everything.
:p.
As you enter filemasks they're saved for later redisplay in this
dialog's listbox. If a filemask desired is in the listbox you can use
it by selecting it (double-clicking it). You can get rid of a mask in
the listbox by highlighting it and clicking :hp1.Delete:ehp1..
:p.
Multiple filemasks can be used by separating the masks with semi-colons.
:p.
"No filemask" can be quickly entered by just clicking the
:hp1.Clear:ehp1. and &OkayButton. buttons, or "*" can be used.
:p.
Wildcard matching is not case-sensitive.
:p.
If a filename does not have a period, an implicit one is automatically
appended to the end during matching operations.
:p.
Some characters have the following special meaning:
:p.
 :hp1.?:ehp1. A question mark matches one character, unless what it would
match is a period, slash or backslash, in which case it matches no
characters.
:p.
 :hp1.*:ehp1. An asterisk matches characters from the source to the target
until it finds a filename character that matches the non-wild character
following it in the filemask, or a period, slash, backslash or
the end of the filename and/or filemask. The pattern *stuff* where "stuff"
doesn't contain an * or ? will match based on a search for "stuff" in the
file name so it will match even is the first letter occurs mutliple times
prior to the sring being matched. The pattern *stuff (stuff can contain an
* or ?) is search backwards on the assumption that for most situations the
final extension is the marker for the file type.
:p.
Therefore, "*.f?o" matches "anything.foo" but not "anything.foe". While read*
will fail to match rreadme.txt *read* will result in it being matched. Additionally,
*.zip will match any file with the last extension of .zip (ie it will match myfile.zip
and myfile.1.zip but not myfile.zip.bak). While *.zip* will match all 3 of the files
from the preceeding example.
:p.
Up to 24 masks may be "cascaded" by separation with semicolons.
When specifying multiple filemasks, you can use '/' as the first character
of a mask to mean _don't_ match this filespec. Exclusions should usually
be listed before inclusions to attain the desired effect.
:p.
:hp2.Filemask examples&colon.:ehp2.
:p.
:parml compact tsize=37 break=none.
:pt./*.obj;/*.res;*
:pd.(Show all but *.obj and *.res files where .obj and .res are the last file extension)
:pt./*.obj*;/*.res*;*
:pd.(Show all but *.obj and *.res files where .obj and .res are any file extension)
:pt.*.c;*.h
:pd.(Show only C source and header files)
:pt.*.ico
:pd.(Show only icon files)
:pt.*.zip;*.lzh;*.zoo;*.arj;*.arc;*.rar
:pd.(Show only archive files)
:pt.*
:pd.(Show everything)
:eparml.
:p.
:hp1.A reminder&colon.:ehp1. CD-ROM files are marked ReadOnly and will
not appear in your containers unless you have the ReadOnly attribute on
(checked). If you copy these files to your hard drive the ReadOnly
attribute goes with them; use the menu command
:hp1.Files->Edit->Attribs:ehp1. (&CtrlKey. + a accelerator) to reset it (you
can do this to multiple files at once).
:p.
This dialog also appears when you are selecting or deselecting file system
objects using a mask. In this case, attribute information will be grayed
out and an additional entry field (:hp1.Text&colon.:ehp1.) appears. You
can enter text into this entry field, and only files containing the text
and matching the mask(s) will be (de)selected. The title bar will remind
you why you called up the dialog.
