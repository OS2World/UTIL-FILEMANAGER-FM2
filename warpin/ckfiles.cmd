/*      Check files to see if they have been updated */

call RxFuncAdd 'SysLoadFuncs', 'REXXUTIL', 'SysLoadFuncs'
call SysLoadFuncs

signal on novalue

wpi_file          =  'fm2.wpi'
flag_file         =  'lastfile.upd'
dummy_date_time   =  '99 99 99 23 59 59'
wpi_file_date_time=  GetDate(wpi_file)
if wpi_file_date_time = dummy_date_time then
   return

infile   =  'bld_fm2_wpidirs.txt'

do while lines(infile) > 0
   line = translate(strip(linein(infile)))
   if line \= '' then
      if word(line, 1) = 'FILE:' then
         do
            file  =  word(line, 2)
            dir   =  word(line, 4)
            file_date_time = GetDate( '..\' || dir || '\' || file )
            if file_date_time >  wpi_file_date_time then
               do
                  'if exist 'flag_file' del 'flag_file
                  call lineout flag_file
                  leave
               end
         end
end

return

GetDate: procedure expose dummy_date_time
   parse arg file
   call SysFileTree file, 'list.', 'FL'
   if list.0 \= 1 then
      return dummy_date_time
   else
      return translate(word(list.1, 1) word(list.1, 2), '   ', '-/:')
return

novalue:
   say 'Uninitialized value: 'condition('D')' on line: 'sigl
   say 'Line text: 'sourceline(sigl)
   exit
