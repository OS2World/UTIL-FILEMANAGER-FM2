.***********************************************************************
.*
.* $Id$
.*
.* Edit archiver definitions
.*
.* Copyright (c) 1993-98 M. Kimes
.* Copyright (c) 2004 Steven H.Levine
.*
.* Revisions	31 Jul 04 SHL - Rework file name position description
.*
.***********************************************************************
.*
:h1 res=90200 name=PANEL_ARCLIST.Archive Container
:i1 id=aboutArchiveListing.Archive Container
Archive Container windows presents you with a list of an archive's
contents.  The menus available here present you with commands which you
can perform on selected files and the archive as a whole, as well as the
usual configuration of the window's appearance.  You get here by double-
clicking an archive file in an FM/2 Directory Container window.
:p.
Archive Container windows have their own sorting method, accessible via
a context menu requested over whitespace (the Views menu).
:p.
Besides the container showing the archive contents and a couple of
informational fields above that, there's an entry field at the bottom of
the window that shows the current :hp1.extract directory:ehp1..  This is
the directory in which any extracted files will be placed.  (Extraction
refers to copying files from the archive onto your disk as normal
files.)  To change the extract directory, enter a new directory into the
entry field (if it doesn't already exist you'll get an option to create
it), drag a directory onto the entry field, or click the folder button
with B1.
:p.
You can drag files onto the Archive Container's listing to add them to
the archive, and you can drag files from the archive to an FM/2
directory window.  OS/2's drag and drop "rendering" mechanism would make
this painfully slow for any other application's window, as each file is
processed individually (imagine extracting each file in an archive by
typing a separate command line for each to get an idea how slow it would
be).  The menu extract items allow optimizing extraction so that only
one "pass" needs to be done (note that :hp1.Files->Extract:ehp1. must
also deal with the OS/2 command line length restriction of 1024 bytes,
so if you want to extract all the files in a large archive,
:hp1.Files->Extract All:ehp1. is a superior choice), and the internal
drag to an FM/2 Directory Container window ... well, cheats, to get
around it.
:p.
You can get a popup menu in the container by pressing B2.
:p.
Following is a discussion of the pull-down menus:
:p.
:hp2.Files Menu:ehp2.
:p.
:hp1.View:ehp1. causes FM/2 to extract the file(s) to a temporary
directory and display it/them.  If, for some reason, the files don't
appear when you attempt to view them, try :hp1.Test:ehp1.ing the archive.
:p.
:hp1.Edit:ehp1. works as above but the file(s) are loaded into the
configured editor instead of being viewed.  File(s) can then be
:hp1.Refreshed:ehp1. back into the archive.  :hp2.Note&colon.:ehp2.
Do not attempt to Refresh files that were stored with pathnames.
The pathname will either be lost or an additional file without
pathname will be stored (depending on the :link reftype=hd
res=10015.archiver:elink.'s behavior).
:p.
:hp1.Extract:ehp1. causes FM/2 to extract the selected file(s) to the
extract directory.
:p.
:hp1.Extract w/ Dirs:ehp1. causes FM/2 to extract the selected file(s)
to the extract directory in such a way that, if directories have been
included with the filenames, the directories are recreated.
:p.
:hp1.Delete:ehp1. causes FM/2 to delete the selected file(s) from the
archive.
:p.
:hp1.Exec:ehp1. causes FM/2 to extract all selected files to a temporary
directory and then runs the cursored file.  If you pick this from a
popup menu, that would be the file under the mouse pointer when you
requested the popup.  This allows you to select DLLs, help files, data
files, etc. required to get the application to run correctly as well as
the executable file.
:p.
:hp1.Print:ehp1. causes FM/2 to extract and print selected files.  This
uses the FM/2 printing method, not the OS/2 printer objects (see
Config->Printer in an FM/2 main window's pulldown menu).
:p.
:hp1.Find:ehp1. causes FM/2 to scan the extract directory for any files
matching the names of the selected files within the archive and Collect
them if found.
:p.
:hp1.Virus Scan:ehp1. causes FM/2 to extract the selected files and then
run the configured virus checker.  See the :hp1.:link reftype=hd
res=92200.internal Settings notebook's Files/Dirs page:elink.:ehp1..
:p.
:hp1.Extract All:ehp1. extracts all files from the archive to the
extract directory.  :hp1.Extract All &amp. Exit:ehp1. does the
same thing but closes the archive listing window after starting the
extraction.
:p.
:hp1.Extract All w/ Dirs:ehp1. does the same thing including any
enclosed directories (i.e. files are extracted into the directories they
were archived "with," if any, rather than all going into the extract
directory). :hp1.Extract All w/ Dirs &amp. Exit:ehp1. does the same
thing but closes the archive listing window after starting the
extraction.  (:hp2.Note:ehp2. that if all you want to do to an archive
is extract from it, you can do so without ever opening a contents box;
just pull up a context menu on the archive in an FM/2 main window and
select :hp1.Extract:ehp1..  This is the fastest and most efficient
method of extracting files from an archive.)
:p.
:hp1.Test:ehp1. tests the archive's integrity.
:p.
See also&colon.
.br
:link reftype=hd res=94200.Editing Archiver Details:elink.

:h2 res=10015 name=PANEL_ARCHIVERS.Archivers
:i1 id=aboutArchivers.Archivers

:hp2.Archivers:ehp2. are programs that create files composed of
(usually) compressed data that represents, and allows recreation of,
normal (uncompressed) files.  These archiver programs are widely used to
create archive files for downloading from BBSs, information services and
the Internet, and to extract from those archive files once downloaded.
They're also used to create archives locally for backup purposes, as the
files thus created are smaller than the original files and contain many
other files within them.
:p.
Following is a partial list of OS/2 archivers available at the time of
this writing&colon.
:p.
:parml compact tsize=12 break=none.
:pt.Extension
:pd.Name of archiver
:pt.
:pd.
:pt.&period.ZIP
:pd.Zip and Unzip
:pt.&period.LZH
:pd.LH
:pt.&period.ZOO
:pd.Zoo
:pt.&period.RAR
:pd.RAR
:pt.&period.ARJ
:pd.UnArj
:pt.&period.ARC
:pd.Arc
:eparml.
:p.
You normally find these archivers with names like ZIP*.EXE or LH*.EXE,
where the * will be a number indicating the version of the program.
These are self-extracting archives (archives that extract themselves
when you run the .EXE) so you don't get a chicken-or-egg scenario.  You
can find them on BBSs, information services and the Internet -- in other
words, you can find them the same places where archive files are most
often used.
:p.
FM/2 can work with DOS archivers, but they aren't supported.  If you
decide you want to use your DOS archivers rather than OS/2 native
archive programs, you'll have to figure it out on your own (see
:link reftype=hd res=94200.Editing Archiver Details:elink. topic and
the ARCHIVER.BB2 datafile that came with FM/2).

:h2 res=94200 name=PANEL_AD_FRAME.Editing Archiver Details
:i1 id=aboutEditArc.Editing Archiver Details
:p.
This dialog box, which you can reach from the Config menu, allows you to
edit the details of an :link reftype=hd res=10015.archiver:elink.. Entry
boxes are present for all the twenty one fields represented in
ARCHIVER.BB2 (the text file that contains control information about your
archivers which FM/2 uses to interface with the archivers).  It's
probably easier for most people to edit ARCHIVER.BB2 directly with a
text editor.
:p.
:hp1.NOTE&colon.:ehp1.  The simplest method to ensure that your archivers
work properly with FM/2 is to make sure they're in a directory named in
your PATH= statement, and check the names of the files to make sure they
match what's on your system (i.e. UNZIP.EXE in both ARCHIVER.BB2 and on
your hard disk, not UNZIP.EXE in one and UNZIP32.EXE in another).
:p.
In the event that you attempt to list an archive and FM/2 feels you've
probably bungled the entry in ARCHIVER.BB2, you'll be given an
opportunity to use this dialog to fix the entry.  In this case, you'll
see the listbox at the right of the dialog filled with the listing of
the archive that your archiver made.  You can highlight a line and click
the << button next to the Start List or End List fields to move the line
to that entry field (these are the most common mistakes, and FM/2 cannot
find any files if the Start List string is wrong).  You can double-click
on a listbox line to have FM/2 "parse" it into the Fld# text boxes for
you, to make it easier to judge field positions for sizes, dates, and
filenames.  The filename field in particular is extremely important.  If
it's too high, FM/2 finds no files.  If it's "in range" but wrong, FM/2 gets
the wrong fields for filenames.
:p.
You may still need to refer to your archiver's documentation, or run it
to get the help on its command syntax.  FM/2 can't do everything for you,
but it holds your hand as best it can.
:p.
Refer to the ARCHIVER.BB2 file that came with FM/2 for additional
information and an example.
:p.
When you've completed editing the archiver's details, click Okay.  FM/2
will ask you if you want to rewrite ARCHIVER.BB2 (be sure you save the
original copy for its complete notes; FM/2 will back it up one version to
ARCHIVER.BAK).  If you don't rewrite ARCHIVER.BB2, changes are good only
for the current session (handy for testing).
:p.
You can also get to this dialog box from Select Files' Config submenu.
:p.
See also&colon.
:p.
:link reftype=hd res=94300.Archiver Details Fields:elink.

:h3 res=94300 name=PANEL_ARCFLDS.Archiver Details Fields
:i1 id=aboutArcFlds.Archiver Details Fields
:p.
:hp2.ID:ehp2.  This field contains an ID for the archiver; something for
human consumption.  It's a good idea to include the version number of the
archiver for reference.  An example might be "LHArc 2.11".
:p.
:hp2.Add:ehp2.  This field should contain the command that creates
and adds files to an archive.  An example might be "PKZIP.EXE -a" (NOTE:
This example assumes the file is on your PATH (see PATH= in CONFIG.SYS).
If it's not, you'd need to give a full pathname, like
"C&colon.\UTILS\PKZIP.EXE -a".) Note that commands should include the
extension so that FM/2 can check them without guessing.  Above this
field is a button labeled "See." If clicked, the program named in this
field will be run (you'll be given the opportunity to add arguments to
the command line) in another window. This lets you check command syntax
and archiver version, as well as assuring that FM/2 can find your archiver
executables.
:p.
:hp2.Move:ehp2.  This field should contain the command that moves files
to the archive (adds then deletes the file).  An example might be
"ARC.EXE mwn".
:p.
:hp2.Extension:ehp2.  This field contains the extension normally
associated with files created by this archiver.  An example might be
"ZOO" for files created by the Zoo archiver.
:p.
:hp2.Extract:ehp2.  This field contains the command that extracts files
from the archive.  This command should not delete the files from the
archive when it extracts them, and *must* be present for FM/2 to show you
a member of the archive (commands other than Extract and List may be
left blank if necessary).  An example might be "PKUNZIP.EXE -o".  Note
the "-o" option given; this tells PKUNZIP to automatically overwrite any
existing files (FM/2 will check to see if any of the files exist and warn
you if so).  It's important to always include your archiver's "don't
stop for user input" option; some things occur as detached processes and
you can't interact with them; the program would be hung, which is
uncool. Above this field is a button labeled "See." If clicked, the
program named in this field will be run (you'll be given the opportunity
to add arguments to the command line) in another window. This lets you
check command syntax and archiver version.
:p.
:hp2.Extract w/Dirs:ehp2.  This field contains the command that extracts
files from the archive and places them into directories embedded in the
archive.  An example might be "LH.EXE x /o /s".
:p.
:hp2.Signature:ehp2.  This field contains the signature for the archive
type.  There is usually a byte or few in a particular place in any
archive that indicates that it is, indeed, an archive of that type.  FM/2
uses these signatures to "sniff out" which archiver is used to
manipulate the archive.  Since these signatures sometimes contain
characters which are "unprintable," you can use \x<hexnum> to represent
any "strange" characters.  A side effect of this is that two backslashes
are required to represent a single backslash ("\\" == "\").  See
:link reftype=hd res=99500.C-style encoding:elink. for more information.
:p.
To determine what an archiver's signature is, either ask the archiver's
author or check several different archives of the type for one or more
bytes present in each at the same location, usually near the beginning
of the file.
:p.
This field must be entered and valid for FM/2 to detect this type of
archive (see also Sig(nature) Pos(ition)).
:p.
:hp2.List:ehp2.  This field contains the command to list the archive's
contents.  This command *must* be present and correct for FM/2 to work
properly with this type of archive.  An example might be "ZOO.EXE v".
:p.
:hp2.Test:ehp2.  This field contains the command to test the archive's
integrity.  An example might be "PKUNZIP.EXE -t".
:p.
:hp2.Add with paths:ehp2.  This field contains the command to add files
to the archiver with their paths (i.e. \FM3\FM3.EXE instead of just
FM3.EXE).  This can be omitted if the archiver doesn't support the
command.  An example might be "ZIP.EXE" (Zip defaults to adding paths).
:p.
:hp2.Move with paths:ehp2.  As above, but moves the files instead of
just adding them.  An example might be "PKZIP -m -P".
:p.
:hp2.Add and recurse:ehp2.  Adds files to the archive, with paths, and
recurses into subdirectories.  An example might be "LH a /s".
:p.
:hp2.Delete:ehp2.  This field contains the command to delete files from
the archive.  An example might be "LH.EXE /o /d".
:p.
:hp2.Sig(nature) Pos(ition):ehp2.  This field contains a number
indicating how many bytes into the file the signature is located.  If
this number is negative, FM/2 looks from the end of the file instead of
the beginning.
:p.
:hp2.F(ile)Name Pos(ition):ehp2.  This field tells FM/2 which field on
the line of an archive listing is the file name.
Archive listing fields are numbered from 0.
The file name position item consists of 4 subfields separated by commas.
All subfields must be present and correct for FM/2 to get the right
file names from the archive listing.
The first is the field number.
The second is a flag which is set to 1 to indicate that the file
name is the last field on the line.
This allows unquoted archive member names to contain spaces
The third is a flag which is set to 1 to indicate that the file name starts
at the second character of the field.
This allows files that are surrounded by unusual bracketing characters.
The fourth is a flag which is set to 1 to indicate the the file name
stands alone at the first field in the listing line and
the file details are on the next line.
:p.
To understand what "field on the line of an archive listing" means,
think of a text line as being broken up into tokens, or words, separated
by spaces.  These tokens, or words, are fields.  Therefore,
:p.
I like Ike.
:p.
contains three fields.  Field 0 is "I", field 1 is "like", and field 3
is "Ike."  Think of it like this&colon.
.br
:xmp.
 +--------------------+
 |  0   |  1   |  2   | Field Numbers
 +------+------+------+
 |  I   | like | Ike  | Field Contents
 +------+------+------+
:exmp.
:p.
:hp2.OldS(i)z(e) Pos(ition):ehp2.  This field tells FM/2 which field on
the line of an archive listing is the old (uncompressed) size of the
file.  If this isn't available or you don't care about it, you can enter
a -1 to disable detection of this field entirely.
:p.
:hp2.NewS(i)z(e) Pos(ition):ehp2.  This field tells FM/2 which field on
the line of an archive listing is the new (compressed) size of the file.
If this isn't available or you don't care about it, you can enter a -1
to disable detection of this field entirely.
:p.
:hp2.Date Pos(ition):ehp2.  This field tells FM/2 which field on
the line of an archive listing is where the time/datestring is.  If this
isn't available or you don't care about it, you can enter a -1 to
disable detection of this field entirely.  You can optionally follow this
number with a comma and another number that indicates the type of the
date from any of the formats in the following list&colon.
.br
:xmp.
 1.  02-08-96  23&colon.55&colon.32
 2.   8 Feb 96 23&colon.55&colon.32
 3.   8 Feb 96  11&colon.55p
 4.  96-02-08 23&colon.55&colon.32
:exmp.
:p.
:hp2.NumDateF(ie)lds:ehp2.  This field tells FM/2 how many fields comprise
the time/datestring.
:p.
:hp2.Start-of-list:ehp2.  The line that comes just before the list of
files in the archiver listing (see example below).  You can use the
:hp2.<<:ehp2. button to insert a selected line directly from the listbox
into this field.
:p.
:hp2.End-of-list:ehp2.  The line that comes just after the list of
files in the archiver listing (see example below).  You can use the
:hp2.<<:ehp2. button to insert a selected line directly from the listbox
into this field.
:p.
Here's an example of an ARC listing (5.12mpl, command "ARC l"; you may
need to widen the help windows for this to look right...):
:p.
:xmp.
Name          Length    Date
============  ========  =========    <--this line is start-of-list
MAKEFILE           374  28 Nov 89
QSORT.C          14279  29 Nov 89
QSORT.EXE        24629  29 Nov 89
STUFF.H            371  29 Nov 89
        ====  ========               <--this line is end-of-list
Total      4     39653
:exmp.
:p.
Note the filename is in position 0, old length in position 1, and the
date starts in position 2, with 3 parts, and there's no new length
field (so it'd be -1).  Compare that to the archiver entry for
ARC 5.12mpl in the "stock" ARCHIVER.BB2 and you should get a feel for
what all those fields mean.

:h2 res=98300 name=PANEL_ARCERR.Archive Errors
:i1 id=aboutArchiveErrors.Archive Errors
Sometimes FM/2 can't get information from the archiver about an archive.
There are a couple of reasons this could happen&colon.  The information
in ARCHIVER.BB2 may be incorrect for the archive type, or the archive may
be damaged.  Possibly the file "smelled" like an archive but wasn't.
Perhaps you selected :hp1.View->As archive:ehp1. on a file that wasn't
actually an archive.
:p.
When this happens, this dialog appears.  You'll be given as much
information as possible, including the text of what the archiver had to
say about the archive when it was asked to list its contents, presented
in an MLE, and allowed four choices&colon.  :link reftype=hd
res=94200.Edit the archiver details:elink., Test the archive (if you
have told FM/2 how to test archives with this archiver in ARCHIVER.BB2),
View the archive, or Cancel the whole thing.
:p.
Generally speaking, first Test the archive.  If the archive is okay or
if the test won't run at all, the problem is most likely in your archive
information record in ARCHIVER.BB2.  You can View the archive to assure
yourself that it is, in fact, an archive, and perhaps hunt down the
signature so you can add it to ARCHIVER.BB2 if it's not a listed archive
type.
