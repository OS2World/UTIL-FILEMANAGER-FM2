
BASE=sysinfo

!INCLUDE makefile_pre.mk

ALL: sysinfo.EXE \
     sysinfo.res

sysinfo.res: sysinfo.rc

sysinfo.obj: sysinfo.c

!INCLUDE makefile_post.mk

# The end

