/*
 * $Id$
 *
 * ReleaseEdit: a program which will set the version number
 * 	and the date and time of a FM/2 release
 *
 * Usage:
 * 	ReleaseEdit <version> <filename>
 *
 * 	where <version> is x.y.z
 *
 * 	and <filename> is ONE of the files (i.e. no wildcards)
 * 	which need a version number and/or date/time:
 * 	   *.def, file_id.diz, dll\version.h, warpin\fm2.wis
 *
 * Change log:
 * 	18 Nov 08 JBS Ticket 297: Various build improvements/corrections
 *    	- Use SysTempFilename instead of hard-coded temp file name
 *       - Commented out support for changing version in FM2.WIS
 *
*/

signal on Error
signal on FAILURE name Error
signal on Halt
signal on NOTREADY name Error
signal on NOVALUE name Error
signal on SYNTAX name Error
/*
signal on novalue             /* for debugging */
*/

parse arg ver file .
if ver = '' | file = '' then
   call Usage 0			/* and exit */
else if stream(file, 'c', 'query exists') = '' then
   call Usage 2			/* and exit */
parse var ver major '.' minor '.' CSDLevel
if CSDlevel = '' then
   CSDlevel = 0
mkstr_makefile  = 'DLL\INTERNAL\MAKEFILE'
warpin_makefile = 'WARPIN\MAKEFILE'
warpin_db_ver = (major + 0) || '\\' || (minor + 0) || '\\' || (CSDlevel + 0)
warpin_makefile_ver  = major || '-' || minor || '-' || CSDlevel
ext = substr(translate(file), lastpos('.', file) +1)   /* if no extension, ext <-- uppercase(file) */
parse value date('s') with year 5 month 7 day
call RxFuncAdd 'SysTempFilename', 'REXXUTIL', 'SysTempFilename'
tmpfile = SysTempFilename('redittmp.???')
'copy' file tmpfile
if rc \= 0 then
   do
      say;say 'Unable to copy to ' || tmpfile || '!! Proceesing aborted.'
      exit 1
   end
'del' file
select
   when ext = 'DEF' then
      do
         'sed -r -e "/Copyright/s/(Copyright.*20[0-9][0-9], )[0-9]*/\1' || year || '/g" -e "/desc/s/(SLAInc:).*(#@##1## )[0-9/]+ [0-9:]+/\1' || ver || '\2' || month || '\/' || day || '\/' || year right(major, 2, '0') || ':' || right(minor, 2, '0') || ':' || right(CSDlevel, 2, '0') || '/" ' || tmpfile || ' >' file
      end
   when ext = 'H' then
      do
         'sed -r -e "/#define[ \t]+VERMAJOR/s/(#define[ \t]+VERMAJOR[ \t]+)[^ \t]+/\1' || major || '/g" -e "/#define[ \t]+VERMINOR/s/(#define[ \t]+VERMINOR[ \t]+)[^ \t]+/\1' || minor || '/g" ' || tmpfile || ' >' file
      end
   when ext = 'DIZ' then
      do
         'sed -r "/FM\/2 v/s/(FM\/2 v)[0-9]+\.[0-9.]+/\1' || ver || '/" ' || tmpfile || ' >' file
      end
   when right(ext, length(mkstr_makefile)) = mkstr_makefile then
      do
         'sed -r -e "/desc/s/(SLAInc:).*(\$#@\$#\$#1\$#\$# )[0-9/]+ [0-9:]+/\1' || ver || '\2' || month || '\/' || day || '\/' || year right(major, 2, '0') || ':' || right(minor, 2, '0') || ':' || right(CSDlevel, 2, '0') || '/" ' || tmpfile || ' >' file
      end
   when right(ext, length(warpin_makefile)) = warpin_makefile then
      do
         'sed -r -e "/FM2_VER=-/s/(FM2_VER=-)[-0-9]+/\1' || warpin_makefile_ver || '/" ' || tmpfile || ' >' file
      end
   otherwise
      nop			/* Or error message or usage info? */
end
'@if exist' tmpfile 'del' tmpfile
return

Usage:
   parse arg plus
	say;say;say
	lastline = sigl - (14 + plus)
	do i = 1 to lastline
		say sourceline(i)
	end
exit

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


