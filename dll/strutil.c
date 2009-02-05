
/***********************************************************************

  $Id$

  External strings support - stored in STRINGTABLE

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2006, 2009 Steven H. Levine

  22 Jul 06 SHL Comments
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  05 Jan 08 SHL Rename from string.c to avoid string.h conflict
  03 Feb 09 SHL Switch to STRINGTABLE and const return

***********************************************************************/

#include <stdio.h>
#include <share.h>
#include <string.h>

#define INCL_DOSPROCESS			// DosSleep

#include "fm3dll.h"
#include "fm3str.h"
#include "init.h"			// Data declaration(s)
#include "mainwnd.h"			// FM3ModHandle
#include "wrappers.h"
#include "errutil.h"
#include "strutil.h"
#include "version.h"

static PSZ pszSrcFile = __FILE__;

//== GetPString() return a readonly pointer to the requested string in memory ==

PCSZ GetPString(ULONG id)
{
  PSZ psz;
  LONG l;
  CHAR sz[257];
  ULONG ulNewFirstId;
  ULONG ulNewLastId;

  static PSZ *pLoadedStrings;
  static ULONG ulFirstId;
  static ULONG ulLastId;

  // Strings that must be combined because stringtable items limited to 256
  static struct LongString {
    ULONG id;
    ULONG sub_id;
  } LongStrings[] = {
    {IDS_SUGGEST1TEXT, IDS_SUGGEST1TEXT1},
    {IDS_SUGGEST1TEXT, IDS_SUGGEST1TEXT2},
    {IDS_ARCHIVERBB2TEXT, IDS_ARCHIVERBB2TEXT1},
    {IDS_ARCHIVERBB2TEXT, IDS_ARCHIVERBB2TEXT2},
    {IDS_ARCHIVERBB2TEXT, IDS_ARCHIVERBB2TEXT3},
    {IDS_ARCHIVERBB2TEXT, IDS_ARCHIVERBB2TEXT4},
    {IDS_ARCHIVERBB2TEXT, IDS_ARCHIVERBB2TEXT5},
    {IDS_ARCHIVERBB2TEXT, IDS_ARCHIVERBB2TEXT6},
    {IDS_ARCHIVERBB2TEXT, IDS_ARCHIVERBB2TEXT7},
    {IDS_INIBINARYDATASKIPTEXT, IDS_INIBINARYDATASKIPTEXT1},
    {IDS_INIBINARYDATASKIPTEXT, IDS_INIBINARYDATASKIPTEXT2},
    {IDS_INSTANTHELPTEXT, IDS_INSTANTHELPTEXT1},
    {IDS_INSTANTHELPTEXT, IDS_INSTANTHELPTEXT2},
    {IDS_FSDERRORTEXT, IDS_FSDERRORTEXT1},
    {IDS_FSDERRORTEXT, IDS_FSDERRORTEXT2},
    {IDS_LANERRORTEXT, IDS_LANERRORTEXT1},
    {IDS_LANERRORTEXT, IDS_LANERRORTEXT2},
    {IDS_MAKESHADOWHELPTEXT, IDS_MAKESHADOWHELPTEXT1},
    {IDS_MAKESHADOWHELPTEXT, IDS_MAKESHADOWHELPTEXT2},
    {IDS_UNDELETEHELPTEXT, IDS_UNDELETEHELPTEXT1},
    {IDS_UNDELETEHELPTEXT, IDS_UNDELETEHELPTEXT2},
    {IDS_KILLPROCHELPTEXT, IDS_KILLPROCHELPTEXT1},
    {IDS_KILLPROCHELPTEXT, IDS_KILLPROCHELPTEXT2},
    {IDS_ARCNOTTHERETEXT, IDS_ARCNOTTHERETEXT1},
    {IDS_ARCNOTTHERETEXT, IDS_ARCNOTTHERETEXT2},
    {IDS_FM2CMDHELPTEXT, IDS_FM2CMDHELPTEXT1},
    {IDS_FM2CMDHELPTEXT, IDS_FM2CMDHELPTEXT2},
    {IDS_FM2CMDHELPTEXT, IDS_FM2CMDHELPTEXT3}
  };

  static UINT cLongStrings = sizeof(LongStrings) / sizeof(struct LongString);

  static volatile INT cBusy;		// Need to be MT-safe
  static ULONG ulDbgId;			// 13 Jan 09 SHL fixme to be gone?
  static UINT uDbgState;		// 03 Feb 09 SHL fixme to be gone?
  static ULONG ulDbgTid;		// 13 Jan 09 SHL fixme to be gone?

  UINT c;
  // 23 Jan 09 SHL fixme to use SMP safe inc/dec?
  extern void SMPSafeInc(void);
  extern void SMPSafeDec(void);
  #pragma aux SMPSafeInc = "lock inc cBusy" modify exact [];
  #pragma aux SMPSafeDec = "lock dec cBusy" modify exact [];
  // SMPSafeInc();
  for (c = 0; ; c++) {
    if (++cBusy == 1)
      break;
    cBusy--;
    // Hold off 1 cycle before reporting since some contention expected
    if (c == 1)
      DbgMsg(pszSrcFile, __LINE__, "GetPString(%lu) waiting for tid %lu GetPString(%lu), state=%u", id, ulDbgTid, ulDbgId, uDbgState);
    DosSleep(1);			// Let current owner finish
  }
  if (c > 1)
    DbgMsg(pszSrcFile, __LINE__, "continuing with GetPString(%lu) after tid %lu GetPString(%lu), state=%u", id, ulDbgTid, ulDbgId, uDbgState);

  // Remember id and thread ordinal for diagnosing MT hangs
  // Use fast DosGetInfoBlocks to ensure debug logic does not change timing
  {
    extern PTIB2 GetPTIB2(void);
    #pragma aux GetPTIB2 = "mov eax,fs:[12]" value [eax];
    // PIB *ppib;
    // TIB *ptib;
    TIB2 *ptib2 = GetPTIB2();
    // APIRET apiret = DosGetInfoBlocks(&ptib, &ppib);
    ulDbgId = id;
    // ulDbgTid = apiret == 0 ? ptib->tib_ptib2->tib2_ultid : 0;
    ulDbgTid = ptib2->tib2_ultid;
  }

  // DbgMsg(pszSrcFile, __LINE__, "Fetching %lu", id);

  // If string already loaded, return it now
  if (id >= ulFirstId &&
      id <= ulLastId &&
      pLoadedStrings &&
      (psz = pLoadedStrings[id - ulFirstId]) != NULL) {
    cBusy--;
    if (((ULONG)psz & 0xffff0000) == 0)
      DbgMsg(pszSrcFile, __LINE__, "id %lu corrupted %p", id, psz);
    // DbgMsg(pszSrcFile, __LINE__, "id %lu \"%s\"", id, psz ? psz : "(null)");
    return psz;
  }

  // Try to load
  // 11 Jan 09 SHL fixme to use global HAB?
  uDbgState = 1;
  l = WinLoadString((HAB)NULL, FM3ModHandle, id, sizeof(sz), sz);
  uDbgState = 2;

  if (l != 0) {
    psz = xstrdup(sz, pszSrcFile, __LINE__);
    if (!psz) {
      cBusy--;
      return NullStr;
    }
  }
  else {
    // Assume string must be built from multiple strings - find first
    UINT i;
    psz = NULL;
    for (i = 0; i < cLongStrings && LongStrings[i].id != id; i++);	// Scan

    if (i < cLongStrings) {
      // Combine stringtable items to build long string
      // DbgMsg(pszSrcFile, __LINE__, "Building long string %lu", id);
      for (; LongStrings[i].id == id; i++) {
	uDbgState = 3;
	l = WinLoadString((HAB)NULL, FM3ModHandle, LongStrings[i].sub_id, sizeof(sz), sz);
	uDbgState = 4;
	if (l == 0) {
	  cBusy--;
	  Runtime_Error(pszSrcFile, __LINE__, "string %lu missing", LongStrings[i].sub_id);
	  xfree(psz, pszSrcFile, __LINE__);
	  return NullStr;
	}
	if (!psz) {
	  // Remember 1st string
	  psz = strdup(sz);
	  if (!psz) {
	    cBusy--;
	    return NullStr;
	  }
	}
	else {
	  // Append string
	  UINT curLen = strlen(psz);
	  PSZ psz2 = xrealloc(psz, curLen + l + 1, pszSrcFile, __LINE__);
	  if (!psz2) {
	    xfree(psz, pszSrcFile, __LINE__);
	    cBusy--;
	    return NullStr;
	  }
	  memcpy(psz2 + curLen, sz, l);	// Append
	  *(psz2 + curLen + l) = 0;	// Terminate
	  psz = psz2;			// Remember
	  l += curLen;
	}
      } // while
    } // if long
  } // if loaded

  if (l == 0) {
    DbgMsg(pszSrcFile, __LINE__, "Error loading %lu", id);
    sprintf(sz, "** Error loading id %lu **", id);
    psz = xstrdup(sz, pszSrcFile, __LINE__);
    if (psz)
      l = strlen(sz);
    else
      psz = NullStr;		// Oh heck
  }

  uDbgState = 5;
  // Add to cache
  // DbgMsg(pszSrcFile, __LINE__, "Caching %lu", id);

  // Calculate new array limits
  if (!pLoadedStrings) {
    ulNewFirstId = id;
    ulNewLastId = id;
    ulFirstId = id;
    ulLastId = id;
  }
  else {
    ulNewFirstId = id < ulFirstId ? id : ulFirstId;
    ulNewLastId = id > ulLastId ? id : ulLastId;
  }

  if (ulNewFirstId != ulFirstId ||
      ulNewLastId != ulLastId ||
      !pLoadedStrings) {
    PSZ *pNewLoadedStrings;
    // DbgMsg(pszSrcFile, __LINE__, "Reallocating for %lu", id);
    pNewLoadedStrings = xrealloc(pLoadedStrings,
				 (ulNewLastId - ulNewFirstId + 1) * sizeof(PSZ),
				 pszSrcFile, __LINE__);
    if (!pNewLoadedStrings) {
      cBusy--;
      Runtime_Error(pszSrcFile, __LINE__, "realloc failed");
      xfree(psz, pszSrcFile, __LINE__);
      return NullStr;
    }
    // Align existing entries and zero fill unused entries as needed
    if (ulNewFirstId < ulFirstId) {
      // Move room for new entries at head of array
      memmove(pNewLoadedStrings + (ulFirstId - ulNewFirstId),
	     pNewLoadedStrings,
	     (ulLastId - ulFirstId + 1) * sizeof(PSZ));
      // Null unused placeholder entries
      if (ulFirstId - ulNewFirstId > 1)
	memset(pNewLoadedStrings + 1, 0, (ulFirstId - ulNewFirstId - 1) * sizeof(PSZ));
    }
    if (ulNewLastId - ulLastId > 1) {
      // Null unused placeholder entries
      memset(pNewLoadedStrings + (ulLastId - ulNewFirstId + 1),
	     0,
	     (ulNewLastId - ulLastId - 1) * sizeof(PSZ));
    }
    pLoadedStrings = pNewLoadedStrings;
    ulFirstId = ulNewFirstId;
    ulLastId = ulNewLastId;
  }

  uDbgState = 6;
  pLoadedStrings[id - ulFirstId] = psz;
  cBusy--;
  // DbgMsg(pszSrcFile, __LINE__, "id %lu \"%s\"", id, psz ? psz : "(null)");
  return psz;
}

#pragma alloc_text(STRINGS,LoadStrings,GetPString)
