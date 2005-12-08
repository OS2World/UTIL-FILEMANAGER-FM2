:h2 res=94100 name=PANEL_OBJECTPATH.Object Container
:i1 id=aboutObjectPath.Object Container
This dialog allows you to select a new folder (directory) to hold
objects that FM/2 creates. By default it is <WP_DESKTOP>.  FM/2 tries
to find the directory being used as the desktop folder first in the OS/2
system INI, and if that fails, uses the directory \DESKTOP on your boot
drive as the "desktop window object" directory -- this is the OS/2 2.1+
default. 
:p.
A button labeled :hp1.Desktop:ehp1. allows you to restore the default
<WP_DESKTOP> setting. This will work whether the above fails or not.

:h2 res=93100 name=PANEL_QUICKTREE.Quick Tree
:i1 id=aboutQuickTree.Quick Tree
The Quick Tree dialog displays a container showing all the
subdirectories of the directory currently displayed by a Directory
Container window (or one of its subdirectories, depending on how
you picked the command).
:p.
If you select one of these subdirectories, the Directory Container
window will switch to look into that directory. You can click
Cancel if you change your mind.
:p.
This might be useful when you want to move to the bottom of a long
subdirectory chain in one step.

