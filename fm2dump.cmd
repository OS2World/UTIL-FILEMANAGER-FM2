@echo off
:: fm2dump - Prepare for fm/2 process dump
:: $Id$

:: Run as fm2dump ? for usage help

:: Copyright (c) 2002, 2003 Steven Levine and Associates, Inc.
:: All rights reserved.

:: This program is free software licensed under the terms of the GNU
:: General Public License.  The GPL Software License can be found in
:: gnugpl2.txt or at http://www.gnu.org/licenses/licenses.html#GPL


:: Revisions	22 Oct 02 SHL - Baseline
::		02 Dec 03 SHL - Comments and usage help

setlocal

:: Edit this to point to existing directory on drive with sufficient free space
set D=j:\tmp\dumpsx

:: Try to validate
dir %D%\nul >nul 2>&1
if errorlevel 1 goto BadDir

:: Edit this to name process to be dumped
set P=fm3

if not "%2" == "" goto Help
if "%1" == "o" goto TurnOff
if "%1" == "O" goto TurnOff
if "%1" == "p" goto Configure
if "%1" == "P" goto Configure
if "%1" == "" goto Reset
goto Help

:: Reset and set dump directory

:Reset
echo on
:: Reset to defaults
procdump reset /l
@if errorlevel 1 pause
procdump reset /pid:all
@if errorlevel 1 pause
:: Turn on dump facility - set dump directory
procdump on /l:%d%
@if errorlevel 1 pause

:: Configure dump settings for fm/2

:Configure

procdump set /proc:%P% /pd:summ,sysfs,private,instance,shared,syssem,sysio /pc:0 /pu
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
procdump reset /l
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

:BadDir
  echo %D% does not exist.  Check set D= statement
  goto end

::=== Help: Show usage help ===

:Help
  echo.
  echo Usage: fm2dump [o] [p]
  echo.
  echo   o     Turn off dump facility
  echo   p     Retain personal dump settings
  echo   ?     This message`
  echo.
  echo   Only one arg alllowed
  echo   Default is to clear personal settings and configure fm/2 specfic settings

:end
