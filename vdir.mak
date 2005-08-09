# $Id$

BASE=vdir

!include makefile_pre.mk

all: $(BASE).exe $(BASE).res

$(BASE).res: $(BASE).rc

$(BASE).rc: dll\fm3dll.h icons\$(BASE).ico bitmaps\*.bmp

$(BASE).obj: $(BASE).c

!include makefile_post.mk

# The end

