# warpin\makefile - build the FM/2 Warpin installation file: FM2.WIS
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

!ifndef FM2_VER                  # if defined on wmake command, use it
FM2_VER=-3-11-0                  # default value
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
wpi: checkfiles fm2.wis fm2$(FM2_VER).wpi .symbolic
!else
wpi: checkfiles fm2.wis fm2$(FM2_VER).wpi fm2utils$(FM2UTILS_VER).wpi .symbolic
!endif


checkfiles: .symbolic
   @echo Checking to see if files have been changed...
    ckfiles fm2$(FM2_VER).wpi

fm2.wis: fm2.wis.in fm2utils\fm2utils.doc fm2utils\license.txt makefile
   sed -e "/@@@include fm2utils.doc@@@/r fm2utils\fm2utils.doc" -e "s/@@@include fm2utils.doc@@@//" -e "/@@@include license.txt@@@/r fm2utils\license.txt" -e "s/@@@include license.txt@@@//" -e "s/@@@FM2_VER@@@/$(FM2_VER)/" -e "s/@@@FM2UTILS_VER@@@/$(FM2UTILS_VER)/" -e "/PACKAGEID/s/-/\\/g" fm2.wis.in >fm2.wis
#  sed  -e "s/@@@FM2_VER@@@/$(FM2_VER)/" -e "/PACKAGEID/s/-/\\/g" fm2.wis.in >fm2.wis

fm2$(FM2_VER).wpi:: bld_fm2_wpidirs.in makefile
   @echo Staging files for WPI file...
   bld_fm2_wpidirs
   @echo Creating/updating the WPI file
   bld_fm2_wpi fm2$(FM2_VER).wpi

fm2$(FM2_VER).wpi:: fm2.wis makefile
   @echo Inserting the new Warpin script into the WPI file...
   bld_fm2_wpi fm2$(FM2_VER).wpi /SCRIPT

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
