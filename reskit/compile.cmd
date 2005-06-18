@Echo off
:: $Id$
:: Build resource kit

:: 17 Jun 05 SHL Rework comments add missing missing file check

if "%1" == "/S" goto JustStr
if "%1" == "/s" goto JustStr
if "%1" == "-S" goto JustStr
if "%1" == "-s" goto JustStr
if "%1" == "/B" goto JustStr
if "%1" == "/b" goto JustStr
if "%1" == "-B" goto JustStr
if "%1" == "-b" goto JustStr

Echo.
Echo FM/2 Translation Toolkit
Echo.
Echo Usage: compile [/B] [/S]
Echo.
Echo   /B rebuild both FM3RES.DLL and FM3RES.STR
Echo   /S rebuild FM3RES.STR
Echo.
Echo   See reskit.txt for detailed instructions.
goto End

:JustStr
Echo.
if not exist fm3dll.str goto Error
if not exist mkstr.exe goto Error
mkstr
if "%1" == "/S" goto End
if "%1" == "/s" goto End
if "%1" == "-S" goto End
if "%1" == "-s" goto End
Echo.
if not exist fm3res.rc goto Error
if not exist fm3res.dlg goto Error
if not exist fm3dlg.h goto Error
if not exist fm3dll2.h goto Error
if not exist fm3hlp.h goto Error
if not exist icons goto Error
if not exist fm3res.dll goto Error
rc -w2 -r fm3res
if errorlevel == 1 goto End
rc -x2 fm3res.res fm3res.dll
goto End
:Error
Echo.
Echo Required files are missing.  You need the following files in the default
Echo directory:
Echo.
Echo FM3DLL.STR
Echo MKSTR.EXE
if "%1" == "/S" goto SkipStr
if "%1" == "/s" goto SkipStr
if "%1" == "-S" goto SkipStr
if "%1" == "-s" goto SkipStr
Echo FM3RES.RC
Echo FM3RES.DLG
Echo FM3DLG.H
Echo FM3DLL2.H
Echo FM3HLP.H
Echo FM3RES.DLL
Echo Various icons in the ICONS subdirectory.
:SkipStr
Echo.
pause
Echo [A                 Here's what you've got in this directory:
dir /b
:End
