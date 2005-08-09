/****************************************************************************
 *                        FM/2 installation program                         *
 *                                                                          *
 * This program creates folders to hold program and data objects,           *
 * then creates program objects for each executable.  It only needs to be   *
 * run once (unless you move the FM/2 directory -- see say notes at end).   *
 * Run this program in the FM/2 directory (where you unpacked the archive). *
 *                                                                          *
 * Note that if you place the FM/2 Utilities package into a directory named *
 * UTILS off of the FM/2 directory, this install program will create some   *
 * objects for you for some of those programs.                              *
 *                                                                          *
 * For unattended installation, call with /UNATTENDED as the first          *
 * argument.                                                                *
 *                                                                          *
 * To avoid any WPS associations being set, use the /NOASSOC argument.      *
 *                                                                          *
 * For usage help, run as INSTALL /?                                        *
 *                                                                          *
 * $Id$                    *
 ****************************************************************************/

/* Identify ourself */

'@echo off'
'cls'

say'     ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿'
say'     ³                    FM/2 Installation Program                      ³'
say'     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ'

assocfilter = ';ASSOCFILTER=*.ZIP,*.ARC,*.LZH,*.ARJ,*.ZOO,*.MO0,READ.ME,README,README.1ST,README.OS2,REGISTER.TXT'

/* check arguments and adjust as required */

parse upper arg dummy1 dummy2
if dummy1 = '/NOASSOC' then assocfilter = ''
if dummy2 = '/NOASSOC' then assocfilter = ''
if assocfilter = '' then say '     /NOASSOC = TRUE'

if dummy1 = '/UNATTENDED' then unattended = ''
if dummy2 = '/UNATTENDED' then unattended = ''
if unattended = '' then say '     /UNATTENDED = TRUE'

/* if user asked for usage help, give it */

if dummy1 = '/?' | dummy2 = '/?' then
do
  say ''
  say 'Usage:  INSTALL [/NOASSOC] [/UNATTENDED]'
  say ''
  say ' /NOASSOC             don''t set any WPS associations for FM/2 programs.'
  say ' /UNATTENDED          don''t ask any questions; run without stopping.'
  say ''
  say 'Examples:'
  say ' INSTALL'
  say ' INSTALL /NOASSOC'
  say ' INSTALL /UNATTENDED'
  say ' INSTALL /NOASSOC /UNATTENDED'
  exit
end

/* see if we might be in the right directory... */

rc = stream('fm3.exe','c','query exists')
if rc = '' then
do
  say 'Sorry, FM3.EXE not found.  Must not be right directory.  Terminating.'
  call beep 100,100
  exit
end

/* tell user what we're doing */

say ''
say 'This program creates objects for FM/2 and does some drudgework for you.'
say ''

/* load rexx utility functions */

call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

/* give user a chance to hit CTRL-C */

if unattended = 'UNATTENDED' then
do
  call charout ,'  Press [Enter] to continue...'
  dummy = ''
  do until dummy = '0d'x
    dummy = SysGetKey('NOECHO')
  end
  call charout ,'0d1b'x'[K'
end

/* save current directory */

curdir = directory()

/* say it, then do it */

say "Creating File Manager/2 folders and objects..."

/* first, create FM/2 folder */

title = "File Manager/2"
classname = 'WPFolder'
location = '<WP_DESKTOP>'
setup = ''
rc = stream('fm2fldr.ico','c','query exists')
if rc \= '' then setup = setup'ICONFILE='rc';'
rc = stream('fm2fldr2.ico','c','query exists')
if rc \= '' then setup = setup'ICONNFILE=1,'rc';'
setup = setup'OBJECTID=<FM3_Folder>'
result = SysCreateObject(classname,title,location,setup,f)
'DEL FM2FLDR.ICO 1>NUL 2>NUL'
'DEL FM2FLDR2.ICO 1>NUL 2>NUL'

if unattended = 'UNATTENDED' then
do
  if result = 0 then
  do
    assocfilter = ''
    existed = ''
    say ''
    say 'The File Manager/2 folder already exists.'
    call charout ,"Should I update the objects (it's painless)? (Y/N) "
    dummy = ''
    do forever
      dummy = SysGetKey('NOECHO')
      parse upper var dummy dummy
      if dummy = '1b'x then dummy = 'N'
      if dummy = '0d'x then dummy = 'Y'
      if dummy = 'N' then leave
      if dummy = 'Y' then leave
      call beep 262,10
    end
    call charout ,dummy
    say ''
    if dummy = 'N' then exit
  end
end
else
do
  if result = 0 then
  do
    assocfilter = ''
    existed = ''
    say 'Updating objects.'
  end
end

/* create objects in appropriate folders */

rc = stream('fm3.exe','c','query exists')
if rc \= '' then
do
  title = "FM/2"
  classname = 'WPProgram'
  location = '<FM3_Folder>'
  setup = 'EXENAME='rc';STARTUPDIR='curdir
  if existed = 'EXISTED' then setup = setup';PARAMETERS=%*'
  setup = setup';OBJECTID=<FM/2>'
  call SysCreateObject classname,title,location,setup,u
end

rc = stream('fm4.exe','c','query exists')
if rc \= '' then
do
  title = "FM/2 Lite"
  classname = 'WPProgram'
  location = '<FM3_Folder>'
  setup = 'EXENAME='rc';STARTUPDIR='curdir
  if existed = 'EXISTED' then setup = setup';PARAMETERS=%*'
  setup = setup';OBJECTID=<FM/2 LITE>'
  call SysCreateObject classname,title,location,setup,u
end

/* create toolbox folder in FM/2 folder */
rc = stream('toolbox.ico','c','query exists')
title = "FM/2 Tools"
classname = 'WPFolder'
location = '<FM3_Folder>'
setup = ''
if rc \= '' then setup = 'ICONFILE='rc';'
rc = stream('toolbox2.ico','c','query exists')
if rc \= '' then setup = setup'ICONNFILE=1,'rc';'
setup = setup'OBJECTID=<FM3_Tools>'
result = SysCreateObject(classname,title,location,setup,u)
'DEL TOOLBOX.ICO 1>NUL 2>NUL'
'DEL TOOLBOX2.ICO 1>NUL 2>NUL'

/* create documentation folder in FM/2 folder */
rc = stream('docs.ico','c','query exists')
title = "FM/2 Docs"
classname = 'WPFolder'
location = '<FM3_Folder>'
setup = ''
if rc \= '' then setup = 'ICONFILE='rc';'
rc = stream('docs2.ico','c','query exists')
if rc \= '' then setup = setup'ICONNFILE=1,'rc';'
setup = setup'OBJECTID=<FM3_Docs>'
result = SysCreateObject(classname,title,location,setup,u)
'DEL DOCS.ICO 1>NUL 2>NUL'
'DEL DOCS2.ICO 1>NUL 2>NUL'

rc = stream('av2.exe','c','query exists')
if rc \= '' then
do
  title = "Archive Viewer/2"
  classname = 'WPProgram'
  location = '<FM3_Tools>'
  setup = 'EXENAME='rc';STARTUPDIR='curdir''assocfilter
  if existed = 'EXISTED' then setup = setup';PARAMETERS=%*'
  setup = setup';OBJECTID=<FM/2_AV/2>'
  call SysCreateObject classname,title,location,setup,u
end

rc = stream('eas.exe','c','query exists')
if rc \= '' then
do
  title = "EA Viewer"
  classname = 'WPProgram'
  location = '<FM3_Tools>'
  setup = 'EXENAME='rc';STARTUPDIR='curdir
  if existed = 'EXISTED' then setup = setup';PARAMETERS=%*'
  setup = setup';OBJECTID=<FM/2_EAVIEW>'
  call SysCreateObject classname,title,location,setup,u
end

rc = stream('image.exe','c','query exists')
if rc = '' then rc = stream('utils\image.exe','c','query exists')
if rc \= '' then
do
  title = "Image Viewer"
  classname = 'WPProgram'
  location = '<FM3_Tools>'
  setup = 'EXENAME='rc';STARTUPDIR='curdir
  if existed = 'EXISTED' then setup = setup';PARAMETERS=%*'
  setup = setup';OBJECTID=<FM/2_IMAGEVIEW>'
  call SysCreateObject classname,title,location,setup,u
end

rc = stream('dirsize.exe','c','query exists')
if rc \= '' then
do
  title = "Dir Sizes"
  classname = 'WPProgram'
  location = '<FM3_Tools>'
  setup = 'EXENAME='rc';STARTUPDIR='curdir
  if existed = 'EXISTED' then setup = setup';PARAMETERS=%*'
  setup = setup';OBJECTID=<FM/2_DIRSIZE>'
  call SysCreateObject classname,title,location,setup,u
end

rc = stream('ini.exe','c','query exists')
if rc \= '' then
do
  if assocfilter \= '' then assocfilter = ';ASSOCFILTER=*.INI'
  title = "INI Viewer"
  classname = 'WPProgram'
  location = '<FM3_Tools>'
  setup = 'EXENAME='rc';STARTUPDIR='curdir''assocfilter
  if existed = 'EXISTED' then setup = setup';PARAMETERS=%*'
  setup = setup';OBJECTID=<FM/2_INIVIEW>'
  call SysCreateObject classname,title,location,setup,u
end

rc = stream('viewinfs.exe','c','query exists')
if rc \= '' then
do
  title = "Bookshelf Viewer"
  classname = 'WPProgram'
  location = '<FM3_Tools>'
  setup = 'EXENAME='rc';STARTUPDIR='curdir';OBJECTID=<FM/2_BOOKSHELF>'
  call SysCreateObject classname,title,location,setup,u
  title = "Helpfile Viewer"
  classname = 'WPProgram'
  location = '<FM3_Tools>'
  setup = 'EXENAME='rc';PARAMETERS=DUMMY;STARTUPDIR='curdir
  call SysCreateObject classname,title,location,setup,u
end

rc = stream('killproc.exe','c','query exists')
if rc \= '' then
do
  title = "Process Killer"
  classname = 'WPProgram'
  location = '<FM3_Tools>'
  setup = 'EXENAME='rc';PARAMETERS=%;STARTUPDIR='curdir';OBJECTID=<FM/2_KILLPROC>'
  call SysCreateObject classname,title,location,setup,u
end

rc = stream('undel.exe','c','query exists')
if rc \= '' then
do
  title = "Undeleter"
  classname = 'WPProgram'
  location = '<FM3_Tools>'
  setup = 'EXENAME='rc';STARTUPDIR='curdir';OBJECTID=<FM/2_UNDEL>'
  if existed = 'EXISTED' then setup = setup';PARAMETERS=%*'
  call SysCreateObject classname,title,location,setup,u
end

rc = stream('vtree.exe','c','query exists')
if rc \= '' then
do
  title = "Visual Tree"
  classname = 'WPProgram'
  location = '<FM3_Tools>'
  setup = 'EXENAME='rc';STARTUPDIR='curdir';OBJECTID=<FM/2_VTREE>'
  call SysCreateObject classname,title,location,setup,u
end

rc = stream('vdir.exe','c','query exists')
if rc \= '' then
do
  title = "Visual Directory"
  classname = 'WPProgram'
  location = '<FM3_Tools>'
  setup = 'EXENAME='rc';STARTUPDIR='curdir';OBJECTID=<FM/2_VDIR>'
  if existed = 'EXISTED' then setup = setup';PARAMETERS=%*'
  call SysCreateObject classname,title,location,setup,u
end

rc = stream('vcollect.exe','c','query exists')
if rc \= '' then
do
  title = "Collector"
  classname = 'WPProgram'
  location = '<FM3_Tools>'
  setup = 'EXENAME='rc';STARTUPDIR='curdir';OBJECTID=<FM/2_VCOLLECT>'
  call SysCreateObject classname,title,location,setup,u
  title = "Seek and scan"
  classname = 'WPProgram'
  location = '<FM3_Tools>'
  setup = 'EXENAME='rc';PARAMETERS=**;STARTUPDIR='curdir';OBJECTID=<FM/2_VSEEK>'
  call SysCreateObject classname,title,location,setup,u
end

rc = stream('global.exe','c','query exists')
if rc \= '' then
do
  title = "Global File Viewer"
  classname = 'WPProgram'
  location = '<FM3_Tools>'
  setup = 'EXENAME='rc';STARTUPDIR='curdir';OBJECTID=<FM/2_SEEALL>'
  call SysCreateObject classname,title,location,setup,u
end

rc = stream('databar.exe','c','query exists')
if rc \= '' then
do
  title = "Databar"
  classname = 'WPProgram'
  location = '<FM3_Tools>'
  setup = 'EXENAME='rc';STARTUPDIR='curdir';OBJECTID=<FM/2_DATABAR>'
  call SysCreateObject classname,title,location,setup,u
end

rc = stream('sysinfo.exe','c','query exists')
if rc \= '' then
/*
do
  title = "SysInfo"
  classname = 'WPProgram'
  location = '<FM3_Tools>'
  setup = 'EXENAME='rc';STARTUPDIR='curdir';OBJECTID=<FM/2_SYSINFO>'
  call SysCreateObject classname,title,location,setup,u
end
*/
do
  'del 'rc' 1>NUL 2>NUL'
   call SysDestroyObject '<FM/2_SYSINFO>'
end

rc = stream('fm2play.exe','c','query exists')
if rc = '' then rc = stream('utils\fm2play.exe','c','query exists')
if rc \= '' then
do
  title = "FM2Play"
  classname = 'WPProgram'
  location = '<FM3_Tools>'
  setup = 'EXENAME='rc';OBJECTID=<FM/2_FM2PLAY>;PARAMETERS=/! %*'
  call SysCreateObject classname,title,location,setup,u
end

rc = stream('makearc.exe','c','query exists')
if rc = '' then rc = stream('utils\makearc.exe','c','query exists')
if rc \= '' then
do
  title = "Make Archive"
  classname = 'WPProgram'
  location = '<FM3_Tools>'
  setup = 'EXENAME='rc';STARTUPDIR='curdir
  if existed = 'EXISTED' then setup = setup';PARAMETERS=%*'
  setup = setup';OBJECTID=<FM/2_MAKEARC>'
  call SysCreateObject classname,title,location,setup,u
end

rc = stream('READ.ME','c','query exists')
if rc \= '' then
do
  title = "READ.ME"
  classname = 'WPShadow'
  location = '<FM3_Docs>'
  setup = 'SHADOWID='rc
  call SysCreateObject classname,title,location,setup,u
end

rc = stream('FM3.HLP','c','query exists')
if rc \= '' then
do
  title = "FM/2 Online Help"
  classname = 'WPProgram'
  location = '<FM3_Docs>'
  setup = 'EXENAME='VIEW.EXE';STARTUPDIR='curdir';PARAMETERS='curdir'\FM3.HLP'
  call SysCreateObject classname,title,location,setup,u
end

rc = stream('UTILS\MAKEOBJ.CMD','c','query exists')
if rc \= '' then
do
  title = "Make Object"
  classname = 'WPProgram'
  location = '<FM3_Tools>'
  setup = 'EXENAME='rc';PARAMETERS=%*'
  call SysCreateObject classname,title,location,setup,u
end

rc = stream('UTILS\FM2UTILS.DOC','c','query exists')
if rc \= '' then
do
  title = "FM2UTILS.DOC"
  classname = 'WPShadow'
  location = '<FM3_Docs>'
  setup = 'SHADOWID='rc
  call SysCreateObject classname,title,location,setup,u
end

/* create sample customizations for the user so they don't start 'blank' */
/* but avoid overwriting any existing customizations */

rc = stream('FM3MENU.DAT','c','query exists')
if rc = '' then
do
  rc = stream('FM2MENU.TMP','c','query exists')
  if rc \= '' then
  do
    say 'Creating a sample FM3MENU.DAT file for you.'
    'REN FM3MENU.TMP FM3MENU.DAT 1>NUL 2>NUL'
  end
end
'DEL FM3MENU.TMP 1>NUL 2>NUL'
rc = stream('ASSOC.DAT','c','query exists')
if rc = '' then
do
  rc = stream('ASSOC.TMP','c','query exists')
  if rc \= '' then
  do
    say 'Creating a sample ASSOC.DAT file for you.'
    'REN ASSOC.TMP ASSOC.DAT 1>NUL 2>NUL'
  end
end
'DEL ASSOC.TMP 1>NUL 2>NUL'
rc = stream('COMMANDS.DAT','c','query exists')
if rc = '' then
do
  rc = stream('COMMANDS.TMP','c','query exists')
  if rc \= '' then
  do
    say 'Creating a sample COMMANDS.DAT file for you.'
    'REN COMMANDS.TMP COMMANDS.DAT 1>NUL 2>NUL'
  end
end
'DEL COMMANDS.TMP 1>NUL 2>NUL'
rc = stream('FILTERS.DAT','c','query exists')
if rc = '' then
do
  rc = stream('FILTERS.TMP','c','query exists')
  if rc \= '' then
  do
    say 'Creating a sample FILTERS.DAT file for you.'
    'REN FILTERS.TMP FILTERS.DAT 1>NUL 2>NUL'
  end
end
'DEL FILTERS.TMP 1>NUL 2>NUL'
rc = stream('FM3TOOLS.DAT','c','query exists')
if rc = '' then
do
  rc = stream('FM3TOOLS.TMP','c','query exists')
  if rc \= '' then
  do
    say 'Installing an FM3TOOLS.DAT file for you.'
    'REN FM3TOOLS.TMP FM3TOOLS.DAT 1>NUL 2>NUL'
  end
end
'DEL FM3TOOLS.TMP 1>NUL 2>NUL'
rc = stream('EXAMPLE.CMD','c','query exists')
if rc = '' then
do
  rc = stream('EXAMPLE.TMP','c','query exists')
  if rc \= '' then
  do
    say 'Creating a sample EXAMPLE.CMD file for you.'
    'REN EXAMPLE.TMP EXAMPLE.CMD 1>NUL 2>NUL'
  end
end
'DEL EXAMPLE.TMP 1>NUL 2>NUL'
rc = stream('HPFSOPT.CMD','c','query exists')
if rc = '' then
do
  rc = stream('HPFSOPT.TMP','c','query exists')
  if rc \= '' then
  do
    say 'Creating a sample HPFSOPT.CMD file for you.'
    'REN HPFSOPT.TMP HPFSOPT.CMD 1>NUL 2>NUL'
  end
end
'DEL HPFSOPT.TMP 1>NUL 2>NUL'
rc = stream('JFSOPT.CMD','c','query exists')
if rc = '' then
do
  rc = stream('JFSOPT.TMP','c','query exists')
  if rc \= '' then
  do
    say 'Creating a sample JFSOPT.CMD file for you.'
    'REN JFSOPT.TMP JFSOPT.CMD 1>NUL 2>NUL'
  end
end
'DEL JFSOPT.TMP 1>NUL 2>NUL'
rc = stream('FATOPT.CMD','c','query exists')
if rc = '' then
do
  rc = stream('FATOPT.TMP','c','query exists')
  if rc \= '' then
  do
    say 'Creating a sample FATOPT.CMD file for you.'
    'REN FATOPT.TMP FATOPT.CMD 1>NUL 2>NUL'
  end
end
'DEL FATOPT.TMP 1>NUL 2>NUL'

rc = stream('QUICKTLS.DAT','c','query exists')
if rc = '' then
do
  rc = stream('QUICKTLS.TMP','c','query exists')
  if rc \= '' then
  do
    rc = stream('CMDS.TMP','c','query exists')
    if rc \= '' then
    do
      rc = stream('UTILS.TMP','c','query exists')
      if rc \= '' then
      do
        rc = stream('SORT.TMP','c','query exists')
        if rc \= '' then
        do
          rc = stream('SELECT.TMP','c','query exists')
          if rc \= '' then
          do
            say 'Creating a sample QUICKTLS.DAT file and toolboxes for you.'
            'REN QUICKTLS.TMP QUICKTLS.DAT 1>NUL 2>NUL'
            'REN CMDS.TMP CMDS.TLS 1>NUL 2>NUL'
            'REN UTILS.TMP UTILS.TLS 1>NUL 2>NUL'
            'REN SORT.TMP SORT.TLS 1>NUL 2>NUL'
            'REN SELECT.TMP SELECT.TLS 1>NUL 2>NUL'
          end
        end
      end
    end
  end
end
'DEL SELECT.TMP 1>NUL 2>NUL'
'DEL SORT.TMP 1>NUL 2>NUL'
'DEL UTILS.TMP 1>NUL 2>NUL'
'DEL CMDS.TMP 1>NUL 2>NUL'
'DEL QUICKTLS.TMP 1>NUL 2>NUL'

/*
 * create command files that the user can execute from anywhere (we'll
 * ask the user to put this utils directory on the PATH) and that other
 * programs can execute to use FM/2 as "their" file manager.
 */

'del SETENV.CMD 1>NUL 2>NUL'
dummy = stream('SETENV.CMD','C','open')
if dummy = 'READY:' then
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
  call lineout 'SETENV.CMD','SET PATH=%PATH%;'curdir'\utils;'
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
if rc \= '' then
do
  dummy = value('COMSPEC',,'OS2ENVIRONMENT')
  if dummy = '' then dummy = value('OS2_SHELL',,'OS2ENVIRONMENT')
  if dummy = '' then dummy = 'CMD.EXE'
  title = "FM/2 Utilities command line"
  classname = 'WPProgram'
  location = '<FM3_Tools>'
  setup = 'EXENAME='dummy';PARAMETERS=/K 'rc';STARTUPDIR=C:\'
  call SysCreateObject classname,title,location,setup,u
end

call SysMkDir curdir'\UTILS'
dummy = directory(curdir'\UTILS')
if dummy = curdir'\UTILS' then
do
  'set PATH=%PATH%;'curdir'\utils'
  'move ..\example.cmd 1>NUL 2>NUL'
  'del FM2.CMD 1>NUL 2>NUL'
  dummy = stream('FM2.CMD','C','open')
  if dummy = 'READY:' then
  do
    say 'Creating an FM2.CMD file.'
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
    dummy = stream('AV2.CMD','C','open')
    if dummy = 'READY:' then
    do
      say 'Creating an AV2.CMD file.'
      call lineout 'AV2.CMD', "@echo off"
      call lineout 'AV2.CMD', "rem AV/2 command file.  Locate in a directory"
      call lineout 'AV2.CMD', "rem on your PATH."
      call lineout 'AV2.CMD', "setlocal"
      call lineout 'AV2.CMD', "set FM3INI="curdir
      call lineout 'AV2.CMD', "set BEGINLIBPATH="curdir
      call lineout 'AV2.CMD', "start "curdir"\av2.exe %1 %2 %3 %4 %5 %6 %7 %8 %9"
      call lineout 'AV2.CMD', "endlocal"
      call stream 'AV2.CMD','C','close'
    end
    else say "Couldn't create AV2.CMD file."
    'del VDIR.CMD 1>NUL 2>NUL'
    dummy = stream('VDIR.CMD','C','open')
    if dummy = 'READY:' then
    do
      say 'Creating a VDIR.CMD file.'
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
      call lineout 'VDIR.CMD', "'start vdir.exe 'arg1 '%2 %3 %4 %5 %6 %7 %8 %9'"
      call lineout 'VDIR.CMD', "n = endlocal()"
      call stream 'VDIR.CMD','C','close'
    end
    else say "Couldn't create VDIR.CMD file."
    'del VTREE.CMD 1>NUL 2>NUL'
    dummy = stream('VTREE.CMD','C','open')
    if dummy = 'READY:' then
    do
      say 'Creating a VTREE.CMD file.'
      call lineout 'VTREE.CMD', "/* VTREE (FM/2) command file.  Locate in a directory"
      call lineout 'VTREE.CMD', " * on your PATH. */"
      call lineout 'VTREE.CMD', "'@echo off'"
      call lineout 'VTREE.CMD', "n = setlocal()"
      call lineout 'VTREE.CMD', "n = directory('"curdir"')"
      call lineout 'VTREE.CMD', "'start vtree.exe %1 %2 %3 %4 %5 %6 %7 %8 %9'"
      call lineout 'VTREE.CMD', "n = endlocal()"
      call stream 'VTREE.CMD','C','close'
    end
    else say "Couldn't create VTREE.CMD file."
    'del VCOLLECT.CMD 1>NUL 2>NUL'
    dummy = stream('VCOLLECT.CMD','C','open')
    if dummy = 'READY:' then
    do
      say 'Creating a VCOLLECT.CMD file.'
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
      call lineout 'VCOLLECT.CMD', "'start vcollect.exe 'arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9"
      call lineout 'VCOLLECT.CMD', "n = endlocal()"
      call stream 'VCOLLECT.CMD','C','close'
    end
    else say "Couldn't create VCOLLECT.CMD file."
    'del INI.CMD 1>NUL 2>NUL'
    dummy = stream('INI.CMD','C','open')
    if dummy = 'READY:' then
    do
      say 'Creating an INI.CMD file.'
      call lineout 'INI.CMD', "/* INI (FM/2) command file.  Locate in a directory"
      call lineout 'INI.CMD', " * on your PATH. */"
      call lineout 'INI.CMD', "'@echo off'"
      call lineout 'INI.CMD', "arg arg1"
      call lineout 'INI.CMD', "if arg1 \= '' then arg1 = stream(arg1,'c','query exists')"
      call lineout 'INI.CMD', "n = setlocal()"
      call lineout 'INI.CMD', "n = directory('"curdir"')"
      call lineout 'INI.CMD', "'start INI.exe 'arg1"
      call lineout 'INI.CMD', "n = endlocal()"
      call stream 'INI.CMD','C','close'
    end
    else say "Couldn't create INI.CMD file."
    'del EAS.CMD 1>NUL 2>NUL'
    dummy = stream('EAS.CMD','C','open')
    if dummy = 'READY:' then
    do
      say 'Creating an EAS.CMD file.'
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
      call lineout 'EAS.CMD', "'start eas.exe 'arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9"
      call lineout 'EAS.CMD', "n = endlocal()"
      call stream 'EAS.CMD','C','close'
    end
    else say "Couldn't create EAS.CMD file."
    'del DIRSIZE.CMD 1>NUL 2>NUL'
    dummy = stream('DIRSIZE.CMD','C','open')
    if dummy = 'READY:' then
    do
      say 'Creating a DIRSIZE.CMD file.'
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
      call lineout 'DIRSIZE.CMD', "'start dirsize.exe 'arg1 '%2 %3 %4 %5 %6 %7 %8 %9'"
      call lineout 'DIRSIZE.CMD', "n = endlocal()"
      call stream 'DIRSIZE.CMD','C','close'
    end
    else say "Couldn't create DIRSIZE.CMD file."
    'del UNDEL.CMD 1>NUL 2>NUL'
    dummy = stream('UNDEL.CMD','C','open')
    if dummy = 'READY:' then
    do
      say 'Creating an UNDEL.CMD file.'
      call lineout 'UNDEL.CMD', "/* UNDEL (FM/2) command file.  Locate in a directory"
      call lineout 'UNDEL.CMD', " * on your PATH. */"
      call lineout 'UNDEL.CMD', "'@echo off'"
      call lineout 'UNDEL.CMD', "n = setlocal()"
      call lineout 'UNDEL.CMD', "n = directory('"curdir"')"
      call lineout 'UNDEL.CMD', "'start undel.exe %1'"
      call lineout 'UNDEL.CMD', "n = endlocal()"
      call stream 'UNDEL.CMD','C','close'
    end
    else say "Couldn't create UNDEL.CMD file."
    'del KILLPROC.CMD 1>NUL 2>NUL'
    dummy = stream('KILLPROC.CMD','C','open')
    if dummy = 'READY:' then
    do
      say 'Creating a KILLPROC.CMD file.'
      call lineout 'KILLPROC.CMD', "/* KILLPROC (FM/2) command file.  Locate in a directory"
      call lineout 'KILLPROC.CMD', " * on your PATH. */"
      call lineout 'KILLPROC.CMD', "'@echo off'"
      call lineout 'KILLPROC.CMD', "n = setlocal()"
      call lineout 'KILLPROC.CMD', "n = directory('"curdir"')"
      call lineout 'KILLPROC.CMD', "'start killproc.exe'"
      call lineout 'KILLPROC.CMD', "n = endlocal()"
      call stream 'KILLPROC.CMD','C','close'
    end
    else say "Couldn't create KILLPROC.CMD file."
    'del VIEWINFS.CMD 1>NUL 2>NUL'
    dummy = stream('VIEWINFS.CMD','C','open')
    if dummy = 'READY:' then
    do
      say 'Creating a VIEWINFS.CMD file.'
      call lineout 'VIEWINFS.CMD', "/* VIEWINFS (FM/2) command file.  Locate in a directory"
      call lineout 'VIEWINFS.CMD', " * on your PATH. */"
      call lineout 'VIEWINFS.CMD', "'@echo off'"
      call lineout 'VIEWINFS.CMD', "n = setlocal()"
      call lineout 'VIEWINFS.CMD', "n = directory('"curdir"')"
      call lineout 'VIEWINFS.CMD', "'start viewinfs.exe'"
      call lineout 'VIEWINFS.CMD', "n = endlocal()"
      call stream 'VIEWINFS.CMD','C','close'
    end
    else say "Couldn't create VIEWINFS.CMD file."
    'del VIEWHELP.CMD 1>NUL 2>NUL'
    dummy = stream('VIEWHELP.CMD','C','open')
    if dummy = 'READY:' then
    do
      say 'Creating a VIEWHELP.CMD file.'
      call lineout 'VIEWHELP.CMD', "/* VIEWHELP (FM/2) command file.  Locate in a directory"
      call lineout 'VIEWHELP.CMD', " * on your PATH. */"
      call lineout 'VIEWHELP.CMD', "'@echo off'"
      call lineout 'VIEWHELP.CMD', "n = setlocal()"
      call lineout 'VIEWHELP.CMD', "n = directory('"curdir"')"
      call lineout 'VIEWHELP.CMD', "'start viewinfs.exe DUMMY'"
      call lineout 'VIEWHELP.CMD', "n = endlocal()"
      call stream 'VIEWHELP.CMD','C','close'
    end
    else say "Couldn't create VIEWHELP.CMD file."
    'del GLOBAL.CMD 1>NUL 2>NUL'
    dummy = stream('GLOBAL.CMD','C','open')
    if dummy = 'READY:' then
    do
      say 'Creating a GLOBAL.CMD file.'
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
      call lineout 'GLOBAL.CMD', "'start global.exe' arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9"
      call lineout 'GLOBAL.CMD', "n = endlocal()"
      call stream 'GLOBAL.CMD','C','close'
    end
    else say "Couldn't create GLOBAL.CMD file."
    say ""
  end
  else say "Couldn't create FM2.CMD file.  Panic."
  dummy = directory(curdir)
end
else say "Couldn't switch to "curdir"\utils"

/* type the install.dat file to show any critical info/notices */

rc = stream('install.dat','c','query exists')
if rc \= '' then
do
  'type install.dat'
  'DEL INSTALL.DAT 1>NUL 2>NUL'
  if unattended = 'UNATTENDED' then
  do
    call charout ,'  Press [Enter] to continue...'
    dummy = ''
    do until dummy = '0d'x
      dummy = SysGetKey('NOECHO')
    end
    call charout ,'0d1b'x'[K'
  end
end

/* Final words */

say 'ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿'
say '³To move FM/2 to another directory, move the files, delete the FM/2 folder,³'
say '³then rerun INSTALL.  There is no need to "uninstall" to move FM/2.  See   ³'
say '³READ.ME for more information.                                             ³'
say '³To remove FM/2 completely, run UNINSTAL and follow the directions.        ³'
say 'ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ'
say "I'm done now."
say ''
