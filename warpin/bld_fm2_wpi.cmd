/*  BLD_FM2_WPI - Create a Warpin archive (WPI) for installation of FM/2 */

/*
 * Author
 *    John Small
 *    jsmall@os2world.net
 *
 * Requirements/assumptions
 *    -  This program should reside in the Warpin subdirectory of the
 *       the FM/2 build subtree.
 *    -  This program assumes that all files needed for the WPI file have
 *       been already placed in subdirectories of the directory which
 *       contains this file (i.e. BLD_FM2_WPIDIRS.CMD has been run).
 *    -  The WPI and WIS filenames can be passed as parameters:
 *          BLD_FM2_WPI <WPI-filename>
 *          BLD_FM2_WPI <WPI-filename> <WIS-filename>
 *       If the WIS filename is omitted, then the name used will use the
 *       same base name as the WPI filename with a 'WIS' extension.
 *
 * Todo/Issues/Questions:
 *    -  Archive previous WPI file
 *       - Yes or No (i.e. just replace it)?
 *       - How?
 *          Just Rename with timestamp or zip it, too
 *          What kind of timestamp (full (YYMMDDHHMMSS) or secs since midnight (SSSSS) or other?)
 *    -  Delete files after they have been added to the WPI archive?
 *       (This would keep obsolete files from being added to future
 *       archives.)
 *    -  Merge BLD_FM2_WPIDIRS or call BLD_FM2_WPIDIRS from this program?
 *    -  Is use of BEGIN/ENDLIBPATH OK to ensure access to WPIRTL.DLL?
 *       IOW, require FM/2 builders to have a new enough version of OS/2 or
 *       eCS which includes support for the extended LIBPATHs?
 *
 */

archive_previous_WPI       = 0
delete_files_afterwards    = 1

n = setlocal()

globals                    = 'WPI.'
WPI.                       = ''
WPI.default_archivename    = 'fm2.wpi'

/*
signal on novalue             /* for debugging */
*/
signal on Error
signal on FAILURE name Error
signal on Halt
signal on NOTREADY name Error
signal on NOVALUE name Error
signal on SYNTAX name Error

call RxFuncAdd 'SysLoadFuncs', 'REXXUTIL', 'SysLoadFuncs'
call SysLoadFuncs

/*
 * Init
 *    -  Initializes global variables
 *    -  Ensures that the current directory is set to the
 *       directory where this file is located
 *    -  Ensures that WIC (the Warpin WPI building utility)
 *       is runnable (WIC.EXE on the PATH and WPITRL.DLL on the
 *       extended LIBPATH).
 */
parse arg args
call Init strip(args)

if WPI.scriptonly == 0 then
/* Add the files to the WPI   */
   do
      if stream(WPI.archivename, 'c', 'query exists') \= '' then
         if archive_previous_WPI == 1 then
            'ren 'WPI.archivename WPI.archivename || '.' || time('S')
         else
            'del 'WPI.archivename
      do p = 1 to WPI.pkg.0
         call SysFileTree WPI.pkg.p.dir || '\*', 'pkgfilelist.', 'FOS'
         if pkgfilelist.0 = 0 then
            WPI.WIC_pgm WPI.archivename' -a 'WPI.pkg.p.number' NUL'
         else
            WPI.WIC_pgm WPI.archivename' -a 'WPI.pkg.p.number' -r -c'WPI.pkg.p.dir' *'
         if rc \= 0 then
            call ErrorExit 3 rc
      end
      if delete_files_afterwards = 1 then
         do pkgnum = 1 to WPI.pkg.0
            call EmptyDir WPI.pkg.pkgnum.dir
         end
   end

/* Add the script file to the WPI   */
/*
'eautil 'WPI.scriptname' NUL /s'
*/
WPI.WIC_pgm WPI.archivename' -s 'WPI.scriptname
if rc \= 0 then
   call ErrorExit 4 rc

/*
call Deinit
*/

/* Exit routines */
if WPI.retval = 0 then
   signal NormalExit

ErrorExit:
   parse arg errorcode errorcode2
   select
      when errorcode = 1 then
         nop            /* error 1 messages are handled in the 'Novalue' routine */
      when errorcode = 2 then
         do
            say 'Error: Unable to locate WIC.EXE.'
            say
            say 'Please (re)install a current version of Warpin before trying again.'
         end
      when errorcode = 3 then
         say 'Error: Add to WPI archive error code 'errorcode2
      when errorcode = 4 then
         say 'Error: Set script for WPI archive error code 'errorcode2
      when errorcode = 5 then
         say 'Error: Invalid parameter.'
      otherwise
         nop
   end
   say
   say 'Program aborted....'
   WPI.retval = errorcode

NormalExit:
n = endlocal()
exit WPI.retval

/* Subroutines */
Init: procedure expose (globals)
   WPI.retval = 0
   WPI.scriptonly = 0
/*
   parse arg WPI.archivename WPI.scriptname .
   if WPI.archivename == '' then
      WPI.archivename = WPI.default_archivename
   if WPI.scriptname == '' then
      WPI.scriptname = left(WPI.archivename, length(WPI.archivename) - 3) || 'wis'
*/
   WPI.archivename = WPI.default_archivename
   WPI.scriptname = left(WPI.archivename, length(WPI.archivename) - 3) || 'wis'
   parse arg args
   do while args \= ''
      parse var args word1 args
      select
         when translate(word1) == '/SCRIPT' then
            WPI.scriptonly = 1
         when translate(right(word1, 4)) == '.WIS' then
            WPS.scriptname = word1
         when translate(right(word1, 4)) == '.WPI' then
            WPS.archivename = word1
         otherwise
            call ErrorExit 5
      end
   end
   WPI.WIC_pgm = 'WIC.EXE'
   WIC_filename = SysSearchPath( 'PATH', 'WIC.EXE' )
   if WIC_filename == '' then
      do
         parse value SysIni( , 'WarpIN', 'Path' ) with Warpin_Path '00'x .
         WPI.WIC_pgm = Warpin_Path || '\WIC.EXE'
         if stream( WPI.WIC_pgm, 'C', 'query exists') = '' then
            call ErrorExit 2
      end
   else
      Warpin_Path = left(WIC_filename, lastpos('\', WIC_filename) - 1)

   Warpin_pathentry = ';' || translate(Warpin_path) || ';'

   ext_libpath = SysQueryExtLibpath('B')
   if pos( Warpin_pathentry, ';' || translate(SysQueryExtLibpath('B')) || ';' ) == 0 then
      call SysSetExtLibpath Warpin_pathentry || ';' || ext_libpath, 'B'

   parse source . . thispgm
   thisdir = left(thispgm, lastpos('\', thispgm) - 1)
   if length(thisdir) = 2 then
      thisdir = thisdir || '\'
   call directory thisdir

   call SysFileTree 'PACKAGE.*', 'pkgdirs.', 'DO'
   WPI.pkg.0 = pkgdirs.0
   do p = 1 to pkgdirs.0
      pkgnum = substr(pkgdirs.p, lastpos('\', pkgdirs.p) + 9) + 0    /* + 0 converts '01' to '1', etc. */
      WPI.pkg.p.number  = pkgnum
      WPI.pkg.p.dir     = pkgdirs.p
   end
return

/*
Deinit: procedure expose (globals)
return
*/

Emptydir: procedure
   parse arg dir
   call SysFileTree dir'\*', 'dirs.', 'DO'
   do i = 1 to dirs.0
      call EmptyDir dirs.i
   end
   call SysFileTree dir'\*', 'files.', 'FO'
   do i = 1 to files.0
      '@attrib -r -s -h 'files.i' >NUL 2>NUL'
      '@del 'files.i' >NUL 2>NUL'
   end
   '@rd 'dir
return

novalue:
   say 'Error: Uninitialized value: ' || condition('D') || ' encountered on line 'sigl':'
   say '   'sourceline(sigl)
   call ErrorExit 1

/*=== Error() Report ERROR, FAILURE etc. and exit ===*/

Error:
  say
  parse source . . cmd
  say 'CONDITION'('C') 'signaled at' cmd 'line' SIGL'.'
  if 'CONDITION'('D') \= '' then
    say 'REXX reason =' 'CONDITION'('D')'.'
  if 'CONDITION'('C') == 'SYNTAX' & 'SYMBOL'('RC') == 'VAR' then
    say 'REXX error =' RC '-' 'ERRORTEXT'(RC)'.'
  else if 'SYMBOL'('RC') == 'VAR' then
    say 'RC =' RC'.'
  say 'Source =' 'SOURCELINE'(SIGL)

  if 'CONDITION'('I') \== 'CALL' | 'CONDITION'('C') == 'NOVALUE' | 'CONDITION'('C') == 'SYNTAX' then do
    trace '?A'
    say 'Exiting.'
    call 'SYSSLEEP' 2
    exit 'CONDITION'('C')
  end

  return

/* end Error */

