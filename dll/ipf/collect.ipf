:h2 res=90100 name=PANEL_COLLECTOR.Collector
:i1 id=aboutCollector.Collector
:artwork name='..\..\bitmaps\collect.bmp' align=center.
:p.
The :hp1.Collector:ehp1. is a temporary place to hold objects that you
want to manipulate later; it allows you to temporarily group objects
regardless of where they're physically stored in the file system. No
physical (disk) storage is used; the Collector just holds the objects
(something like WPS shadows) until you're ready to do something with
them. You might think of it as an additional clipboard containing names
of file system objects. Note that objects in the Collector, unlike
objects in main tree and directory containers, show their full
pathnames.
:p.
You can drag file system objects from and to the Collector. Be careful
where you drop the objects; directories and files already in the
Collector are "targets." If you drop on a directory, the files are moved
or copied to the directory, not into the Collector.
:p.
The Collector allows you to manipulate the files it contains just as you
would in a main tree or directory window. Context menus are available
just as they are in a main window. The context menu for the collector
container obtained by clicking mouse :color fc=default bc=cyan.B2:color fc=default bc=default. over container whitespace allows clearing the
container, collecting files from the clipboard (a good way to import a
selection from some other program that might save a list of files, one
file per line), and collecting from a list file (a file containing
fully qualified pathnames, one per line).
:p.
Additionally, the pulldown menu gives you access to a :link reftype=hd
res=91600.Seek and scan:elink. function. This leads to a dialog that
lets you search for and Collect files based on filemasks and text
content.

:h2 res=91600 name=PANEL_GREP.Seek and scan
:i1 id=aboutGrep.Seek and scan
:i1 id=aboutGrepDupe.Find duplicate files
This dialog, accessed from the Utilities menu, a Collector context menu,
a Drives context menu or directories context menu allows you to
search for files (by filemasks and, optionally, text within the files)
and :link reftype=hd res=90100.Collects:elink. the files found. When
selected from the drives or directories menu the cursored drive or
directory is inserted as the root for the search.
:p.
:hp1.In a hurry?  See the :ehp1.:hp9.quicky:ehp9.:hp1. instructions at
the bottom.:ehp1.
:p.
In the :hp6.Filemasks:ehp6. entry field you can enter one or several
filemasks. To enter multiple masks, separate them with semi-colons.
Entries can total up to 8095 bytes in length. You can use exclusion
masks as well by prefacing that portion of the mask with a forward slash
(/), which can be used to speed up the search by excluding paths or
filemasks that you are not interested in finding. Example&colon.
"C&colon.\*;/C&colon.\OS2\*;/C&colon.\DESKTOP;D&colon.\*;/*.DLL".
:p.
The :hp6.Add:ehp6. and :hp6.Delete:ehp6. buttons can be used to add the
current filemask to the listbox for later retrieval or delete a line
from the listbox if you want to get rid of it. The :hp6.Append:ehp6.
checkbox allows you to control whether selected masks replace what is
in the Filemasks entry field, or are appended to the current contents.
Note that if Append is checked, you must double-click (or press :color fc=default bc=palegray.
Enter:color fc=default bc=default.) to append the mask; otherwise, it is only necessary to highlight the
desired mask in the listbox.
:p.
The :hp6.Walk:ehp6. button brings up the :link reftype=hd res=91500.Walk
Directories:elink. dialog. When you select a directory in the dialog
it's added to the Filemasks entry field.
:p.
The :hp6.Env:ehp6. button allows you to enter the name of an environment
variable which points to a list of directories (like the PATH variable)
and fills them into the Filemasks entry field.
:p.
In the :hp6.Search text:ehp6. MLE you can enter text that must be found
for the file to match. All files matching the filemask(s) are searched
for this text. If no text is entered, a simple file find is performed.
Note that each line of the MLE is a separate search string. If any of
the strings are found, FM/2 will consider a match to be made. You can
enter up to 4096 characters in this MLE, and as many lines as you like
within that limitation. Note that a "line break," for our purposes
here, is created when you press :color fc=default bc=palegray. Enter:color fc=default bc=default. in the MLE; word wrap is not
a consideration.
:p.
Simple regular expressions are supported. These can be briefly
summarized&colon.
:parml compact tsize=11 break=none.
:pt.'*'
:pd.matches any string
:pt.'?'
:pd.matches any single character
:pt.'['XYZ']'
:pd.matches any of X, Y or Z
:pt.' '
:pd.matches 0 or more whitespace characters
:pt.'\\'
:pd.'escapes' the next character
:pt.C
:pd.matches C
:eparml.
:p.
The :hp6.Include Subdirs:ehp6. checkbox controls whether the search
extends into subdirectories. If the box is checked, subdirectories are
searched.
:p.
The :hp6.Absolute:ehp6. checkbox disables regular expressions in the
:hp6.Search text:ehp6. entry field.
:p.
The :hp6.Case Sensitive:ehp6. checkbox, if checked, makes text searches
case sensitive. Otherwise they are not ('c' matches 'C' and 'c').
:p.
The :hp6.Say files as found:ehp6. checkbox tells FM/2 to display the
filenames it finds based on the filemasks as it encounters them, if
checked.
:p.
The :hp6.Search files:ehp6. checkbox, when checked, tells FM/2 to look
inside files for the text in the Search text entry field. This has no
effect if no search text was entered.
:p.
The :hp6.Search EAs:ehp6. checkbox, when checked, tells FM/2 to look at
the text EAs of files for the text in the Search text entry field. This
has no effect if no search text was entered.
:p.
The :hp6.Find duplicates:ehp6. checkbox, when checked, tells FM/2 to
find :hp2.potential:ehp2. duplicate files. Files with the same name are
found, as are files with the same size and date/time.
:hp9.Note&colon.:ehp9. Finding duplicates is a time and resource
consuming operation due to the usually large number of files, and
therefore comparisons, involved (FM/2 on a P3/500 with 256 mb RAM
required about 4 minutes to search 12,000 files for dupes). Once
potential dupe files have been found and Collected you can use the
Collector's sort and information facilities and commands to check the
files and delete or archive any you don't want to keep around -- use
filename sort to see files grouped with the same name, size sort to see
files grouped with the same size and date/time. The three switches listed
below also affect how potential duplicate finding works.
:p.
The :hp6.CRC dupes:ehp6. checkbox, when checked, causes FM/2 to
determine and check the CRCs of potential duplicate files. Only files
with matching CRCs will then pass the dupe test begun as described
above. This approximately doubles the time it takes to find potential
duplicate files, depending on how many potential dupes there are, but
not using it means that invariably a few files will be flagged as
possible dupes that aren't (this is possible even with this switch on,
but far less likely). Note that in order to CRC a file, FM/2 must be
able to open it -- if it can't, FM/2 will consider the two files being
compared to be duplicates.
:p.
The :hp6.No size dupes:ehp6. checkbox, when checked, tells FM/2 to not
consider files as potential dupes unless their names match (size/date
matches aren't considered). Be aware that you will probably miss true
duplicate files if you turn this off, as not all dupes have the same
name.
:p.
The :hp6.Ignore extensions:ehp6. checkbox, when checked, tells FM/2 to
ignore the (last) extension on filenames when checking for dupes by
name. This is handy when you're comparing files in BBS upload
directories where the same file may have been archived with different
archivers, resulting in identical rootnames but different extensions.
:p.
The :hp6.Larger:ehp6. entry field can be used to find files larger than
the number of bytes input (zero means all files). The :hp6.k:ehp6.
button next to the entry field multiplies the value by 1024 for you to
make kilobytes instead of bytes. The :hp6.Smaller:ehp6. entry field
works the same except that it causes files smaller than the number of
bytes input to be found. When used together (both fields are nonzero),
files found will be greater than the Larger field's value or less than
the Smaller field's value.
:p.
The :hp6.Newer:ehp6. and :hp6.Older:ehp6. entry fields work similarly.
When non-zero, these fields cause the search to find only files newer
or older than the number of days entered. The :hp6.m:ehp6. buttons
multiply the value by 30 for you to make "months" instead of days.
:p.
The :hp6.AllHDs:ehp6. button prompts you for a single simple filemask
(one without a drive or path) then builds a :hp6.Filemasks:ehp6. string
that will search all hard drives for that mask. Similarly, the
:hp6.LocalHDs:ehp6. builds a mask for all local hard drives, and
:hp6.RemoteHDs:ehp6. builds a mask for all remote (LAN) hard drives.
:p.
The listbox on the left top of the dialog contains a list of valid
drives. You can double-click one of these drives to add a mask for
that drive to the entry field.
:p.
The file search is performed using the current Collector Filter's
attribute values.
:p.
Click :hp6.Okay:ehp6. when ready to search, or :hp6.Cancel:ehp6. to exit
without searching. Note that this function is designed to run in the
background while you're doing other work, rather than running full-tilt
and making you wait for it.
:p.
:hp9.Quicky instructions for file finding:ehp9.&colon. type a mask into
the :hp6.Filemasks:ehp6. entry field (for example, "C&colon.\*.BAK") and
press :color fc=default bc=palegray.Enter:color fc=default bc=default..
:p.
:hp9.Quicky instructions for dupe finding:ehp9.&colon. type a mask into
the :hp6.Filemasks:ehp6. entry field, check :hp6.Find Duplicates:ehp6.,
check :hp6.CRC dupes:ehp6. if you want greater accuracy (and more time
spent looking), and press :color fc=default bc=palegray.Enter:color fc=default bc=default..
:p.
See also :link reftype=hd res=98500.See all files:elink..

:h3 res=100050 name=PANEL_ENV.Enter environment variable name
:i1 id=aboutEnterEnvironment.Enter environment variable name

This dialog, reached from the :link reftype=hd res=91600.Seek and Scan
files dialog:elink., allows you to enter or select the name of an
environment variable (like PATH, LIBPATH, DPATH, etc.) that points to a
list of directories separated by semi-colons. The Seek and Scan dialog
will then build the filemask for you from that list of directories.
:p.
Some of the most frequently encountered environment variable names are
displayed in the listbox. If you select one, its name is placed in the
entry field for you. You can also type in a name. Click :hp1.Okay:ehp1.
when you have entered the desired name, or click :hp1.Cancel:ehp1. if
you changed your mind and don't wish to use an environment variable
name.

:h2 res=98500 name=PANEL_SEEALL.See all files
:i1 id=aboutSeeAllFiles.See all files

FM/2 can present a window listing all the files on one or more drives.
This command is part of the :link reftype=hd res=90100.Collector:elink.,
and can be accessed from the Utilities menu or the Collector's context menu.
Selecting :hp1.Miscellaneous->Show allfiles:ehp1. (:color fc=default bc=palegray.Ctrl:color fc=default bc=default.
 + :color fc=default bc=palegray.S:color fc=default bc=default.) from a Drive
Tree directory's context menu will also get you here.
:p.
After you select the drives to scan from a dialog, FM/2 will scan the
selected drives and then fill a custom control list with the names of
all the files found.
:p.
Once the filenames are displayed, together with their sizes, attributes
and last written dates and times, you can perform a variety of actions
with selected files from the list. Commands exist to sort the list
in several ways and to filter and select files in several ways.
:p.
You can also drag selected files from the list.
:p.
You can use the Filter to pare a list down to a smaller subset. For
example, if you are viewing all the files on the C&colon. drive, you can
limit the view to all the files in C&colon.\OS2 and its subdirectories with
the filter "C&colon.\OS2\*".
:p.
FM/2 displays Hidden and System files in red, Readonly files in blue,
and all others in black on a light gray background. Selected files are
displayed in reverse video (white text on a black background). These
colors are configurable. The current file has a ">" pointing at it in
the left margin. Single selections and swipe selections are supported,
as well as select and deselect all, and select and deselect based on
masks and/or attributes.
:p.
Keyboard selection is performed with the :color fc=default bc=palegray.spacebar:color fc=default bc=default.
, or you can hold down the :color fc=default bc=palegray.Shift:color fc=default bc=default.  key while
moving the cursored selection with the  :color fc=default bc=palegray.arrow:color fc=default bc=default. keys.
You can press the first letter of a file (as displayed; if fullnames are
on, you must type the first letter of the full filename, including path)
to "find" a file that begins with that letter. In fact, in this window,
you can type more than one letter of a filename (each within one, count
'em, one, second of the previous) to narrow the search further -- something
that standard OS/2 controls don't support.
:p.
Double-clicking a file results in a default action as in Directory
Containers (usually a view of the file). If you need a refresher, check
the :link reftype=hd res=90000.General Help:elink. topic. The keyboard
equivalent is the :color fc=default bc=palegray.Enter:color fc=default bc=default. key.
:p.
A large subset of the commands available in Directory Containers is
available in this window. See the :link reftype=hd res=93700.Context
menus:elink. topic for more information. Commands in this window
always apply to selected files.
:p.
You'll find an additional option to the usual Copy and Move commands
here, in the conditional cascades for those commands&colon. :hp6.Copy
and preserve...:ehp6. and :hp6.Move and preserve...:ehp6.. These
commands copy or move the selected files but preserve the directory
relationship of the files. The effect of this can be non-obvious, so
use with care.
:p.
Let's say you select three files&colon. G&colon.\FOO\BAR\DUDE,
G&colon.\FOO\BAR\WOW\DUDE and G&colon.\FOO\BAR\RUFF\DUDE. If you select
:hp1.Copy and preserve:ehp1. and pick a destination directory of
H&colon.\HERE, the resultant files will be H&colon.\HERE\DUDE,
H&colon.\HERE\WOW\DUDE and H&colon.\HERE\RUFF\DUDE.
:p.
Note that drives are not considered when preserving directory
relationships, so if one of our three files above resided on drive
F&colon., the results would be the same.
:p.
The :hp6.Duplicates...:ehp6. command can be used to find files that are
potential duplicates in the window. There are several options that you
can select in the :link reftype=hd res=100035.dialog:elink. that
appears. Duplicate finding can take a considerable amount of time, but
you can continue to do things in other windows while you wait.
:p.
See also&colon. :link reftype=hd res=91600.Seek and Scan:elink.

:h3 res=98600 name=PANEL_DRVSWND.Pick drives

Highlight the drive(s) to list, then click Okay. Click Cancel to abort.
:p.
FM/2 will quickly load all the files on the selected drives into an ugly
but fast custom control list for you to further examine and manipulate.
:p.
The custom control is used rather than a container due to the slowness
of containers when dealing with such large numbers of records. You can
verify this by using :link reftype=hd res=91600.Seek and scan:elink. to
Collect the same files into the Collector container, if you're a
masochist.

:h3 res=100035 name=PANEL_DUPES.Duplicate finding options

Select from the options for finding duplicate files. Files must meet
:hp1.all:ehp1. selected criteria to be considered duplicates (dupes).
Therefore, the more options you check, the more likely the files you're
presented with when FM/2's finished actually :hp1.are:ehp1. duplicate
files.
:p.
Note that CRCing files can take considerable time. Only files that
match all other criteria will be CRCed, and files will only be CRCed
once (i.e. the logic's not :hp1.totally:ehp1. stupid), but it will take
longer. FM/2 will display some info about where it is during the dupe
check, and checking is done in a background thread at a low priority so
that other windows and applications remain usable. FM/2 must be able
to open the file for reading to CRC it -- if it cannot do so, it will
consider the file a match based on the other criteria specified.
:p.
After finding duplicate files, you can "restore" the rest of the files
by using the :hp6.Filter:ehp6. command. It often helps to set the
Sort to Name or Size when viewing files found as potential duplicates
to best see their relationships.
:p.
See also :link reftype=hd res=91600.Seek and scan:elink..
