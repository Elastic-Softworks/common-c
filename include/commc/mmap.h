/*
   ===================================
   C O M M O N - C
   MEMORY-MAPPED FILE I/O HEADER
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

         --- MEMORY-MAPPED FILE I/O ---

    cross-platform memory-mapped file interface providing
    uniform access to Windows CreateFileMapping/MapViewOfFile
    and Unix mmap() functionality. supports shared and
    private mappings with read/write access control.

    this module enables direct memory access to file contents,
    allowing for efficient file I/O operations, shared memory
    between processes, and memory-efficient file processing.

*/

#ifndef COMMC_MMAP_H
#define COMMC_MMAP_H

/*
	==================================
             --- INCLUDES ---
	==================================
*/

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>  /* for size_t */

#ifdef _WIN32
    #include <windows.h>
#endif

/*
	==================================
             --- TYPES ---
	==================================
*/

/*

         commc_mmap_access_t
	       ---
	       access mode enumeration for memory mapping.
	       defines how mapped memory can be accessed.

*/

typedef enum {
    COMMC_MMAP_READ_ONLY  = 0x01,  /* READ-ONLY ACCESS */
    COMMC_MMAP_READ_WRITE = 0x02,  /* READ-WRITE ACCESS */
    COMMC_MMAP_EXEC       = 0x04,  /* EXECUTABLE ACCESS */
    COMMC_MMAP_COPY       = 0x08   /* COPY-ON-WRITE ACCESS */
} commc_mmap_access_t;

/*

         commc_mmap_flags_t
	       ---
	       mapping behavior flags that control how
	       memory mapping is created and managed.

*/

typedef enum {
    COMMC_MMAP_SHARED     = 0x01,  /* SHARED BETWEEN PROCESSES */
    COMMC_MMAP_PRIVATE    = 0x02,  /* PRIVATE TO PROCESS */
    COMMC_MMAP_ANONYMOUS  = 0x04,  /* NO BACKING FILE */
    COMMC_MMAP_FIXED      = 0x08,  /* FIXED ADDRESS MAPPING */
    COMMC_MMAP_LOCKED     = 0x10,  /* LOCK IN MEMORY */
    COMMC_MMAP_POPULATE   = 0x20   /* POPULATE PAGES IMMEDIATELY */
} commc_mmap_flags_t;

/*

         commc_mmap_sync_t
	       ---
	       synchronization modes for flushing mapped
	       memory changes to backing file.

*/

typedef enum {
    COMMC_MMAP_SYNC_ASYNC = 0x01,  /* ASYNCHRONOUS SYNC */
    COMMC_MMAP_SYNC_SYNC  = 0x02,  /* SYNCHRONOUS SYNC */
    COMMC_MMAP_SYNC_INVALIDATE = 0x04  /* INVALIDATE OTHER MAPPINGS */
} commc_mmap_sync_t;

/*

         commc_mmap_t
	       ---
	       memory mapping structure containing all
	       information about a mapped memory region.

*/

typedef struct {
    void*             address;      /* MAPPED MEMORY ADDRESS */
    size_t            size;         /* SIZE OF MAPPED REGION */
    size_t            offset;       /* OFFSET INTO FILE */
    int               file_handle;  /* FILE DESCRIPTOR/HANDLE */
    commc_mmap_access_t access;     /* ACCESS PERMISSIONS */
    commc_mmap_flags_t  flags;      /* MAPPING FLAGS */
    
    int               is_valid;     /* MAPPING VALIDITY FLAG */
    int               is_locked;    /* MEMORY LOCK STATUS */
    
#ifdef _WIN32
    HANDLE            file_mapping; /* WINDOWS FILE MAPPING HANDLE */
    HANDLE            file_handle_win; /* WINDOWS FILE HANDLE */
#else
    int               fd;           /* UNIX FILE DESCRIPTOR */
#endif
} commc_mmap_t;

/*
	==================================
             --- PROTOTYPES ---
	==================================
*/

/*

         commc_mmap_create()
	       ---
	       creates a new memory mapping structure with
	       default initialization. returns NULL on failure.

	       this function only creates the structure; use
	       commc_mmap_map() to actually perform the mapping.

*/

commc_mmap_t* commc_mmap_create(void);

/*

         commc_mmap_destroy()
	       ---
	       destroys memory mapping structure and unmaps
	       any associated memory. automatically handles
	       synchronization and cleanup.

*/

void commc_mmap_destroy(commc_mmap_t* mmap);

/*

         commc_mmap_map_file()
	       ---
	       maps a file into memory with specified access
	       permissions and flags. creates mapping at
	       given offset with specified size.

	       if size is 0, maps entire file from offset.
	       if address is NULL, system chooses address.

*/

int commc_mmap_map_file(commc_mmap_t* mmap,
                       const char* filename,
                       commc_mmap_access_t access,
                       commc_mmap_flags_t flags,
                       size_t offset,
                       size_t size,
                       void* preferred_address);

/*

         commc_mmap_map_anonymous()
	       ---
	       creates anonymous memory mapping not backed
	       by a file. useful for allocating shared or
	       private memory regions.

*/

int commc_mmap_map_anonymous(commc_mmap_t* mmap,
                            size_t size,
                            commc_mmap_access_t access,
                            commc_mmap_flags_t flags,
                            void* preferred_address);

/*

         commc_mmap_unmap()
	       ---
	       unmaps memory region but preserves mapping
	       structure for potential reuse. synchronizes
	       changes before unmapping if needed.

*/

int commc_mmap_unmap(commc_mmap_t* mmap);

/*

         commc_mmap_remap()
	       ---
	       remaps memory region with new size or address.
	       platform support varies; may create new mapping
	       and copy data if resize is not supported.

*/

int commc_mmap_remap(commc_mmap_t* mmap,
                    size_t new_size,
                    void* new_address);

/*

         commc_mmap_sync()
	       ---
	       synchronizes mapped memory changes to backing
	       file using specified synchronization mode.

*/

int commc_mmap_sync(commc_mmap_t* mmap,
                   commc_mmap_sync_t sync_mode);

/*

         commc_mmap_sync_range()
	       ---
	       synchronizes specific range within mapped
	       memory to backing file. more efficient for
	       partial updates.

*/

int commc_mmap_sync_range(commc_mmap_t* mmap,
                         size_t offset,
                         size_t length,
                         commc_mmap_sync_t sync_mode);

/*

         commc_mmap_lock()
	       ---
	       locks mapped memory pages in physical RAM
	       to prevent swapping. requires appropriate
	       system privileges.

*/

int commc_mmap_lock(commc_mmap_t* mmap);

/*

         commc_mmap_unlock()
	       ---
	       unlocks previously locked memory pages,
	       allowing them to be swapped if needed.

*/

int commc_mmap_unlock(commc_mmap_t* mmap);

/*

         commc_mmap_lock_range()
	       ---
	       locks specific range within mapped memory.
	       more granular control than locking entire
	       mapping.

*/

int commc_mmap_lock_range(commc_mmap_t* mmap,
                         size_t offset,
                         size_t length);

/*

         commc_mmap_unlock_range()
	       ---
	       unlocks specific range within mapped memory
	       that was previously locked.

*/

int commc_mmap_unlock_range(commc_mmap_t* mmap,
                           size_t offset,
                           size_t length);

/*

         commc_mmap_protect()
	       ---
	       changes access protection for mapped memory
	       region. allows dynamic permission changes
	       after mapping is established.

*/

int commc_mmap_protect(commc_mmap_t* mmap,
                      commc_mmap_access_t new_access);

/*

         commc_mmap_protect_range()
	       ---
	       changes access protection for specific range
	       within mapped memory. enables fine-grained
	       permission control.

*/

int commc_mmap_protect_range(commc_mmap_t* mmap,
                            size_t offset,
                            size_t length,
                            commc_mmap_access_t new_access);

/*

         commc_mmap_advise()
	       ---
	       provides hints to system about memory usage
	       patterns for optimization. platform support
	       varies.

*/

int commc_mmap_advise(commc_mmap_t* mmap,
                     int advice);

/*

         commc_mmap_get_address()
	       ---
	       returns address of mapped memory region.
	       NULL if not currently mapped.

*/

void* commc_mmap_get_address(const commc_mmap_t* mmap);

/*

         commc_mmap_get_size()
	       ---
	       returns size of mapped memory region.
	       0 if not currently mapped.

*/

size_t commc_mmap_get_size(const commc_mmap_t* mmap);

/*

         commc_mmap_get_offset()
	       ---
	       returns file offset of mapping.

*/

size_t commc_mmap_get_offset(const commc_mmap_t* mmap);

/*

         commc_mmap_is_valid()
	       ---
	       checks if mapping is currently valid and
	       accessible.

*/

int commc_mmap_is_valid(const commc_mmap_t* mmap);

/*

         commc_mmap_is_locked()
	       ---
	       checks if mapping is locked in memory.

*/

int commc_mmap_is_locked(const commc_mmap_t* mmap);

/*

         commc_mmap_get_page_size()
	       ---
	       returns system memory page size for proper
	       alignment calculations.

*/

size_t commc_mmap_get_page_size(void);

/*

         commc_mmap_align_size()
	       ---
	       aligns size to page boundary for efficient
	       memory mapping operations.

*/

size_t commc_mmap_align_size(size_t size);

/*

         commc_mmap_align_address()
	       ---
	       aligns address to page boundary for proper
	       memory mapping alignment.

*/

void* commc_mmap_align_address(void* address);

#ifdef __cplusplus
}
#endif

#endif /* COMMC_MMAP_H */

/*
	==================================
             --- EOF ---
	==================================
*/