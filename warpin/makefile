# warpin\makefile - build the FM/2 Warpin installation file: FM2.WIS

# Environment:

# DEBUG  0 = release build, 1 = debug build

BASE = fm3

all: wpi

wpi: checkfiles fm2.wpi fm2utils.wpi

checkfiles:
   @echo Checking to see if files have been changed...
    ckfiles

fm2.wpi:: bld_fm2_wpidirs.in
   @echo Staging files for WPI file...
   bld_fm2_wpidirs
   @echo Creating/updating the WPI file
   bld_fm2_wpi

fm2.wpi:: fm2.wis
   @echo Inserting the new Warpin script into the WPI file...
   bld_fm2_wpi /SCRIPT

!ifdef BUILD_FM2UTILS
fm2utils.wpi: bld_fm2utils_wpi.cmd fm2utils.wis fm2utils\fm2utils.doc fm2utils\license.txt ..\shadowmgr.cmd
   @echo Building FM2Utils
   bld_fm2utils_wpi
!endif

distclean:
  -if exist fm2.wpi del fm2.wpi
  -if exist fm2utils.wpi del fm2utils.wpi
  -if exist bld_fm2_wpidirs.in del bld_fm2_wpidirs.in
  clean_wpidirs

# The end
