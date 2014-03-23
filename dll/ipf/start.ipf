.***********************************************************************
.*
.* $Id$
.*
.* Start up
.*
.* Copyright (c) 1993-98 M. Kimes
.* Copyright (c) 2009, 2011 Steven H.Levine
.*
.* 2009-12-29 SHL Split from install.ipf
.* 2011-09-14 SHL Correct typo
.* 2014-03-23 GKY Add section on program object parameters and behaviors 
.*
.***********************************************************************
.*
:h1 res=100125 name=PANEL_FIRSTTIME. Starting FM&slash.2
:p.
If you start File Manager&slash.2 &lpar.filename FM3&per.EXE&rpar. with no
command line parameters&comma. it opens the  :link reftype=hd res=97600.Drive Tree window:elink.
but no directory windows &lpar.unless the :link reftype=hd res=97000."Save state of dir windows":elink.
toggle is checked&per.
:p.File Manager&slash.2 optionally accepts names of directories in the
parameters field of its program object&semi. it will open a directory window for each
one&per.
:p.
Hint&colon.  If you often do different specific things with FM&slash.2&comma.
you might set up a WPS object customized to start up ideally for each activity&per.
For example&comma. if you wanted to maintain your BBS areas&comma. you might
have one object with parameters like&colon. "D&colon.&bsl.BBSDIR
D&colon.&bsl.BBSDIR&bsl.MAILIN
D&colon.&bsl.BBSDIR&bsl.MAILOUT"&comma. and if you also like to use FM&slash.2
to set icons on files&comma. you might have another object with parameters like&colon.
"E&colon.&bsl.MYICONS D&colon.&bsl.NEWFILES"&per.  The State :link reftype=hd res=99400.quicklist:elink.
can also be used for this within FM&slash.2 itself&per.
:p.You can exclude drives in the parameters by prefacing the drive letter
with "&slash."&comma. and there can be more than one drive letter behind the
"&slash." &lpar.i&per.e&per. "&slash.BH" to exclude both drives B&colon. and
H&colon.&rpar.&per.  Hint&colon.  Many people like
to use &slash.B to exclude "phantom" drive B&colon. if they have a
single&endash.floppy system&per.
Similarly&comma. you can cause drives NOT to be prescanned by the
Drive Tree by prefacing the drive letter with "&semi."&comma. drives NOT to load
icons for objects by prefacing the drive letter with "&comma."&comma. drives NOT
to load subjects for objects by prefacing the drive letter with "&grave." and
drives NOT to load longnames for objects by prefacing the drive letter
with "&apos."&per.  This can be handy if you have a very slow drive like a
CD&endash.ROM or Zip drive&per.  FM&slash.2 Lite recognizes these switches&comma. too&per.
 See also the :link reftype=hd res=99980.Drive Flags dialog:elink. in FM&slash.2&per.
:p.If you place the plus sign "&plus." &lpar.alone&comma. separated from
anything else by spaces&rpar. in the parameters&comma. FM&slash.2 will log&comma. to
FM2&per.LOG&comma. delete&comma. rename&comma.
move and copy operations that are performed by drag&endash.and&endash.drop or
with the menus&per.  FM&slash.2 Lite recognizes this switch&comma. too&per.
:p.The parameter "&endash." &lpar.alone&comma. separated from anything else by
spaces&rpar. causes FM&slash.2 to ignore&comma. not load or save&comma. the previous state of
directory windows &lpar.see the :link reftype=hd res=97000."Save state of dir windows":elink.
toggle under Config Menu&endash.&gt.Toggles in the online help&rpar. for that invocation&per.
:p.You can specify the name of an alternate INI file with "&endash.&lt.inifilename&gt."
&endash.&endash. for example&comma.
"&endash.C&colon.&bsl.FM2&bsl.MYINI&per.INI"&per.
:p.You can also specify the INI file location with the environment variable
FM3INI &endash.&endash. for example&colon.  SET FM2INI&eq.C&colon.&bsl.FM2&bsl.MYINI&per.INI&per.
:p.Program objects are optionally created on the desktop when FM2 is installed. These contain 
the parameter &percent.&asterisk. it is present so FM2 and the other apps can be opened
using drag and drop. If you drop a file on FM2 it opens as if you had just double clicked
on it. If you drop a folder or object on it&comma. it opens the directory associated with the
object or folder in addition to the shutdown directories &lpar.if any&rpar.. Archive viewer&slr.2 
will open files in the viewers or association you have setup in FM2 if the file isn't an archive.
Additional parameters for FM2, FM2Lite and Vtree you may find useful include placing drive 
letters following a &slash. &lpar.no spaces&rpar. to exclude them such as &slash.B so the 
phantom B drive won't be shown on single floppy systems.

