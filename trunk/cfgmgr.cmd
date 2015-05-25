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
      Fully implement logging

   Change log:
      16 Mar 09 JBS Added code to copy discontinued directory container state ini keys to new names
      12 Apr 10 JBS Ticket 429: Speed up the deletion of obsolete state-related FM3.INI keys.
      12 Apr 10 JBS Ticket 429: Add code to delete obsolete "LastToolBox" key(s).
      30 May 10 JBS Ticket 428: Add support for UNZIP v6. This program was modified to:
         if not already done then
            backup existing .\ARCHIVER.BB2 file to .\USER_CONFIG_BACKUP\B4UNZIP6.BB2
            insert the UNZIP v6 entry from .\TMPLATES\ARCHVER.TMP into ,\ARCHIVER.BB2 (at the end)
      30 Jun 11 JBS Ticket 459: Fix a bug and improve text when support for Unzip v6 has already been added.
      05 Mar 14 JBS Ticket #532: Modified to append additional archiver definitions to ARCHIVER,BB2
         Also, reworked the code to make appending more definitions easier in the future
         Also, added missing features and a partial implementation of logging.

*/

n = setlocal()

signal on Error
signal on FAILURE name Error
signal on Halt
signal on NOTREADY name Error
signal on NOVALUE name Error
signal on SYNTAX name Error
signal on novalue             /* for debugging */

call RxFuncAdd 'SysLoadFuncs', 'REXXUTIL', 'SysLoadFuncs'
call SysLoadFuncs

globals = 'cfg. button.'

call Init

parse arg args
call ProcessArgs strip(args)
if cfg.errorcode \= 0 then
   signal ErrorExit

if cfg.operation = 'INSTALL' then
   call UpdateFM2Ini

if cfg.attended = 1 then
   do
      call GetUserOptions
      if cfg.userabort = 1 then
         signal NormalExit
   end

/*
 *
 * The following code
 *    Installs default files if /DEFAUTLS is specified or if a cfg file is missing
 *    Handles all /DEINSTALL actions
 * It should precede the call to UpdateArchiverBB2
 *
 */

/*
trace '?i'
*/
do f = 1 to cfg.file.0
   file_exists = stream(cfg.file.f.name, 'c' , 'query exists')
   if cfg.operation = 'INSTALL' then
      if file_exists \= '' then
         if (cfg.toolbarsonly = 0 | cfg.file.f.toolbar = 1) then
            do
               if cfg.defaults = 1 then
                  if FilesAreDifferent(cfg.file.f.default, cfg.file.f.name) = 1 then
                     do
                        if cfg.attended = 1 then
                           do
                              if PromptForReplaceOption(f) = 'N' then
                                 iterate
                           end
                        /* attended = 0 or user choice = 'Y' */
                        cfg.errorcode = CfgAction( 'RESETTODEFAULT', f )
                     end
                  else
                     nop
               else
                  if (cfg.file.f.toolbar = 1) & (cfg.attended = 0) & (translate(right(cfg.file.f.name, 13, ' ')) \= '\QUICKTLS.DAT') then
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
         if cfg.attended = 1 then
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

call UpdateArchiverBB2

ErrorExit:
select
   when cfg.errorcode = 0 then
      nop
   when cfg.errorcode = 1 then
      call Usage
   when cfg.errorcode = 2 then
      call Usage
   when cfg.errorcode = 5 then
      do
         say 'Fatal error occurred in updating your ARCHIVER.BB2 file!'
      end
   when cfg.errorcode = 3 then
      call ProgramError
   otherwise
      call ProgramError
end

NormalExit:
signal off notready
call stream cfg.log_file, 'c', 'close'
n = endlocal()
exit cfg.errorcode

/* Subroutines */

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
   cfg.attended      = 1
   cfg.toolbarsonly  = 0
   cfg.actionmethod  = 'COPY'       /* The default (and currently only) method of backing up and restoring */
   cfg.log_file      = 'cfgmgr.log'
   cfg.vio_output_control = '>NUL 2>NUL'

   parse source . . thispgm
   p = lastpos('\', thispgm)
   cfg.this_dir = filespec('D', thispgm) || filespec('P', thispgm)  /* Directory w/ backslash */
   cfg.this_dir = left(cfg.this_dir, length(cfg.this_dir) - 1)      /* Directory w/o backslash */
   if length(cfg.this_dir) = 2 then /* If root dir */
      call directory cfg.this_dir || '\'
   else                            /* not root dir */
      call directory left(cfg.this_dir, length(cfg.this_dir) - 1)

   cfg.backup_dir = cfg.this_dir || '\User_Config_Backup'
   call SysMkDir cfg.backup_dir

   cfg.pgmname  = translate(substr(thispgm, p + 1, lastpos('.', thispgm) - p - 1))

   fm3ini = value('FM3INI',,'OS2ENVIRONMENT')
   if fm3ini = '' then
      cfg.inifile = cfg.this_dir || '\FM3.INI'
   else
      cfg.inifile = fm3ini


   /*   Read from a file instead?   */

   f = 0

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


/*
 * Unused code
 *
   parse value SysTextScreenSize() with . cfg.screen_width
   if cfg.screen_width = 0 then           /* for running in PMREXX */
      cfg.screen_width = 80
*/

return

ProcessArgs: procedure expose (globals)
   parse arg args
   call lineout cfg.log_file, 'Arguments:' args
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
            cfg.attended = 0
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
/*
   else
      if cfg.operation = 'INSTALL' & cfg.defaults = 0 then
         cfg.attended = 0
*/
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
               if GetResponse('Type ''Y'' to proceed, anything else cancels') = 'Y' then
                  cfg.attended = 0
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
               cfg.attended = 0
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
   drop file.
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
   say 'Fatal program error.'
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
                  call CopyFile cfg.file.f.name, cfg.backup_dir
                  call CopyFile cfg.file.f.default, cfg.file.f.name
               end
            /* Implement other archive/restore methods here */
            otherwise
               retval = 4
         end
      when action = 'INSTALLDEFAULT' then
         select
            when cfg.actionmethod = 'COPY' then
               do
                  call CopyFile cfg.file.f.default, cfg.file.f.name
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
                     call CopyFile cfg.backup_dir || '\' || cfg.file.f.name, ' .'
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
                           '@del 'cfg.file.f.name cfg.vio_output_control
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
                  backup_file = cfg.backup_dir || '\' || cfg.file.f.name
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
      do while StateNames \= ''
         parse var StateNames StateName (null) StateNames
         NumDirCnrs = SysIni(cfg.inifile, 'FM/3', StateName || '.NumDirsLastTime')
         if NumDirCnrs \= 'ERROR:' then
            do
               NumDirCnrs = c2d(reverse(NumDirCnrs)) - 1  /* for 0 to num-1 loop */
               do d = 0 to NumDirCnrs
                  d2 = NumDirCnrs + 1 - d /* Work from the last to the first */
                  do f = 1 to NumKeyFragments
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
         'copy' infile cfg.backup_dir         /* backup tls file */
         'copy' outfile infile                   /* "install" new  tls file */
      end
   call SysFileDelete outfile
return

/* Write a message to the log file and, if attended, to the screen */
WriteMsg: procedure expose (globals)
   parse arg msg
   call lineout cfg.log_file, date() time() ':' msg
   if cfg.attended = 1 then
      say msg
return

CopyFile: procedure expose (globals)
   parse arg from_file, to_file
   '@copy' from_file to_file cfg.vio_output_control
return rc

UpdateArchiverBB2: procedure expose (globals)
   myrc = 0

   /* If CFGMGR was called with /DEFAULTS and/or /DEINSTALL parameters
      then there is nothing to do here! */
   if cfg.defaults = 1 | cfg.operation = 'DEINSTALL' then
      return myrc

   bb2_file = cfg.this_dir || '\ARCHIVER.BB2'
   bb2_template_file = cfg.this_dir || '\Tmplates\archiver.tmp'

   bb2_backup_file = cfg.backup_dir || '\Backup.bb2'  /* <== Change this each new archivers are to be inserted */

   myrc = CopyFile(bb2_file, bb2_backup_file)
   if myrc \= 0 then
      do
         if cfg.attended = 1 then
            do
               say
               call WriteMsg 'Error: Unable to back up' filespec('n', bb2_file) || '.'
               say
               if cfg.defaults = 1 then
                  msgend = 'reverting to a default FM/2 ARCHIVER.BB2.'
               else
                  msgend = 'appending new archiver definitions to' filespec('n', bb2_file) || '.'
               call WriteMsg "Enter 'P' to proceed with" msgend ', or...'
               user_option = GetResponse("Enter 'Q' to abort" cfg.pgmname '(P/q)')
            end
         else
            user_option = 'P'
      end
   else
      user_option = 'P'
   if user_option = 'P' then
      do
         if cfg.attended = 1 & myrc \= 0 then
            do
               say
               call WriteMsg 'Appending new archiver definitions to' filespec('n', bb2_file) || '.'
               say
               user_option = GetResponse("OK to proceed? (Y/n)")
            end
         else
            user_option = 'Y'
         if user_option = 'Y' then
            do
               myrc = InsertNewArchivers(bb2_file, bb2_template_file)
               select
                  when myrc < 0 then
                     do
                        if cfg.attended = 1 then
                           do
                              say
                              call WriteMsg filespec('n', bb2_file) 'already has definitions for the new archivers.'
                           end
                        myrc = 0
                     end
                  when myrc = 0 then
                     if cfg.attended = 1 then
                        do
                           say
                           call WriteMsg filespec('n', bb2_file) 'successfully updated with new archivers definitions.'
                        end
                  otherwise
                     do
                        if cfg.attended = 1 then
                           do
                              say
                              call WriteMsg 'Error: Unable to update' filespec('n', bb2_file) 'with new archiver definitions.'
                              say
                              call WriteMsg 'Current file continues in use.'
                              say
                              call WriteMsg "Enter 'P' to proceed with other updates or..."
                              user_option = GetResponse("Enter 'Q' to abort" cfg.pgmname '(P/q)')
                              if user_option = 'Q' then
                                 cfg.userabort = 1
                           end
                     end
               end
            end
      end /* Install, append new */
   else /* Backup not OK, no proceed */
      cfg.userabort = 1
return (5*(myrc \= 0))  /* Convert any non-zero into a 5 */

InsertNewArchivers: procedure expose(globals)
   parse arg bb2_file, bb2_template_file
   myrc = 0

   files.1.name = bb2_file
   files.2.name = bb2_template_file
   files.0 = 2

   do i = 1 to files.0
      j = 0
      do while lines(files.i.name) > 0
         j = j + 1
         files.i.lines.j = linein(files.i.name)
      end
      files.i.lines.0 = j
      call stream files.i.name, 'c', 'close'
   end

   zip6_liststart_line = '--------  ------  ------- ---- ---------- ----- --------  ----'
   zip6_listend_line =   '--------          -------  ---                            -------'

   markers. = ''
   i = 0
   i = i + 1
   markers.i.name = 'Zip v6.0'
   markers.i.markerline_start = zip6_listend_line
   markers.i.markerline_pos_in_def = 16
   markers.i.pos_of_def_in_file.1 = 0
   markers.i.pos_of_def_in_file.2 = 0
   i = i + 1
   markers.i.name = '7 Zip'
   markers.i.markerline_start = '7ZA.EXE'
   markers.i.markerline_pos_in_def   = 4
   markers.i.pos_of_def_in_file.1 = 0
   markers.i.pos_of_def_in_file.2 = 0
   i = i + 1
   markers.i.name = 'LZip'
   markers.i.markerline_start = 'LZIP.EXE'
   markers.i.markerline_pos_in_def   = 4
   markers.i.pos_of_def_in_file.1 = 0
   markers.i.pos_of_def_in_file.2 = 0
   i = i + 1
   markers.i.name = 'XC'
   markers.i.markerline_start = 'XC.EXE'
   markers.i.markerline_pos_in_def   = 4
   markers.i.pos_of_def_in_file.1 = 0
   markers.i.pos_of_def_in_file.2 = 0

   markers.0 = i - 1 /* test: skip xc for now */

   do i = 1 to files.0
      in_def = 0
      do j = 1 to files.i.lines.0
         if left(files.i.lines.j, 1) = ';' then
            do
               if in_def = 1 then in_def = 0
               iterate
            end
         else
            if in_def = 0 then
               if strip(files.i.lines.j) = '' then  /* 1st line of def can't be blank? */
                  iterate
               else
                  do
                     in_def = 1
                     def_line = 1
                     last_def.i = j
                  end
            else
               def_line = def_line + 1
         do m = 1 to markers.0
            if markers.m.pos_of_def_in_file.i = 0 then
               if markers.m.markerline_pos_in_def = def_line then
                  do /* Check for marker line here */
                     if abbrev(translate(files.i.lines.j), markers.m.markerline_start) = 1 then
                        do
                           if markers.m.name \= 'Zip v6.0' then
                              do
                                 markers.m.pos_of_def_in_file.i = j - markers.m.markerline_pos_in_def + 1
/*
                                 call WriteMsg 'Found' markers.m.name 'definition in' filespec('n', files.i.name) || '.'
*/
                              end
                           else
                              do /* special code for zip 6 with 2 marker lines */
                                 q = j - 1
                                 if abbrev(files.i.lines.q, zip6_liststart_line) = 1 then
                                    do
                                       markers.m.pos_of_def_in_file.i = j - markers.m.markerline_pos_in_def + 1
/*
                                       call WriteMsg 'Found' markers.m.name 'definition in' filespec('n', files.i.name) || '.'
*/
                                    end
                              end
                        end
                  end
         end
      end
   end
   end_of_archivers_line = last_def.1 + 21

   temp_file = SysTempFilename(cfg.this_dir || '\bb2temp.???')
   new_data_written = 0
   do j = 1 to end_of_archivers_line
      call lineout temp_file, files.1.lines.j
   end
   do m = 1 to markers.0
      if markers.m.pos_of_def_in_file.1 = 0 then
/*
   Don't bother checking to see if archiver.tmp has new defs. Just assume this.
         if markers.m.pos_of_def_in_file.2 = 0 then
            do
               call WriteMsg 'Definition for' markers.m.name 'not found in either file!!'
            end
         else
*/
            do
               call WriteMsg 'Inserting definition for' markers.m.name
               start_at = markers.m.pos_of_def_in_file.2 - 3
               end_at = start_at + 21 + 4 - 1
               do q = start_at to end_at
                  call lineout temp_file, files.2.lines.q
               end
               new_data_written = 1
            end
/*
   Debug code
      else
         if markers.m.pos_of_def_in_file.2 = 0 then
            do
               call WriteMsg 'Definition for' markers.m.name 'not found in' filespec('n', files.2.name) || '!!'
            end
         else
            do
               call WriteMsg markers.m.name 'found in' files.1.name 'and' files.2.name
            end
*/
   end
   if new_data_written = 1 then
      do
         do q = j to files.1.lines.0   /* "j" is from the earlier loop */
            call lineout temp_file, files.1.lines.q
         end
         call stream temp_file, 'c', 'close'
         call SysFileDelete files.1.name
         myrc = CopyFile( temp_file, files.1.name )
      end
   else
      do /* No need to update ARCHIVER.BB2 */
         myrc = -1
         call stream temp_file, 'c', 'close'
      end
   call SysFileDelete temp_file
return myrc

