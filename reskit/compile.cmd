@Echo off
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
Echo Translate the _text only_ in FM3DLL.STR, FM3RES.RC and FM3RES.DLG
Echo (do _not_ reorder, remove or create new lines in FM3DLL.STR or alter
Echo any other files, except, perhaps, icons!).  Then type COMPILE /B at
Echo a command line to build both the new FM3RES.DLL and FM3RES.STR files.
Echo Copy these new files to your FM/2 directory to use them.  You will
Echo need a copy of RC.EXE on your PATH and appropriate OS/2 #include
Echo files in order to replace the resources (dialogs, icons, menus, etc.)
Echo in FM3RES.DLL.  RC and the #include files come with the OS/2
Echo developer's toolkit and various OS/2 compilers.  See comments in
Echo FM3STR.H for remarks about additions and insertions.
Echo.
Echo Type COMPILE /S to compile only FM3RES.STR from FM3DLL.STR (RC and
Echo #include files not required).  Don't fool with those odd-looking %%s,
Echo %%lu, etc. thingies in FM3DLL.STR -- they're used by the C runtime
Echo *printf routines, and FM/2 may crash if you remove or reorder them.
Echo.
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
Echo MKSTR2.EXE
if "%1" == "/S" goto SkipStr
if "%1" == "/s" goto SkipStr
if "%1" == "-S" goto SkipStr
if "%1" == "-s" goto SkipStr
Echo FM3RES.RC
Echo FM3RES.DLG
Echo FM3DLG.H
Echo FM3DLL2.H
Echo FM3RES.DLL
Echo Various icons in the ICONS subdirectory.
:SkipStr
Echo.
pause
Echo [A                 Here's what you've got in this directory:
dir /b
:End
