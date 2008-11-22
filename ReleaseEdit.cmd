/*
 * $Id$
 *
 * ReleaseEdit: a program which will set the version number
 *    and the date and time of a FM/2 release
 *
 * Usage:
 *    ReleaseEdit <version> <relative-filename>
 *
 *    where <version> is x.y.z
 *
 *    and <relative-filename> is ONE of the files (i.e. no wildcards)
 *    which need a version number and/or date/time:
 *       *.def, file_id.diz, dll\version.h, warpin\fm2.wis
 *
 * Change log:
 *    18 Nov 08 JBS Ticket 297: Various build improvements/corrections
 *       -  Use SysTempFilename instead of hard-coded temp file name
 *       -  Removed support for changing version in FM2.WIS (not needed)
 *    21 Nov 08 JBS Ticket 297: Various build improvements/corrections
 *       -  Changed code for option description lines in DEF files.
 *          Instead of reading the current lines, they are recontructed
 *          scratch. The data for these lines is:
 *          - provided as a parameter (version/revision)
 *          - read from a repository file, option_descriptions.txt
 *            - all fields read from have default values
 *            - support for environment variables
 *          - edits length of all fields in option description
 *       - added support for changing copyright years
 *       - improved error handling: tmp file renamed back to deleted file
 *       - improved "usage" routine
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

/*
   sed_separator:
     - used only for SED edits of DEF file descriptions
     - MUST be a character not found in ANY DEF file description!
     - MUST be a character acceptable to SED as a separator
*/
sed_separator = '&'

/* Change to proper directory */
parse source . . thispgm
thisdir = left(thispgm, lastpos('\', thispgm)) || '.'
call directory thisdir

/* Process parameters */
parse arg ver file .
if ver = '' | file = '' then
   call Usage           /* and exit */
else if stream(file, 'c', 'query exists') = '' then
   call Usage           /* and exit */
/* Set extension, if none set it to uppercase(file) */
ext = substr(translate(file), lastpos('.', file) +1)
parse var ver major '.' minor '.' CSDLevel
if CSDlevel = '' then
   CSDlevel = 0

/* Prepare temporary file */
call RxFuncAdd 'SysTempFilename', 'REXXUTIL', 'SysTempFilename'
tmpfile = SysTempFilename('redittmp.???')
'copy' file tmpfile
if rc \= 0 then
   do
      say;say 'Unable to copy to ' || tmpfile || '!! Proceesing aborted.'
      exit 1
   end
if wordpos(ext, 'H') = 0 then
   'del' file

/* Set fixed strings */
globals = 'repository copyright_year_marker copyright_year_marker_len'
repository = 'option_descriptions.txt'
mkstr_makefile  = 'DLL\INTERNAL\MAKEFILE'
warpin_makefile = 'WARPIN\MAKEFILE'
copyright_h     = 'DLL\COPYRIGHT.H'
parse value date('s') with year 5 month 7 day
last_year = year - 1

/* Process the request */
select
   when ext = 'DEF' then
      do
         if stream(repository, 'c', 'query exists') = '' then
            do
               say 'Error! Unable to locate repository file:' repository
               say
               say 'Unable to proceed with request.'
               say
               '@if not exist' file 'copy' tmpfile file '>nul 2>nul'
               '@pause'
               signal exit_routine
            end

         copyright_year_marker = 'copyright-year'
         copyright_year_marker_len = length(copyright_year_marker)

         vendor         = GetFromRepository( 'vendor', 'The Netlabs FM/2 team', 31 )
         revision       = ver
         buildhost      = GetFromRepository( 'buildhost', 'GKYBuild', 11 )
         asd_feature_id = GetFromRepository( 'asd_feature_id', '', 11 )
         language_code  = GetFromRepository( 'language_code', 'EN', 4 )
         country_code   = GetFromRepository( 'country_code', 'US', 4 )
         build          = GetFromRepository( 'build', '0', 7 )
         processor_type = GetFromRepository( 'processor_type', 'U', 1 )
         fixpack_ver    = GetFromRepository( 'fixpack_ver', '', 11 )
         description    = GetFromRepository( 'desc.' || left(file, pos('.', file) - 1), '', 579 /* i.e. disable length check */ )
         call stream repository, 'c', 'close'

         option_description = '@#' || vendor || ':' || revision || '#@##1## ' || ,
                              month || '/' || day || '/' || year || ' ' || ,  /* or day month year? */
                              right(major, 2, '0') || ':' || right(minor, 2, '0') || ':' || right(CSDlevel, 2, '0') || ,
                              copies(' ', 6) || buildhost || ':' || asd_feature_id || ':' || ,
                              language_code || ':' || country_code || ':' || build || ':' || ,
                              processor_type || ':' || fixpack_ver || '@@' || description
         do forever
            p = pos(copyright_year_marker, option_description)
            if p = 0 then
               leave
            else
               if p = 1 then
                  option_description = year || substr(option_description, p + copyright_year_marker_len)
               else
                  if p + copyright_year_marker_len >= length(option_description) then
                     option_description = left(option_description, p - 1) || year
                  else
                     option_description = left(option_description, p - 1) || year || substr(option_description, p + copyright_year_marker_len)
         end
         option_description = "option description '" || option_description || "'"
         'sed -r -e "/option description/s' || sed_separator || '.*' || sed_separator || option_description || sed_separator || '" ' || tmpfile || ' >' file
      end
   when ext = 'H' then
      do
         if translate(file) = copyright_h then
            'grep -E "^#define.*COPYRIGHT_YEAR.*' || year || '" ' || file || ' >nul || del ' || file || ' && sed -r -e "/#define.*COPYRIGHT_YEAR/s/[0-9]+/' || year || '/" ' || tmpfile || ' >' file
         else
            do /* change below to delete and then update version.h only if version is different? */
               'del' file
               'sed -r -e "/#define[ \t]+VERMAJOR/s/(#define[ \t]+VERMAJOR[ \t]+)[^ \t]+/\1' || major || '/g" -e "/#define[ \t]+VERMINOR/s/(#define[ \t]+VERMINOR[ \t]+)[^ \t]+/\1' || minor || '/g" ' || tmpfile || ' >' file
            end
      end
   when ext = 'DIZ' then
      do
         'sed -r "/FM\/2 v/s/(FM\/2 v)[0-9]+\.[0-9.]+/\1' || ver || '/" ' || tmpfile || ' >' file
      end
   when ext = mkstr_makefile then
      do
         'sed -r -e "/desc/s/(SLAInc:).*(\$#@\$#\$#1\$#\$# )[0-9/]+ [0-9:]+/\1' || ver || '\2' || month || '\/' || day || '\/' || year right(major, 2, '0') || ':' || right(minor, 2, '0') || ':' || right(CSDlevel, 2, '0') || '/" ' || tmpfile || ' >' file
      end
   when ext = warpin_makefile then
      do
         warpin_db_ver = (major + 0) || '\\' || (minor + 0) || '\\' || (CSDlevel + 0)
         warpin_makefile_ver  = major || '-' || minor || '-' || CSDlevel
         'sed -r -e "/FM2_VER=-/s/(FM2_VER=-)[-0-9]+/\1' || warpin_makefile_ver || '/" ' || tmpfile || ' >' file
      end
   otherwise
      nop         /* Or error message or usage info? */
end
exit_routine:
'@if exist' tmpfile 'del' tmpfile
n = endlocal()
return

/* Subroutines */
GetFromRepository: procedure expose (globals)
   parse arg value_name, value_default, max_value_len
   /* Replace this with code for each DEF file */
   call SysFileSearch value_name || '=', repository, 'lines.'
   if lines.0 = 0 then
      if left(value_name, 5) = 'desc.' then
         call MissingDescriptionInRepository value_name
      else
         value_value = value_default
   else
      do
         n = lines.0
         parse var lines.n . '=' value_value
      end
   value_value = strip(value_value)
   if length(value_value) > 1 then
      if left(value_value, 1) = '%' & right(value_value, 1) = '%' then
         do
            value_value = value(substr(value_value, 2, length(value_value) - 2),, 'OS2ENVIRONMENT')
            if value_value = '' then
               value_value = value_default
         end
   if pos(copyright_year_marker, value_value) > 0 then
      value_len = length(value_value) - copyright_year_marker_len + 4
   else
      value_len = length(value_value)
   if value_len > max_value_len then
      do
         say
         say 'Error in length of data field!'
         say '  Field name:' value_name
         say '  Length    :' value_len
         say '  Max. len. :' max_value_len
         say '  Field     :' value_value
         say '  Trunc''d  :' left(value_value, max_value_len)
         say
         say 'This should be corrected in the repository:' repository
         say
         '@pause'
      end
return value_value

Usage:
   say;say;say
   i = 1
   do forever
      srcline = sourceline(i)
      if pos('CHANGE LOG', translate(srcline)) > 0 then
         exit
      say srcline
      i = i + 1
   end

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
    'if exist' tmpfile 'move' tmpfile file
    call 'SYSSLEEP' 2
    exit 'CONDITION'('C')
  end

  return

novalue:
   say 'Uninitialized variable: ' || condition('D') || ' on line: 'sigl
   say 'Line text: 'sourceline(sigl)
   cfg.errorcode = 3
   signal ErrorExit

/*
 * bldlevel string docs:
    Format of BLDLEVEL string (Type III)

@#<Vendor>:<Revision>#@##1## DD.MM.YY hh:mm:ss      <BuildHost>:<ASDFeatureID>:<LanguageCode>:<CountryCode>:<Build>:<Unknown>:<FixPackVer>@@<Description>

where

    * DD.MM.YY is the build date in day/month/year, preceded by 1 space
    * hh:mm:ss is the build time in hour/minute/second, preceded by 1 space
    * <BuildHost> is machine on which build compiled, preceded by 8 spaces
    * <ASDFeatureID> is identifier of ASD feature
    * <LanguageCode> is code of language of component
    * <CountryCode> is country code of component
    * <Build> is build number
    * <Unknown> is not known information (must be empty)
    * <FixPackVer> is FixPack version (if distibuted as part of).

Note: If you leave build date and/or build time empty you still have to provide the same amount of spaces to replace build date/build time.


*/


