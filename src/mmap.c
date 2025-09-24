/*
   ===================================
   C O M M O N - C
   MEMORY-MAPPED FILE I/O IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

         --- MEMORY-MAPPED FILE I/O ---

    implementation of cross-platform memory-mapped file
    interface using Windows CreateFileMapping/MapViewOfFile
    and Unix mmap() functionality. provides unified access
    to memory-mapped files with proper resource management.

    see include/commc/mmap.h for function prototypes
    and comprehensive documentation.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#include <stdlib.h>        /* for malloc, free */
#include <string.h>        /* for memset */
#include <stdio.h>         /* for sprintf */

#ifdef _WIN32
    #include <windows.h>   /* for CreateFileMapping, MapViewOfFile */
    #include <io.h>        /* for _open_osfhandle */
#else
    #include <sys/mman.h>  /* for mmap, munmap */
    #include <sys/stat.h>  /* for fstat */
    #include <fcntl.h>     /* for open */
    #include <unistd.h>    /* for close, getpagesize */
    #include <errno.h>     /* for errno */
#endif

#include "commc/mmap.h"
#include "commc/error.h"

/*
	==================================
             --- GLOBALS ---
	==================================
*/

static size_t page_size = 0;  /* CACHED PAGE SIZE */

/*
	==================================
             --- HELPERS ---
	==================================
*/

/*

         validate_mmap()
	       ---
	       validates that mmap structure is properly
	       initialized and not NULL.

*/

static int validate_mmap(const commc_mmap_t* mmap) {

    return (mmap != NULL);
}

/*

         init_page_size()
	       ---
	       initializes cached page size value using
	       platform-specific system calls.

*/

static void init_page_size(void) {

    if (page_size == 0) {
#ifdef _WIN32
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        page_size = si.dwPageSize;
#else
        page_size = getpagesize();
#endif
    }
}

/*

         platform_open_file()
	       ---
	       opens file with appropriate flags for
	       memory mapping on current platform.

*/

static int platform_open_file(const char* filename,
                              commc_mmap_access_t access,
                              commc_mmap_t* mmap) {

#ifdef _WIN32
    DWORD   desired_access = 0;
    DWORD   share_mode = FILE_SHARE_READ;
    DWORD   creation_disposition = OPEN_EXISTING;
    HANDLE  file_handle;

    /* determine access flags */
    if (access & COMMC_MMAP_READ_ONLY) {
        desired_access |= GENERIC_READ;
    }
    if (access & COMMC_MMAP_READ_WRITE) {
        desired_access |= GENERIC_READ | GENERIC_WRITE;
        share_mode |= FILE_SHARE_WRITE;
    }

    /* open file */
    file_handle = CreateFileA(filename, desired_access, share_mode,
                             NULL, creation_disposition,
                             FILE_ATTRIBUTE_NORMAL, NULL);

    if (file_handle == INVALID_HANDLE_VALUE) {
        return -1;
    }

    mmap->file_handle_win = file_handle;
    mmap->file_handle = _open_osfhandle((intptr_t)file_handle, 0);

    return 0;

#else
    int flags = 0;
    int fd;

    /* determine access flags */
    if (access & COMMC_MMAP_READ_WRITE) {
        flags = O_RDWR;
    } else {
        flags = O_RDONLY;
    }

    /* open file */
    fd = open(filename, flags);
    if (fd == -1) {
        return -1;
    }

    mmap->fd = fd;
    mmap->file_handle = fd;

    return 0;

#endif
}

/*

         platform_close_file()
	       ---
	       closes file handle using appropriate
	       platform-specific functions.

*/

static void platform_close_file(commc_mmap_t* mmap) {

#ifdef _WIN32
    if (mmap->file_handle_win != INVALID_HANDLE_VALUE) {
        CloseHandle(mmap->file_handle_win);
        mmap->file_handle_win = INVALID_HANDLE_VALUE;
    }
#else
    if (mmap->fd != -1) {
        close(mmap->fd);
        mmap->fd = -1;
    }
#endif

    mmap->file_handle = -1;
}

/*

         platform_get_file_size()
	       ---
	       retrieves file size using platform-specific
	       system calls.

*/

static size_t platform_get_file_size(commc_mmap_t* mmap) {

#ifdef _WIN32
    LARGE_INTEGER file_size;
    
    if (!GetFileSizeEx(mmap->file_handle_win, &file_size)) {
        return 0;
    }
    
    return (size_t)file_size.QuadPart;
#else
    struct stat st;
    
    if (fstat(mmap->fd, &st) != 0) {
        return 0;
    }
    
    return st.st_size;
#endif
}

/*

         platform_create_mapping()
	       ---
	       creates memory mapping using platform-specific
	       functions and settings.

*/

static int platform_create_mapping(commc_mmap_t* mmap) {

#ifdef _WIN32
    DWORD  protect = 0;
    DWORD  access_flags = 0;
    HANDLE mapping_handle;

    /* determine protection flags */
    if (mmap->access & COMMC_MMAP_READ_ONLY) {
        protect = PAGE_READONLY;
        access_flags = FILE_MAP_READ;
    }
    if (mmap->access & COMMC_MMAP_READ_WRITE) {
        protect = PAGE_READWRITE;
        access_flags = FILE_MAP_ALL_ACCESS;
    }
    if (mmap->access & COMMC_MMAP_COPY) {
        protect = PAGE_WRITECOPY;
        access_flags = FILE_MAP_COPY;
    }
    if (mmap->access & COMMC_MMAP_EXEC) {
        if (mmap->access & COMMC_MMAP_READ_WRITE) {
            protect = PAGE_EXECUTE_READWRITE;
        } else {
            protect = PAGE_EXECUTE_READ;
        }
        access_flags |= FILE_MAP_EXECUTE;
    }

    /* create file mapping */
    mapping_handle = CreateFileMappingA(mmap->file_handle_win, NULL,
                                       protect, 0, 0, NULL);

    if (mapping_handle == NULL) {
        return -1;
    }

    mmap->file_mapping = mapping_handle;

    /* map view of file */
    mmap->address = MapViewOfFile(mapping_handle, access_flags,
                                 (DWORD)(mmap->offset >> 32),
                                 (DWORD)(mmap->offset & 0xFFFFFFFF),
                                 mmap->size);

    if (mmap->address == NULL) {
        CloseHandle(mapping_handle);
        mmap->file_mapping = NULL;
        return -1;
    }

    return 0;

#else
    int prot = 0;
    int flags = 0;

    /* determine protection flags */
    if (mmap->access & COMMC_MMAP_READ_ONLY) {
        prot |= PROT_READ;
    }
    if (mmap->access & COMMC_MMAP_READ_WRITE) {
        prot |= PROT_READ | PROT_WRITE;
    }
    if (mmap->access & COMMC_MMAP_EXEC) {
        prot |= PROT_EXEC;
    }

    /* determine mapping flags */
    if (mmap->flags & COMMC_MMAP_SHARED) {
        flags |= MAP_SHARED;
    }
    if (mmap->flags & COMMC_MMAP_PRIVATE) {
        flags |= MAP_PRIVATE;
    }
    if (mmap->flags & COMMC_MMAP_ANONYMOUS) {
        flags |= MAP_ANONYMOUS;
    }
    if (mmap->flags & COMMC_MMAP_FIXED) {
        flags |= MAP_FIXED;
    }

#ifdef MAP_LOCKED
    if (mmap->flags & COMMC_MMAP_LOCKED) {
        flags |= MAP_LOCKED;
    }
#endif

#ifdef MAP_POPULATE
    if (mmap->flags & COMMC_MMAP_POPULATE) {
        flags |= MAP_POPULATE;
    }
#endif

    /* create mapping */
    mmap->address = mmap(NULL, mmap->size, prot, flags,
                        mmap->fd, mmap->offset);

    if (mmap->address == MAP_FAILED) {
        mmap->address = NULL;
        return -1;
    }

    return 0;

#endif
}

/*

         platform_destroy_mapping()
	       ---
	       destroys memory mapping using platform-specific
	       cleanup functions.

*/

static void platform_destroy_mapping(commc_mmap_t* mmap) {

#ifdef _WIN32
    if (mmap->address) {
        UnmapViewOfFile(mmap->address);
        mmap->address = NULL;
    }
    
    if (mmap->file_mapping != NULL) {
        CloseHandle(mmap->file_mapping);
        mmap->file_mapping = NULL;
    }
#else
    if (mmap->address) {
        munmap(mmap->address, mmap->size);
        mmap->address = NULL;
    }
#endif

    mmap->is_valid = 0;
}

/*
	==================================
             --- API ---
	==================================
*/

/*

         commc_mmap_create()
	       ---
	       creates new memory mapping structure with
	       default initialization.

*/

commc_mmap_t* commc_mmap_create(void) {

    commc_mmap_t* mmap;

    /* initialize page size if needed */
    init_page_size();

    mmap = (commc_mmap_t*)malloc(sizeof(commc_mmap_t));
    if (!mmap) {
        return NULL;
    }

    /* initialize with default values */
    memset(mmap, 0, sizeof(commc_mmap_t));
    mmap->address     = NULL;
    mmap->size        = 0;
    mmap->offset      = 0;
    mmap->file_handle = -1;
    mmap->access      = COMMC_MMAP_READ_ONLY;
    mmap->flags       = COMMC_MMAP_PRIVATE;
    mmap->is_valid    = 0;
    mmap->is_locked   = 0;

#ifdef _WIN32
    mmap->file_mapping    = NULL;
    mmap->file_handle_win = INVALID_HANDLE_VALUE;
#else
    mmap->fd = -1;
#endif

    return mmap;
}

/*

         commc_mmap_destroy()
	       ---
	       destroys memory mapping structure and unmaps
	       any associated memory.

*/

void commc_mmap_destroy(commc_mmap_t* mmap) {

    if (!validate_mmap(mmap)) {
        return;
    }

    /* unlock if locked */
    if (mmap->is_locked) {
        commc_mmap_unlock(mmap);
    }

    /* unmap memory */
    if (mmap->is_valid) {
        platform_destroy_mapping(mmap);
    }

    /* close file */
    platform_close_file(mmap);

    /* free structure */
    free(mmap);
}

/*

         commc_mmap_map_file()
	       ---
	       maps file into memory with specified parameters.

*/

int commc_mmap_map_file(commc_mmap_t* mmap,
                       const char* filename,
                       commc_mmap_access_t access,
                       commc_mmap_flags_t flags,
                       size_t offset,
                       size_t size,
                       void* preferred_address) {

    size_t file_size;

    if (!validate_mmap(mmap) || !filename) {
        return -1;
    }

    /* ensure not already mapped */
    if (mmap->is_valid) {
        return -1;
    }

    /* store parameters */
    mmap->access = access;
    mmap->flags  = flags;
    mmap->offset = offset;

    /* open file */
    if (platform_open_file(filename, access, mmap) != 0) {
        return -1;
    }

    /* determine size */
    if (size == 0) {
        file_size = platform_get_file_size(mmap);
        if (file_size == 0 || offset >= file_size) {
            platform_close_file(mmap);
            return -1;
        }
        size = file_size - offset;
    }

    mmap->size = size;

    /* create mapping */
    if (platform_create_mapping(mmap) != 0) {
        platform_close_file(mmap);
        return -1;
    }

    mmap->is_valid = 1;
    return 0;
}

/*

         commc_mmap_map_anonymous()
	       ---
	       creates anonymous memory mapping not backed
	       by a file.

*/

int commc_mmap_map_anonymous(commc_mmap_t* mmap,
                            size_t size,
                            commc_mmap_access_t access,
                            commc_mmap_flags_t flags,
                            void* preferred_address) {

    if (!validate_mmap(mmap) || size == 0) {
        return -1;
    }

    /* ensure not already mapped */
    if (mmap->is_valid) {
        return -1;
    }

    /* store parameters */
    mmap->size   = size;
    mmap->offset = 0;
    mmap->access = access;
    mmap->flags  = flags | COMMC_MMAP_ANONYMOUS;
    mmap->file_handle = -1;

#ifdef _WIN32
    mmap->file_handle_win = INVALID_HANDLE_VALUE;
    mmap->file_mapping = NULL;
#else
    mmap->fd = -1;
#endif

    /* create anonymous mapping */
    if (platform_create_mapping(mmap) != 0) {
        return -1;
    }

    mmap->is_valid = 1;
    return 0;
}

/*

         commc_mmap_unmap()
	       ---
	       unmaps memory region but preserves structure.

*/

int commc_mmap_unmap(commc_mmap_t* mmap) {

    if (!validate_mmap(mmap) || !mmap->is_valid) {
        return -1;
    }

    /* unlock if locked */
    if (mmap->is_locked) {
        commc_mmap_unlock(mmap);
    }

    /* destroy mapping */
    platform_destroy_mapping(mmap);

    /* close file if opened */
    platform_close_file(mmap);

    return 0;
}

/*

         commc_mmap_remap()
	       ---
	       remaps memory region with new size or address.

*/

int commc_mmap_remap(commc_mmap_t* mmap,
                    size_t new_size,
                    void* new_address) {

    if (!validate_mmap(mmap) || !mmap->is_valid) {
        return -1;
    }

#ifdef _WIN32
    /* Windows doesn't support in-place remapping */
    /* Would need to unmap and remap */
    return -1;
#else
    void* result;

    result = mremap(mmap->address, mmap->size, new_size, MREMAP_MAYMOVE);
    if (result == MAP_FAILED) {
        return -1;
    }

    mmap->address = result;
    mmap->size = new_size;
    return 0;
#endif
}

/*

         commc_mmap_sync()
	       ---
	       synchronizes mapped memory changes to backing file.

*/

int commc_mmap_sync(commc_mmap_t* mmap,
                   commc_mmap_sync_t sync_mode) {

    if (!validate_mmap(mmap) || !mmap->is_valid) {
        return -1;
    }

#ifdef _WIN32
    if (!FlushViewOfFile(mmap->address, 0)) {
        return -1;
    }
    
    if (sync_mode & COMMC_MMAP_SYNC_SYNC) {
        if (!FlushFileBuffers(mmap->file_handle_win)) {
            return -1;
        }
    }
    
    return 0;
#else
    int flags = 0;

    if (sync_mode & COMMC_MMAP_SYNC_ASYNC) {
        flags |= MS_ASYNC;
    }
    if (sync_mode & COMMC_MMAP_SYNC_SYNC) {
        flags |= MS_SYNC;
    }
    if (sync_mode & COMMC_MMAP_SYNC_INVALIDATE) {
        flags |= MS_INVALIDATE;
    }

    return msync(mmap->address, mmap->size, flags);
#endif
}

/*

         commc_mmap_sync_range()
	       ---
	       synchronizes specific range within mapped memory.

*/

int commc_mmap_sync_range(commc_mmap_t* mmap,
                         size_t offset,
                         size_t length,
                         commc_mmap_sync_t sync_mode) {

    if (!validate_mmap(mmap) || !mmap->is_valid) {
        return -1;
    }

    if (offset >= mmap->size || offset + length > mmap->size) {
        return -1;
    }

#ifdef _WIN32
    void* start_addr = (char*)mmap->address + offset;
    
    if (!FlushViewOfFile(start_addr, length)) {
        return -1;
    }
    
    if (sync_mode & COMMC_MMAP_SYNC_SYNC) {
        if (!FlushFileBuffers(mmap->file_handle_win)) {
            return -1;
        }
    }
    
    return 0;
#else
    int   flags = 0;
    void* start_addr = (char*)mmap->address + offset;

    if (sync_mode & COMMC_MMAP_SYNC_ASYNC) {
        flags |= MS_ASYNC;
    }
    if (sync_mode & COMMC_MMAP_SYNC_SYNC) {
        flags |= MS_SYNC;
    }
    if (sync_mode & COMMC_MMAP_SYNC_INVALIDATE) {
        flags |= MS_INVALIDATE;
    }

    return msync(start_addr, length, flags);
#endif
}

/*

         commc_mmap_lock()
	       ---
	       locks mapped memory pages in physical RAM.

*/

int commc_mmap_lock(commc_mmap_t* mmap) {

    if (!validate_mmap(mmap) || !mmap->is_valid) {
        return -1;
    }

#ifdef _WIN32
    if (!VirtualLock(mmap->address, mmap->size)) {
        return -1;
    }
#else
    if (mlock(mmap->address, mmap->size) != 0) {
        return -1;
    }
#endif

    mmap->is_locked = 1;
    return 0;
}

/*

         commc_mmap_unlock()
	       ---
	       unlocks previously locked memory pages.

*/

int commc_mmap_unlock(commc_mmap_t* mmap) {

    if (!validate_mmap(mmap) || !mmap->is_valid || !mmap->is_locked) {
        return -1;
    }

#ifdef _WIN32
    if (!VirtualUnlock(mmap->address, mmap->size)) {
        return -1;
    }
#else
    if (munlock(mmap->address, mmap->size) != 0) {
        return -1;
    }
#endif

    mmap->is_locked = 0;
    return 0;
}

/*

         commc_mmap_lock_range()
	       ---
	       locks specific range within mapped memory.

*/

int commc_mmap_lock_range(commc_mmap_t* mmap,
                         size_t offset,
                         size_t length) {

    void* start_addr;

    if (!validate_mmap(mmap) || !mmap->is_valid) {
        return -1;
    }

    if (offset >= mmap->size || offset + length > mmap->size) {
        return -1;
    }

    start_addr = (char*)mmap->address + offset;

#ifdef _WIN32
    if (!VirtualLock(start_addr, length)) {
        return -1;
    }
#else
    if (mlock(start_addr, length) != 0) {
        return -1;
    }
#endif

    return 0;
}

/*

         commc_mmap_unlock_range()
	       ---
	       unlocks specific range within mapped memory.

*/

int commc_mmap_unlock_range(commc_mmap_t* mmap,
                           size_t offset,
                           size_t length) {

    void* start_addr;

    if (!validate_mmap(mmap) || !mmap->is_valid) {
        return -1;
    }

    if (offset >= mmap->size || offset + length > mmap->size) {
        return -1;
    }

    start_addr = (char*)mmap->address + offset;

#ifdef _WIN32
    if (!VirtualUnlock(start_addr, length)) {
        return -1;
    }
#else
    if (munlock(start_addr, length) != 0) {
        return -1;
    }
#endif

    return 0;
}

/*

         commc_mmap_protect()
	       ---
	       changes access protection for mapped memory region.

*/

int commc_mmap_protect(commc_mmap_t* mmap,
                      commc_mmap_access_t new_access) {

    if (!validate_mmap(mmap) || !mmap->is_valid) {
        return -1;
    }

#ifdef _WIN32
    DWORD old_protect;
    DWORD new_protect = 0;

    if (new_access & COMMC_MMAP_READ_ONLY) {
        new_protect = PAGE_READONLY;
    }
    if (new_access & COMMC_MMAP_READ_WRITE) {
        new_protect = PAGE_READWRITE;
    }
    if (new_access & COMMC_MMAP_EXEC) {
        if (new_access & COMMC_MMAP_READ_WRITE) {
            new_protect = PAGE_EXECUTE_READWRITE;
        } else {
            new_protect = PAGE_EXECUTE_READ;
        }
    }

    if (!VirtualProtect(mmap->address, mmap->size, new_protect, &old_protect)) {
        return -1;
    }
#else
    int prot = 0;

    if (new_access & COMMC_MMAP_READ_ONLY) {
        prot |= PROT_READ;
    }
    if (new_access & COMMC_MMAP_READ_WRITE) {
        prot |= PROT_READ | PROT_WRITE;
    }
    if (new_access & COMMC_MMAP_EXEC) {
        prot |= PROT_EXEC;
    }

    if (mprotect(mmap->address, mmap->size, prot) != 0) {
        return -1;
    }
#endif

    mmap->access = new_access;
    return 0;
}

/*

         commc_mmap_protect_range()
	       ---
	       changes access protection for specific range.

*/

int commc_mmap_protect_range(commc_mmap_t* mmap,
                            size_t offset,
                            size_t length,
                            commc_mmap_access_t new_access) {

    void* start_addr;

    if (!validate_mmap(mmap) || !mmap->is_valid) {
        return -1;
    }

    if (offset >= mmap->size || offset + length > mmap->size) {
        return -1;
    }

    start_addr = (char*)mmap->address + offset;

#ifdef _WIN32
    DWORD old_protect;
    DWORD new_protect = 0;

    if (new_access & COMMC_MMAP_READ_ONLY) {
        new_protect = PAGE_READONLY;
    }
    if (new_access & COMMC_MMAP_READ_WRITE) {
        new_protect = PAGE_READWRITE;
    }
    if (new_access & COMMC_MMAP_EXEC) {
        if (new_access & COMMC_MMAP_READ_WRITE) {
            new_protect = PAGE_EXECUTE_READWRITE;
        } else {
            new_protect = PAGE_EXECUTE_READ;
        }
    }

    if (!VirtualProtect(start_addr, length, new_protect, &old_protect)) {
        return -1;
    }
#else
    int prot = 0;

    if (new_access & COMMC_MMAP_READ_ONLY) {
        prot |= PROT_READ;
    }
    if (new_access & COMMC_MMAP_READ_WRITE) {
        prot |= PROT_READ | PROT_WRITE;
    }
    if (new_access & COMMC_MMAP_EXEC) {
        prot |= PROT_EXEC;
    }

    if (mprotect(start_addr, length, prot) != 0) {
        return -1;
    }
#endif

    return 0;
}

/*

         commc_mmap_advise()
	       ---
	       provides hints about memory usage patterns.

*/

int commc_mmap_advise(commc_mmap_t* mmap, int advice) {

    if (!validate_mmap(mmap) || !mmap->is_valid) {
        return -1;
    }

#ifdef _WIN32
    /* Windows doesn't have direct equivalent to madvise */
    return 0;
#else
    return madvise(mmap->address, mmap->size, advice);
#endif
}

/*

         commc_mmap_get_address()
	       ---
	       returns address of mapped memory region.

*/

void* commc_mmap_get_address(const commc_mmap_t* mmap) {

    if (!validate_mmap(mmap) || !mmap->is_valid) {
        return NULL;
    }

    return mmap->address;
}

/*

         commc_mmap_get_size()
	       ---
	       returns size of mapped memory region.

*/

size_t commc_mmap_get_size(const commc_mmap_t* mmap) {

    if (!validate_mmap(mmap) || !mmap->is_valid) {
        return 0;
    }

    return mmap->size;
}

/*

         commc_mmap_get_offset()
	       ---
	       returns file offset of mapping.

*/

size_t commc_mmap_get_offset(const commc_mmap_t* mmap) {

    if (!validate_mmap(mmap)) {
        return 0;
    }

    return mmap->offset;
}

/*

         commc_mmap_is_valid()
	       ---
	       checks if mapping is currently valid.

*/

int commc_mmap_is_valid(const commc_mmap_t* mmap) {

    if (!validate_mmap(mmap)) {
        return 0;
    }

    return mmap->is_valid;
}

/*

         commc_mmap_is_locked()
	       ---
	       checks if mapping is locked in memory.

*/

int commc_mmap_is_locked(const commc_mmap_t* mmap) {

    if (!validate_mmap(mmap)) {
        return 0;
    }

    return mmap->is_locked;
}

/*

         commc_mmap_get_page_size()
	       ---
	       returns system memory page size.

*/

size_t commc_mmap_get_page_size(void) {

    init_page_size();
    return page_size;
}

/*

         commc_mmap_align_size()
	       ---
	       aligns size to page boundary.

*/

size_t commc_mmap_align_size(size_t size) {

    init_page_size();
    return ((size + page_size - 1) / page_size) * page_size;
}

/*

         commc_mmap_align_address()
	       ---
	       aligns address to page boundary.

*/

void* commc_mmap_align_address(void* address) {

    uintptr_t addr;
    
    init_page_size();
    addr = (uintptr_t)address;
    addr = (addr / page_size) * page_size;
    
    return (void*)addr;
}

/*
	==================================
             --- EOF ---
	==================================
*/