
/***********************************************************************

  $Id$

  Edit ICON EA

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2006 Steven H.Levine

  17 Jul 06 SHL Use Runtime_Error

***********************************************************************/

#define INCL_DOS
#define INCL_WIN
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <share.h>

#include "fm3dll.h"
#include "fm3dlg.h"

#pragma data_seg(DATA2)

static PSZ pszSrcFile = __FILE__;

#pragma alloc_text(MENU,SetIconDlgProc)


MRESULT EXPENTRY SetIconDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2)
{
  switch(msg) {
    case WM_INITDLG:
      WinSetWindowPtr(hwnd,0,(PVOID)mp2);
      WinCheckButton(hwnd,SETICON_SPTR_ARROW,TRUE);
      break;

    case WM_CONTROL:
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case DID_OK:
          {
            CHAR    *filename = WinQueryWindowPtr(hwnd,0),*buff = NULL;
            ICONINFO icf;
            ULONG    icid = SPTR_ARROW;
            INT      x;
            HWND     hwndDeskTop;
            FILE    *fp;

            hwndDeskTop = WinQueryDesktopWindow(WinQueryAnchorBlock(hwnd),
                                                NULLHANDLE);
            memset(&icf,0,sizeof(ICONINFO));
            icf.cb = sizeof(ICONINFO);
            icf.fFormat = ICON_DATA;
            if (filename && *filename) {
              fp = xfsopen(filename,"rb",SH_DENYNO,pszSrcFile,__LINE__);
              if(!fp)
                break;
	      else {
                fseek(fp,0L,SEEK_END);
                icf.cbIconData = ftell(fp);
                fseek(fp,0L,SEEK_SET);
                buff = xmalloc(icf.cbIconData,pszSrcFile,__LINE__);
                if (!buff) {
                  fclose(fp);
                  break;
                }
                fread(buff,icf.cbIconData,1,fp);
                icf.pIconData = (PVOID)buff;
                fclose(fp);
              }
            }
            for(x = 1;x < 15;x++) {
              if(WinQueryButtonCheckstate(hwnd,SETICON_FRAME + x)) {
                icid = (ULONG)x;
                break;
              }
            }
            for(x = 18;x < 23;x++) {
              if(WinQueryButtonCheckstate(hwnd,SETICON_FRAME + x)) {
                icid = (ULONG)x;
                break;
              }
            }
            if(!WinSetSysPointerData(hwndDeskTop,icid,
                                     (PICONINFO)((filename && *filename) ?
                                      &icf : NULL)))
	    {
              Win_Error(hwnd,hwnd,pszSrcFile,__LINE__,"WinSetSysPointerData");
	    }
            if(buff)
              free(buff);
          }
          WinDismissDlg(hwnd,1);
          break;

        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,
                       HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_SETICON,0),
                       MPFROMSHORT(HM_RESOURCEID));
          break;

        case DID_CANCEL:
          WinDismissDlg(hwnd,0);
          break;
      }
      return 0;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}

