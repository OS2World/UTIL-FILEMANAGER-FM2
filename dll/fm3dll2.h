
/***********************************************************************

  $Id$

  Resource item definitions

  Copyright (c) 1993-02 M. Kimes
  Copyright (c) 2003, 2008 Steven H.Levine

  15 Oct 02 MK Baseline
  04 Nov 03 SHL Drop obsoletes
  01 Nov 04 SHL Rename SKULL? defines to avoid rc issues
  06 Jul 06 SHL Add Select Same Content support
  31 Aug 06 GKY Add partitioning menu items
  17 Feb 06 GKY Add more drive types and no drive stat option
  10 Jan 08 SHL Add IDM_*SETTINGS for individual notebook pages
  12 Jan 08 SHL Add IDM_TOGGLEDRAGDIALOG
  19 Jan 08 GKY Add IDM_COMMANDLINESUBMENU, IDM_SYSTEMSUBMENU & IDM_BOOKSELFSUBMENU
  15 Feb 08 SHL Add definitions to support settings menu conditional cascade
  15 Feb 08 SHL Drop obsoletes

  Align with spaces only - no tabs please

***********************************************************************/

#define COLR_WHITE          0
#define COLR_BLACK          1
#define COLR_BLUE           2
#define COLR_RED            3
#define COLR_PINK           4
#define COLR_GREEN          5
#define COLR_CYAN           6
#define COLR_YELLOW         7
#define COLR_DARKGRAY       8
#define COLR_DARKBLUE       9
#define COLR_DARKRED        10
#define COLR_DARKPINK       11
#define COLR_DARKGREEN      12
#define COLR_DARKCYAN       13
#define COLR_BROWN          14
#define COLR_PALEGRAY       15

#define SORT_FIRSTEXTENSION 0x00000001
#define SORT_LASTEXTENSION  0x00000002
#define SORT_SIZE           0x00000004
#define SORT_EASIZE         0x00000008
#define SORT_LWDATE         0x00000010
#define SORT_LADATE         0x00000020
#define SORT_CRDATE         0x00000040
#define SORT_DIRSFIRST      0x00000080
#define SORT_DIRSLAST       0x00000100
#define SORT_FILENAME       0x00000200
#define SORT_REVERSE        0x00000400
#define SORT_PATHNAME       0x00000800
#define SORT_NOSORT         0x00001000
#define SORT_SUBJECT        0x00002000

#define DRIVE_REMOVABLE     0x00000001
#define DRIVE_NOTWRITEABLE  0x00000002
#define DRIVE_IGNORE        0x00000004
#define DRIVE_CDROM         0x00000008
#define DRIVE_NOLONGNAMES   0x00000010
#define DRIVE_REMOTE        0x00000020
#define DRIVE_BOOT          0x00000040
#define DRIVE_INVALID       0x00000080
#define DRIVE_NOPRESCAN     0x00000100
#define DRIVE_ZIPSTREAM     0x00000200
#define DRIVE_NOLOADICONS   0x00000400
#define DRIVE_NOLOADSUBJS   0x00000800
#define DRIVE_NOLOADLONGS   0x00001000
#define DRIVE_SLOW          0x00002000
#define DRIVE_INCLUDEFILES  0x00004000
#define DRIVE_VIRTUAL       0x00008000
#define DRIVE_NOSTATS       0x00010000
#define DRIVE_RAMDISK       0x00020000

#define MAIN_FRAME          1
#define MAIN_STATUS         2
#define MAIN_TOOLS          3
#define MAIN_USERLIST       4
#define MAIN_DRIVELIST      5
#define MAIN_SETUPLIST      6
#define COMMAND_LINE        7
#define MAIN_AUTOVIEW       8
#define MAIN_POPUP          9
#define MAIN_BUTTONLIST     10
#define MAIN_DRIVES         11
#define MAIN_LED            12
#define MAIN_LEDHDR         13
#define DEFMENU             14
#define MAIN2_FRAME         15
#define MAIN_AUTOVIEWMLE    16
#define MAIN_CMDLIST        17
#define COMMAND_BUTTON      18

#define MAIN_HELP           97
#define MAIN_TRASHCAN       98

#define ID_TIMER            4
#define ID_STICK1           5
#define ID_STICK2           6
#define ID_STICK3           7
#define ID_STICK4           8
#define ID_STICK5           9
#define ID_STICK12          10
#define ID_STICK22          11
#define ID_STICK32          12
#define ID_STICK42          13
#define ID_STICK52          14
#define ID_TIMER2           15
#define ID_TIMER3           16

#define ID_TIMER4           19
#define ID_TIMER5           20
#define ID_TIMER6           21

#define MAIN_STATUS2        19

#define OBJ_FRAME           20

#define ID_IMAGE            40
#define ID_IMAGEOBJECT      49

#define ID_HELPTABLE        50
#define ID_HELPSUBTABLE     51
#define ID_BUTTONMENU       99

#define DIR_FRAME           100
#define DIR_CNR             101
#define DIR_FOLDERICON      102
#define DIR_TOTALS          103
#define DIR_SELECTED        104
#define DIR_MAX             105
#define DIR_RESTORE         106
#define DIR_VIEW            107
#define DIR_SORT            108
#define DIR_FILTER          109
#define DIROBJ_FRAME        200

#define NEWVIEW_FRAME       280
#define NEWVIEW_STATUS1     281
#define NEWVIEW_STATUS2     282
#define NEWVIEW_STATUS3     283
#define NEWVIEW_POPUP       284
#define NEWVIEW_LISTBOX     285
#define NEWVIEW_DRAG        286

#define SEEALL_FRAME        290
#define SEEALL_STATUS       291
#define SEEALL_OBJ          295
#define SEEALL_POPUP        296

#define BITMAP_FRAME        299

#define TREE_FRAME          300
#define TREE_CNR            301
#define TREE_TOTALS         302
#define TREEOBJ_FRAME       400

#define COLLECTOR_FRAME     450
#define COLLECTOR_CNR       451
#define COLLECTOROBJ_FRAME  460

#define ARC_FRAME           500
#define ARC_CNR             501
#define ARC_EXTRACTDIR      502
#define ARCOBJ_FRAME        600

#define SKULL0_ICON         690
#define SKULL1_ICON         691
#define SKULL2_ICON         692
#define SKULL3_ICON         693
#define SKULL4_ICON         694
#define SKULL5_ICON         695
#define SKULL6_ICON         696
#define SKULL7_ICON         697
#define SKULL8_ICON         698
#define SKULL9_ICON         699

#define INI_FRAME           700
#define INI_APPLIST         701
#define INI_KEYLIST         702
#define INI_DATALIST        703
#define INI_DELETEAPP       704
#define INI_DELETEKEY       705
#define INI_ADDENTRY        706
#define INI_EDITENTRY       707
#define INI_REFRESH         708
#define INI_USERPROFILE     709
#define INI_SYSTEMPROFILE   710
#define INI_OTHERPROFILE    711
#define INI_NUMAPPS         712
#define INI_NUMKEYS         713
#define INI_NUMDATA         714
#define INI_APPHDR          715
#define INI_KEYHDR          716
#define INI_DATAHDR         717
#define INI_CONFIRM         718
#define INI_BACKUPINI       719
#define INI_CHANGEINI       720
#define INI_SWAPINI         721
#define INI_RENAMEAPP       722
#define INI_RENAMEKEY       723
#define INI_COPYAPP         724
#define INI_COPYKEY         725
#define INI_APPMENU         726
#define INI_KEYMENU         727

#define DIR_POPUP           750
#define FILE_POPUP          751
#define TREE_POPUP          752
#define DIRCNR_POPUP        753
#define TREECNR_POPUP       754
#define ARCCNR_POPUP        755
#define ARC_POPUP           756
#define COLLECTORDIR_POPUP  757
#define COLLECTORFILE_POPUP 758
#define COLLECTORCNR_POPUP  759

#define REMOVABLE_ICON      900
#define CDROM_ICON          901
#define REMOTE_ICON         902
#define FLOPPY_ICON         903
#define FILE_ICON           904
#define LASTITEM_ICON       905
#define DRAG_ICON           906
#define FINGER_ICON         907
#define APP_ICON            908
#define DUNNO_ICON          909
#define FILE_SYSTEM_ICON    910
#define FILE_HIDDEN_ICON    911
#define FILE_READONLY_ICON  912
#define DRIVE_ICON          913
#define ZIPSTREAM_ICON      914
#define ART_ICON            915
#define COMPARE_ICON        916
#define FINGER2_ICON        917
#define DRAG2_ICON          918
#define LEDON_BMP           919
#define LEDOFF_BMP          920
#define DIRSIZE_ICON        921
#define LEDON2_BMP          922
#define LEDOFF2_BMP         923
#define ENV_ICON            924
#define VIRTUAL_ICON        925
#define RAMDISK_ICON        926

#define IDM_HELPMOUSE       987
#define IDM_HELPCONTEXT     988
#define IDM_HELPPIX         989
#define IDM_HELPTUTOR       990
#define IDM_HELPHINTS       991
#define IDM_HELPGENERAL     992
#define IDM_HELPKEYS        993
#define IDM_UNDO            994
#define IDM_EXIT            995
#define IDM_HELPCONTENTS    996
#define IDM_HIDEMENU        997
#define IDM_ABOUT           998
#define IDM_HELP            999

#define IDM_FILESMENU       1000
#define IDM_MOVE            1001
#define IDM_COPY            1002
#define IDM_COMPARE         1003
#define IDM_DELETE          1004
#define IDM_RENAME          1005
#define IDM_PERMDELETE      1006
#define IDM_PRINT           1007
#define IDM_EAS             1008
#define IDM_ATTRS           1009
#define IDM_INFO            1010
#define IDM_COLLECT         1011
#define IDM_OPENSUBMENU     1012
#define IDM_OPENSETTINGS    1013
#define IDM_OPENICON        1014
#define IDM_OPENDETAILS     1015
#define IDM_OPENTREE        1016
#define IDM_OPENDEFAULT     1017
#define IDM_OPENWINDOW      1018
#define IDM_OPENWALK        1019
#define IDM_OBJECTSUBMENU   1020
#define IDM_SHADOW          1021
#define IDM_OBJECT          1022
#define IDM_VIEW            1023
#define IDM_EDIT            1024
#define IDM_SUBJECT         1025
#define IDM_MKDIR           1026
#define IDM_SAVETOCLIP      1027
#define IDM_SAVETOLIST      1028
#define IDM_ARCHIVE         1029
#define IDM_EXTRACT         1030
#define IDM_SIZES           1031
#define IDM_SAVESUBMENU     1032
#define IDM_RESORT          1033
#define IDM_FIND            1034
#define IDM_EXTRACTWDIRS    1035
#define IDM_TEST            1036
#define IDM_ARCEXTRACT      1037
#define IDM_ARCEXTRACTWDIRS 1038
#define IDM_VIRUSSCAN       1039
#define IDM_EXEC            1040
#define IDM_ARCEXTRACTEXIT  1041
#define IDM_ARCEXTRACTWDIRSEXIT 1042
#define IDM_REMOVE          1043
#define IDM_CLEARCNR        1044
#define IDM_COLLECTFROMCLIP 1045
#define IDM_VIEWSMENU       1046
#define IDM_GREP            1047
#define IDM_EDITCOMMANDS    1048
#define IDM_FORMAT          1049
#define IDM_CHKDSK          1050
#define IDM_EJECT           1051
#define IDM_PARTITION       1052
#define IDM_SETICON         1053
#define IDM_APPENDTOCLIP    1054
#define IDM_TREE            1055
#define IDM_QTREE           1056
#define IDM_FAKEEXTRACT     1057
#define IDM_MCIPLAY         1058
#define IDM_VIEWSUBMENU     1059
#define IDM_COLLECTFROMFILE 1060
#define IDM_COLLECTSELECT   1061
#define IDM_COLLECTMENU     1062
#define IDM_UPDATE          1063
#define IDM_OPTIMIZE        1064
#define IDM_ARCHIVEM        1065
#define IDM_FAKEEXTRACTM    1066
#define IDM_SEEALL          1067
#define IDM_REFRESH         1068
#define IDM_WILDMOVE        1069
#define IDM_WILDCOPY        1070
#define IDM_WILDRENAME      1071
#define IDM_COPYMENU        1072
#define IDM_MOVEMENU        1073
#define IDM_ADDTOUSERLIST   1074
#define IDM_DELETEFROMUSERLIST 1075
#define IDM_SAVEDIRCNRSTATE 1076
#define IDM_DELETEDIRCNRSTATE 1077
#define IDM_CLI             1078
#define IDM_EXTRACTSUBMENU  1079
#define IDM_VIEWORARC       1080
#define IDM_VIEWTEXT        1081
#define IDM_VIEWBINARY      1082
#define IDM_EDITTEXT        1083
#define IDM_EDITBINARY      1084
#define IDM_EDITSUBMENU     1085
#define IDM_SHOWSORT        1086
#define IDM_SHOWSELECT      1087
#define IDM_DOITYOURSELF    1088
#define IDM_EXPANDSUBMENU   1089
#define IDM_MISCSUBMENU     1090
#define IDM_DELETESUBMENU   1091
#define IDM_SHOWDRIVES      1092
#define IDM_DOSCOMMANDLINE  1093
#define IDM_WINFULLSCREEN   1094
#define IDM_HEXMODE         1095
#define IDM_SEARCHMENU      1096
#define IDM_FINDFIRST       1097
#define IDM_FINDNEXT        1098
#define IDM_GOTOLINE        1099
#define IDM_GOTOOFFSET      1100
#define IDM_CODEPAGE        1101
#define IDM_NEXTSELECTED    1102
#define IDM_PREVSELECTED    1103
#define IDM_FINDPREV        1104
#define IDM_SELECTFOUND     1105
#define IDM_DESELECTFOUND   1106
#define IDM_SAVETOCLIP2     1107
#define IDM_APPENDTOCLIP2   1108
#define IDM_SAVETOLIST2     1109
#define IDM_HELPUSERLIST    1110
#define IDM_KILLME          1111
#define IDM_SHADOW2         1112
#define IDM_OPENICONME      1113
#define IDM_OPENDETAILSME   1114
#define IDM_OPENTREEME      1115
#define IDM_OPENSETTINGSME  1116
#define IDM_NEXTBLANKLINE   1117
#define IDM_PREVBLANKLINE   1118
#define IDM_MOREBUTTONS     1119
#define IDM_TOAUTOMLE       1120
#define IDM_BEGINEDIT       1121
#define IDM_ENDEDIT         1122
#define IDM_FINDINTREE      1123
#define IDM_DRVFLAGS        1124
#define IDM_SHOWALLFILES    1125
#define IDM_MOVEPRESERVE    1126
#define IDM_COPYPRESERVE    1127
#define IDM_SHOWALLFILESCNR 1128
#define IDM_DRIVEBAR        1129
#define IDM_LOCK            1130
#define IDM_UNLOCK          1131
#define IDM_UUDECODE        1132
#define IDM_MERGE           1133
#define IDM_MERGEBINARY     1134
#define IDM_MERGETEXT       1135
#define IDM_MERGEBINARYAPPEND 1136
#define IDM_MERGETEXTAPPEND   1137
#define IDM_SHOWNOTEWND     1138
#define IDM_HIDENOTEWND     1139
#define IDM_TWODIRS         1140
#define IDM_MAXIMIZE        1141
#define IDM_CONTEXTMENU     1142
#define IDM_VIEWARCHIVE     1143
#define IDM_CREATE          1144
#define IDM_VTREE           1145
#define IDM_DUPES           1146
#define IDM_WRAP            1147
#define IDM_IGNOREHTTP      1148
#define IDM_IGNOREFTP       1149
#define IDM_DETACH          1150
#define IDM_TREEVIEW        1151
#define IDM_WPSMOVE         1152
#define IDM_WPSCOPY         1153
#define IDM_SETTARGET       1154
#define IDM_CLOSETRAY       1155
#define IDM_PARTITIONDF     1156
#define IDM_PARTITIONLVMG   1157
#define IDM_PARTITIONFD     1158
#define IDM_IGNOREMAIL      1159
#define IDM_TOGGLEDRAGDIALOG 1160

#define IDM_UTILITIESMENU   2000
#define IDM_INIVIEWER       2001
#define IDM_COLLECTOR       2002
#define IDM_KILLPROC        2003
#define IDM_UNDELETE        2004
#define IDM_UNDELETESPEC    2005
#define IDM_INSTANT         2006
#define IDM_COMMANDLINE     2007
#define IDM_SYSINFO         2008
#define IDM_SYSTEMCLOCK     2009
#define IDM_VIEWINFS        2010
#define IDM_VIEWHELPS       2011
#define IDM_REMAP           2012
#define IDM_COMMANDLINESUBMENU  2013
#define IDM_SYSTEMSUBMENU   2014
#define IDM_BOOKSELFSUBMENU 2015

#define IDM_CONFIGMENU      3000
#define IDM_EDITASSOC       3001
#define IDM_TOOLBAR         3002
#define IDM_SORTSUBMENU     3003
#define IDM_SORTNAME        3004
#define IDM_SORTFILENAME    3005
#define IDM_SORTSIZE        3006
#define IDM_SORTEASIZE      3007
#define IDM_SORTFIRST       3008
#define IDM_SORTLAST        3009
#define IDM_SORTLWDATE      3010
#define IDM_SORTLADATE      3011
#define IDM_SORTCRDATE      3012
#define IDM_SORTDIRSFIRST   3013
#define IDM_SORTDIRSLAST    3014
#define IDM_SORTREVERSE     3015
#define IDM_SORTSMARTNAME   3016
#define IDM_SORTNONE        3017
#define IDM_SORTSUBJECT     3018
#define IDM_FONTPALETTE     3020
#define IDM_COLORPALETTE    3021
#define IDM_PROGSETUP       3022
#define IDM_TOGGLESSUBMENU  3033
#define IDM_FOLLOWTREE      3034
#define IDM_LOADSUBJECTS    3035
#define IDM_LOADLONGNAMES   3036
#define IDM_DONTMOVEMOUSE   3037
#define IDM_UNHILITE        3038
#define IDM_CONFIRMDELETE   3039
#define IDM_SYNCUPDATES     3040
#define IDM_FORCEUPPER      3041
#define IDM_FORCELOWER      3042
#define IDM_VERIFYWRITES    3043
#define IDM_NOICONSFILES    3044
#define IDM_NOICONSDIRS     3045
#define IDM_DCOPENS         3046
#define IDM_LINKSETSICON    3047
#define IDM_TEXTTOOLS       3048
#define IDM_TOOLTITLES      3049
#define IDM_PRINTER         3050
#define IDM_EDITANYARCHIVER 3051
#define IDM_SCHEMEPALETTE   3052
#define IDM_SYSTEMSETUP     3053
#define IDM_SAVESTATE       3054
#define IDM_DEFAULTCOPY     3055
#define IDM_TOOLSUBMENU     3056
#define IDM_LOGFILE         3057
#define IDM_IDLECOPY        3058
#define IDM_NOTEBOOK        3059
#define IDM_BLINK           3060
#define IDM_HICOLORPALETTE  3061
#define IDM_USERLIST        3062
#define IDM_AUTOVIEW        3063
#define IDM_MAINPOPUP       3064
#define IDM_SELECTSUBMENU   3065
#define IDM_FOLDERAFTEREXTRACT 3066
#define IDM_AUTOVIEWSUBMENU 3067
#define IDM_AUTOVIEWFILE    3068
#define IDM_AUTOVIEWCOMMENTS 3069
#define IDM_TILEBACKWARDS   3070
#define IDM_AUTOVIEWMLE     3071
#define IDM_PARTITIONSMENU 3072

#define IDM_NOTEBOOKSUBMENU             3310

#define IDM_DIRCNRSETTINGS              3311
#define IDM_DIRVIEWSETTINGS             3312
#define IDM_DIRSORTSETTINGS             3313
#define IDM_COLLECTORVIEWSETTINGS       3314
#define IDM_COLLECTORSORTSETTINGS       3315
#define IDM_ARCHIVERSETTINGS            3316
#define IDM_TREECNRVIEWSETTINGS         3317
#define IDM_TREECNRSORTSETTINGS         3318
#define IDM_VIEWERSETTINGS              3319
#define IDM_VIEWERSETTINGS2             3320
#define IDM_COMPARESETTINGS             3321
#define IDM_MONOLITHICSETTINGS          3322
#define IDM_GENERALSETTINGS             3323
#define IDM_SCANSETTINGS                3324
#define IDM_BUBBLESSETTINGS             3325
#define IDM_QUICKSETTINGS               3326

#define IDM_DRIVESMENU      3899
#define IDM_DRIVEA          3900
#define IDM_DRIVEATEXT      3950

#define IDM_COMMANDSMENU    4000
#define IDM_COMMANDSTART    4001
#define IDM_COMMANDNUM0     4001
#define IDM_COMMANDNUM1     4002
#define IDM_COMMANDNUM2     4003
#define IDM_COMMANDNUM3     4004
#define IDM_COMMANDNUM4     4005
#define IDM_COMMANDNUM5     4006
#define IDM_COMMANDNUM6     4007
#define IDM_COMMANDNUM7     4008
#define IDM_COMMANDNUM8     4009
#define IDM_COMMANDNUM9     4010
#define IDM_COMMANDNUM10    4011
#define IDM_COMMANDNUM11    4012
#define IDM_COMMANDNUM12    4013
#define IDM_COMMANDNUM13    4014
#define IDM_COMMANDNUM14    4015
#define IDM_COMMANDNUM15    4016
#define IDM_COMMANDNUM16    4017
#define IDM_COMMANDNUM17    4018
#define IDM_COMMANDNUM18    4019
#define IDM_COMMANDNUM19    4020

#define IDM_QUICKTOOLSTART  4900

#define IDM_CNRMENU         5000
#define IDM_FILTER          5001
#define IDM_ICON            5002
#define IDM_TEXT            5003
#define IDM_NAME            5004
#define IDM_DETAILS         5005
#define IDM_MINIICONS       5006
#define IDM_DETAILSTITLES   5007
#define IDM_SHOWLNAMES      5008
#define IDM_SHOWSUBJECT     5009
#define IDM_SHOWEAS         5010
#define IDM_SHOWSIZE        5011
#define IDM_SHOWICON        5012
#define IDM_SHOWLWDATE      5013
#define IDM_SHOWLWTIME      5014
#define IDM_SHOWLADATE      5015
#define IDM_SHOWLATIME      5016
#define IDM_SHOWCRDATE      5017
#define IDM_SHOWCRTIME      5018
#define IDM_SHOWATTR        5019
#define IDM_DETAILSSETUP    5020
#define IDM_RESCAN          5021
#define IDM_LOADLISTFILE    5022
#define IDM_SAVELISTFILE    5023

#define IDM_HIDETOOL        6000
#define IDM_HIDEANYTOOL     6001
#define IDM_DELETETOOL      6002
#define IDM_DELETEANYTOOL   6003
#define IDM_SHOWTOOLS       6004
#define IDM_EDITTOOL        6005
#define IDM_EDITANYTOOL     6006
#define IDM_ADDTOOL         6007
#define IDM_REORDERTOOLS    6008
#define IDM_CREATETOOL      6009
#define IDM_TOOLLEFT        6010
#define IDM_TOOLRIGHT       6011
#define IDM_SAVETOOLS       6012
#define IDM_LOADTOOLS       6013

#define IDM_WINDOWSMENU     9000
#define IDM_TILE            9001
#define IDM_CASCADE         9002
#define IDM_RESTORE         9003
#define IDM_MINIMIZE        9004
#define IDM_NEXTWINDOW      9005
#define IDM_PREVWINDOW      9006
#define IDM_ARRANGEICONS    9007
#define IDM_AUTOTILE        9008
#define IDM_FREETREE        9009
#define IDM_WINDOWDLG       9010
#define IDM_IDEALSIZE       9011
#define IDM_SWITCHLIST      9012
#define IDM_TILEMENU        9013
#define IDM_WINDOWSTART     9100        /* Allow for 499 windows */
#define IDM_SWITCHSTART     9500        /* Allow for 499 switch entries */

#define IDM_PARENT          10000
#define IDM_PREVIOUS        10001
#define IDM_WALKDIR         10002
#define IDM_SWITCH          10003
#define IDM_COLLAPSE        10004
#define IDM_EXPAND          10005

#define IDM_SELECTALL       10006
#define IDM_DESELECTALL     10007
#define IDM_SELECTALLFILES  10008
#define IDM_DESELECTALLFILES 10009
#define IDM_SELECTALLDIRS   10010
#define IDM_DESELECTALLDIRS 10011
#define IDM_SELECTMASK      10012
#define IDM_DESELECTMASK    10013
#define IDM_INVERT          10014
#define IDM_SELECTBOTH      10015
#define IDM_SELECTONE       10016
#define IDM_SELECTNEWER     10017
#define IDM_SELECTOLDER     10018
#define IDM_SELECTBIGGER    10019
#define IDM_SELECTSMALLER   10020
#define IDM_DESELECTBOTH    10021
#define IDM_DESELECTONE     10022
#define IDM_DESELECTNEWER   10023
#define IDM_DESELECTOLDER   10024
#define IDM_DESELECTBIGGER  10025
#define IDM_DESELECTSMALLER 10026
#define IDM_RESELECT        10027
#define IDM_SELECTCLIP      10028
#define IDM_DESELECTCLIP    10029
#define IDM_SELECTIDENTICAL 10030
#define IDM_SELECTSAME      10031
#define IDM_COLLAPSEALL     10032
#define IDM_SELECTLIST      10033
#define IDM_SELECTMORE      10034
#define IDM_DESELECTMORE    10035
#define IDM_HIDEALL         10036
#define IDM_SELECTCOMPAREMENU 10037
#define IDM_SELECTSAMECONTENT 10038

#define CHECK_FILES 1

#ifndef MM_PORTHOLEINIT
#  define MM_PORTHOLEINIT   0x01fb
#endif
#ifndef MS_POPUP
#  define MS_POPUP          0x00000010L
#endif
#ifndef CCS_MINIICONS
#  define CCS_MINIICONS     0x0800
#endif
#ifndef CRA_SOURCE
#  define CRA_SOURCE        0x00004000
#endif
#ifndef CV_EXACTMATCH
#  define CV_EXACTMATCH     0x10000000
#endif
#ifndef CBN_SETFOCUS
#  define CBN_SETFOCUS      20
#endif
#ifndef CBN_KILLFOCUS
#  define CBN_KILLFOCUS     21
#endif
#ifndef CN_VERIFYEDIT
#  define CN_VERIFYEDIT     134
#endif
#ifndef CN_PICKUP
#  define CN_PICKUP         135
#endif
#ifndef CN_DROPNOTIFY
#  define CN_DROPNOTIFY     136
#endif
#ifndef CN_GRIDRESIZED
#  define CN_GRIDRESIZED    137
#endif
#ifndef BKS_MERLINSTYLE
#  define BKS_MERLINSTYLE   0x0800
#endif
