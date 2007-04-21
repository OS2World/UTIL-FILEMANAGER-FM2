/* REXX program to delete FM/2 Warpin WPI file staging directories */
call RxFuncAdd 'SysFileTree', 'REXXUTIL', 'SysFileTree'
call SysFileTree 'PACKAGE.*', 'pkgdirs.', 'DO'
do p = 1 to pkgdirs.0
   call RemoveDir pkgdirs.p
end
return

RemoveDir: procedure
   parse arg dir
   call SysFileTree dir'\*', 'files.', 'FO'
   do i = 1 to files.0
      '@attrib -r -s -h 'files.i' >NUL 2>NUL'
      '@del 'files.i' >NUL 2>NUL'
   end
   drop files.
   call SysFileTree dir'\*', 'dirs.', 'DO'
   do i = 1 to dirs.0
      call RemoveDir dirs.i
   end
   '@rd 'dir
return

