# $Id$

BASE=av2

!include makefile_pre.mk

all: $(BASE).exe $(BASE).res

$(BASE).res: $(BASE).rc

$(BASE).rc: dll\fm3dll.h icons\view3.ico bitmaps\*.bmp

$(BASE).obj: $(BASE).c dll\version.h

!include makefile_post.mk

# The end
