:h1 res=99300 name=PANEL_NEWVIEWER.Internal Viewer
:i1 id=aboutNewViewer.Internal Viewer

:artwork name='..\..\bitmaps\view.bmp' align=center.
The internal viewer is used to view files unless you have an external
viewing program defined in the internal Settings notebook's :link
reftype=hd res=92200.Viewers page:elink., or the :hp2.Fast internal
viewer:ehp2. checkbox :hp1.off:ehp1..  This viewer loads and displays a
one megabyte text file in less than two seconds on a 486/66. The :link
reftype=hd res=93900."old" viewer:elink. loads large text files
considerably more slowly, but you might want to use it anyway; your
choice.
:p.
Both :link reftype=hd res=98800.hex:elink. and ASCII (plain text)
display modes are supported, and a variety of text sizes.  These
controls are under the :hp1.View:ehp1. pulldown submenu.
:p.
Selected lines are displayed in reverse video (white text on a black
background).  The current line has a ">" pointing at it in the left
margin.  Single selections and swipe selections are supported, as well
as select and deselect all (under the :hp1.Select:ehp1. pulldown
submenu) and select/deselect all "found" lines (see :hp1.Search:ehp1.
below).  Keyboard selection is performed with the spacebar, or you can
hold down the Shift key while moving the cursored selection with the
arrow keys.  Discontiguous lines can be selected.
:p.
If you double-click a line in the viewer window, a listbox appears above
the text containing that line.  Select (single-click) the line in the
listbox and the viewer window will scroll to that line -- a sort of
instant bookmarking facility.  To remove a line from this bookmark
listbox, double-click it in the listbox.  See the :link reftype=hd
res=97600.FM/2 window layouts:elink. topic for a picture to help you
understand this one -- or just try it.
:p.
The :hp1.Clipboard:ehp1. pulldown submenu allows you to save selected
lines to the clipboard or a file.  :hp7.Warning&colon.:ehp7.  Warp
appears to have a 64K limit to the size of text that can be placed in
the clipboard.  You can also save lines that you've double-clicked into
the bookmark listbox.  When you write lines to a file, you append to the
file (if it already exists).
:p.
When you search for text in the file, you can search for more than one
"phrase" at a time.  Each line you fill into the MLE on the
:hp1.Search->Find text:ehp1. dialog is a separate search string.  You
can also search case sensitively (i.e. 'A' doesn't match 'a'), translate
C-like :link reftype=hd res=99500.\-encoded characters:elink. (\r = a
carriage return, for example -- useful when searching binary files),
and/or select lines as they're found by checking the appropriate
checkboxes on the Find First dialog. All matching lines are displayed in
red.  The :hp1.Search->Next found line:ehp1. command moves to the next
highlighted line in the file (from the current position), and
:hp1.Search->Previous found line:ehp1. moves to the previous highlighted
line.  Colors are configurable.
:p.
If you're looking for more powerful viewing software, you might be
interested in Michael Schacter's :hp1.Hyperview PM:ehp1. shareware
program.  Michael can be contacted on Compuserve at user ID 76170,1627,
and hangs out in the OS2SHARE (library 1 of OS2BVEN) forum. You can
easily set Hyperview up to be used automatically by FM/2 using the :link
reftype=hd res=92200.Viewers page:elink. of the internal Settings
notebook.

:h2 res=93900 name=PANEL_EDITOR.Internal Viewer/Editor
:i1 id=aboutEditor.Internal Viewer/Editor

:artwork name='..\..\bitmaps\edit.bmp' align=center.
:artwork name='..\..\bitmaps\view.bmp' align=center.
The internal viewer/editor is an extremely simplistic MLE window.  It is
:hp2.strongly:ehp2. recommended that you replace it with a better one
via the :link reftype=hd res=94600.Settings Notebook:elink.'s :hp1.:link
reftype=hd res=92200.Viewers page:elink.:ehp1..  The reasons are
simple&colon. a product designed specifically and exclusively for
editing will generally do a better job, and MLEs tend to be sluggish
when loading anything larger than about 58K.
:p.
Suggestions&colon. EPM (which comes with OS/2), or QEdit for OS/2 (an
excellent and inexpensive text-mode editor from Semware highly
recommended, and used, by FM/2's author) or Visual Slickedit by
Microedge, PM and more powerful (and, of course, more expensive) than
QEdit, also used by the author.  There are many other editors, freeware,
shareware and shrinkwrap, available, I simply listed those with which I
have some familiarity and feel I can recommend as very good software.
:p.
There's :link reftype=hd res=99300.another, faster internal viewer (no
editor):elink. which is used as the default for viewing in FM/2.
:p.
The internal viewer/editor creates a window for each file being viewed/
edited.  The :link reftype=hd res=91100.Windows->Dialog:elink. dialog
can be used to quickly close several windows at once or find a
particular window and bring it to the front.
:p.
:hp7.Note:ehp7. that when saving files the editor formats the file so
that it appears as it does in the MLE.  The appearance of a file can be
different in the MLE or in the created disk file depending on various
settings under the editor's :hp1.Config->Format Control:ehp1., notably
Wrap.  :hp8.Be sure you have these settings right for the way you want
the resultant file to look.:ehp8.
:p.
When the viewer/editor is in readonly (viewing) mode, several menu items
are disabled to prevent you from changing the file by accident.
:p.
See also&colon.
.br
:link reftype=hd res=96500.Codepages:elink.
.br
:link reftype=hd res=98800.Hex dumps:elink.

:h2 res=96500 name=PANEL_CODEPAGE.Codepages
:i1 id=aboutCodePage.Codepages

FM/2 will allow you to change the codepage (character set) in use in the
internal viewer by selecting a codepage from the listbox.  The codepage
must be one of those supported in your CONFIG.SYS (see :link
reftype=launch object='CMD.EXE' data='/C HELP CODEPAGE'.CODEPAGE:elink.
in the online OS/2 command reference) or codepage 1004.

:h2 res=98800 name=PANEL_HEXDUMPS.Hex Dumps
:i1 id=aboutHexDumps.Hex Dumps

Hex dumps show two hexadecimal digits (0-9 and a-f represent 0 to 15
decimal in hexadecimal (base 16) representation) for each byte of data
followed by the actual data (some data may be unrepresentable in the
current control and therefore displayed as a period)&colon..
:p.
:xmp.
0000  0a 0d 46 4d 2f 32 0a 0d                          ..FM/2..
:exmp.
.br
This is a common method for representing binary data (as opposed to text,
or ASCII, data) for human viewing.

:h2 res=99500 name=PANEL_CENCODING.C-style \encoding
:i1 id=aboutEncoding.C-style \encoding

In many areas, FM/2 allows you to use C-style backslash encoding (or
more properly "escaping") to give constants you normally couldn't enter
into an entry field or MLE.  There are some differences from standard
C escaping, so pay attention.
:p.
The following escapes are permissible&colon.
.br
:parml compact tsize=8 break=none.
:pt.\\
:pd.single backslash character
:pt.\r
:pd.carriage return (ASCII 13)
:pt.\n
:pd.linefeed  (ASCII 10)
:pt.\t
:pd.tab (ASCII 9)
:pt.\b
:pd.backspace (ASCII 8)
:pt.\a
:pd.bell (ASCII 7)
:pt.\f
:pd.formfeed (ASCII 12)
:pt.\'
:pd.'
:pt.\"
:pd."
:pt.\27
:pd.escape character (ASCII 27; this is decimal encoding)
:pt.\x1b
:pd.escape character (ASCII 27; this is hexadecimal encoding)
:eparml.
:p.
Therefore, "This\x20is\32a test of \\FM2\\SETENV.\r\n"
.br
becomes "This is a test of \FM2\SETENV." (followed by a carriage return and linefeed).
:p.
A :link reftype=hd res=98800.hex dump:elink. of the above after
conversion&colon.
:xmp.
00000000  54 68 69 73 20 69 73 20 61 20 74 65 73 74 20 6f  This is a test o
00000010  66 20 5c 46 4d 32 5c 53 45 54 45 4e 56 2e 0d 0a  f \FM2\SETENV...
:exmp.
