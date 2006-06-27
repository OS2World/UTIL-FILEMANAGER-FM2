# makefile_post.mk - common makefile suffix settings for all makefiles
# $Id$

# 16 Aug 05 SHL Clean up
# 16 Apr 06 SHL Add lxlite target

!ifndef MAKERES

$(BASE).exe: $(BASE).obj $(BASE).res $(BASE).def
  @$(LINK) @<<$(BASE).lrf
  $(LFLAGS)
  $(BASE).obj
  dll\fm3dll.lib
  os2386.lib
  $(BASE).def
<<
  @rem type $(BASE).lrf
  $(RC) -x2 $(BASE).res $@
  bldlevel $@

!else

$(BASE).exe: $(BASE).res
  @if not exist $@ echo $@ missing
  lxlite $@ /x+ /b-
  lxlite $@ /c:minstub
  $(RC) -x2 $(BASE).res $@
  lxlite $@ /x- /b-
  bldlevel $@

!endif

lxlite:: $(BASE).exe
  lxlite /x- /b- $?

clean:
  -del $(BASE).exe
  -del $(BASE).lrf
  -del $(BASE).map
  -del $(BASE).obj
  -del $(BASE).res

# The end
