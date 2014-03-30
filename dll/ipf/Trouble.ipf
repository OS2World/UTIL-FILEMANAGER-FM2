.***********************************************************************
.*
.* $Id$
.*
.* fm/2 help - Troubleshooting
.*
.* Copyright (c) 1993-98 M. Kimes
.* Copyright (c) 2002-2014 Steven H.Levine
.*
.* 06 Apr 07 GKY Added drag limit information
.* 24 Jun 07 GKY Added change from VAC to open Watcom notes and Global issue with 4OS2.
.* 27 Dec 09 GKY Added information regarding use of the xworkplace trash can with FM/2
.* 09 Oct 11 GKY Added note about GBM.dll related trap.
.* 08 Jan 12 GKY Updated GBM.DLL note.
.* 01 Jan 13 GKY Removed GBM.DLL note as the problem has been fixed.
.* 01 Jan 13 GKY Removed reference to install.cmd which is no longer in the FM/2 package
.* 23 Mar 14 GKY Add known problems with starting with redirection and inability to copy
             from "completed" windows. Fixed some typos
.*
.***********************************************************************
.*
:h2 res=100100 id='PANEL_TROUBLE'.
Troubleshooting
:p.If FM&slash.2 won&apos.t run&comma. &lpar.the probable culprit is CONFIG.SYS.  
Your LIBPATH statement should contain a "." entry.
If yours doesn&apos.t&comma. add it&per.  It&apos.s
standard for an OS&slash.2 installation&comma. but some buggy install programs knock
it out because they translate entries to their full pathname before
rewriting &lpar.so "." gets translated to whatever the current
directory is for the buggy install program&rpar..  What this "." entry does is
allow a program to find and use .DLL files in the program&apos.s current directory
&endash.&endash. obviously something you want programs to be able to do&comma. otherwise
you&apos.d have to put every application&apos.s .DLLs into directories already on the
LIBPATH&comma. or add the directories of all applications to the LIBPATH&comma. a
rather huge pain in the&comma. uh&comma. neck.
:p.If things in FM&slash.2 are suddenly acting strange after an upgrade&comma. try
reinstalling the Warpin package.
:p.If the "FM&slash.2 Online Help" object in the "FM&slash.2 Docs" sub-folder
won&apos.t work properly&comma. you&apos.ve got a version of VIEW&per.EXE that won&apos.t
directly display help &lpar.&per.HLP&rpar. files&per.  Use SEEHELP&per.EXE from the
:link reftype=hd res=100090.FM&slash.2 Utilities:elink. package to
get around this problem &lpar.upgrade your version of OS&slash.2 or switch to NewView.exe&rpar.&per.
:p.Don&apos.t drag files over Netscape &endash.&endash. it will lock up if files &lpar.not WPS
objects&semi. there&apos.s a subtle difference&rpar. are dragged over it&per.

:h2 res=100105 name=PANEL_KPROBLEMS.
Known problems&slash.shortcomings
:p. &endash. FM&slash.2 does not properly report sizes of some files with long
name resident on Win95&slash.NT drives&per.
:p. &endash. File List Container fails to fill after drive change SYS0039
error&per.
:p. &endash. Tree switching on Focus&slash.Directory Change appears to be slower
than expected&per.
:p. &endash. Access to LS120 and FAT32 drives slower than expected
:p. &endash. Icons displayed in Directory Containers do not always match WPS icons
:p. &endash. Spurious WPS Objects Handles are created during some operations
:p. &endash. PM has a 64k draginfo buffer for compatibility with 16 bit programs.
This limits each drag operation to a maximum of about 1800 objects, however in some testing
we were limited to under 1700. The main problem is PM is happy to over write this buffer.
The result is significant corruption of share memory forcing a reboot. We have limited drag
operations to maximum number of objects that will fit in the dragitem
structure (700-1700 depending on path length and other factors)
to prevent this problem.
:p. &endash. Trying to run Global.exe from the command line in 4OS2 (perhaps other shells)
results in the execution of 4OS2's "Global" command. Global.exe can be run by placing
it in quotes or prefixing it with *. See 4OS2's documentation for more information.
:p. &endash. As with any OS/2 program significant changes (such as our change to OpenWatcom)
will result in the incompatibility of older (VAC) exes with new dlls and vice-versa. Attempting to run
OpenWatcom exes with a VAC dll loaded in memory or in your libpath ahead of the new version
will result in a SYS3175 in popuplog.os2. The reverse gives a SYS2070. If you experience these
problems search your libpath for fm3dll.dll and remove or rename it. Your libpath should have "."
(without the quotes) as your first entry to minimize the likely hood of this problem. FM/2's installer
does not add the FM/2 directory to the libpath.
:p. &endash. Accessing a subdirectory on a vfat (fat) formatted USB removable drive may result
in a SYS3175. If this occurs try accessing the directory using "open" from the context (popup)
menu. Mounting it using netdrives' vfat plugin also solves the problem.
:p. &endash. Move to trash can is only active for local hard drive (this is a design limitation 
of the xworkplace trash can). The result will be a permanent delete for all other 
drive types. Also be aware that deleted files are still retained on the drive 
they were deleted from. The result can be full drive type errors. If you are 
deleting to free up drive space you must either empty the trash can or use 
:hp6.Permanent Delete:ehp6. which deletes the files directly bypassing the trash can.
:p. &endash. You can't copy from a completed command window in OS&slash.2. If you run 
commands where you wish to copy the output following their completion you have 2
options. If you have dragtext installed you can select the text, drag it to the desktop
and then copy it from the file created by this operation. Alternatively, you can use a
commandline of the form CMD.EXE &slash.k BLDLEVEL.EXE  &percent.a. This leaves a running
window because the command shell doesn't terminate and the text can be copied in the 
usual ways from the active window.
:p. &endash. Starting FM2 from commandline with stdout and stderr redirected to a file may
result in command shell windows started from FM2 opening blank &lpar.and useless&rpar.. 
You can work around this in 2 different ways. First you can use logFM2.cmd located in 
the debug directory and as an object in the FM2 folder which starts FM2 with redirection. 
Otherwise you can use &numsign. as the first switch on the commandline &lpar.FM3.EXE 
&numsign. 2&gtsym.&gtsym.FM2.LOG&rpar.. The drawback to the second solution is it uses 
the base environment, the one set by CONFIG.SYS. This means that command prompts start 
directed at the root of the boot drive not the directory from the active directory container. 
You also lose any other environment variables created by the method you used to start FM2.

