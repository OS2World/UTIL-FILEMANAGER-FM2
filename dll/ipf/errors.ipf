:h1 res=96700 name=PANEL_OS2ERRORS.OS/2 error list
:i1 id=aboutOS2ErrorList.OS/2 error list
Following are a list of errors you might occasionally see from OS/2,
and what they mean&colon.
:xmp.
2    ERROR_FILE_NOT_FOUND
        File not found.
3    ERROR_PATH_NOT_FOUND
        Path not found.
4    ERROR_TOO_MANY_OPEN_FILES
        Too many open files
        (no handles left).
5    ERROR_ACCESS_DENIED
        Access denied.
6    ERROR_INVALID_HANDLE
        Invalid handle.
11   ERROR_BAD_FORMAT
        Invalid format.
15   ERROR_INVALID_DRIVE
        Invalid drive specified.
16   ERROR_CURRENT_DIRECTORY
        Attempting to remove
        current directory.
18   ERROR_NO_MORE_FILES
        No more files.
19   ERROR_WRITE_PROTECT
        Attempt to write on
        write-protected diskette.
20   ERROR_BAD_UNIT
        Unknown unit.
21   ERROR_NOT_READY
        Drive not ready.
23   ERROR_CRC
        Data error (CRC).
25   ERROR_SEEK
        Seek error.
26   ERROR_NOT_DOS_DISK
        Unknown media type.
27   ERROR_SECTOR_NOT_FOUND
        Sector not found.
28   ERROR_OUT_OF_PAPER
        Printer out of paper.
29   ERROR_WRITE_FAULT
        Write fault.
30   ERROR_READ_FAULT
        Read fault.
31   ERROR_GEN_FAILURE
        General failure.
32   ERROR_SHARING_VIOLATION
        Sharing violation.
33   ERROR_LOCK_VIOLATION
        Lock violation.
34   ERROR_WRONG_DISK
        Invalid disk change.
35   ERROR_FCB_UNAVAILABLE
        FCB unavailable.
36   ERROR_SHARING_BUFFER_EXCEEDED
        Sharing buffer overflow.
50   ERROR_NOT_SUPPORTED
        Network request not supported.
65   ERROR_NETWORK_ACCESS_DENIED
        Access denied.
80   ERROR_FILE_EXISTS
        File exists.
82   ERROR_CANNOT_MAKE
        Cannot make directory entry.
84   ERROR_OUT_OF_STRUCTURES
        Too many redirections.
85   ERROR_ALREADY_ASSIGNED
        Duplicate redirection.
88   ERROR_NET_WRITE_FAULT
        Network device fault.
99   ERROR_DEVICE_IN_USE
        Device in use.
107  ERROR_DISK_CHANGE
        Insert drive B disk into
        drive A.
108  ERROR_DRIVE_LOCKED
        Drive locked by another
        process.
110  ERROR_OPEN_FAILED
        Open/create failed due
        to explicit fail command.
112  ERROR_DISK_FULL
        Not enough space on the disk.
113  ERROR_NO_MORE_SEARCH_HANDLES
        Cannot allocate another
        search structure and handle.
118  ERROR_INVALID_VERIFY_SWITCH
        Invalid value passed for
        verify flag.
123  ERROR_INVALID_NAME
        Illegal character or bad
        file-system name.
124  ERROR_INVALID_LEVEL
        Non-implemented level for
        information retrieval or setting.
125  ERROR_NO_VOLUME_LABEL
        No volume label found with
        DosQFsInfo command.
130  ERROR_DIRECT_ACCESS_HANDLE
        Handle operation invalid for
        direct disk-access handles.
131  ERROR_NEGATIVE_SEEK
        Attempting seek to negative
        offset.
132  ERROR_SEEK_ON_DEVICE
        Application trying to seek
        on device or pipe.
133  ERROR_IS_JOIN_TARGET
        Drive has previously joined
        drives.
134  ERROR_IS_JOINED
        Drive is already joined.
135  ERROR_IS_SUBSTED
        Drive is already substituted.
136  ERROR_NOT_JOINED
        Cannot delete drive that is
        not joined.
137  ERROR_NOT_SUBSTED
        Cannot delete drive that is
        not substituted.
138  ERROR_JOIN_TO_JOIN
        Cannot join to a joined drive.
139  ERROR_SUBST_TO_SUBST
        Cannot substitute to a
        substituted drive.
140  ERROR_JOIN_TO_SUBST
        Cannot join to a substituted
        drive.
141  ERROR_SUBST_TO_JOIN
        Cannot substitute to a joined
        drive.
142  ERROR_BUSY_DRIVE
        Specified drive is busy.
143  ERROR_SAME_DRIVE
        Cannot join or substitute a
        drive to a directory on the
        same drive.
144  ERROR_DIR_NOT_ROOT
        Directory must be a
        subdirectory of the root.
145  ERROR_DIR_NOT_EMPTY
        Directory must be empty
        to use join command.
146  ERROR_IS_SUBST_PATH
        Path specified is being
        used in a substitute.
147  ERROR_IS_JOIN_PATH
        Path specified is being
        used in join.
148  ERROR_PATH_BUSY
        Path specified is being
        used by another process.
149  ERROR_IS_SUBST_TARGET
        Cannot join or substitute drive
        having directory that is target
        of a previous substitute.
154  ERROR_LABEL_TOO_LONG
        Volume label too big.
161  ERROR_BAD_PATHNAME
        Bad path name passed to exec.
166  ERROR_UNC_DRIVER_NOT_INSTALLED
        Default redir return code
167  ERROR_LOCK_FAILED
        Locking failed.
168  ERROR_SWAPIO_FAILED
        Swap IO failed.
169  ERROR_SWAPIN_FAILED
        Swap in failed.
170  ERROR_BUSY
        Busy.
192  ERROR_EXE_MARKED_INVALID
        EXE marked invalid - link
        detected errors when
        application created.
193  ERROR_BAD_EXE_FORMAT
        Bad EXE format - file is
        DOS mode program or
        improper program.
206  ERROR_FILENAME_EXCED_RANGE
        File name or extension
        greater than "8.3" characters.
211  ERROR_INFO_NOT_AVAIL
        File system information not
        available for this file.
212  ERROR_LOCKED
        Locked error.
250  ERROR_CIRCULARITY_REQUESTED
        Renaming a directory that
        would cause a circularity
        problem.
251  ERROR_DIRECTORY_IN_CDS
        Renaming a directory that is
        in use.
252  ERROR_INVALID_FSD_NAME
        Trying to access nonexistent FSD.
253  ERROR_INVALID_PATH
        Bad pseudo device.
254  ERROR_INVALID_EA_NAME
        Bad character in name, or
        bad cbName.
255  ERROR_EA_LIST_INCONSISTENT
        List does not match its size,
        or bad EAs in list.
256  ERROR_EA_LIST_TOO_LONG
        EAList > 64K-1 bytes.
259  ERROR_NO_MORE_ITEMS
        DosQFSAttach ordinal query.
260  ERROR_SEARCH_STRUC_REUSED
        DOS mode findfirst/next search
        structure reused.
263  ERROR_INVALID_ATTR
        Invalid attribute.
266  ERROR_CANNOT_COPY
        Cannot copy.
267  ERROR_DIRECTORY
        Used by DOSCOPY in doscall1.
268  ERROR_OPLOCKED_FILE
        Oplocked file.
270  ERROR_VOLUME_CHANGED
        Volume changed.
275  ERROR_EAS_DIDNT_FIT
        EAS didn't fit.
:exmp.

Additional information for some error codes can be obtained by typing "Help <Error code &numsign.>" 
on an OS/2 command line.
