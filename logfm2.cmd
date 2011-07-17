/*
 * $Id$
 *
 * Run FM/2 with logging enabled
 *
 * Change log:
 *    11 Jul 17 JBS Corrected redirection syntax
 *
 */
	
parse arg logfile
fulllogfilename = stream(logfile, 'c', 'query exists')
if fulllogfilename = '' then
   fulllogfilename = logfile
call lineout fulllogfilename, date() time() 'Logging started...'
call directory '..'
'start fm3.exe 1>>'fulllogfilename '2>&1'
return
