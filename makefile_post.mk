# makefile_post.mk - common makefile suffix settings for all makefiles
# $Id$

# 16 Aug 05 SHL Clean up
# 16 Apr 06 SHL Add lxlite target
# 02 Jun 07 SHL Convert to OpenWatcom
# 23 Feb 08 JBS Add support for building SYM files (Ticket 226)
# 25 Oct 08 JBS Rework DEBUG usage to match what C code expects
# 18 Nov 08 JBS Ticket 297: Various build improvements/corrections
# 19 Nov 08 JBS Ticket 297: Removed bldlevel calls

!ifndef MAKERES

# Build executable
# Common parameters go in .lrf
# Executable specific paramters go in .def

$(BASE).exe: $(BASE).lrf $(BASE).obj $(BASE).res $(BASE).def dll\copyright.obj .explicit
  $(LINK) @$(BASE).lrf @$(BASE).def
  @echo.
  @echo Compiling resource: $@
  @echo.
  $(RC) $(RCFLAGS2) $(BASE).res $@

$(BASE).lrf: $(__MAKEFILES__) .explicit
   @%write $^@ $(LFLAGS)
   @%append $^@ name $(BASE)
   @%append $^@ file $(BASE).obj
   @%append $^@ file dll\copyright.obj
!ifdef %EXCEPTQ
    @%append $^@ file exceptq.lib
!endif
   @%append $^@ library dll\fm3dll.lib
   @%append $^@ library os2386.lib

$(BASE).sym: $(BASE).map
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
  @echo Compiling resource: $@
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

# The end
