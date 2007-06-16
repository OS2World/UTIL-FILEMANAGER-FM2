# dll\ipf\makefile - build help file(s)
# $Id$

# Copyright (c) 2006, 2007 Steven H. Levine

# 19 Oct 06 SHL Baseline
# 15 Jun 07 SHL Update for OpenWatcom

all: fm3.hlp .symbolic

fm3.hlp: *.ipf bitmaps\*.bmp ..\..\bitmaps\*.bmp
  ipfc /CODEPAGE=850 fm3.ipf
  ren fm3.hlp fm3.hlp

clean: .symbolic
   -del *.hlp
