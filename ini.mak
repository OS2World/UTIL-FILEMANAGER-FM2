# $Id$

# Copyright (c) 2002, 2007 Steven H. Levine

# 16 Jun 07 GKY Convert to OpenWatcom

BASE=ini

!include makefile_pre.mk

all: $(BASE).exe $(BASE).res .symbolic

$(BASE).res: $(BASE).rc icons\$(BASE).ico bitmaps\file.bmp

$(BASE).obj: $(BASE).c

!include makefile_post.mk

# The end
