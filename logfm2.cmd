/*
 * $Id$
 *
 * Run FM/2 with logging enabled
 *
 * Change log:
 *    17 Jul 11 JBS Corrected redirection syntax
 *    17 Feb 14 JBS Ticket 512: Improvements
 *        Enhanced error-handling
 *           No log file parameter defaults to fm2.log
 *           Checks for write-ability of the log file and prompts for action if unwritable
 *        Other improvements
 *           All logs are written to the <fm2dir>\debug\logfiles directory
 *             (so they are easy to find and do not clutter other directories)
 *           Headers and footers are written to the log file to aid in interpreting
 *             the contents
 *           When FM/2 closes the log file is opened
 *
 */

/* Determine and ensure existence of log file directory */
parse source . . thispgm
logfiledir = filespec('D', thispgm) || filespec('P', thispgm) || 'LogFiles'
call SysMkDir logfiledir

/* Process parameter, using default if none is passed. */
parse arg logfile
logfile = strip(logfile)
if logfile = '' then
  logfile = 'fm2.log'         /* Default to "fm2.log" */

/* Check "write-ablility" and set the full filename for the log file */
do until logfile_ok = 1
   fulllogfilename = logfiledir || '\' || filespec('N', logfile)
   logfile_exists = (stream(fulllogfilename, 'c', 'query exists') \= '')
   /* Check for locked file here */
   if stream(fulllogfilename, 'c', 'open write') = 'READY:' then
      logfile_ok = 1
   else
      do
         logfile_ok = 0
         rc = stream(fulllogfilename, 'c', 'close')
         say 'Unable to open existing log file:' logfile
         say
         say logfile 'may be locked by another process.'
         say
         say 'Please close any processes that may be using' logfile || '.'
         say '(Another instance of a logged FM/2 or an editor perhaps?)'
         say
         say 'Just press the Enter key to retry' logfile || '.'
         say 'Or type in a new log file name:'
         newlogfile = strip(linein())
         if newlogfile \= '' then
            logfile = newlogfile
      end
end

/* Write "header" */
if logfile_exists = 1 then
   do
      call lineout fulllogfilename, LinePrefix() 'Created log file:' fulllogfilename
      /* Write "instructions" to file here? */
   end
call lineout fulllogfilename, LinePrefix() 'Logging started.'
call stream fulllogfilename, 'c', 'close'

/* Start FM/2 */
call directory logfiledir || '\..\..'
'fm3.exe # 2>>' || fulllogfilename

/* Write "footer" */
call lineout fulllogfilename,  LinePrefix() 'Logging ended.'
call stream fulllogfilename, 'c', 'close'

/* Open log file for viewing */
call SysSetObjectData fulllogfilename, 'OPEN=DEFAULT'
return

LinePrefix: procedure
return '###' date() time()

