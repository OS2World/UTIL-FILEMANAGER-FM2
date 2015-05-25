.***********************************************************************
.*
.* $Id$
.*
.* Installation help
.*
.* Copyright (c) 1993-98 M. Kimes
.* Copyright (c) 2009 Steven H.Levine
.*
.* 02 Dec 09 SHL Correct FM2INI typo
.* 29 Dec 09 SHL Split start up doc to start.ipf
.*
.***********************************************************************
.*
:h1 res=100110 id='PANEL_INSTALL'.
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

:h2 res=100120 name=PANEL_UNINSTALL.
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

:h2 res=100115 name=PANEL_MOVING.
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
