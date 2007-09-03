
/***********************************************************************

  $Id$

  Define DosQProcStatus interface

  02 Sep 07 SHL Update for OpenWatcom

***********************************************************************/

/**********************************************************************
 * MODULE NAME :  procstat.h             AUTHOR:  Rick Fishman        *
 * DATE WRITTEN:   2-10-92                                            *
 *                                                                    *
 * DESCRIPTION:                                                       *
 *                                                                    *
 *  This header file contains the function prototype for the asofyet  *
 *  undocumented DosQProcStatus API.  It also contains the structure  *
 *  definitions that are used in the buffer that is returned by the   *
 *  API call.                                                         *
 *                                                                    *
 *  In order to invoke DosQProcStatus, your .DEF file must contain    *
 *  this entry:                                                       *
 *                                                                    *
 *           IMPORTS DOSQPROCSTATUS=DOSCALLS.154                      *
 *                                                                    *
 **********************************************************************/

#if defined(__WATCOMC__)
APIRET16 APIENTRY16 DosQProcStatus(ULONG *pBuf, USHORT cbBuf);
#endif

#if defined(__IBMC__)
#pragma linkage( DosQProcStatus, far16 pascal )
USHORT DosQProcStatus(PVOID pvBuf, USHORT cbBuf);
#endif

#define PROCESS_END_INDICATOR   3	// Indicates end of process structs

#pragma pack(1)

typedef struct _SUMMARY
{
  ULONG ulThreadCount;		// Number of threads in system
  ULONG ulProcessCount;		// Number of processes in system
  ULONG ulModuleCount;		// Number of modules in system

}
SUMMARY, *PSUMMARY;

typedef struct _THREADINFO
{
  ULONG ulRecType;		// Record type (thread = 100)
  USHORT tidWithinProcess;	// TID within process (TID is 4 bytes!!)
  USHORT usSlot;		// Unique thread slot number
  ULONG ulBlockId;		// Sleep id thread is sleeping on
  ULONG ulPriority;		// Priority
  ULONG ulSysTime;		// Thread System Time
  ULONG ulUserTime;		// Thread User Time
  UCHAR uchState;		// 1=ready,2=blocked,5=running
  UCHAR uchPad;			// Filler
  USHORT usPad;			// Filler

}
THREADINFO, *PTHREADINFO;

typedef struct _PROCESSINFO
{
  ULONG ulEndIndicator;		// 1 means not end, 3 means last entry
  PTHREADINFO ptiFirst;		// Address of the 1st Thread Control Blk
  USHORT pid;			// Process ID (2 bytes - PID is 4 bytes)
  USHORT pidParent;		// Parent's process ID
  ULONG ulType;			// Process Type
  ULONG ulStatus;		// Process Status
  ULONG idSession;		// Session ID
  USHORT hModRef;		// Module handle of EXE
  USHORT usThreadCount;		// Number of threads in this process
  ULONG ulReserved;		// Unknown
  PVOID pvReserved;		// Unknown
  USHORT usSem16Count;		// Number of 16-bit system semaphores
  USHORT usDllCount;		// Number of Dlls used by process
  USHORT usShrMemHandles;	// Number of shared memory handles
  USHORT usReserved;		// Unknown
  PUSHORT pusSem16TableAddr;	// Address of a 16-bit semaphore table
  PUSHORT pusDllTableAddr;	// Address of a Dll table
  PUSHORT pusShrMemTableAddr;	// Address of a shared memory table

}
PROCESSINFO, *PPROCESSINFO;

typedef struct _SEMINFO
{
  struct _SEMINFO *pNext;	// Ptr to next block (NULL on last one)
  UINT idOwningThread;		// ID of owning thread?
  UCHAR fbFlags;		// Semaphore flags
  UCHAR uchReferenceCount;	// Number of references
  UCHAR uchRequestCount;	// Number of requests
  UCHAR uchReserved;		// Unknown
  ULONG ulReserved;		// Unknown
  UINT uiReserved;		// Unknown
  CHAR szSemName[1];		// ASCIIZ semaphore name

}
SEMINFO, *PSEMINFO;

typedef struct _SHRMEMINFO
{
  struct _SHRMEMINFO *pNext;	// Ptr to next block (NULL on last one)
  USHORT usMemHandle;		// Shared memory handle (?)
  SEL selMem;			// Selector
  USHORT usReferenceCount;	// Number of references
  CHAR szMemName[1];		// ASCIIZ shared memory name

}
SHRMEMINFO, *PSHRMEMINFO;

typedef struct _MODINFO
{
  struct _MODINFO *pNext;	// Ptr to next block (NULL on last one)
  USHORT hMod;			// Module handle
  USHORT usModType;		// Module type (0=16bit,1=32bit)
  ULONG ulModRefCount;		// Count of module references
  ULONG ulSegmentCount;		// Number of segments in module
  ULONG ulDontKnow1;		//
  PSZ szModName;		// Addr of fully qualified module name
  USHORT usModRef[1];		// Handles of module references

}
MODINFO, *PMODINFO;

typedef struct _BUFFHEADER
{
  PSUMMARY psumm;		// SUMMARY section ptr
  PPROCESSINFO ppi;		// PROCESS section ptr
  PSEMINFO psi;			// SEM section ptr (add 16 to offset)
  PVOID pDontKnow1;		//
  PSHRMEMINFO psmi;		// SHARED MEMORY section ptr
  PMODINFO pmi;			// MODULE section ptr
  PVOID pDontKnow2;		//
  PVOID pDontKnow3;		//

}
BUFFHEADER, *PBUFFHEADER;

#pragma pack()
