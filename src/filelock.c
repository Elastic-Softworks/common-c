/*
   ===================================
   C O M M O N - C
   FILE LOCKING PRIMITIVES IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- FILELOCK MODULE ---

    implementation of cross-platform file locking with
    Windows (LockFileEx/UnlockFileEx) and Unix
    (fcntl/flock) primitives. provides coordinated
    file access for process synchronization.

    see include/commc/filelock.h for function
    prototypes and documentation.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#include <stdlib.h>        /* for malloc, free */
#include <string.h>        /* for memset */

#ifdef _WIN32
    #include <windows.h>   /* for LockFileEx, UnlockFileEx */
    #include <io.h>        /* for _get_osfhandle */
#else
    #include <unistd.h>    /* for close */
    #include <fcntl.h>     /* for fcntl, F_SETLK, F_SETLKW */
    #include <errno.h>     /* for errno */
#endif

#include "commc/filelock.h"
#include "commc/error.h"

/*
	==================================
             --- HELPERS ---
	==================================
*/

/*

         validate_lock()
	       ---
	       validates that lock structure is properly
	       initialized and contains valid handle.

*/

static int validate_lock(const commc_filelock_t* lock) {

    if (!lock) {
        return 0;
    }

    if (!lock->handle) {
        return 0;
    }

    return 1;
}

/*

         platform_lock()
	       ---
	       performs platform-specific file locking.
	       handles differences between Windows and
	       Unix locking mechanisms.

*/

static commc_error_t platform_lock(commc_filelock_t* lock,
                                   commc_filelock_type_t type,
                                   long offset,
                                   long length,
                                   commc_filelock_mode_t mode) {

#ifdef _WIN32

    HANDLE        handle;
    OVERLAPPED*   overlapped;
    DWORD         flags;
    DWORD         offset_low;
    DWORD         offset_high;
    DWORD         length_low;
    DWORD         length_high;

    /* convert file descriptor to Windows handle if needed */
    if ((int)(intptr_t)lock->handle < 1024) {
        handle = (HANDLE)_get_osfhandle((int)(intptr_t)lock->handle);
        if (handle == INVALID_HANDLE_VALUE) {
            return COMMC_ERROR_INVALID_ARG;
        }
    } else {
        handle = (HANDLE)lock->handle;
    }

    /* allocate overlapped structure if not exists */
    if (!lock->overlapped) {
        overlapped = (OVERLAPPED*)malloc(sizeof(OVERLAPPED));
        if (!overlapped) {
            return COMMC_ERROR_OUT_OF_MEMORY;
        }
        memset(overlapped, 0, sizeof(OVERLAPPED));
        lock->overlapped = overlapped;
    } else {
        overlapped = (OVERLAPPED*)lock->overlapped;
    }

    /* set up flags */
    flags = 0;
    if (type == COMMC_FILELOCK_EXCLUSIVE) {
        flags |= LOCKFILE_EXCLUSIVE_LOCK;
    }
    if (mode == COMMC_FILELOCK_NOWAIT) {
        flags |= LOCKFILE_FAIL_IMMEDIATELY;
    }

    /* split offset and length into high/low parts */
    offset_low  = (DWORD)(offset & 0xFFFFFFFF);
    offset_high = (DWORD)((offset >> 32) & 0xFFFFFFFF);
    length_low  = (DWORD)(length & 0xFFFFFFFF);
    length_high = (DWORD)((length >> 32) & 0xFFFFFFFF);

    overlapped->Offset     = offset_low;
    overlapped->OffsetHigh = offset_high;

    /* attempt to acquire lock */
    if (!LockFileEx(handle, flags, 0, length_low, length_high, overlapped)) {
        DWORD error = GetLastError();
        if (error == ERROR_LOCK_VIOLATION || error == ERROR_IO_PENDING) {
            return COMMC_ERROR_WOULD_BLOCK;
        }
        return COMMC_ERROR_SYSTEM;
    }

    return COMMC_ERROR_NONE;

#else

    struct flock  fl;
    int           fd;
    int           cmd;

    fd = (int)(intptr_t)lock->handle;

    /* set up flock structure */
    memset(&fl, 0, sizeof(fl));
    fl.l_type   = (type == COMMC_FILELOCK_SHARED) ? F_RDLCK : F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start  = offset;
    fl.l_len    = length;

    /* choose command based on mode */
    cmd = (mode == COMMC_FILELOCK_NOWAIT) ? F_SETLK : F_SETLKW;

    /* attempt to acquire lock */
    if (fcntl(fd, cmd, &fl) == -1) {
        if (errno == EAGAIN || errno == EACCES) {
            return COMMC_ERROR_WOULD_BLOCK;
        }
        return COMMC_ERROR_SYSTEM;
    }

    return COMMC_ERROR_NONE;

#endif
}

/*

         platform_unlock()
	       ---
	       performs platform-specific file unlocking.
	       handles cleanup of platform resources.

*/

static commc_error_t platform_unlock(commc_filelock_t* lock) {

#ifdef _WIN32

    HANDLE      handle;
    OVERLAPPED* overlapped;
    DWORD       offset_low;
    DWORD       offset_high;
    DWORD       length_low;
    DWORD       length_high;

    if (!lock->overlapped) {
        return COMMC_ERROR_NONE;  /* nothing to unlock */
    }

    /* convert file descriptor to Windows handle if needed */
    if ((int)(intptr_t)lock->handle < 1024) {
        handle = (HANDLE)_get_osfhandle((int)(intptr_t)lock->handle);
        if (handle == INVALID_HANDLE_VALUE) {
            return COMMC_ERROR_INVALID_ARG;
        }
    } else {
        handle = (HANDLE)lock->handle;
    }

    overlapped = (OVERLAPPED*)lock->overlapped;

    /* split offset and length into high/low parts */
    offset_low  = overlapped->Offset;
    offset_high = overlapped->OffsetHigh;
    length_low  = (DWORD)(lock->length & 0xFFFFFFFF);
    length_high = (DWORD)((lock->length >> 32) & 0xFFFFFFFF);

    /* unlock the region */
    if (!UnlockFileEx(handle, 0, length_low, length_high, overlapped)) {
        return COMMC_ERROR_SYSTEM;
    }

    return COMMC_ERROR_NONE;

#else

    struct flock  fl;
    int           fd;

    fd = (int)(intptr_t)lock->handle;

    /* set up unlock structure */
    memset(&fl, 0, sizeof(fl));
    fl.l_type   = F_UNLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start  = lock->offset;
    fl.l_len    = lock->length;

    /* unlock the region */
    if (fcntl(fd, F_SETLK, &fl) == -1) {
        return COMMC_ERROR_SYSTEM;
    }

    return COMMC_ERROR_NONE;

#endif
}

/*
	==================================
             --- API ---
	==================================
*/

/*

         commc_filelock_create()
	       ---
	       creates new file lock structure for specified
	       file handle with proper initialization.

*/

commc_filelock_t* commc_filelock_create(void* file_handle) {

    commc_filelock_t* lock;

    if (!file_handle) {
        return NULL;
    }

    lock = (commc_filelock_t*)malloc(sizeof(commc_filelock_t));
    if (!lock) {
        return NULL;
    }

    /* initialize structure */
    memset(lock, 0, sizeof(commc_filelock_t));
    lock->handle    = file_handle;
    lock->type      = COMMC_FILELOCK_SHARED;
    lock->offset    = 0;
    lock->length    = 0;
    lock->is_locked = 0;

#ifdef _WIN32
    lock->overlapped = NULL;
#endif

    return lock;
}

/*

         commc_filelock_destroy()
	       ---
	       destroys file lock and releases any held locks.

*/

void commc_filelock_destroy(commc_filelock_t* lock) {

    if (!lock) {
        return;
    }

    /* release any held lock */
    if (lock->is_locked) {
        commc_filelock_release(lock);
    }

#ifdef _WIN32
    /* cleanup Windows-specific resources */
    if (lock->overlapped) {
        free(lock->overlapped);
        lock->overlapped = NULL;
    }
#endif

    /* free the structure */
    free(lock);
}

/*

         commc_filelock_acquire()
	       ---
	       acquires file lock on specified byte range
	       with given type and blocking behavior.

*/

commc_error_t commc_filelock_acquire(commc_filelock_t* lock,
                                     commc_filelock_type_t type,
                                     long offset,
                                     long length,
                                     commc_filelock_mode_t mode) {

    commc_error_t result;

    if (!validate_lock(lock)) {
        return COMMC_ERROR_INVALID_ARG;
    }

    if (length <= 0) {
        return COMMC_ERROR_INVALID_ARG;
    }

    /* release existing lock if held */
    if (lock->is_locked) {
        result = commc_filelock_release(lock);
        if (result != COMMC_ERROR_NONE) {
            return result;
        }
    }

    /* attempt to acquire new lock */
    result = platform_lock(lock, type, offset, length, mode);
    if (result != COMMC_ERROR_NONE) {
        return result;
    }

    /* update lock state */
    lock->type      = type;
    lock->offset    = offset;
    lock->length    = length;
    lock->is_locked = 1;

    return COMMC_ERROR_NONE;
}

/*

         commc_filelock_acquire_full()
	       ---
	       convenience function to lock entire file.

*/

commc_error_t commc_filelock_acquire_full(commc_filelock_t* lock,
                                          commc_filelock_type_t type,
                                          commc_filelock_mode_t mode) {

    /* lock from beginning to very end */
    return commc_filelock_acquire(lock, type, 0, 0x7FFFFFFF, mode);
}

/*

         commc_filelock_release()
	       ---
	       releases previously acquired file lock.

*/

commc_error_t commc_filelock_release(commc_filelock_t* lock) {

    commc_error_t result;

    if (!validate_lock(lock)) {
        return COMMC_ERROR_INVALID_ARG;
    }

    if (!lock->is_locked) {
        return COMMC_ERROR_NONE;  /* already unlocked */
    }

    /* perform platform-specific unlock */
    result = platform_unlock(lock);
    if (result != COMMC_ERROR_NONE) {
        return result;
    }

    /* clear lock state */
    lock->is_locked = 0;
    lock->offset    = 0;
    lock->length    = 0;

    return COMMC_ERROR_NONE;
}

/*

         commc_filelock_is_locked()
	       ---
	       returns lock status for query purposes.

*/

int commc_filelock_is_locked(const commc_filelock_t* lock) {

    if (!validate_lock(lock)) {
        return 0;
    }

    return lock->is_locked;
}

/*

         commc_filelock_get_type()
	       ---
	       returns current lock type or shared if unlocked.

*/

commc_filelock_type_t commc_filelock_get_type(const commc_filelock_t* lock) {

    if (!validate_lock(lock) || !lock->is_locked) {
        return COMMC_FILELOCK_SHARED;
    }

    return lock->type;
}

/*

         commc_filelock_try_upgrade()
	       ---
	       attempts to upgrade shared lock to exclusive.

*/

commc_error_t commc_filelock_try_upgrade(commc_filelock_t* lock,
                                         commc_filelock_mode_t mode) {

    long          offset;
    long          length;

    if (!validate_lock(lock)) {
        return COMMC_ERROR_INVALID_ARG;
    }

    if (!lock->is_locked) {
        return COMMC_ERROR_INVALID_STATE;
    }

    if (lock->type == COMMC_FILELOCK_EXCLUSIVE) {
        return COMMC_ERROR_NONE;  /* already exclusive */
    }

    /* save current lock parameters */
    offset = lock->offset;
    length = lock->length;

    /* acquire exclusive lock on same region */
    return commc_filelock_acquire(lock, COMMC_FILELOCK_EXCLUSIVE,
                                  offset, length, mode);
}

/*

         commc_filelock_downgrade()
	       ---
	       downgrades exclusive lock to shared lock.

*/

commc_error_t commc_filelock_downgrade(commc_filelock_t* lock) {

    long          offset;
    long          length;

    if (!validate_lock(lock)) {
        return COMMC_ERROR_INVALID_ARG;
    }

    if (!lock->is_locked) {
        return COMMC_ERROR_INVALID_STATE;
    }

    if (lock->type == COMMC_FILELOCK_SHARED) {
        return COMMC_ERROR_NONE;  /* already shared */
    }

    /* save current lock parameters */
    offset = lock->offset;
    length = lock->length;

    /* acquire shared lock on same region */
    return commc_filelock_acquire(lock, COMMC_FILELOCK_SHARED,
                                  offset, length, COMMC_FILELOCK_NOWAIT);
}

/*
	==================================
             --- EOF ---
	==================================
*/