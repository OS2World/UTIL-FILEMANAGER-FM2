.***********************************************************************
.*
.* $Id$
.*
.* fm/2 help - Objects and shadows
.*
.* Copyright (c) 1993-98 M. Kimes
.* Copyright (c) 2002-2015 Steven H.Levine
.*
.* 02 May 15 GKY Changes to allow a JAVA executable object to be created using "Real object"
.*                menu item on a jar file.
.*
.***********************************************************************
.*
:h2 res=93600 name=PANEL_SHADOW.Shadow
:i1 id=aboutShadow.Shadow
:artwork name='..\..\bitmaps\shadow.bmp' align=center.
:p.
FM/2 has the ability to create shadows of objects on your WPS desktop
(or in other WPS folders). To create shadows, select objects in an FM/2
window, then select :hp1.Shadow:ehp1. from a context menu or the Files
pulldown submenu (Create Objects submenu).
:p.
If only one shadow is being created, the shadow is placed directly into
the folder you specify. If more than one shadow is being created, FM/2
first prompts you for the name of a folder. This folder is then created
on the desktop and the shadows are placed inside that folder. You can
move the shadows or folder elsewhere after that.
:h2 res=91700 name=PANEL_OBJECTS.Real Objects
:i1 id=aboutObjects.Real Objects
:artwork name='..\..\bitmaps\object.bmp' align=center.
:p.
FM/2 has the ability to create objects on your WPS desktop (or in other
WPS folders). To create objects, select objects in an FM/2 window, then
select :hp1.Real Objects:ehp1. from a context menu or the Files pulldown
submenu (Create Objects submenu).
:p.
If only one object is being created, the object is placed directly into
the folder you specify. If more than one object is being created, FM/2
first prompts you for the name of a folder. This folder is then created
on the desktop and the objects are placed inside that folder. You can
move the objects or folder elsewhere after that.
:p.
FM/2 handles JAVA jar files in a way that creates an executable program
object. The first time you use it you must select the JAVA executable
you wish to use. I recommend PATH\OPENJDK6\bin\javaw.exe. 
Your selection is saved in the FM/2 ini file for future
use. It can be changed by selecting the :hp1.Change JAVA executable:ehp1. 
from a context menu or the Files pulldown submenu (Create Objects submenu)
You will be prompted to select an icon file for the object. Some JAVA 
programs will need additional command line switches (e.g. SmartSVN requires
-Dsmartsvn.checkIncompatibleJava=false). You can add these by opening the
object properties and editing the parameters box. You should restart your
desktop before doing so since the object tends to lose its icon when opening
in properties view if you don't

