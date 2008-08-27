/*
	$Id: $
	
   CFGMGR.CMD - manage installation, maintenance and deinstallation
   				 of FM/2 configuration files

   Optional Parameters:

           /DEFAULTS     Forces overwrite of existing configuration
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
t = time('e')
signal on novalue                 /* for debugging */

call RxFuncAdd 'SysLoadFuncs', 'REXXUTIL', 'SysLoadFuncs'
call SysLoadFuncs

globals = 'cfg. button.'

call Init

parse arg args
call ProcessArgs strip(args)
if cfg.errorcode \= 0 then
   signal ErrorExit

if cfg.unattended = 0 then
   call GetUserOptions

if cfg.userabort = 1 then
   signal NormalExit

if cfg.operation = 'INSTALL' then
   call UpdateFM2Ini

cfg.action_taken = 0
do f = 1 to cfg.file.0
   file_exists = stream(cfg.file.f.name, 'c' , 'query exists')
   if cfg.operation = 'INSTALL' then
      if file_exists \= '' then
         do
            if cfg.defaults = 1 then
               if FilesAreDifferent(cfg.file.f.default, cfg.file.f.name) = 1 then
                  do
                     if cfg.unattended = 0 then
                        do
                           user_choice = PromptForReplaceOption(f)
                           if user_choice == 'Q' then
                              signal NormalExit
                           if user_choice == 'N' then
                              iterate
                        end
                     /* unattended = 1 or user_choice = 'Y' */
                     cfg.errorcode = CfgAction( 'RESETTODEFAULT', f )
                  end
               else
                  nop
            else
               if (cfg.file.f.toolbar = 1) & (cfg.unattended = 1) then
                  call Ticket267Fix cfg.file.f.name
         end
      else
        cfg.errorcode = CfgAction( 'INSTALLDEFAULT', f )
   else /* operation = deinstall */
      do
         if BackupFileIsOK(f) = 0 then
            iterate
         if cfg.unattended = 0 then
            do
               user_choice = PromptForReplaceOption(f)
               if user_choice == 'Q' then
                  signal NormalExit
               if user_choice == 'N' then
                  iterate
            end
         cfg.errorcode = CfgAction('DEINSTALL', f)
         if cfg.errorcode \= 0 then
            leave
      end
end

if cfg.errorcode \= 0 then
   signal ErrorExit

if cfg.action_taken = 0 then
   do
      say
      say 'No action taken.'
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
say 'Time elapsed:' time('e')
n = endlocal()
exit cfg.errorcode

/* Subroutines */
Init: procedure expose (globals)

	button.				= ''
	
	button.CMDS.helptext = 'Commands toolbar'
	button.CMDS.text     = 'Cmds'
	button.CMDS.id       = 4900
	
	button.UTILS.helptext = 'Utility toolbar'
	button.UTILS.text     = 'Utils'
	button.UTILS.id       = 4901
	
	button.SORT.helptext = 'Sort toolbar'
	button.SORT.text     = 'Sort'
	button.SORT.id       = 4902
	
	button.SELECT.helptext = 'Select toolbar'
	button.SELECT.text     = 'Select'
	button.SELECT.id       = 4903
	
	button.CONFIG.helptext = 'Configuration toolbar'
	button.CONFIG.text     = 'Cmds'
	button.CONFIG.id       = 4904
	
	button.FILES.helptext = 'Files toolbar'
	button.FILES.text     = 'Files'
	button.FILES.id       = 4905
	
	button.VIEWS.helptext = 'Views toolbar'
	button.VIEWS.text     = 'Views'
	button.VIEWS.id       = 4906
	
   cfg.              = ''
   cfg.errorcode     = 0
   cfg.defaults      = 0
   cfg.unattended    = 0
   cfg.actionmethod  = 'COPY'       /* The default method of backing up and restoring */
   cfg.backupdir     = '.\User_Config_Backup'

   call SysMkDir cfg.backupdir

   f = 0

   /*   Read from a file instead?   */

   f = f + 1
   cfg.file.f.default = '.\Tmplates\assoc.tmp'
   cfg.file.f.name    = '.\assoc.dat'
   cfg.file.f.desc.1  = 'FM/2 associations'
   cfg.file.f.desc.0  = 1
   cfg.file.f.toolbar = 0

   f = f + 1
   cfg.file.f.default = '.\Tmplates\commands.tmp'
   cfg.file.f.name    = '.\commands.dat'
   cfg.file.f.desc.1  = 'FM/2 user-defined commands'
   cfg.file.f.desc.0  = 1
   cfg.file.f.toolbar = 0

   f = f + 1
   cfg.file.f.default = '.\Tmplates\archiver.tmp'
   cfg.file.f.name    = '.\archiver.bb2'
   cfg.file.f.desc.1  = 'FM/2 archiver definitions'
   cfg.file.f.desc.0  = 1
   cfg.file.f.toolbar = 0

   f = f + 1
   cfg.file.f.default = '.\Tmplates\cmds.tmp'
   cfg.file.f.name    = '.\cmds.tls'
   cfg.file.f.desc.1  = 'FM/2 toolbar: commands'
   cfg.file.f.desc.0  = 1
   cfg.file.f.toolbar = 1

   f = f + 1
   cfg.file.f.default = '.\Tmplates\files.tmp'
   cfg.file.f.name    = '.\files.tls'
   cfg.file.f.desc.1  = 'FM/2 toolbar: files'
   cfg.file.f.desc.0  = 1
   cfg.file.f.toolbar = 1

   f = f + 1
   cfg.file.f.default = '.\Tmplates\select.tmp'
   cfg.file.f.name    = '.\select.tls'
   cfg.file.f.desc.1  = 'FM/2 toolbar: select'
   cfg.file.f.desc.0  = 1
   cfg.file.f.toolbar = 1

   f = f + 1
   cfg.file.f.default = '.\Tmplates\sort.tmp'
   cfg.file.f.name    = '.\sort.tls'
   cfg.file.f.desc.1  = 'FM/2 toolbar: sort'
   cfg.file.f.desc.0  = 1
   cfg.file.f.toolbar = 1

   f = f + 1
   cfg.file.f.default = '.\Tmplates\utils.tmp'
   cfg.file.f.name    = '.\utils.tls'
   cfg.file.f.desc.1  = 'FM/2 toolbar: utils'
   cfg.file.f.desc.0  = 1
   cfg.file.f.toolbar = 1

   f = f + 1
   cfg.file.f.default = '.\Tmplates\views.tmp'
   cfg.file.f.name    = '.\views.tls'
   cfg.file.f.desc.1  = 'FM/2 toolbar: views'
   cfg.file.f.desc.0  = 1
   cfg.file.f.toolbar = 1

   f = f + 1
   cfg.file.f.default = '.\Tmplates\config.tmp'
   cfg.file.f.name    = '.\config.tls'
   cfg.file.f.desc.1  = 'FM/2 toolbar: config'
   cfg.file.f.desc.0  = 1
   cfg.file.f.toolbar = 1

   f = f + 1
   cfg.file.f.default = '.\Tmplates\fm3tools.tmp'
   cfg.file.f.name    = '.\fm3tools.dat'
   cfg.file.f.desc.1  = 'FM/2 toolbar: tools?'
   cfg.file.f.desc.0  = 1
   cfg.file.f.toolbar = 0

   f = f + 1
   cfg.file.f.default = '.\Tmplates\filters.tmp'
   cfg.file.f.name    = '.\filters.dat'
   cfg.file.f.desc.1  = 'FM/2 filters'
   cfg.file.f.desc.0  = 1
   cfg.file.f.toolbar = 0

   f = f + 1
   cfg.file.f.default = '.\Tmplates\quicktls.tmp'
   cfg.file.f.name    = '.\quicktls.dat'
   cfg.file.f.desc.1  = 'FM/2 toolbar list'
   cfg.file.f.desc.0  = 1
   cfg.file.f.toolbar = 0

   f = f + 1
   cfg.file.f.default = '.\Tmplates\fatopt.tmp'
   cfg.file.f.name    = '.\fatopt.cmd'
   cfg.file.f.desc.1  = 'FM/2 FAT optimizing command'
   cfg.file.f.desc.0  = 1
   cfg.file.f.toolbar = 0

   f = f + 1
   cfg.file.f.default = '.\Tmplates\hpfsopt.tmp'
   cfg.file.f.name    = '.\hpfsopt.cmd'
   cfg.file.f.desc.1  = 'FM/2 HPFS optimizing command'
   cfg.file.f.desc.0  = 1
   cfg.file.f.toolbar = 0

   f = f + 1
   cfg.file.f.default = '.\Tmplates\jfsopt.tmp'
   cfg.file.f.name    = '.\jfsopt.cmd'
   cfg.file.f.desc.1  = 'FM/2 JFS optimizing command'
   cfg.file.f.desc.0  = 1
   cfg.file.f.toolbar = 0

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
         when param = '/INSTALL' then
            if cfg.operation \= '' then
               cfg.errorcode = 1
            else
               cfg.operation = 'INSTALL'
         when param = '/DEINSTALL' then
            if cfg.operation \= '' then
               cfg.errorcode = 1
            else
               cfg.operation = 'DEINSTALL'
         when param = '/UNATTENDED' then
            cfg.unattended = 1
         when param = '/DEFAULTS' then
            cfg.defaults = 1
         otherwise
            cfg.errorcode = 2
      end
   end
   if cfg.operation = '' then
      cfg.errorcode = 1
   else
      if cfg.operation = 'INSTALL' & cfg.defaults = 0 then
         cfg.unattended = 1
return

GetUserOptions: procedure expose (globals)
   call SysCls
   option = ''
   if cfg.defaults = 1 then
      do
         do while wordpos(option, 'A C Q') == 0
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
            say
            option = GetResponse('Type ''A'' for ALL files, ''C'' for Confirmations, ''Q'' to quit')
         end
         cfg.userabort = (option == 'Q')
         if option == 'A' then
            do
               say
               say
               say '****  WARNING  ****' || copies(d2c(7), 3)
               say
               say 'You have chosen to replace ALL configuration files with'
               say 'default values.'
               say
               if GetResponse('Type ''Y'' to proceed, anthing else cancels') = 'Y' then
                  cfg.unattended = 1
               else
                  cfg.userabort = 1
            end
      end
   else
      if cfg.operation = 'DEINSTALL' then
         do
            /* messages here about delete */
            call SysCls
            say
            say '****  WARNING  ****' || copies(d2c(7), 3)
            say
            say 'If you provide your consent, this program will'
            say 'DELETE all FM/2 configuration files.'
            say
            if GetResponse('Type ''Y'' to proceed, anthing else cancels') = 'Y' then
               cfg.unattended = 1
            else
               cfg.userabort = 1
         end
return

GetResponse: procedure
   parse arg prompt
   call charout , prompt || ': '
   reply = translate(strip(SysGetKey()))
   say
   return reply

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

PromptForReplaceOption: procedure expose (globals)
   parse arg f
   filename = filespec('N', cfg.file.f.name)
   call SysCls
   say
   say
   say 'Configuration file reset confirmation'
   say
   say 'File:'
   say '   'filename
   say
   say 'Description:'
   do d = 1 to cfg.file.f.desc.0
      say '   'cfg.file.f.desc.d
   end
   say
   if cfg.operation = 'INSTALL' then
      do
         say 'Back up and then overwrite your current 'filename
         msg = 'with default values'
      end
   else
      msg = 'Replace your current 'filename' with the backup file'
return GetResponse(msg '(Y/n)?')

Usage: procedure expose (globals)
   say 'Proper usage of 'cfg.pgmname':'
   say
   say cfg.pgmname' /INSTALL [/UNATTENDED]'
   say '   This installs any missing configuration files with default values.'
   say
   say cfg.pgmname' /INSTALL /DEFAULTS [/UNATTENDED]'
   say '   This action backs up user-modified configuration files and replaces'
   say '   them with default values. Unless /UNATTENDED, you are asked to confirm'
   say '   this action.'
   say
   say cfg.pgmname' /DEINSTALL /DEFAULTS [/UNATTENDED]'
   say '   This reverses the action described above.'
   say
   say cfg.pgmname' /DEINSTALL [/UNATTENDED]'
   say '   This deletes all configuration files. Unless /UNATTENDED, you are asked'
   say '   to confirm this action. (This is action is automatically done during'
   say '   de-installation of FM/2. It is not an action users should normally take.)'
   say
   say 'The optional parameter /UNATTENDED means there will be NO user interaction'
   say '   or confirmation of the operation!!!'
   say 'The order of the parameters is not important.'
return

ProgramError: procedure expose (globals)
   say 'Fatal prgram error.'
   say
   say 'This file may have been altered in some way. Please re-install FM/2.'
   say
   say 'If this error continues after re-installing FM/2, contact FM/2 support'
   say 'through the FM2USER group on Yahoo.'
exit

novalue:
   say 'Uninitialized variable: ' || condition('D') || ' on line: 'sigl
   say 'Line text: 'sourceline(sigl)
   cfg.errorcode = 3
   signal ErrorExit

CfgAction: procedure expose (globals)
   parse arg action, f
   retval = 0
   select
      when action = 'RESETTODEFAULT' then
         select
            when cfg.actionmethod = 'COPY' then
               do
                  'copy 'cfg.file.f.name cfg.backupdir
                  'copy 'cfg.file.f.default cfg.file.f.name
                  cfg.action_taken = 1
               end
            /* Implement other archive/restore methods here */
            otherwise
               retval = 4
         end
      when action = 'INSTALLDEFAULT' then
         select
            when cfg.actionmethod = 'COPY' then
               do
                  'copy 'cfg.file.f.default cfg.file.f.name
                  cfg.action_taken = 1
               end
            /* Implement other archive/restore methods here */
            otherwise
               retval = 4
         end
      when action = 'DEINSTALL' then
         if cfg.defaults = 1 then
            select
               when cfg.actionmethod = 'COPY' then
                  do
                     'copy 'cfg.backupdir || '\' || cfg.file.f.name' .'
                     cfg.action_taken = 1
                  end
               /* Implement other archive/restore methods here */
               otherwise
                  retval = 4
            end
         else
            select
               when cfg.actionmethod = 'COPY' then
                  if stream(cfg.file.f.name, 'c', 'query exists') \= '' then
                     if FilesAreDifferent(cfg.file.f.name, cfg.file.f.default) = 0 then
                        do
                           'del 'cfg.file.f.name
                           cfg.action_taken = 1
                        end
               /* Implement other archive/restore methods here */
               otherwise
                  retval = 4
            end
      otherwise
         retval = 5

   end
return retval

BackupFileIsOK: procedure expose (globals)
   parse arg f
   retval = 1                    /* assume yes */
   if cfg.defaults = 1 then      /* Not needed when cfg.defaults = 0 */
      do
         retval = 0
         select
            when cfg.actionmethod = 'COPY' then
               do
                  backup_file = cfg.backupdir || '\' || cfg.file.f.name
                  if stream(backup_file, 'c', 'query exists') \= '' then
                     retval = FilesAreDifferent(cfg.file.f.name, backup_file)
               end
            /* Implement other archive/restore methods here */
            otherwise
               cfg.errorcode = 4
         end
      end
return retval

UpdateFM2Ini: procedure expose (globals)
   parse source . . thispgm
   thisdir = left(thispgm, lastpos('\', thispgm))
   inifile = thisdir || 'fm3.ini'
   EnvVarList = strip(SysIni(inifile, 'FM/3', 'TreeEnvVarList'))
   if EnvVarList = '' | EnvVarList = 'ERROR:' then
      call SysIni inifile, 'FM/3', 'TreeEnvVarList', 'PATH;DPATH;LIBPATH;HELP;BOOKSHELF;LIB;INCLUDE;LOCPATH;SMINCLUDE;LPATH;CODELPATH'
   LastToolbox = SysIni(inifile, 'FM/3', 'LastToolBox')
   LastToolbar = SysIni(inifile, 'FM/3', 'LastToolbar')
   if LastToolbar = 'ERROR:' then
      do
         if LastToolBox = 'ERROR:' then
            LastToolbar = 'CMDS.TLS'
         else
	      	LastToolbar = LastToolbox
      	call SysIni inifile, 'FM/3', 'LastToolbar', LastToolbar
      end
   if SysIni(inifile, 'FM/3', 'FM2Shudown.Toolbar') = 'ERROR:' then
  		call SysIni inifile, 'FM/3', 'FM2Shutdown.Toolbar', LastToolbar
return

Ticket267Fix: procedure expose (globals)
	parse arg infile
	fix_string = 'Fixed: Ticket 267'
	abort_fix = 0
	outfile = SysTempFilename('cfgmgr??.fix')
	call lineout outfile, ';' fix_string
	/* Read file, if comment has "already repaired" message then stop */
	do while lines(infile) > 0
		line = linein(infile)
		if left(line, 1) = ';' then
			if pos(fix_string, line) > 0 then
				do
					abort_fix = 1
					leave
				end
			else
				call lineout outfile, line
		else if word(line, 1) = ':spacer' then
			call lineout outfile, ';' || substr(line, 2)
		else
			do
				tool.1 = line
				tool.2 = linein(infile)
				tool.3 = linein(infile)
				tool.4 = linein(infile)
				button_name = translate(tool.2)
				if (button.button_name.text \= '') & (tool.4 >= 4900) & (tool.4 < 4950) then
					do
						call lineout outfile, button.button_name.helptext
						call lineout outfile, button.button_name.text
						call lineout outfile, tool.3
						call lineout outfile, button.button_name.id
					end
				else
					do
						call lineout outfile, tool.1
						call lineout outfile, tool.2
						call lineout outfile, tool.3
						call lineout outfile, tool.4
					end
			end					
	end
	call stream infile, 'c', 'close'
	call stream outfile, 'c', 'close'
	if abort_fix = 0 then
		do
         'copy' infile cfg.backupdir         /* backup tls file */
			'copy' outfile infile         		/* "install" new  tls file */
			cfg.action_taken = 1
		end
	call SysFileDelete outfile
return

