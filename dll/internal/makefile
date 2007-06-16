# makefile - build makestr.exe
# $Id$

# Copyright (c) 2002, 2007 Steven H. Levine

# 14 Jun 07 SHL Convert to OpenWatcom

BASE=mkstr

!include ..\..\makefile_pre.mk

# CFLAGS = /G5 /Gm+ /Gs- /Gt- /IDLL /Mp /O- /Q+     /Sp4 /Ss /Ti+ /W3
# CFLAGS = /G5 /Gm+ /Gs- /Gt-       /Mp /O+ /Q+ /Sm      /Ss /W3
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
CFLAGS = -bt=os2 -mf -bm -d1 -olirs   -s -j -wx -zfp -zgp -zq -hd

# fixme
#LFLAGS = /EXEPACK:2 /MAP /PMTYPE:VIO

all: $(BASE).exe .symbolic

$(BASE).obj: $(BASE).c ..\version.h ..\fm3str.h

$(BASE).exe: $(BASE).obj $(BASE).lrf
  @$(LINK) @$(BASE).lrf
  bldlevel $@

$(BASE).lrf: $(__MAKEFILES__)
   @%write $^@ system os2v2
   @%write $^@ option quiet
   @%write $^@ option verbose
   @%write $^@ option cache
   @%write $^@ option caseexact
!ifdef %DEBUG
    @%write $^@ debug dwarf all
!endif
   @%write $^@ option map
   @%write $^@ name $(BASE)
   @%write $^@ op desc '@$#$(%VENDOR):$(%BUILDVER)$#@$#$#1$#$# $(%BUILDTIME)      slamain::EN:US:0:U:@@Fm/2 Strings Compiler'
   @%write $^@ file $(BASE).obj
   @%write $^@ library os2386.lib

clean: .symbolic
  -del $(BASE).exe
  -del $(BASE).lrf
  -del $(BASE).map
  -del $(BASE).obj

cleanobj: .symbolic
  -del $(BASE).obj

# The end
