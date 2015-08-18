
/***********************************************************************

  $Id: $

  Linked list utilities

  Copyright (c) 2015 Steven H. Levine

  07 Aug 15 SHL Baseline

***********************************************************************/

#if !defined(LISTUTIL_H)
#define LISTUTIL_H

// Singly linked list
typedef struct LISTHEADER {
  struct LIST *first;			// First item in list
} LISTHEADER;
typedef LISTHEADER *PLISTHEADER;

typedef struct LIST {
  struct LIST *next;			// Next item in list
} LIST;
typedef LIST *PLIST;

// Doubly linked list
typedef struct LIST2HEADER {
  struct LIST2 *first;			// First item in list
  struct LIST2 *last;			// Last item in list
} LIST2HEADER;
typedef LIST2HEADER *PLIST2HEADER;

typedef struct LIST2 {
  struct LIST2 *next;			// Next item in list
  struct LIST2 *prev;			// Previoius item in list
} LIST2;
typedef LIST2 *PLIST2;

VOID ListAppend(PLISTHEADER header, PLIST item);
VOID ListDelete(PLIST header, PLIST item);
PLIST ListDeleteFirst(PLISTHEADER *header);
PLIST ListGetFirst(PLISTHEADER header);
typedef BOOL LISTMATCH(PLIST item, PVOID pdata);		// For ListSearch
typedef LISTMATCH *PLISTMATCH;
PLIST ListSearch(PLISTHEADER header, PLISTMATCH matchFunc);

VOID List2Append(PLIST2HEADER header, PLIST2 item);
VOID List2Delete(PLIST2HEADER header, PLIST2 item);
PLIST2 List2DeleteFirst(PLIST2HEADER header);
PLIST2 List2GetFirst(PLIST2HEADER header);
typedef BOOL LIST2MATCH(PLIST2 item, PVOID data);	// For List2Search
typedef LIST2MATCH *PLIST2MATCH;
PLIST2 List2Search(PLIST2HEADER header, PLIST2MATCH matchFunc, PVOID data);

#endif // LISTUTIL_H
