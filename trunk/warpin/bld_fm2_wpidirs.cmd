/*
 * $Id$
 *
 * REXX code to create and populate directories in preparation for the creation of the
 * FM/2 Warpin archive (WPI file).
 *
 * TODO:
 *    -  Support multiple levels of destination directories
 *    -  Add logic to read script to determine
 *       -  Number of packages
 *       -  Package numbers
 *    -  Document errors internally (Error. ?)
 *    -  Display more descriptive error messages to user
 *
 */

call RxFuncAdd 'SysLoadFuncs', 'REXXUTIL', 'SysLoadFuncs'
call SysLoadFuncs

signal on novalue     /* for debugging */

n = setlocal()

parse source . . thispgm
thisdir = left(thispgm, lastpos('\', thispgm) - 1)
if length(thisdir) = 2 then
   thisdir = thisdir || '\'
call directory thisdir

globals = 'WPI.'

WPI. = ''
WPI.retval = 0
WPI.max_package_number_length = 3

parse arg args
call ProcessArgs strip(args)

call ReadFile

do f = 1 to WPI.fileline.0
   parse var WPI.fileline.f . filename pkgnum src_subdir dest_subdir
   if WPI.src_basedir \= '..\' then  /* Ignore src_subdir from file if not using build subtree */
      src_subdir = '.'
   dest_basedir   = MakePackageDir( pkgnum )
   dest_dir = dest_basedir || '\' || strip(dest_subdir)
   call SysMkDir dest_dir
   'copy ' || WPI.src_basedir || src_subdir || '\' || filename || ' ' || dest_dir
end

if WPI.retval = 0 then
   signal NormalExit

ErrorExit:
   parse arg errorcode
   select
      when errorcode = 3 then
         do
            say 'Error: Too many tokens on a line in 'WPI.infile
            say 'Line : 'txtline
         end
      when errorcode = 4 then
         say 'Error: Keyword is not FILE or NOFILESPACKAGE in 'WPI.infile
      when errorcode = 5 then
novalue:
         do
            say 'Error: Unitialized value: "' || condition('D') || ' encountered on line: 'sigl
            say '   'sourceline(sigl)
            signal NormalExit
         end
      otherwise
         say 'Error: unknown error code: 'errorcode
   end
   say
   say 'Exiting...'
   say

NormalExit:
   n = endlocal()
   exit WPI.retval

ReadFile: procedure expose (globals)
   f = 0
   do while lines(WPI.infile) > 0
      txtline = strip(linein(WPI.infile))
      if txtline == '' then
         iterate
      p = pos(';', txtline)
      if p = 1 then
         iterate
      if p > 1 then
         do
            parse var txtline txtline ';' .     /* strip off comments */
            txtline = strip(txtline)
         end
      parse value translate(txtline) with word1 word2 word3 .
      if (word1 == 'FILE:') then
         do
            if words(txtline) \= 5 then
               call ErrorExit 3  /* Invalid number of parameters */
            else
               do
                  f = f + 1
                  WPI.fileline.f = strip(txtline)
/*
                  if word3 > WPI.max_package_number then
                      WPI.max_package_number = word3
*/
               end
         end
      else
         if word1 == 'NOFILESPACKAGE:' then
            call MakePackageDir word2
         else
            call ErrorExit 4        /* Invalid line/word1 */
   end
   WPI.fileline.0 = f
   call stream WPI.infile, 'c', 'close'
return

ProcessArgs: procedure expose (globals)
   parse upper arg args
   p = pos('SRC=', args)
   if p > 0 then
      do
         if substr(args, p + 4, 1) == '"' then
            parse var args part1 'SRC="' WPI.src_basedir '"' part2
         else
            parse var args part1 'SRC=' WPI.src_basedir part2
         args = strip(part1 part2)
      end
   else
      do
         env_var = value('BLD_FM2_WPIDIRS_SRC',, 'OS2ENVIRONMENT')
         if env_var == '' then
            WPI.src_basedir = '..\'
         else
            WPI.src_basedir = env_var
      end
   if right(WPI.src_basedir, 1) \= '\' then
      WPI.src_basedir = WPI.src_basedir || '\'
   if args == '' then
      WPI.infile = 'BLD_FM2_WPIDIRS.IN'
   else
      if left(args, 1) == '"' then
         parse var args '"' WPI.infile '"' .
      else
         WPI.infile = word(args, 1)
   if stream(WPI.infile, 'c', 'query exists') == '' then
      WPI.infile = 'BLD_FM2_WPIDIRS.TXT'
return

MakePackageDir: procedure expose (globals)
   parse arg pkgnum
   full_dirname = 'PACKAGE.' || right(pkgnum, WPI.max_package_number_length, '0')
   call SysMkDir full_dirname
return full_dirname

