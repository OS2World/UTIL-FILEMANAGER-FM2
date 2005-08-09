# $Id$

BASE=dirsize

!include makefile_pre.mk

all: $(BASE).exe $(BASE).res

$(BASE).res: dirsize.h dll\fm3dll.h $(BASE).rc $(BASE).dlg

$(BASE).obj: $(BASE).c

!include makefile_post.mk

# The end
