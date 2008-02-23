# makefile_pre.mk - common makefile prefix settings for all makefiles
# $Id$

# 01 Sep 06 SHL Adjust .res case
# 02 Jun 07 SHL Convert to OpenWatcom
# 27 Jun 07 SHL Use same CFLAGS for all builds
# 27 Jun 07 SHL Allow DEBUG set from command line or environment
# 03 Jul 07 SHL Change DEBUG semantics to ifdef/ifndef
# 04 Jul 07 SHL Pass DEBUG settings to sub-make
# 22 Sep 07 SHL Switch to 4 byte packing (-zp4)
# 26 Sep 07 SHL Support USE_WRC from environment
# 03 Jan 08 SHL Switch to wrc.exe default; support USE_RC from environment
# 23 Jan 08 JBS Add support for building SYM files (Ticket 226)

CC = wcc386
LINK = wlink

!ifndef USE_RC			# if not defined on command line
!ifdef %USE_RC			# if defined in environment
USE_RC = $(%USE_RC)
!else
USE_RC = 0
!endif
!endif

!if $(USE_RC)
RC = rc
!else
RC = wrc
!endif

# Keep this code in sync with dll\makefile
!ifdef DEBUG                  	# if defined on wmake command line
DEBUG_OPT = DEBUG=$(DEBUG)	# set in case needed by sub-make
!else
!ifdef %DEBUG                  	# if defined in environment
DEBUG = $(%DEBUG)              	# use value from environment
DEBUG_OPT = DEBUG=$(DEBUG)	# set in case needed by sub-make
!endif
!endif

!ifdef DEBUG                    #if DEBUG is defined
!  ifeq DEBUG 1                 #if it is 1
!    ifdef %PERLLIB             #if PRELLIB env. var is defined (i.e. Perl is installed?)
SYMS = $(BASE).sym              #set a target for building SYM files
!    endif	
!  endif	
!endif	

# Some flags are order dependent - see OpenWatcom docs
# -bc		console app
# -bd		build target is a Dynamic Link Library (DLL) (see bd)
# -bg		gui app with WinMain entry point
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
# -wcd14	no reference to symbol
# -wcd726	no reference to formal parameter
# -we		treat warnings as errors
# -wx		max warnings
# -zfp		disable fs use
# -zgp		disable gs use
# -zp4		align 4
# -zq		quiet

# We always compile with debug info to avoid needed a full rebuild just to debug
CFLAGS = -bt=os2 -mf -bm -d2 -olirs   -s -j -we -wx -zfp -zgp -zp4 -zq -hd

LFLAGS = sys os2v2_pm op quiet op verbose op cache op caseexact op map
!ifdef DEBUG
LFLAGS += debug dwarf all
!endif

# rc Includes can be in current director or dll subdirectory
!if $(USE_RC)
RCFLAGS = -r -i dll
RCFLAGS2 = -x2
!else
# Pass 1 flags
RCFLAGS = -r -i=dll -ad
# Pass 2 flags
RCFLAGS2 =-ad
!endif

.SUFFIXES:
.SUFFIXES: .obj .c .res .rc .ipf .sym .map

!if $(USE_RC)
.rc.res:
   $(RC) $(RCFLAGS) $*.rc
   ren $*.res $*.res
!else
.rc.res: .AUTODEPEND
  $(RC) $(RCFLAGS) $*.rc
!endif

.c.obj: .AUTODEPEND
  $(CC) $(CFLAGS) $*.c

# The end
