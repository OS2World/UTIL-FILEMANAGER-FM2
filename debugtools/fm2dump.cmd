@echo off
:: $Id$

:: Prepare for fm/2 process dump
:: Run script as fm2dump ? for usage help
:: Cmd.exe compatible
:: Search for "Edit this" markers to find lines that may need site specific edits

:: Copyright (c) 2002, 2007 Steven Levine and Associates, Inc.
:: All rights reserved.

:: This program is free software licensed under the terms of the GNU
:: General Public License.  The GPL Software License can be found in
:: gnugpl2.txt or at http://www.gnu.org/licenses/licenses.html#GPL

:: 22 Oct 02 SHL Baseline
:: 02 Dec 03 SHL Comments and usage help
:: 12 Apr 04 SHL Rework settings
:: 09 Aug 05 SHL Add some more system data
:: 11 Aug 05 SHL Comments
:: 24 Mar 07 SHL Write dump file to %TMP%
:: 14 Jul 07 SHL Add options, sync with pdumpctl
:: 14 Sep 07 SHL Sync with current standards

:: Version 0.3

setlocal

:: Edit this to point to existing directory on drive with sufficient free space
set D=U:\Dump

:: Try to validate dump directory
dir %D%\nul >nul 2>&1
if not errorlevel 1 goto DirOK
  echo Directory %D% does not exist - check set D= statement on line 26
  goto end
:dirok

:: Edit this to name process to be dumped - .exe is optional
set P=fm3

if "%1" == "" goto Help

:next

if "%1" == "a" goto All
if "%1" == "A" goto All
if "%1" == "c" goto Configure
if "%1" == "C" goto Configure
if "%1" == "f" goto Force
if "%1" == "F" goto Force
if "%1" == "n" goto TurnOn
if "%1" == "N" goto TurnOn
if "%1" == "q" goto Query
if "%1" == "Q" goto Query
if "%1" == "o" goto TurnOff
if "%1" == "O" goto TurnOff
if "%1" == "r" goto Reset
if "%1" == "R" goto Reset

if "%1" == "?" goto Help

goto Usage

:: Configure to dump all memory

:All
  echo on
  pdumpusr reset
  @if errorlevel 1 pause
  pdumpusr paddr(all)
  @if errorlevel 1 pause
  :: Check
  procdump query
  @if errorlevel 1 pause
  @echo off
  echo Dump facility configured to dump all memory
  goto shift

:: Configure optimal dump settings for fm/2

:Configure
  :: Turn on dump facility - set dump directory
  procdump on /l:%D%
  @if errorlevel 1 pause
  :: Configure settings
  procdump set /proc:%P% /pd:instance,private,shared,sysfs,sysio,sysldr,syssem,syssumm,systk,sysvm /pc:0
  @if errorlevel 1 pause
  @echo off
  echo.
  echo Dump facility configured to dump %P% to %D%
  goto shift

:: Force dump with current settings

:Force
  echo on
  procdump force /proc:%P%
  @echo off
  echo Forced dump for process %P%
  goto shift

:: Query current settings

:Query
  echo.
  echo on
  procdump query
  @if errorlevel 1 pause
  @echo off
  goto shift

:: Reset to defaults

:Reset
  echo on
  procdump reset /f /l
  @if errorlevel 1 pause
  @echo off
  echo.
  echo Dump facility reset to default settings
  goto shift

:TurnOff
  echo on
  procdump reset /l
  @if errorlevel 1 pause
  procdump reset /pid:all
  @if errorlevel 1 pause
  procdump off
  @if errorlevel 1 pause
  @echo off
  echo.
  echo Dump facility turned off
  goto shift

:: Turn on dump facility - set dump directory

:TurnOn
  echo on
  procdump on /l:%D%
  @if errorlevel 1 pause
  @echo off
  echo.
  echo Dump facility turned on
  goto shift

:shift

  shift
  if not "%1" == "" goto next
  goto end

::=== Usese: Report usage error ===

:Usage
  echo.
  echo Usage: fm2dump [a] [c] [f] [n] [o] [p] [q] [r]
  echo Use ? to get detailed help
  goto end

::=== Help: Show usage help ===

:Help
  echo.
  echo Simple Dump Facility controller for fm/2
  echo.
  echo Usage: fm2dump [a] [c] [f] [n] [o] [p] [q] [r]
  echo.
  echo   a     Configure to dump all memory
  echo   c     Configure optimally for fm/2 dump
  echo   f     Force dump using current settings
  echo   n     Turn on dump facility
  echo   o     Turn off dump facility
  echo   q     Query current settings
  echo   r     Reset to default settings
  echo   q     Query current settings
  echo   ?     Display this message
  echo.
  echo   Requests are processed left to right
  echo   Errors will pause script
  echo   Edit line 26 to match your system
  echo   Do not forget to turn off dump facility when done
  echo.  Typical use cases
  echo     fm2dump c
  echo     fm2dump c f o
  echo     fm2dump r o

:end
