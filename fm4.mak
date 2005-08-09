# $Id$

BASE=fm4

!include makefile_pre.mk

all: $(BASE).exe $(BASE).res

$(BASE).res: $(BASE).rc

$(BASE).rc: $(BASE).h icons\$(BASE).ico bitmaps\*.bmp

$(BASE).obj: $(BASE).c $(BASE).h dll\version.h

!include makefile_post.mk

# The end
