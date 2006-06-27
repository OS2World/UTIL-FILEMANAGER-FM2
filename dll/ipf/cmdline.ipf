:h1 res=90600 name=PANEL_CMDLINE.Editing Commandline
:i1 id=aboutCmdline.Editing Commandline
Enter any optional arguments to the program here as you would on the
command line. Remember that, when passing commands to a command
processor such as CMD.EXE, like DIR, it's CMD.EXE /C DIR, not just
CMD.EXE DIR.
:p.
:hp1.Full Screen, Maximized, Minimized, Invisible, Default&colon.:ehp1.
control how the program will be run. Default is usually in a window
with OS/2 controlling the size of the initial window. Note that PM
programs will always run on the desktop in a window and programs marked
full screen only will always run in a full screen session. This
corresponds to START /FS, /MAX, /MIN, /I or just START.
:p.
:hp1.Keep when done&colon.:ehp1. determines whether the window will
remain until you close it, or go away when the command completes. It's
like START /K. For reasons of safety (too complex to explain briefly)
you aren't allowed to Keep a DOS session; this flag is ignored for DOS
executables.
:p.
The :hp1.Environment:ehp1. MLE control lets you enter environment strings
for the program to inherit. Generally speaking, this is only for running
DOS programs as any strings entered here are interpreted as DOS settings.
For example, :hp3.IDLE_SECONDS=5:ehp3. would adjust the DOS setting
IDLE_SECONDS to 5. Names of DOS settings are as shown in the Settings
notebook for a DOS program.
:p.
Note&colon. FM/2 automatically stores command lines you use here for
you if the :hp1.Save command line:ehp1. checkbox is checked. You can
get to them by clicking the :hp1.V:ehp1. button beside the entry field.
Up to 250 command lines can be stored in this manner, kept in a file
named CMDLINES.DAT between sessions. Pressing the :color fc=default bc=palegray.Delete:color fc=default bc=default.
 key (or clicking the :hp1.Del:ehp1. button that appears) while one of the
names in the listbox is highlighted will remove it. Pressing :color fc=default bc=palegray.Ctrl
:color fc=default bc=default. + :color fc=default bc=palegray.Delete:color fc=default bc=default.. will remove (wipe) all entries.
:p.
Tech note&colon. FM/2 picks the directory in which to start the process
using the following criteria&colon.
.br
If the executable contains a path, FM/2 uses that directory.
.br
Otherwise, if the first filename argument contains a path, FM/2 uses
that directory.
.br
Otherwise, FM/2 uses its default directory.
:p.
See also (in the online Command Reference CMDREF.INF)&colon.
:p.
:link reftype=launch object='VIEW.EXE' data='CMDREF.INF
CMD.EXE'.CMD.EXE:elink.
.br
:link reftype=launch object='VIEW.EXE' data='CMDREF.INF START'.START
command:elink.
