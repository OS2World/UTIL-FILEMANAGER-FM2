# warpin\makefile - build the FM/2 Warpin installation file: FM2$(FM2_VER).WPI

# $Id$

# 28 Apr 07 SHL Add clean target
# 13 May 07 SHL Tweak macro handling to work as documented
# 13 May 07 SHL Correct clean target
# 08 Jun 07 JBS Corrected BUILD_FM2UTILS code so that it is actually
#               possible to disable the build of the FM2UTILS*.WPI file
# 22 Jun 07 JBS Reworked the code related to macros in both this file
#               and the base makefile. Also made the fm2.wis file a
#               dependant of the WPI target.
# 10 Dec 07 SHL Add missing makefile dependencies
# 10 Dec 07 SHL Set FM2_VER for 3.9.0
# 07 Oct 08 JBS Changed SED command to use script file and to eliminate
#               extraneous blank lines with the included files.
# 23 Oct 08 JBS Ticket 293: Add "chksed" target to check for sufficient SED and GREP and
#               improve support for -a option in the checkfiles target.
# 30 Oct 08 JBS Code improvements, including condensed script-related code and
#               eliminated occasional duplicate addition of the script to the WPI file
# 01 Dec 08 JBS SED.IN is now rewritten (via WMAKE's write/append) every time the Warpin
#               script needs to be built. This allows the version strings to be included
#               in SED.IN which, in turn, means the SED command line is much simplified.
#               Therefore SED.IN has now been removed from the SVN repository.
# 20 Sep 09 SHL Set FM2_VER for v3.17

!ifndef FM2_VER                  # if defined on wmake command, use it
FM2_VER=-3-18-0                  # default value
!ifdef %FM2_VER                  # if defined via env. var.
FM2_VER=$(%FM2_VER)              #     use the env. var.
!endif
!endif

# FM2UTILS_VER defines the fm2utils WPI file name suffix.
# e.g. FM2UTILS_VER=-1-0 results in FM2Utils-1.0.wpi being built
# If FM2UTILS_VER is empty, then FM2UTILS.wpi is built
# NOTE: Start the variable with '-'

!ifndef FM2UTILS_VER             # if defined on wmake command, use it
FM2UTILS_VER=-1-1                # default value
!ifdef %FM2UTILS_VER             # if defined via env. var.
FM2UTILS_VER=$(%FM2UTILS_VER)    #     use the env. var.
!endif
!endif

# If BUILD_FM2UTILS = 1, build FM2UTILS*.wpi and FM2*.wpi
# Otherwise build just FM2*.wpi

!ifndef BUILD_FM2UTILS           # if defined on wmake command, use it
!ifdef %BUILD_FM2UTILS           # else if defined via env. var.
!ifneq %BUILD_FM2UTILS 1         #     if env. var. is anything but 1
BUILD_FM2UTILS=0                 #     use a value of 0
!else
BUILD_FM2UTILS=1
!endif
!else
BUILD_FM2UTILS=0                 # use default value if not defined via env. or command line
!endif
!endif


all: wpi .symbolic

!ifeq BUILD_FM2UTILS 0
wpi: chksed checkfiles fm2$(FM2_VER).wpi fm2.wis .symbolic
!else
wpi: chksed checkfiles fm2$(FM2_VER).wpi fm2.wis fm2utils$(FM2UTILS_VER).wpi .symbolic
!endif

checkfiles: .symbolic
   @echo Checking to see if files have been changed...
    ckfiles "MAKEOPTS=$(__MAKEOPTS__)" fm2$(FM2_VER).wpi

chksed: .symbolic
   @sed -V <nul 2>&1 | grep -i "GNU sed" >nul 2>&1 || (@echo. & @echo GNU SED and GREP required and not found on the PATH. & @echo.)

fm2.wis: fm2.wis.in fm2utils\fm2utils.doc fm2utils\license.txt makefile
   @echo Resetting SED commands
   @%write  sed.in s/@@@FM2_VER@@@/$(FM2_VER)/
   @%append sed.in s/@@@FM2UTILS_VER@@@/$(FM2UTILS_VER)/
   @%append sed.in /@@@include fm2utils.doc@@@/{
   @%append sed.in s/@@@include fm2utils.doc@@@/@@@include/
   @%append sed.in r fm2utils\fm2utils.doc
   @%append sed.in }
   @%append sed.in /@@@include license.txt@@@/{
   @%append sed.in s/@@@include license.txt@@@/@@@include/
   @%append sed.in r fm2utils\license.txt
   @%append sed.in }
   @%append sed.in /PACKAGEID/s/-/\\/g
   @%append sed.in /@@@include/{s/@@@include//;N;s/\n//}
   @echo Inserting the new Warpin script into the WPI file...
   sed -f sed.in fm2.wis.in >fm2.wis
   bld_fm2_wpi fm2$(FM2_VER).wpi /SCRIPT

fm2$(FM2_VER).wpi: bld_fm2_wpidirs.in makefile
   @echo Staging files for WPI file...
   bld_fm2_wpidirs
   @echo Creating/updating the WPI file
   bld_fm2_wpi fm2$(FM2_VER).wpi

fm2utils$(FM2UTILS_VER).wpi: bld_fm2utils_wpi.cmd fm2utils.wis fm2utils\fm2utils.doc fm2utils\license.txt ..\shdwmgr.cmd makefile
   @echo Building FM2Utils
   bld_fm2utils_wpi fm2utils$(FM2UTILS_VER).wpi

clean: .symbolic
  -if exist fm2$(FM2_VER).wpi del fm2$(FM2_VER).wpi
  -if exist fm2utils$(FM2UTILS_VER).wpi del fm2utils$(FM2UTILS_VER).wpi
  -if exist bld_fm2_wpidirs.in del bld_fm2_wpidirs.in
  clean_wpidirs

# fixme to decide if these should be cleaned and not distcleaned
distclean: clean .symbolic
  -if exist fm2.wis del fm2.wis

# The end
