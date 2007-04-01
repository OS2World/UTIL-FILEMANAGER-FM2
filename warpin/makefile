# warpin\makefile - build the fm/2 Warpin installation file: FM2.WIS

#    Notes
#       if no WPI file or changed bld_fm2_wpidirs.txt or any of the FILE:'s within have changed
#          bld_fm2_wpidirs
#          bld_fm2_wpi
#       else
#          if changed fm2.wis file
#             bls_fm2_wpi /SCRIPT

# Environment:

# DEBUG  0 = release build, 1 = debug build

BASE = fm2

# !include makefile_pre.mk

# all: files script
#
# files: bld_fm2_wpidirs.last
# files: bld_fm2_wpidirs.last
#
# bld_fm2_wpidirs.last: bld_fm2_wpidirs.txt bld_fm2_wpidirs.cmd
# # stage the files to be included in the WPI file
#    @echo Staging files for WPI file...
#    bld_fm2_wpidirs
# # perform a complete build of the WPI file
#    @echo Creating new WPI file...
#    bld_fm2_wpi
#
# script: fm2.wpi
# #script: fm2.wis
#
# fm2.wpi: fm2.wis
# # just add the new script to the WPI file
#    @echo Adding new Warpin script to WPI file
#    bld_fm2_wpi /SCRIPT

wpi: ckfiles fm2.wpi

ckfiles:
   @echo Checking to see if files have been changed...
    -ckfiles || del fm2.wpi

fm2.wpi:: bld_fm2_wpidirs.txt bld_fm2_wpidirs.cmd bld_fm2_wpi.cmd
# stage the files to be included in the WPI file
   @echo Staging files for WPI file...
   bld_fm2_wpidirs
# perform a complete build of the WPI file
   @echo Creating new WPI file...
   bld_fm2_wpi

fm2.wpi:: fm2.wis
# just add the new script to the WPI file
   @echo Adding new Warpin script to WPI file...
   bld_fm2_wpi /SCRIPT

distclean:
  -del *.wpi

# The end
