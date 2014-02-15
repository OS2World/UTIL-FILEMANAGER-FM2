/*
 * $Id$
 *
 * ShdwMgr: Create Shadows for FM/2 installation
 *
 * 14 Feb 14  JBS Ticket 501: Corrected a syntax error and improved the error handling.
 *                            Errors are now logged to a file.
 *
 */

call RxFuncAdd 'SysLoadFuncs', 'REXXUTIL', 'SysLoadFuncs'
call SysLoadFuncs

parse source . . thispgm
thisdir = left(thispgm, lastpos('\', thispgm) - 1)
if length(thisdir) = 2 then
   thisdir = thisdir || '\'
call directory thisdir
parse value filespec('N', thispgm) with program_stem '.' .

parse upper arg package_parm .
if strip(package_parm) == '' then
   package_parm = 'FM2'
pkgnum = wordpos(package_parm, 'FM2 FM2UTILS')

error_log = program_stem || pkgnum || '.err'
call SysFileDelete error_log              /* Delete previous error logs for this package */
errors = 0

if pkgnum = 0 then
   do
      call lineout error_log, 'Invalid parameter:' package_parm
      call lineout error_log, 'Program aborted....'
      exit 1
   end

i = 0

i = i + 1
Shadow.i.package  = 'FM2'
Shadow.i.folderid = '<FM3_Docs>'
Shadow.i.filename = '..\docs\readme'
Shadow.i.longname = 'Read Me'

i = i + 1
Shadow.i.package  = 'FM2'
Shadow.i.folderid = '<FM3_Docs>'
Shadow.i.filename = '..\docs\copying'
Shadow.i.longname = 'GNU General Public License'

i = i + 1
Shadow.i.package  = 'FM2'
Shadow.i.folderid = '<FM3_Docs>'
Shadow.i.filename = '..\docs\history'
Shadow.i.longname = 'History'

i = i + 1
Shadow.i.package  = 'FM2UTILS'
Shadow.i.folderid = '<FM2_Utilities>'
Shadow.i.filename = '.\fm2utils.doc'
Shadow.i.longname = 'FM/2 Utilities Read Me'

Shadow.0 = i

do i = 1 to Shadow.0
   if package_parm = Shadow.i.package then
      do
         fullfilename = stream(thisdir || '\' || Shadow.i.filename, 'c', 'query exists')
         if fullfilename = '' then
            do
               call lineout error_log, 'Unable to find file: 'Shadow.i.filename
               errors = errors + 1
            end
         else
            do
               lenbytes = X2C( D2X( LENGTH( Shadow.i.longname ), 4 ))
               rc = SysPutEA( fullfilename, '.LONGNAME', X2C('FDFF') || REVERSE( lenbytes ) || Shadow.i.longname )
               if rc \= 0 then
                  do
                     call lineout error_log, 'Error in setting .LONGNAME EA: 'rc
                     errors = errors + 1
                  end

               p = lastpos('\', Shadow.i.filename)
               if p > 0 then
                  shadowidname = substr(Shadow.i.filename, p + 1)
               else
                  shadowidname = Shadow.i.filename
               rc = SysCreateObject( 'WPShadow', Shadow.i.longname, Shadow.i.folderid , 'SHADOWID='fullfilename';OBJECTID=<FM3_Shadow_'shadowidname'>;', 'R')
               if rc \= 1 then
                  do
                     call lineout error_log, 'Unable to create shadow object for :'Shadow.i.filename
                     errors = errors + 1
                  end
            end
      end
end
if errors > 0 then
   do
      call lineout error_log, errors 'errors encountered.'
      call stream error_log, 'c', 'close'
   end

