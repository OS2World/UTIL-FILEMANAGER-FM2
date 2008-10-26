# makefile - build makestr.exe
# $Id$

# Copyright (c) 2002, 2008 Steven H. Levine

# 14 Jun 07 SHL Convert to OpenWatcom
# 27 Jun 07 SHL Support DEBUG set from command line or environment
# 25 Oct 08 SHL Rework dependencies to avoid target dependent build failures

BASE=mkstr

!include ..\..\makefile_pre.mk

# -bc		console app
# -bm		multithread libs
# -bt=os2	target
# -d2		full debug
# -d3		full debug w/unref
# -hd		dwarf
# -j		signed char
# -mf		flat
# -olinars	optimze loops, inline, e(n)able fp recip, relax (a)lias, reordering, space
# -s		disable stack checks
# -sg		generate calls to grow the stack
# -st		touch stack through SS first
# -wx		max warnings
# -zfp		disable fs use
# -zgp		disable gs use
# -zp4		align 4
# -zq		quiet

CFLAGS = -bt=os2 -mf -bm -d2 -olirs -s -j -wx -zfp -zgp -zq -hd

all: $(BASE).exe .symbolic

syms: $(BASE).sym .symbolic

$(BASE).obj: $(BASE).c .AUTODEPEND

$(BASE).exe $(BASE).map: $(BASE).obj $(BASE).lrf
  $(LINK) @$(BASE).lrf
  bldlevel $@

$(BASE).lrf: $(__MAKEFILES__)
   @%write $^@ system os2v2
   @%write $^@ option quiet
   @%write $^@ option verbose
   @%write $^@ option cache
   @%write $^@ option caseexact
!ifdef DEBUG
    @%write $^@ debug dwarf all
!endif
   @%write $^@ option map
   @%write $^@ name $(BASE)
   @%write $^@ op desc '@$#SLAInc:3.13.0$#@$#$#1$#$# 09/01/2008 03:13:00      slamain::EN:US:0:U:@@FM/2 Strings Compiler'
   @%write $^@ file $(BASE).obj
   @%write $^@ library os2386.lib

lxlite: $(BASE).exe .symbolic
!ifndef DEBUG
  lxlite /x- /b- $?
!endif

$(BASE).sym: $(BASE).map
   @echo Processing: $?
   -perl ..\..\debugtools\mapsymw.pl $(BASE).map

clean: .symbolic
  -del $(BASE).exe
  -del $(BASE).lrf
  -del $(BASE).map
  -del $(BASE).obj
  -del $(BASE).sym

cleanobj: .symbolic
  -del $(BASE).obj

# The end
