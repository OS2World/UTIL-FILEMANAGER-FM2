# $Id$

# Copyright (c) 2002, 2007 Steven H. Levine

# 16 Jun 07 GKY Convert to OpenWatcom

BASE=av2

!include makefile_pre.mk

all: $(BASE).exe $(BASE).res .symbolic

$(BASE).res: $(BASE).rc dll\fm3dll.h icons\view3.ico bitmaps\*.bmp

$(BASE).obj: $(BASE).c dll\version.h

!include makefile_post.mk

# The end
