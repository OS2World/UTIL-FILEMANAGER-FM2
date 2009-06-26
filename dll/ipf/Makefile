# dll\ipf\makefile - build help file(s)
# $Id$

# Copyright (c) 2006, 2009 Steven H. Levine

# 19 Oct 06 SHL Baseline
# 15 Jun 07 SHL Update for OpenWatcom
# 25 Apr 09 SHL Support wipfc

!ifndef USE_WIPFC			# if not defined on command line
!ifdef %USE_WIPFC			# if defined in environment
USE_WIPFC = $(%USE_WIPFC)
!endif
!endif

!ifdef USE_WIPFC
IPFC = wipfc
IFLAGS = -l en_US
!else
IPFC = ipfc
IFLAGS = /CODEPAGE=850
!endif

all: fm3.hlp .symbolic

fm3.hlp: *.ipf bitmaps\*.bmp ..\..\bitmaps\*.bmp
  set WIPFC=$(%WATCOM)\wipfc
  $(IPFC) $(IFLAGS) fm3.ipf
  ren fm3.hlp fm3.hlp

clean: .symbolic
   -del *.hlp
