/*
 * $Id$
 *
 * ReleaseEdit: a program which will edit a file and set the version
 *    number and the date of an FM/2 release as appropriate for that file.
 *    This program operates non-interactively.
 *
 * Usage:
 *    ReleaseEdit <version> <relative-filename> [trace=<trace-option>]
 *
 *    where
 *       <version> is in the form x.y.z or x.y (where x, y and z are numbers)
 *       <relative-filename> is ONE of the files (i.e. no wildcards)
 *          which need a version number and/or date set. For example:
 *             *.def, file_id.diz, dll\version.h, warpin\fm2.wis
 *       <trace-option> is a valid comnbination of trace options:
 *          '? a c e f i l n o r'. This is an OPTIONAL parameter.
 *
 * Examples:
 *    ReleaseEdit 3.15.0 av2.def
 *    ReleaseEdit 3.15.0 fm3.def trace=i
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
 *    22 Nov 08 JBS Ticket 297
 *       - Fix bugs in version edits and added
 *       - Support for an optional trace parameter.
 *       - Improved "usage" routine
 *       - Suppressed output of 'del' commands
 *    23 Nov 08 JBS Improved handling of invalid or missing <trace-option>
 *       and a 'NOTREADY' condition when closing the repository.
 *
*/

n = setlocal()

/*
   sed_separator:
     - used only for SED edits of DEF file descriptions
     - MUST be a character not found in ANY DEF file description!
     - MUST be a character acceptable to SED as a separator
*/
sed_separator = '&'

call RxFuncAdd 'SysLoadFuncs', 'REXXUTIL', 'SysLoadFuncs'
call SysLoadFuncs

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

signal on Error
signal on FAILURE name Error
signal on Halt
signal on NOTREADY name Error
signal on NOVALUE name Error
signal on SYNTAX name Error
/*
signal on novalue             /* for debugging */
*/

/* Change to proper directory */
parse source . . thispgm
thisdir = left(thispgm, lastpos('\', thispgm)) || '.'
call directory thisdir

/* Process parameters */
parse var args ver file .
if ver = '' | file = '' then
   call Usage           /* and exit */
else if stream(file, 'c', 'query exists') = '' then
   call Usage           /* and exit */
parse var ver major '.' minor '.' CSDLevel
if CSDlevel = '' then
   CSDlevel = 0

if datatype(major) \= 'NUM' | datatype(minor) \= 'NUM' | datatype(CSDlevel) \= 'NUM' then
   call Usage
ver = major || '.' || minor || '.' || CSDlevel

/* Set extension, if none set it to uppercase(file) */
ext = substr(translate(file), lastpos('.', file) +1)

/* Prepare temporary file */
tmpfile = SysTempFilename('redittmp.???')
'@copy' file tmpfile '1>nul 2>nul'
if rc \= 0 then
   do
      say;say 'Unable to copy to ' || tmpfile || '!! Proceesing aborted.'
      exit 1
   end
if wordpos(ext, 'H') = 0 then
   '@del' file '1>nul 2>nul'

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
         say 'Processing file:' file
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
         signal off NOTREADY
         call stream repository, 'c', 'close'
         signal on NOTREADY name Error

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
            do
               say 'Processing file:' file
               'grep -E "^#define.*COPYRIGHT_YEAR.*' || year || '" ' || file || ' >nul || del ' || file || '>nul 2>nul && sed -r -e "/#define.*COPYRIGHT_YEAR/s/[0-9]+/' || year || '/" ' || tmpfile || ' >' file
            end
         else
            do /* change below to delete and then update version.h only if version is different? */
               say 'Processing file:' file
               '@del' file '1>nul 2>nul'
               'sed -r -e "/#define[ \t]+VERMAJOR/s/(#define[ \t]+VERMAJOR[ \t]+)[^ \t]+/\1' || major || '/g" -e "/#define[ \t]+VERMINOR/s/(#define[ \t]+VERMINOR[ \t]+)[^ \t]+/\1' || minor || '/g" ' || tmpfile || ' >' file
            end
      end
   when ext = 'DIZ' then
      do
         say 'Processing file:' file
         'sed -r "/FM\/2 v/s/(FM\/2 v)[0-9]+\.[0-9.]+/\1' || ver || '/" ' || tmpfile || ' >' file
      end
   when ext = mkstr_makefile then
      do
         say 'Processing file:' file
         'sed -r -e "/desc/s/(SLAInc:).*(\$#@\$#\$#1\$#\$# )[0-9/]+ [0-9:]+/\1' || ver || '\2' || month || '\/' || day || '\/' || year right(major, 2, '0') || ':' || right(minor, 2, '0') || ':' || right(CSDlevel, 2, '0') || '/" ' || tmpfile || ' >' file
      end
   when ext = warpin_makefile then
      do
         say 'Processing file:' file
         warpin_db_ver = (major + 0) || '\\' || (minor + 0) || '\\' || (CSDlevel + 0)
         warpin_makefile_ver  = major || '-' || minor || '-' || CSDlevel
         'sed -r -e "/FM2_VER=-/s/(FM2_VER=-)[-0-9]+/\1' || warpin_makefile_ver || '/" ' || tmpfile || ' >' file
      end
   otherwise
      nop         /* Or error message or usage info? */
end
exit_routine:
'@if exist' tmpfile 'del' tmpfile '>nul 2>nul'
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
      if (i // 22) = 0 then
         '@pause'
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


