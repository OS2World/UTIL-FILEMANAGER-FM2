@echo off
:: fm2dump2 - Prepare for fm/2 paddr(all) process dump
:: $Id: $

:: Search for "EDITME" markers to find lines that may need edits
:: Run script as fm2dump ? for usage help

:: Copyright (c) 2002, 2005 Steven Levine and Associates, Inc.
:: All rights reserved.

:: This program is free software licensed under the terms of the GNU
:: General Public License.  The GPL Software License can be found in
:: gnugpl2.txt or at http://www.gnu.org/licenses/licenses.html#GPL

:: 05 Mar 07 SHL Adapt from fm2dump.cmd
:: 24 Mar 07 SHL Write dump file to %TMP%

setlocal

:: EDITME to name dump directory - directory must exist and drive must have sufficient free space
set D=%TMP%

:: Try to validate
dir %D%\nul >nul 2>&1
if not errorlevel 1 goto DirOK
  echo Directory %D% does not exist - check set D= statement
  goto end
:DirOK

:: EDITME to name process to be dumped - .exe is optional
set P=fm3

:: Warn if not visible just by name - OK if in PATH
if exist %P%.exe goto ExeOK
if exist %P% goto ExeOK
  echo Warning: %P% not found
:ExeOK

if not "%2" == "" goto Help
if "%1" == "o" goto TurnOff
if "%1" == "O" goto TurnOff
if "%1" == "r" goto Reset
if "%1" == "R" goto Reset
if "%1" == "" goto Configure
goto Help

:: Reset to defaults

:Reset
echo on
:: Reset directory and freespace limits to defaults
procdump reset /f /l
@if errorlevel 1 pause
procdump reset /pid:all
@if errorlevel 1 pause

:: Configure fm/2 specific dump settings

:Configure

:: Turn on dump facility - set dump directory
procdump on /l:%D%
@if errorlevel 1 pause
:: Configure fm/2 dump
procdump set /proc:%P% /pd:paddr(all)
@if errorlevel 1 pause
:: Check
procdump query
@if errorlevel 1 pause
@echo off
echo Dump facility configured to dump %P% to %D%

goto end

:TurnOff

echo on
:: Reset to defaults
procdump reset /f /l
@if errorlevel 1 pause
procdump reset /pid:all
@if errorlevel 1 pause
:: Turn off
procdump off
@if errorlevel 1 pause
procdump query
@if errorlevel 1 pause
@echo off
echo Dump facility turned off
goto end

::=== Help: Show usage help ===

:Help
  echo.
  echo Usage: fm2dump [o] [r]
  echo.
  echo   o     Turn off dump facility
  echo   r     Reset to defaults then configure
  echo   ?     This message`
  echo.
  echo   Only one arg alllowed
  echo   Default is retain current settings and configure fm/2 specfic settings

:end
