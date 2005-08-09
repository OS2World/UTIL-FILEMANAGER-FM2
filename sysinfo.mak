# $Id$

BASE=sysinfo

!include makefile_pre.mk

all: $(BASE).exe $(BASE).res

$(BASE).res: $(BASE).rc

$(BASE).rc: icons\$(BASE).ico

$(BASE).obj: $(BASE).c

!include makefile_post.mk

# The end

