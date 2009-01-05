/*
 * $Id$
 *
 * ReleaseTool: Provides a guide and assitance to the process
 *              of building a release version of FM/2.
 *
 * As a minimum, the menu provides a list of what needs to be done.
 * Additionally, many of the steps can be accomplished by selecting
 * the relevant number from the menu.
 *
 * Usage:
 *    ReleaseTool [trace=<trace-option>]
 *
 *    where
 *       <trace-option> is a valid comnbination of trace options:
 *          '? a c e f i l n o r'. This is an OPTIONAL parameter.
 *
 * This program uses the following enviromental variables, if set
 *    SVN_EDITOR or EDITOR to set the text editor that is called
 *    SVN_TESTER to call a cmd file that copies files to your test directory
 *               and changes to that directory
 *    SVN_KILL allows you to set a program to kill any running version of FM/2
 *             killpid is used as default (must be in path)
 *
 * Change log:
 *    18 Nov 08 JBS Ticket 297: Various build improvements/corrections
 *       - Use same file list for option 8 and option 20
 *       - Set the file list for option 8 and option 20 once, in Init routine
 *       - Removed fm2.wis from the file list for option 8 and option 20 (not needed)
 *       - Added optional commit to option 20
 *       - Removed extraneous 'pause' in option 20
 *       - Fixed a bug in option 0 (run a command shell)
 *    22 Nov 08 JBS Ticket 297
 *       - Fix bugs in version edits
 *       - Support for an optional trace parameter.
 *       - Support for EDITOR env var
 *       - Improved "usage" routine
 *    23 Nov 08 JBS Improved handling of invalid or missing <trace-option>
 *    04 Jan 09 JBS Fixed bug in option 9: Apply tag
 *
*/

n = setlocal()

signal on Error
signal on FAILURE name Error
signal on Halt
signal on NOTREADY name Error
signal on NOVALUE name Error
signal on SYNTAX name Error
/*
signal on novalue             /* for debugging */
*/


globals = 'ver cmd prompt editor editorcmds killpid tester killtarget version_filelist pager prev_action'

parse arg args
p = pos('TRACE=', translate(args))
if p > 0 then
   do
      args = args || ' ' /* make sure there's a space at the end */
      traceopt = substr(args, p+6, pos(' ', args, p+5) - (p+6))
      args = delstr(args, p, 6+length(traceopt))
      if traceopt \= '' then
         if verify(translate(traceopt), '?ACEFILNOR') = 0 & length(traceopt) < 3 then
            trace value traceopt
         else
            call Usage
      else
         do
            parse source . called_as .
            if called_as = 'COMMAND' then
               call Usage
            else
               nop /* traceopt = '' OK for ReleaseEdit because it is usually called from ReleaseTool? */
         end
   end
else
   traceopt = ''
if strip(args) \= '' then  /* trace= is the only supported parameter */
   call Usage

call Init

do forever
   call SysCls
   action = DisplayMenu()
   if stream(action || '.cmd', 'c', 'query exists') \= '' then
      do
         say;say;say
         say 'Found 'action'.cmd.'
         say
         call charout , 'Do you want to run it instead of the default processing? (Y/n): '
         entry = translate(SysGetKey())
         say
         if entry \= 'N' then
            do
               signal off Error
               '@'cmd '/c 'action
               signal on Error
               action = -1             /* Skip SELECT below */
            end
         prev_action = action
      end
   select
      when action = 0 then
         do /* Open a command line */
            signal off Error
            if right(translate(cmd), 8) = '4OS2.EXE' then
               '@' || cmd 'prompt [''exit'' to return to ReleaseTool]' || prompt
            else
               do
                  '@set prompt=[''exit'' to return to ReleaseTool]' || prompt
                  '@' || cmd
               end
            signal on Error
            prev_action = action
         end
      when action = 1 then
         do /* Ensure all work (by others) is comitted */
            call NotYet action
            say 'Notify programmers to commit their work for this release.'
            say
            say 'Use the Netlabs FM/2 Developer''s mailing list and wait 24 hours.'
            prev_action = action
         end
      when action = 2 then
         do /* Verify completed tickets are marked closed */
            call NotYet action
            say 'Check for committed work.'
            prev_action = action
         end
      when action = 3 then
         do /* Ensure all work (by others) is local */
            svn_cmd = 'svn update'
            say;say;say
            say 'Online and OK to execute: 'svn_cmd'? (Y/n)'
            if translate(SysGetKey()) \= 'N' then svn_cmd
            prev_action = action
         end
      when action = 4 then
         do /* Ensure all local source is up to date */
            if cmd = '4OS2' then
               svn_cmd = 'svn status -v |& less'
            else
               svn_cmd = 'svn status -v | less'
            say;say;say
            say 'Online and OK to execute: 'svn_cmd'? (Y/n)'
            if translate(SysGetKey()) \= 'N' then svn_cmd
            prev_action = action
         end
      when action = 5 then
         do /* Edit version #'s and date/time stamps (ReleaseTool) */
            call SysCls
            if strip(ver) = '' then
               ver = GetVer('the pending release')
            do f = 1 to words(version_filelist)
               call ReleaseEdit ver word(version_filelist, f) 'trace=' || traceopt
               say
            end
            version_filelist2 = 'HISTORY README'
            do f = 1 to words(version_filelist2)
               file = word(version_filelist2, f)
               call SysCls
               say;say;say
               say 'Next, edit the' file 'file.'
               say
               say 'Include descriptions of salient changes to FM/2.'
               say
               say 'And be sure to update the version number to' ver
               say
               '@pause'
               editor file editorcmds
            end
            call BuildHobbesTxt(ver)
            prev_action = action
         end
      when action = 6 then
         do /* Ensure the edits build */
            'set WARNALL=1'
            'wmake -a all |' pager
            prev_action = action
         end
      when action = 7 then
         do /* Test built code */
            if tester == '' then
               do
                  call NotYet action
                  say 'Test the built code.'
               end
             else
               do  /*kills FM/2 using killpid from FM/2 utils (must be in path) and run cmd to copy files
                    to test directory and change to that directory must type exit to return here*/
                  killpid killtarget
                  cmd tester
               end
            prev_action = action
         end
      when action = 8 then
         do /* Commit code */
            call CommitifOK
            prev_action = action
         end
      when action = 9 then
         do /* Apply tag */
            call SysCls
            if strip(ver) = '' then
               ver = GetVer('the pending release')
            svn_cmd = 'svn copy -m"Tag release FM2-' || Tag_ver(ver) || '" http://svn.netlabs.org/repos/fm2/trunk http://svn.netlabs.org/repos/fm2/tags/FM2-' || Tag_ver(ver)
            say;say;say
            say 'Online and OK to execute: 'svn_cmd'? (y/N)'
            if translate(SysGetKey()) = 'Y' then svn_cmd
            prev_action = action
         end
      when action = 10 then
         do /* Build for the release */
            'set WARNALL='
            'set FORTIFY='
            'set DEBUG='
            'wmake -a all'
            prev_action = action
         end
      when action = 11 then
         do /* Test the binaries */
            if tester == '' then
               do
                  call NotYet action
                  say 'Test the binaries.'
                  say
                  say 'At a minimum you should run all the exes and do some'
                  say 'basic file manipulation with each.'
                  say
                  say 'You should, where possible, also verify that any bugs'
                  say 'that were fixed for the release are working as expected.'
               end
            else
               do  /*kills FM/2 using killpid from FM/2 utils (must be in path) and run cmd to copy files
                    to test directory and change to that directory must type exit to return here*/
                  killpid killtarget
                  cmd tester
               end
            prev_action = action
         end
      when action = 12 then
         do /* Lxlite */
            'wmake lxlite'
            prev_action = action
         end
      when action = 13 then
         do /* Test the release code */
            if tester == '' then
               do
                  call NotYet action
                  say 'Test the (compressed) release code.'
                  say
                  say 'Verify that all exe''s continue to load and run after being compressed.'
               end
            else
              do  /*kills FM/2 using killpid from FM/2 utils (must be in path) and run cmd to copy files
                    to test directory and change to that directory must type exit to return here*/
                  killpid killtarget
                  cmd tester
              end
            prev_action = action
         end
      when action = 14 then
         do /* Build distro */
            call SysCls
            'wmake dist'
            prev_action = action
         end
      when action = 15 then
         do /* Zip distro */
            call SysCls
            if strip(ver) = '' then
               ver = GetVer('the pending release')
            zip_ver = translate(ver, '-', '.')
            'zip -j9 fm2-' || zip_ver || ' warpin\fm2-' || zip_ver || '.wpi'
            prev_action = action
         end
      when action = 16 then
         do /* Zip FM/2 Utilities' */
            call NotYet action
            say 'If FM/2 Utilities have been updated, then zip them up.'
            prev_action = action
         end
      when action = 17 then
         do /* Wiki updates */
            call SysCls
            if strip(ver) = '' then
               ver = GetVer('the pending release')
            call NotYet action
            say 'Wiki updates'
            say
            say '   Update "WikiStart" with the new version:' ver
            say
            say '   Update "RBuild" with the new tag: FM2-' || translate(ver, '-', '.')
            prev_action = action
         end
      when action = 18 then
         do /* TRAC updates */
            call NotYet action
            say 'TRAC updates'
            say
            say '   Create a TRAC version for the next version, if needed.'
            say
            say '   Mark the next version as the default version for new tickets.'
            say
            say '   Create a TRAC milestone for the next version release, if needed.'
            say
            say '   Mark the new milestone as the default milestone.'
            say
            say '   Mark the completed milestone as complete and'
            say '   move any residual tickets to a future milestone.'
            say '   (TRAC can move the tickets in bulk when you mark'
            say '   the version milestone complete.)'
            prev_action = action
         end
      when action = 19 then
         do /* Upload to distribution points and announce. */
            call NotYet action
            say 'Upload WPI file to distribution points.'
            say
            say 'Announce release to mailing lists, Usenet, etc.'
            say
            say 'Post a note to "Netlabs Community" <community@netlabs.org>'
            say 'requesting that the release be moved to pub/fm2.'
            prev_action = action
         end
      when action = 20 then
         do /* Set next version */
            next_ver = GetVer('the next release')
            do f = 1 to words(version_filelist)
               call ReleaseEdit next_ver word(version_filelist, f) 'trace=' || traceopt
               say
            end
            call CommitifOK version_filelist
            prev_action = action
         end
      otherwise
         nop
   end
   say;say;say
   if action \= 0 then
      '@pause'
end

n = endlocal()

return

/*** Error routine ***/
novalue:
  say 'Uninitialized value found on line:' sigl
  do i = sigl-3 to sigl+3
     if i = sigl then
        call charout , '--> '
     else
        call charout , '    '
     say sourceline(i)
  end
exit

Usage:
   say;say;say
   i = 1
   do forever
      srcline = sourceline(i)
      if pos('CHANGE LOG', translate(srcline)) > 0 then
         leave
      else
         say srcline
      i = i + 1
      if (i // 22) = 0 then
         '@pause'
   end
exit

/*** Subroutines ***/
Init: procedure expose (globals)
   call RxFuncAdd 'SysLoadFuncs', 'REXXUTIL', 'SysLoadFuncs'
   call SysLoadFuncs

   action = 0
   ver = ''

   editor = value('SVN_EDITOR',,'OS2ENVIRONMENT')
   editorcmds = ""
   if editor = '' then
      editor = value('EDITOR',,'OS2ENVIRONMENT')
   if editor = '' then
      editor = 'tedit'
   else
      do
         upperwrd1 = translate(word(editor, 1))
         if upperwrd1 = 'EPM' | upperwrd1 = 'EPM.EXE' then
            editorcmds = "'3'"
      end
   cmd      = value('COMSPEC',,'OS2ENVIRONMENT')
   prompt   = value('PROMPT',,'OS2ENVIRONMENT')
   tester       = value('SVN_TESTER',,'OS2ENVIRONMENT')
   killpid      = value('SVN_KILL',,'OS2ENVIRONMENT')
   if killpid == '' then
       killpid      = 'killpid'
   killtarget  = ' FM/2'

   version_filelist = 'av2.def databar.def dirsize.def dll\fm3dll.def dll\fm3res.def'
   version_filelist = version_filelist 'dll\version.h eas.def fm3.def fm4.def global.def ini.def'
   version_filelist = version_filelist 'killproc.def sysinfo.def undel.def vcollect.def vdir.def'
   version_filelist = version_filelist 'viewinfs.def vtree.def file_id.diz'
   version_filelist = version_filelist 'warpin\makefile dll\internal\makefile'
   version_filelist = version_filelist 'dll\copyright.h'

   if SysSearchPath( 'PATH', 'less.exe') \= '' then
      pager = 'less'
   else
      pager = 'more'
   prev_action = 'N/A'
return

DisplayMenu: procedure expose (globals)
   do forever
      call SysCls
      say;say;say
      say 'Release Tasks' || copies(' ', 35) || 'Previous action:' prev_action
      say
      say '1.  Ensure all work for this release is committed.'
      say '2.  Verify completed tickets are marked closed.'
      say '3.  (SVN) Update local files.'
      say '4.  Check (svn) status of local files.'
      say '5.  Edit various files with version #''s and date''s.'
      say '6.  Ensure the edits build. (WARNALL build)'
      say '7.  Test built code.'
      say '8.  Commit code.'
      say '9.  Apply tag.'
      say '10. Build (all) for the release.'
      say '11. Test the release build.'
      say '12. Lxlite the build.'
      say '13. Re-test the "lxlited" release code.'
      say '14. Build distribution WPI file.'
      say '15. Zip distribution WPI file.'
      say '16. (Optional) Zip FM/2 Utilities.'
      say '17. Wiki updates.'
      say '18. TRAC updates.'
      say '19. Upload to distribution points and announce release.'
      say '20. Set next version (no HISTORY or README update).'
      say
      call charout , 'Enter the number of your choice (''X'' to exit; ''0'' to open a command line): '
      action = strip(translate(linein()))
      say
      if action = 'X' then
         exit
      if datatype(action) \= 'NUM' then
         iterate
      if action \= trunc(action) then
         iterate
      if action < 0 then
         iterate
      if action <= 20 then
         leave
   end
return action

NotYet: procedure
   parse arg action
   call SysCls
   say;say;say
   say 'This option, ' || action || ', has not yet been (and may never be) automated.'
   say
   say 'You will have to do this manually. See instructions below:'
   say;say;say
return

GetVer: procedure
   parse arg ver_text
ver_retry:
   say
   say 'Please enter the version (x.yy.zz) for' ver_text ':'
   ver_value = linein()
   parse var ver_value major '.' minor '.' CSDlevel
   if minor    = '' then
      minor    = 0
   if CSDlevel = '' then
      CSDlevel = 0
   if datatype(major) \= 'NUM' | datatype(minor) \= 'NUM' | datatype(CSDlevel) \= 'NUM' then
      do
         say 'Error: Invalid version entered:' ver_value
         say;
         say 'Try again.'
         signal ver_retry
      end
   /* The following will remove leading zeros */
   major = major + 0
   minor = minor + 0
   CSDlevel = CSDlevel + 0
return major || '.' || minor || '.' || CSDlevel

Tag_ver: procedure
   parse arg ver
   parse var ver major '.' minor '.' CSDlevel
return major || '_' || right(minor, 2, '0') || '_' || right(CSDlevel, 2, 0)

WPI_ver: procedure
   parse arg ver
return translate(Tag_ver(ver), '-', '_')

BuildHobbesTxt: procedure expose (globals)
   parse arg ver
   wpi_version = WPI_ver(ver)
   HobbesTxtFilename = 'FM2-' || wpi_version || '.txt'
   if stream(HobbesTxtFilename, 'c', 'query exists') \= '' then
      do
         say;say;say
         say HobbesTxtFilename 'already exists!'
         call charout , 'Do you want to replace this file? (y/N) '
         if translate(SysGetKey()) \= 'Y' then
            do
               say;say;
               say HobbesTxtFilename 'update aborted.'
               return
            end
         call SysFileDelete HobbesTxtFilename
      end
   /* Prompt for user input (name, email, permission to email, previous version) here? */
   default_name            = 'Gregg Young'
   default_email           = 'ygk@qwest.net'
   default_OKtoListEmail   = 'yes'
   entry = ''
   do until (entry = 'Y' | entry = '0d'x)
      say;say
      say 'You will now be prompted for potentially variable fields within' HobbesTxtFilename
      say
      say 'A default value will be given for most fields.'
      say 'To accept the default just press the Enter key.'
      say
      call charout , 'Name of the relaser (default:' default_name '): '
      entry = strip(linein())
      if entry = '' then
         name = default_name
      else
         name = entry
      say;say
      call charout , 'Email address of the relaser (default:' default_email '): '
      entry = strip(linein())
      if entry = '' then
         email = default_email
      else
         email = entry
      say;say
      call charout , 'OK to list email address of the relaser (default:' default_OKtoListEmail '): '
      entry = strip(linein())
      if entry = '' then
         OKtoListEmail = default_OKtoListEmail
      else
         if translate(left(entry, 1)) = translate(left(default_OKtoListEmail, 1)) then
            OKtoListEmail = default_OKtoListEmail
         else
            if left(default_OKtoListEmail, 1) = 'y' then
               OKtoListEmail = 'no'
            else
               OKtoListEmail = 'yes'
      say;say
      replaced_ver = WPI_Ver(GetVer('version to be replaced'))
      replaced_ver_wpi = 'fm2-' || replaced_ver || '.wpi'
      say;say
      say 'Data entered:'
      say '  Name of releaser  :' name
      say '  Email of releaser :' email
      say '  OK to list email  :' OKtoListEmail
      say '  WPI to be replaced:' replaced_ver_wpi
      say;say
      call charout , 'OK to proceed with file write? (Y/n) '
      entry = translate(SysGetKey())
   end

   rm1 = 73
   rm2 = 25
   call lineout HobbesTxtFilename, right('Upload Information Template for Hobbes.nmsu.edu', rm1)
   call lineout HobbesTxtFilename, right(copies('=', length('Upload Information Template for Hobbes.nmsu.edu')), rm1)
   call lineout HobbesTxtFilename, ''
   call lineout HobbesTxtFilename, right('Archive Filename:', rm2) || ' fm2-' || wpi_version || '.wpi'
   call lineout HobbesTxtFilename, right('Short Description:', rm2) || ' Mark Kimes FM/2 File Manager ' || ver
   call lineout HobbesTxtFilename, right('Long Description:', rm2) || ' The package contains a warpin package for installing FM2.'
   call lineout HobbesTxtFilename, copies(' ', rm2) || ' The FM2 source code is available on netlabs.'
   call lineout HobbesTxtFilename, right('Proposed directory ', rm2)
   call lineout HobbesTxtFilename, right('for placement:', rm2) || ' os2/util/browser'
   call lineout HobbesTxtFilename, ''
   call lineout HobbesTxtFilename, right('Your name:', rm2) name
   call lineout HobbesTxtFilename, right('Email address:', rm2) email
   call lineout HobbesTxtFilename, right('Program contact name:', rm2) || ' (same)'
   call lineout HobbesTxtFilename, right('Program contact email:', rm2) || ' (same)'
   call lineout HobbesTxtFilename, right('Program URL:', rm2) || ' http://svn.netlabs.org/fm2'
   call lineout HobbesTxtFilename, ''
   call lineout HobbesTxtFilename, right('Would you like the ', rm2)
   call lineout HobbesTxtFilename, right('contact email address ', rm2)
   call lineout HobbesTxtFilename, right('included in listings?', rm2) OKtoListEmail
   call lineout HobbesTxtFilename, ''
   call lineout HobbesTxtFilename, right('Operating System/Version:', rm2) || ' OS/2 Warp 3.0 and up.'
   call lineout HobbesTxtFilename, right('Additional requirements:', rm2)
   call lineout HobbesTxtFilename, ''
   call lineout HobbesTxtFilename, right('Replaces:', rm2) replaced_ver_wpi
   call stream  HobbesTxtFilename, 'c', 'close'
   say;say;say
   say HobbesTxtFilename 'has been written.'
return

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

novalue:
   say 'Uninitialized variable: ' || condition('D') || ' on line: 'sigl
   say 'Line text: 'sourceline(sigl)
   cfg.errorcode = 3
   signal ErrorExit

CommitIfOK: procedure
   parse arg filelist
   svn_cmd = 'svn commit'
   say;say;say
   say 'Online and OK to execute: 'svn_cmd'? (Y/n)'
   if translate(SysGetKey()) \= 'N' then svn_cmd filelist
return
