/* 	
   ===================================
   P A T H . H 
   CROSS-PLATFORM PATH UTILITIES
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

	                  --- ABOUT ---

	    this module provides comprehensive cross-platform
	    path manipulation utilities with proper normalization,
	    validation, and conversion. handles Windows and Unix
	    path conventions transparently.
	       
	    designed to simplify file system path operations
	    while maintaining compatibility across different
	    operating systems and following C89 standards.

*/

#ifndef COMMC_PATH_H
#define COMMC_PATH_H

/* 
	==================================
             --- SETUP ---
	==================================
*/

#include "error.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 
	==================================
             --- CONST ---
	==================================
*/

#define COMMC_PATH_MAX_LENGTH      4096   /* MAXIMUM PATH LENGTH */
#define COMMC_PATH_MAX_COMPONENTS  256    /* MAX PATH COMPONENTS */

/* 
	==================================
             --- ENUMS ---
	==================================
*/

/*

         commc_path_type_t
	       ---
	       enumeration defining different types of file system
	       paths that can be identified and processed by the
	       path utilities system.

*/

typedef enum {

    COMMC_PATH_ABSOLUTE,    /* ABSOLUTE PATH (/home/user OR C:\Users) */
    COMMC_PATH_RELATIVE,    /* RELATIVE PATH (../file.txt OR ..\file.txt) */
    COMMC_PATH_CURRENT,     /* CURRENT DIRECTORY PATH (./file OR .\file) */
    COMMC_PATH_UNC,         /* UNIVERSAL NAMING CONVENTION (\\server\share) */
    COMMC_PATH_DRIVE        /* DRIVE-RELATIVE PATH (C:file.txt) */
    
} commc_path_type_t;

/*

         commc_path_format_t
	       ---
	       enumeration specifying the target format for
	       path normalization and conversion operations.

*/

typedef enum {

    COMMC_PATH_FORMAT_NATIVE,    /* USE NATIVE OS FORMAT */
    COMMC_PATH_FORMAT_WINDOWS,   /* FORCE WINDOWS FORMAT (BACKSLASH) */
    COMMC_PATH_FORMAT_UNIX,      /* FORCE UNIX FORMAT (FORWARD SLASH) */
    COMMC_PATH_FORMAT_URI        /* URI FORMAT (FORWARD SLASH + PERCENT ENCODING) */
    
} commc_path_format_t;

/* 
	==================================
           --- STRUCTURES ---
	==================================
*/

/*

         commc_path_info_t
	       ---
	       structure containing detailed information about
	       a parsed file system path including type
	       classification, component breakdown, and format.

*/

typedef struct {

    commc_path_type_t   type;                              /* PATH TYPE CLASSIFICATION */
    commc_path_format_t format;                            /* DETECTED PATH FORMAT */
    
    char  drive[8];                                        /* DRIVE LETTER (WINDOWS) */
    char  directory[COMMC_PATH_MAX_LENGTH];                /* DIRECTORY PORTION */
    char  filename[COMMC_PATH_MAX_LENGTH];                 /* FILENAME PORTION */
    char  extension[COMMC_PATH_MAX_LENGTH];                /* FILE EXTENSION */
    
    char* components[COMMC_PATH_MAX_COMPONENTS];           /* PATH COMPONENT ARRAY */
    int   component_count;                                 /* NUMBER OF COMPONENTS */
    
    int   is_directory;                                    /* DIRECTORY FLAG */
    int   has_extension;                                   /* EXTENSION PRESENT FLAG */

} commc_path_info_t;

/*

         commc_path_builder_t
	       ---
	       structure for building paths incrementally with
	       automatic normalization and validation. provides
	       a clean interface for constructing complex paths.

*/

typedef struct {

    char  buffer[COMMC_PATH_MAX_LENGTH];                   /* PATH BUILD BUFFER */
    int   length;                                          /* CURRENT PATH LENGTH */
    
    commc_path_format_t format;                            /* TARGET FORMAT */
    
    int   normalized;                                      /* NORMALIZATION FLAG */
    int   absolute;                                        /* ABSOLUTE PATH FLAG */

} commc_path_builder_t;

/* 
	==================================
             --- CORE ---
	==================================
*/

/*

         commc_path_normalize()
	       ---
	       normalizes a file system path by resolving relative
	       components (. and ..), removing redundant separators,
	       and converting to the specified format.
	       
	       handles cross-platform path conventions and ensures
	       the resulting path follows standard formatting rules.

*/

commc_error_t commc_path_normalize(const char*         input_path,
                                   char*               output_path,
                                   size_t              output_size,
                                   commc_path_format_t format);

/*

         commc_path_join()
	       ---
	       joins multiple path components into a single path
	       with proper separator handling. automatically adds
	       separators between components as needed.

*/

commc_error_t commc_path_join(char*               output_path,
                              size_t              output_size,
                              commc_path_format_t format,
                              int                 component_count,
                              ...);

/*

         commc_path_split()
	       ---
	       splits a path into its component parts including
	       drive, directory, filename, and extension. provides
	       detailed breakdown for path analysis.

*/

commc_error_t commc_path_split(const char*      path,
                               commc_path_info_t* info);

/*

         commc_path_basename()
	       ---
	       extracts the filename portion from a path,
	       optionally removing the extension. equivalent
	       to basename() function on Unix systems.

*/

commc_error_t commc_path_basename(const char* path,
                                  char*       basename,
                                  size_t      basename_size,
                                  int         strip_extension);

/*

         commc_path_dirname()
	       ---
	       extracts the directory portion from a path,
	       removing the filename component. equivalent
	       to dirname() function on Unix systems.

*/

commc_error_t commc_path_dirname(const char* path,
                                 char*       dirname,
                                 size_t      dirname_size);

/* 
	==================================
           --- VALIDATION ---
	==================================
*/

/*

         commc_path_is_absolute()
	       ---
	       determines whether a path is absolute or relative.
	       handles platform-specific absolute path formats
	       including Unix root and Windows drive letters.

*/

int commc_path_is_absolute(const char* path);

/*

         commc_path_is_valid()
	       ---
	       validates a path string for proper format and
	       legal characters. checks for reserved names
	       and invalid character sequences.

*/

int commc_path_is_valid(const char* path);

/*

         commc_path_exists()
	       ---
	       checks whether a path exists in the file system.
	       performs actual file system lookup to verify
	       path existence.

*/

int commc_path_exists(const char* path);

/*

         commc_path_get_type()
	       ---
	       analyzes a path string and determines its type
	       classification (absolute, relative, UNC, etc.).

*/

commc_path_type_t commc_path_get_type(const char* path);

/* 
	==================================
           --- CONVERSION ---
	==================================
*/

/*

         commc_path_to_absolute()
	       ---
	       converts a relative path to an absolute path
	       by resolving it against the current working
	       directory or a specified base path.

*/

commc_error_t commc_path_to_absolute(const char* relative_path,
                                     const char* base_path,
                                     char*       absolute_path,
                                     size_t      absolute_size);

/*

         commc_path_to_relative()
	       ---
	       converts an absolute path to a relative path
	       based on a reference directory. useful for
	       creating portable path references.

*/

commc_error_t commc_path_to_relative(const char* absolute_path,
                                     const char* base_path,
                                     char*       relative_path,
                                     size_t      relative_size);

/*

         commc_path_convert_separators()
	       ---
	       converts path separators between Windows backslash
	       and Unix forward slash formats. preserves UNC
	       prefixes and handles mixed separator paths.

*/

commc_error_t commc_path_convert_separators(const char*         input_path,
                                            char*               output_path,
                                            size_t              output_size,
                                            commc_path_format_t format);

/* 
	==================================
             --- BUILDER ---
	==================================
*/

/*

         commc_path_builder_create()
	       ---
	       creates and initializes a path builder for
	       incremental path construction with automatic
	       normalization and validation.

*/

commc_error_t commc_path_builder_create(commc_path_builder_t* builder,
                                        commc_path_format_t   format);

/*

         commc_path_builder_append()
	       ---
	       appends a path component to the builder with
	       automatic separator insertion and validation.

*/

commc_error_t commc_path_builder_append(commc_path_builder_t* builder,
                                        const char*           component);

/*

         commc_path_builder_append_extension()
	       ---
	       appends a file extension to the current path
	       in the builder, automatically adding the dot
	       separator if not present.

*/

commc_error_t commc_path_builder_append_extension(commc_path_builder_t* builder,
                                                   const char*           extension);

/*

         commc_path_builder_get()
	       ---
	       retrieves the current path from the builder,
	       applying final normalization and validation.

*/

commc_error_t commc_path_builder_get(commc_path_builder_t* builder,
                                     char*                 output_path,
                                     size_t                output_size);

/*

         commc_path_builder_reset()
	       ---
	       resets the path builder to empty state while
	       preserving format and configuration settings.

*/

commc_error_t commc_path_builder_reset(commc_path_builder_t* builder);

/* 
	==================================
           --- UTILITIES ---
	==================================
*/

/*

         commc_path_get_extension()
	       ---
	       extracts the file extension from a path,
	       returning a pointer to the extension string
	       (including the dot) or NULL if no extension.

*/

const char* commc_path_get_extension(const char* path);

/*

         commc_path_has_extension()
	       ---
	       checks whether a path contains a file extension.
	       optionally validates against a specific extension.

*/

int commc_path_has_extension(const char* path,
                             const char* expected_extension);

/*

         commc_path_change_extension()
	       ---
	       replaces the file extension in a path with
	       a new extension, handling cases where no
	       extension exists.

*/

commc_error_t commc_path_change_extension(const char* input_path,
                                          const char* new_extension,
                                          char*       output_path,
                                          size_t      output_size);

/*

         commc_path_get_common_prefix()
	       ---
	       finds the common directory prefix between two
	       paths. useful for path comparison and relative
	       path calculation.

*/

commc_error_t commc_path_get_common_prefix(const char* path1,
                                           const char* path2,
                                           char*       common_prefix,
                                           size_t      prefix_size);

/*

         commc_path_compare()
	       ---
	       compares two paths for equality with platform-aware
	       case sensitivity handling. normalizes paths before
	       comparison for accurate results.

*/

int commc_path_compare(const char* path1,
                       const char* path2);

/* 
	==================================
           --- PLATFORM ---
	==================================
*/

/*

         commc_path_get_separator()
	       ---
	       returns the appropriate path separator character
	       for the specified format or native platform.

*/

char commc_path_get_separator(commc_path_format_t format);

/*

         commc_path_get_current_directory()
	       ---
	       retrieves the current working directory path
	       in the specified format.

*/

commc_error_t commc_path_get_current_directory(char*               output_path,
                                               size_t              output_size,
                                               commc_path_format_t format);

/*

         commc_path_get_temp_directory()
	       ---
	       retrieves the system temporary directory path
	       based on environment variables and platform
	       conventions.

*/

commc_error_t commc_path_get_temp_directory(char*               output_path,
                                            size_t              output_size,
                                            commc_path_format_t format);

/*

         commc_path_get_home_directory()
	       ---
	       retrieves the user's home directory path
	       based on environment variables and platform
	       conventions.

*/

commc_error_t commc_path_get_home_directory(char*               output_path,
                                            size_t              output_size,
                                            commc_path_format_t format);

#ifdef __cplusplus
}
#endif

#endif /* COMMC_PATH_H */

/* 
	==================================
             --- EOF ---
	==================================
*/