/*
 *
 * REXX code to create and populate directories in preparation for the creation of the
 * FM/2 Warpin archive (WPI file).
 *
 * TODO:
 *    -  Once the new dummy file idea is accepted and debugged, change this and *.TXT
 *       so that the filename is not included in *.TXT. (Or implement the
 *       auto-determination of needed dummy files.)
 *    -  Add logic to figure out "DUMMYFILE" needs automatically
 *    -  Try setting +r attrib on DUMMYFILE's?
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
         if WPI.pkg.p.file.f.src = 'DUMMY:' then
            do
               dummy_filename = dest_dir || '\' || 'Keep_Me.' || right(WPI.pkg.p.number, length(WPI.max_package_number), '0')
               '@if exist 'dummy_filename' del 'dummy_filename' 1>NUL 2>NUL'
               call lineout dummy_filename, ''
               call lineout dummy_filename, 'Due to limitations of Warpin that existed at the time this FM/2 install'
               call lineout dummy_filename, 'was developed, this otherwise pointless file is REQUIRED.'
               call lineout dummy_filename, ''
               call lineout dummy_filename, 'Please do NOT modify or delete this file.'
               call lineout dummy_filename, ''
               call lineout dummy_filename              /* This closes the file */
/*
               'attrib 'dummy_filename' +r'
*/
               say 'Warpin dummy file: "'dummy_filename || '" written.'
            end
         else
            do
               'copy ' || WPI.src_basedir || WPI.pkg.p.file.f.src || '\' || WPI.pkg.p.file.f.name || ' ' || dest_dir
/*
               if translate(right(WPI.pkg.p.file.f.name, 4)) == '.CMD' then
                  'eautil ' || dest_dir || '\' || WPI.pkg.p.file.f.name || ' NUL /s'
*/
            end
      end
end

if WPI.retval \= 0 then
   signal ErrorExit
   '@if exist bld_fm2_wpidirs.last del bld_fm2_wpidirs.last'
   call lineout 'bld_fm2_wpidirs.last'
   signal NormalExit

ErrorExit:
   parse arg errorcode
   say 'Error code: 'errorcode
   say
   say 'Exiting...'
   say

NormalExit:
   n = endlocal()
   exit WPI.retval

novalue:
   say 'Error: Unitialized value: "' || condition('D') || ' encountered on line: 'sigl
   say '   'sourceline(sigl)
   call ErrorExit 5

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
               call ErrorExit 1      /* More than one PACKAGECOUNT */
         when word1 == 'PACKAGE:' then
            do
               parse var txtline . pkgnum pkgdesc
               if wordpos(pkgnum, package_number_list) > 0 then
                  call ErrorExit 6
               package_number_list = package_number_list pkgnum
               p = words(package_number_list)
               if p > WPI.pkg.0 then        /*  More packages than PACKAGECOUNT?  */
                  call ErrorExit 2
               WPI.pkg.p.number = pkgnum
               WPI.pkg.p.desc = strip(pkgdesc)
            end
         when (word1 == 'FILE:' | word1 == 'DUMMYFILE:') then
            do
               if word1 == 'DUMMYFILE:' then
                  parse var txtline . filename pkgnum filedest rest
               else
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
                        if word1 == 'DUMMYFILE:' then
                           WPI.pkg.p.file.i.src  = 'DUMMY:'
                        else
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

