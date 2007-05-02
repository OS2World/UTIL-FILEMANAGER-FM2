/* ShadowMgr: Create Shadows for FM/2 installation */
call RxFuncAdd 'SysLoadFuncs', 'REXXUTIL', 'SysLoadFuncs'
call SysLoadFuncs

parse upper arg package_parm .
if strip(package_parm) == '' then
   package_parm = 'FM2'
else
   if wordpos(package_parm, 'FM2 FM2UTILS') = 0 then
      do
         say 'Invalid parameter: 'arg(1)
         say 'Program aborted....'
         '@pause'
         exit 1
      end

curdir = directory()

i = 0

i = i + 1
Shadow.i.package  = 'FM2'
Shadow.i.folderid = '<FM3_Docs>'
Shadow.i.filename = '.\docs\readme'
Shadow.i.longname = 'Read Me'

i = i + 1
Shadow.i.package  = 'FM2'
Shadow.i.folderid = '<FM3_Docs>'
Shadow.i.filename = '.\docs\copying'
Shadow.i.longname = 'GNU General Public License'

i = i + 1
Shadow.i.package  = 'FM2'
Shadow.i.folderid = '<FM3_Docs>'
Shadow.i.filename = '.\docs\history'
Shadow.i.longname = 'History'

i = i + 1
Shadow.i.package  = 'FM2'
Shadow.i.folderid = '<FM3_Docs>'
Shadow.i.filename = '.\docs\customizingresources.txt'
Shadow.i.longname = 'Customizing^Resources'

i = i + 1
Shadow.i.package  = 'FM2UTILS'
Shadow.i.folderid = '<FM2_Utilities>'
Shadow.i.filename = '.\fm2utils.doc'
Shadow.i.longname = 'FM/2 Utilities Read Me'

Shadow.0 = i

do i = 1 to Shadow.0
   if package_parm = Shadow.i.package then
      do
         fullfilename = stream(curdir || '\' || Shadow.i.filename, 'c', 'query exists')
         if fullfilename = '' then
            do
               say 'Unable to find file: 'Shadow.i.filename
               say 'curdir: 'curdir
               '@pause'
               exit 1
            end
         lenbytes = X2C( D2X( LENGTH( Shadow.i.longname ), 4 ))
         rc = SysPutEA( fullfilename, '.LONGNAME', X2C('FDFF') || REVERSE( lenbytes ) || Shadow.i.longname )
         if rc \= 0 then
            do
               say 'Error in setting .LONGNAME EA: 'rc
               say 'Exiting...'
               '@pause'
               exit 2
            end
         p = lastpos('\', Shadow.i.filename)
         if p > 0 then
            shadowidname = substr(Shadow.i.filename, p + 1)
         else
            shadowidname = Shadow.i.filename
         rc = SysCreateObject( 'WPShadow', Shadow.i.longname, Shadow.i.folderid , 'SHADOWID='fullfilename';OBJECTID=<FM3_Shadow_'shadowidname'>;', 'R')
         if rc \= 1 then
            do
               say 'Unable to create shadow object for :'Shadow.i.filename
               say 'Exiting...'
               '@pause'
               exit 3
            end
      end
end

