# $Id$

# Copyright (c) 2002, 2007 Steven H. Levine

# 16 Jun 07 GKY Convert to OpenWatcom
# 23 Feb 08 JBS Add support for building SYM files

BASE=dirsize

!include makefile_pre.mk

all: $(BASE).exe $(BASE).res .symbolic

sym: $(BASE).sym .symbolic

$(BASE).res: dll\dirsize.h dll\fm3dll.h $(BASE).rc $(BASE).dlg

$(BASE).obj: $(BASE).c

!include makefile_post.mk

# The end
