
BASE=undel

!INCLUDE makefile_pre.mk

ALL: $(BASE).EXE \
     $(BASE).res

$(BASE).res: $(BASE).rc

$(BASE).obj: $(BASE).c

!INCLUDE makefile_post.mk

# The end

