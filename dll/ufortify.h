/*
 * $Id$
 *
 * FILE:
 *   ufortify.h
 *
 * DESCRIPTION:
 *   User options for fortify. Changes to this file require fortify.c to be
 * recompiled, but nothing else.
 */

 /* 18 Jul 08 SHL Add FORTIFY_VERBOSE_SCOPE_ENTER_EXIT
 */

// 28 Jan 08 SHL
#define FORTIFY_STORAGE _export		/* storage for public functions   */

#define FORTIFY_ALIGNMENT        sizeof(double) /* Byte alignment of all memory blocks */

#define FORTIFY_BEFORE_SIZE      32  /* Bytes to allocate before block */
#define FORTIFY_BEFORE_VALUE   0xA3  /* Fill value before block        */

#define FORTIFY_AFTER_SIZE       32  /* Bytes to allocate after block  */
#define FORTIFY_AFTER_VALUE    0xA5  /* Fill value after block         */

#define FORTIFY_FILL_ON_ALLOCATE               /* Nuke out malloc'd memory       */
#define FORTIFY_FILL_ON_ALLOCATE_VALUE   0xA7  /* Value to initialize with       */

#define FORTIFY_FILL_ON_DEALLOCATE             /* free'd memory is cleared       */
#define FORTIFY_FILL_ON_DEALLOCATE_VALUE 0xA9  /* Value to de-initialize with    */

#define FORTIFY_FILL_ON_CORRUPTION             /* Nuke out corrupted memory    */

/* #define FORTIFY_CHECK_ALL_MEMORY_ON_ALLOCATE */
/* #define FORTIFY_CHECK_ALL_MEMORY_ON_DEALLOCATE */
#define FORTIFY_PARANOID_DEALLOCATE

/* #define FORTIFY_WARN_ON_ZERO_MALLOC */ /* A debug is issued on a malloc(0) */
/* #define FORTIFY_FAIL_ON_ZERO_MALLOC */ /* A malloc(0) will fail            */

#define FORTIFY_WARN_ON_ALLOCATE_FAIL    /* A debug is issued on a failed alloc  */
#define FORTIFY_WARN_ON_FALSE_FAIL       /* See Fortify_SetAllocateFailRate      */
#define FORTIFY_WARN_ON_SIZE_T_OVERFLOW  /* Watch for breaking the 64K limit in  */
                                         /* some braindead architectures...      */

#define FORTIFY_TRACK_DEALLOCATED_MEMORY
#define FORTIFY_DEALLOCATED_MEMORY_LIMIT 1048576 /* Maximum amount of deallocated bytes to keep */
/* #define FORTIFY_WARN_WHEN_DISCARDING_DEALLOCATED_MEMORY */
/* #define FORTIFY_VERBOSE_WARN_WHEN_DISCARDING_DEALLOCATED_MEMORY */
#define FORTIFY_VERBOSE_SCOPE_ENTER_EXIT	/* Log all scope entry and exit */

/* #define FORTIFY_NO_PERCENT_P */       /* sprintf() doesn't support %p */
#define FORTIFY_STRDUP              /* if you use non-ANSI strdup() */

// #include "_malloc.h"
#ifdef __cplusplus
extern "C" {
#endif
#ifdef __BORLANDC__
// See bc20\source\rtl\rtlinc\_malloc.h
void _RTLENTRY _EXPFUNC _lock_heap  (void);
void _RTLENTRY _EXPFUNC _unlock_heap(void);
#endif

#ifdef __WATCOMC__
// 05 May 08 SHL
// J:\sla_dev2\ow.dev\bld\clib\h\heapacc.h
extern void         (*_AccessNHeap)( void );
extern void         (*_ReleaseNHeap)( void );
#endif
#ifdef __cplusplus
}
#endif

#ifdef __BORLANDC__
#define FORTIFY_LOCK()	_lock_heap()	// 28 Jan 08 SHL
#define FORTIFY_UNLOCK() _unlock_heap()	// 28 Jan 08 SHL
#endif
#ifdef __WATCOMC__
#define FORTIFY_LOCK()	_AccessNHeap()	// 05 May 08 SHL
#define FORTIFY_UNLOCK() _ReleaseNHeap()	// 05 May 08 SHL
#endif

#define FORTIFY_DELETE_STACK_SIZE    256

#ifdef __cplusplus                /* C++ only options go here */

/*    #define FORTIFY_PROVIDE_ARRAY_NEW     */
/*    #define FORTIFY_PROVIDE_ARRAY_DELETE  */

/*    #define FORTIFY_AUTOMATIC_LOG_FILE */
    #define FORTIFY_LOG_FILENAME            "fortify.log"
    #include <iostream.h>
    #define FORTIFY_FIRST_ERROR_FUNCTION    cout << "\a\a\aFortify Hit Generated!\n"

#endif /* __cplusplus */
