
BASE=fm4

!INCLUDE makefile_pre.mk

ALL: $(BASE).EXE \
     $(BASE).res

$(BASE).res: $(BASE).rc \
     $(BASE).h

$(BASE).obj: $(BASE).c \
     $(BASE).h dll\version.h

!INCLUDE makefile_post.mk

# The end
