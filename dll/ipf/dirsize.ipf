:h2 res=95200 name=PANEL_DIRSIZE.Total size of directories
:i1 id=aboutTotals.Total size of directories
This dialog's container contains a breakdown of disk usage for a
directory and its subdirectories.  The container is reached by selecting
:hp1.Miscellaneous->Sizes:ehp1. from a tree directory's context menu or
by running the :hp1.Dir Sizes:ehp1. program object from the FM/2 Tools
subfolder created by INSTALL.
:p.
The container displays a tree view of a directory and all its
subdirectories.  :hp1.Expand:ehp1. and :hp1.Collapse:ehp1. buttons allow
you to quickly open and close branches of the tree.  After the container
has completely filled, each record displays the object's name and four
numbers, as in&colon.
:p.
:lines.
  ADIR  4096k + 8192k = 12288k (8.24%)
:elines.
:p.
The first number is the percentage of the used space this directory
contains, in relation to the total used space on the drive.  The first
number is the number of kilobytes occupied by the directory and any
files and subdirectories it contains.  The second number indicates the
total number of kilobytes occupied by all subdirectories and their files
and subdirectories.  The third number is the total of the first and
second (addition performed on byte count before rounding to kilobytes).
The percentage in parentheses is the percentage of the space used by the
tree that this directory contains of the entire tree displayed.  A graph
appears below the line displaying a "picture" of this percentage.
:p.
FM/2 also color-codes the text describing the directory.  Black text
indicates that something is below the directory.  Blue text indicates
that nothing is below the directory (note there may be subdirectories,
but they are empty).  Grey text indicates that the directory is totally
empty.
:p.
The first (parent) item in the tree shows the percentage of the drive
used by the entire tree.  This is noted in the parentheses containing
the percentage, and the graph for this item is green instead of red.
:p.
The totals reflect the size of files and extended attributes.  Due to
minimum allocation units on the disk, more space may be physically
allocated than is accounted for in the totals.  The text field just
above the pushbuttons gives you stats that _do_ take allocation units
into account, for the entire drive.  Also be aware that directories
containing more than 0 bytes but less than 1024 bytes will be displayed
as containing 1k (1 kilobyte, equal to 1024 bytes) so that you know that
a 0k figure denotes a directory with a truly 0 byte count.
:p.
You can double-click a directory to open it so you can see its files.
:p.
This is a quick way to see where your disk space has gone.
:p.
:hp6.Hint&colon.:ehp6.  You can get a printout of this information by
entering :hp1.PRN:ehp1. as the name of the :hp1.Save:ehp1. file.  The
information printed is the same as that visible in the container, which
is to say that unexpanded branches aren't printed, so you can be
somewhat selective about the information you print.
