:h2 res=100085 id='PANEL_CRESOURCES'.
Customizing FM&slash.2 Resources
:p.Since FM&slash.2 is a GPL application&comma. anyone can build a private
version of FM&slash.2 from source and modify the program as they choose&per.
However&comma. if all one wants to do is replace icons&comma. bitmaps and
text strings&comma. this is not required&per.
The FM&slash.2 build system provides support for replacing resources without
a full rebuild from source&per.
:p.&eq.&eq. Prerequistes &eq.&eq.
:p.You will need
:p. &endash. a copy of the current FM&slash.2 sources
.br
 &endash. a copy of the FM&slash.2 binary release
.br
 &endash. a copy of the resource compiler&comma. RC&per.EXE
.br
 &endash. a copy of the help compiler&comma. IPFC&per.EXE
.br
 &endash. a copy of the exe compression tool&comma. LXLITE&per.EXE
.br
 &endash. a subset of the Warp4 toolkit &numsign.include files
:p.RC&per.EXE&comma. IPFC&per.EXE and the &numsign.include files are included in
Warp4 Toolkit which comes with eCS or MCP&per.
:p.Not all versions of RC&per.EXE are created equal&per.  Currently&comma. the
best version to use is&colon.
:p. 12&endash.18&endash.97   6&colon.01         868&comma.000           0
 RC&per.EXE
:p.which reports itself as&colon.
:p. IBM RC &lpar.Resource Compiler&rpar. Version 5&per.00&per.002 Dec 18 1997
:p.and it is available at&colon.
:p.
  &lt.ftp&colon.&slash.&slash.ftp&per.software&per.ibm&per.com&slash.ps&slash.products&slash.warpzilla&slash.os2tk40rc&per.zip&gt.
:p.You can get LXLITE&per.EXE at&colon.
:p.
 &lt.http&colon.&slash.&slash.hobbes&per.nmsu&per.edu&slash.cgi&endash.bin&slash.h&endash.search?key&eq.lxlite&amp.pushbutton&eq.Search&gt.
:p.It is recommended that you install the Warp4 toolkit&per.  However&comma. if
space is tight&comma. you only need to have RC&per.EXE&comma. IPFC&per.EXE and the
following include files available&colon.
:p.  dirsize&per.h
.br
  fm3&per.h
.br
  fm3dlg&per.h
.br
  fm3dll&per.h
.br
  fm3dll2&per.h
:p.&eq.&eq. What you can change &eq.&eq.
:p.In general&comma. you can change any of the bitmaps or icons and you can make
changes to the text strings in FM3DLL&per.STR&comma. FM3RES&per.RC or
FM3RES&per.DLG&per. When making changes to text strings&comma. translate the &us.text
only&us.&per.  Do &us.not&us. reorder&comma. remove or create new lines in FM3DLL&per.STR&per.
Do not modify the odd&endash.looking &percent.&percent.s&comma. &percent.&percent.lu&comma.
etc&per. items in FM3DLL&per.STR&per. They are used by the formatted print routines&per.
:p.When replacing icons&comma. make sure you provide an appropriate set of
resolutions&per.  When replacing bitmaps&comma. make sure you to size the bitmap
to fit correctly in place of the original bitmap&per. You need to limit icons to 256 colors and 40x40 pixels&per.
:p.Do not alter any other files&per.  If you do not follow the above
guidelines&comma. the resource update may fail or FM&slash.2 may refuse to run correctly&per.
:p.&eq.&eq. Installation &eq.&eq.
:p. &endash. Create a work directory&per.
:p. &endash. Unzip the FM&slash.2 sources to this directory preserving
the directory structure&per. The resulting tree will look like
:p.    &bxle.&bxh.&bxh.fm2&per.work
.br
       &bxle.&bxh.&bxh.bitmaps
.br
       &bxle.&bxh.&bxh.dll
.br
       &bxv.  &bxle.&bxh.&bxh.icons
.br
       &bxv.  &bxle.&bxh.&bxh.internal
.br
       &bxv.  &bxll.&bxh.&bxh.ipf
.br
       &bxv.     &bxll.&bxh.&bxh.bitmaps
.br
       &bxll.&bxh.&bxh.icons
:p. &endash. Unzip the FM&slash.2 binaries to the work directory&per.
:p. &endash. Copy the DLLs from the the work directory to the dll subdirectory
:p. &endash. Copy FM2RES&per.STR from the the work directory to the dll subdirectory
:p.Alternatively&comma. you can use your installed FM&slash.2 binaries&per.
:p. &endash. Copy the FM&slash.2 EXEs to the work directory
:p. &endash. Copy the FM&slash.2 DLLs to the dll subdirectory
:p. &endash. Copy FM2RES&per.STR to the dll subdirectory
:p.&eq.&eq. Verification &eq.&eq.
:p.To test your setup&comma. run&colon.
:p.  nmake res from the work directory&per.  The makefile should run to completion without
errors&per.
:p.&eq.&eq. Customizing &eq.&eq.
:p.To customize your copy of FM&slash.2
:p. &endash. Edit the resources&comma. bitmaps and icons&comma. as needed&per.
:p. &endash. Apply the updates to the EXEs and DLLs with nmake res
:p.   This will compile the resources&comma. decompress the the the EXEs and
DLLs and apply the updated resources&per.  The output of this process will be updated
versions of one or more of the FM&slash.2 EXEs&comma. DLLs and FM3RES&per.STR&per.
The nmake is efficient and only rebuilds the files with changed content&per.
:p. &endash. Test your updates&colon.
:p.     set LIBPATHSTRICT&eq.T
.br
     cd dll
.br
     &per.&per.&bsl.fm3
:p.LIBPATHSTRICT allows you to test the new FM&slash.2 version without shutting
down the production version&per. When you are satisfied with your changes&comma. copy the updated executable
files to your FM&slash.2 program directory and restart FM&slash.2&per.  Some files may
be locked&comma. so use the unlock&per.exe utility that comes with lxlite to allow the locked files
to be overwritten&per.  I recommend adding an unlock item to the commands
menus&comma. so that you have it available from FM&slash.2&per.
:p.&eq.&eq. Trouble Shooting &eq.&eq.
:p.&endash.&endash. nmake does not rebuild the target &endash.&endash.
:p.nmake depends on file timestamps to know what targets to rebuild&per.  If you
replace a file with an older version nmake may not do what you want it to&per.
:p.One workaround is to use one of the available touch utilities&comma. to
change the dependent file&apos.s timestamp&per.
:p.If you do not want to change the file&apos.s timestamp&comma. use
nmake &endash.a res to force all targets to be rebuilt&per.



