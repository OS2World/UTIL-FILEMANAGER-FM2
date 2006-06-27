:h2 res=99985 name=PANEL_PRINTER.Print files
:i1 id=aboutPrinter.Print files
:artwork name='..\..\bitmaps\print.bmp' align=center.
:p.
This dialog, which appears before a selected group of one or more files
is to be printed, lets you finalize your selection and set the parameters
to be used when printing occurs.
:p.
The listbox at the top of the dialog contains the files that are to be
printed. You can unhighlight (deselect) any files that you do
:hp1.not:ehp1. want to print. :hp6.Hint:ehp6.&colon. you might need to
hold down the :color fc=default bc=palegray.Ctrl:color fc=default bc=default. key while clicking to unhighlight the first file.
:p.
At the bottom of the dialog you can tell FM/2 whether to use
:hp1.formatted printing:ehp1., which performs pagination for you, or to
print the text file raw (it should already be paginated). You can also
specify the printer to which output should go (default is PRN, the
default system printer, but you could specify LPT1, LPT2, etc., or even
a text file). Note that FM/2's printing is really meant for situations
in which the PM printer objects and spooler aren't available -- you can
just drag objects to the printer objects otherwise.
:p.
The rest of the controls are used to determine how FM/2 should format
the text file for printing. You can specify the width and length of the
page (in columns and rows respectively), how many lines to leave blank
for bottom and top margins, how many characters to leave blank for left
and right margins, whether to print pages sequentially or print first
odd pages, then even pages (:hp1.Alternate pages:ehp1.). You can specify
the line spacing used (1 for single-spaced, 2 for double-spaced, etc.),
and tell FM/2 if it needs to send a formfeed before and/or after each
file it prints.
:p.
Click :hp1.Okay:ehp1. when you're ready to print. Click :hp1.Cancel:ehp1.
if you changed your mind and don't want to print anything.
:p.
:hp6.Notes&colon.:ehp6.
:p.
Not all files will print well with formatted printing turned on. The
file should :hp1.not:ehp1. contain any control codes aside from carriage
returns and linefeeds. The file's lines should be short enough to fit
within the confines of width - (left margin + right margin). For files
that were formatted to be viewed on-screen, this may mean setting your
printer to use a narrower typeface and increasing the width parameter in
the FM/2 print setup dialog above 80. Refer to your printer
documentation for how to change the default font.
:p.
When printing :hp1.Alternate pages:ehp1., FM/2 reverses the left and
right margins when printing even numbered pages. This is designed to
let you print front and back (page 2 on the back of page 1, etc.) and
then bind the result along the left side (left of page 1, right of page
2, etc.). FM/2 will first print the odd numbered pages, beginning with
page 1, then prompt you before beginning to print the even numbered
pages, beginning with page 2, thereby allowing you to reload the printer
so as to print on the back sides of the already printed pages.
