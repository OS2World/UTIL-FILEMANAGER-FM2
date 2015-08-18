
/***********************************************************************

  $Id: $

  Linked list utilities

  Copyright (c) 2015 Steven H. Levine

  07 Aug 15 SHL Baseline

***********************************************************************/

#define INCL_DOS

#include "fm3dll.h"
#include "errutil.h"			// Dos_Error...
#include "listutil.h"

// Data definitions
#pragma data_seg(DATA1)

static PSZ pszSrcFile = __FILE__;

/**
 * Append item to doubly linked list
 */

VOID List2Append(PLIST2HEADER header, PLIST2 item)
{
  item->next = NULL;
  item->prev = header->last;
  if (item->prev)
    item->prev->next = item;
  header->last = item;
  if (!header->first)
    header->first = item;		// List empty
}

/**
 * Get first item from doubly linked list
 * @return item or NULL
 */

PLIST2 List2GetFirst(PLIST2HEADER header)
{
  return header->first;
}

/**
 * Delete first item from doubly linked list
 * @return deleted item or NULL
 */

PLIST2 List2DeleteFirst(PLIST2HEADER header)
{
  PLIST2 item = header->first;

  // If list not empty
  if (item) {
    header->first = item->next;
    if (header->first)
      header->first->prev = NULL;
    if (header->last == item)
      header->last = NULL;
    item->next = NULL;			// Catch illegal accesses
    item->prev = NULL;			// Catch illegal accesses
  }
  return item;
}

/**
 * Delete specified item from doubly linked list
 * @param header is list header
 * @param item point to item to be deleted
 * @return deleted item or NULL
 * @note item must be a member of list
 */

VOID List2Delete(PLIST2HEADER header, PLIST2 item)
{
  if (item->next) {
    if (item->next->prev != item)
      Runtime_Error(pszSrcFile, __LINE__, "item %u item->next->prev %p", item, item->next->prev);
    item->next->prev = item->prev;
  }
  else
    header->last = item->prev;

  if (item->prev) {
    if (item->prev->next != item)
      Runtime_Error(pszSrcFile, __LINE__, "item %u item->prev->next%p", item, item->prev->next);
    item->prev->next = item->next;
  }
  else
    header->first = item->next;

  item->prev = NULL;			// Catch illegal accesses
  item->next = NULL;			// Catch illegal accesses
}

/**
 * Search list for matching item
 * @returns list pointer or NULL
 */

PLIST2 List2Search(PLIST2HEADER header, PLIST2MATCH matchFunc, PVOID data) {
  PLIST2 item;
  for (item = header->first; item; item = item->next) {
    if (matchFunc(item, data))
      break;				// Matched
  } // for
  return item;
}

// #pragma alloc_text(LISTUTIL, FIXME)
