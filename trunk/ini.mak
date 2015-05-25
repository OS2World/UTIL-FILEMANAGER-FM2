# $Id$

# Copyright (c) 2002, 2007 Steven H. Levine

# 16 Jun 07 GKY Convert to OpenWatcom
# 23 Feb 08 JBS Add support for building SYM files
# 04 Jul 11 GKY Make xqs files an explicit target so they will be rebuild if lost somehow.

BASE=ini

!include makefile_pre.mk

all: $(BASE).exe $(BASE).res .symbolic

sym: $(BASE).sym $(BASE).xqs .symbolic

$(BASE).res: $(BASE).rc icons\$(BASE).ico bitmaps\file.bmp

$(BASE).obj: $(BASE).c

!include makefile_post.mk

# The end
