# $Id$

BASE=ini

!include makefile_pre.mk

all: $(BASE).exe $(BASE).res

$(BASE).res: $(BASE).rc

$(BASE).rc: icons\$(BASE).ico bitmaps\file.bmp

$(BASE).obj: $(BASE).c

!include makefile_post.mk

# The end
