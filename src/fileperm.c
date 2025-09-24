/*
   ===================================
   C O M M O N - C
   FILE PERMISSIONS ABSTRACTION IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- FILEPERM MODULE ---

    implementation of cross-platform file permissions
    with Windows security descriptors and Unix mode bits.
    provides unified interface for permission management
    across different operating systems.

    see include/commc/fileperm.h for function
    prototypes and documentation.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#include <stdlib.h>        /* for malloc, free */
#include <string.h>        /* for strlen, strcpy */
#include <stdio.h>         /* for sprintf */

#ifdef _WIN32
    #include <windows.h>   /* for GetFileSecurity, SetFileSecurity */
    #include <aclapi.h>    /* for GetNamedSecurityInfo */
    #include <sddl.h>      /* for ConvertSidToStringSid */
#else
    #include <sys/stat.h>  /* for stat, chmod */
    #include <unistd.h>    /* for access, getuid, getgid */
    #include <pwd.h>       /* for getpwuid */
    #include <grp.h>       /* for getgrgid */
    #include <errno.h>     /* for errno */
#endif

#include "commc/fileperm.h"
#include "commc/error.h"

/*
	==================================
             --- HELPERS ---
	==================================
*/

/*

         validate_perm()
	       ---
	       validates that permissions structure is
	       properly initialized.

*/

static int validate_perm(const commc_fileperm_t* perm) {

    return (perm != NULL);
}

/*

         safe_string_copy()
	       ---
	       safely copies string with memory allocation.
	       returns allocated copy or NULL on failure.

*/

static char* safe_string_copy(const char* src) {

    char*  copy;
    size_t len;

    if (!src) {
        return NULL;
    }

    len = strlen(src);
    copy = (char*)malloc(len + 1);
    if (!copy) {
        return NULL;
    }

    strcpy(copy, src);
    return copy;
}

/*

         platform_get_permissions()
	       ---
	       retrieves platform-specific file permissions
	       and populates permission structure.

*/

static commc_error_t platform_get_permissions(const char* path,
                                              commc_fileperm_t* perm) {

#ifdef _WIN32

    DWORD                 result;
    PSECURITY_DESCRIPTOR  security_desc = NULL;
    PSID                  owner_sid = NULL;
    PSID                  group_sid = NULL;
    PACL                  dacl = NULL;
    DWORD                 attributes;
    LPSTR                 owner_name = NULL;
    LPSTR                 group_name = NULL;

    /* get file attributes */
    attributes = GetFileAttributesA(path);
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        return COMMC_ERROR_SYSTEM;
    }

    /* set basic flags */
    perm->is_directory = (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    perm->is_hidden    = (attributes & FILE_ATTRIBUTE_HIDDEN) != 0;
    perm->is_readonly  = (attributes & FILE_ATTRIBUTE_READONLY) != 0;
    perm->is_system    = (attributes & FILE_ATTRIBUTE_SYSTEM) != 0;

    /* get security information */
    result = GetNamedSecurityInfoA((LPSTR)path, SE_FILE_OBJECT,
                                   OWNER_SECURITY_INFORMATION |
                                   GROUP_SECURITY_INFORMATION |
                                   DACL_SECURITY_INFORMATION,
                                   &owner_sid, &group_sid, &dacl, NULL,
                                   &security_desc);

    if (result != ERROR_SUCCESS) {
        return COMMC_ERROR_SYSTEM;
    }

    /* convert SIDs to names */
    if (owner_sid) {
        ConvertSidToStringSidA(owner_sid, &owner_name);
        if (owner_name) {
            perm->owner_name = safe_string_copy(owner_name);
            LocalFree(owner_name);
        }
    }

    if (group_sid) {
        ConvertSidToStringSidA(group_sid, &group_name);
        if (group_name) {
            perm->group_name = safe_string_copy(group_name);
            LocalFree(group_name);
        }
    }

    /* store security descriptor for later use */
    perm->security_desc = security_desc;

    /* set basic permissions based on readonly flag */
    perm->permissions = COMMC_FILEPERM_OWNER_READ | COMMC_FILEPERM_GROUP_READ | COMMC_FILEPERM_OTHER_READ;
    if (!perm->is_readonly) {
        perm->permissions |= COMMC_FILEPERM_OWNER_WRITE;
    }
    if (!perm->is_directory && (attributes & FILE_ATTRIBUTE_READONLY) == 0) {
        perm->permissions |= COMMC_FILEPERM_OWNER_EXEC;
    }

    return COMMC_ERROR_NONE;

#else

    struct stat   st;
    struct passwd *pw;
    struct group  *gr;

    /* get file stats */
    if (stat(path, &st) != 0) {
        return COMMC_ERROR_SYSTEM;
    }

    /* store Unix-specific information */
    perm->unix_mode = st.st_mode;
    perm->uid = st.st_uid;
    perm->gid = st.st_gid;

    /* set basic flags */
    perm->is_directory = S_ISDIR(st.st_mode);
    perm->is_hidden    = 0;  /* Unix doesn't have hidden attribute */
    perm->is_readonly  = (access(path, W_OK) != 0);
    perm->is_system    = 0;  /* Unix doesn't have system attribute */

    /* convert permissions */
    perm->permissions = 0;
    if (st.st_mode & S_IRUSR) perm->permissions |= COMMC_FILEPERM_OWNER_READ;
    if (st.st_mode & S_IWUSR) perm->permissions |= COMMC_FILEPERM_OWNER_WRITE;
    if (st.st_mode & S_IXUSR) perm->permissions |= COMMC_FILEPERM_OWNER_EXEC;
    if (st.st_mode & S_IRGRP) perm->permissions |= COMMC_FILEPERM_GROUP_READ;
    if (st.st_mode & S_IWGRP) perm->permissions |= COMMC_FILEPERM_GROUP_WRITE;
    if (st.st_mode & S_IXGRP) perm->permissions |= COMMC_FILEPERM_GROUP_EXEC;
    if (st.st_mode & S_IROTH) perm->permissions |= COMMC_FILEPERM_OTHER_READ;
    if (st.st_mode & S_IWOTH) perm->permissions |= COMMC_FILEPERM_OTHER_WRITE;
    if (st.st_mode & S_IXOTH) perm->permissions |= COMMC_FILEPERM_OTHER_EXEC;

    /* special flags */
    if (st.st_mode & S_ISUID) perm->permissions |= COMMC_FILEPERM_SETUID;
    if (st.st_mode & S_ISGID) perm->permissions |= COMMC_FILEPERM_SETGID;
    if (st.st_mode & S_ISVTX) perm->permissions |= COMMC_FILEPERM_STICKY;

    /* get owner name */
    pw = getpwuid(st.st_uid);
    if (pw && pw->pw_name) {
        perm->owner_name = safe_string_copy(pw->pw_name);
    }

    /* get group name */
    gr = getgrgid(st.st_gid);
    if (gr && gr->gr_name) {
        perm->group_name = safe_string_copy(gr->gr_name);
    }

    return COMMC_ERROR_NONE;

#endif
}

/*

         platform_set_permissions()
	       ---
	       applies permissions to file using
	       platform-specific mechanisms.

*/

static commc_error_t platform_set_permissions(const char* path,
                                              const commc_fileperm_t* perm) {

#ifdef _WIN32

    DWORD attributes;

    /* set basic file attributes */
    attributes = 0;
    if (perm->is_directory) attributes |= FILE_ATTRIBUTE_DIRECTORY;
    if (perm->is_hidden)    attributes |= FILE_ATTRIBUTE_HIDDEN;
    if (perm->is_readonly)  attributes |= FILE_ATTRIBUTE_READONLY;
    if (perm->is_system)    attributes |= FILE_ATTRIBUTE_SYSTEM;

    if (attributes == 0) {
        attributes = FILE_ATTRIBUTE_NORMAL;
    }

    if (!SetFileAttributesA(path, attributes)) {
        return COMMC_ERROR_SYSTEM;
    }

    /* TODO: Set ACL permissions if security_desc is available */

    return COMMC_ERROR_NONE;

#else

    mode_t mode = 0;

    /* convert permissions back to Unix mode */
    if (perm->permissions & COMMC_FILEPERM_OWNER_READ)  mode |= S_IRUSR;
    if (perm->permissions & COMMC_FILEPERM_OWNER_WRITE) mode |= S_IWUSR;
    if (perm->permissions & COMMC_FILEPERM_OWNER_EXEC)  mode |= S_IXUSR;
    if (perm->permissions & COMMC_FILEPERM_GROUP_READ)  mode |= S_IRGRP;
    if (perm->permissions & COMMC_FILEPERM_GROUP_WRITE) mode |= S_IWGRP;
    if (perm->permissions & COMMC_FILEPERM_GROUP_EXEC)  mode |= S_IXGRP;
    if (perm->permissions & COMMC_FILEPERM_OTHER_READ)  mode |= S_IROTH;
    if (perm->permissions & COMMC_FILEPERM_OTHER_WRITE) mode |= S_IWOTH;
    if (perm->permissions & COMMC_FILEPERM_OTHER_EXEC)  mode |= S_IXOTH;

    /* special flags */
    if (perm->permissions & COMMC_FILEPERM_SETUID) mode |= S_ISUID;
    if (perm->permissions & COMMC_FILEPERM_SETGID) mode |= S_ISGID;
    if (perm->permissions & COMMC_FILEPERM_STICKY) mode |= S_ISVTX;

    /* apply permissions */
    if (chmod(path, mode) != 0) {
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

         commc_fileperm_create()
	       ---
	       creates new permissions structure with
	       default initialization.

*/

commc_fileperm_t* commc_fileperm_create(void) {

    commc_fileperm_t* perm;

    perm = (commc_fileperm_t*)malloc(sizeof(commc_fileperm_t));
    if (!perm) {
        return NULL;
    }

    /* initialize with default values */
    memset(perm, 0, sizeof(commc_fileperm_t));
    perm->permissions   = COMMC_FILEPERM_OWNER_READ | COMMC_FILEPERM_OWNER_WRITE;
    perm->owner_name    = NULL;
    perm->group_name    = NULL;
    perm->is_directory  = 0;
    perm->is_hidden     = 0;
    perm->is_readonly   = 0;
    perm->is_system     = 0;

#ifdef _WIN32
    perm->security_desc = NULL;
#else
    perm->unix_mode = 0644;
    perm->uid = -1;
    perm->gid = -1;
#endif

    return perm;
}

/*

         commc_fileperm_destroy()
	       ---
	       destroys permissions structure and frees memory.

*/

void commc_fileperm_destroy(commc_fileperm_t* perm) {

    if (!perm) {
        return;
    }

    /* free name strings */
    if (perm->owner_name) {
        free(perm->owner_name);
        perm->owner_name = NULL;
    }

    if (perm->group_name) {
        free(perm->group_name);
        perm->group_name = NULL;
    }

#ifdef _WIN32
    /* free Windows security descriptor */
    if (perm->security_desc) {
        LocalFree(perm->security_desc);
        perm->security_desc = NULL;
    }
#endif

    /* free the structure */
    free(perm);
}

/*

         commc_fileperm_get_from_file()
	       ---
	       retrieves permissions from file and creates
	       new structure.

*/

commc_fileperm_t* commc_fileperm_get_from_file(const char* path) {

    commc_fileperm_t* perm;
    commc_error_t     result;

    if (!path) {
        return NULL;
    }

    perm = commc_fileperm_create();
    if (!perm) {
        return NULL;
    }

    result = platform_get_permissions(path, perm);
    if (result != COMMC_ERROR_NONE) {
        commc_fileperm_destroy(perm);
        return NULL;
    }

    return perm;
}

/*

         commc_fileperm_set_to_file()
	       ---
	       applies permissions structure to file.

*/

commc_error_t commc_fileperm_set_to_file(const char* path,
                                         const commc_fileperm_t* perm) {

    if (!path || !validate_perm(perm)) {
        return COMMC_ERROR_INVALID_ARG;
    }

    return platform_set_permissions(path, perm);
}

/*

         commc_fileperm_has_flag()
	       ---
	       checks if permission flag is set.

*/

int commc_fileperm_has_flag(const commc_fileperm_t* perm,
                           commc_fileperm_flags_t flag) {

    if (!validate_perm(perm)) {
        return 0;
    }

    return (perm->permissions & flag) != 0;
}

/*

         commc_fileperm_set_flag()
	       ---
	       sets permission flag in structure.

*/

void commc_fileperm_set_flag(commc_fileperm_t* perm,
                            commc_fileperm_flags_t flag) {

    if (!validate_perm(perm)) {
        return;
    }

    perm->permissions |= flag;
}

/*

         commc_fileperm_clear_flag()
	       ---
	       clears permission flag from structure.

*/

void commc_fileperm_clear_flag(commc_fileperm_t* perm,
                              commc_fileperm_flags_t flag) {

    if (!validate_perm(perm)) {
        return;
    }

    perm->permissions &= ~flag;
}

/*

         commc_fileperm_set_owner()
	       ---
	       sets owner name with string copy.

*/

commc_error_t commc_fileperm_set_owner(commc_fileperm_t* perm,
                                       const char* owner_name) {

    if (!validate_perm(perm)) {
        return COMMC_ERROR_INVALID_ARG;
    }

    /* free existing name */
    if (perm->owner_name) {
        free(perm->owner_name);
        perm->owner_name = NULL;
    }

    /* copy new name */
    if (owner_name) {
        perm->owner_name = safe_string_copy(owner_name);
        if (!perm->owner_name) {
            return COMMC_ERROR_OUT_OF_MEMORY;
        }
    }

    return COMMC_ERROR_NONE;
}

/*

         commc_fileperm_set_group()
	       ---
	       sets group name with string copy.

*/

commc_error_t commc_fileperm_set_group(commc_fileperm_t* perm,
                                       const char* group_name) {

    if (!validate_perm(perm)) {
        return COMMC_ERROR_INVALID_ARG;
    }

    /* free existing name */
    if (perm->group_name) {
        free(perm->group_name);
        perm->group_name = NULL;
    }

    /* copy new name */
    if (group_name) {
        perm->group_name = safe_string_copy(group_name);
        if (!perm->group_name) {
            return COMMC_ERROR_OUT_OF_MEMORY;
        }
    }

    return COMMC_ERROR_NONE;
}

/*

         commc_fileperm_get_owner()
	       ---
	       returns owner name pointer.

*/

const char* commc_fileperm_get_owner(const commc_fileperm_t* perm) {

    if (!validate_perm(perm)) {
        return NULL;
    }

    return perm->owner_name;
}

/*

         commc_fileperm_get_group()
	       ---
	       returns group name pointer.

*/

const char* commc_fileperm_get_group(const commc_fileperm_t* perm) {

    if (!validate_perm(perm)) {
        return NULL;
    }

    return perm->group_name;
}

/*

         commc_fileperm_can_read()
	       ---
	       checks read access for current user.

*/

int commc_fileperm_can_read(const commc_fileperm_t* perm,
                           const char* path) {

    if (!validate_perm(perm) || !path) {
        return 0;
    }

#ifdef _WIN32
    return (GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES);
#else
    return (access(path, R_OK) == 0);
#endif
}

/*

         commc_fileperm_can_write()
	       ---
	       checks write access for current user.

*/

int commc_fileperm_can_write(const commc_fileperm_t* perm,
                            const char* path) {

    if (!validate_perm(perm) || !path) {
        return 0;
    }

#ifdef _WIN32
    DWORD attrs = GetFileAttributesA(path);
    return (attrs != INVALID_FILE_ATTRIBUTES && 
            (attrs & FILE_ATTRIBUTE_READONLY) == 0);
#else
    return (access(path, W_OK) == 0);
#endif
}

/*

         commc_fileperm_can_execute()
	       ---
	       checks execute access for current user.

*/

int commc_fileperm_can_execute(const commc_fileperm_t* perm,
                              const char* path) {

    if (!validate_perm(perm) || !path) {
        return 0;
    }

#ifdef _WIN32
    /* Windows doesn't have execute permission concept like Unix */
    return 1;
#else
    return (access(path, X_OK) == 0);
#endif
}

/*

         commc_fileperm_to_string()
	       ---
	       converts permissions to Unix-style string.

*/

char* commc_fileperm_to_string(const commc_fileperm_t* perm) {

    char* str;

    if (!validate_perm(perm)) {
        return NULL;
    }

    str = (char*)malloc(12);  /* "rwxrwxrwx" + special flags + null */
    if (!str) {
        return NULL;
    }

    /* owner permissions */
    str[0] = (perm->permissions & COMMC_FILEPERM_OWNER_READ)  ? 'r' : '-';
    str[1] = (perm->permissions & COMMC_FILEPERM_OWNER_WRITE) ? 'w' : '-';
    str[2] = (perm->permissions & COMMC_FILEPERM_OWNER_EXEC)  ? 'x' : '-';

    /* group permissions */
    str[3] = (perm->permissions & COMMC_FILEPERM_GROUP_READ)  ? 'r' : '-';
    str[4] = (perm->permissions & COMMC_FILEPERM_GROUP_WRITE) ? 'w' : '-';
    str[5] = (perm->permissions & COMMC_FILEPERM_GROUP_EXEC)  ? 'x' : '-';

    /* other permissions */
    str[6] = (perm->permissions & COMMC_FILEPERM_OTHER_READ)  ? 'r' : '-';
    str[7] = (perm->permissions & COMMC_FILEPERM_OTHER_WRITE) ? 'w' : '-';
    str[8] = (perm->permissions & COMMC_FILEPERM_OTHER_EXEC)  ? 'x' : '-';

    str[9] = '\0';

    return str;
}

/*

         commc_fileperm_from_string()
	       ---
	       parses permissions from Unix-style string.

*/

commc_error_t commc_fileperm_from_string(commc_fileperm_t* perm,
                                         const char* perm_string) {

    if (!validate_perm(perm) || !perm_string) {
        return COMMC_ERROR_INVALID_ARG;
    }

    if (strlen(perm_string) < 9) {
        return COMMC_ERROR_INVALID_ARG;
    }

    /* clear existing permissions */
    perm->permissions = 0;

    /* parse owner permissions */
    if (perm_string[0] == 'r') perm->permissions |= COMMC_FILEPERM_OWNER_READ;
    if (perm_string[1] == 'w') perm->permissions |= COMMC_FILEPERM_OWNER_WRITE;
    if (perm_string[2] == 'x') perm->permissions |= COMMC_FILEPERM_OWNER_EXEC;

    /* parse group permissions */
    if (perm_string[3] == 'r') perm->permissions |= COMMC_FILEPERM_GROUP_READ;
    if (perm_string[4] == 'w') perm->permissions |= COMMC_FILEPERM_GROUP_WRITE;
    if (perm_string[5] == 'x') perm->permissions |= COMMC_FILEPERM_GROUP_EXEC;

    /* parse other permissions */
    if (perm_string[6] == 'r') perm->permissions |= COMMC_FILEPERM_OTHER_READ;
    if (perm_string[7] == 'w') perm->permissions |= COMMC_FILEPERM_OTHER_WRITE;
    if (perm_string[8] == 'x') perm->permissions |= COMMC_FILEPERM_OTHER_EXEC;

    return COMMC_ERROR_NONE;
}

/*

         commc_fileperm_copy()
	       ---
	       creates deep copy of permissions structure.

*/

commc_fileperm_t* commc_fileperm_copy(const commc_fileperm_t* perm) {

    commc_fileperm_t* copy;

    if (!validate_perm(perm)) {
        return NULL;
    }

    copy = commc_fileperm_create();
    if (!copy) {
        return NULL;
    }

    /* copy basic fields */
    copy->permissions  = perm->permissions;
    copy->is_directory = perm->is_directory;
    copy->is_hidden    = perm->is_hidden;
    copy->is_readonly  = perm->is_readonly;
    copy->is_system    = perm->is_system;

    /* copy names */
    if (perm->owner_name) {
        commc_fileperm_set_owner(copy, perm->owner_name);
    }
    if (perm->group_name) {
        commc_fileperm_set_group(copy, perm->group_name);
    }

#ifndef _WIN32
    copy->unix_mode = perm->unix_mode;
    copy->uid = perm->uid;
    copy->gid = perm->gid;
#endif

    return copy;
}

/*

         commc_fileperm_compare()
	       ---
	       compares two permissions structures.

*/

int commc_fileperm_compare(const commc_fileperm_t* perm1,
                          const commc_fileperm_t* perm2) {

    if (!validate_perm(perm1) || !validate_perm(perm2)) {
        return -1;
    }

    if (perm1->permissions != perm2->permissions) {
        return 1;
    }

    if (perm1->is_directory != perm2->is_directory ||
        perm1->is_hidden != perm2->is_hidden ||
        perm1->is_readonly != perm2->is_readonly ||
        perm1->is_system != perm2->is_system) {
        return 1;
    }

    return 0;
}

/*
	==================================
             --- EOF ---
	==================================
*/