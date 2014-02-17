/*
 * $Id$
 *
 * ReleaseTool: Provides a guide and assitance to the process of
 *              of building, uploading and announcing a release of FM/2.
 *
 * As a minimum, the menu provides a list of what needs to be done.
 * Additionally, many of the steps can be accomplished by selecting
 * the relevant number from the menu.
 *
 * Usage:
 *    ReleaseTool [version] [/?] [trace=<trace-option>]
 *
 *    where
 *       version is the version for the pending release
 *       /? : Causes help/usage screens to be displayed (and exits).
 *       <trace-option> is a valid comnbination of trace options:
 *          '? a c e f i l n o r'. This is an OPTIONAL parameter.
 *    All parameters are optional.
 *
 * This program uses the following enviromental variables, if set
 *    SVN_EDITOR or EDITOR to set the text editor that is called
 *       If neither is set, then TEDIT is used.
 *    SVN_TESTER to call a cmd file that copies files to your test directory
 *               and changes to that directory
 *    SVN_KILL allows you to set a program to kill any running version of FM/2
 *             killpid is used as default (must be in path)
 *    SVN_WPIVIEWER or WPIVIEWER to set the program to view WPI files
 *       If neither is set, then WIC is used.
 *    PAGER to set the "pager" program (usually MORE or LESS)
 *       If PAGER is not set, then LESS if used if it is on the PATH
 *       and MORE is used if LESS is not on the PATH.
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
 *    02 Apr 10 JBS Removed reference to obsolete "internal\makefile" file
 *    29 May 10 GKY Use TEE to generate log files for the build options
 *       - Use DIFF to compare changes in WARNALL builds
 *    31 May 10 JBS Add support of use of PAGER env. variable to the the pager used.
 *    31 May 10 JBS Build zip file and Hobbes txt file in Warpin directory
 *    29 Jun 11 JBS Ticket 462: Fixed bug in NNTP code and standardized version format to x.yy.z
 *    02 Jul 11 JBS Ticket 462: Updated Wiki updates instructions per SHL
 *    23 Jul 11 JBS Ticket 462:
 *       - Major rework of email code.
 *          - Simplified process to add/change CFG data dynamically
 *          - Added support for COMMAND to FTP and NNTP code
 *          - Added support for multiple SMTP servers
 *          - Added support non-authenticated SMTP transactions
 *          - Added structure to support other SMTP authentication methods
 *       - Major rework of CFG file.
 *          - Improved documentation
 *          - Added support for COMMAND in all sections
 *          - New syntax used!
 *       - Added email to developers asking for work to be committed. (Task #1)
 *       - Improved support for external commands which perform some or all of a task
 *          - Improved error handling
 *          - Added support for FTP and NNTP commands
 *    07 Aug 11 JBS Ticket 462: Fix bug in NNTP "To" code
 *    11 Feb 14 JBS Ticket 462: Improved handling of missing cfg file.
 *
 * To Do
 *    -  Better error handling for emails/NNTP
 *    -  Support multiple description lines for SMTP/NNTP (to improve readabliity on verification screens)
 *    -  Make sure ##macro## key_values are filled before use?
 *    -  More flexibility for SMTP settings for the "Commit work" (to FM/2 developers)
 *       and the "Please move uplaoded file" (to Netlabs) emails. Currently only the
 *       first SMTP definition found in the CFG file will be used for both.
 *    -  Support multiple commands for an SMTP (or NNTP or FTP) task?
 *    -  Support configurable Hobbes names and email address? via parameter or CFG file?
 *    -  "hidden" display/entry for passwords?
 *    -  Prompt for edit to fix or delete the Hobbes TXT file if invalid email address found
 *
*/

n = setlocal()

signal on Error
signal on Failure name Error
signal on Notready name Error
signal on Syntax name Error
signal on Novalue name Error
/* JBS: for debugging */
signal on Novalue

globals = 'cfg. ver. mainmenu. cmd. email. Hobbes. available. prev_user_choice currentSMTP'

parse arg args
if wordpos('/?', args) > 0 | ,
   wordpos('-?', args) > 0 | ,
   wordpos('/H', translate(args)) > 0 | ,
   wordpos('-H', args) > 0 then
   do
      call Usage
      exit
   end

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

call Init strip(args)
cfginit_rc = CfgInit()
select
   when cfginit_rc > 10000 then
      do
         say
         say 'Syntax error detected in' cfg.file
         say
         say 'A line that was not:'
         say '  - blank'
         say '  - a comment line'
         say '  - a section name line (e.g. [FTP])'
         say '  - a "key-name = key-value" line'
         say 'was found at or around line:' (cfginit_rc - 10000)
         say
         say 'Correct the error and restart.'
         exit
      end
   when cfginit_rc < -10000 then
      do
         say
         say 'Syntax error detected in' cfg.file
         say
         say 'An unknown key-name was found at or around line:' (-cfginit_rc - 10000)
         say
         say 'Correct the error and restart.'
         exit
      end
   otherwise
      do forever
         user_choice = DisplayMenu()
         if datatype(user_choice) = 'NUM' then
            call NewScreenWithHeading user_choice
         cmdfile_option = ''
         if stream(user_choice || '.cmd', 'c', 'query exists') \= '' then
            do
               say
               say 'Found 'user_choice'.cmd.'
               say
               say 'Do you want to run it...'
               say '   I)nstead of the default processing'
               say '   B)efore the default processing'
               say '   A)fter the default processing'
               say '   N)ot at all (Default)'
               say
               call charout , 'Type the first letter of your choice (I/B/A/N): '
               cmdfile_option = translate(SysGetKey())
               say
               if cmdfile_option = 'I' | cmdfile_option = 'B' then
                  do
                     call ExecCmd 'call' user_choice || '.cmd.'
                     if cmdfile_option = 'I' then
                        user_choice = -1             /* Skip SELECT below */
                  end
               prev_user_choice = user_choice
            end
         select
            when user_choice = 'C' then
               do /* Open a command line */
                  call Commandline
                  prev_user_choice = user_choice
               end
/* JBS: Disabled internal CFG code:
            when user_choice = 'G' then
               do /* Lead user through configuration */
                  call Configuration
                  prev_user_choice = user_choice
               end
*/
            when user_choice = 'S' then
               do /* Run SmartSVN */
                  dirsave = directory()
                  svndrive = filespec('D', cmd.smartsvn)
                  svndrive_dirsave = directory(svndrive)
                  call directory strip(svndrive || filespec('P', cmd.smartsvn), 'T', '\')
                  call ExecCmd 'call smartsvn.cmd'
                  call directory svndrive_dirsave
                  call directory dirsave
                  say;say
                  say 'If SmartSVN did not run correctly, make sure the default Java'
                  say 'is Java 1.4.1 or later before running ReleaseTool.'
                  prev_user_choice = user_choice
               end
            when user_choice = mainmenu.Ensure_work_done_num then
               do /* Ensure all work (by others) is comitted */
                  if Email(user_choice) \= 0 then
                     do
                        say 'Error prevented automated emails.'
                        say 'This notification will have to be done manually.'
                     end
                  prev_user_choice = user_choice
               end
            when user_choice = mainmenu.Verify_tickets_closed_num then
               do /* Verify completed tickets are marked closed */
                  call NotYet user_choice
                  say 'Check for committed work.'
                  prev_user_choice = user_choice
               end
            when user_choice = mainmenu.Local_SVN_update_num then
               do /* Ensure all work (by others) is local */
                  svn_cmd = 'svn update'
                  say
                  say 'Online and OK to execute: 'svn_cmd'? (Y/n)'
                  if translate(SysGetKey()) \= 'N' then call ExecCmd svn_cmd
                  prev_user_choice = user_choice
               end
            when user_choice = mainmenu.Check_SVN_status_num then
               do /* Ensure all local source is up to date */
                  if cmd.processor_is_4os2 = 1 then
                     svn_cmd = 'svn status -v |&' cmd.pager
                  else
                     svn_cmd = 'svn status -v |' cmd.pager
                  say
                  say 'Online and OK to execute: 'svn_cmd'? (Y/n)'
                  if translate(SysGetKey()) \= 'N' then call ExecCmd svn_cmd
                  prev_user_choice = user_choice
               end
            when user_choice = mainmenu.Version_number_edits_num then
               do /* Edit version #'s and date/time stamps (ReleaseTool) */
                  do f = 1 to words(ver.filelist)
                     call ReleaseEdit ver.full word(ver.filelist, f) 'trace=' || traceopt
                     say
                  end
                  ver.filelist2 = 'HISTORY README'
                  do f = 1 to words(ver.filelist2)
                     file = word(ver.filelist2, f)
                     call NewScreenWithHeading user_choice
                     say
                     say 'Next, edit the' file 'file.'
                     say
                     say 'Include descriptions of salient changes to FM/2.'
                     say
                     say 'And be sure to update the version number to' ver.full
                     say
                     '@pause'
                     call ExecCmd cmd.editor file cmd.editorcmds
                  end
                  call BuildHobbesTxt
                  prev_user_choice = user_choice
               end
            when user_choice = mainmenu.WARNALL_build_num then
               do /* Ensure the edits build */
                  say
                  'set WARNALL=1'
                  call ExecCmd 'wmake -a all | tee warnall.log |' cmd.pager
                  'set WARNALL='
                  call ExecCmd 'diff.exe -rub warnall.base warnall.log > warnall.diff'
                  call ExecCmd cmd.editor 'warnall.diff'
                  say
                  call charout , 'Make WARNALL.LOG the new WARNALL.BASE? (y/N): '
                  if translate(SysGetKey()) = 'Y' then
                     'copy WARNALL.LOG WARNALL.BASE'
                  say
                  prev_user_choice = user_choice
               end
            when user_choice = mainmenu.Test_WARNALL_build_num then
               do /* Test built code */
                  if cmd.tester == '' then
                     do
                        call NotYet user_choice
                        say 'Test the built code.'
                     end
                   else
                     call RunTester
                  prev_user_choice = user_choice
               end
            when user_choice = mainmenu.Commit_code_num then
               do /* Commit code */
                  call CommitifOK
                  prev_user_choice = user_choice
               end
            when user_choice = mainmenu.Build_all_for_release_num then
               do /* Build for the release */
                  'set WARNALL='
                  'set FORTIFY='
                  'set DEBUG='
                  call ExecCmd 'wmake -a all | tee build.log'
                  prev_user_choice = user_choice
               end
            when user_choice = mainmenu.Test_release_build_num then
               do /* Test the binaries */
                  if cmd.tester == '' then
                     do
                        call NotYet user_choice
                        say 'Test the binaries.'
                        say
                        say 'At a minimum you should run all the exes and do some'
                        say 'basic file manipulation with each.'
                        say
                        say 'You should, where possible, also verify that any bugs'
                        say 'that were fixed for the release are working as expected.'
                     end
                  else
                     call RunTester
                  prev_user_choice = user_choice
               end
            when user_choice = mainmenu.LXLITE_build_num then
               do /* Lxlite */
                  'wmake lxlite | tee lxlite.log'
                  prev_user_choice = user_choice
               end
            when user_choice = mainmenu.Test_LXLITE_build_num then
               do /* Test the release code */
                  if cmd.tester == '' then
                     do
                        call NotYet user_choice
                        say 'Test the (compressed) release code.'
                        say
                        say 'Verify that all exe''s continue to load and run after being compressed.'
                     end
                  else
                     call RunTester
                  prev_user_choice = user_choice
               end
            when user_choice = mainmenu.Build_WPI_file_num then
               do /* Build distro */
                  call ExecCmd 'wmake dist | tee dist.log'
                  prev_user_choice = user_choice
               end
            when user_choice = mainmenu.Test_WPI_file_num then
               do /* Test WPI file */
                  call TestWPI user_choice
                  prev_user_choice = user_choice
               end
            when user_choice = mainmenu.Apply_tag_num then
               do /* Apply tag */
                  svn_cmd = 'svn copy -m"Tag release FM2-' || ver.tag || '" http://svn.netlabs.org/repos/fm2/trunk http://svn.netlabs.org/repos/fm2/tags/FM2-' || ver.tag
                  say;say;say
                  say 'Online and OK to execute:'
                  call charout , svn_cmd'? (y/N) '
                  choice2 = translate(SysGetKey())
                  say
                  if choice2 = 'Y' then call ExecCmd svn_cmd
                  prev_user_choice = user_choice
               end
            when user_choice = mainmenu.Zip_files_num then
               do /* Zip up files */
                  call ZipFiles
                  prev_user_choice = user_choice
               end
            when user_choice = mainmenu.WIKI_updates_num then
               do /* Wiki updates */
                  call WikiUpdates user_choice
                  prev_user_choice = user_choice
               end
            when user_choice = mainmenu.Upload_num then
               do /* Upload to distribution points and request Netlabs move of uploaded file. */
                  say
                  if UploadRelease() = 0 then
                     do
                        say
                        say 'Attempting to notify Netlabs of the upload...'
                        say
                        call Email user_choice
                     end
                  else
                     say 'Upload(s) failed. Email to Netlabs cancelled.'
                  prev_user_choice = user_choice
              end
            when user_choice = mainmenu.Announce_num then
               do /* Announce the release. */
                  call Email user_choice
                  call AnnounceToNewsgroups
                  prev_user_choice = user_choice
              end
            when user_choice = mainmenu.TRAC_update_and_Next_ver_num then
               do /* TRAC updates */
                  call TracUpdates
                  prev_user_choice = user_choice
               end
            when user_choice = 'Q' then
               leave
            otherwise
               nop
         end
         if cmdfile_option = 'A' then
            call Commandline
         say;say;say
         if user_choice \= 'Q' & /* JBS: Disabled internal CFG code: user_choice \= 'G' & */ ,
            user_choice \= 'C' & ,
            user_choice \= mainmenu.Test_WPI_file_num then
            '@pause'
      end
      if available.tee = 0 then
         call SysFileDelete 'tee.cmd'
end

n = endlocal()

return

/*** Subroutines ***/
Email: procedure expose (globals)
   parse arg user_choice
   if cfg.SMTP.0 = 0 then
      do
        if \cfg.file_exists then
          say 'Error: Cannot send Email without the configuration file:' cfg.file
        else
          say 'Error: Cannot send Email. No SMTP sections were found in' cfg.file ||'.'
        say
        '@pause'
        return -1
      end
   rcx = -1
   email.body_file = SysTempFilename('SMTPBody.???')
   select
      when user_choice = mainmenu.Ensure_work_done_num then
         do
            server_count = 1
            page_title = 'Note to FM/2 developer list requesting work commit.'
            email.subject = 'Request: Commit work for upcoming release.'
            email.to_list = '"FM/2 Developers" <fm2-dev@netlabs.org>'
            call lineout email.body_file, 'The release of FM/2' ver.full 'is imminent.'
            call lineout email.body_file, ''
            call lineout email.body_file, 'Please commit all work for this release within 24 hours.'
            call lineout email.body_file, ''
            call lineout email.body_file, 'Reply to this email if there are reasons to delay the release.'
            call stream  email.body_file, 'c', 'close'
        end
      when user_choice = mainmenu.Upload_num then
         do
            server_count = 1
            page_title = 'Note to Netlabs requesting move of release file'
            email.subject = 'Request: Move uploaded file'
            email.to_list = '"Netlabs Community" <community@netlabs.org>'
            call lineout email.body_file, 'The FM/2' ver.full 'has been released.'
            call lineout email.body_file, ''
            call lineout email.body_file, 'The file: fm2-' || ver.wpi
            call lineout email.body_file, 'has been uploaded to /incoming/fm2'
            call lineout email.body_file, ''
            call lineout email.body_file, 'Please move this file to /pub/fm2'
            call stream  email.body_file, 'c', 'close'
        end
      when user_choice = mainmenu.Announce_num then
         do
            server_count = cfg.SMTP.0
            page_title = 'Release announcement'
            email.subject = 'File Manager/2 v3.19.1 released'
            call SetDefaultAnnouncementText email.body_file
        end
      otherwise
         do
            say 'Email for option' user_choice 'has not been implemented.'
            return
         end
   end
   _text = '<Standard>'
   do until ((option = 'C') | (option = 'Q'))
      call SysCls
      say
      say 'Email: Verify/edit content'
      say
      say 'Subject  :' email.subject
      say 'Text     :' _text
      say
      say 'Type "C" to confirm the above and proceed.'
      say '     "Q" to abort.'
      say '     "S" to edit the subject.'
      say '     "T" to edit the message text (in an editor).'
      say
      call charout, '==> '
      option = translate(SysGetKey())
      say
      say
      say
      select
         when ((option = 'C') | (option = 'Q')) then
            nop
         when option = 'S' then
            do
               say 'Enter the Subject (followed by the ENTER key):'
               email.subject = linein()
            end
         when option = 'T' then
            do
               b4_timestamp = SysGetFileDateTime(email.body_file)
               say 'The current body of the email will now be loaded into an editor.'
               say 'Make desired changes, if any, and save the file.'
               say
               call charout, 'Press any key when ready to load the message body into an editor: '
               call SysGetKey
               say
               call ExecCmd cmd.editor email.body_file
               if b4_timestamp \= SysGetFileDateTime(email.body_file) then
                  _text = '<Modified>'
            end
         otherwise
            nop
      end
   end
   if option \= 'Q' then
      do currentSMTP = 1 to server_count
         if user_choice = mainmenu.Announce_num then
            email.to_list = cfg.SMTP.currentSMTP.to
         do until ((option = 'C') | (option = 'Q'))
            call SysCls
            say
            say 'Email: Verify/edit server-specific data for' cfg.SMTP.currentSMTP.description
            say
            say 'To       :' email.to_list
            say 'UserID   :' cfg.SMTP.currentSMTP.userid
            say 'Password :' cfg.SMTP.currentSMTP.password
            say
            say 'Type "C" to confirm the above and send.'
            say '     "Q" to abort sending this email.'
            say '     "T" to change the list of addressees.'
            say '     "U" to change the Userid.'
            say '     "P" to change the Password.'
            say
            call charout, '==> '
            option = translate(SysGetKey())
            say
            say
            select
               when option = 'T' then
                  do
                     say 'NOTE: Currently no edits are done on the email addresses.'
                     say 'So make sure you use a proper format.'
                     say
                     say 'For multiple addressees, enter a comma-separated list.'
                     say
                     say 'Enter the addressee(s) (followed by the ENTER key).'
                     email.to_list = linein()
                  end
               when option = 'U' then
                  do
                     say 'Enter the UserID (followed by the ENTER key):'
                     cfg.SMTP.currentSMTP.userid = linein()
                  end
               when option = 'P' then
                  do
                     say 'Enter the Password (followed by the ENTER key):'
                     cfg.SMTP.currentSMTP.password = linein()
                  end
               otherwise
                  nop
            end
         end
         if option = 'C' then
            if cfg.SMTP.currentSMTP.command = '' then /* Use internal SMTP code */
               rcx = SendEmail()
            else                                      /* Use external command */
               do
                  say 'Using external command(s) to send email...'
                  _command = cfg.SMTP.currentSMTP.Command
                  do while pos('##TO##', _command) > 0
                     parse var _command part1 '##TO##' part2
                     _command = part1 || email.to_list || part2
                  end
                  do while pos('##FROM##', _command) > 0
                     parse var _command part1 '##FROM##' part2
                     _command = part1 || cfg.SMTP.currentSMTP.from || part2
                  end
                  do while pos('##SUBJECT##', _command) > 0
                     parse var _command part1 '##SUBJECT##' part2
                     _command = part1 || email.subject || part2
                  end
                  do while pos('##MESSAGE_BODY_FILE##', _command) > 0
                     parse var _command part1 '##MESSAGE_BODY_FILE##' part2
                     _command = part1 || stream(email.body_file, 'c', 'query exists') || part2
                  end
                  do while pos('##HOST##', _command) > 0
                     parse var _command part1 '##HOST##' part2
                     _command = part1 || cfg.SMTP.currentSMTP.host || part2
                  end
                  do while pos('##PORT##', _command) > 0
                     parse var _command part1 '##PORT##' part2
                     _command = part1 || cfg.SMTP.currentSMTP.port || part2
                  end
                  do while pos('##USERID##', _command) > 0
                     parse var _command part1 '##USERID##' part2
                     _command = part1 || cfg.SMTP.currentSMTP.userid || part2
                  end
                  do while pos('##PASSWORD##', _command) > 0
                     parse var _command part1 '##PASSWORD##' part2
                     _command = part1 || cfg.SMTP.currentSMTP.password || part2
                  end
                  do while pos('##UTCOFFSET##', _command) > 0
                     parse var _command part1 '##UTCOFFSET##' part2
                     _command = part1 || cfg.SMTP.currentSMTP.UTCOffset || part2
                  end
                  retval = ExecCmd(_command)
                  '@pause'
               end
         else
            rcx = -1
      end
   call SysFileDelete email.body_file
return rcx

SetDefaultAnnouncementText: procedure expose (globals)
   parse arg tempfile
   release_file = 'fm2-' || ver.wpi || '.zip'
   call lineout tempfile, 'FM/2' ver.full 'has been released. The file name is' release_file
   call lineout tempfile, ''
   call lineout tempfile, 'It has been uploaded to Netlabs and to Hobbes.'
   call lineout tempfile, ''
   call lineout tempfile, 'NETLABS'
   call lineout tempfile, 'Initially uploaded to:'
   call lineout tempfile, '  <ftp://ftp.netlabs.org/incoming/' || release_file || '>'
   call lineout tempfile, 'Eventual location:'
   call lineout tempfile, '  <ftp://ftp.netlabs.org/pub/fm2/' || release_file || '>'
   call lineout tempfile, ''
   call lineout tempfile, 'HOBBES'
   call lineout tempfile, 'Search for it at:'
   call lineout tempfile, '  <http://hobbes.nmsu.edu/h-search.php?sh=1&button=Search&key=' || release_file || '&dir=%2Fpub>'
   call lineout tempfile, 'Initially uploaded to:'
   call lineout tempfile, '  <http://hobbes.nmsu.edu/pub/incoming/' || release_file || '>'
   call lineout tempfile, 'Eventual location:'
   call lineout tempfile, '  <http://hobbes.nmsu.edu/pub/os2/util/browser/' || release_file || '>'
/*
   call lineout tempfile, '  http://hobbes.nmsu.edu/h-search.php?sh=1&button=Search&key=fm2&dir=%2Fpub%2Fincoming'
   call lineout tempfile, 'which will eventually move to'
   call lineout tempfile, '  http://hobbes.nmsu.edu/h-search.php?sh=1&button=Search&key=fm2&dir=%2Fpub%2Fos2%2Futil%2Fbrowser'
*/
   call lineout tempfile, ''
   call lineout tempfile, ver.full 'Changes:'
   history_file = 'history'
   do until left(history_line || 'xxx', 3)  = ' o '
      history_line = linein(history_file)
   end
   history_line = strip(history_line, 'T')
   do until left(history_line, 1) > ' '
      if length(history_line) > 3 then
         if left(history_line, 3) = ' o ' then
            history_line = ' * ' || substr(history_line, 4)
      call lineout tempfile, history_line
      history_line = strip(linein(history_file), 'T')
   end
   call stream tempfile, 'c', 'close'
   call stream history_file, 'c', 'close'
return

SendEmail: procedure expose (globals)
   retval = -2    /* Assume error of some sort */
   say "Using ReleaseTool's internal SMTP code to send email..."
   say 'Connecting to mail server...'
   do 1
      socket = ConnectToMailServer(cfg.SMTP.currentSMTP.host, cfg.SMTP.currentSMTP.port)
      if socket < 0 then
         do
            say 'Unable to connect to' cfg.SMTP.currentSMTP.host
            call SockPSock_errno
            leave
         end
      reply = GetServerReply(socket)
      if left(reply, 3) \= '220' then
          do
             say 'Unexpected initial response from:' cfg.SMTP.currentSMTP.host
             say reply
             leave
          end
      reply = SendDataAndGetServerReply(socket, 'EHLO' cfg.myHostName)
      if left(reply, 3) \= '250' then
         do
            /* JBS: Send HELO instead? */
            say reply
            leave
         end
      select
         when pos('AUTH LOGIN', reply) > 0 then
            do
               /* AUTH LOGIN login here */
               reply = SendDataAndGetServerReply(socket, 'AUTH LOGIN')
               if left(reply, 3) \= '334' then
                  do
                     say 'Authorization failed (or unexpected response to AUTH LOGIN).' || cfg.crlf || reply
                     leave
                  end
               reply = SendDataAndGetServerReply(socket, EncodeB64(cfg.SMTP.currentSMTP.userid))
               if left(reply, 3) \= '334' then
                  do
                     say 'Authorization failed (or unexpected response to userid).' || cfg.crlf || reply
                     leave
                  end
               reply = SendDataAndGetServerReply(socket, EncodeB64(cfg.SMTP.currentSMTP.password))
               if left(reply, 3) \= '235' then
                  do
                     say 'Authorization failed (or unexpected response to password).' || cfg.crlf || reply
                     leave
                  end
            end
         when pos('AUTH PLAIN', reply) > 0 then
            do
               /* JBS: Implment AUTH PLAIN here? */
               /*
                * "The mechanism consists of a single message from the client to the server.
                * The client sends the authorization identity (identity to login as),
                * followed by a US-ASCII NULL character, followed by the authentication
                * identity (identity whose password will be used), followed by a US-ASCII
                * NULL character, followed by the clear-text password. The client may leave the
                * authorization identity empty to indicate that it is the same as the authentication identity."
                */
               say 'AUTH PLAIN not yet implemented!'
               leave
            end
         when pos('AUTH CRAM-MD5', reply) > 0 then
            do
               /* JBS: Implment AUTH CRAM-MD5 here? */
               /* JBS: If implemented, move to top to become "preferred" authentication? */
               say 'AUTH CRAM-MD5 not yet implemented!'
               leave
            end
         otherwise /* No or unknown authorization required */
            /* JBS: OK to assume no authorization? */
            say 'Assuming no authentication required...'
      end
      say 'Preparing data to send...'
      reply = SendDataAndGetServerReply(socket, 'MAIL FROM:<' || StrippedEmailAddress(cfg.SMTP.currentSMTP.from) || '>')
      if left(reply, 3) \= '250' then
         do
            say 'Unexpected response to MAIL FROM:' || cfg.crlf || reply
            leave
         end
      temp_to_list = email.to_list
      do until (temp_to_list = '' | left(reply, 3) \= 250)
         parse var temp_to_list addressee ',' temp_to_list
         reply = SendDataAndGetServerReply(socket, 'RCPT TO: <' || StrippedEmailAddress(addressee) || '>')
      end
      if left(reply, 3) \= '250' then
         do
            say 'Unexpected response to RCPT TO:' || cfg.crlf || reply
            leave
         end
      reply = SendDataAndGetServerReply(socket, 'DATA')
      if left(reply, 3) \= '354' then
         do
            say 'Unexpected response to DATA:' || cfg.crlf || reply
            leave
         end
      data = 'Date:' left(date('W'),3) || ', ' || date('N') || ' ' || time('N') || ' ' || cfg.SMTP.currentSMTP.UTCOffset || cfg.crlf
      /* JBS: Message Id? */
      data = data || 'To:' email.to_list || cfg.crlf
      data = data || 'From:' cfg.SMTP.currentSMTP.FROM || cfg.crlf
      /* JBS: Reply-to same as From? */
      data = data || 'Reply-To:' cfg.SMTP.currentSMTP.FROM || cfg.crlf
      data = data || 'Subject:' email.subject || cfg.crlf
      data = data || 'MIME-Version: 1.0' || cfg.crlf
      data = data || 'X-Mailer:' cfg.user_agent || cfg.crlf
      data = data || 'X-Mailer-Platform: OS/2; architecture=x86; version=20.45' || cfg.crlf
      data = data || cfg.crlf /* End of Header */
      do while lines(email.body_file) > 0
         line = linein(email.body_file)
         data = data || line || cfg.crlf
      end
      call stream email.body_file, 'c', 'close'

      data = data || cfg.SMTP.signature_preface || cfg.crlf   /* Start of signature */
      do i = 1 to cfg.SMTP.currentSMTP.signature.0
         data = data || cfg.SMTP.currentSMTP.signature.i || cfg.crlf
      end
      data = data || cfg.closing                             /* End of Message */
      say

/*
      call charout , 'OK to send email? (Y/n): '
      reply = translate(SysGetKey())
      say
      if reply \= 'N' then
*/

         do
            reply = SendDataAndGetServerReply(socket, data)
            if left(reply, 3) \= '250' then
               do
                  say 'Unexpected response to message send:' || cfg.crlf || reply
                  leave
               end
            else
               do
                  say 'Message sent successfully...'
                  retval = 0
               end
         end
      say 'Sending "QUIT" to server...'
      reply = SendDataAndGetServerReply(socket, 'QUIT')
      if left(reply, 3) \= '221' then
         do
            say 'Unexpected response to QUIT:' || cfg.crlf || reply
         end
   end
   if socket >= 0 then
      do
         say 'Disconnecting from server...'
         call SockClose(socket)
      end
return retval

StrippedEmailAddress: procedure
   parse arg email_address
   email_address = strip(email_address)
   if left(email_address, 1) = '"' then
      parse var email_address '"' . '"' email_address
return strip(strip(strip(email_address), 'L', '<'), 'T', '>')

GetServerReply: procedure expose (globals)
   /* JBS: 1-time receive with large buffer or as-many-as-needed receives? */
   max_recv_buffersize = 1024
   parse arg socket
   rc = SockRecv(socket, 'server_reply', max_recv_buffersize)
   select
      when rc < 0 then
         return 'ERROR on receive:' rc
      when rc = 0 then
         return 'ERROR on receive: Connection closed prematurely.'
      when rc = max_recv_buffersize then
         return 'ERROR on receive: Buffer size too small?'
      when pos(cfg.crlf, server_reply) = 0 then
         return 'ERROR No CRLF found. Buffer too small?' || cfg.crlf || 'Reply:' server_reply
      otherwise
         nop
   end
return server_reply

/**********************************************************************/
EncodeB64: procedure expose (globals) /* encodes a text string */
/**********************************************************************/
   parse arg Text  /* get the argument */
   B64Chars = xrange('A','Z')||xrange('a','z')||xrange('0','9')||'+/'  /* define the base64 character set */
   B64Str = ''  /* start with nothing */
   do while (length(Text) > 3)  /* go on while the length is sufficient */
      parse var Text NextBlock 4 Text  /* get the next block of 3 characters */
      NextBits = x2b(c2x(NextBlock))  /* convert it to 24 bits */
      do 4  /* do 4 times */
         parse var NextBits NextSext 7 NextBits  /* get the next sextet */
         B64Str = B64Str||substr(B64Chars,x2d(b2x(NextSext))+1,1)  /* convert to decimal, get the corresponding B64 character, and add */
      end
   end
   TextLeft = length(Text)  /* the number of 8-bit characters left (1, 2, or 3) */
   if (TextLeft > 0) then  /* if we have anything left */
      do
         NextBits = x2b(c2x(Text))||copies('00',(3-TextLeft))  /* convert to bits and add zeroes */
         do (TextLeft + 1)  /* do so many times */
            parse var NextBits NextSext 7 NextBits  /* get the next sextet */
            B64Str = B64Str||substr(B64Chars,x2d(b2x(NextSext))+1,1)  /* convert to decimal, get the corresponding B64 character, and add */
         end
         B64Str = B64Str||copies('=',3 - TextLeft)  /* add this */
      end
return B64Str  /* end of Encode64 */

ConnectToMailServer: procedure expose (globals)
   parse arg email_server_name, email_server_port
   if strip(email_server_port) = '' then
      email_server_port = 25

   myIPaddr = SockGetHostid()
   rc = SockGetHostByAddr(myIPaddr, 'hoststem.')
   if rc = 0 then
      do
         say 'Error: SockGetHostByAddr'
         return (-1000)
      end
   cfg.myHostName = hoststem.name
   rc = SockGetHostByName(email_server_name, 'hoststem.')
   if rc = 0 then
      do
         say 'Error: SockGetHostByAddr'
         return (-1001)
      end
   serverIPaddr = hoststem.addr

   socket = SockSocket('AF_INET', 'SOCK_STREAM', 'IPPROTO_TCP')
   if socket < 0 then
      exit_rc = -1
   else
      do
         inetaddr.family = 'AF_INET'
         inetaddr.addr   = serverIPaddr
         inetaddr.port   = email_server_port
         exit_rc         = SockConnect(socket, 'inetaddr.')
      end
   if exit_rc < 0 then
      return exit_rc
   else
      return socket

SendDataAndGetServerReply: procedure expose (globals)
   parse arg socket, data
   p = pos(cfg.crlf, data)
   if p > 1 then
      say 'Sending:' left(data, min(p - 1, 60)) || '...'
   else if length(data) >  60 then
      say 'Sending:' left(data, 60) || '...'
   else
      say 'Sending:' data
   rc = SockSend(socket, data || cfg.crlf)
   if rc <= 0 then
      do
         call SockPSock_errno
         return 'ERROR on send:' rc
      end
   else
      return GetServerReply(socket)

Init: procedure expose (globals)
   call RxFuncAdd 'SysLoadFuncs', 'REXXUTIL', 'SysLoadFuncs'
   call SysLoadFuncs

   call RxFuncAdd "SockLoadFuncs", "RXSOCK", "SockLoadFuncs"
   call SockLoadFuncs 0
   available.RXSOCK = 1

/*
   rcx = 0
   call RxFuncAdd "SockLoadFuncs", "RXSOCK", "SockLoadFuncs"
   signal on syntax name rxsockerr
   call SockLoadFuncs 0
   rcx = 1
rxsockerr:
   signal on syntax name Error
   rcx = 1
   if rcx = 1 then
      available.RXSOCK = 1
   else
      do
         available.RXSOCK = 0
         call RxFuncDrop "SockLoadFuncs", "RXSOCK", "SockLoadFuncs"
         say
         say 'RXSOCK.DLL load failed.'
         say 'Newsgroup announcements will not be possible.'
         say
         '@pause'
      end

*/

   rcx = 0
   call RxFuncAdd "FtpLoadFuncs", "RXFTP", "FTPLOADFUNCS"
   signal on syntax name rxftperr
   call FtpLoadFuncs 1
   rcx = FtpSetBinary("Binary")
rxftperr:
   signal on syntax name Error
   if rcx = 1 then
      available.RXFTP = 1
   else
      do
         available.RXFTP = 0
         say
         say 'RXFTP.DLL load failed.'
         say 'Uploads will not be possible.'
         say
         '@pause'
      end

   ver. = ''
   parse arg ver.in
   call SetVer
   ver.filelist = 'av2.def databar.def dirsize.def dll\fm3dll.def dll\fm3res.def'
   ver.filelist = ver.filelist 'dll\version.h eas.def fm3.def fm4.def global.def ini.def'
   ver.filelist = ver.filelist 'killproc.def sysinfo.def undel.def vcollect.def vdir.def'
   ver.filelist = ver.filelist 'viewinfs.def vtree.def file_id.diz'
   ver.filelist = ver.filelist 'warpin\makefile dll\copyright.h'

   Hobbes.                       = ''
   Hobbes.default_name           = 'Gregg Young'
   Hobbes.default_email          = 'ygk@qwest.net'
   Hobbes.TxtFilename            = '.\warpin\fm2-' || ver.wpi || '.txt'
   if stream(Hobbes.TxtFilename, 'c', 'query exists') \= ''then
      do
         call SysFileSearch 'Email address:', Hobbes.TxtFilename, 'lines.'
         if lines.0 = 1 then
            Hobbes.uploader_email_address = word(lines.1, 3)
         else
            Hobbes.uploader_email_address = 'N/A (' || Hobbes.TxtFilename 'has zero or more than one email addresses!)'
      end
   else
      Hobbes.uploader_email_address = 'N/A (' || Hobbes.TxtFilename 'does not yet exist!)'

   cmd.           = ''
   cmd.processor  = value('COMSPEC',,'OS2ENVIRONMENT')
   cmd.processor_is_4os2 = Is4OS2()
   if cmd.processor_is_4os2 = 1 then
      available.tee = 1
   else
      do
         if SysSearchPath('PATH', 'tee.exe') \= '' then
            available.tee = 1
         else
            do
               available.tee = 0
               call SysFileDelete 'tee.cmd'
               call lineout 'tee.cmd', '/* */'
               call lineout 'tee.cmd', 'parse arg filename'
               call lineout 'tee.cmd', 'call SysFileDelete filename'
               call lineout 'tee.cmd', 'do while lines() > 0'
               call lineout 'tee.cmd', '   parse pull line'
               call lineout 'tee.cmd', '   call lineout filename, line'
               call lineout 'tee.cmd', '   say line'
               call lineout 'tee.cmd', 'end'
               call lineout 'tee.cmd', "call stream filename, 'c', 'close'"
               call stream  'tee.cmd', 'c', 'close'
            end
      end
   userprompt = value('PROMPT',,'OS2ENVIRONMENT')
   parse var userprompt . '$e[' colorset 'm' .
   if colorset = '' then
      colorset = '0'
   if pos('$i', userprompt) > 0 then
      row = '2'
   else
      row = '1'
   cmd.prompt     = "$e[s$e[" || row || ";1H$e[34;47mType 'exit' to return to ReleaseTool$e[K$e[" || colorset || "m$e[u" || userprompt
   cmd.pager      = value('PAGER',,'OS2ENVIRONMENT')
   if cmd.pager = '' then
      if SysSearchPath( 'PATH', 'less.exe') \= '' then
         cmd.pager = 'less'
      else
         cmd.pager = 'more'

   cmd.editor = value('SVN_EDITOR',,'OS2ENVIRONMENT')
   cmd.editorparms = ""
   if cmd.editor = '' then
      cmd.editor = value('EDITOR',,'OS2ENVIRONMENT')
   if cmd.editor = '' then
      cmd.editor = 'tedit'
   else
      do
         upperwrd1 = translate(word(cmd.editor, 1))
         if upperwrd1 = 'EPM' | upperwrd1 = 'EPM.EXE' then
            cmd.editorparms = "/m '3'"
      end

   cmd.smartsvn   = SysSearchPath('PATH', 'smartsvn.cmd')
   cmd.tester     = value('SVN_TESTER',,'OS2ENVIRONMENT')
   cmd.killpid    = value('SVN_KILL',,'OS2ENVIRONMENT')
   if cmd.killpid = '' then
      cmd.killpid = 'killpid'
   cmd.killtarget = 'FM/2'

   m = 0

   m = m + 1
   mainmenu.Ensure_work_done_num       = m
   mainmenu.m.text                     = 'Ensure all work for this release is committed.'
   m = m + 1
   mainmenu.Verify_tickets_closed_num  = m
   mainmenu.m.text                     = 'Verify completed tickets are marked closed.'
   m = m + 1
   mainmenu.Local_SVN_update_num       = m
   mainmenu.m.text                     = '(SVN) Update local files.'
   m = m + 1
   mainmenu.Check_SVN_status_num       = m
   mainmenu.m.text                     = 'Check (SVN) status of local files.'
   m = m + 1
   mainmenu.Version_number_edits_num   = m
   mainmenu.m.text                     = 'Edit various files with version #''s and date''s.'
   m = m + 1
   mainmenu.WARNALL_build_num          = m
   mainmenu.m.text                     = 'Ensure the edits build. (WARNALL build)'
   m = m + 1
   mainmenu.Test_WARNALL_build_num     = m
   mainmenu.m.text                     = 'Test built code.'
   m = m + 1
   mainmenu.Commit_code_num            = m
   mainmenu.m.text                     = 'Commit code.'
   m = m + 1
   mainmenu.Build_all_for_release_num  = m
   mainmenu.m.text                     = 'Build (all) for the release.'
   m = m + 1
   mainmenu.Test_release_build_num     = m
   mainmenu.m.text                     = 'Test the release build.'
   m = m + 1
   mainmenu.LXLITE_build_num           = m
   mainmenu.m.text                     = 'Lxlite the build.'
   m = m + 1
   mainmenu.Test_LXLITE_build_num      = m
   mainmenu.m.text                     = 'Re-test the "lxlited" release code.'
   m = m + 1
   mainmenu.Build_WPI_file_num         = m
   mainmenu.m.text                     = 'Build distribution WPI file.'
   m = m + 1
   mainmenu.Test_WPI_file_num          = m
   mainmenu.m.text                     = 'Test WPI file.'
   m = m + 1
   mainmenu.Apply_tag_num              = m
   mainmenu.m.text                     = 'Apply tag.'
   m = m + 1
   mainmenu.Zip_files_num              = m
   mainmenu.m.text                     = 'Zip distribution WPI file(s).'
   m = m + 1
   mainmenu.WIKI_updates_num           = m
   mainmenu.m.text                     = 'Wiki updates.'
   m = m + 1
   mainmenu.Upload_num                 = m
   mainmenu.m.text                     = 'Upload to distribution points.'
   m = m + 1
   mainmenu.Announce_num               = m
   mainmenu.m.text                     = 'Announce the release.'
   m = m + 1
   mainmenu.TRAC_update_and_Next_ver_num  = m
   mainmenu.m.text                        = 'TRAC updates and set next version (no HISTORY or README update).'

   mainmenu.0 = m

   prev_user_choice = 'N/A'
return

SetVer: procedure expose (globals)
   do forever
      if ver.in = '' then
         do
            say
            call charout , 'Please enter the version (x.yy.z) for the pending release: '
            ver.in = linein()
         end
      parse var ver.in num1 '.' num2 '.' num3
      if num2    = '' then
         num2    = 0
      if num3 = '' then
         num3 = 0
      if datatype(num1) \= 'NUM' | datatype(num2) \= 'NUM' | datatype(num3) \= 'NUM' then
         do
            say 'Error: Invalid version entered:' ver.in
            ver.in = ''
         end
      else
         leave
   end
   /* The following will remove leading zeros */
   ver.major      = num1 + 0
   ver.minor      = num2 + 0
   ver.CSDlevel   = num3 + 0
   ver.full       = ver.major || '.' || ver.minor || '.' || ver.CSDlevel
   ver.list       = ver.major || '.' || right(ver.minor, 2, '0') || '.' || ver.CSDlevel
   ver.wpi        = translate(ver.list, '-', '.')
   ver.tag        = translate(ver.list, '_', '.')
return

CfgInit: procedure expose (globals)
   cfg.            = ''
/*
   trace '?i'
*/
   cfg.file        = 'ReleaseTool.cfg'
   cfg.file_exists = (stream(cfg.file, 'c', 'query exists') \= '')
   if  \cfg.file_exists then
     do
       template_file = left(cfg.file, lastpos('.', cfg.file)) || 'tmp'
       /* JBS: Check for missing template file? */
       no_cfg_option = 0
       do until wordpos(no_cfg_option, '1 2 3 4') > 0
         call SysCls
         say
         say 'No' cfg.file 'file found.'
         say
         say 'Options:'
         say '   1. Proceed without FTP, SMTP and NNTP features until one is configured.'
         say '   2. Copy the template file (' || template_file || ') to' cfg.file 'and proceed'
         say '         with (mostly) invalid CFG data.'
         say '   3. Copy the template file to' cfg.file 'and edit it right now with'
         say '         your SVN_EDITOR to suit your environment.'
         say '   4. Exit now, create your own' cfg.file 'and restart ReleaseTool later.'
         say
         call charout , 'Enter the number of your choice: '
         no_cfg_option = SysGetKey()
         say
         select
           when no_cfg_option = 1 then
             cfg.file_exists = 0
           when no_cfg_option = 2 then
             do
               call CopyTemplateFile template_file, cfg.file /* No return on error */
               cfg.file_exists = 1
             end
           when no_cfg_option = 3 then
             do
               call CopyTemplateFile template_file, cfg.file /* No return on error */
               cmd.editor cmd.editorparams cfg.file
               cfg.file_exists = 1
             end
           when no_cfg_option = 4 then
             exit
           otherwise
             nop
         end
       end
     end

   cfg.crlf       = '0D0A'x
   cfg.closing    = cfg.crlf || cfg.crlf || '.'
   cfg.user_agent = 'FM2ReleaseTool'

   cfg.FTP.0      = 0
   cfg.NNTP.0     = 0
   cfg.SMTP.0     = 0
   cfg.FTP.keys   = 'DESCRIPTION COMMAND HOST USERID PASSWORD DIRECTORY FILE'
   cfg.NNTP.keys  = 'DESCRIPTION COMMAND HOST USERID PASSWORD TO FROM SIGNATURE'
   cfg.SMTP.keys  = 'DESCRIPTION COMMAND HOST USERID PASSWORD PORT TO FROM SIGNATURE UTCOFFSET'

   cfg.NNTP.signature_preface     = cfg.crlf || '-- ' || cfg.crlf
   cfg.NNTP.mime_version          = '1.0'
   cfg.NNTP.content_type          = 'text/plain; charset=US-ASCII'
   cfg.NNTP.content_transfer_encoding = '8bit'
   cfg.maxNNTPrecvbufsize = 200   /* JBS: Better number? */
   /* JBS: Better name? */

   cfg.SMTP.signature_preface     = cfg.crlf || '-- ' || cfg.crlf

   retval = 0
   linenum = 0
   node = ''
   if cfg.file_exists then
     do
       do while (lines(cfg.file) > 0 & retval = 0)
          parse value linein(cfg.file) || ';' with line ';'
          linenum = linenum + 1
          line = strip(line)
          select
             when line = '' then
                nop      /* Skip blank/empty lines */
             when left(line, 1) = ';' then
                nop      /* Skip comment lines */
             when left(line, 1) = '[' then
                do
                   select
                      when node = 'FTP' then
                         do
                            cfg.node.n.directory.0 = d
                            cfg.node.n.directory.d.file.0 = f
                         end
                      when node = 'SMTP' | node = 'NNTP' then
                         do
                            cfg.node.n.signature.0 = s
                            s = 0
                         end
                      otherwise
                         nop
                   end
                   parse upper var line '[' node ']'
                   cfg.node.0 = cfg.node.0 + 1
                   n = cfg.node.0
                   d = 0
                   f = 0
                   s = 0
                end
             when pos('=', line) = 0 then
                retval = 10000 + linenum
             otherwise
                do
                   parse var line key_name '=' key_value
                   key_name = translate(strip(key_name))
                   key_value = strip(key_value)
                   p =  lastpos(';', key_value)
                   if p = 1 then
                      do
                         retval = 10000 + linenum
                         iterate
                      end
                   else if p > 1 then
                      key_value = strip(left(key_value, p - 1))
                   select
                      when wordpos(key_name, cfg.node.keys) = 0 then
                         retval = -(10000 + linenum)
                      when (node = 'SMTP' | node = 'NNTP') & key_name = 'SIGNATURE' then
                         do
                            s = s + 1
                            cfg.node.n.signature.s = key_value
                         end
                      when node = 'FTP' & key_name = 'DIRECTORY' then
                         do
                            d = d + 1
                            cfg.node.n.directory.d = key_value
                         end
                      when node = 'FTP' & key_name = 'FILE' then
                         do
                            if d = 0 then
                               do
                                  d = 1
                                  cfg.node.n.directory.d = ''
                               end
                            f = f + 1
                            cfg.node.n.directory.d.file.f = key_value
                         end
                      otherwise
                         cfg.node.n.key_name = key_value
                   end
                end
          end
       end
       if node = 'FTP' then
          do
             cfg.node.n.directory.0 = d
             cfg.node.n.directory.d.file.0 = f
          end
       else if (node = 'SMTP' | node = 'NNTP') then
          cfg.node.n.signature.0 = s
       call stream cfg.file, 'c', 'close'
     end
return retval

CopyTemplateFile: procedure
  parse arg template_file, cfg_file
  do i = 1 to 2
    if stream(template_file, 'c', 'query exists') = '' then
      do
        say
        say 'Unable to find template file:' template_file
        if i = 1 then
          do
            say 'Trying to get one from the repository...'
            'svn update' template_file
          end
        else
          do
            say 'svn update failed.'
            say 'Exiting...'
            exit
          end
      end
  end
  signal off error
  '@copy' template_file cfg_file '>NUL 2>NUL'
  if rc \= 0 then
    do
      say 'Copy of template file failed.'
      say 'Exiting...'
      exit
    end
  signal on error
return

DisplayMenu: procedure expose (globals)
   do forever
      call SysCls
      say;say;say
      say 'Release Tasks' || copies(' ', 35) || 'Previous action:' prev_user_choice
      say
      do m = 1 to mainmenu.0
         say right(m, 2) || '.' mainmenu.m.text
      end
      say
      call charout , 'N)ext C)ommandline '
/*
      call charout , 'cfG) '
*/
      if cmd.smartsvn \= '' then
         call charout , 'S)martSVN '
      call charout , 'Q)uit or menu item number: '
      user_choice = translate(translate(SysGetKey()), 'N', '0d'x)
      if user_choice = 'N' | /* JBS: Disabled internal CFG code: user_choice = 'G' | */ ,
         user_choice = 'C' | ,
         user_choice = 'Q' | ,
         (cmd.smartsvn \= '' & user_choice = 'S') then
         leave
      if datatype(user_choice) = 'NUM' then
         do
            user_choice = user_choice || linein()
            say
            if datatype(user_choice) = 'NUM' then
               if user_choice = trunc(user_choice) then
                  if user_choice > 0 & user_choice <= mainmenu.0 then
                     leave
         end
   end
   say
   if user_choice = 'N' then
      if prev_user_choice = 'N/A' then
         user_choice = 1
      else
         if prev_user_choice < mainmenu.0 then
            user_choice = prev_user_choice + 1
return user_choice

NotYet: procedure
   parse arg user_choice
   say;say
   say 'This option has not yet been (and may never be) automated.'
   say 'You will have to do this manually. See instructions below:'
   say;say
return

BuildHobbesTxt: procedure expose (globals)
   if stream(Hobbes.TxtFilename, 'c', 'query exists') \= '' then
      do
         say;say;say
         say Hobbes.TxtFilename 'already exists!'
         call charout , 'Do you want to replace this file? (y/N) '
         if translate(SysGetKey()) \= 'Y' then
            do
               say;say;
               say Hobbes.TxtFilename 'update aborted.'
               return
            end
         call SysFileDelete Hobbes.TxtFilename
      end
   /* Prompt for user input (name, email, permission to email, previous version) here? */
   default_OKtoListEmail   = 'yes'
   entry = ''
   do until (entry = 'Y' | entry = '0d'x)
      say;say
      say 'You will now be prompted for potentially variable fields within:'
      say '   ' Hobbes.TxtFilename
      say
      say 'A default value will be given for most fields.'
      say 'To accept the default just press the Enter key.'
      say
      call charout , 'Name of the releaser (default:' Hobbes.default_name '): '
      entry = strip(linein())
      if entry = '' then
         name = Hobbes.default_name
      else
         name = entry
      say;say
      call charout , 'Email address of the releaser (default:' Hobbes.default_email '): '
      entry = strip(linein())
      if entry = '' then
         Hobbes.uploader_email_address = Hobbes.default_email
      else
         Hobbes.uploader_email_address = entry
      say;say
      call charout , 'OK to list email address of the releaser (default:' default_OKtoListEmail '): '
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
      say 'Please enter the version of the zip file to be replaced:'
      replaced_ver_zip = 'fm2-' || translate(linein(), '-', '.') || '.zip'
      say;say
      say 'Data entered:'
      say '  Name of releaser   :' name
      say '  Email of releaser  :' Hobbes.uploader_email_address
      say '  OK to list email   :' OKtoListEmail
      say '  Zip to be replaced :' replaced_ver_zip
      say;say
      call charout , 'OK to proceed with file write? (Y/n) '
      entry = translate(SysGetKey())
      say
   end

   rm1 = 73
   rm2 = 25
   call lineout Hobbes.TxtFilename, right('Upload Information Template for Hobbes.nmsu.edu', rm1)
   call lineout Hobbes.TxtFilename, right(copies('=', length('Upload Information Template for Hobbes.nmsu.edu')), rm1)
   call lineout Hobbes.TxtFilename, ''
   call lineout Hobbes.TxtFilename, right('Archive Filename:', rm2) || ' fm2-' || ver.wpi || '.wpi'
   call lineout Hobbes.TxtFilename, right('Short Description:', rm2) || ' Mark Kimes FM/2 File Manager ' || ver.full
   call lineout Hobbes.TxtFilename, right('Long Description:', rm2) || ' The package contains a warpin package for installing FM2.'
   call lineout Hobbes.TxtFilename, copies(' ', rm2) || ' The FM2 source code is available on netlabs.'
   call lineout Hobbes.TxtFilename, right('Proposed directory ', rm2)
   call lineout Hobbes.TxtFilename, right('for placement:', rm2) || ' os2/util/browser'
   call lineout Hobbes.TxtFilename, ''
   call lineout Hobbes.TxtFilename, right('Your name:', rm2) name
   call lineout Hobbes.TxtFilename, right('Email address:', rm2) Hobbes.uploader_email_address
   call lineout Hobbes.TxtFilename, right('Program contact name:', rm2) || ' (same)'
   call lineout Hobbes.TxtFilename, right('Program contact email:', rm2) || ' (same)'
   call lineout Hobbes.TxtFilename, right('Program URL:', rm2) || ' http://svn.netlabs.org/fm2'
   call lineout Hobbes.TxtFilename, ''
   call lineout Hobbes.TxtFilename, right('Would you like the ', rm2)
   call lineout Hobbes.TxtFilename, right('contact email address ', rm2)
   call lineout Hobbes.TxtFilename, right('included in listings?', rm2) OKtoListEmail
   call lineout Hobbes.TxtFilename, ''
   call lineout Hobbes.TxtFilename, right('Operating System/Version:', rm2) || ' OS/2 Warp 3.0 and up.'
   call lineout Hobbes.TxtFilename, right('Additional requirements:', rm2)
   call lineout Hobbes.TxtFilename, ''
   call lineout Hobbes.TxtFilename, right('Replaces:', rm2) replaced_ver_zip
   call stream  Hobbes.TxtFilename, 'c', 'close'
   say;say;say
   say Hobbes.TxtFilename 'has been written.'
return

Commandline: procedure expose (globals)
   signal off Error
   if cmd.processor_is_4os2 = 1 then
      '@' || cmd.processor "prompt"  cmd.prompt
   else
      '@' || cmd.processor "/k prompt=" || cmd.prompt
   signal on Error
return

ExecCmd: procedure
   parse arg command
   signal off Error
   command
   rcx = rc
   signal on Error
return rcx

/*=== Is4OS2() Return true if 4OS2 running ===*/
Is4OS2: procedure
  call setlocal
  '@set X=%@eval[0]'
  yes = value('X',, 'OS2ENVIRONMENT') = 0
  call endlocal
return yes				/* if running under 4OS2 */

RunTester: procedure expose (globals)
/* kills FM/2 using killpid from FM/2 utils (must be in path) ... */
   call ExecCmd cmd.killpid cmd.killtarget
/* and run cmd to copy files to test directory and change to that directory must type exit to return here */
   call ExecCmd cmd.processor cmd.tester
return

TestWPI: procedure expose (globals)
   parse arg tasknum
   if cmd.wic = '' then
      do
         warpin_path = strip(SysIni('USER', 'WarpIN', 'Path'), 'T', '00'x)
         if warpin_path = 'ERROR:' then
            do
               call NewScreenWithHeading tasknum
               say
               say
               say 'Unable to find Warpin directory in OS2.INI file.'
               say
               say 'Testing the WPI file is not possible without it.'
               say
               say 'Please reinstall Warpin and try again.'
               say
            end
         else
            do
               cmd.wic = warpin_path || '\wic.exe'
               cmd.warpin = warpin_path || '\warpin.exe'
               beginlibpath = SysQueryExtLibpath('B')
               if pos( ';' || translate(warpin_path) || ';', ';' || translate(beginlibpath) || ';') = 0 then
                  do
                     call SysSetExtLibpath warpin_path || ';' || beginlibpath, 'B'
                  end
               warpin_ini = warpin_path || '\datbas_' || left(SysBootDrive(), 1) || '.ini'
               cmd.wpi_viewer = value('SVN_WPIVIEWER',,'OS2ENVIRONMENT')
               if cmd.wpi_viewer = '' then
                  do
                     cmd.wpi_viewer = value('WPIVIEWER',,'OS2ENVIRONMENT')
                     if cmd.wpi_viewer = '' then
                        cmd.wpi_viewer = SysSearchPath('PATH', 'wpiview.exe')
                  end
            end
      end
   if cmd.wic \= '' then
      do
         previous_choice = 0
         do until choice = 'Q'
            call NewScreenWithHeading tasknum
            say
            say '1. View FM/2 WPI file'
            say '2. De-install FM/2 and fully delete FM/2 installation directories.'
            say '3. Install FM/2'
            say '4. Test the new installation.'
            say '5. Create ZIP file for debugging WPI file problems.'
            say
            call charout , 'N)ext C)ommandline Q)uit (to main menu) or number of choice: '
            choice = translate(translate(SysGetKey()), 'N', '0d'x)
            say
            if choice = 'N' then
               if previous_choice = '5' then
                  iterate
               else
                  choice = previous_choice + 1
            select
               when choice = 'C' then
                  call Commandline
               when choice = '1' then
                  do
                     if cmd.wpi_viewer = '' then
                        do
                           say
                           say 'This program has searched for w WPI viewer (in the order listed):'
                           say '  1. Environment variable: SVN_WPIVIEWER'
                           say '  2. Environment variable: WPIVIEWER'
                           say '  3. WPIView.exe on the PATH'
                           say
                           say 'A viewer was not found. Using wic.exe instead...'
                           say
                           '@pause'
                           call ExecCmd cmd.wic '-l warpin\fm2-' || ver.wpi || '.wpi | rxqueue'
                           p = 0
                           do while queued() > 0
                              parse pull line
                              if word(line, 1) = 'Package' then
                                 do
                                    p = p + 1
                                    pkg.p.num = word(line, 2)
                                 end
                           end
                           pkg.0 = p
                           do p = 1 to pkg.0
                              call ExecCmd cmd.wic '-l warpin\fm2-' || ver.wpi || '.wpi' pkg.p.num '|' cmd.pager
                           end
                           if pos('less', cmd.page) = 0 then 'pause'
                        end
                     else
                        do
                           cmd.wpi_viewer 'warpin\fm2-' || ver.wpi || '.wpi'
                        end
                  end
               when choice = '2' then
                  do
                     deinstall. = ''
                     call SysIni warpin_ini, 'ALL:', 'warpin_apps.'
                     do i = 1 to warpin_apps.0
                        select
                           when left(warpin_apps.i, 17) = 'Netlabs\FM2\Base\'then
                              deinstall.base_dir_before  = strip(SysIni( warpin_ini, warpin_apps.i, 'TargetPath' ), 'T', '00'x)
                           when left(warpin_apps.i, 31) = 'Netlabs\FM2 Utilities\FM2Utils\'then
                              deinstall.utils_dir_before = strip(SysIni( warpin_ini, warpin_apps.i, 'TargetPath' ), 'T', '00'x)
                           otherwise
                              nop
                        end
                     end
                     drop warpin_apps.
                     say
                     say 'There are two parts to this process:'
                     say '  1. Invoking WarpIN to deinstall FM/2'
                     say "  2. Using the DELTREE utility to delete all remaining files and directories."
                     say
                     say 'Part 1: Warpin de-install. WarpIN is a PM program and the deinstallation will'
                     say 'not be automated by this program. Please follow these steps once WarpIN'
                     say 'has started:'
                     say '  a) It will appear just as a normal installation. Proceed to the window where'
                     say '     packages are selected.'
                     say '  b) If you want to deinstall ALL packages, use the "Selections" menu and'
                     say '     choose "De-install". Otherwise, for each package you want to de-install,'
                     say '     double click on its icon until it changes to one with a red "X"'
                     say '  c) Once the packages to be de-installed each have an icon with a red "X",'
                     say '     click on "Next".'
                     say '  d) Then click on "Install" in the next window.'
                     say '  e) You should get a message box asking if it is OK to proceed.'
                     say '     Click on OK when ready.'
                     say
                     say 'WarpIN will be started asyncronously. This will allow you to switch back'
                     say 'to this window for these instructions, if needed. Once WarpIN has finished,'
                     say 'switch back to this window.'
                     say
                     call charout , 'Press any key when ready to start WarpIN...'
                     call SysGetKey
                     say
                     call ExecCmd 'start' cmd.warpin 'warpin\fm2-' || ver.wpi || '.wpi'
                     say
                     do until choice2 = 'Y' | choice2 = 'N'
                        say
                        say 'Part 2: Deleting the remaining files'
                        say
                        call charout , 'Has WarpIN finshed completely and is it OK to proceed? (y/n): '
                        choice2 = translate(SysGetKey())
                        say;say
                     end
                     if choice2 = 'Y' then
                        do
                           call SysIni warpin_ini, 'ALL:', 'warpin_apps.'
                           do i = 1 to warpin_apps.0
                              select
                                 when left(warpin_apps.i, 17) = 'Netlabs\FM2\Base\'then
                                    deinstall.base_dir_after  = strip(SysIni( warpin_ini, warpin_apps.i, 'TargetPath' ), 'T', '00'x)
                                 when left(warpin_apps.i, 31) = 'Netlabs\FM2 Utilities\FM2Utils\'then
                                    deinstall.utils_dir_after = strip(SysIni( warpin_ini, warpin_apps.i, 'TargetPath' ), 'T', '00'x)
                                 otherwise
                                    nop
                              end
                           end
                           drop warpin_apps.
                           if deinstall.base_dir_after = '' then
                              if deinstall.base_dir_before \= '' then
                                 do
                                    do until (choice2 = 'Y' | choice2 = 'N')
                                       say
                                       say 'You have de-installed the FM/2 base package.'
                                       say
                                       say 'Next is the deletion of ALL files and directories in its installation directory:'
                                       say '   ' deinstall.base_dir_before
                                       call charout , 'Are you sure you want to proceed? (y/N): '
                                       choice2 = translate(SysGetKey())
                                    end
                                    if choice2 = 'Y' then
                                       do
                                          signal off Error
                                          'warpin\fm2utils\deltree' deinstall.base_dir_before
                                          signal on Error
                                       end
                                 end
                           if deinstall.utils_dir_after = '' then
                              if deinstall.utils_dir_before \= '' then
                                 do
                                    call SysFileTree deinstall.utils_dir_before, 'dirlist.', 'DO'
                                    if dirlist.0 = 1 then
                                       do
                                          do until (choice2 = 'Y' | choice2 = 'N')
                                             say
                                             say 'You have de-installed the FM/2 Utilities package.'
                                             say
                                             say 'Next is the deletion of ALL files and directories in its installation directory:'
                                             say '   ' deinstall.utils_dir_before
                                             call charout , 'Are you sure you want to proceed? (y/N): '
                                             choice2 = translate(SysGetKey())
                                          end
                                          if choice2 = 'Y' then
                                             do
                                                call ExecCmd 'warpin\fm2utils\deltree' deinstall.utils_dir_before
                                             end
                                       end
                                 end
                        end
                  end
               when choice = '3' then
                  do
                     say
                     say 'Opening a commandline session for testing...'
                     say
                     call ExecCmd cmd.warpin 'warpin\fm2-' || ver.wpi || '.wpi'
                  end
               when choice = '4' then
                  do
                     call SysIni warpin_ini, 'ALL:', 'warpin_apps.'
                     do i = 1 to warpin_apps.0
                        if left(warpin_apps.i, 17) = 'Netlabs\FM2\Base\'then
                           do
                              fm2_installation_dir  = SysIni( warpin_ini, warpin_apps.i, 'TargetPath' )
                              leave
                           end
                     end
                     drop warpin_apps.
                     curdir = directory()
                     installation_drive = left(fm2_installation_dir, 2)
                     curdir2 = directory(installation_drive)
                     call directory fm2_installation_dir
                     call Commandline
                     call directory installation_drive
                     call directory curdir2
                     call directory left(curdir, 2)
                     call directory curdir
                  end
               when choice = '5' then
                  do
                     if cmd.zip = '' then
                        cmd.zip = SysSearchPath('PATH', 'zip.exe')
                     if cmd.zip = '' then
                        do
                           say
                           say 'ZIP.EXE was not found on the PATH. Please correct'
                           say 'this ASAP and execute this option again.'
                        end
                     else
                        do
                           tmpzipfile = SysTempFilename('rtdbg???.zip')
                           tmpdirfile = SysTempFilename('rtdbgdir.???')
                           call ExecCmd '@dir * /s >' tmpdirfile
                           call ExecCmd cmd.zip '-9X' tmpzipfile 'warpin\fm2-' || ver.wpi || '.wpi'
                           call ExecCmd cmd.zip '-9X' tmpzipfile 'warpin\bld_fm2_wpidirs.*'
                           call ExecCmd cmd.zip '-9X' tmpzipfile 'warpin\fm2.wis.in'
                           call ExecCmd cmd.zip '-9mX' tmpzipfile tmpdirfile
                           say
                           say 'If the preceding zip commands were successful, please send an email to'
                           say '     jsmall@os2world.net'
                           say 'and attach the file:'
                           say '   ' tmpzipfile
                           say 'as an attachment. In the text of the message, please describe'
                           say 'the errors you detected in detail. After the email has been sent'
                           say 'you may delete:'
                           say '   ' tmpzipfile
                           say
                           'pause'
                        end
                  end
               otherwise
                  nop
            end
         end
      end
return

UploadRelease: procedure expose (globals)
  rcx = 0
  select
    when (available.RXFTP = 0 | \cfg.file_exists) then
      do
        if \cfg.file_exists then
          say 'Until' cfg.file ' has been created and customized,'
        else
          say 'Until RXFTP.DLL is placed in a directory on the LIBPATH,'
        say 'manual uploading will be required.'
        say
        call charout , 'Would you like a commandline session to upload? (Y/n) '
        reply = translate(SysGetKey())
        say
        if reply \= 'N' then
           call Commandline
        else
           rcx = 1
        say
      end
    otherwise
      do
         retval = 0
         do u = 1 to cfg.FTP.0
            say 'Uploading to' cfg.FTP.u.description || '...'
            if cfg.FTP.u.command \= '' then
               do
                  say 'Using external command(s) to upload...'
                  _command = cfg.FTP.u.command
                  do while pos('##HOST##', _command) > 0
                     parse var _command part1 '##HOST##' part2
                     _command = part1 || cfg.FTP.u.host || part2
                  end
                  do while pos('##PORT##', _command) > 0
                     parse var _command part1 '##PORT##' part2
                     _command = part1 || cfg.FTP.u.port || part2
                  end
                  do while pos('##USERID##', _command) > 0
                     parse var _command part1 '##USERID##' part2
                     _command = part1 || cfg.FTP.u.userid || part2
                  end
                  do while pos('##PASSWORD##', _command) > 0
                     parse var _command part1 '##PASSWORD##' part2
                     _command = part1 || cfg.FTP.u.password || part2
                  end
                  do while pos('[Release-zip]', _command) > 0
                     parse var _command part1 '[Release-zip]' part2
                     _command = part1 || 'warpin\fm2-' || ver.wpi || '.zip' || part2
                  end
                  do while pos('[Hobbes-text]', _command) > 0
                     parse var _command part1 '[Hobbes-text]' part2
                     _command = part1 || 'warpin\fm2-' || ver.wpi || '.txt' || part2
                  end
                  rcx = ExecCmd(_command)
                  say
               end
            else
               do
                  say '   Setting up logon data...'
                  if cfg.FTP.u.password = '[Hobbes-email]' then
                     if left(Hobbes.uploader_email_address, 3) = 'N/A' then
                        cfg.FTP.u.password = ''
                     else
                        cfg.FTP.u.password = Hobbes.uploader_email_address
                  if (cfg.FTP.u.userid = '' | cfg.FTP.u.password = '') then
                     do
                        say
                        say '      The userid and/or password were not found in ReleaseTool.cfg.'
                        say '      You will now be prompted for the missing data.'
                        say
                        if cfg.FTP.u.userid = '' then
                           do
                              call charout , '      Please enter the userid for' cfg.FTP.u.descriptive_hostname ||': '
                              cfg.FTP.u.userid = strip(linein())
                              say
                           end
                        say
                        if cfg.FTP.u.password = '' then
                           do
                              call charout , '      Please enter the password for' cfg.FTP.u.descriptive_hostname ||': '
                              cfg.FTP.u.password  = strip(linein())
                              say
                           end
                        say '      In order to avoid being prompted in the future, edit the'
                        say '      ReleaseTool.cfg file to include this data.'
                        say
                     end
                  rcx = FtpSetUser( cfg.FTP.u.host, StripNotNeeded(cfg.FTP.u.userid), StripNotNeeded(cfg.FTP.u.password), '')
                  if rcx \= 1 then
                     say '      Unable to set user data. Unable to continue.'
                  else
                     do
                        do d = 1 to cfg.FTP.u.directory.0
                           if cfg.FTP.u.directory.d \= '' then
                              do
                                 say '   Changing directory to:' cfg.FTP.u.directory.d
                                 rcx = FtpChDir(cfg.FTP.u.directory.d)
                                 if rcx \= 0 then
                                    do
                                       say '   Unable to change directory. FTP Error:' FTPERRNO
                                       iterate
                                    end
                              end
                           do f = 1 to cfg.FTP.u.directory.d.file.0
                              select
                                 when cfg.FTP.u.directory.d.file.f = '[Release-zip]' then
                                    uploadfile = 'warpin\fm2-' || ver.wpi || '.zip'
                                 when cfg.FTP.u.directory.d.file.f = '[Hobbes-text]' then
                                    uploadfile = 'warpin\fm2-' || ver.wpi || '.txt'
                                 otherwise
                                    uploadfile = cfg.FTP.u.directory.d.file.f
                              end
                              say '   Uploading:' uploadfile
                              rcx = FtpPut(uploadfile, filespec('n', uploadfile), "Binary")
                              if rcx \= 0 then
                                 do
                                    say '   Unable to upload. FTP Error:' FTPERRNO
                                    leave
                                 end
                           end
                        end
                        say '   Logging off' cfg.FTP.u.descriptive_hostname || '...'
                        call FtpLogoff
                        say
                     end
               end
            if rcx \= 0 then
               leave
         end
      end
  end
return rcx

AnnounceToNewsgroups: procedure expose (globals)
  if \cfg.file_exists then
    do
      say 'Unable to post to newsgroups without the configuration file:' cfg.file
      say
      return
    end
  body_file = SysTempFilename('NNTPBody.???')
  call SetDefaultAnnouncementText body_file
  cfg.NNTP.subject = 'FM/2' ver.full 'has been released.'
  _text = '<Standard>'
  do until ((option = 'C') | (option = 'Q'))
     call SysCls
     say
     say 'News: Verify/edit content'
     say
     say 'Subject  :' cfg.NNTP.subject
     say 'Text     :' _text
     say
     say 'Type "C" to confirm the above and proceed.'
     say '     "Q" to abort.'
     say '     "S" to edit the subject.'
     say '     "T" to edit the message text (in an editor).'
     say
     call charout, '==> '
     option = translate(SysGetKey())
     say
     say
     select
        when ((option = 'C') | (option = 'Q')) then
           nop
        when option = 'S' then
           do
              say 'Enter the Subject (followed by the ENTER key):'
              cfg.NNTP.subject = linein()
           end
        when option = 'T' then
           do
              b4_timestamp = SysGetFileDateTime(body_file)
              say 'The current body of the newsgroup message will now be loaded into an editor.'
              say 'Make desired changes, if any, and save the file.'
              say
              call charout, 'Press any key when ready to load the message body into an editor: '
              call SysGetKey
              say
              call ExecCmd cmd.editor body_file
              if b4_timestamp \= SysGetFileDateTime(body_file) then
                 _text = '<Modified>'
           end
        otherwise
           nop
     end
  end
  if option = 'Q' then
     rcx = -1      /* User aborted operation */
  else
     do s = 1 to cfg.NNTP.0
        call SysCls
        say
        do until ((option = 'C') | (option = 'Q'))
           call SysCls
           say
           say 'News: Verify/edit server-specific data for' cfg.NNTP.s.description
           say
           say 'Host     :' cfg.NNTP.s.host
           say 'To       :' cfg.NNTP.s.to
           say 'From     :' cfg.NNTP.s.from
           say 'UserID   :' cfg.NNTP.s.userid
           say 'Password :' cfg.NNTP.s.password
           say
           say 'Type "C" to confirm the above and send.'
           say '     "Q" to abort sending this email.'
           say '     "H" to change the name of the host.'
           say '     "T" to change the list of newsgroups.'
           say '     "F" to change the From address.'
           say '     "U" to change the Userid.'
           say '     "P" to change the Password.'
           say
           call charout, '==> '
           option = translate(SysGetKey())
           say
           say
           say
           select
              when option = 'H' then
                 do
                    say 'Enter the newsgroup host to use (followed by the ENTER key).'
                    cfg.NNTP.s.host = linein()
                 end
              when option = 'T' then
                 do
                    say 'Enter a comma=separated list of newagroup(s) (followed by the ENTER key).'
                    cfg.NNTP.s.to = linein()
                 end
              when option = 'F' then
                 do
/*
                    say 'F.Y.I. The Hobbes email address is:' Hobbes.uploader_email_address
                    say
                 if reply \= '' then
                    cfg.NNTP.s.from = reply
                 else
                    cfg.NNTP.s.from = Hobbes.uploader_email_address
*/
                    say 'Enter the From email address. (followed by the ENTER key).'
                    say '(You may want to disguise the address to avoid spam.)'
                    cfg.NNTP.s.from = linein()
                 end
              when option = 'U' then
                 do
                    say 'Enter the UserID (followed by the ENTER key):'
                    cfg.NNTP.s.userid = linein()
                 end
              when option = 'P' then
                 do
                    say 'Enter the Password (followed by the ENTER key):'
                    cfg.NNTP.s.password = linein()
                 end
              otherwise
                 nop
           end
        end
        if option = 'C' then
          do
           if cfg.NNTP.s.command \= '' then
              do
                 say 'Using external command(s) to post to newsgroup(s)...'
                 _command = cfg.NNTP.s.command
                 do while pos('##HOST##', _command) > 0
                    parse var _command part1 '##HOST##' part2
                    _command = part1 || cfg.NNTP.s.host || part2
                 end
                 do while pos('##PORT##', _command) > 0
                    parse var _command part1 '##PORT##' part2
                    _command = part1 || cfg.NNTP.s.port || part2
                 end
                 do while pos('##USERID##', _command) > 0
                    parse var _command part1 '##USERID##' part2
                    _command = part1 || cfg.NNTP.s.userid || part2
                 end
                 do while pos('##PASSWORD##', _command) > 0
                    parse var _command part1 '##PASSWORD##' part2
                    _command = part1 || cfg.NNTP.s.password || part2
                 end
                 do while pos('##MESSAGE_BODY_FILE##', _command) > 0
                    parse var _command part1 '##MESSAGE_BODY_FILE##' part2
                    _command = part1 || stream(body_file, 'c', 'query exists') || part2
                 end
                 do while pos('##FROM##', _command) > 0
                    parse var _command part1 '##FROM##' part2
                    _command = part1 || cfg.NNTP.s.from || part2
                 end
                 do while pos('##TO##', _command) > 0
                    parse var _command part1 '##TO##' part2
                    _command = part1 || cfg.NNTP.s.to || part2
                 end
                 rcx = ExecCmd(_command)
              end
           else
              do
                 cfg.NNTP.message_body = ''
                 do while lines(body_file) > 0
                    cfg.NNTP.message_body = cfg.NNTP.message_body || linein(body_file) || cfg.crlf
                 end
                 call stream body_file, 'c', 'close'
                 rcx = SendNNTP(s)
              end
            if rcx \= 0 then 'pause'
          end
        else
           rcx = -1 /* User aborted */
     end
  call SysFileDelete body_file
return rcx

SendNNTP: procedure expose (globals)
   parse arg s
   say
   say 'NNTP: Sending to' cfg.NNTP.s.host '...'
   exit_rc = SockGetHostByName( cfg.NNTP.s.host, 'hostip.' )
   if exit_rc = 0 then
      do
         say 'NNTP: DNS lookup on' cfg.NNTP.s.host 'failed.'
      end
   else
      do
         socket = SockSocket('AF_INET', 'SOCK_STREAM', 'IPPROTO_TCP')
         if socket < 0 then
            do
               say 'NNTP: Unable to create a socket for use with' cfg.NNTP.s.host || '.'
               iterate
            end
         else
            do
               inetaddr.family = 'AF_INET'
               inetaddr.addr   = hostip.addr
               inetaddr.port   = 119
               exit_rc         = SockConnect(socket, 'inetaddr.')
            end
         if exit_rc < 0 then
            do
               say 'NNTP: Unable to connect to' cfg.NNTP.s.host || '.'
            end
         else
            do forever
               parse value NNTPReceive(socket, cfg.maxNNTPrecvbufsize) with rc server_reply
               if rc \= 0 then leave
               say 'NNTP: Sending initial POST...'
               rc = SockSend(socket, 'POST' || cfg.crlf)
               if rc < 0 then
                  do
                     say 'NNTP: Failed on sending initial POST request.'
                     leave
                  end
               say 'NNTP: Receiving response to initial POST...'
               parse value NNTPReceive(socket, cfg.maxNNTPrecvbufsize) with rc server_reply
               if pos('480', server_reply) > 0 then /* authentication required */
                  do
                     say 'NNTP: Sending userid...'
                     rc = SockSend(socket, 'AUTHINFO USER' StripNotNeeded(cfg.NNTP.1.userid) || cfg.crlf)
                     if rc < 0 then
                        do
                           say 'NNTP: Failed on sending userid.'
                           leave
                        end
                     say 'NNTP: Receiving response to userid...'
                     parse value NNTPReceive(socket, cfg.maxNNTPrecvbufsize) with rc server_reply
                     if pos('381', server_reply) > 0 then /* further authentication (password) required */
                        do
                     say 'NNTP: Sending password...'
                           rc = SockSend(socket, 'AUTHINFO PASS' StripNotNeeded(cfg.NNTP.1.password) || cfg.crlf)
                           if rc < 0 then
                              do
                                 say 'NNTP: Failed on sending password.'
                                 leave
                              end
                        end
                     else
                        if pos('501', server_reply) > 0 then
                           do
                              say 'NNTP: Bad authentication'
                              leave
                           end
                     say 'NNTP: Receiving response to password...'
                     parse value NNTPReceive(socket, cfg.maxNNTPrecvbufsize) with rc server_reply
                     if rc \= 0 then leave
                     if pos('281', server_reply) = 0 then
                        do
                           say 'Bad response to password:' server_reply
                           leave
                        end
                     say 'NNTP: Sending POST (After authorization)...'
                     rc = SockSend(socket, 'POST' || cfg.crlf)
                     if rc < 0 then
                        do
                           say 'NNTP: Failed on sending second POST request.'
                           leave
                        end
                     say 'NNTP: Receiving response to POST (After authorization)...'
                     parse value NNTPReceive(socket, cfg.maxNNTPrecvbufsize) with rc server_reply
                     if rc \= 0 then leave
                  end
               if pos('340', server_reply) > 0 then /* OK to send */
                  do
                     say 'NNTP: Sending message...'
                     NNTPsig = ''
                     do i = 1 to cfg.NNTP.s.signature.0
                        NNTPsig = NNTPsig || cfg.NNTP.s.signature.i || cfg.crlf
                     end
                     rc = SockSend(socket, 'From:' cfg.NNTP.s.from || cfg.crlf || ,
                                           'Newsgroups:' cfg.NNTP.s.to || cfg.crlf || ,
                                           'Subject:' cfg.NNTP.subject || cfg.crlf || ,
                                           'User-Agent:' cfg.user_agent || cfg.crlf || ,
                                           'MIME-version:' cfg.NNTP.mime_version || cfg.crlf || ,
                                           'Content-Type:' cfg.NNTP.content_type || cfg.crlf || ,
                                           'Content-Transfer-Encoding:' cfg.NNTP.content_transfer_encoding || cfg.crlf || ,
                                           cfg.crlf || ,
                                           cfg.NNTP.message_body || ,
                                           cfg.NNTP.signature_preface || ,
                                           NNTPsig || ,
                                           cfg.closing || cfg.crlf)
                     if rc < 0 then
                        do
                           say 'NNTP: Send message failed.'
                           leave
                        end
                     say 'NNTP: Receiving response to message send...'
                     parse value NNTPReceive(socket, cfg.maxNNTPrecvbufsize) with rc server_reply
                     if rc \= 0 then leave
                     if pos('240', server_reply) = 0 then
                        do
                           say 'NNTP: Unknown response from server to message send:' server_reply
                        end
                  end
               else
                  say 'NNTP: Failed to get "OK to send" respsonse from server.'
               leave
            end
         call SockSoClose(socket)

      end
return rc

NNTPReceive: procedure expose (globals)
   parse arg socket, maxdatasize
   reply = ""
   exit_rc = 10
   do while (exit_rc = 10)
      rc = SockRecv(socket, 'indata', maxdatasize )
      select
         when rc > 0 then
            do
               reply = reply || indata
               if pos(cfg.crlf, reply) > 0 then
                  do
                     exit_rc = 0
                     if rc = maxdatasize then
                        say 'NNTP: Receive buffer size,' masdatasize || ', too small???'
                  end

            end
         when rc = 0 then /* connection closed */
            do
               say "NNTP: Connection closed prematurely."
               exit_rc = 4
            end
         otherwise /* Error occurred */
            do
               exit_rc = rc
               /* JBS: SockPSock_errno write to STDERR, is this OK? */
               call SockPSock_errno
            end
      end
   end
   if exit_rc \= 0 then
      do
         say 'NNTP: Error on initial response.'
         say 'NNTP: Error code:' rc
         say 'NNTP: Text returned:' server_reply
      end
return exit_rc reply

ZipFiles: procedure expose (globals)
   release_zipfile = '.\warpin\fm2-' || ver.wpi || '.zip'
   /* Zip distro */
   'zip -jX9' release_zipfile 'warpin\fm2-' || ver.wpi || '.wpi'

   /* Zip FM/2 Utilities' */
   /* JBS: Following code is temprarily abandoned */
/*
   call SysFileTree 'warpin\fm2utils*.wpi', 'utilswpi.', 'FO'
   utilsmajor = '1'
   utilsminor = '1'
   do u = 1 to utilswpi.0
      parse var utilswpi.u '-' utilsfilemajor '-' utilsfileminor '.'
      utilsfilemajor = utilsfilemajor + 0
      utilsfileminor = utilsfileminor + 0
      if (utilsfilemajor > utilsmajor | ,
          (utilsfilemajor = utilsmajor & ,
           utilsfileminor > utilsminor)) then
         do
            utilsmajor = utilsfilemajor
            utilsminor = utilsfileminor
         end
   end
   utilsver = utilsmajor || '-' || utilsminor
   if utilsver \= '1-1' then
   else
   if stream( 'warpin\fm2utils-' || utilsver || '.zip', 'c', 'query exists') = '' then
      do
         say 'Zipping up (new) FM2Utils...'
         'zip -9jX' release_zipfile  'warpin\fm2utils-' || utilsver || '.wpi'
      end
*/
return

StripNotNeeded: procedure
   parse arg instring
   if instring = '[NotNeeded]' then
      return ''
   else
      return instring

TracUpdates: procedure expose (globals)
   nextver.1 = ver.major || '.' || ver.minor + 1 || '.0'
   nextver.2 = ver.major || '.' || ver.minor || '.' || ver.CSDlevel + 1
   nextver.3 = ver.major + 1 || '.0.0'
   nextver.0 = 3
   do until (choice2 = 1 | choice2 = 2 | choice2 = 3)
      say
      say 'Set next version and update TRAC'
      say
      say 'The just-released version of FM/2 is:' ver.full
      say
      say 'Please select the next version:'
      say
      do i = 1 to nextver.0
         say '  ' || i || ')' nextver.i
      end
      say
      call charout , 'Enter the number of your choice: '
      choice2 = SysGetKey()
      say
      say
   end
   say;say
   say '   Create a TRAC version for FM/2' nextver.choice2
   say
   say '   Mark the next version as the default version for new tickets.'
   say
   say '   Create a TRAC milestone for FM/2' nextver.choice2 || ', if needed.'
   say
   say '   Mark the new milestone as the default milestone.'
   say
   say '   Mark the completed milestone as complete and'
   say '   move any residual tickets to a future milestone.'
   say '   (TRAC can move the tickets in bulk when you mark'
   say '   the version milestone complete.)'
   say
/* JBS: Code disabled until its need/usefulness is determined. */
/*
   call charout , 'Press any key when ready to proceed: '
   call SysGetKey
   say
*/
   /* Set next version */
/* JBS: Code disabled until its need/usefulness is determined. */
/*
   do f = 1 to words(ver.filelist)
      call ReleaseEdit nextver.choice2 word(ver.filelist, f) 'trace=' || traceopt
      say
   end
   call CommitifOK ver.filelist
*/
return

NewScreenWithHeading: procedure expose (globals)
   parse arg tasknum
   call SysCls
   say
   say 'Task:' mainmenu.tasknum.text
return

CommitIfOK: procedure
   parse arg filelist
   svn_cmd = 'svn commit'
   say;say;say
   say 'Online and OK to execute:' svn_cmd filelist || '? (Y/n)'
   if translate(SysGetKey()) \= 'N' then call ExecCmd svn_cmd filelist
return

/*** Usage/Error routines ***/

Usage: procedure
   say;say;say
   i = 1
   parse value SysTextScreenSize() with rows .
   linesperscreen = rows - 2
   do forever
      srcline = sourceline(i)
      if pos('CHANGE LOG', translate(srcline)) > 0 then
         leave
      else
         say srcline
      i = i + 1
      if (i // linesperscreen) = 0 then
         do
            say
            call charout , 'F)orward B)ack Q)uit: '
            reply = translate(SysGetKey())
            say
            select
               when reply = 'B' then
                  i = i - (2 * linesperscreen)
               when reply = 'Q' then
                  leave
               otherwise
                  nop
            end
         end
   end
exit

Novalue:
  say 'Uninitialized value found on line:' sigl
  say 'Variable:' condition('D')
  say 'Text:'
  do i = sigl-3 to sigl+3
     if i = sigl then
        call charout , '--> '
     else
        call charout , '    '
     say sourceline(i)
  end
exit

/*=== Error() Report ERROR, FAILURE etc. and exit ===*/
Error:
  say;say;say
  parse source . . cmdfile
  say 'CONDITION'('C') 'signaled at' cmdfile 'line' SIGL'.'
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

WikiUpdates: procedure expose (globals)
   call NotYet arg(1)
   say '1) Add the next version to: http://svn.netlabs.org/fm2/admin/ticket/versions'
   say '2) Make this version the default'
   say '3) Add the next milestone'
   say '4) Make the next milestone the default'
   say '5) Close the old milestone and move residual tickets to the new one.'
   say '6) Update "WikiStart" with the newly released version:' ver.full
   say '7) Update "RBuild" with the new tag: FM2-' || ver.tag
return

/* JBS: Disabled internal CFG code:
Configuration: procedure expose (globals)
   do until option = 'S' | option = 'Q'
      call SysCls
      say
      say 'ReleaseTool configuration'
      say
      say 'If this is the first time through this configuration, please maake sure all'
      say 'of the following are properly configured for you. Defaults will not work'
      say 'for everyone.'
      say
      say 'With proper configuration, the following activities can be automated:'
      say '   Uploads of files (to Netlabs, Hobbes, etc.).'
      say '   Requesting that Netlabs move the uploaded file.'
      say '   Email notifications of the release (to lists and/or individuals).'
      say '   Newsgroup notifications of the release (to various newsgroups).'
      say
      say "Enter the first letter of the activity to configure or..."
      say "'S' to save or 'Q' to quit. ('S' or 'Q' will return you to the main menv)."
      call charout , "Enter 'U', 'R', 'E', 'N', 'S' or 'Q': "
      option = translate(SysGetKey())
      say
      select
         when option = 'S' then
            do
               /* save configuration */
               call SaveConfiguration
            end
         when option = 'Q' then
            do
               /* restore initially read configuration */
               /*
                * if missing or invalid config data, get confirmation
                * before restoring file data.
                */
            end
         when option = 'U' then
            do
               /* configure uploads                    */
               call ConfigureUploads
            end
         when option = 'R' then
            do
               /* configure Netlabs request            */
               call ConfigureNetlabsRequest
            end
         when option = 'E' then
            do
               /* configure (other) email notifications*/
               call ConfigureEmailNotifications
            end
         when option = 'N' then
            do
               /* configure newsgroup  notifications   */
               call ConfigureNewsgroupNotifications
            end
         otherwise
            nop
      end
   end
return

SaveConfiguration: procedure expose (globals)
return

ConfigureUploads: procedure expose (globals)
   do until option = 'Q'
      call SysCls
      say
      say 'Configure uploads'
      say
      say 'The following uploads are currently configured:'
      do i = 1 to cfg.FTP.0
         say '   ' || i || ')' cfg.FTP.i.descriptive_hostname
      end
      say
      say 'Enter the number of the upload to configure, or ...'
      say "'A' to add a new upload definition, or ..."
      say "'D' to delete one of these definitions, or ..."
      call charout , "'Q' to return to the previous menu: "
      option = translate(SysGetKey())
      say
      select
         when option = 'Q' then
            nop
         when option = 'A' then
            call AddUpload
         when option = 'D' then
            do
               say
               say 'Enter the number of the upload definition to delete.'
               call charout , '(An invalid entry will cancel the deletion): '
               i = SysGetKey()
               say
               if CheckNum(i, cfg.FTP.0) then
                  do
                     call charout , "Confirm deletion of upload #" || i '(y/N): '
                     if translate(SysGetKey()) = 'Y' then
                        do
                        /* Delete an upload definition here */
                        end
                  end
            end
         when CheckNum(option, cfg.FTP.0) then
            do
               /* Edit an upload here */
               call EditUpload
            end
         otherwise
            nop
      end
   end
return

CheckNum: procedure
   parse arg num, maxval
   retval = 0
   if datatype(num, 'NUM') then
      if num <= maxval then
         if num > 0 then
            if trunc(num) = num then
               retval = 1
return retval

AddUpload: procedure expose (globals)
return

EditUpload: procedure expose (globals)
return

ConfigureNetlabsRequest: procedure expose (globals)
return

ConfigureEmailNotifications: procedure expose (globals)
return

ConfigureNewsgroupNotifications: procedure expose (globals)
return
*/

