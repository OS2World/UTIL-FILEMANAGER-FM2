:h1 res=100030 name=PANEL_FILEDLG.File Dialog
:i1 id=aboutFileDlg.File Dialog

This works exactly like the standard OS/2 file dialog with the exception
that it contains a :hp6.UserDirs:ehp6. dropdown list. See the :link
reftype=hd res=91500.Walk Directories:elink. topic for more info on
user directories, including how to set them up.
:p.
The idea here is pretty simple -- you want to give the dialog a
filename. You can type it directly into the :hp6.Filename:ehp6. entry
field or select it using the list controls.
:p.
The :hp6.Drives:ehp6. dropdown list can be used to change the current
drive. The :hp6.Directories:ehp6. listbox can be used to switch
directories on the current drive (as can the :hp6.UserDirs:ehp6.
dropdown). The :hp6.Files:ehp6. listbox can be used to pick an existing
file.
:p.
You can also enter a simple filemask (for example, "*.EXE") into the
:hp6.Filename:ehp6. entry field to filter the files displayed in the
:hp6.Files:ehp6. listbox.
