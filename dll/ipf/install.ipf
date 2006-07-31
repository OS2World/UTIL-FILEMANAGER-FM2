:h2 res=100110 id='PANEL_INSTALL'.
Installation Instructions
:p.First&endash.time installation&colon.
.br
&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.
.br
1&per. Unpack the FM2&asterisk.&per.ZIP archive in a private directory &lpar.for example&comma.
C&colon.&bsl.FM2&rpar.&per.  UNZIP&per.EXE works nicely to unpack the file&per.
:p.  2&per.  Run the provided INSTALL&per.CMD in that directory to build a folder
and program objects&per.  INSTALL &slash.? will give you simple command line
help on a couple of options&per.
:p.  3&per.  Run it &endash.&endash. double&endash.click the "FM&slash.2" or
"FM&slash.2 Lite" object in the File Manager&slash.2 folder&per.
Play&comma. browse&comma. have fun&per.  Full help is
available from within the program&comma. and it helps you with quick
initial setup the first time it&apos.s run&per.
:p.Upgrading existing versions of FM&slash.2&colon.
.br
&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.
.br
      If upgrading&comma. just unpack the files in the FM&slash.2 directory&per.
Be sure to overwrite old files&per.  You might want to skip ARCHIVER&per.BB2
&comma.by deleting ARCHIVER&per.TMP prior to running INSTALL&comma.
if you&apos.ve customized it &endash.&endash. other customizations are
automatically preserved&per.  You might then want to rerun INSTALL or just delete
&asterisk.&per.TMP and &asterisk.&per.ICO to clean up some stuff that you
won&apos.t need when upgrading&per.  Note&colon.  don&apos.t attempt to unpack using FM&slash.2
into the directory in which FM&slash.2 is already running &endash.&endash.
OS&slash.2 locks in&endash.use files and the upgrade won&apos.t be completed properly&semi. not all files
will be unpacked&per.
:h3 res=100115 name=PANEL_MOVING.
Moving FM&slash.2
      Rerun INSTALL&per.CMD if you move the FM&slash.2 directory&per.  This will
make sure all the objects in the folders and &asterisk.&per.CMD files point to
the right place&per.  Compare to other products&comma. where&apos.d you&apos.d
have to reinstall if you could only figure out how to completely uninstall
the existing version&per.&per.&per. Remember to update config&per.sys if you have
added the FM2 or Utils directory to your paths or have set an FM2 INI file name&per.
:h3 res=100120 name=PANEL_UNINSTALL.
Uninstalling FM&slash.2
      If you later decide to remove FM&slash.2&comma. run UNINSTAL&per.CMD in
the FM&slash.2 directory and follow simple directions&per.  Both installation and
deinstallation are no&endash.brainers&per.  UNINSTAL will help you quickly
and easily remove all traces of FM&slash.2 from your system&comma. if you
don&apos.t like it for some reason&per.  All programs should be so nice&per.

:h3 res=100125 name=PANEL_FIRSTTIME.
Starting FM&slash.2
:p.Starting FM&slash.2 &endash.&endash. how it works&comma. customizing
parameters&colon.
.br
&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.&endash.
.br
If you start File Manager&slash.2 &lpar.filename FM3&per.EXE&rpar. with no
parameters&comma. it opens the  :link reftype=hd res=97600.Drive Tree window:elink.
but no directory windows &lpar.unless the :link reftype=hd res=97000."Save state of dir windows":elink.
toggle is checked&per.
:p.File Manager&slash.2 optionally accepts names of directories in the
parameters field of its program object&semi. it&apos.ll open a directory window for each
one&per.
.br
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
FM2INI &endash.&endash. for example&colon.  SET FM2INI&eq.C&colon.&bsl.FM2&bsl.MYINI&per.INI&per.

