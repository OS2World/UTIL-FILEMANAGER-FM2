# makefile_post.mk - common makefile suffix settings for all makefiles
# $Id$

# 16 Aug 05 SHL Clean up
# 16 Apr 06 SHL Add lxlite target
# 02 Jun 07 SHL Convert to OpenWatcom

!ifndef MAKERES

# Build executable
# Common parameters go in .lrf
# Executable specific paramters go in .def

$(BASE).exe: $(BASE).lrf $(BASE).obj $(BASE).res $(BASE).def .explicit
  @$(LINK) @$(BASE).lrf @$(BASE).def
  $(RC) $(RCFLAGS2) $(BASE).res $@
  bldlevel $@

$(BASE).lrf: $(__MAKEFILES__) .explicit
   @%write $^@ $(LFLAGS)
   @%append $^@ name $(BASE)
   @%append $^@ file $(BASE).obj
!ifdef %EXCEPTQ
    @%append $^@ file exceptq.lib
!endif
   @%append $^@ library dll\fm3dll.lib
   @%append $^@ library os2386.lib

!else

# Replace resources
$(BASE).exe: $(BASE).res .explicit
  @if not exist $@ echo $@ missing
  lxlite $@ /x+ /b-
  lxlite $@ /c:minstub
  $(RC) $(RCFLAGS2) $(BASE).res $@
  lxlite $@ /x- /b-
  bldlevel $@

!endif

lxlite:: $(BASE).exe .symbolic .explicit
  lxlite /x- /b- $?

clean:: .symbolic .explicit
  -del $(BASE).exe
  -del $(BASE).lrf
  -del $(BASE).map
  -del $(BASE).obj
  -del $(BASE).res

# The end
