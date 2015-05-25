# makefile_post.mk - common makefile suffix settings for all makefiles
# $Id$

# Copyright (c) 2002, 2012 Steven H. Levine

# 16 Aug 05 SHL Clean up
# 16 Apr 06 SHL Add lxlite target
# 02 Jun 07 SHL Convert to OpenWatcom
# 23 Feb 08 JBS Add support for building SYM files (Ticket 226)
# 25 Oct 08 JBS Rework DEBUG usage to match what C code expects
# 18 Nov 08 JBS Ticket 297: Various build improvements/corrections
# 19 Nov 08 JBS Ticket 297: Removed bldlevel calls
# 14 Dec 08 SHL Drop EXCEPTQ support - will not be used
# 24 Jul 09 SHL Comments
# 21 Jun 11 GKY Add exceptq .xqs support
# 21 Jun 11 GKY Make high memory builds the default resources only for exes
# 04 Jul 11 GKY Make xqs files an explicit target so they will be rebuild if lost somehow.
# 25 Jan 12 SHL Renamae LOW -> NOHIGHMEM and allow set from enviroment
# 17 Jan 14 JBS Ticket 500: Stop setting exe objects to high-memory

!ifndef MAKERES

# Build executable
# Common parameters go in .lrf
# Executable specific paramters go in .def
# Put 32-bit data in high memoryt unless overridden

!ifndef NOHIGHMEM
!ifdef %NOHIGHMEM
NOHIGHMEM=$(%NOHIGHMEM)
!endif
!endif

$(BASE).exe: $(BASE).lrf $(BASE).obj $(BASE).res $(BASE).def .explicit
  @echo Linking $(BASE).exe
  $(LINK) @$(BASE).lrf @$(BASE).def
  @echo.
  @echo Attaching resources to $@
  @echo.
  $(RC) $(RCFLAGS2) $(BASE).res $@
# !ifndef NOHIGHMEM
#   !exehdr /highmem:3 $@
# !endif

# 2012-01-25 SHL fixme to be gone - does not undefine
# NOHIGHMEM =

$(BASE).lrf: $(__MAKEFILES__) .explicit
   @%write $^@ $(LFLAGS)
   @%append $^@ name $(BASE)
   @%append $^@ file $(BASE).obj
   @%append $^@ library dll\fm3dll.lib
   @%append $^@ library os2386.lib

$(BASE).xqs: $(BASE).map .explicit
   @echo Processing: $?
   -mapxqs $?

$(BASE).sym: $(BASE).map .explicit
   @echo Processing: $?
   -perl debugtools\mapsymw.pl $?

!else

# Replace resources
$(BASE).exe: $(BASE).res .explicit
  @if not exist $@ echo $@ missing
!ifndef DEBUG
     lxlite $@ /x+ /b-
     lxlite $@ /c:minstub
!endif
  @echo.
  @echo Attaching resources to $@
  @echo.
  $(RC) $(RCFLAGS2) $(BASE).res $@
!ifndef DEBUG
  lxlite $@ /x- /b-
!endif

!endif

lxlite:: $(BASE).exe .symbolic .explicit
!ifndef DEBUG
  lxlite /x- /b- $?
!endif

clean:: .symbolic .explicit
  -del $(BASE).exe
  -del $(BASE).lrf
  -del $(BASE).map
  -del $(BASE).obj
  -del $(BASE).res
  -del $(BASE).sym
  -del $(BASE).xqs

# The end
