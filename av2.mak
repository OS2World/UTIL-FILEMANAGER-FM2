
BASE=av2

!INCLUDE makefile_pre.mk

ALL: $(BASE).EXE \
     $(BASE).res

$(BASE).res: $(BASE).rc

$(BASE).obj: $(BASE).c \
    dll\version.h

!INCLUDE makefile_post.mk

# The end
