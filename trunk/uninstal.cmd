/*****************************************************************************
 *                        FM/2 deinstallation program                        *
 *****************************************************************************
 *                                                                           *
 * This program removes the FM/2 folder and any OS2USER.INI records          *
 * for FM/2.  Optionally it will wipe all files and remove all directories   *
 * created by the FM/2 installation program.  Run from the FM/2 directory.   *
 *                                                                           *
 *****************************************************************************
 * For unattended deinstallation, call with /UNATTENDED as the first         *
 * argument.                                                                 *
 *****************************************************************************
 *                                                                           *
 *                        UNINSTAL /? for usage help.                        *
 *                                                                           *
 *****************************************************************************/

'@Echo off'
'cls'
'echo  ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿'
'echo  ³                   FM/2 Deinstallation Program                     ³'
'echo  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ'

/* see if we might be in the right directory... */

curdir = directory()

rc = stream('fm3.exe','c','query exists')
if rc = '' then
do
  say 'Sorry, FM3.EXE not found.  Must not be right directory.  Terminating.'
  exit
end

/* tell user what we're doing, give him a chance to hit CTRL-C */

say ''
say ' **WARNING:  This program uninstalls FM/2.'
say ''

parse upper arg dummy

if dummy = '/?' then
do
  say ''
  say 'Usage:  UNINSTAL [/UNATTENDED]'
  say ''
  say ' /UNATTENDED          don''t ask any questions; run without stopping.'
  say ''
  say 'Examples:'
  say ' UNINSTAL'
  say ' UNINSTAL /UNATTENDED'
  exit
end

if dummy = '/UNATTENDED' then unattended = ''
if unattended = '' then say '     /UNATTENDED = TRUE'

if unattended = 'UNATTENDED' then
do
  say 'Press [Enter] (or CTRL-C then [Enter] to abort).'
  Pull dummy .
  say ''
  /* to be extra safe... */
  if dummy \= '' then
  do
    say 'Something besides just [Enter] pressed -- aborting.'
    exit
  end
end

/* load rexx utility functions */

call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

say 'Working...'

/* remove FM/2 folder object */
call SysDestroyObject "<FM3_Folder>"

/* remove all FM/2-related information from user ini file */
call SysIni 'USER', 'FM/2'
call SysIni 'USER', 'FM2'
call SysIni 'USER', 'FM/3'
call SysIni 'USER', 'FM/4'

/* done */

dummy = ''
if unattended = 'UNATTENDED' then
do
  say ''
  say "Delete programs and directories "curdir
  say "and "curdir"\utils"
  say "as follows, and then you're done:"
  say "CD\"
  say "DEL "curdir"\UTILS\*"
  say "RMDIR "curdir"\UTILS"
  say "DEL "curdir"\*"
  say "RMDIR "curdir
  say ""
  say "Shall I perform these tasks for you?"
  parse upper pull dummy
end
else dummy = 'Y'

if left(dummy,1) = 'Y' then
do
  'CD\'
  '@ECHO ON'
  'ECHO Y | DEL 'curdir'\UTILS\* && RD 'curdir'\UTILS'
  'ECHO Y | DEL 'curdir'\* && RD 'curdir
  '@ECHO OFF'
end
else say "Okay, it's a simple enough operation that even a human can do it. :-)"

say ""
say "Don't forget to remove any PATH statements from CONFIG.SYS that you"
say "entered for FM/2 or its utilities."
say ""
say "I'm done now."
