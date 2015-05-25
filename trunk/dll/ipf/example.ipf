
:h3 res=100080 id='PANEL_REXAMPLE'. Sample REXX File
:color fc=default bc=green.
&slash. &asterisk. Example of how a rexx file can operate on a list file created by FM&slash.2 &lpar.the list file should contain filenames only&comma.
one per line&rpar.&per. This example can be editted at the boxed comment area below and
used with FM&slash.2 Commands containing the &percent.&xclm.metastring&per. Call as "&percent.c &slash.C&lt.drive&colon.&bsl.path&bsl.&gt.EXAMPLE&per.CMD &percent.&xclm." &lpar.exclude
quotes&rpar. &lpar.for example&colon.  &percent.c &slash.C E&colon.&bsl.fm2&bsl.EXAMPLES&per.CMD &percent.&xclm.&rpar.
&asterisk.&slash.:color fc=default bc=default.

.br

:color fc=default bc=green.:p.&slash.&asterisk. suppress echo from "batch" commands &asterisk.&slash.
:color fc=default bc=default.
.br
&apos.&atsign.Echo off&apos.
:p.
:color fc=default bc=green.&slash.&asterisk. clear screen &lpar.GPs&rpar.&per. &asterisk.&slash.
:color fc=default bc=default.
.br
&apos.&atsign.cls&apos.
:p.
:color fc=default bc=green.&slash.&asterisk. get name of listfile from command line&per.&asterisk.&slash.:color fc=default bc=default.
.br
parse arg listfile
:p.
:color fc=default bc=green.&slash.&asterisk. if no listfile name was given&comma. issue help and exit&per. &asterisk.&slash.
:color fc=default bc=default.
.br
if listfile &eq. &apos.&apos. then
.br
do
.br
  say &apos.Give the name of a listfile as an argument to this REXX
script&per.&apos.
.br
  exit
.br
end
:p.
:color fc=default bc=green.&slash.&asterisk. for debugging purposes&colon. &asterisk.&slash.
:color fc=default bc=default.
.br
say &apos.Name of our listfile is "&apos.listfile&apos."&per.&apos.
:p.
:color fc=default bc=green.&slash.&asterisk. see if the listfile given exists &endash.&endash. exit with
error message if not &asterisk.&slash.:color fc=default bc=default.
.br
rc &eq. stream&lpar.listfile&comma.&apos.C&apos.&comma.&apos.QUERY
EXISTS&apos.&rpar.
.br
if rc &eq. &apos.&apos. then
.br
do
.br
  say &apos.File "&apos.listfile&apos." doesn&apos.&apos.t exist&per.&apos.
.br
  exit
.br
end
:p.
:color fc=default bc=green.&slash.&asterisk. attempt to open the listfile given on the command line
&asterisk.&slash.:color fc=default bc=default.
.br
rc &eq. stream&lpar.listfile&comma.&apos.C&apos.&comma.&apos.OPEN&apos.&rpar.
:p.
:color fc=default bc=green.&slash.&asterisk. if open was successful&comma. enter loop &asterisk.&slash.
:color fc=default bc=default.
.br
if rc &eq. &apos.READY&colon.&apos. then
.br
do
.br
  counter &eq. 0    :color fc=default bc=green.&slash.&asterisk. initialize counter &lpar.debugging
aid&rpar. &asterisk.&slash.
:p.
&slash.&asterisk. read each line of the listfile into filename
&asterisk.&slash.:color fc=default bc=default.
.br
  do while lines&lpar.listfile&rpar. &eq. 1
.br
    filename &eq. linein&lpar.listfile&rpar.
:p.:color fc=default bc=green.&slash.&asterisk. remove any leading&slash.trailing blanks
&asterisk.&slash.:color fc=default bc=default.
.br
    filename &eq. strip&lpar.filename&comma.&apos.b&apos.&rpar.
:p.:color fc=default bc=green.&slash.&asterisk. process only non&endash.blank strings &asterisk.&slash.
:color fc=default bc=default.
.br
    if filename &bsl.&eq. &apos.&apos. then
.br
    do
.br

.br
:color fc=default bc=green.&slash.&asterisk. here you would do something to&slash.with the file in
filename&per. since this is only an example&comma. we&apos.ll just print the
name&per. Note that you could do most anything to the file here
&endash.&endash.  use your imagination&per.&asterisk.&slash.:color fc=default bc=default.
.br

:p.      say filename  :color fc=default bc=green.&slash.&asterisk.replace with your
command&lpar.s&rpar.&xclm.&asterisk.&slash.
:p.
&slash.&asterisk.end of area where you&apos.d do your special processing&per.&asterisk.&slash.:color fc=default bc=default.
.br

.br
      counter &eq. counter &plus. 1  :color fc=default bc=green.&slash.&asterisk. count files processed for
debugging&per. &asterisk.&slash.:color fc=default bc=default.
.br
    end
.br
  end
:p.:color fc=default bc=green.&slash.&asterisk. close the listfile&per. &asterisk.&slash.:color fc=default bc=default.
.br
  rc &eq. stream&lpar.listfile&comma.&apos.C&apos.&comma.&apos.CLOSE&apos.&rpar.
:p.:color fc=default bc=green.&slash.&asterisk. remove the listfile &endash.&endash. checks to disallow
wildcards in name &lpar.GPs&rpar.&per. &asterisk.&slash.:color fc=default bc=default.
.br
  if &lpar.pos&lpar.&apos.&asterisk.&apos.&comma.listfile&rpar. &eq. 0&rpar.
&amp. &lpar.pos&lpar.&apos.?&apos.&comma.listfile&rpar. &eq. 0&rpar. then
.br
  do
.br
    &apos.del "&apos.listfile&apos." 1&gt.NUL 2&gt.NUL&apos.
.br
  end
:p.end
.br
else :color fc=default bc=green.&slash.&asterisk. couldn&apos.t open listfile &asterisk.&slash.:color fc=default bc=default.
.br
do
.br
  say &apos.Error opening "&apos.listfile&apos."&per.&apos.
.br
  exit
.br
end
:p.:color fc=default bc=green.&slash.&asterisk. we&apos.re done &endash.&endash. issue count for
debugging&per. &asterisk.&slash.:color fc=default bc=default. .
.br
say &apos.   &asterisk.&asterisk.I processed &apos.counter&apos.
objects&per.&apos.
.br

