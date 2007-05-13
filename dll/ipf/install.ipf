:h2 res=100110 id='PANEL_INSTALL'.
Installation Instructions
:p.Pre&endash.requisite(s)&colon.
:ul.
:li.WarpIN version 0.9.20 or newer must be installed.
:eul.

:p.First-time installation or upgrading FM/2&colon.
:ul.

:li.Install (or re-install) from a FM/2 ZIP file distribution
:ol.
:li.Unpack the FM2&asterisk.&per.ZIP archive in a private directory &lpar.for example&comma.
C&colon.&bsl.FM2&rpar.&per.  (UNZIP&per.EXE works nicely to unpack the file&per.)
Two WPI files, FM2&asterisk.&per.WPI and FM2UTILS&asterisk.&per.WPI will be unzipped.
:li.Follow the instructions for installing from WPI files below.
:eol.

:li.Install (or re-install) from a FM/2 WPI file distribution
:ol.
:li.If upgrading an existing FM/2, be sure to exit/close any running instances of FM/2 and its utility programs.
:li.Open the WPS folder containing the WPI file(s).
:li.Double click on FM2*.WPI. Since WarpIN associates itself
with WPI files this will start WarpIN with FM2*.WPI as the
parameter.
:note.
If you double click on FM2UTILS*.WPI instead of
FM2*.WPI, you will only be able to install the FM/2
Utilities. It is recommended that, even if you only want to
install FM/2 Utilities, that you do so using FM2*.WPI.
:li.You may then install FM/2 to a directory of your choice
and, if FM2UTILS*.WPI is present, install FM/2 Utilities to
a directory of your choice.
:eol.
:eul.

:h3 res=100120 name=PANEL_UNINSTALL.
De-installing FM&slash.2
:p.De-installing FM/2 and/or FM/2 Utilities&colon.
:ol.
:li.Exit/close any running instances of FM/2 and its utilities.
:li.Run Warpin without parameters.
:li.Right click on the package you want to de-install and
select "de-install all packages".
:li.Click on OK in the "De-install Packages" window that
appears.
:li.Repeat steps 2 and 3 if you want to de-install both packages.
:eol.

:h3 res=100115 name=PANEL_MOVING.
Moving FM&slash.2
:p.Moving FM&slash.2 and/or FM&slash.2 Utilities&colon.
:ol.
:li.To preserve any customizations you may have made, copy your existing installation directory to the new
location.
:li.If you have added your own icons to any of the FM/2
folders, then you should move them elsewhere temporarily.
:li.Follow the instructions for
:link reftype=hd res=100120.
de-installing FM/2 and/or FM/2 Utilities
:elink.&per.
:li.Re-install to the new location using the
:link reftype=hd res=100110.
installation instructions above
:elink.
&per. This will update the WarpIN database and
reset your icons to the new locations.
:li.Move the icons you saved in step #2, back into the FM/2
folders if you wish.
:eol.

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

