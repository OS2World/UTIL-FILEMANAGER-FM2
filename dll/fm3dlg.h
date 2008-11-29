
/***********************************************************************

  $Id$

  Dialog item definesions

  Copyright (c) 1993-02 M. Kimes
  Copyright (c) 2002, 2008 Steven H.Levine

  15 Oct 02 MK Baseline
  16 Oct 02 SHL Localize
  29 May 06 SHL Add Edit Archiver enhancements
  05 Jul 06 SHL Add Hide not selected support
  10 Sep 06 GKY Add Assoc enhancements
  23 Sep 06 GKY Add Commands enhancements
  02 Jan 07 GKY Add always paths opt to extract
  07 Jan 07 GKY Add remember search flags to seek and scan
  21 Aug 07 GKY Make Subject column in dircnr sizable and movable from the right to the left pane
  11 Jan 08 SHL Correct all to 3 column tabs since dialog editors assume this
  29 Feb 08 GKY Changes to enable user settable command line length
  20 Jul 08 JBS Ticket 114: Support user-selectable env. strings in Tree container.
  29 Nov 08 GKY Add the option of creating a subdirectory from the arcname
                for the extract path.

***********************************************************************/

#include "fm3hlp.h"

#define MSK_FRAME											20100
#define MSK_LISTBOX										20101
#define MSK_MASK											20102
#define MSK_DELETE										20103
#define MSK_HIDDEN										20104
#define MSK_SYSTEM										20105
#define MSK_READONLY										20106
#define MSK_ARCHIVED										20107
#define MSK_MUSTHIDDEN									20108
#define MSK_MUSTSYSTEM									20109
#define MSK_MUSTREADONLY								20110
#define MSK_MUSTARCHIVED								20111
#define MSK_SHOWDIRS										20112
#define MSK_CLEAR											20113
#define MSK_DIRECTORY									20114
#define MSK_MUSTDIRECTORY								20115
#define MSK_ALL											20116
#define MSK_TEXT											20117

#define ENV_FRAME											20150
#define ENV_NAME											20151
#define ENV_LISTBOX										20152

#define EXEC_FRAME										20200
#define EXEC_CL											20201
#define EXEC_DEFAULT										20202
#define EXEC_MINIMIZED									20203
#define EXEC_MAXIMIZED									20204
#define EXEC_FULLSCREEN									20205
#define EXEC_INVISIBLE									20206
#define EXEC_KEEP											20207
#define EXEC_ABORT										20208
#define EXEC_WARNING										20209
#define EXEC_WARNING2									20210
#define EXEC_ENVIRON										20211
#define EXEC_DROP											20212
#define EXEC_LISTBOX										20213
#define EXEC_SAVECMD										20214

#define EXEC2_FRAME										20250
#define EXEC2_LISTBOX									20251
#define EXEC2_DEL											20252
#define EXEC2_CLR											20253
#define EXEC2_KEEP										20254
#define EXEC2_SAVE										20255
#define EXEC2_FILTER										20256
#define EXEC2_CLOSE										20257
#define EXEC2_OPEN										20258

#define GREP_FRAME										20300
#define GREP_MASK											20301
#define GREP_SEARCH										20302
#define GREP_ALLHDS										20303
#define GREP_RECURSE										20304
#define GREP_ABSOLUTE									20305
#define GREP_CASE											20306
#define GREP_SAYFILES									20307
#define GREP_SEARCHFILES								20308
#define GREP_SEARCHEAS									20309
#define GREP_GREATER										20310
#define GREP_LESSER										20311
#define GREP_NEWER										20312
#define GREP_OLDER										20313
#define GREP_GK											20314
#define GREP_LK											20315
#define GREP_NK											20316
#define GREP_OM											20317
#define GREP_NM											20318
#define GREP_HELP											20319
#define GREP_FINDDUPES									20320
#define GREP_CRCDUPES									20321
#define GREP_NOSIZEDUPES								20322
#define GREP_IGNOREEXTDUPES							20323
#define GREP_LISTBOX										20324
#define GREP_DELETE										20325
#define GREP_ADD											20326
#define GREP_WALK											20327
#define GREP_FINDIFANY									20328
#define GREP_DRIVELIST									20329
#define GREP_LOCALHDS									20330
#define GREP_REMOTEHDS									20331
#define GREP_ENV											20332
#define GREP_APPEND										20333
#define GREP_REMEMBERFLAGS								20334

#define INFO_FRAME										20400
#define INFO_FS											20401
#define INFO_LABEL										20402
#define INFO_TOTAL										20403
#define INFO_AVAILABLE									20404
#define INFO_ALLOCUNITS									20405
#define INFO_SERIAL										20406
#define INFO_FLAGS										20407
#define INFO_FREE											20408
#define INFO_USED											20409
#define INFO_USEDPERCENT								20410
#define INFO_FREEPERCENT								20411
#define INFO_REALPATH									20412

#define EXT_FRAME											20500
#define EXT_MASK											20501
#define EXT_NORMAL										20502
#define EXT_WDIRS											20503
#define EXT_COMMAND										20504
#define EXT_FILENAME										20505
#define EXT_DIRECTORY									20506
#define EXT_PICK											20507
#define EXT_SEE											20508
#define EXT_WALK											20509
#define EXT_HELP											20510
#define EXT_REMEMBER										20511
#define EXT_AWDIRS										20512
#define EXT_FILENAMEEXT                                                                         20513

#define ARCH_FRAME										20600
#define ARCH_ARCNAME										20601
#define ARCH_ADD											20602
#define ARCH_MOVE											20603
#define ARCH_INCLPATH									20604
#define ARCH_COMMAND										20605
#define ARCH_MASKS										20606
#define ARCH_RECURSE										20607
#define ARCH_SEE											20608
#define ARCH_HELP											20609
#define ARCH_REMEMBER									20610
#define ARCH_FIND											20611

#define ASEL_FRAME										20700
#define ASEL_EDIT_FRAME									20701
#define ASEL_LISTBOX										20702
#define ASEL_PB_ADD										20703
#define ASEL_PB_DELETE									20704
#define ASEL_PB_UP										20705
#define ASEL_PB_DOWN										20706
#define ASEL_PB_REVERT									20707

#define WALK_FRAME										20800
#define WALK_DIRLIST										20801
#define WALK_DRIVELIST									20802
#define WALK_PATH											20803
#define WALK_USERLIST									20804
#define WALK_ADD											20805
#define WALK_DELETE										20806
#define WALK_HELP											20807
#define WALK_RECENT										20808

#define WALK2_FRAME										20850
#define WALK2_PATH										20851
#define WALK2_DRIVELIST									20852
#define WALK2_DIRLIST									20853

#define AD_FRAME											20900
#define AD_ID												20901
#define AD_STARTLIST										20902
#define AD_ENDLIST										20903
#define AD_ADD												20904
#define AD_MOVE											20905
#define AD_EXT												20906
#define AD_EXTRACT										20907
#define AD_WDIRS											20908
#define AD_SIG												20909
#define AD_LIST											20910
#define AD_TEST											20911
#define AD_DELETE											20912
#define AD_SIGPOS											20913
#define AD_FNAMEPOS										20914
#define AD_NUMDATEFLDS									20915
#define AD_DATEPOS										20916
#define AD_NEWSZ											20917
#define AD_OLDSZ											20918
#define AD_MOVEWPATHS									20919
#define AD_ADDRECURSE									20920
#define AD_ADDWPATHS										20921
#define AD_LISTBOX										20922
#define AD_TOSTART										20923
#define AD_TOEND											20924
#define AD_FLD1											20925
#define AD_FLD2											20926
#define AD_FLD3											20927
#define AD_FLD4											20928
#define AD_FLD5											20929
#define AD_FLD6											20930
#define AD_FLD7											20931
#define AD_FLD8											20932
#define AD_FLD9											20933
#define AD_FLD10											20934
#define AD_SEEADDER										20935
#define AD_SEEEXTRACTOR									20936
#define AD_HELP											20937
#define AD_NAMEISLAST									20938

#define COMP_FRAME										21000
#define COMP_LEFTDIR										21001
#define COMP_RIGHTDIR									21002
#define COMP_COLLECT										21003
#define COMP_VIEW											21004
#define COMP_NOTE											21005
#define COMP_TOTALLEFT									21006
#define COMP_SELLEFT										21007
#define COMP_TOTALRIGHT									21008
#define COMP_SELRIGHT									21009
#define COMP_CNRMENU										21010
#define COMP_DIRMENU										21011
#define COMP_MENU											21012
#define COMP_INCLUDESUBDIRS							21013
#define COMP_SETDIRS										21014
#define COMP_COPYLEFT									21015
#define COMP_MOVELEFT									21016
#define COMP_DELETELEFT									21017
#define COMP_COPYRIGHT									21018
#define COMP_MOVERIGHT									21019
#define COMP_DELETERIGHT								21020
#define COMP_TOTALLEFTHDR								21021
#define COMP_SELLEFTHDR									21022
#define COMP_TOTALRIGHTHDR								21023
#define COMP_SELRIGHTHDR								21024
#define COMP_FILTER										21025
#define COMP_HIDENOTSELECTED							21026

#define EA_FRAME											21100
#define EA_LISTBOX										21101
#define EA_TEXT											21102
#define EA_ENTRY											21103
#define EA_MLE												21104
#define EA_CHANGE											21105
#define EA_DELETE											21106
#define EA_HEXDUMP										21107
#define EA_ADD												21108
#define EA_NAMES											21109
#define EA_HELP											21110

#define EAC_FRAME											21200
#define EAC_TEXT											21201
#define EAC_NAME											21202
#define EAC_ASCII											21203
#define EAC_MVST											21204
#define EAC_MVMT											21205

#define DSZ_FRAME											21300
#define DSZ_CNR											21301
#define DSZ_EXPAND										21302
#define DSZ_COLLAPSE										21303
#define DSZ_FREESPACE									21304
#define DSZ_NUMFILES										21305
#define DSZ_PRINT											21306

#define IAD_FRAME											21400
#define IAD_APPNAME										21401
#define IAD_KEYNAME										21402
#define IAD_DATA											21403
#define IAD_ISBINARY										21404

#define CHECK_FRAME										21500
#define CHECK_LISTBOX									21501
#define CHECK_PROMPT										21502
#define CHECK_HELP										21503
#define CHECK_INFO										21504
#define CHECK_BITMAP										21505

#define STR_FRAME											21600
#define STR_PROMPT										21601
#define STR_INPUT											21602

#define MLE_POPUP											21699
#define MLE_FRAME											21700
#define MLE_MLE											21701
#define MLE_TOGWRAP										21702
#define MLE_STRIPTRAILBLANKS							21703
#define MLE_STRIPTRAILLINES							21704
#define MLE_EXPANDTABS									21705
#define MLE_SENSITIVE									21706
#define MLE_CODEPAGE										21707
#define MLE_SETEXPORTFILE								21708
#define MLE_EXPORTFILE									21709
#define MLE_LOADFILE										21710
#define MLE_INSERTFILE									21711
#define MLE_TAB											21712
#define MLE_FORMAT										21713
#define MLE_XOR											21714
#define MLE_ROT13											21715
#define MLE_UPPERCASE									21716
#define MLE_LOWERCASE									21717
#define MLE_TOGGLECASE									21718
#define MLE_JUMP											21719
#define MLE_CUTLINE										21720
#define MLE_CLEAR											21721
#define MLE_QUIT											21722
#define MLE_SETFONT										21723
#define MLE_SELECTALL									21724
#define MLE_DESELECTALL									21725
#define MLE_UNDO											21726
#define MLE_COPYCLIP										21727
#define MLE_CUTCLIP										21728
#define MLE_PASTECLIP									21729
#define MLE_FINDFIRST									21730
#define MLE_FINDNEXT										21731
#define MLE_END											21732
#define MLE_NEWFILE										21733
#define MLE_FILEMENU										21734
#define MLE_BLOCKMENU									21735
#define MLE_WRITEBLOCK									21736
#define MLE_ABOUT											21737
#define MLE_TOGGLEREADONLY								21738
#define MLE_APPENDCLIP									21739
#define MLE_EDITMENU										21740
#define MLE_VIEWHTTP										21741
#define MLE_VIEWFTP										21742
#define MLE_EXPORTAS										21743

#define SRCH_FRAME										21800
#define SRCH_HELP											21801
#define SRCH_SANDR										21802
#define SRCH_RALL											21803
#define SRCH_REPLACE										21804
#define SRCH_SEARCH										21805

#define PICK_FRAME										21900
#define PICK_LISTBOX										21901
#define PICK_HELP											21902
#define PICK_SAVEPOS										21903
#define PICK_INPUT										21904

#define SAV_FRAME											22000
#define SAV_PATTERN										22001
#define SAV_FILENAME										22002
#define SAV_FIND											22003
#define SAV_LISTBOX										22004
#define SAV_ADD											22005
#define SAV_DEL											22006
#define SAV_APPEND										22007

#define ASS_FRAME											22100
#define ASS_LISTBOX										22101
#define ASS_MASK											22102
#define ASS_CL												22103
#define ASS_ADD											22104
#define ASS_DELETE										22105
#define ASS_FULLSCREEN									22106
#define ASS_MINIMIZED									22107
#define ASS_MAXIMIZED									22108
#define ASS_INVISIBLE									22109
#define ASS_ICON											22110
#define ASS_DEFAULT										22111
#define ASS_PROMPT										22112
#define ASS_SIG											22113
#define ASS_OFFSET										22114
#define ASS_KEEP											22115
#define ASS_FIND											22116
#define ASS_ENVIRON										22117
#define ASS_DIEAFTER										22118
#define ASS_TOP											22119

#define ATR_FRAME											22200
#define ATR_YEAR											22201
#define ATR_MONTH											22202
#define ATR_DAY											22203
#define ATR_HOUR											22204
#define ATR_MINUTES										22205
#define ATR_SECONDS										22206
#define ATR_READONLY										22207
#define ATR_ARCHIVED										22208
#define ATR_HIDDEN										22209
#define ATR_SYSTEM										22210
#define ATR_FILENAME										22211
#define ATR_ICON											22212
#define ATR_FILESIZE										22213
#define ATR_SUBJ											22214
#define ATR_EAS											22215
#define ATR_USEDATETIME									22216
#define ATR_LISTBOX										22217
#define ATR_HELP											22218
#define ATR_NOW											22219
#define ATR_LEAVEALL										22220

#define REN_FRAME											22300
#define REN_SOURCE										22301
#define REN_SOURCEINFO									22302
#define REN_TARGET										22303
#define REN_TARGETINFO									22304
#define REN_INFORMATION									22305
#define REN_DONTASK										22306
#define REN_OVERWRITE									22307
#define REN_SKIP											22308
#define REN_OVEROLD										22309
#define REN_OVERNEW										22310
#define REN_RENEXIST										22311

#define FLE_FRAME											22400
#define FLE_NAME											22401
#define FLE_LASTWRITE									22402
#define FLE_CREATE										22403
#define FLE_LASTACCESS									22404
#define FLE_SIZES											22405
#define FLE_SLACK											22406
#define FLE_READONLY										22407
#define FLE_ARCHIVED										22408
#define FLE_DIRECTORY									22409
#define FLE_HIDDEN										22410
#define FLE_SYSTEM										22411
#define FLE_READABLE										22412
#define FLE_WRITEABLE									22413
#define FLE_OS2FS											22414
#define FLE_OS2WIN										22415
#define FLE_OS2PM											22416
#define FLE_DOS											22417
#define FLE_32BIT											22418
#define FLE_WIN											22419
#define FLE_BOUND											22420
#define FLE_WINREAL										22421
#define FLE_WINPROT										22422
#define FLE_WINENH										22423
#define FLE_DLL											22424
#define FLE_PHYSDRV										22425
#define FLE_VIRTDRV										22426
#define FLE_PROTDLL										22427
#define FLE_ICON											22428
#define FLE_EAS											22429
#define FLE_OPEN											22430
#define FLE_ISARCHIVE									22431
#define FLE_ARCNAME										22432
#define FLE_SETTINGS										22433
#define FLE_BINARY										22434

#define SETICON_FRAME									22500
#define SETICON_SPTR_ARROW								22501
#define SETICON_SPTR_TEXT								22502
#define SETICON_SPTR_WAIT								22503
#define SETICON_SPTR_SIZE								22504
#define SETICON_SPTR_MOVE								22505
#define SETICON_SPTR_SIZENWSE							22506
#define SETICON_SPTR_SIZENESW							22507
#define SETICON_SPTR_SIZEWE							22508
#define SETICON_SPTR_SIZENS							22509
#define SETICON_SPTR_APPICON							22510
#define SETICON_SPTR_ICONINFORMATION				22511
#define SETICON_SPTR_ICONQUESTION					22512
#define SETICON_SPTR_ICONERROR						22513
#define SETICON_SPTR_ICONWARNING						22514

#define SETICON_SPTR_ILLEGAL							22518
#define SETICON_SPTR_FILE								22519
#define SETICON_SPTR_FOLDER							22520
#define SETICON_SPTR_MULTFILE							22521
#define SETICON_SPTR_PROGRAM							22522

#define OBJCNR_FRAME										22600
#define OBJCNR_CNR										22601
#define OBJCNR_DIR										22602
#define OBJCNR_DESKTOP									22603
#define OBJCNR_NOTE										22604

#define RE_FRAME											22700
#define RE_ORIG											22701
#define RE_ADDLISTBOX									22702
#define RE_ADD												22703
#define RE_REMOVELISTBOX								22704
#define RE_REMOVE											22705

#define ADDBTN_FRAME										22800
#define ADDBTN_TEXT										22801
#define ADDBTN_HELP										22802
#define ADDBTN_DROPABLE									22803
#define ADDBTN_VISIBLE									22804
#define ADDBTN_ID											22805
#define ADDBTN_BITMAP									22806
#define ADDBTN_SHOWTEXT									22807
#define ADDBTN_SEPARATOR								22808
#define ADDBTN_HELPME									22809
#define ADDBTN_MYICON									22810
#define ADDBTN_EDITBMP									22811
#define ADDBTN_BMP										22812

#define PICKBTN_FRAME									22900
#define PICKBTN_LISTBOX									22901

#define CMD_FRAME											23000
#define CMD_LISTBOX										23001
#define CMD_CL												23002
#define CMD_ADD											23003
#define CMD_DELETE										23004
#define CMD_KEEP											23005
#define CMD_FULLSCREEN									23006
#define CMD_MINIMIZED									23007
#define CMD_MAXIMIZED									23008
#define CMD_INVISIBLE									23009
#define CMD_ICON											23010
#define CMD_DEFAULT										23011
#define CMD_PROMPT										23012
#define CMD_ONCE											23013
#define CMD_TITLE											23014
#define CMD_REORDER										23015
#define CMD_FIND											23016
#define CMD_ENVIRON										23017

#define KILL_FRAME										23100
#define KILL_LISTBOX										23101
#define KILL_RESCAN										23102
#define KILL_CHECKBOX									23103
#define KILL_SHOW											23104
#define KILL_HDR											23105
#define KILL2_CHECKBOX									23106

#define UNDEL_FRAME										23200
#define UNDEL_LISTBOX									23201
#define UNDEL_DEL											23202
#define UNDEL_DRIVELIST									23203
#define UNDEL_DEBUG										23204
#define UNDEL_ENTRY										23205
#define UNDEL_SUBDIRS									23206
#define UNDEL_COUNT										23207
#define UNDEL_MASKHDR									23208
#define UNDEL_DRVHDR										23209

#define BAT_FRAME											23300
#define BAT_MLE											23301

#define SYS_FRAME											23400
#define SYS_LISTBOX										23401

#define ABT_FRAME											23600
#define ABT_ICON											23601
#define ABT_VERSION										23605
#define ABT_STICK1										23606
#define ABT_STICK2										23607
#define ABT_PROGNAME										23608

#define NOTE_FRAME										23800
#define NOTE_MAX											23898
#define NOTE_LISTBOX										23801

#define WLIST_FRAME										23900
#define WLIST_LISTBOX									23901
#define WLIST_MINIMIZE									23902
#define WLIST_CLOSE										23903

#define QTREE_FRAME										24000

#define SVBTN_FRAME										24100
#define SVBTN_LISTBOX									24101
#define SVBTN_ENTRY										24102
#define SVBTN_CURRENT									24103

#define VINF_FRAME										24200
#define VINF_LISTBOX										24201
#define VINF_ENTRY										24202
#define VINF_SRCH											24203
#define VINF_FILTER										24204
#define VINF_DIRS											24205
#define VINF_RESCAN										24206
#define VINF_DEBUG										24207
#define VINF_TOPIC										24208
#define VINF_TOPICHDR									24209

#define INIR_FRAME										24300
#define INIR_USERPROFILE								24301
#define INIR_SYSTEMPROFILE								24302
#define INIR_FIND											24303

#define INII_FRAME										24400
#define INII_NEWAPP										24401
#define INII_NEWKEY										24402
#define INII_NEWKEYHDR									24403
#define INII_OLDAPP										24404
#define INII_OLDKEY										24405

#define CFG_FRAME											25000
#define CFG_NOTEBOOK										25001

#define CFGT_FRAME										25050
#define CFGT_SWITCHTREEONFOCUS						25051
#define CFGT_SWITCHTREE									25052
#define CFGT_SWITCHTREEEXPAND							25053
#define CFGT_COLLAPSEFIRST								25054
#define CFGT_DCOPENS										25055
#define CFGT_FOLLOWTREE									25056
#define CFGT_VTREEOPENSWPS								25057
#define CFGT_TOPDIR										25058
#define CFGT_SHOWENV										25059
#define CFGT_ENVVARLIST 									25060

#define CFGC_FRAME										25100
#define CFGC_COMPARE										25101
#define CFGC_DIRCOMPARE									25102
#define CFGC_FIND											25103

#define CFGD_FRAME										25150
#define CFGD_UNHILITE									25151
#define CFGD_SYNCUPDATES								25152
#define CFGD_LOOKINDIR									25153
#define CFGD_MINONOPEN									25154
#define CFGD_SELECTEDALWAYS							25155
#define CFGD_NOSEARCH									25156
#define CFGD_MULTIPLESEL								25157
#define CFGD_EXTENDEDSEL								25158
#define CFGD_LEAVETREE									25159
#define CFGD_NOFOLDMENU									25160

#define CFGG_FRAME										25200
#define CFGG_DONTMOVEMOUSE								25201
#define CFGG_IDLECOPY									25202
#define CFGG_CONFIRMDELETE								25203
#define CFGG_DEFAULTCOPY								25204
#define CFGG_LINKSETSICON								25205
#define CFGG_VERIFYWRITES								25206
#define CFGG_DNDDLG										25207
#define CFGG_DEFAULTDELETEPERM	  					25208
#define CFGG_PRINTER										25209
#define CFGG_NODEAD										25210
#define CFGG_BORING										25211
#define CFGG_CUSTOMFILEDLG								25212
#define CFGG_FM2DELETES									25213
#define CFGG_CONFIRMTARGET								25214
#define CFGG_TARGETDIR									25215
#define CFGG_CMDLNLNGTH                                                                 25216
#define CFGG_TRASHCAN                                                                   25217

#define CFGB_FRAME									25250
#define CFGB_TOOLBARHELP								25251
#define CFGB_DRIVEBARHELP								25252
#define CFGB_OTHERHELP									25253

#define CFGM_FRAME										25300
#define CFGM_SAVESTATE									25301
#define CFGM_AUTOTILE									25302
#define CFGM_FREETREE									25303
#define CFGM_SPLITSTATUS								25304
#define CFGM_NOTREEGAP									25305
#define CFGM_STARTMIN									25306
#define CFGM_STARTMAX									25307
#define CFGM_DATAMIN										25308
#define CFGM_TILEBACKWARDS								25309
#define CFGM_WSANIMATE									25310
#define CFGM_USERLISTSWITCHES							25311
#define CFGM_RECENTDIRS									25312
#define CFGM_EXTERNALARCBOXES							25313
#define CFGM_EXTERNALVIEWER							25314
#define CFGM_EXTERNALINIS								25315
#define CFGM_EXTERNALCOLLECTOR						25316
#define CFGM_SEPARATEPARMS								25317
#define CFGM_BLUELED										25318
#define CFGM_SHOWTARGET									25319

#define CFGV_FRAME										25330
#define CFGV_EDITOR										25331
#define CFGV_BINED										25332
#define CFGV_VIEWER										25333
#define CFGV_BINVIEW										25334
#define CFGV_FIND											25335
#define CFGV_USENEWVIEWER								25336
#define CFGV_GUESSTYPE									25337
#define CFGV_VIEWCHILD									25340
#define CFGV_CHECKMM										25341

#define CFGH_FRAME										25344
#define CFGH_NOMAILTOMAILRUN							25345
#define CFGH_HTTPRUN										25346
#define CFGH_FTPRUN										25347
#define CFGH_RUNFTPWORKDIR								25348
#define CFGH_RUNHTTPWORKDIR							25349
#define CFGH_RUNMAILWORKDIR							25351
#define CFGH_MAILRUN										25352
#define CFGH_HTTPRUNWPSDEFAULT						25354
#define CFGH_FTPRUNWPSDEFAULT							25355
#define CFGH_LIBPATHSTRICTHTTPRUN					25357
#define CFGH_LIBPATHSTRICTFTPRUN						25358
#define CFGH_FIND											25359
#define CFGH_LIBPATHSTRICTMAILRUN					25360

#define CFGS_FRAME										25360
#define CFGS_LOADSUBJECTS								25361
#define CFGS_LOADLONGNAMES								25362
#define CFGS_NOICONSFILES								25363
#define CFGS_NOICONSDIRS								25364
#define CFGS_FORCEUPPER									25365
#define CFGS_FORCELOWER									25366
#define CFGS_REMOTEBUG									25367
#define CFGS_NOREMOVABLESCAN							25368
#define CFGS_FILESTOGET									25369

#define CFGA_FRAME										25370
#define CFGA_QUICKARCFIND								25371
#define CFGA_DEFARCNAME									25372
#define CFGA_DEFARC										25373
#define CFGA_FOLDERAFTEREXTRACT						25374
#define CFGA_ARCSTUFFVISIBLE							25375
#define CFGA_VIRUS										25376
#define CFGA_EXTRACTPATH								25377
#define CFGA_FIND											25378

#define CFG5_FRAME										25500
#define CFG5_ICON											25501
#define CFG5_DETAIL										25502
#define CFG5_NAME											25503
#define CFG5_TEXT											25504
#define CFG5_MINIICONS									25505
#define CFG5_SHOWTITLES									25506
#define CFG5_SHOWICON									25507
#define CFG5_SHOWLNAMES									25508
#define CFG5_SHOWSUBJECT								25509
#define CFG5_SHOWATTR									25510
#define CFG5_SHOWSIZE									25511
#define CFG5_SHOWEAS										25512
#define CFG5_SHOWLWDATE									25513
#define CFG5_SHOWLWTIME									25514
#define CFG5_SHOWLADATE									25515
#define CFG5_SHOWLATIME									25516
#define CFG5_SHOWCRDATE									25517
#define CFG5_SHOWCRTIME									25518
#define CFG5_FILTER										25519
#define CFG5_EXTERNALCOLLECTOR						25520
#define CFG5_SUBJECTLENGTHMAX							25521
#define CFG5_SUBJECTDISPLAYWIDTH						25522
#define CFG5_SUBJECTINLEFTPANE						25523

#define CFGTS_FRAME				25599

#define CFG6_FRAME										25600
#define CFG6_SORTNAME									25601
#define CFG6_SORTFILENAME								25602
#define CFG6_SORTSIZE									25603
#define CFG6_SORTEASIZE									25604
#define CFG6_SORTFIRST									25605
#define CFG6_SORTLAST									25606
#define CFG6_SORTLWDATE									25607
#define CFG6_SORTLADATE									25608
#define CFG6_SORTCRDATE									25609
#define CFG6_SORTREVERSE								25610
#define CFG6_SORTDIRSFIRST								25611
#define CFG6_SORTDIRSLAST								25612

#define CFG9_FRAME										25900
#define CFG9_MAXIMUMUI									25901
#define CFG9_MINIMUMUI									25902
#define CFG9_MAXINFOPRETTY								25903
#define CFG9_MAXINFOPLAIN								25904
#define CFG9_MAXFILENAMES								25905
#define CFG9_DEFAULT										25906
#define CFG9_MAXSPEED									25907
#define CFG9_1X											25908
#define CFG9_DOSTHINK									25909
#define CFG9_HECTOR										25910
#define CFG9_WINDOZETHINK								25911

#define CINI_FRAME										26000
#define CINI_FIRSTLIST									26001
#define CINI_CHANGEDLIST								26002
#define CINI_SECONDLIST									26003
#define CINI_FIRSTINI									26004
#define CINI_SECONDINI									26005

#define AUTHOR_FRAME										26100
#define AUTHOR_BITMAP									26101
#define AUTHOR_PICTURE									26102

#define ARCERR_FRAME										26200
#define ARCERR_TEXT										26201
#define ARCERR_TEST										26202
#define ARCERR_MLE										26203
#define ARCERR_VIEW										26204

#define SEEF_FRAME										26300
#define SEEF_LISTBOX										26301
#define SEEF_DIR											26302
#define SEEF_TOTAL										26303

#define DRVS_FRAME										26350
#define DRVS_LISTBOX										26351
#define DRVS_BYNAME										26352
#define DRVS_BYSIZE										26353
#define DRVS_BYDATE										26354

#define DND_FRAME											26400
#define DND_COPY											26401
#define DND_MOVE											26402
#define DND_SHADOW										26403
#define DND_COMPARE										26404
#define DND_SETICON										26405
#define DND_LISTBOX										26406
#define DND_HELP											26407
#define DND_TARGET										26408
#define DND_RENAME										26409
#define DND_OBJECT										26410
#define DND_WILDCOPY										26411
#define DND_WILDMOVE										26412
#define DND_LAUNCH										26413
#define DND_INFO											26414
#define DND_TARGETINFO									26415
#define DND_EXTRACT										26416
#define DND_APPEND										26417
#define DND_CHANGETARGET								26418

#define FCMP_FRAME										26500
#define FCMP_LISTBOX										26501
#define FCMP_HELP											26502

#define MINI_FRAME										26600
#define MINI_SWAP											26601
#define MINI_TIME											26602
#define MINI_MEM											26603
#define MINI_PROC											26604
#define MINI_SPOOL										26005
#define MINI_INCLREMOTE									26606
#define MINI_SHOW											26607
#define MINI_FLOAT										26608
#define MINI_CLOSE										26609
#define MINI_BORING										26610
#define MINI_DRIVEA										26620

#define NEWFIND_FRAME									26650
#define NEWFIND_MLE										26651
#define NEWFIND_SENSITIVE								26652
#define NEWFIND_LITERAL									26653
#define NEWFIND_ALSOSELECT								26654

#define COLOR_FRAME										26700
#define COLOR_FIRST										26710
#define COLOR_WHITE										26710
#define COLOR_BLACK										26711
#define COLOR_BLUE										26712
#define COLOR_RED											26713
#define COLOR_PINK										26714
#define COLOR_GREEN										26715
#define COLOR_CYAN										26716
#define COLOR_YELLOW										26717
#define COLOR_DARKGRAY									26718
#define COLOR_DARKBLUE									26719
#define COLOR_DARKRED									26720
#define COLOR_DARKPINK									26721
#define COLOR_DARKGREEN									26722
#define COLOR_DARKCYAN									26723
#define COLOR_BROWN										26724
#define COLOR_PALEGRAY									26725
#define COLOR_LAST										26725
#define COLOR_RECT										26730
#define COLOR_LISTBOX									26731

#define IAF_FRAME											26800
#define IAF_MLE											26801
#define IAF_PMFILTERS									26802
#define IAF_LISTBOX										26803
#define IAF_SAVE											26804
#define IAF_LOAD											26805
#define IAF_DELETE										26806
#define IAF_SAVENAME										26807
#define IAF_HELP											26808

#define FND_FRAME											26900
#define FND_LISTBOX										26901
#define FND_HELP											26902
#define FND_INSTRUCT										26903
#define FND_EDITASSOC									26904

#define DVS_FRAME											27000
#define DVS_REMOVABLE									27001
#define DVS_NOTWRITEABLE								27002
#define DVS_IGNORE										27003
#define DVS_CDROM											27004
#define DVS_NOLONGNAMES									27005
#define DVS_REMOTE										27006
#define DVS_BOOT											27007
#define DVS_INVALID										27008
#define DVS_NOPRESCAN									27009
#define DVS_ZIPSTREAM									27010
#define DVS_NOLOADICONS									27011
#define DVS_NOLOADSUBJS									27012
#define DVS_NOLOADLONGS									27013
#define DVS_SLOW											27014
#define DVS_INCLUDEFILES								27015
#define DVS_VIRTUAL										27016
#define DVS_NOSTATS										27017
#define DVS_RAMDISK										27018

#define MRG_FRAME											27100
#define MRG_LISTBOX										27101
#define MRG_TOP											27102
#define MRG_APPEND										27103
#define MRG_BINARY										27104
#define MRG_REMOVE										27105
#define MRG_HELP											27106
#define MRG_CHANGETARGET								27107
#define MRG_TARGETNAME									27108
#define MRG_BOTTOM										27109

#define PRN_FRAME											27150
#define PRN_WIDTH											27151
#define PRN_LENGTH										27152
#define PRN_LMARGIN										27153
#define PRN_RMARGIN										27154
#define PRN_TMARGIN										27155
#define PRN_BMARGIN										27156
#define PRN_PAGENUMS										27157
#define PRN_FORMBEFORE									27158
#define PRN_FORMAFTER									27159
#define PRN_ALT											27160
#define PRN_FORMAT										27161
#define PRN_LISTBOX										27162
#define PRN_BITMAP										27163
#define PRN_PRINTER										27164
#define PRN_SPACING										27165
#define PRN_TABSPACES									27166

#define FDLG_FRAME										27200
#define FDLG_USERDIRS									27201
#define FDLG_HELP											27202

#define DUPE_FRAME										27300
#define DUPE_NAMES										27301
#define DUPE_DATES										27302
#define DUPE_SIZES										27303
#define DUPE_CRCS											27304
#define DUPE_EXTS											27305

#define MAP_FRAME											27400
#define MAP_ATTACHLIST									27401
#define MAP_DETACHLIST									27402
#define MAP_ATTACHTO										27403
#define MAP_ATTACH										27404
#define MAP_DETACH										27405
#define MAP_INFO											27406
#define MAP_DELETE										27407
#define MAP_CLEAR											27408

#define URL_FRAME											27500
#define URL_LISTBOX										27501
#define URL_BOOKMARK										27502

#define IMGS_FRAME										27600
#define IMGS_LISTBOX										27601
#define IMGS_NAME											27602
#define ASS_BOTTOM										27602
#define ASS_REPLACE										27603
#define CMD_REPLACE										27604
