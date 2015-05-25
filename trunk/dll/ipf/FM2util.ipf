:h2 res=100090 id='PANEL_FM2UTIL'.
FM&slash.2 Utilities

These Utilities are not part of the current open source version of FM2. They are necessary for
some functions in FM2. They are avalable on Hobbes (http&colon.//hobbes.nmsu.edu/pub/os2/util/browser/fm2utils.zip)
as freeware under the following licence&colon.
:p.
They are copyright &lpar.c&rpar. 1994&slash.95 by M&per. Kimes &lpar.Barebones Software&rpar. all rights reserved

These utilities may be freely used by end users&per.  M. Kimes' retains copyright and reserves all rights&per.  There is NO warranty expressed or implied&per.
:p.Distribution rights are granted for online &lpar.dial&endash.up&comma.
Internet&comma. CIS&comma. etc&per.&rpar. services ONLY without prior written permission from author&per.
If you want to distribute for money&comma. you MUST contact me first and make arrangements&per.

:p.This collection of utilities may prove useful in conjunction with FM&slash.2 &lpar. as it provides
some of its functionality such as HPFS and Fat optimization&rpar.
:p.All the &per.EXE utilities provide help if you run them followed by &slash.?
&lpar.hint&colon.  arguments shown in brackets &lbracket.&rbracket. are
optional&comma. arguments shown in braces &lt.&gt. are required&rpar.&per.
:p.
The &per.CMD utilities may be directly inspected using a text editor and are commented
as to use and function&per.
.br
It&apos.s recommended that you place these utilities into a directory on your
PATH &lpar.type HELP PATH at a command prompt if you don&apos.t know what that
means&rpar.&per.  If you have FM&slash.2 installed&comma. may I suggest you make
a UTILS directory off your FM&slash.2 directory &lpar.the FM&slash.2 INSTALL&per.CMD
assumes you the utilities will be in that location&comma. and will make some program objects for you&rpar.?
:p.These utilities are standalone&comma. meaning that one doesn&apos.t rely on
another&per.  You can therefore delete any you don&apos.t want to keep around
without affecting the operation of any of the others&per. Files followed by an asterisk
are used by FM2 to provide some of its base functionality.
:p.
Following is a very brief description of each program in the package&colon.
:p.BEEP&per.EXE
.br
    Makes a beep noise&per.
:p.CRC&per.EXE
.br
    Calculates and prints the CRC of a file or files&per.
:p.CVT4OS2&per.CMD
.br
    Converts 4DOS&slash.4OS2 non&endash.standard DESCRIPT&per.ION files to
WPS&endash.standard &per.SUBJECT EAs &lpar.see comments in program&rpar.&per.
:p.CVTFBBS&per.CMD
.br
    Converts FILES&per.BBS file comments to WPS&endash.standard &per.SUBJECT EAs
&lpar.see comments in program&rpar.&per.
:p.DELTREE&per.EXE
.br
    Deletes a directory and all its subdirectories and files&per.
:p.DRVRLIST&per.EXE
.br
    Lists all mounted drivers&per.  Use &slash.w switch for a "wide" listing&per.
:p.DRVTYPES&per.EXE
.br
    Lists all drives with information on their types&per.
:p.DSTART&per.EXE
.br
    A START command that will work from DOS VDMs &lpar.penalty box&rpar.&per.
:p.EA&per.EXE
.br
    An extended attribute manipulation program&per.  Can be used to zap all the EAs from REXX &per.CMD files&per.
:p.EJECT&per.EXE
.br
    Eject media from a removable drive&per.  Notes&colon.  when EJECT&comma.
LOCK or UNLOCK returns an error code of 31&comma. it usually means that function
is not supported for the requested drive&per.  "The parameter is not correct" may mean
you tried it on a non&endash.existent or fixed drive&per. Error 163 is not in the OS&slash.2
message file&semi. it&apos.s "uncertain media&comma."and usually just repeating the command
will "fix" it&per.
:p.FATOPT&per.EXE&asterisk.
.br
    An OS&slash.2 FAT optimizer &endash.&endash. defragment&comma. condense
freespaces&comma. sort file and directory names&comma. test sectors in system areas&comma. test
&lpar.and repair some damaged&rpar. clusters&comma. report on drive usage&per.  If you
receive a "FATOpt can&apos.t lock" message from FATOpt&comma. the drive is in use and
FATOpt can&apos.t optimize it&semi. try booting from floppies &lpar.place
FATOPT&per.EXE on the floppy together with VIOCALLS&per.DLL&rpar.&per.  If you
receive a "FATOpt ran out of memory" error&comma. add RAM and&slash.or set
up a swapfile on a partition other than the one being optimized &lpar.type
HELP SWAPPATH at a command line for more info&rpar. and&slash.or try the
&endash.p or &endash.q switches&per.  Other errors &lpar.uncommon&rpar. should be
self&endash.explanatory&comma. if distressing&per.  CHKDSK&slash.F should be run before running
FATOpt&comma. and it&apos.s always a good idea to back up before defragging&per.  FATOpt
errorlevel returns for batch files&colon.  0 &eq. no problems&comma. 1 &eq.
user abort&comma. 2 &eq. out of memory&comma. 3 &eq. bad file &lpar.run
CHKDSK&slash.F&comma. dammit&rpar.&comma. 4 &eq. can&apos.t open drive&comma. 5 &eq. can&apos.t lock drive&comma. 6
&eq. can&apos.t fix bad cluster&comma. 7 &eq. insufficient free space on drive&comma. 8 &eq. unknown
error&comma. 9 &eq. bad system area&comma. 10 &eq. error during standard file copy&per.
.br
.br
                                                  :color fc=default bc=red.WARNING&colon. BACKUP BEFORE OPTIMIZING&per.
.ce REMEMBER THE BEST OPTIMIZATION IS ACCOMPLISHED BY A BACKUP&slash.DELETE&slash.RESTORE&per.
:color fc=default bc=default.
:p.FINDPATH&per.EXE
.br
    List a PATH&endash.style environment variable&apos.s directories &lpar.or
LIBPATH&rpar.&comma. or find a file or files in those directories&per.  Perhaps useful for
tracking down files that appear in more than one directory on the PATH or LIBPATH&per.
:p.FLUSH&per.EXE
.br
    Flush file system caches and buffers &lpar.make system quiescent&rpar.&per.
:p.FM2PLAY&per.EXE&asterisk.
.br
    Used by FM&slash.2 to play multimedia files with the "Play Multimedia"
command &lpar.requires MMPM&slash.2&rpar.&per.
:p.HPFSOPT&per.EXE&asterisk.
.br
    An HPFS optimizer&per.
:p.ICONIFY&per.EXE
.br
    "Iconifies" image files&comma. which is to say that it sets the icon
of image files to be miniature versions of the image files&per.  A
PM program&comma. but designed to be run in batch mode from a command line&per.
:p.IMAGE&per.EXE&asterisk.
.br
    Used by FM&slash.2 to quickly display image files &lpar.requires
MMPM&slash.2&rpar.&per. Request a context menu for several options&per.
:p.ISTEXT&per.EXE
.br
    Returns ERRORLEVEL 1 if a file appears to be a text &lpar.ASCII&rpar.
file&comma. 0 if not or file is inaccessible&per.
:p.KILL2&per.EXE
.br
    A timed deletion&slash.touch&slash.list utility&per.
:p.KILLPID&per.EXE
.br
    Kill processes by PID or title&comma. or list PIDs of running processes&per.
.br
    See also SWITCHTO&per.EXE&per.
:p.KEYLOCKS&per.EXE
.br
    Control state of num lock&comma. caps lock&comma. scroll lock&per.
Note&colon. PM program&comma. but still designed to be run from command line&per.
:p.LA&per.EXE
.br
    Lists the contents of archive files&comma. extracts files from archives
using one command without you having to know the archive type or
archiver commands &lpar.requires ARCHIVER&per.BB2 datafile to be on your
PATH or DPATH&comma. copy included in the archive&comma. edit to taste&slash.need&rpar.&per.
:p.LINES&per.EXE
.br
    Counts lines in text files&per.
:p.LOCK&per.EXE
.br
    Lock a removable drive&per.
:p.MAKEOBJ&per.CMD
.br
    Creates WPS objects for filenames &endash.&endash. program objects for
programs&comma. shadows for everything else &endash.&endash. on the desktop&per.
:p.MKPATH&per.EXE
.br
    Make directories many levels deep with one command&per.
:p.MOV&per.EXE
.br
    A MOVE command that can move between drives and optionally allows overwriting&per.
:p.MSENSIT&per.EXE
.br
    Set sensitivity of mouse&per.
:p.NO&per.EXE
.br
    Hides file system objects&comma. then runs a command &lpar.excludes file
system objects from a command&rpar.&per.
:p.NOCAD&per.EXE
.br
    Disables&slash.reenables &lpar.toggles&rpar. CTRL&endash.ALT&endash.DEL&per.
:p.OPEN&per.EXE
.br
    Opens a WPS object or website&per.
:p.PRIORITY&per.EXE
.br
    A launch&endash.with&endash.priority program that lets you set the priority
of other programs when you start them&per.
:p.PTREE&per.EXE
.br
    Displays the tree of a given path&comma. several options&per.
:p.QFORMAT&per.EXE
.br
    Quick format floppies by zapping the root and FATs&per.  NOTE&colon.
Warp&apos.s FORMAT will do this now&comma. too &lpar.&slash.Q switch&rpar.&per.
:p.QPLAY&per.EXE
.br
    Quiet play plays multimedia files via MMPM&slash.2 without showing itself
&lpar.perhaps useful from batch files&rpar.&per.
:p.REBOOT&per.EXE
.br
    Reboots the system as though you&apos.d pressed CTRL&endash.ALT&endash.DEL&per.
:p.REBOOTP&per.EXE
.br
    Reboots the system as though you&apos.d pressed CTRL&endash.ALT&endash.DEL
after broadcasting a WM&us.QUIT message to all children of the desktop and
waiting 60 seconds for the applications to save their settings and
quit &lpar.note that VIO and DOS applications will pop up their "Are you
sure?" boxes&comma. but you don&apos.t have to be there to answer &lpar.and
shouldn&apos.t be &endash.&endash. use Shutdown instead if you&apos.re not
automating&rpar.&comma. though such apps won&apos.t pick up on the hint to shut down&rpar.&per.
This is a PM application&comma. but still designed to be run from command line&per.
:p.REMTAB&per.EXE
.br
    Removes TABs from text files&per.
:p.RENCASE&per.EXE
.br
    Renames files to the same name but all upper&comma. lower or "mixed" case&per.
:p.RESET&per.EXE
.br
    Resets a drive&per.
:p.SAVEDESK&per.EXE
.br
    Saves the Desktop&comma. optionally restarts WPS by killing it and letting it restart itself&per.
:p.SCOPY&per.EXE
.br
    Copy new files and&slash.or newer versions of existing files&per.  Primarily
intended for backup and synchronization&per.
:p.SDIR&per.CMD
.br
    Directory listing that shows &per.SUBJECT EAs&per.
:p.SEEHELP&per.EXE
.br
    For systems where VIEW&per.EXE will not display &per.HLP files&comma. this
lets you view help files without starting the program to which they belong&per.
:p.SMODE&per.EXE
.br
    Set screen modes &lpar.columns&comma. rows&comma. resolutions&rpar. for
VIO&slash.full screen sessions or reports on available modes&per.
:p.SNAPSHOT&per.EXE
.br
    Creates a "snapshot" file compatible with PMDMatch&per.
:p.SR&per.EXE
.br
    Simple search&endash.and&endash.replace command&comma. optional interactive
mode&per. Intended to be used non&endash.interactively for many files in a single
pass &lpar.otherwise a text editor is more suitable&rpar.&per.
:p.SRCH&per.CMD
.br
    A search command for OS&slash.2 &lpar.finds text in files&rpar.&per.
:p.STRIP&per.EXE
.br
    Filter that strips specified strings from input&per.
:p.SUBJ&per.CMD
.br
    Show and modify &per.SUBJECT EAs&per.
:p.STRIPCDS&per.EXE
.br
    A filter that strips ANSI and VT&endash.100 escape codes from stdin and
writes the results to stdout&per.
:p.SWEEP&per.CMD
.br
    A sweep command for OS&slash.2&semi. performs a command in all
subdirectories of the current directory&per.
:p.SWITCHTO&per.EXE
.br
    Switch current or a specified process &lpar.by PID or partial title&rpar. to
the foreground&per.
:p.TEST&per.EXE
.br
TEST&per.CMD
.br
TESTPM&per.EXE
.br
    Repeat all the arguments it receives&semi. useful for debugging command
lines and figuring out what arguments one program receives from another&per.
:p.TESTDRV&per.EXE
.br
    Test a drive&per.  Should be run with a drive other than the one being
tested as the default for best results &lpar.saves a logfile listing any
bad sectors found to the default directory&rpar.&per.
:p.TESTFIND&per.EXE
.br
    Tests an IFS&apos.s DosFindFirst&slash.Next implementation for troubleshooting&per.
:p.TESTHAND&per.EXE
.br
    Reports the number and type of inherited file handles&per.
:p.TESTRC&per.EXE
.br
    Runs a program and reports its return code &lpar.ERRORLEVEL&rpar.&per.
:p.TYPERATE&per.EXE
.br
    Sets the typematic rate&per.
:p.UNARJER&per.CMD
.br
    Unarjs any arj file dropped onto it into the same directory where
the arj file resides&comma. then opens a folder for that directory&per.
:p.UNIQUE&per.EXE
.br
    Counts the number of lines&comma. words and unique words in a text
file&comma. gives a breakdown of how many times each unique word was used&per.
:p.UNLOCK&per.EXE
.br
    Unlock a removable drive&per.
:p.UNLZHER&per.CMD
.br
    Unlzhs any lzh file dropped onto it into the same directory where
the lzh file resides&comma. then opens a folder for that directory&per.
:p.UNZIPPER&per.CMD
.br
    Unzips any zip file dropped onto it into the same directory where
the zip file resides&comma. then opens a folder for that directory&per.
:p.WHERE&per.CMD
.br
    A where command for OS&slash.2&semi. finds files on a specified or the
default drive&per.
:p.WIPE&per.EXE
.br
    Wipes then erases files so that their data cannot be recovered&per.
Meets DOD standards&per.

:p. This section is adapted from the FM2utilities readme (fm2utils.doc) written by M. Kimes


