/*
   UTILSMGR - FM/2 program which is used to install various CMD files
   which facilitate access to FM/2 and its associated utilities from
   the command line.

   For unattended operation, call with /UNATTENDED argument.

   For usage help, run as MKUTILS /?

   List of CMD files:
      FM2.CMD
      AV2.CMD
      DIRSIZE.CMD
      EAS.CMD
      GLOBAL.CMD
      INI.CMD
      KILLPROC.CMD
      UNDEL.CMD
      VCOLLECT.CMD
      VDIR.CMD
      VIEWHELP.CMD
      VIEWINFS.CMD
      VTREE.CMD


*/

n = setlocal()

signal on novalue

/* Identify ourself */

/*
'@echo off'
'cls'

say'     ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿'
say'     ³                    FM/2 Installation Program                      ³'
say'     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ'
*/

/* set defaults, check arguments and adjust as required */

attended = 1
delete   = 0

parse upper arg tail
do while tail \= ''
  parse var tail curarg tail
  curarg = strip(curarg)
  /* if user asked for usage help, give it */
  if curarg = '/?' then do
    call report ''
    call report 'Usage:  MKUTILS [/DELETE] [/UNATTENDED]'
    call report ''
    call report ' /DELETE              Remove the CMD files'
    call report ' /UNATTENDED          don''t ask any questions; run without stopping.'
    call report ''
    call report 'Examples:'
    call report ' MKUTILS'
    call report ' MKUTILS /UNATTENDED'
    call report ' MKUTILS /DELETE'
    call report ' MKUTILS /DELETE /UNATTENDED'
    exit
  end
  if curarg = '/DELETE' then delete = 1
  else if curarg = '/UNATTENDED' then attended = 0
end

if attended = 1 then
   do
      if delete   = 1 then call report '     /DELETE     = TRUE'
                           call report '     /UNATTENDED = FALSE'
   end

parse source . . thispgm
thisdir           = left(thispgm, lastpos('\', thispgm) - 1)
if length(thisdir) = 2 then
   thisdir = thisdir || '\'
call directory thisdir

/* see if we might be in the right directory... */

/*
if delete = 1 then
   checkfile = 'setenv.cmd'
else
   checkfile = 'fm3.exe'
*/
checkfile = 'fm3.exe'
rc = stream(checkfile,'c','query exists')
if rc = '' then
do
  call report 'Sorry, 'checkfile' not found.  Must not be right directory.  Terminating.'
  call beep 100,100
  exit
end

/* load rexx utility functions */

call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

/* give user a chance to hit CTRL-C */

if attended then do
  call charout ,'  Press [Enter] to continue...'
  ans = ''
  do until ans = '0d'x
    ans = SysGetKey('NOECHO')
  end
  call charout ,'0d1b'x'[K'
end

if delete = 0 then
   call CreateCmds directory()
else
   call DeleteCmds

call report 'Done.'
n = endlocal()
exit


/* Subroutines */
DeleteCmds: procedure
   'if exist SETENV.CMD del SETENV.CMD  1>NUL 2>NUL'
   'if exist Utils\FM2.CMD del Utils\FM2.CMD  1>NUL 2>NUL'
   'if exist Utils\AV2.CMD del Utils\AV2.CMD 1>NUL 2>NUL'
   'if exist Utils\DIRSIZE.CMD del Utils\DIRSIZE.CMD 1>NUL 2>NUL'
   'if exist Utils\EAS.CMD del Utils\EAS.CMD 1>NUL 2>NUL'
   'if exist Utils\GLOBAL.CMD del Utils\GLOBAL.CMD 1>NUL 2>NUL'
   'if exist Utils\INI.CMD del Utils\INI.CMD 1>NUL 2>NUL'
   'if exist Utils\KILLPROC.CMD del Utils\KILLPROC.CMD 1>NUL 2>NUL'
   'if exist Utils\UNDEL.CMD del Utils\UNDEL.CMD 1>NUL 2>NUL'
   'if exist Utils\VCOLLECT.CMD del Utils\VCOLLECT.CMD 1>NUL 2>NUL'
   'if exist Utils\VDIR.CMD del Utils\VDIR.CMD 1>NUL 2>NUL'
   'if exist Utils\VIEWHELP.CMD del Utils\VIEWHELP.CMD 1>NUL 2>NUL'
   'if exist Utils\VIEWINFS.CMD del Utils\VIEWINFS.CMD 1>NUL 2>NUL'
   'if exist Utils\VTREE.CMD del Utils\VTREE.CMD 1>NUL 2>NUL'
   'rd Utils 1>NUL 2>NUL'
return

CreateCmds: procedure expose attended
   parse arg curdir
   /*
    * create command files that the user can execute from anywhere (we'll
    * ask the user to put this utils directory on the PATH) and that other
    * programs can execute to use FM/2 as "their" file manager.
    */
   /* say it, then do it */
   call report "Creating File Manager/2 folders and objects..."

   'del SETENV.CMD 1>NUL 2>NUL'
   rc = stream('SETENV.CMD','C','open')
   if rc = 'READY:' then
   do
     call lineout 'SETENV.CMD','@ECHO OFF'
     call lineout 'SETENV.CMD','REM'
     call lineout 'SETENV.CMD','REM If you prefer, you can call this .CMD file'
     call lineout 'SETENV.CMD','REM instead of altering the PATH= statement in'
     call lineout 'SETENV.CMD','REM CONFIG.SYS to gain access to the FM/2'
     call lineout 'SETENV.CMD','REM command line utilities.'
     call lineout 'SETENV.CMD','REM'
     call lineout 'SETENV.CMD','IF "%FM2ENVSET%" == "" GOTO INSTALL'
     call lineout 'SETENV.CMD','GOTO SKIP'
     call lineout 'SETENV.CMD',':INSTALL'
     call lineout 'SETENV.CMD','SET FM2ENVSET=YES'
     call lineout 'SETENV.CMD','SET PATH=%PATH%;'curdir'\Utils;'
     call lineout 'SETENV.CMD','SET BEGINLIBPATH='curdir
     call lineout 'SETENV.CMD','SET FM3INI='curdir
     call lineout 'SETENV.CMD','ECHO Path set for FM/2 and utilities.'
     call lineout 'SETENV.CMD','GOTO END'
     call lineout 'SETENV.CMD',':SKIP'
     call lineout 'SETENV.CMD','ECHO Paths already set for FM/2 and utilities.'
     call lineout 'SETENV.CMD',':END'
     call stream 'SETENV.CMD','C','close'
   end

   /*
    * place an object for SETENV.CMD in folder so user doesn't have to diddle
    * their PATH statement if they don't want to for command line work.
    */

   rc = stream('SETENV.CMD','c','query exists')
/*
   if rc \= '' then
   do
     shell = value('COMSPEC',,'OS2ENVIRONMENT')
     if shell = '' then shell = value('OS2_SHELL',,'OS2ENVIRONMENT')
     if shell = '' then shell = 'CMD.EXE'
     title = "FM/2 Utilities command line"
     classname = 'WPProgram'
     location = '<FM3_Tools>'
     setup = 'EXENAME='shell';PARAMETERS=/K 'rc';STARTUPDIR=?:\'
     call SysCreateObject classname,title,location,setup,'u'
   end
*/

   call SysMkDir curdir'\Utils'
   if translate(directory(curdir'\Utils')) \= translate(curdir'\Utils') then
     call report "Couldn't switch to "curdir"\Utils"
   else
   do
     'set PATH=%PATH%;'curdir'\Utils'
/*
     'move ..\example.cmd 1>NUL 2>NUL'
*/
     'del FM2.CMD 1>NUL 2>NUL'
     rc = stream('FM2.CMD','C','open')
     if rc = 'READY:' then
     do
       call report 'Creating an FM2.CMD file.'
       call lineout 'FM2.CMD', "/* FM/2 command file.  Locate in a directory"
       call lineout 'FM2.CMD', " * on your PATH. */"
       call lineout 'FM2.CMD', "'@echo off'"
         call lineout 'FM2.CMD', "mydir = directory()"
         call lineout 'FM2.CMD', "arg arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9"
         call lineout 'FM2.CMD', "if arg1 \= '' then "
         call lineout 'FM2.CMD', "do"
         call lineout 'FM2.CMD', "  if left(arg1,1,1) \= '/' then"
         call lineout 'FM2.CMD', "  do"
         call lineout 'FM2.CMD', "    arg0 = arg1"
         call lineout 'FM2.CMD', "    arg1 = stream(arg1,'c','query exists')"
         call lineout 'FM2.CMD', "    if arg1 = '' then"
         call lineout 'FM2.CMD', "    do"
         call lineout 'FM2.CMD', "      arg1 = directory(arg0)"
         call lineout 'FM2.CMD', "      call directory mydir"
         call lineout 'FM2.CMD', "    end"
         call lineout 'FM2.CMD', "  end"
         call lineout 'FM2.CMD', "end"
         call lineout 'FM2.CMD', "else arg1 = mydir"
         call lineout 'FM2.CMD', "if arg2 \= '' then "
         call lineout 'FM2.CMD', "do"
         call lineout 'FM2.CMD', "  if left(arg2,1,1) \= '/' then"
         call lineout 'FM2.CMD', "  do"
         call lineout 'FM2.CMD', "    arg0 = arg2"
         call lineout 'FM2.CMD', "    arg2 = stream(arg2,'c','query exists')"
         call lineout 'FM2.CMD', "    if arg2 = '' then"
         call lineout 'FM2.CMD', "    do"
         call lineout 'FM2.CMD', "      arg2 = directory(arg0)"
         call lineout 'FM2.CMD', "      call directory mydir"
         call lineout 'FM2.CMD', "    end"
         call lineout 'FM2.CMD', "  end"
         call lineout 'FM2.CMD', "end"
         call lineout 'FM2.CMD', "if arg3 \= '' then "
         call lineout 'FM2.CMD', "do"
         call lineout 'FM2.CMD', "  if left(arg3,1,1) \= '/' then"
         call lineout 'FM2.CMD', "  do"
         call lineout 'FM2.CMD', "    arg0 = arg3"
         call lineout 'FM2.CMD', "    arg3 = stream(arg3,'c','query exists')"
         call lineout 'FM2.CMD', "    if arg3 = '' then"
         call lineout 'FM2.CMD', "    do"
         call lineout 'FM2.CMD', "      arg3 = directory(arg0)"
         call lineout 'FM2.CMD', "      call directory mydir"
         call lineout 'FM2.CMD', "    end"
         call lineout 'FM2.CMD', "  end"
         call lineout 'FM2.CMD', "end"
         call lineout 'FM2.CMD', "if arg4 \= '' then "
         call lineout 'FM2.CMD', "do"
         call lineout 'FM2.CMD', "  if left(arg4,1,1) \= '/' then"
         call lineout 'FM2.CMD', "  do"
         call lineout 'FM2.CMD', "    arg0 = arg4"
         call lineout 'FM2.CMD', "    arg4 = stream(arg4,'c','query exists')"
         call lineout 'FM2.CMD', "    if arg4 = '' then"
         call lineout 'FM2.CMD', "    do"
         call lineout 'FM2.CMD', "      arg4 = directory(arg0)"
         call lineout 'FM2.CMD', "      call directory mydir"
         call lineout 'FM2.CMD', "    end"
         call lineout 'FM2.CMD', "  end"
         call lineout 'FM2.CMD', "end"
         call lineout 'FM2.CMD', "if arg5 \= '' then "
         call lineout 'FM2.CMD', "do"
         call lineout 'FM2.CMD', "  if left(arg5,1,1) \= '/' then"
         call lineout 'FM2.CMD', "  do"
         call lineout 'FM2.CMD', "    arg0 = arg5"
         call lineout 'FM2.CMD', "    arg5 = stream(arg5,'c','query exists')"
         call lineout 'FM2.CMD', "    if arg5 = '' then"
         call lineout 'FM2.CMD', "    do"
         call lineout 'FM2.CMD', "      arg5 = directory(arg0)"
         call lineout 'FM2.CMD', "      call directory mydir"
         call lineout 'FM2.CMD', "    end"
         call lineout 'FM2.CMD', "  end"
         call lineout 'FM2.CMD', "end"
         call lineout 'FM2.CMD', "if arg6 \= '' then "
         call lineout 'FM2.CMD', "do"
         call lineout 'FM2.CMD', "  if left(arg6,1,1) \= '/' then"
         call lineout 'FM2.CMD', "  do"
         call lineout 'FM2.CMD', "    arg0 = arg6"
         call lineout 'FM2.CMD', "    arg6 = stream(arg6,'c','query exists')"
         call lineout 'FM2.CMD', "    if arg6 = '' then"
         call lineout 'FM2.CMD', "    do"
         call lineout 'FM2.CMD', "      arg6 = directory(arg0)"
         call lineout 'FM2.CMD', "      call directory mydir"
         call lineout 'FM2.CMD', "    end"
         call lineout 'FM2.CMD', "  end"
         call lineout 'FM2.CMD', "end"
         call lineout 'FM2.CMD', "if arg7 \= '' then "
         call lineout 'FM2.CMD', "do"
         call lineout 'FM2.CMD', "  if left(arg7,1,1) \= '/' then"
         call lineout 'FM2.CMD', "  do"
         call lineout 'FM2.CMD', "    arg0 = arg7"
         call lineout 'FM2.CMD', "    arg7 = stream(arg7,'c','query exists')"
         call lineout 'FM2.CMD', "    if arg7 = '' then"
         call lineout 'FM2.CMD', "    do"
         call lineout 'FM2.CMD', "      arg7 = directory(arg0)"
         call lineout 'FM2.CMD', "      call directory mydir"
         call lineout 'FM2.CMD', "    end"
         call lineout 'FM2.CMD', "  end"
         call lineout 'FM2.CMD', "end"
         call lineout 'FM2.CMD', "if arg8 \= '' then "
         call lineout 'FM2.CMD', "do"
         call lineout 'FM2.CMD', "  if left(arg8,1,1) \= '/' then"
         call lineout 'FM2.CMD', "  do"
         call lineout 'FM2.CMD', "    arg0 = arg8"
         call lineout 'FM2.CMD', "    arg8 = stream(arg8,'c','query exists')"
         call lineout 'FM2.CMD', "    if arg8 = '' then"
         call lineout 'FM2.CMD', "    do"
         call lineout 'FM2.CMD', "      arg8 = directory(arg0)"
         call lineout 'FM2.CMD', "      call directory mydir"
         call lineout 'FM2.CMD', "    end"
         call lineout 'FM2.CMD', "  end"
         call lineout 'FM2.CMD', "end"
         call lineout 'FM2.CMD', "if arg9 \= '' then "
         call lineout 'FM2.CMD', "do"
         call lineout 'FM2.CMD', "  if left(arg9,1,1) \= '/' then"
         call lineout 'FM2.CMD', "  do"
         call lineout 'FM2.CMD', "    arg0 = arg9"
         call lineout 'FM2.CMD', "    arg9 = stream(arg9,'c','query exists')"
         call lineout 'FM2.CMD', "    if arg9 = '' then"
         call lineout 'FM2.CMD', "    do"
         call lineout 'FM2.CMD', "      arg9 = directory(arg0)"
         call lineout 'FM2.CMD', "      call directory mydir"
         call lineout 'FM2.CMD', "    end"
         call lineout 'FM2.CMD', "  end"
         call lineout 'FM2.CMD', "end"
       call lineout 'FM2.CMD', "n = setlocal()"
       call lineout 'FM2.CMD', "n = directory('"curdir"')"
       call lineout 'FM2.CMD', "'start fm3.exe 'arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9"
       call lineout 'FM2.CMD', "n = endlocal()"
       call stream 'FM2.CMD','C','close'
       'del AV2.CMD 1>NUL 2>NUL'
       rc = stream('AV2.CMD','C','open')
       if rc = 'READY:' then
       do
         call report 'Creating an AV2.CMD file.'
         call lineout 'AV2.CMD', "@echo off"
         call lineout 'AV2.CMD', "rem AV/2 command file.  Locate in a directory"
         call lineout 'AV2.CMD', "rem on your PATH."
         call lineout 'AV2.CMD', "setlocal"
         call lineout 'AV2.CMD', "set FM3INI="curdir
         call lineout 'AV2.CMD', "set BEGINLIBPATH="curdir
         call lineout 'AV2.CMD', "start "curdir"\FM2Tools\av2.exe %1 %2 %3 %4 %5 %6 %7 %8 %9"
         call lineout 'AV2.CMD', "endlocal"
         call stream 'AV2.CMD','C','close'
       end
       else call report "Couldn't create AV2.CMD file."
       'del VDIR.CMD 1>NUL 2>NUL'
       rc = stream('VDIR.CMD','C','open')
       if rc = 'READY:' then
       do
         call report 'Creating a VDIR.CMD file.'
         call lineout 'VDIR.CMD', "/* VDIR (FM/2) command file.  Locate in a directory"
         call lineout 'VDIR.CMD', " * on your PATH. */"
         call lineout 'VDIR.CMD', "'@echo off'"
         call lineout 'VDIR.CMD', "mydir = directory()"
         call lineout 'VDIR.CMD', "arg arg1 arg2"
         call lineout 'VDIR.CMD', "if arg1 \= '' then "
         call lineout 'VDIR.CMD', "do"
         call lineout 'VDIR.CMD', "  if left(arg1,1,1) \= '/' then"
         call lineout 'VDIR.CMD', "  do"
         call lineout 'VDIR.CMD', "    arg0 = arg1"
         call lineout 'VDIR.CMD', "    arg1 = stream(arg1,'c','query exists')"
         call lineout 'VDIR.CMD', "    if arg1 = '' then"
         call lineout 'VDIR.CMD', "    do"
         call lineout 'VDIR.CMD', "      arg1 = directory(arg0)"
         call lineout 'VDIR.CMD', "      call directory mydir"
         call lineout 'VDIR.CMD', "    end"
         call lineout 'VDIR.CMD', "  end"
         call lineout 'VDIR.CMD', "end"
         call lineout 'VDIR.CMD', "else arg1 = mydir"
         call lineout 'VDIR.CMD', "n = setlocal()"
         call lineout 'VDIR.CMD', "n = directory('"curdir"')"
         call lineout 'VDIR.CMD', "'start FM2Tools\vdir.exe 'arg1 '%2 %3 %4 %5 %6 %7 %8 %9'"
         call lineout 'VDIR.CMD', "n = endlocal()"
         call stream 'VDIR.CMD','C','close'
       end
       else call report "Couldn't create VDIR.CMD file."
       'del VTREE.CMD 1>NUL 2>NUL'
       rc = stream('VTREE.CMD','C','open')
       if rc = 'READY:' then
       do
         call report 'Creating a VTREE.CMD file.'
         call lineout 'VTREE.CMD', "/* VTREE (FM/2) command file.  Locate in a directory"
         call lineout 'VTREE.CMD', " * on your PATH. */"
         call lineout 'VTREE.CMD', "'@echo off'"
         call lineout 'VTREE.CMD', "n = setlocal()"
         call lineout 'VTREE.CMD', "n = directory('"curdir"')"
         call lineout 'VTREE.CMD', "'start FM2Tools\vtree.exe %1 %2 %3 %4 %5 %6 %7 %8 %9'"
         call lineout 'VTREE.CMD', "n = endlocal()"
         call stream 'VTREE.CMD','C','close'
       end
       else call report "Couldn't create VTREE.CMD file."
       'del VCOLLECT.CMD 1>NUL 2>NUL'
       rc = stream('VCOLLECT.CMD','C','open')
       if rc = 'READY:' then
       do
         call report 'Creating a VCOLLECT.CMD file.'
         call lineout 'VCOLLECT.CMD', "/* VCOLLECT (FM/2) command file.  Locate in a directory"
         call lineout 'VCOLLECT.CMD', " * on your PATH. */"
         call lineout 'VCOLLECT.CMD', "'@echo off'"
         call lineout 'VCOLLECT.CMD', "arg arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9"
         call lineout 'VCOLLECT.CMD', "if arg1 \= '' then "
         call lineout 'VCOLLECT.CMD', "do"
         call lineout 'VCOLLECT.CMD', "  if left(arg1,1,1) \= '/' then"
         call lineout 'VCOLLECT.CMD', "  do"
         call lineout 'VCOLLECT.CMD', "    arg1 = stream(arg1,'c','query exists')"
         call lineout 'VCOLLECT.CMD', "  end"
         call lineout 'VCOLLECT.CMD', "end"
         call lineout 'VCOLLECT.CMD', "if arg2 \= '' then "
         call lineout 'VCOLLECT.CMD', "do"
         call lineout 'VCOLLECT.CMD', "  if left(arg2,1,1) \= '/' then"
         call lineout 'VCOLLECT.CMD', "  do"
         call lineout 'VCOLLECT.CMD', "    arg2 = stream(arg2,'c','query exists')"
         call lineout 'VCOLLECT.CMD', "  end"
         call lineout 'VCOLLECT.CMD', "end"
         call lineout 'VCOLLECT.CMD', "if arg3 \= '' then "
         call lineout 'VCOLLECT.CMD', "do"
         call lineout 'VCOLLECT.CMD', "  if left(arg3,1,1) \= '/' then"
         call lineout 'VCOLLECT.CMD', "  do"
         call lineout 'VCOLLECT.CMD', "    arg3 = stream(arg3,'c','query exists')"
         call lineout 'VCOLLECT.CMD', "  end"
         call lineout 'VCOLLECT.CMD', "end"
         call lineout 'VCOLLECT.CMD', "if arg4 \= '' then "
         call lineout 'VCOLLECT.CMD', "do"
         call lineout 'VCOLLECT.CMD', "  if left(arg4,1,1) \= '/' then"
         call lineout 'VCOLLECT.CMD', "  do"
         call lineout 'VCOLLECT.CMD', "    arg4 = stream(arg4,'c','query exists')"
         call lineout 'VCOLLECT.CMD', "  end"
         call lineout 'VCOLLECT.CMD', "end"
         call lineout 'VCOLLECT.CMD', "if arg5 \= '' then "
         call lineout 'VCOLLECT.CMD', "do"
         call lineout 'VCOLLECT.CMD', "  if left(arg5,1,1) \= '/' then"
         call lineout 'VCOLLECT.CMD', "  do"
         call lineout 'VCOLLECT.CMD', "    arg5 = stream(arg5,'c','query exists')"
         call lineout 'VCOLLECT.CMD', "  end"
         call lineout 'VCOLLECT.CMD', "end"
         call lineout 'VCOLLECT.CMD', "if arg6 \= '' then "
         call lineout 'VCOLLECT.CMD', "do"
         call lineout 'VCOLLECT.CMD', "  if left(arg6,1,1) \= '/' then"
         call lineout 'VCOLLECT.CMD', "  do"
         call lineout 'VCOLLECT.CMD', "    arg6 = stream(arg6,'c','query exists')"
         call lineout 'VCOLLECT.CMD', "  end"
         call lineout 'VCOLLECT.CMD', "end"
         call lineout 'VCOLLECT.CMD', "if arg7 \= '' then "
         call lineout 'VCOLLECT.CMD', "do"
         call lineout 'VCOLLECT.CMD', "  if left(arg7,1,1) \= '/' then"
         call lineout 'VCOLLECT.CMD', "  do"
         call lineout 'VCOLLECT.CMD', "    arg7 = stream(arg7,'c','query exists')"
         call lineout 'VCOLLECT.CMD', "  end"
         call lineout 'VCOLLECT.CMD', "end"
         call lineout 'VCOLLECT.CMD', "if arg8 \= '' then "
         call lineout 'VCOLLECT.CMD', "do"
         call lineout 'VCOLLECT.CMD', "  if left(arg8,1,1) \= '/' then"
         call lineout 'VCOLLECT.CMD', "  do"
         call lineout 'VCOLLECT.CMD', "    arg8 = stream(arg8,'c','query exists')"
         call lineout 'VCOLLECT.CMD', "  end"
         call lineout 'VCOLLECT.CMD', "end"
         call lineout 'VCOLLECT.CMD', "if arg9 \= '' then "
         call lineout 'VCOLLECT.CMD', "do"
         call lineout 'VCOLLECT.CMD', "  if left(arg9,1,1) \= '/' then"
         call lineout 'VCOLLECT.CMD', "  do"
         call lineout 'VCOLLECT.CMD', "    arg9 = stream(arg9,'c','query exists')"
         call lineout 'VCOLLECT.CMD', "  end"
         call lineout 'VCOLLECT.CMD', "end"
         call lineout 'VCOLLECT.CMD', "n = setlocal()"
         call lineout 'VCOLLECT.CMD', "n = directory('"curdir"')"
         call lineout 'VCOLLECT.CMD', "'start FM2Tools\vcollect.exe 'arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9"
         call lineout 'VCOLLECT.CMD', "n = endlocal()"
         call stream 'VCOLLECT.CMD','C','close'
       end
       else call report "Couldn't create VCOLLECT.CMD file."
       'del INI.CMD 1>NUL 2>NUL'
       rc = stream('INI.CMD','C','open')
       if rc = 'READY:' then
       do
         call report 'Creating an INI.CMD file.'
         call lineout 'INI.CMD', "/* INI (FM/2) command file.  Locate in a directory"
         call lineout 'INI.CMD', " * on your PATH. */"
         call lineout 'INI.CMD', "'@echo off'"
         call lineout 'INI.CMD', "arg arg1"
         call lineout 'INI.CMD', "if arg1 \= '' then arg1 = stream(arg1,'c','query exists')"
         call lineout 'INI.CMD', "n = setlocal()"
         call lineout 'INI.CMD', "n = directory('"curdir"')"
         call lineout 'INI.CMD', "'start FM2Tools\INI.exe 'arg1"
         call lineout 'INI.CMD', "n = endlocal()"
         call stream 'INI.CMD','C','close'
       end
       else call report "Couldn't create INI.CMD file."
       'del EAS.CMD 1>NUL 2>NUL'
       rc = stream('EAS.CMD','C','open')
       if rc = 'READY:' then
       do
         call report 'Creating an EAS.CMD file.'
         call lineout 'EAS.CMD', "/* EAS (FM/2) command file.  Locate in a directory"
         call lineout 'EAS.CMD', " * on your PATH. */"
         call lineout 'EAS.CMD', "'@echo off'"
         call lineout 'EAS.CMD', "arg arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9"
         call lineout 'EAS.CMD', "if arg1 \= '' then "
         call lineout 'EAS.CMD', "do"
         call lineout 'EAS.CMD', "  if left(arg1,1,1) \= '/' then"
         call lineout 'EAS.CMD', "  do"
         call lineout 'EAS.CMD', "    arg1 = stream(arg1,'c','query exists')"
         call lineout 'EAS.CMD', "  end"
         call lineout 'EAS.CMD', "end"
         call lineout 'EAS.CMD', "else arg1 = directory()"
         call lineout 'EAS.CMD', "if arg2 \= '' then "
         call lineout 'EAS.CMD', "do"
         call lineout 'EAS.CMD', "  if left(arg2,1,1) \= '/' then"
         call lineout 'EAS.CMD', "  do"
         call lineout 'EAS.CMD', "    arg2 = stream(arg2,'c','query exists')"
         call lineout 'EAS.CMD', "  end"
         call lineout 'EAS.CMD', "end"
         call lineout 'EAS.CMD', "if arg3 \= '' then "
         call lineout 'EAS.CMD', "do"
         call lineout 'EAS.CMD', "  if left(arg3,1,1) \= '/' then"
         call lineout 'EAS.CMD', "  do"
         call lineout 'EAS.CMD', "    arg3 = stream(arg3,'c','query exists')"
         call lineout 'EAS.CMD', "  end"
         call lineout 'EAS.CMD', "end"
         call lineout 'EAS.CMD', "if arg4 \= '' then "
         call lineout 'EAS.CMD', "do"
         call lineout 'EAS.CMD', "  if left(arg4,1,1) \= '/' then"
         call lineout 'EAS.CMD', "  do"
         call lineout 'EAS.CMD', "    arg4 = stream(arg4,'c','query exists')"
         call lineout 'EAS.CMD', "  end"
         call lineout 'EAS.CMD', "end"
         call lineout 'EAS.CMD', "if arg5 \= '' then "
         call lineout 'EAS.CMD', "do"
         call lineout 'EAS.CMD', "  if left(arg5,1,1) \= '/' then"
         call lineout 'EAS.CMD', "  do"
         call lineout 'EAS.CMD', "    arg5 = stream(arg5,'c','query exists')"
         call lineout 'EAS.CMD', "  end"
         call lineout 'EAS.CMD', "end"
         call lineout 'EAS.CMD', "if arg6 \= '' then "
         call lineout 'EAS.CMD', "do"
         call lineout 'EAS.CMD', "  if left(arg6,1,1) \= '/' then"
         call lineout 'EAS.CMD', "  do"
         call lineout 'EAS.CMD', "    arg6 = stream(arg6,'c','query exists')"
         call lineout 'EAS.CMD', "  end"
         call lineout 'EAS.CMD', "end"
         call lineout 'EAS.CMD', "if arg7 \= '' then "
         call lineout 'EAS.CMD', "do"
         call lineout 'EAS.CMD', "  if left(arg7,1,1) \= '/' then"
         call lineout 'EAS.CMD', "  do"
         call lineout 'EAS.CMD', "    arg7 = stream(arg7,'c','query exists')"
         call lineout 'EAS.CMD', "  end"
         call lineout 'EAS.CMD', "end"
         call lineout 'EAS.CMD', "if arg8 \= '' then "
         call lineout 'EAS.CMD', "do"
         call lineout 'EAS.CMD', "  if left(arg8,1,1) \= '/' then"
         call lineout 'EAS.CMD', "  do"
         call lineout 'EAS.CMD', "    arg8 = stream(arg8,'c','query exists')"
         call lineout 'EAS.CMD', "  end"
         call lineout 'EAS.CMD', "end"
         call lineout 'EAS.CMD', "if arg9 \= '' then "
         call lineout 'EAS.CMD', "do"
         call lineout 'EAS.CMD', "  if left(arg9,1,1) \= '/' then"
         call lineout 'EAS.CMD', "  do"
         call lineout 'EAS.CMD', "    arg9 = stream(arg9,'c','query exists')"
         call lineout 'EAS.CMD', "  end"
         call lineout 'EAS.CMD', "end"
         call lineout 'EAS.CMD', "n = setlocal()"
         call lineout 'EAS.CMD', "n = directory('"curdir"')"
         call lineout 'EAS.CMD', "'start FM2Tools\eas.exe 'arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9"
         call lineout 'EAS.CMD', "n = endlocal()"
         call stream 'EAS.CMD','C','close'
       end
       else call report "Couldn't create EAS.CMD file."
       'del DIRSIZE.CMD 1>NUL 2>NUL'
       rc = stream('DIRSIZE.CMD','C','open')
       if rc = 'READY:' then
       do
         call report 'Creating a DIRSIZE.CMD file.'
         call lineout 'DIRSIZE.CMD', "/* DIRSIZE (FM/2) command file.  Locate in a directory"
         call lineout 'DIRSIZE.CMD', " * on your PATH. */"
         call lineout 'DIRSIZE.CMD', "'@echo off'"
         call lineout 'DIRSIZE.CMD', "mydir = directory()"
         call lineout 'DIRSIZE.CMD', "arg arg1 arg2"
         call lineout 'DIRSIZE.CMD', "if arg1 \= '' then "
         call lineout 'DIRSIZE.CMD', "do"
         call lineout 'DIRSIZE.CMD', "  if left(arg1,1,1) \= '/' then"
         call lineout 'DIRSIZE.CMD', "  do"
         call lineout 'DIRSIZE.CMD', "    arg0 = arg1"
         call lineout 'DIRSIZE.CMD', "    arg1 = stream(arg1,'c','query exists')"
         call lineout 'DIRSIZE.CMD', "    if arg1 = '' then"
         call lineout 'DIRSIZE.CMD', "    do"
         call lineout 'DIRSIZE.CMD', "      arg1 = directory(arg0)"
         call lineout 'DIRSIZE.CMD', "      call directory mydir"
         call lineout 'DIRSIZE.CMD', "    end"
         call lineout 'DIRSIZE.CMD', "  end"
         call lineout 'DIRSIZE.CMD', "end"
         call lineout 'DIRSIZE.CMD', "else arg1 = mydir"
         call lineout 'DIRSIZE.CMD', "n = setlocal()"
         call lineout 'DIRSIZE.CMD', "n = directory('"curdir"')"
         call lineout 'DIRSIZE.CMD', "'start FM2Tools\dirsize.exe 'arg1 '%2 %3 %4 %5 %6 %7 %8 %9'"
         call lineout 'DIRSIZE.CMD', "n = endlocal()"
         call stream 'DIRSIZE.CMD','C','close'
       end
       else call report "Couldn't create DIRSIZE.CMD file."
       'del UNDEL.CMD 1>NUL 2>NUL'
       rc = stream('UNDEL.CMD','C','open')
       if rc = 'READY:' then
       do
         call report 'Creating an UNDEL.CMD file.'
         call lineout 'UNDEL.CMD', "/* UNDEL (FM/2) command file.  Locate in a directory"
         call lineout 'UNDEL.CMD', " * on your PATH. */"
         call lineout 'UNDEL.CMD', "'@echo off'"
         call lineout 'UNDEL.CMD', "n = setlocal()"
         call lineout 'UNDEL.CMD', "n = directory('"curdir"')"
         call lineout 'UNDEL.CMD', "'start FM2Tools\undel.exe %1'"
         call lineout 'UNDEL.CMD', "n = endlocal()"
         call stream 'UNDEL.CMD','C','close'
       end
       else call report "Couldn't create UNDEL.CMD file."
       'del KILLPROC.CMD 1>NUL 2>NUL'
       rc = stream('KILLPROC.CMD','C','open')
       if rc = 'READY:' then
       do
         call report 'Creating a KILLPROC.CMD file.'
         call lineout 'KILLPROC.CMD', "/* KILLPROC (FM/2) command file.  Locate in a directory"
         call lineout 'KILLPROC.CMD', " * on your PATH. */"
         call lineout 'KILLPROC.CMD', "'@echo off'"
         call lineout 'KILLPROC.CMD', "n = setlocal()"
         call lineout 'KILLPROC.CMD', "n = directory('"curdir"')"
         call lineout 'KILLPROC.CMD', "'start FM2Tools\killproc.exe'"
         call lineout 'KILLPROC.CMD', "n = endlocal()"
         call stream 'KILLPROC.CMD','C','close'
       end
       else call report "Couldn't create KILLPROC.CMD file."
       'del VIEWINFS.CMD 1>NUL 2>NUL'
       rc = stream('VIEWINFS.CMD','C','open')
       if rc = 'READY:' then
       do
         call report 'Creating a VIEWINFS.CMD file.'
         call lineout 'VIEWINFS.CMD', "/* VIEWINFS (FM/2) command file.  Locate in a directory"
         call lineout 'VIEWINFS.CMD', " * on your PATH. */"
         call lineout 'VIEWINFS.CMD', "'@echo off'"
         call lineout 'VIEWINFS.CMD', "n = setlocal()"
         call lineout 'VIEWINFS.CMD', "n = directory('"curdir"')"
         call lineout 'VIEWINFS.CMD', "'start FM2Tools\viewinfs.exe'"
         call lineout 'VIEWINFS.CMD', "n = endlocal()"
         call stream 'VIEWINFS.CMD','C','close'
       end
       else call report "Couldn't create VIEWINFS.CMD file."
       'del VIEWHELP.CMD 1>NUL 2>NUL'
       rc = stream('VIEWHELP.CMD','C','open')
       if rc = 'READY:' then
       do
         call report 'Creating a VIEWHELP.CMD file.'
         call lineout 'VIEWHELP.CMD', "/* VIEWHELP (FM/2) command file.  Locate in a directory"
         call lineout 'VIEWHELP.CMD', " * on your PATH. */"
         call lineout 'VIEWHELP.CMD', "'@echo off'"
         call lineout 'VIEWHELP.CMD', "n = setlocal()"
         call lineout 'VIEWHELP.CMD', "n = directory('"curdir"')"
         call lineout 'VIEWHELP.CMD', "'start FM2Tools\viewinfs.exe DUMMY'"
         call lineout 'VIEWHELP.CMD', "n = endlocal()"
         call stream 'VIEWHELP.CMD','C','close'
       end
       else call report "Couldn't create VIEWHELP.CMD file."
       'del GLOBAL.CMD 1>NUL 2>NUL'
       rc = stream('GLOBAL.CMD','C','open')
       if rc = 'READY:' then
       do
         call report 'Creating a GLOBAL.CMD file.'
         call lineout 'GLOBAL.CMD', "/* GLOBAL (FM/2) command file.  Locate in a directory"
         call lineout 'GLOBAL.CMD', " * on your PATH. */"
         call lineout 'GLOBAL.CMD', "'@echo off'"
         call lineout 'GLOBAL.CMD', "arg arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9"
         call lineout 'GLOBAL.CMD', "if arg1 \= '' then "
         call lineout 'GLOBAL.CMD', "do"
         call lineout 'GLOBAL.CMD', "  if left(arg1,1,1) \= '/' then"
         call lineout 'GLOBAL.CMD', "  do"
         call lineout 'GLOBAL.CMD', "    arg1 = stream(arg1,'c','query exists')"
         call lineout 'GLOBAL.CMD', "  end"
         call lineout 'GLOBAL.CMD', "end"
         call lineout 'GLOBAL.CMD', "else arg1 = directory()"
         call lineout 'GLOBAL.CMD', "if arg2 \= '' then "
         call lineout 'GLOBAL.CMD', "do"
         call lineout 'GLOBAL.CMD', "  if left(arg2,1,1) \= '/' then"
         call lineout 'GLOBAL.CMD', "  do"
         call lineout 'GLOBAL.CMD', "    arg2 = stream(arg2,'c','query exists')"
         call lineout 'GLOBAL.CMD', "  end"
         call lineout 'GLOBAL.CMD', "end"
         call lineout 'GLOBAL.CMD', "if arg3 \= '' then "
         call lineout 'GLOBAL.CMD', "do"
         call lineout 'GLOBAL.CMD', "  if left(arg3,1,1) \= '/' then"
         call lineout 'GLOBAL.CMD', "  do"
         call lineout 'GLOBAL.CMD', "    arg3 = stream(arg3,'c','query exists')"
         call lineout 'GLOBAL.CMD', "  end"
         call lineout 'GLOBAL.CMD', "end"
         call lineout 'GLOBAL.CMD', "if arg4 \= '' then "
         call lineout 'GLOBAL.CMD', "do"
         call lineout 'GLOBAL.CMD', "  if left(arg4,1,1) \= '/' then"
         call lineout 'GLOBAL.CMD', "  do"
         call lineout 'GLOBAL.CMD', "    arg4 = stream(arg4,'c','query exists')"
         call lineout 'GLOBAL.CMD', "  end"
         call lineout 'GLOBAL.CMD', "end"
         call lineout 'GLOBAL.CMD', "if arg5 \= '' then "
         call lineout 'GLOBAL.CMD', "do"
         call lineout 'GLOBAL.CMD', "  if left(arg5,1,1) \= '/' then"
         call lineout 'GLOBAL.CMD', "  do"
         call lineout 'GLOBAL.CMD', "    arg5 = stream(arg5,'c','query exists')"
         call lineout 'GLOBAL.CMD', "  end"
         call lineout 'GLOBAL.CMD', "end"
         call lineout 'GLOBAL.CMD', "if arg6 \= '' then "
         call lineout 'GLOBAL.CMD', "do"
         call lineout 'GLOBAL.CMD', "  if left(arg6,1,1) \= '/' then"
         call lineout 'GLOBAL.CMD', "  do"
         call lineout 'GLOBAL.CMD', "    arg6 = stream(arg6,'c','query exists')"
         call lineout 'GLOBAL.CMD', "  end"
         call lineout 'GLOBAL.CMD', "end"
         call lineout 'GLOBAL.CMD', "if arg7 \= '' then "
         call lineout 'GLOBAL.CMD', "do"
         call lineout 'GLOBAL.CMD', "  if left(arg7,1,1) \= '/' then"
         call lineout 'GLOBAL.CMD', "  do"
         call lineout 'GLOBAL.CMD', "    arg7 = stream(arg7,'c','query exists')"
         call lineout 'GLOBAL.CMD', "  end"
         call lineout 'GLOBAL.CMD', "end"
         call lineout 'GLOBAL.CMD', "if arg8 \= '' then "
         call lineout 'GLOBAL.CMD', "do"
         call lineout 'GLOBAL.CMD', "  if left(arg8,1,1) \= '/' then"
         call lineout 'GLOBAL.CMD', "  do"
         call lineout 'GLOBAL.CMD', "    arg8 = stream(arg8,'c','query exists')"
         call lineout 'GLOBAL.CMD', "  end"
         call lineout 'GLOBAL.CMD', "end"
         call lineout 'GLOBAL.CMD', "if arg9 \= '' then "
         call lineout 'GLOBAL.CMD', "do"
         call lineout 'GLOBAL.CMD', "  if left(arg9,1,1) \= '/' then"
         call lineout 'GLOBAL.CMD', "  do"
         call lineout 'GLOBAL.CMD', "    arg9 = stream(arg9,'c','query exists')"
         call lineout 'GLOBAL.CMD', "  end"
         call lineout 'GLOBAL.CMD', "end"
         call lineout 'GLOBAL.CMD', "n = setlocal()"
         call lineout 'GLOBAL.CMD', "n = directory('"curdir"')"
         call lineout 'GLOBAL.CMD', "'start FM2Tools\global.exe' arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9"
         call lineout 'GLOBAL.CMD', "n = endlocal()"
         call stream 'GLOBAL.CMD','C','close'
       end
       else call report "Couldn't create GLOBAL.CMD file."
       call report ""
     end
     else call report "Couldn't create FM2.CMD file.  Panic."
     call directory curdir
   end

   if attended = 1 then
      do
         /* type the install.dat file to show any critical info/notices */

         rc = stream('install.dat','c','query exists')
         if rc \= '' then
            do
              'type install.dat'
              'DEL INSTALL.DAT 1>NUL 2>NUL'
              if attended then
              do
                call charout ,'  Press [Enter] to continue...'
                call linein
   /*
                ans = ''
                do until ans = '0d'x
                  ans = SysGetKey('NOECHO')
                end
   */
                call charout ,'0d1b'x'[K'
              end
            end

      end
return

/* InstallFile(file, template) install file from template unless file already exists */

InstallFile: procedure

  parse arg sFile sTemplate

  rc = stream(sFile,'c','query exists')
  if rc = '' then do
    rc = stream(sTemplate,'c','query exists')
    if rc \= '' then do
      call report 'Creating a sample' sFile 'file for you.'
      'ren' sTemplate sFile '1>nul 2>nul'
    end
  end
  'del' sTemplate '1>nul 2>nul'

  return

  /* end InstallFile */

report: procedure expose attended
   if attended then
      do
         parse arg msg
         say msg
      end
   return

novalue:
   call report 'Uninitialized value: ' || condition('D') || ' on line: ' || sigl
   call report 'Lines: 'sourceline(sigl)
   say
   call report 'Exiting...'
   '@pause'
   exit
