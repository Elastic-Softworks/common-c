/*
   ===================================
   C O M M O N - C
   FILE LOCKING PRIMITIVES API
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- FILELOCK MODULE ---

    cross-platform file locking primitives for coordinating
    access to shared files between processes and threads.
    provides both advisory and mandatory locking with
    timeout support and proper error handling.

    supports both shared (read) and exclusive (write) locks
    with byte-range locking capabilities for fine-grained
    file access control.

*/

#ifndef COMMC_FILELOCK_H
#define COMMC_FILELOCK_H

/* 
	==================================
             --- SETUP ---
	==================================
*/

#include "commc/error.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
	==================================
             --- TYPES ---
	==================================
*/

/*

         commc_filelock_type_t
	       ---
	       specifies the type of file lock to acquire.
	       shared locks allow multiple readers, exclusive
	       locks allow only one writer.

*/

typedef enum {

    COMMC_FILELOCK_SHARED    = 0,    /* READ LOCK - MULTIPLE ALLOWED */
    COMMC_FILELOCK_EXCLUSIVE = 1     /* WRITE LOCK - EXCLUSIVE ACCESS */

} commc_filelock_type_t;

/*

         commc_filelock_mode_t
	       ---
	       specifies the locking behavior when lock
	       cannot be immediately acquired.

*/

typedef enum {

    COMMC_FILELOCK_NOWAIT    = 0,    /* RETURN IMMEDIATELY IF LOCKED */
    COMMC_FILELOCK_WAIT      = 1     /* BLOCK UNTIL LOCK ACQUIRED */

} commc_filelock_mode_t;

/*

         commc_filelock_t
	       ---
	       represents an active file lock with all
	       necessary platform-specific information
	       for proper cleanup and management.

*/

typedef struct {

    void*                    handle;      /* PLATFORM FILE HANDLE */
    commc_filelock_type_t    type;        /* LOCK TYPE (SHARED/EXCLUSIVE) */
    long                     offset;      /* BYTE OFFSET OF LOCKED REGION */
    long                     length;      /* LENGTH OF LOCKED REGION */
    int                      is_locked;   /* LOCK STATUS FLAG */

#ifdef _WIN32
    void*                    overlapped;  /* WINDOWS OVERLAPPED STRUCTURE */
#endif

} commc_filelock_t;

/*
	==================================
             --- API ---
	==================================
*/

/*

         commc_filelock_create()
	       ---
	       creates a new file lock structure for the
	       specified file handle. does not acquire
	       the lock - use commc_filelock_acquire().

*/

commc_filelock_t* commc_filelock_create(void* file_handle);

/*

         commc_filelock_destroy()
	       ---
	       destroys file lock structure and releases
	       any acquired locks. always call this to
	       prevent resource leaks.

*/

void commc_filelock_destroy(commc_filelock_t* lock);

/*

         commc_filelock_acquire()
	       ---
	       acquires file lock of specified type on
	       byte range [offset, offset + length).
	       returns COMMC_ERROR_NONE on success.

*/

commc_error_t commc_filelock_acquire(commc_filelock_t* lock,
                                     commc_filelock_type_t type,
                                     long offset,
                                     long length,
                                     commc_filelock_mode_t mode);

/*

         commc_filelock_acquire_full()
	       ---
	       acquires file lock on entire file.
	       convenience function for common use case.

*/

commc_error_t commc_filelock_acquire_full(commc_filelock_t* lock,
                                          commc_filelock_type_t type,
                                          commc_filelock_mode_t mode);

/*

         commc_filelock_release()
	       ---
	       releases previously acquired file lock.
	       safe to call multiple times or on
	       non-locked structures.

*/

commc_error_t commc_filelock_release(commc_filelock_t* lock);

/*

         commc_filelock_is_locked()
	       ---
	       returns 1 if lock is currently held,
	       0 otherwise. useful for status checking.

*/

int commc_filelock_is_locked(const commc_filelock_t* lock);

/*

         commc_filelock_get_type()
	       ---
	       returns the type of currently held lock
	       or COMMC_FILELOCK_SHARED if not locked.

*/

commc_filelock_type_t commc_filelock_get_type(const commc_filelock_t* lock);

/*

         commc_filelock_try_upgrade()
	       ---
	       attempts to upgrade shared lock to exclusive.
	       returns COMMC_ERROR_NONE on success,
	       COMMC_ERROR_WOULD_BLOCK if cannot upgrade.

*/

commc_error_t commc_filelock_try_upgrade(commc_filelock_t* lock,
                                         commc_filelock_mode_t mode);

/*

         commc_filelock_downgrade()
	       ---
	       downgrades exclusive lock to shared lock.
	       allows other readers while maintaining
	       some level of coordination.

*/

commc_error_t commc_filelock_downgrade(commc_filelock_t* lock);

#ifdef __cplusplus
}
#endif

#endif /* COMMC_FILELOCK_H */

/*
	==================================
             --- EOF ---
	==================================
*/