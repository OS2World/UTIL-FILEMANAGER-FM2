# dll\ipf\makefile - build help file(s)
# $Id$

# Copyright (c) 2006 Steven H. Levine

# 19 Oct 06 SHL Baseline

all: fm3.hlp

fm3.hlp: *.ipf bitmaps\*.bmp ..\..\bitmaps\*.bmp
  ipfc /CODEPAGE=850 fm3.ipf
  ren fm3.hlp fm3.hlp

clean:
   -del *.hlp
