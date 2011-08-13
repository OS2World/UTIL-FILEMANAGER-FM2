# dll\ipf\makefile - build help file(s)
# $Id$

# Copyright (c) 2006, 2010 Steven H. Levine

# 19 Oct 06 SHL Baseline
# 15 Jun 07 SHL Update for OpenWatcom
# 25 Apr 09 SHL Support wipfc; use USE_WIPFC to enable
# 19 Mar 10 SHL Default to wipfc; support USE_IPFC option to override
# 04 Jul 11 GKY Add eFTE backup files (*~) to clean target

!ifndef USE_WIPFC			# if not defined on command line
!ifdef %USE_WIPFC			# if defined in environment
USE_WIPFC = $(%USE_WIPFC)
!endif
!endif

!ifndef USE_IPFC			# if not defined on command line
!ifdef %USE_IPFC			# if defined in environment
USE_IPFC = $(%USE_IPFC)
!endif
!endif

!ifdef USE_IPFC
IPFC = ipfc
IFLAGS = /CODEPAGE=850
!else
IPFC = wipfc
IFLAGS = -l en_US
!endif

all: fm3.hlp .symbolic

fm3.hlp: *.ipf bitmaps\*.bmp ..\..\bitmaps\*.bmp
  set WIPFC=$(%WATCOM)\wipfc
  $(IPFC) $(IFLAGS) fm3.ipf
#  ren fm3.hlp fm3.hlp  # pointless rename

clean: .symbolic
   -del *.hlp
   -del *~
