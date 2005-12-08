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
contents. The menus available here present you with commands which you
can perform on selected files and the archive as a whole, as well as the
usual configuration of the window's appearance. You get here by double-
clicking an archive file in an FM/2 Directory Container window.
:p.
Archive Container windows have their own sorting method, accessible via
a context menu requested over whitespace (the Views menu).
:p.
Besides the container showing the archive contents and a couple of
informational fields above that, there's an entry field at the bottom of
the window that shows the current :hp1.extract directory:ehp1.. This is
the directory in which any extracted files will be placed. (Extraction
refers to copying files from the archive onto your disk as normal
files.)  To change the extract directory, enter a new directory into the
entry field (if it doesn't already exist you'll get an option to create
it), drag a directory onto the entry field, or click the folder button
with B1.
:p.
You can drag files onto the Archive Container's listing to add them to
the archive, and you can drag files from the archive to an FM/2
directory window. OS/2's drag and drop "rendering" mechanism would make
this painfully slow for any other application's window, as each file is
processed individually (imagine extracting each file in an archive by
typing a separate command line for each to get an idea how slow it would
be). The menu extract items allows optimizing extraction so that only
one "pass" needs to be done (note that :hp1.Files->Extract:ehp1. must
also deal with the OS/2 command line length restriction of 1024 bytes,
so if you want to extract all the files in a large archive,
:hp1.Files->Extract All:ehp1. is a superior choice), and the internal
drag to an FM/2 Directory Container window ... well, cheats, to get
around it.
:p.
You can get a context menu in the container by pressing B2.
:p.
Following is a discussion of the pull-down menus:
:p.
:hp2.Files Menu:ehp2.
:p.
:hp1.View:ehp1. causes FM/2 to extract the file(s) to a temporary
directory and display it/them. If, for some reason, the files don't
appear when you attempt to view them, try :hp1.Test:ehp1.ing the archive.
:p.
:hp1.Edit:ehp1. works as above but the file(s) are loaded into the
configured editor instead of being viewed. File(s) can then be
:hp1.Refreshed:ehp1. back into the archive. :hp2.Note&colon.:ehp2.
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
directory and then runs the cursored file. If you pick this from a
popup menu, that would be the file under the mouse pointer when you
requested the popup. This allows you to select DLLs, help files, data
files, etc. required to get the application to run correctly as well as
the executable file.
:p.
:hp1.Print:ehp1. causes FM/2 to extract and print selected files. This
uses the FM/2 printing method, not the OS/2 printer objects (see
Config->Printer in an FM/2 main window's pulldown menu).
:p.
:hp1.Find:ehp1. causes FM/2 to scan the extract directory for any files
matching the names of the selected files within the archive and Collect
them if found.
:p.
:hp1.Virus Scan:ehp1. causes FM/2 to extract the selected files and then
run the configured virus checker. See the :hp1.:link reftype=hd
res=92200.internal Settings notebook's Files/Dirs page:elink.:ehp1..
:p.
:hp1.Extract All:ehp1. extracts all files from the archive to the
extract directory. :hp1.Extract All &amp. Exit:ehp1. does the
same thing but closes the archive listing window after starting the
extraction.
:p.
:hp1.Extract All w/ Dirs:ehp1. does the same thing including any
enclosed directories (i.e. files are extracted into the directories they
were archived "with," if any, rather than all going into the extract
directory). :hp1.Extract All w/ Dirs &amp. Exit:ehp1. does the same
thing but closes the archive listing window after starting the
extraction. (:hp2.Note:ehp2. that if all you want to do to an archive
is extract from it, you can do so without ever opening a contents box;
just pull up a context menu on the archive in an FM/2 main window and
select :hp1.Extract:ehp1.. This is the fastest and most efficient
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
normal (uncompressed) files. These archiver programs are widely used to
create archive files for downloading from 
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
:pt.&period.TAR
:pd.Tar
:eparml.
:p.
You normally find these archivers with names like ZIP*.EXE or LH*.EXE,
where the * will be a number indicating the version of the program.
These are self-extracting archives (archives that extract themselves
when you run the .EXE) so you don't get a chicken-or-egg scenario. You
can find them on the Internet (Hobbes) -- in other
words, you can find them the same places where archive files are most
often found.
:p.
FM/2 can work with DOS archivers, but they aren't supported. If you
decide you want to use your DOS archivers rather than OS/2 native
archive programs, you'll have to figure it out on your own (see
:link reftype=hd res=94200.Editing Archiver Details:elink. topic and
the ARCHIVER.BB2 datafile that came with FM/2).


:h2 res=98300 name=PANEL_ARCERR.Archive Errors
:i1 id=aboutArchiveErrors.Archive Errors
Sometimes FM/2 can't get information from the archiver about an archive.
There are a couple of reasons this could happen&colon. The information
in ARCHIVER.BB2 may be incorrect for the archive type, or the archive may
be damaged. Possibly the file "smelled" like an archive but wasn't.
Perhaps you selected :hp1.View->As archive:ehp1. on a file that wasn't
actually an archive.
:p.
When this happens, this dialog appears. You'll be given as much
information as possible, including the text of what the archiver had to
say about the archive when it was asked to list its contents, presented
in an MLE, and allowed four choices&colon. :link reftype=hd
res=94200.Edit the archiver details:elink., Test the archive (if you
have told FM/2 how to test archives with this archiver in ARCHIVER.BB2),
View the archive, or Cancel the whole thing.
:p.
Generally speaking, first Test the archive. If the archive is okay or
if the test won't run at all, the problem is most likely in your archive
information record in ARCHIVER.BB2. You can View the archive to assure
yourself that it is, in fact, an archive, and perhaps hunt down the
signature so you can add it to ARCHIVER.BB2 if it's not a listed archive
type.
