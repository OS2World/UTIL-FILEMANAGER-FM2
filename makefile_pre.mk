# makefile_pre.mk - common settings for pre !include

LINK = ilink

!IFNDEF DEBUG
DEBUG = 0
!ENDIF

CFLAGS = /G5 /Gm+ /Gs- /Gt- /IDLL /Mp /O- /Q+ /Sp4 /Ss /Ti+ /W3

!IF $(DEBUG)
LFLAGS = /DE /ALIGN:4 /EXEPACK:2 /NOI /MAP /PMTYPE:PM /NOE
!ELSE
LFLAGS = /ALIGN:4 /EXEPACK /MAP /NOI /PMTYPE:PM /NOE
!ENDIF

# INCLUDE = $(INCLUDE);dll;

RCFLAGS = -i dll

.SUFFIXES:

.SUFFIXES: .c .rc .ipf

.rc.res:
   $(RC) $(RCFLAGS) -r $*.RC

.c.obj:
  $(CC) $(CFLAGS) /C $*.c

# The end
