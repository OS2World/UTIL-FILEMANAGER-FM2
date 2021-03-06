.***********************************************************************
.*
.* $Id$
.*
.*  Databar Usage
.*
.* Copyright (c) 1993-98 M. Kimes
.* Copyright (c) 2007 Steven H.Levine
.*
.* 07 Jan 07 GKY How to add remote drives
.*
.***********************************************************************
.*
:h2 res=99000 name=PANEL_DATABAR.Databar
:i1 id=aboutDataBar.Databar
The databar can be set to appear when FM/2 minimizes (see :link
reftype=hd res=97000.Settings notebook's Monolithic page:elink.).
The databar shows swapper information, time and date, memory information
and process[/thread] information.
:p.
In addition, the databar can optionally show the free drive space on all
local fixed drives (and, optionally, remote drives
:color fc=default bc=cyan.B2:color fc=default bc=default.
gives you a context menu select include remote drives). To force a drive's
information bar to be updated immediately, click it once with
:color fc=default bc=cyan.B1:color fc=default bc=default.. The
drive information is presented in both text and as a colored bar showing
the amount of free space in relation to the total drive's size. The
bar is usually green, but will turn blue and then red as space decreases.
When a bar is red, it's probably time to do some archiving to get more
free space on it (less than 10% of the drive remains free). Double-click
if running the Databar free-standing to open a VDir for the drive.
:p.
The swapper information includes the size of the swapfile, in kilobytes,
and the amount of free space on the drive holding the swapfile, again,
in kilobytes. Double-click to open the Undelete program. The memory
information includes the amount of free physical memory and total free
memory (including available swapspace), also in kilobytes.
.* Double-click to open the System Information program.
Double-click this field or the process field to open the Kill Process
program.
:p.
The time/date information is updated about once every three seconds. The
other information is updated about once every ten to sixty seconds. For
about six seconds of every minute the time/date field displays the
elapsed system time (how long the system's been up). You can click the
time/date field with :color fc=default bc=cyan.B1:color fc=default bc=default. to cause this info to show up briefly at any
time, or double-click to open the System Clock's Settings window to
allow you to adjust the time (hold down &ShiftKey. while clicking to get the
clock itself).
:p.
You can move the databar by clicking and holding  :color fc=default bc=cyan.B1:color fc=default bc=default. on the databar and
dragging it. You can request a context menu on the date/time window to
adjust some other items. Double-clicking the databar anywhere but on
the bottom four fields with :color fc=default bc=cyan.B1:color fc=default bc=default. will close it and restore FM/2 if you're
not running the :link reftype=hd res=100095.DATABAR.EXE miniapp:elink.. If you're running the miniapp, or
just feel like it, request a context menu on the databar and click the
:hp1.Close Window:ehp1. command. This context menu also lets you
configure the databar (if drives are displayed, if it floats to the top
of other windows, etc.).
:p.
The databar can only be controlled by the mouse, so don't bother with it
if you don't have a mouse or are afraid of the rodent.

