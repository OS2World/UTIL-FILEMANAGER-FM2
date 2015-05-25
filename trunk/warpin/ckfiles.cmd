/*
 *
 * $Id$
 *
 * Check files destined for the WPI file to see if they have been updated.
 * If so, appropriate lines are written to bld_wpi_dirs.in so that
 * these files can first be staged and then added to the WPI file.
 *
 * Change log:
 * 	23 Oct 08 JBS Ticket 293: Improved support for -a wmake option
 */

call RxFuncAdd 'SysLoadFuncs', 'REXXUTIL', 'SysLoadFuncs'
call SysLoadFuncs

signal on novalue

parse arg '"MAKEOPTS=' make_args '"' wpi_file
wpi_file          =  strip(wpi_file)
wis_file          =  'fm2.wis'
if stream(wpi_file, 'c', 'query exists') == '' then  /* If target WPI file  does not exist, force WIS rebuild */
   call SysFileDelete wis_file

dummy_date_time   =  '-1'
in_file   =  'bld_fm2_wpidirs.txt'
out_file  =  'bld_fm2_wpidirs.in'
retval      = 0


in_file_date_time=  GetDate(in_file)
if in_file_date_time = dummy_date_time then
   do
      say 'Fatal Error: input file, 'in_file' NOT found.'
      say
      say 'Exiting...'
      return 1
   end
out_file_date_time = GetDate(out_file)
wpi_file_date_time = GetDate(wpi_file)

if pos('A', translate(make_args)) > 0 | ,
   in_file_date_time > out_file_date_time | ,
   out_file_date_time = dummy_date_time  | ,
   wpi_file_date_time = dummy_date_time         then
   do
      'copy 'in_file out_file
      call SysSetFileDateTime out_file
      if stream(wpi_file, 'c', 'query exists') \= '' then
         'del 'wpi_file
      call clean_wpidirs
   end
else
   do
      file_reset  = 0
      nfp         = 0
      do while lines(in_file) > 0
         line = strip(linein(in_file))
         if line \= '' then
            if left(line, 1) \= ';' then
               if translate(word(line, 1)) = 'FILE:' then
                  do
                     file_mask  =  word(line, 2)
                     dir   =  word(line, 4)
                     call SysFileTree '..\' || dir || '\' || file_mask, 'filelist.', 'FO'
                     do f = 1 to filelist.0
                        file = filelist.f
                        file_date_time = GetDate( file )
                        if file_date_time > out_file_date_time then
                           do
                              if file_reset = 0 then
                                 do
                                    file_reset = 1
                                    if stream(out_file, 'c', 'query exists') \= '' then
                                       '@del 'out_file
                                    call lineout out_file, '; Do not edit this file!'
                                    call lineout out_file, ';'
                                    call lineout out_file, '; Any desired edits should be done to BLD_FM2_WPIDIRS.TXT, instead.'
                                    call lineout out_file, ';'
                                 end
                              call lineout out_file, 'FILE:' substr(file, lastpos('\', file) + 1) substr(line, wordindex(line, 3))
                           end
                     end
                  end
               else /* must be a NOFILESPACKAGE line */
                  do
                     nfp = nfp + 1
                     nfpline.nfp = line
                  end

      end
      if file_reset = 1 then
         do
            nfpline.0 = nfp
            do nfp = 1 to nfpline.0
               call lineout out_file, nfpline.nfp
            end
            call stream out_file, 'c', 'close'
         end
   end

exit

GetDate: procedure
return SysGetFileDateTime( arg(1), 'W' )

novalue:
   say 'Uninitialized value: 'condition('D')' on line: 'sigl
   say 'Line text: 'sourceline(sigl)
   exit
