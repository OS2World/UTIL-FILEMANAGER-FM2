:h2 res=100100 id='PANEL_TROUBLE'.
Troubleshooting
:p.If FM&slash.2 won&apos.t run&comma. &lpar.Error Message&colon. FM3RES.STR isn't in right format, at least for this version of FM/2.&rpar.
the probable culprit is CONFIG&per.SYS&per.  Your LIBPATH statement should contain a "&per.&bsl." entry&per.
If yours doesn&apos.t&comma. add it&per.  It&apos.s
standard for an OS&slash.2 installation&comma. but some buggy install programs knock
it out because they translate entries to their full pathname before
rewriting &lpar.so "&per.&bsl." gets translated to whatever the current
directory is for the buggy install program&rpar.&per.  What this "&per.&bsl." entry does is
allow a program to find and use &per.DLL files in the program&apos.s current directory
&endash.&endash. obviously something you want programs to be able to do&comma. otherwise
you&apos.d have to put every application&apos.s &per.DLLs into directories already on the
LIBPATH&comma. or add the directories of all applications to the LIBPATH&comma. a
rather huge pain in the&comma. uh&comma. neck&per.
:p.If things in FM&slash.2 are suddenly acting strange after an upgrade&comma.
first make &us.sure&us. you unpacked &us.all&us. the files and overwrote the old
ones&comma. then run INSTALL&per.CMD to update your WPS objects&per.
:p.If the "FM&slash.2 Online Help" object in the "FM&slash.2 Docs" subfolder
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
:p. &endash. Icon display in Directory Container does not always match WPS icons
:p. &endash. Spurious WPS Objects Handles created during some operations
:p. &endash. PM has a 64k draginfo buffer for compatibility with 16 bit programs.
This limits each drag operation to a maximum of about 1800 objects, however in some testing
we were limited to under 1700. The main problem is PM is happy to over write this buffer.
The result is significant corruption of share memory forcing a reboot. We have limited drag
operations to maximum number of objects that will fit in the dragitem 
structure (700- 1700 depending on path length and other factors) 
to prevent this problem.
