# warpin\makefile - build the FM/2 Warpin installation file: FM2.WIS
# $Id: $

# 28 Apr 07 SHL Add clean target
# 13 May 07 SHL Tweak macro handling to work as documented
# 13 May 07 SHL Correct clean target

# Environment:

# DEBUG  0 = release build, 1 = debug build

# If BUILD_FM2UTILS defined, build FM2UTILS*.wpi and FM2*.wpi
# Otherwise build just FM2*.wpi
!ifndef BUILD_FM2UTILS
BUILD_FM2UTILS=1
!endif

# FM2_VER defines fm/2 WPI file name suffix
# e.g. FM2_VER=-3-5-9 results in FM2-3-5-9.wpi being built
# If FM2_VER is empty, then FM2.wpi is built
# NOTE: Start the variable with '-'

!ifndef FM2_VER
FM2_VER=-3-5-9
!endif

# FM2UTILS_VER defines the fm2utils WPI file name suffix.
# e.g. FM2UTILS_VER=-1-0 results in FM2Utils-1.0.wpi being built
# If FM2UTILS_VER is empty, then FM2UTILS.wpi is built
# NOTE: Start the variable with '-'

!ifndef FM2UTILS_VER
FM2UTILS_VER=-1-0
!endif

all: wpi

!ifdef BUILD_FM2UTILS
wpi: checkfiles fm2$(FM2_VER).wpi fm2utils$(FM2UTILS_VER).wpi
!else
wpi: checkfiles fm2$(FM2_VER).wpi
!endif


checkfiles:
   @echo Checking to see if files have been changed...
    ckfiles fm2$(FM2_VER).wpi

fm2.wis: fm2.wis.in
   sed -e "s/###FM2UTILS_VER###/$(FM2UTILS_VER)/g" -e "/###include fm2utils.doc###/r fm2utils\fm2utils.doc" -e "s/###include fm2utils.doc###//g" -e "/###include license.txt###/r fm2utils\license.txt" -e "s/###include license.txt###//g" fm2.wis.in >fm2.wis

fm2$(FM2_VER).wpi:: bld_fm2_wpidirs.in
   @echo Staging files for WPI file...
   bld_fm2_wpidirs
   @echo Creating/updating the WPI file
   bld_fm2_wpi fm2$(FM2_VER).wpi

fm2$(FM2_VER).wpi:: fm2.wis
   @echo Inserting the new Warpin script into the WPI file...
   bld_fm2_wpi fm2$(FM2_VER).wpi /SCRIPT

fm2utils$(FM2UTILS_VER).wpi: bld_fm2utils_wpi.cmd fm2utils.wis fm2utils\fm2utils.doc fm2utils\license.txt ..\shdwmgr.cmd
   @echo Building FM2Utils
   bld_fm2utils_wpi fm2utils$(FM2UTILS_VER).wpi

clean:
  -if exist fm2$(FM2_VER).wpi del fm2$(FM2_VER).wpi
  -if exist fm2utils$(FM2UTILS_VER).wpi del fm2utils$(FM2UTILS_VER).wpi
  -if exist bld_fm2_wpidirs.in del bld_fm2_wpidirs.in
  clean_wpidirs

# fixme to decide if these should be cleaned and not distcleaned
distclean: clean
  -if exist fm2.wis del fm2.wis
  -if exist fm2utils.wis del fm2utils.wis

# The end
