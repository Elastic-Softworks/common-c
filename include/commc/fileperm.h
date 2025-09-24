/*
   ===================================
   C O M M O N - C
   FILE PERMISSIONS ABSTRACTION API
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- FILEPERM MODULE ---

    cross-platform file permissions abstraction providing
    unified interface for Windows security descriptors
    and Unix permission bits. handles owner, group, and
    other permissions with special flags support.

    abstracts differences between Windows ACLs and Unix
    mode bits while maintaining fine-grained control
    over file access permissions.

*/

#ifndef COMMC_FILEPERM_H
#define COMMC_FILEPERM_H

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

         commc_fileperm_flags_t
	       ---
	       individual permission flags that can be
	       combined using bitwise OR operations.

*/

typedef enum {

    COMMC_FILEPERM_NONE         = 0x0000,   /* NO PERMISSIONS */
    
    /* BASIC READ/WRITE/EXECUTE PERMISSIONS */
    COMMC_FILEPERM_READ         = 0x0001,   /* READ PERMISSION */
    COMMC_FILEPERM_WRITE        = 0x0002,   /* WRITE PERMISSION */
    COMMC_FILEPERM_EXECUTE      = 0x0004,   /* EXECUTE PERMISSION */
    
    /* OWNER PERMISSIONS */
    COMMC_FILEPERM_OWNER_READ   = 0x0100,   /* OWNER READ */
    COMMC_FILEPERM_OWNER_WRITE  = 0x0200,   /* OWNER WRITE */
    COMMC_FILEPERM_OWNER_EXEC   = 0x0400,   /* OWNER EXECUTE */
    
    /* GROUP PERMISSIONS */
    COMMC_FILEPERM_GROUP_READ   = 0x0020,   /* GROUP READ */
    COMMC_FILEPERM_GROUP_WRITE  = 0x0040,   /* GROUP WRITE */
    COMMC_FILEPERM_GROUP_EXEC   = 0x0080,   /* GROUP EXECUTE */
    
    /* OTHER PERMISSIONS */
    COMMC_FILEPERM_OTHER_READ   = 0x0004,   /* OTHER READ */
    COMMC_FILEPERM_OTHER_WRITE  = 0x0008,   /* OTHER WRITE */
    COMMC_FILEPERM_OTHER_EXEC   = 0x0010,   /* OTHER EXECUTE */
    
    /* SPECIAL FLAGS */
    COMMC_FILEPERM_SETUID       = 0x0800,   /* SET USER ID ON EXECUTION */
    COMMC_FILEPERM_SETGID       = 0x0400,   /* SET GROUP ID ON EXECUTION */
    COMMC_FILEPERM_STICKY       = 0x0200,   /* STICKY BIT */
    
    /* CONVENIENCE COMBINATIONS */
    COMMC_FILEPERM_OWNER_ALL    = 0x0700,   /* OWNER READ+WRITE+EXEC */
    COMMC_FILEPERM_GROUP_ALL    = 0x0070,   /* GROUP READ+WRITE+EXEC */
    COMMC_FILEPERM_OTHER_ALL    = 0x0007,   /* OTHER READ+WRITE+EXEC */
    COMMC_FILEPERM_ALL          = 0x0777    /* ALL PERMISSIONS */

} commc_fileperm_flags_t;

/*

         commc_fileperm_t
	       ---
	       represents file permissions in a cross-platform
	       format with owner, group, and access information.

*/

typedef struct {

    unsigned int    permissions;    /* COMBINED PERMISSION FLAGS */
    char*           owner_name;     /* OWNER NAME (IF AVAILABLE) */
    char*           group_name;     /* GROUP NAME (IF AVAILABLE) */
    int             is_directory;   /* DIRECTORY FLAG */
    int             is_hidden;      /* HIDDEN FILE FLAG */
    int             is_readonly;    /* READ-ONLY FLAG */
    int             is_system;      /* SYSTEM FILE FLAG */

#ifdef _WIN32
    void*           security_desc;  /* WINDOWS SECURITY DESCRIPTOR */
#else
    unsigned int    unix_mode;      /* UNIX MODE BITS */
    int             uid;            /* UNIX USER ID */
    int             gid;            /* UNIX GROUP ID */
#endif

} commc_fileperm_t;

/*
	==================================
             --- API ---
	==================================
*/

/*

         commc_fileperm_create()
	       ---
	       creates new permissions structure with
	       default values. must be freed with
	       commc_fileperm_destroy().

*/

commc_fileperm_t* commc_fileperm_create(void);

/*

         commc_fileperm_destroy()
	       ---
	       destroys permissions structure and frees
	       all associated memory including names.

*/

void commc_fileperm_destroy(commc_fileperm_t* perm);

/*

         commc_fileperm_get_from_file()
	       ---
	       retrieves current permissions from specified
	       file or directory path. creates new structure
	       that must be freed.

*/

commc_fileperm_t* commc_fileperm_get_from_file(const char* path);

/*

         commc_fileperm_set_to_file()
	       ---
	       applies permissions structure to specified
	       file or directory. updates file system
	       with new permissions.

*/

commc_error_t commc_fileperm_set_to_file(const char* path,
                                         const commc_fileperm_t* perm);

/*

         commc_fileperm_has_flag()
	       ---
	       checks if permissions structure has
	       specified flag set. returns 1 if set,
	       0 otherwise.

*/

int commc_fileperm_has_flag(const commc_fileperm_t* perm,
                           commc_fileperm_flags_t flag);

/*

         commc_fileperm_set_flag()
	       ---
	       sets specified permission flag in
	       permissions structure. does not
	       update file system until applied.

*/

void commc_fileperm_set_flag(commc_fileperm_t* perm,
                            commc_fileperm_flags_t flag);

/*

         commc_fileperm_clear_flag()
	       ---
	       clears specified permission flag from
	       permissions structure.

*/

void commc_fileperm_clear_flag(commc_fileperm_t* perm,
                              commc_fileperm_flags_t flag);

/*

         commc_fileperm_set_owner()
	       ---
	       sets owner name in permissions structure.
	       makes internal copy of name string.

*/

commc_error_t commc_fileperm_set_owner(commc_fileperm_t* perm,
                                       const char* owner_name);

/*

         commc_fileperm_set_group()
	       ---
	       sets group name in permissions structure.
	       makes internal copy of name string.

*/

commc_error_t commc_fileperm_set_group(commc_fileperm_t* perm,
                                       const char* group_name);

/*

         commc_fileperm_get_owner()
	       ---
	       returns pointer to owner name string or
	       NULL if not set. do not free returned pointer.

*/

const char* commc_fileperm_get_owner(const commc_fileperm_t* perm);

/*

         commc_fileperm_get_group()
	       ---
	       returns pointer to group name string or
	       NULL if not set. do not free returned pointer.

*/

const char* commc_fileperm_get_group(const commc_fileperm_t* perm);

/*

         commc_fileperm_can_read()
	       ---
	       checks if current user can read file based
	       on permissions. considers current user/group
	       context on the platform.

*/

int commc_fileperm_can_read(const commc_fileperm_t* perm,
                           const char* path);

/*

         commc_fileperm_can_write()
	       ---
	       checks if current user can write to file
	       based on permissions and current context.

*/

int commc_fileperm_can_write(const commc_fileperm_t* perm,
                            const char* path);

/*

         commc_fileperm_can_execute()
	       ---
	       checks if current user can execute file
	       based on permissions and platform context.

*/

int commc_fileperm_can_execute(const commc_fileperm_t* perm,
                              const char* path);

/*

         commc_fileperm_to_string()
	       ---
	       converts permissions to human-readable string
	       in Unix-style format (e.g., "rwxr-xr-x").
	       caller must free returned string.

*/

char* commc_fileperm_to_string(const commc_fileperm_t* perm);

/*

         commc_fileperm_from_string()
	       ---
	       parses permissions from Unix-style string
	       format and updates structure accordingly.

*/

commc_error_t commc_fileperm_from_string(commc_fileperm_t* perm,
                                         const char* perm_string);

/*

         commc_fileperm_copy()
	       ---
	       creates deep copy of permissions structure
	       including all names and platform-specific data.

*/

commc_fileperm_t* commc_fileperm_copy(const commc_fileperm_t* perm);

/*

         commc_fileperm_compare()
	       ---
	       compares two permissions structures.
	       returns 0 if identical, non-zero if different.

*/

int commc_fileperm_compare(const commc_fileperm_t* perm1,
                          const commc_fileperm_t* perm2);

#ifdef __cplusplus
}
#endif

#endif /* COMMC_FILEPERM_H */

/*
	==================================
             --- EOF ---
	==================================
*/