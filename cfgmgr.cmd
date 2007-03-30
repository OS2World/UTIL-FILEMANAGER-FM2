/* CFGMGR.CMD - manage installation and deinstallation
   of FM/2 configuration files

   Optional Parameters:

           /RESET        Forces overwrite of existing configuration
                   files, i.e. "reset to FM/2 defaults"

           /DELETE        Deletes the configuration files

           /UNATTENDED
                   Runs without user interaction/confirmation

   If CFGMGR.CMD is run without parameters, it will install a default
   configuration file ONLY if that configuration file does not already
   exist.

   TODO:
      Read filename, dirname info from a file
      Allow the above filename to be passed as a parameter
      Edit to make sure this program is located correctly

*/

n = setlocal()

signal on novalue                 /* for debugging */

call RxFuncAdd 'SysLoadFuncs', 'REXXUTIL', 'SysLoadFuncs'
call SysLoadFuncs

globals = 'cfg.'

call Init

call DeleteObsoleteFiles

parse arg args
call ProcessArgs strip(args)
if cfg.errorcode \= 0 then
   signal ErrorExit

if cfg.unattended = 0 then
   call GetUserOptions

if cfg.userabort = 1 then
   signal NormalExit

action_taken = 0
do f = 1 to cfg.file.0
   file_exists = stream(cfg.file.f.name, 'c' , 'query exists')
   if cfg.delete = 1 then
      if file_exists \= '' then
         do
            'del 'cfg.file.f.name
            action_taken = 1
         end
      else
         nop
   else
      /* must be a "reset" operation */
      select
         when file_exists == '' then                     /* If file is missing... */
            do
               'copy 'cfg.file.f.tmpl8 cfg.file.f.name
               action_taken = 1
            end
         when cfg.reset_overwrite == 1 then
            if FilesAreDifferent(cfg.file.f.tmpl8, cfg.file.f.name) == 1 then
               if cfg.unattended == 1 | ,                   /* If unattended reset, or  */
                  cfg.reset_overwrite_option == 'A' then    /* user has wants "ALL" overwrites... */
                  do
                     'copy 'cfg.file.f.tmpl8 cfg.file.f.name
                     action_taken = 1
                  end
               else
                  if PromptedReset(f) == 'Y' then           /* If user has OK'd this overwrite... */
                     do
                        'copy 'cfg.file.f.tmpl8 cfg.file.f.name
                        action_taken = 1
                     end
         otherwise
            nop
      end
end

if cfg.errorcode \= 0 then
   signal ErrorExit
if action_taken = 0 then
   do
      say
      say 'FM/2 configuration files are already as you desire.'
   end
signal NormalExit

ErrorExit:
   select
      when cfg.errorcode = 1 then
         call Usage
      when cfg.errorcode = 2 then
         call Usage
      when cfg.errorcode = 3 then
         call ProgramError
      otherwise
         call ProgramError
   end

NormalExit:
say
say cfg.pgmname' has ended.'
n = endlocal()
exit cfg.errorcode

/* Subroutines */
Init: procedure expose (globals)

   cfg.              = ''
   cfg.errorcode     = 0
   cfg.reset_overwrite = 0
   cfg.unattended    = 0
   cfg.delete        = 0

   f = 0

   /*   Read from a file instead?   */

   f = f + 1
   cfg.file.f.tmpl8   = '.\Tmplates\assoc.tmp'
   cfg.file.f.name    = '.\assoc.dat'
   cfg.file.f.desc.1  = 'FM/2 associations'
   cfg.file.f.desc.0  = 1

   f = f + 1
   cfg.file.f.tmpl8   = '.\Tmplates\commands.tmp'
   cfg.file.f.name    = '.\commands.dat'
   cfg.file.f.desc.1  = 'FM/2 user-defined commands'
   cfg.file.f.desc.0  = 1

   f = f + 1
   cfg.file.f.tmpl8   = '.\Tmplates\archiver.tmp'
   cfg.file.f.name    = '.\archiver.bb2'
   cfg.file.f.desc.1  = 'FM/2 archiver definitions'
   cfg.file.f.desc.0  = 1

   f = f + 1
   cfg.file.f.tmpl8   = '.\Tmplates\cmds.tmp'
   cfg.file.f.name    = '.\cmds.tls'
   cfg.file.f.desc.1  = 'FM/2 toolbar: commands'
   cfg.file.f.desc.0  = 1

   f = f + 1
   cfg.file.f.tmpl8   = '.\Tmplates\files.tmp'
   cfg.file.f.name    = '.\files.tls'
   cfg.file.f.desc.1  = 'FM/2 toolbar: files'
   cfg.file.f.desc.0  = 1

   f = f + 1
   cfg.file.f.tmpl8   = '.\Tmplates\select.tmp'
   cfg.file.f.name    = '.\select.tls'
   cfg.file.f.desc.1  = 'FM/2 toolbar: select'
   cfg.file.f.desc.0  = 1

   f = f + 1
   cfg.file.f.tmpl8   = '.\Tmplates\sort.tmp'
   cfg.file.f.name    = '.\sort.tls'
   cfg.file.f.desc.1  = 'FM/2 toolbar: sort'
   cfg.file.f.desc.0  = 1

   f = f + 1
   cfg.file.f.tmpl8   = '.\Tmplates\utils.tmp'
   cfg.file.f.name    = '.\utils.tls'
   cfg.file.f.desc.1  = 'FM/2 toolbar: utils'
   cfg.file.f.desc.0  = 1

   f = f + 1
   cfg.file.f.tmpl8   = '.\Tmplates\views.tmp'
   cfg.file.f.name    = '.\views.tls'
   cfg.file.f.desc.1  = 'FM/2 toolbar: views'
   cfg.file.f.desc.0  = 1

   f = f + 1
   cfg.file.f.tmpl8   = '.\Tmplates\config.tmp'
   cfg.file.f.name    = '.\config.tls'
   cfg.file.f.desc.1  = 'FM/2 toolbar: config'
   cfg.file.f.desc.0  = 1

   f = f + 1
   cfg.file.f.tmpl8   = '.\Tmplates\fm3tools.tmp'
   cfg.file.f.name    = '.\fm3tools.dat'
   cfg.file.f.desc.1  = 'FM/2 toolbar: tools?'
   cfg.file.f.desc.0  = 1

   f = f + 1
   cfg.file.f.tmpl8   = '.\Tmplates\filters.tmp'
   cfg.file.f.name    = '.\filters.dat'
   cfg.file.f.desc.1  = 'FM/2 filters'
   cfg.file.f.desc.0  = 1

   f = f + 1
   cfg.file.f.tmpl8   = '.\Tmplates\quicktls.tmp'
   cfg.file.f.name    = '.\quicktls.dat'
   cfg.file.f.desc.1  = 'FM/2 toolbar list'
   cfg.file.f.desc.0  = 1

   f = f + 1
   cfg.file.f.tmpl8   = '.\Tmplates\fatopt.tmp'
   cfg.file.f.name    = '.\fatopt.cmd'
   cfg.file.f.desc.1  = 'FM/2 FAT optimizing command'
   cfg.file.f.desc.0  = 1

   f = f + 1
   cfg.file.f.tmpl8   = '.\Tmplates\hpfsopt.tmp'
   cfg.file.f.name    = '.\hpfsopt.cmd'
   cfg.file.f.desc.1  = 'FM/2 HPFS optimizing command'
   cfg.file.f.desc.0  = 1

   f = f + 1
   cfg.file.f.tmpl8   = '.\Tmplates\jfsopt.tmp'
   cfg.file.f.name    = '.\jfsopt.cmd'
   cfg.file.f.desc.1  = 'FM/2 JFS optimizing command'
   cfg.file.f.desc.0  = 1

   cfg.file.0         = f

   parse source . . thispgm
   p = lastpos('\', thispgm)
   cfg.pgmname = translate(substr(thispgm, p + 1, lastpos('.', thispgm) - p - 1))
   thisdir           = left(thispgm, lastpos('\', thispgm) - 1)
   if length(thisdir) = 2 then
      thisdir = thisdir || '\'
   call directory thisdir
   /* edit for correct directory here? */

   parse value SysTextScreenSize() with . cfg.screen_width
   if cfg.screen_width = 0 then           /* for running in PMREXX */
      cfg.screen_width = 80

return

ProcessArgs: procedure expose (globals)
   parse upper arg args
   do while args \= '' & cfg.errorcode = 0
      parse var args param args
      select
         when param = '/RESET' then
            if cfg.delete = 1 then
               cfg.errorcode = 1
            else
               cfg.reset_overwrite = 1
         when param = '/DELETE' then
            if cfg.reset_overwrite = 1 then
               cfg.errorcode = 1
            else
               cfg.delete = 1
         when param = '/UNATTENDED' then
            cfg.unattended = 1
         otherwise
            cfg.errorcode = 2
      end
   end
return

GetUserOptions: procedure expose (globals)
   if cfg.reset_overwrite = 1 then
      do
         /* messages here about reset */
         call SysCls
         do while wordpos(cfg.reset_overwrite_option, 'A C Q') == 0
            say
            say
            say
            say ' *** Reset configuration files to default values ***'
            say
            say 'This program can reset ALL files to default values.'
            say
            say 'OR'
            say
            say 'This program can ask for confirmation before each file,'
            say 'if it already exists, is replaced with its default.'
            cfg.reset_overwrite_option = GetResponse('Type ''A'' for ALL files, ''C'' for Confirmations, ''Q'' to quit')
         end
         cfg.userabort = (cfg.reset_overwrite_option == 'Q')
         if cfg.reset_overwrite_option == 'A' then
            do
               say
               say
               say center(copies('   ** Warning!! **   ', 3), cfg.screen_width)
               say
               say 'You have chosen to replace ALL configuration files with'
               say 'default values.'
               cfg.userabort = (GetResponse('Type ''Y'' to proceed, anthing else cancels') \= 'Y')
            end
      end
   else
      if cfg.delete = 1 then
         do
            /* messages here about delete */
            call SysCls
            say
            say center(copies('   ** Warning!! **   ', 3), cfg.screen_width)
            say
            say 'If you provide your consent, this program will'
            say 'DELETE all FM/2 configuration files.'
            say
            cfg.userabort = (GetResponse('Type ''Y'' to proceed, anthing else cancels') \= 'Y')
         end
return

GetResponse: procedure
   parse arg prompt
   say
   call charout , prompt || ': '
   reply = translate(strip(SysGetKey()))
   say
   return reply

DeleteObsoleteFiles: procedure expose (globals)
   i  =  0

   i  =  i + 1
   Obsolete.i = 'readme.org'

   i  =  i + 1
   Obsolete.i = 'install.cmd'

   i  =  i + 1
   Obsolete.i = 'install.dat'

   i  =  i + 1
   Obsolete.i = 'uninstal.cmd'

   Obsolete.0 = i
   do i = 1 to Obsolete.0
      call SysFileTree Obsolete.i, 'list.', 'FOS'
      do j = 1 to list.0
         'del 'list.j
      end
   end
return

FilesAreDifferent: procedure
   parse arg file.1, file.2
   retval = 1                 /* assume files are different */
   file.1.size = stream(file.1, 'c', 'query size')
   file.2.size = stream(file.2, 'c', 'query size')
   if file.1.size = file.2.size then
      do
         file.1.data = charin(file.1, 1, file.1.size)
         call stream file.1, 'c', 'close'
         file.2.data = charin(file.2, 1, file.2.size)
         call stream file.2, 'c', 'close'
         if file.1.data = file.2.data then
            retval = 0
      end
return retval

PromptedReset: procedure expose (globals)
   parse arg f
   call SysCls
   say
   say
   say 'Configuration file reset confirmation'
   say
   say 'File:'
   say '   'substr(cfg.file.f.name, lastpos('\', cfg.file.f.name) + 1)
   say
   say 'Description:'
   do d = 1 to cfg.file.f.desc.0
      say '   'cfg.file.f.desc.d
   end
   say
return GetResponse('Type ''Y'' to reset to default')

Usage: procedure expose (globals)
   say 'Invalid usage. See below for acceptable calls:'
   say
   say cfg.pgmname' <no parameters>'
   say '   This installs missing configuration files with default values.'
   say
   say cfg.pgmname' /RESET'
   say '   This replaces existing configuration files with default values.'
   say '   You asked to confirm this action. You can choose to replace all'
   say '   files or you can request a file-by-file confirmation.'
   say
   say cfg.pgmname' /DELETE'
   say '   This deletes all configuration files. You are asked to confirm'
   say '   this action. (This is not an action users should normally take.)'
   say
   say cfg.pgmname' /RESET /UNATTENDED'
   say cfg.pgmname' /DELETE /UNATTENDED'
   say '   These operations operate as above except there is NO user'
   say '   interaction or confirmation! Use with extreme care!'
   say
   say 'The order of the parameters is not important.'
return

ProgramError: procedure expose (globals)
   say 'Fatal prgram error.'
   say
   say 'This file may have been altered in some way. Please re-install FM/2.'
   say
   say 'If this error continues after re-installing FM/2, contact FM/2 support'
   say 'through the FM2USER group on Yahoo.'
return

novalue:
   say 'Uninitialized variable: ' || condition('D') || ' on line: 'sigl
   say 'Line text: 'sourceline(sigl)
   cfg.errorcode = 3
   signal ErrorExit

