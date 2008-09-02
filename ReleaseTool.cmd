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
 *    ReleaseTool [<this-version-number>] [<next-version-number>]
 *
 *    where the version number(s) are optional and of the form x.y.z
 *
 * This program will reformat the version numbers to suit their use:
 *    tag names
 *    wpi filenames
 *    warpin packageid/database version number(s)
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


globals = 'cmd prompt editor editorcmds'

parse arg ver next_ver
if (pos('?', ver) > 0 | pos('h', ver) > 0) then
   signal Usage		/* and exit */

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
               '@'cmd '/c 'action
               action = -1					/* Skip SELECT below */
            end
      end
   select
      when action = 0 then
         do /* Open a command line */
            if right(translate(cmd), 8) = '4OS2.EXE' then
               '@' || cmd 'prompt [''exit'' to return to ReleaseTool]' || prompt
            else
               do
                  '@set prompt=[''exit'' to return to ReleaseTool]' || prompt
                  '@' || cmd
               end
         end
      when action = 1 then
         do /* Ensure all work (by others) is comitted */
            call NotYet action
            say 'Notify programmers to commit their work for this release.'
            say
            say 'Use the Netlabs FM/2 Developer''s mailing list and wait 24 hours.'
         end
      when action = 2 then
         do /* Verify completed tickets are marked closed */
            call NotYet action
            say 'Check for committed work.'
         end
      when action = 3 then
         do /* Ensure all work (by others) is local */
            svn_cmd = 'svn update'
            say;say;say
            say 'Online and OK to execute: 'svn_cmd'? (Y/n)'
            if translate(SysGetKey()) \= 'N' then svn_cmd
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
         end
      when action = 5 then
         do /* Edit version #'s and date/time stamps (ReleaseTool) */
            call SysCls
            if strip(ver) = '' then
               ver = GetVer('the pending release')
            filelist = 'av2.def databar.def dirsize.def dll\fm3dll.def dll\fm3res.def eas.def'
            filelist = filelist 'fm3.def fm4.def global.def ini.def killproc.def sysinfo.def'
            filelist = filelist 'undel.def vcollect.def vdir.def viewinfs.def vtree.def'
            filelist = filelist 'warpin\fm2.wis file_id.diz dll\version.h warpin\makefile dll\internal\makefile'
            do f = 1 to words(filelist)
               call ReleaseEdit ver word(filelist, f)
               say
            end
            filelist = 'HISTORY README'
            do f = 1 to words(filelist)
               file = word(filelist, f)
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
         end
      when action = 6 then
         do /* Ensure the edits build */
            'wmake -a all'
         end
      when action = 7 then
         do /* Test built code */
            call NotYet action
            say 'Test the built code.'
         end
      when action = 8 then
         do /* Commit code */
            svn_cmd = 'svn commit'
            say;say;say
            say 'Online and OK to execute: 'svn_cmd'? (Y/n)'
            if translate(SysGetKey()) \= 'N' then svn_cmd
         end
      when action = 9 then
         do /* Apply tag */
            call SysCls
            if strip(ver) = '' then
               ver = GetVer('the pending release')
            svn_cmd = 'svn copy -m"Tag release FM2-' || Tag_ver(ver) || '" http://svn.netlabs.org/repos/fm2/trunk http://svn.netlabs.org/repos/fm2/tags/FM2-' || tag_ver
            say;say;say
            say 'Online and OK to execute: 'svn_cmd'? (y/N)'
            if translate(SysGetKey()) = 'Y' then svn_cmd
         end
      when action = 10 then
         do /* Build for the release */
            'set FORTIFY=& set DEBUG=& wmake -a all'
         end
      when action = 11 then
         do /* Test the binaries */
            call NotYet action
            say 'Test the binaries.'
            say
            say 'At a minimum you should run all the exes and do some'
            say 'basic file manipulation with each.'
            say
            say 'You should, where possible, also verify that any bugs'
            say 'that were fixed for the release are working as expected.'
         end
      when action = 12 then
         do /* Lxlite */
            'wmake lxlite'
         end
      when action = 13 then
         do /* Test the release code */
            call NotYet action
            say 'Test the (compressed) release code.'
            say
            say 'Verify that all exe''s continue to load and run after being compressed.'
         end
      when action = 14 then
         do /* Build distro */
            call SysCls
            if strip(ver) = '' then
               ver = GetVer('the pending release')
            'wmake dist FM2_VER=-' || WPI_ver(ver)
         end
      when action = 15 then
         do /* Zip distro */
            call SysCls
            if strip(ver) = '' then
               ver = GetVer('the pending release')
            zip_ver = translate(ver, '-', '.')
            'zip -j9 fm2-' || zip_ver || ' warpin\fm2-' || zip_ver || '.wpi'
         end
      when action = 16 then
         do /* Zip FM/2 Utilities' */
            call NotYet action
            say 'If FM/2 Utilities have been updated, then zip them up.'
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
         end
      when action = 20 then
         do /* Set next version */
            if strip(next_ver) = '' then
               next_ver = GetVer('the next release')
            filelist = 'av2.def databar.def dirsize.def dll\fm3dll.def dll\fm3res.def eas.def fm3.def fm4.def global.def ini.def killproc.def sysinfo.def undel.def vcollect.def vdir.def viewinfs.def vtree.def warpin\fm2.wis file_id.diz dll\version.h'
            do f = 1 to words(filelist)
               call ReleaseEdit next_ver word(filelist, f)
               say
               '@pause'
            end
         end
/*
      when action = 21 then
         do
         end
*/
   	otherwise
   		nop
   end
  	say;say;say
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
	lastline = sigl - 10
	do i = 1 to lastline
		say sourceline(i)
	end
exit

/*** Subroutines ***/
Init: procedure expose (globals)
   call RxFuncAdd 'SysLoadFuncs', 'REXXUTIL', 'SysLoadFuncs'
   call SysLoadFuncs

	action = 0

	editor = value('SVN_EDITOR',,'OS2ENVIRONMENT')
	if editor == '' then
		editor = 'tedit'
   upperwrd1 = translate(word(editor, 1))
   if upperwrd1 = 'EPM' | upperwrd1 = 'EPM.EXE' then
      editorcmds = "'3'"
   else
      editorcmds = ""
		
   cmd 		= value('COMSPEC',,'OS2ENVIRONMENT')
   prompt 	= value('PROMPT',,'OS2ENVIRONMENT')
return

DisplayMenu: procedure
	do forever
   	call SysCls
   	say;say;say
   	say 'Release Tasks'
   	say
   	say '1.  Ensure all work for this release is committed.'
   	say '2.  Verify completed tickets are marked closed.'
   	say '3.  (svn) update local files.'
   	say '4.  Check (svn) status of local files.'
   	say '5.  Edit various files with version #''s and date''s.'
      say '6.  Ensure the edits build.'
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
      say '20. Set next version.'
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
	say
	say 'Please enter the version (x.yy.zz) for' ver_text ':'
   parse value linein() with major '.' minor '.' CSDlevel
   if minor    = '' then
      minor    = 0
   if CSDlevel = '' then
      CSDlevel = 0
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
	default_name 				= 'Gregg Young'
	default_email 				= 'ygk@qwest.net'
	default_OKtoListEmail 	= 'yes'
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
   	say;say 'Hex(key):' c2x(entry)
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


