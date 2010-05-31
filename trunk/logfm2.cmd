/*
	Run FM/2 with logging enabled
*/
parse arg logfile
fulllogfilename = stream(logfile, 'c', 'query exists')
if fulllogfilename = '' then
   fulllogfilename = logfile
call lineout fulllogfilename, date() time() 'Logging started...'
call directory '..'
'start fm3.exe 2>>&1 1>>'fulllogfilename
return
