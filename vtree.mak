# $Id$

BASE=vtree

!include makefile_pre.mk

all: $(BASE).exe $(BASE).res

$(BASE).res: $(BASE).rc

$(BASE).rc: icons\tree.ico bitmaps\*.bmp

$(BASE).obj: $(BASE).c

!include makefile_post.mk

# The end
