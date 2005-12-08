:h2 res=95300 name=PANEL_INIS.INI Viewer
:i1 id=aboutINIS.INI Viewer

:artwork name='..\..\bitmaps\ini.bmp' align=center.
:p.
INI files are a type of configuration file that OS/2 provides to applications and
utilizes itself. This viewer allows you to take a peek inside them and modify 
them as needed. :hp2.:hp8.Caution:ehp8.:ehp2.Backup the INI file before modifying 
it since corruption of an INI file can result in failure of a program including
the WPS to run.
:p.
Two special INIs are used by the system. They are the User INI (usually
OS2.INI) and the System INI (usually OS2SYS.INI). Both are usually
located in the \OS2 directory of your boot drive. FM/2 will tell you
where they are when you view them.
:p.
Applications normally use private INI files named after the application.
For example, FM/2's INI is FM3.INI and its executable is FM3.EXE.
:p.
A record in an INI is composed of three parts:  An application name,
a keyname, and data. This three-part format is represented in the
window by three listboxes. When you choose an application name and
a keyname, you see the data associated with them.
:p.
This window allows you to delete an application name from an INI
(deleting all keynames and data associated with it) with
:hp1.Edit->Delete Application:ehp1., or to delete individual keynames,
deleting the data associated with them with :hp1.Edit->Delete
Keyname:ehp1.. You can also copy or rename entire application names
or application+keynames.
:p.
The :hp1.Files->User Profile:ehp1. command loads the user INI (usually
OS2.INI) and the :hp1.Files->System Profile:ehp1. command loads the
system INI (usually OS2SYS.INI). The :hp1.Files->Other Profile:ehp1.
command allows you to pick an INI file to load. The
:hp1.Files->Refresh:ehp1. command will refresh the contents of the
listboxes from the INI file on disk; handy if a background process might
modify it.
:p.
The :hp1.Entries->Filter appnames:ehp1. command can be used to
selectively remove or include application names from the first listbox
via a dialog that pops up when the command is selected. One filter
"mask" should be entered per line in the MLE; preface "masks" with "/"
to cause the following mask to exlude rather than include items.
:p.
Note that the Filter command removes entries from the listbox;
refiltering with a more inclusive set of masks will not restore filtered
entries. Use the :hp1.Files->Refresh:ehp1. command for that.
:p.
The :hp1.Utilities->Backup Profile:ehp1. creates a backup of the current
profile. You get to specify the filename. The :hp1.Utilities->Change
System Profiles:ehp1. command lets you change the User and System
profiles that OS/2 uses. The :hp1.Utilities->Replace System
Profiles:ehp1. command lets you replace the default system profiles with
new profiles; the old files are physically overwritten.
:p.
You can also drag from and to the top two listboxes in this window. If
you drag from the left window, you drag the current application name,
all its keynames, and all the data associated with the keynames. If you
drag from the right window, you drag the current application and
keynames, and all the data associated with the keyname. You can either
move or copy the record(s) using the standard key modifiers (you did read
the :link reftype=hd res=90000.General Help:elink. topic, didn't you?).
:p.
This window is reached via the :hp1.Utilities:ehp1. pulldown menu on
FM/2's main window, or by selecting (double-clicking) an INI file in a
directory window, or by starting it from the FM/2 folder. The
double-click behavior can be changed by specifying an association for
*.INI under FM/2's main :hp1.Config->Edit Associations:ehp1. pulldown.
:p.
See also&colon.
.br
:link reftype=hd res=95400.Adding an INI record:elink.
.br
:link reftype=hd res=96800.Changing OS/2's INIs:elink.
.br
:link reftype=hd res=96900.Replacing OS/2's INIs:elink.

:h3 res=95400 name=PANEL_ADDINI.Adding an INI record
:i1 id=aboutADDINI.Adding an INI record

To add an INI record, fill in the three entry fields on the screen. The
appname and keyname pair, together, should form a unique ID, or you'll
end up replacing existing data rather than adding new data. Then click
Okay. Click Cancel to abort.
:p.
You can check the :hp1.:link reftype=hd res=99500.Use
\-encoding:elink.:ehp1. checkbox if you want to enter or edit binary
data. FM/2 attempts to determine the type of data if you're editing
rather than adding and sets the checkbox for you accordingly.
:p.
FM/2 adds a trailing 0 byte (NUL) to string information saved to the
INI file (if you don't have :hp1.Use \-encoding:ehp1. checked), which
seems to be a common convention.

:h3 res=96800 name=PANEL_CHANGEINI.Changing OS/2's INIs
:i1 id=aboutChangeINI.Changing OS/2's INIs

FM/2 lets you change the INIs that OS/2 uses while OS/2 is still active.
This, together with the Backup Profile command, allow you to create and
use alternate profiles. Note that this doesn't change the physical INI
files, it just points OS/2 at the new files; when you reboot, the files
specified in CONFIG.SYS are used.
:p.
There are a couple of potential uses for this function. You might use
it to maintain different desktops and switch between them. You might
use it to allow you to copy backed up profiles to OS2.INI and OS2SYS.INI
(the default OS/2 profiles -- do this by first switching to a different
set of profiles, then copying your backups onto OS2.INI and OS2SYS.INI,
then restoring OS2.INI and OS2SYS.INI as the system profiles. You can't
do this normally as the system profiles are readonly and can't be
overwritten.).
:p.
Note that when the change occurs, OS/2 will restart the WPS. It's
recommended that you back up your system INIs before using this and
shut down all other running processes first.

:h3 res=96900 name=PANEL_SWAPINI.Replacing OS/2's INIs
:i1 id=aboutSwapINI.Replacing OS/2's INIs

This dialog physically replaces the OS/2 system INIs with the INI files
you select. The change is permanent.
:p.
Note that when the change occurs, OS/2 will restart the WPS (twice).
It's HIGHLY recommended that you back up your system INIs before using
this and shut down all other running processes first. (Use of this
after having used the :link reftype=hd res=96800.Change System
Profiles:elink. command probably won't do what you expect unless you
first change back to the system defaults.)

:h3 res=99600 name=PANEL_COPYRENAMEINI.Copying and renaming INI records
:i1 id=aboutCopyRenameINI.Copying and renaming INI records

FM/2 allows you to copy or rename (a move followed by a delete) INI
applications or application+keynames.
:p.
A dialog is presented where you enter the new application name (and
keyname if copying or renaming a specific application+keyname pair).
Once you've filled in the new name(s), click Okay to perform the
operation. Click Cancel if you change your mind.

:h3 res=99700 name=PANEL_FILTERINI.Filter INIs
:i1 id=aboutFilterINI.Filter INIs

This dialog can selectively remove or include application names from the
first listbox. One filter "mask" should be entered per line in the MLE;
preface "masks" with "/" to cause the following mask to exlude rather
than include items.
:p.
Note that the Filter command removes entries from the listbox;
refiltering with a more inclusive set of masks will not restore filtered
entries. Use the :hp1.Files->Refresh:ehp1. command for that.

