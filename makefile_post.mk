# makefile_post.mk - common makefile suffix settings for all makefiles
# $Id$

!ifndef MAKERES

$(BASE).exe: $(BASE).obj $(BASE).res $(BASE).def
  @rem @<<$(BASE).lrf
  $(LFLAGS)
  $(BASE).obj
  dll\fm3dll.lib
  os2386.lib
  $(BASE).def
<<
  type $(BASE).lrf
  $(LINK) @$(BASE).lrf
  $(RC) -x2 $(BASE).res $(BASE).exe

!else

$(BASE).exe: $(BASE).res
  @if not exist $(BASE).exe echo $(BASE).exe missing
  lxlite $(BASE).exe /x+ /b-
  lxlite $(BASE).exe /c:minstub
  $(RC) -x2 $(BASE).res $(BASE).exe
  lxlite $(BASE).exe /x- /b-

!endif

clean:
  -del $(BASE).exe
  -del $(BASE).lrf
  -del $(BASE).map
  -del $(BASE).obj
  -del $(BASE).res

# The end
