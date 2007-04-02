/*
 *
 * REXX code to create and populate directories in preparation for the creation of the
 * FM/2 Warpin archive (WPI file).
 *
 * TODO:
 *    -  Add logic to read script to determine
 *       -  Number of packages
 *       -  Package numbers
 *    -  Add logic to copy only if the source is newer
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
WPI.max_package_number = 999

parse arg args
call ProcessArgs strip(args)

call ReadFile

do p = 1 to WPI.pkg.0
   dest_basedir   = 'PACKAGE.' || right(WPI.pkg.p.number, length(WPI.max_package_number), '0')
   '@if not exist 'dest_basedir'\. md 'dest_basedir
   if WPI.pkg.p.file.0 \= '' then
      do f = 1 to WPI.pkg.p.file.0
         dest_dir = dest_basedir || '\' || WPI.pkg.p.file.f.dest
         '@if not exist 'dest_dir'\. md 'dest_dir
         'copy ' || WPI.src_basedir || WPI.pkg.p.file.f.src || '\' || WPI.pkg.p.file.f.name || ' ' || dest_dir
      end
end

if WPI.retval = 0 then
   signal NormalExit

ErrorExit:
   parse arg errorcode
   select
      when errorcode = 1 then
         say 'Error: More than one PACKAGECOUNT line in 'WPI.infile
      when errorcode = 2 then
         say 'Error: More than PACKAGECOUNT PACKAGE lines in 'WPI.infile
      when errorcode = 3 then
         do
            say 'Error: Too many tokens on a line in 'WPI.infile
            say 'Line : 'txtline
         end
      when errorcode = 4 then
         say 'Error: Keyword is not PACKAGECOUNT, PACKAGE or FILE in 'WPI.infile
      when errorcode = 5 then
novalue:
         do
            say 'Error: Unitialized value: "' || condition('D') || ' encountered on line: 'sigl
            say '   'sourceline(sigl)
            signal NormalExit
         end
      when errorcode = 6 then
         do
            say 'Error: Duplicate PACKAGE line in 'WPI.infile
            say 'Line : 'txtline
         end
      when errorcode = 7 then
         do
            say 'Error: Invalid package number in 'WPI.infile
            say 'Line : 'txtline
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
   package_number_list = ''
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
      word1 = translate(word(txtline, 1))
      select
         when word1 == 'PACKAGECOUNT:' then
            if WPI.pkg.0 == '' then
               WPI.pkg.0 = word(txtline, 2)
            else
               call ErrorExit 1           /* More than one PACKAGECOUNT */
         when word1 == 'PACKAGE:' then
            do
               parse var txtline . pkgnum pkgdesc
               if wordpos(pkgnum, package_number_list) > 0 then
                  call ErrorExit 6         /* Repeated package line */
               package_number_list = package_number_list pkgnum
               p = words(package_number_list)
               if p > WPI.pkg.0 then
                  call ErrorExit 2        /*  More packages than PACKAGECOUNT  */
               WPI.pkg.p.number = pkgnum
               WPI.pkg.p.desc = strip(pkgdesc)
            end
         when (word1 == 'FILE:') then
            do
               parse var txtline . filename pkgnum filesrc filedest rest
               p = wordpos(pkgnum, package_number_list)
               if p = 0 then
                  call ErrorExit 7
               else
                  if strip(rest) \= '' then
                     call ErrorExit 3  /* Too many parameters */
                  else
                     do
                        i = WPI.pkg.p.file.0
                        if i = '' then
                           i = 1
                        else
                           i = i + 1
                        WPI.pkg.p.file.0      = i
                        WPI.pkg.p.file.i.name = filename
                        if WPI.src_basedir \= '..\' then  /* Ignore src-dir if not using build subtree */
                           WPI.pkg.p.file.i.src  = '.'
                        else
                           WPI.pkg.p.file.i.src  = filesrc
                        WPI.pkg.p.file.i.dest = filedest
                     end
            end
         otherwise
            call ErrorExit 4        /* Invalid line/word1 */
      end
   end
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
      WPI.infile = 'BLD_FM2_WPIDIRS.TXT'
   else
      if left(args, 1) == '"' then
         parse var args '"' WPI.infile '"' .
      else
         WPI.infile = word(args, 1)
return

