# $Id$

# Copyright (c) 2002, 2007 Steven H. Levine

# 14 Jun 07 SHL Convert to OpenWatcom
# 23 Feb 08 JBS Add support for building SYM files

BASE=fm4

!include makefile_pre.mk

all: $(BASE).exe $(BASE).res $(SYMS) .symbolic

$(BASE).res: $(BASE).rc icons\$(BASE).ico bitmaps\*.bmp

$(BASE).obj: $(BASE).c dll\version.h

!include makefile_post.mk

# The end
