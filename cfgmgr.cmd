/*
   $Id$

   CFGMGR.CMD - manage installation, maintenance and deinstallation
                of FM/2 configuration files

   Optional Parameters:

           /DEFAULTS     Forces overwrite of existing configuration
                   files, i.e. "reset to FM/2 defaults"

           /DELETE        Deletes the configuration files

           /UNATTENDED
                   Runs without user interaction/confirmation

           /TOOLBARSONLY
                   Process toolbar-related files only

   If CFGMGR.CMD is run without parameters, it will install a default
   configuration file ONLY if that configuration file does not already
   exist.

   TODO:
      Read filename, dirname info from a file
      Allow the above filename to be passed as a parameter
      Edit to make sure this program is located correctly
      Add version checking

   Change log:
      16 Mar 09 JBS Added code to copy discontinued directory container state ini keys to new names
      12 Apr 10 JBS Ticket 429: Speed up the deletion of obsolete state-related FM3.INI keys.
      12 Apr 10 JBS Ticket 429: Add code to delete obsolete "LastToolBox" key(s).
      30 May 10 JBS Ticket 428: Add support for UNZIP v6. This program was modified to:
         if not already done then
            backup existing .\ARCHIVER.BB2 file to .\USER_CONFIG_BACKUP\B4UNZIP6.BB2
            insert the UNZIP v6 entry from .\TMPLATES\ARCHVER.TMP into ,\ARCHIVER.BB2 (at the end)
      30 Jun 11 JBS Ticket 459: Fix a bug and improve text when support for Unzip v6 has already been added.

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
         if (cfg.toolbarsonly = 0 | cfg.file.f.toolbar = 1) then
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
                  if (cfg.file.f.toolbar = 1) & (cfg.unattended = 1) & (translate(right(cfg.file.f.name, 13, ' ')) \= '\QUICKTLS.DAT') then
                     call Ticket267Fix cfg.file.f.name
            end
         else
            nop
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

if cfg.operation = 'INSTALL' then
   call UpdateArchiverBB2

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
n = endlocal()
exit cfg.errorcode

/* Subroutines */
Init: procedure expose (globals)

        button.                         = ''

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
   cfg.toolbarsonly  = 0
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
   cfg.file.f.toolbar = 1

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
   fm2ini = value('FM3INI',,'OS2ENVIRONMENT')
   if fm2ini = '' then
      if right(thisdir, 1) \= '\' then
      	cfg.inifile = thisdir || '\' || 'FM3.INI'
      else
      	cfg.inifile = thisdir || 'FM3.INI'
   else
      cfg.inifile = fm2ini

   /* edit for correct directory here? */

   parse value SysTextScreenSize() with . cfg.screen_width
   if cfg.screen_width = 0 then           /* for running in PMREXX */
      cfg.screen_width = 80

return

ProcessArgs: procedure expose (globals)
   parse arg args
   if pos('/ToOlBaRsOnLy', args) > 0 then
      cfg.specialiconrun = 1
   else
      cfg.specialiconrun = 0
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
         when param = '/TOOLBARSONLY' then
            cfg.toolbarsonly = 1
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
   say 'Proper usage of 'cfg.pgmname' (the order of the parameters is not important):'
   say
   say cfg.pgmname' /INSTALL [/UNATTENDED] [/TOOLBARSONLY]'
   say '   This installs any missing configuration files with default values.'
   say
   say cfg.pgmname' /INSTALL /DEFAULTS [/UNATTENDED] [/TOOLBARSONLY]'
   say '   This action backs up user-modified configuration files and replaces'
   say '   them with default values. Unless /UNATTENDED, you are asked to confirm'
   say '   this action.'
   say
   say cfg.pgmname' /DEINSTALL /DEFAULTS [/UNATTENDED] [/TOOLBARSONLY]'
   say '   This reverses the action described above.'
   say
   say cfg.pgmname' /DEINSTALL [/UNATTENDED] [/TOOLBARSONLY]'
   say '   This deletes all configuration files. Unless /UNATTENDED, you are asked'
   say '   to confirm this action. (This is action is automatically done during'
   say '   de-installation of FM/2. It is not an action users should normally take.)'
   say
   say 'The optional parameter /UNATTENDED means there will be NO user interaction'
   say '   or confirmation of the operation!!!'
   say
   say 'The optional parameter /TOOLBARSONLY means only toolbar-related files will'
   say '   be processed.'
return

ProgramError: procedure expose (globals)
   say 'Fatal prgram error.'
   say
   say 'This file may have been altered in some way. Please re-install FM/2.'
   say
   say 'If this error continues after re-installing FM/2, contact FM/2 support'
   say 'through the FM2USER group on Yahoo.'
exit

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
   EnvVarList = strip(SysIni(cfg.inifile, 'FM/3', 'TreeEnvVarList'))
   if EnvVarList = '' | EnvVarList = 'ERROR:' then
      call SysIni cfg.inifile, 'FM/3', 'TreeEnvVarList', 'PATH;DPATH;LIBPATH;HELP;BOOKSHELF;LIB;INCLUDE;LOCPATH;SMINCLUDE;LPATH;CODELPATH'
   LastToolbox = SysIni(cfg.inifile, 'FM/3', 'LastToolBox')
   LastToolbar = SysIni(cfg.inifile, 'FM/3', 'LastToolbar')
   if LastToolbar = 'ERROR:' then
      do
         if LastToolBox = 'ERROR:' then
            LastToolbar = 'CMDS.TLS'
         else
            LastToolbar = LastToolbox
        call SysIni cfg.inifile, 'FM/3', 'LastToolbar', LastToolbar
      end
   if LastToolbox \= 'ERROR:' then
      call SysIni cfg.inifile, 'FM/3', 'LastToolBox', 'DELETE:'
   if SysIni(cfg.inifile, 'FM/3', 'FM2Shutdown.Toolbar') = 'ERROR:' then
      call SysIni cfg.inifile, 'FM/3', 'FM2Shutdown.Toolbar', LastToolbar
   if SysIni(cfg.inifile, 'FM/4', 'LastToolbar') = 'ERROR:' then
      call SysIni cfg.inifile, 'FM/4', 'LastToolbar', LastToolbar
   if SysIni(cfg.inifile, 'FM/4', 'LastToolbox') \= 'ERROR:' then
      call SysIni cfg.inifile, 'FM/4', 'LastToolBox', 'DELETE:'
   do /* Copy old details keys to new names */
      /* Check to see if this has already been done */
      DetailsKeysConverted = SysIni(cfg.inifile, 'FM/3', 'DetailsKeysConverted')
      StateNames = SysIni(cfg.inifile, 'FM/3', 'LastSetups')
      KeyFragments = 'Dir Filter Pos Sort View'
      NumKeyFragments = words(KeyFragments)
      null = '00'x
      if StateNames = 'ERROR:' then
         StateNames = 'FM2Shutdown' || null
      else
         if pos('FM2Shutdown' || null, StateNames) = 0 then
            StateNames = StateNames || 'FM2Shutdown' || null
      call charout , 'One-time conversion of states to new format...'
      do while StateNames \= ''
         parse var StateNames StateName (null) StateNames
         NumDirCnrs = SysIni(cfg.inifile, 'FM/3', StateName || '.NumDirsLastTime')
         if NumDirCnrs \= 'ERROR:' then
            do
               NumDirCnrs = c2d(reverse(NumDirCnrs)) - 1  /* for 0 to num-1 loop */
               do d = 0 to NumDirCnrs
                  d2 = NumDirCnrs + 1 - d /* Work from the last to the first */
                  do f = 1 to NumKeyFragments
                     call charout , '.'
                     frag = word(KeyFragments, f)
                     OldKey = StateName || '.DirCnr' || frag || '.' || d2
                     OldKeyValue = SysIni(cfg.inifile, 'FM/3', OldKey)
                     if OldKeyValue = 'ERROR:' then
                        do
                           f = NumKeyFragments  /* Force end of loop for this container */
                           d = NumDirCnrs       /* Force end of loop for dir cnrs for this state */
                        end
                     else
                        do
                           call SysIni cfg.inifile, 'FM/3', OldKey, 'DELETE:'
                           NewKey = StateName || '.DirCnr.' || d || '.' || frag
                           call SysIni cfg.inifile, 'FM/3', NewKey, OldKeyValue
                        end
                  end
               end
               say
            end
      end
   end
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
         'copy' outfile infile                   /* "install" new  tls file */
         cfg.action_taken = 1
      end
   call SysFileDelete outfile
return

UpdateArchiverBB2: procedure expose (globals)
   bb2_file = '.\ARCHIVER.BB2'
   bb2_v6_startline = FindUnzip6(bb2_file)
   if bb2_v6_startline = 0 then
      do
         tmp_file       = '.\Tmplates\archiver.tmp'
         tmp_v6_startline = FindUnzip6(tmp_file)
         if tmp_v6_startline \= 0 then
            do
               backup_file       = '.\User_Config_backup\B4Unzip6.bb2'
               if stream(backup_file, 'c', 'query exists') \= '' then
                  if cfg.unattended = 0 then
                     do
                        say 'Support for Unzip v6 has previously been added.'
                        if GetResponse("If you want to repeat this process, type 'Y'") \= 'Y' then
                           return
                     end
                  else
                     return
               insert_line = FindEndOfDefinitions(bb2_file)
               if insert_line > 0 then
                  do
                     '@copy' bb2_file backup_file '>NUL 2>NUL'
                     if rc \= 0 then
                        return
                     call SysFileDelete bb2_file
                     do i = 1 to insert_line
                        call lineout bb2_file, bb2_lines.i
                     end
                     do j = 1 to tmp_v6_startline
                        line = linein(tmp_file)
                     end
                     do 25 /* 21 lines of def + 3 prefix comment lines + 1 postfix comment */
                        line = linein(tmp_file)
                        call lineout bb2_file, line
                     end
                     call stream tmp_file, 'c', 'close'
                     do j = i to bb2_lines.0
                        call lineout bb2_file, bb2_lines.j
                     end
                     call stream bb2_file, 'c', 'close'
                     cfg.action_taken = 1
                  end
               else
                  if cfg.unattended = 1 then
                     say 'Unable to find place to insert definition.'
            end
         else
            if cfg.unattended = 1 then
               say 'Unable to find Unzip v6 definition'
      end
   else
      if cfg.unattended = 1 then
         say 'Unzip v6 definition is already installed.'
return

FindUnzip6: procedure expose (globals)
   parse arg filename
   start_string   = '--------  ------  ------- ---- ---------- ----- --------  ----'
   end_string     = '--------          -------  ---                            -------'
   call SysFileSearch start_string, filename, 'start_lines.', 'N'
   call SysFileSearch end_string, filename, 'end_lines.', 'N'
   start_line = 0
   do i = 1 to start_lines.0
      do j = 1 to end_lines.0
         if word(end_lines.j, 1) = word(start_lines.i, 1) + 1 then
            do
               start_line = word(start_lines.i, 1) - 18
               i = start_lines.0
               j = end_lines.0
            end
      end
   end
   drop start_lines.
   drop end_lines.
return start_line

FindEndOfDefinitions: procedure expose (globals) bb2_lines.
   parse arg bb2_file
   end_of_defs_line = 0
   i = 0
   do while lines(bb2_file) > 0
      i = i + 1
      bb2_lines.i = linein(bb2_file)
      if strip(bb2_lines.i) \= '' then
         if abbrev(bb2_lines.i, ';') then
            if end_of_defs_line = 0 then
               end_of_defs_line = i
            else
               nop
         else
            end_of_defs_line = 0
   end
   call stream bb2_file, 'c', 'close'
   bb2_lines.0 = i
return end_of_defs_line


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
     call 'SYSSLEEP' 2
     exit 'CONDITION'('C')
   end
return

/* end Error */

novalue:
   say 'Uninitialized variable: ' || condition('D') || ' on line: 'sigl
   say 'Line text: 'sourceline(sigl)
   cfg.errorcode = 3
   signal ErrorExit


