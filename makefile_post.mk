# makefile_post.mk - common settings for post !include

$(BASE).exe: $(BASE).obj $(BASE).res $(BASE).def
    @REM @<<$(BASE).@0
    $(LFLAGS)
    $(BASE).obj
    dll\fm3dll.lib os2386.lib
    $(BASE).def
<<
    type $(BASE).@0
    $(LINK) @$(BASE).@0
    $(RC) -x2 $(BASE).res $(BASE).exe

# The end
