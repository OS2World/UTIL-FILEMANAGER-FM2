
BASE=dirsize

!INCLUDE makefile_pre.mk

ALL: $(BASE).EXE \
     $(BASE).res

$(BASE).res: $(BASE).rc $(BASE).dlg

$(BASE).obj: $(BASE).c

!INCLUDE makefile_post.mk

# The end
