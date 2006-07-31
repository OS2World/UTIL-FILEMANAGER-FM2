:h2 res=91000 name=PANEL_EXTRACT.Extract from archives
:i1 id=aboutExtract.Extract from archives
:artwork name='..\..\bitmaps\extract.bmp' align=center.
:p.
To extract from an archive, select the archive(s), then select
:hp1.Extract:ehp1. from a context menu.
:p.
FM/2 presents you with a dialog that allows you to select the method of
extraction, add masks for files to extract, tweak the command line for
exotic settings, and select the extraction directory (the directory to
which the files will be extracted. You can drag file system objects
onto the Extract Directory entryfield to set the extraction directory,
or you can type in one you like, or click the Walk button.
:p.
If you check the :hp1.Remember...:ehp1. checkbox, this dialog will
remember some of its settings for the next time you use it. Uncheck
it and it'll forget them and use the defaults.
:p.
Click :hp1.Okay:ehp1. to begin extracting from the archive, or
:hp1.Cancel:ehp1. if you change your mind.
:p.
You can also extract files from the archive in the :link reftype=hd
res=90200.Archive Container:elink. window using that window's menus;
double-click an archive file in an FM/2 main window to view the
archive listing.
:p.
Notes&colon.
:p.
I have a copy of ARC.EXE here that's 'broken' in that it won't extract
files unless given a DOS filemask (for instance, to extract all files
you need to enter *.* (not *) in the masks field). FM/2 normally gives
:hp2.no:ehp2. filemasks as the argument when you want to extract
everything, which every other archiver in the world understands.
:p.
The ZIP/UNZIP programs are case sensitive even though OS/2 itself is
not, so, for example, trying to extract "*.PKT" when the file inside the
archive is "01234567.pkt" will fail to extract the file -- you'd have to
use "*.pkt". If in doubt, use both or use the -C command line switch to
force UNZIP to ignore case.
:p.
I'm told there's a bug in some versions of 4OS2 that can cause
a call to an archiver to fail if the archiver has an extension (i.e.
UNZIP works, UNZIP.EXE doesn't).  If things fail for no apparent reason
and you're using 4OS2 you might keep it in mind.

:h2 res=90300 name=PANEL_ARCHIVE.Build an archive
:i1 id=aboutArchive.Build an archive
:artwork name='..\..\bitmaps\archive.bmp' align=center.
:p.
To build an archive, select some files, then select :hp1.Archive:ehp1.
from a context menu. You can add files to an existing archive by
link-dragging them onto the archive object, or dragging them onto an
:hp1.:link reftype=hd res=90200.Archive Container:elink.:ehp1. window.
:p.
FM/2 will ask you for the type of the archive by presenting you with a
listbox from which to pick an archiver.
After that, another dialog appears to let you modify how the archive
will be created. Additional masks may be entered (remember that the ZIP
and UNZIP programs are case sensitive), the archiver command line
tweaked, and so forth. Click :hp1.Okay:ehp1. to create the archive, or
:hp1.Cancel:ehp1. if you change your mind.
:p.
Note that the archive name may be an existing archive, in which case
it's modified by adding the new files. If some of the files are
already in the archive, they're replaced.
