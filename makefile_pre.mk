# makefile_pre.mk - common makefile prefix settings for all makefiles
# $Id$

LINK = ilink

!ifndef DEBUG
DEBUG = 0
!endif

CFLAGS = /G5 /Gm+ /Gs- /Gt- /IDLL /Mp /O- /Q+ /Sp4 /Ss /Ti+ /W3

!if $(DEBUG)
LFLAGS = /DE /ALIGN:4 /EXEPACK:2 /NOI /MAP /PMTYPE:PM /NOE
!else
LFLAGS = /ALIGN:4 /EXEPACK /MAP /NOI /PMTYPE:PM /NOE
!endif

# Includes can be in current director or dll subdirectory
RCFLAGS = -i dll

.SUFFIXES:
.SUFFIXES: .c .rc .ipf

.rc.res:
   $(RC) $(RCFLAGS) -r $*.RC

.c.obj:
  $(CC) $(CFLAGS) /C $*.c

# The end
