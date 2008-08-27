/*
 * $Id: $
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
*/
parse arg ver file .
if ver = '' | stream(file, 'c', 'query exists') = '' then
   signal Usage			/* and exit */
parse var ver major '.' minor '.' CSDLevel
if CSDlevel = '' then
   CSDlevel = 0
mkstr_makefile  = 'DLL\INTERNAL\MAKEFILE'
warpin_makefile = 'WARPIN\MAKEFILE'
warpin_db_ver = (major + 0) || '\\' || (minor + 0) || '\\' || (CSDlevel + 0)
warpin_makefile_ver  = major || '-' || minor || '-' || CSDlevel
ext = substr(translate(file), lastpos('.', file) +1)   /* if no extension, ext <-- uppercase(file) */
parse value date('s') with year 5 month 7 day
tmpfile = 'redit.bak'   /* If SysTempFilename is used, SysLoadFuncs first */
'@if exist' tmpfile 'del' tmpfile
'copy' file tmpfile
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
   when ext = 'WIS' then
      do
         'sed -r -e "s/(PACKAGEID=.*Base\\)[0-9\]+/\1' || warpin_db_ver || '/" -e "s/(PACKAGEID=.*Debugging support\\)[0-9\]+/\1' || warpin_db_ver || '/" ' || tmpfile || ' >' file
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
	say;say;say
	lastline = sigl - 3
	do i = 1 to lastline
		say sourceline(i)
	end
exit

