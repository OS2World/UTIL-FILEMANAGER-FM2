
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
  20 Jul 08 GKY Add IDM_SAVETOCLIPFILENAME & IDM_APPENDTOCLIPFILENAME
                to save/append just filename to clipboard
  01 Sep 08 GKY Add IDM_*TLS for icon resources for the default toolbars
  27 Dec 08 GKY Add refresh removable media to tree container menus
  28 Dec 08 GKY Added Databar to utilities menu
  27 Jun 09 GKY Remove IDM_DRIVEATEXT (3950) Drivebar update
  13 Jul 09 SHL Rename timer ids
  12 Sep 09 GKY Add FM3.INI User ini and system ini to submenu for view ini
  14 Sep 09 SHL Drop experimental code
  15 Sep 09 SHL Add rescan progress timer
  22 Nov 09 GKY Add LVM.EXE to partition submenu
  21 Dec 09 GKY Added 20 new hot keys for commands.
  23 Oct 10 GKY Changes to populate and utilize a HELPTABLE for context specific help
  04 Aug 12 GKY Changes to use Unlock to unlock files if Unlock.exe is in path both from menu/toolbar and as part of
                copy, move and delete operations

  Align with spaces only - no tabs please

***********************************************************************/

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

// 13 Jul 09 SHL fixme to know if timer ids unique to queue or window
// WM_TIMER timer ids - assume ids can be shared
#define ID_ABOUT_TIMER      4
#define ID_COMP_TIMER       4
#define ID_NOTIFY_TIMER     15
#define ID_ACTION_TIMER     16          // 13 Jul 09 SHL Added
#define ID_DIRCNR_TIMER     17		// 15 Sep 09 SHL Added
#define ID_NEWVIEW_TIMER    20
#define ID_LED_TIMER        21

// Window ids, etc.
#define MAIN_STATUS2        19

#define OBJ_FRAME           20

#define ID_IMAGE            40
#define ID_IMAGEOBJECT      49

#define ID_HELPTABLE        50
#define ID_HELPSUBTABLE     51
#define ID_HELPSUBTABLEA    52
#define ID_HELPSUBTABLEB    53
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
#define TREE_OPENCNR        303
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
#define DONNO_ICON          927

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
#define IDM_SAVETOCLIPFILENAME 1161
#define IDM_APPENDTOCLIPFILENAME 1162
#define IDM_DATABAR         1163
#define IDM_PARTITIONLVM    1164
#define IDM_OPENSUBCNRMENU  1165
#define IDM_OPENDIRICON     1166
#define IDM_OPENDIRDETAILS  1167
#define IDM_OPENDIRTREE     1168
#define IDM_OPENDIRWINDOW   1169
#define IDM_UNLOCKFILE      1170

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
#define IDM_INIVIEWERFM2    2016
#define IDM_INIVIEWERSUBMENU 2017
#define IDM_INIVIEWERSYS    2018

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
#define IDM_CFGEDITSUBMENU    3073
#define IDM_CFGPALETTESUBMENU 3074
#define IDM_REFRESHREMOVABLES 3075

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
// Drive button IDs use 3900 - 3925

#define IDM_COMMANDSMENU    4000
#define IDM_COMMANDSTART    4001
#define IDM_COMMANDNUM0     4301
#define IDM_COMMANDNUM1     4302
#define IDM_COMMANDNUM2     4303
#define IDM_COMMANDNUM3     4304
#define IDM_COMMANDNUM4     4305
#define IDM_COMMANDNUM5     4306
#define IDM_COMMANDNUM6     4307
#define IDM_COMMANDNUM7     4308
#define IDM_COMMANDNUM8     4309
#define IDM_COMMANDNUM9     4310
#define IDM_COMMANDNUM10    4311
#define IDM_COMMANDNUM11    4312
#define IDM_COMMANDNUM12    4313
#define IDM_COMMANDNUM13    4314
#define IDM_COMMANDNUM14    4315
#define IDM_COMMANDNUM15    4316
#define IDM_COMMANDNUM16    4317
#define IDM_COMMANDNUM17    4318
#define IDM_COMMANDNUM18    4319
#define IDM_COMMANDNUM19    4320
#define IDM_COMMANDNUM20    4321
#define IDM_COMMANDNUM21    4322
#define IDM_COMMANDNUM22    4323
#define IDM_COMMANDNUM23    4324
#define IDM_COMMANDNUM24    4325
#define IDM_COMMANDNUM25    4326
#define IDM_COMMANDNUM26    4327
#define IDM_COMMANDNUM27    4328
#define IDM_COMMANDNUM28    4329
#define IDM_COMMANDNUM29    4330
#define IDM_COMMANDNUM30    4331
#define IDM_COMMANDNUM31    4332
#define IDM_COMMANDNUM32    4333
#define IDM_COMMANDNUM33    4334
#define IDM_COMMANDNUM34    4335
#define IDM_COMMANDNUM35    4336
#define IDM_COMMANDNUM36    4337
#define IDM_COMMANDNUM37    4338
#define IDM_COMMANDNUM38    4339
#define IDM_COMMANDNUM39    4340

#define IDM_QUICKTOOLSTART  4899
#define IDM_CMDTOOLBAR      4900
#define IDM_UTILSTOOLBAR    4901
#define IDM_SORTTOOLBAR     4902
#define IDM_SELECTTOOLBAR   4903
#define IDM_CONFIGTOOLBAR   4904
#define IDM_FILESTOOLBAR    4905
#define IDM_VIEWSTOOLBAR    4906

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

#define IDM_RESCANALL       7000
#define IDM_REFRESHREMOVABLE 7001

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
#define IDM_UNHIDEALL         10039

// #define PP_MAX    PP_MENUDISABLEBGNDCOLORINDEX       // Unused, 13 Sep 08 JBS
// #define PP_MAXBUF 384                                // Unused, 13 Sep 08 JBS

#ifndef MM_PORTHOLEINIT
#define MM_PORTHOLEINIT   0x01fb
#endif
// #ifndef CCS_MINIICONS     defined in Open Watcom headers
// #define CCS_MINIICONS     0x0800
// #endif
// #ifndef CRA_SOURCE                   // defined in Open Watcom headers
// #define CRA_SOURCE        0x00004000
// #endif
// #ifndef CV_EXACTMATCH                // defined as CV_EXACTLENGTH in Open Watcom headers
// #define CV_EXACTMATCH     0x10000000
// #endif
// #ifndef CBN_SETFOCUS                 // Unused, 13 Sep 08 JBS
// #define CBN_SETFOCUS      20
// #endif
// #ifndef CBN_KILLFOCUS                // Unused, 13 Sep 08 JBS
// #define CBN_KILLFOCUS     21
// #endif
// #ifndef CN_VERIFYEDIT                // defined in Open Watcom headers
// #define CN_VERIFYEDIT     134
// #endif
// #ifndef CN_PICKUP                    // defined in Open Watcom headers
// #define CN_PICKUP         135
// #endif
// #ifndef CN_DROPNOTIFY                // defined in Open Watcom headers
// #define CN_DROPNOTIFY     136
// #endif
// #ifndef CN_GRIDRESIZED               // defined in Open Watcom headers
// #define CN_GRIDRESIZED    137
// #endif
#ifndef BKS_MERLINSTYLE
#define BKS_MERLINSTYLE   0x0800
#endif

// User messages
// WM_USER used in AVSSHELL.H & ODRES.H (IBM Toolkit)
// WM_USER +1 used in ODRES.H (IBM Toolkit)
#define UM_RESCAN           (WM_USER + 2)
#define UM_INITIALSIZE      (WM_USER + 3)
#define UM_CONTROL          (WM_USER + 4)
#define UM_COMMAND          (WM_USER + 5)
#define UM_SIZE             (WM_USER + 6)
#define UM_FOCUSME          (WM_USER + 7)
#define UM_FIXEDITNAME      (WM_USER + 8)
#define UM_UPDATERECORD     (WM_USER + 9)
// WM_USER +10 used in DSRES.H (IBM Toolkit)
#define UM_CONTAINER_FILLED (WM_USER + 11)
#define UM_STRETCH          (WM_USER + 12)
#define UM_LOADFILE         (WM_USER + 13)
#define UM_MOUSEMOVE        (WM_USER + 14)
#define UM_ENTER            (WM_USER + 15)
#define UM_CLOSE            (WM_USER + 16)
#define UM_ACTION           (WM_USER + 17)
#define UM_MASSACTION       (WM_USER + 18)
#define UM_UPDATERECORDLIST (WM_USER + 19)
#define UM_FILESMENU        (WM_USER + 20)
#define UM_SELECT           (WM_USER + 21)
#define UM_VIEWSMENU        (WM_USER + 22)
#define UM_CONTAINERHWND    (WM_USER + 23)
#define UM_OPENWINDOWFORME  (WM_USER + 24)
#define UM_FOLDUP           (WM_USER + 25)
#define UM_INITMENU         (WM_USER + 26)
#define UM_COMPARE          (WM_USER + 27)
#define UM_EXPAND           (WM_USER + 28)
#define UM_REPLACEFOCUS     (WM_USER + 29)
#define UM_UNDO             (WM_USER + 30)
#define UM_RENDER           (WM_USER + 31)
// #define UM_BUTTON2DOWN      (WM_USER + 32)           // Unused, 13 Sep 08 JBS
// #define UM_BUTTON2UP        (WM_USER + 33)           // Unused, 13 Sep 08 JBS
#define UM_COLLECTFROMFILE  (WM_USER + 34)
#define UM_TIMER            (WM_USER + 35)              // 13 Jul 09 SHL fixme to have better name
// #define UM_HELPON           (WM_USER + 36)           // Unused, 13 Sep 08 JBS
#define UM_SETUP2           (WM_USER + 37)
#define UM_SETUP3           (WM_USER + 38)
#define UM_CONTEXTMENU      (WM_USER + 39)
// WM_USER + 40-42 used in PMSTDDLG.h (IBM TOOLKIT and Open Watcom)
#define UM_FILLSETUPLIST    (WM_USER + 43)
#define UM_ARRANGEICONS     (WM_USER + 44)
#define UM_SETUP5           (WM_USER + 45)
#define UM_NOTIFY           (WM_USER + 46)
// #define UM_INSERTRECORD     (WM_USER + 47)           // Unused, 13 Sep 08 JBS
#define UM_ADDTOMENU        (WM_USER + 48)
#define UM_COLLECT          (WM_USER + 49)
// WM_USER + 50-55 used in PMSTDDLG.h (IBM TOOLKIT and Open Watcom)
#define UM_SETUSERLISTNAME  (WM_USER + 56)
#define UM_FILTER           (WM_USER + 57)
#define UM_SORTRECORD       (WM_USER + 58)
// #define UM_SIZE2            (WM_USER + 59)           // Unused, 13 Sep 08 JBS
#define UM_RESTORE          (WM_USER + 60)
#define UM_TOPDIR           (WM_USER + 61)
#define UM_SHOWME           (WM_USER + 62)
#define UM_RESCAN2          (WM_USER + 63)
#define UM_BUILDDRIVEBAR    (WM_USER + 64)
#define UM_THREADUSE        (WM_USER + 65)
#define UM_DRIVECMD         (WM_USER + 66)
#define UM_ADVISEFOCUS      (WM_USER + 67)
#define UM_FIXCNRMLE        (WM_USER + 68)
#define UM_FLESH            (WM_USER + 69)
#define UM_FILLCMDLIST      (WM_USER + 70)
#define UM_CLICKED          (WM_USER + 71)
#define UM_CLICKED3         (WM_USER + 72)
#define UM_HIDENOTSELECTED  (WM_USER + 73)
#define UM_FIRSTTIME        (WM_USER + 74)
#define UM_GREP             (WM_USER + 75)   // 15 Sep 09 SHL
#define UM_FILLUSERLIST     (WM_USER + 75)
#define UM_CONTAINERDIR     (WM_USER + 76)
#define UM_SETUP4           (WM_USER + 77)
#define UM_RESTOREDC        (WM_USER + 78)
#define UM_MINIMIZE         (WM_USER + 79)
#define UM_MAXIMIZE         (WM_USER + 80)
#define UM_BUTTON1MOTIONSTART (WM_USER + 81)
#define UM_SETUP6           (WM_USER + 82)
#define UM_FILLBUTTONLIST   (WM_USER + 83)
#define UM_PAINT            (WM_USER + 84)
#define UM_SETUP            (WM_USER + 85)
#define UM_SETDIR           (WM_USER + 86)
// WM_USER + 1000 used in FTPAPI.H (IBM Toolkit)

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


