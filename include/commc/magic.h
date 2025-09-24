/*

   ===================================
   C O M M C   -   M A G I C
   MAGIC NUMBER FILE FORMAT DETECTION
   ELASTIC SOFTWORKS 2025
   ===================================

*/

#ifndef COMMC_MAGIC_H
#define COMMC_MAGIC_H

#include "error.h"
#include <stdio.h>

/* 
	==================================
          --- TYPE DEFS ---
	==================================
*/

/*

	commc_magic_signature_t
	---
	structure defining a file format signature with
	magic number pattern, offset position, mask for
	flexible matching, and associated file type
	information for extensible format detection.

*/

typedef struct {

    unsigned char* pattern;     /* MAGIC NUMBER BYTE PATTERN */
    int            pattern_len; /* LENGTH OF PATTERN IN BYTES */
    int            offset;      /* OFFSET FROM FILE START */
    unsigned char* mask;        /* OPTIONAL MASK FOR PATTERN MATCHING */
    
    const char*    mime_type;   /* MIME TYPE STRING */
    const char*    extension;   /* PRIMARY FILE EXTENSION */
    const char*    description; /* HUMAN-READABLE FORMAT NAME */
    int            confidence;  /* CONFIDENCE LEVEL (0-100) */

} commc_magic_signature_t;

/*

	commc_magic_database_t
	---
	collection of file format signatures with
	efficient lookup and management capabilities.
	supports dynamic signature addition and
	priority-based matching for format detection.

*/

typedef struct {

    commc_magic_signature_t** signatures;  /* ARRAY OF SIGNATURE POINTERS */
    int                       count;        /* NUMBER OF SIGNATURES */
    int                       capacity;     /* ALLOCATED CAPACITY */
    
    int                       sorted;       /* SORTED BY PRIORITY FLAG */

} commc_magic_database_t;

/*

	commc_magic_result_t
	---
	file format detection result containing
	matched signature information, confidence
	level, and additional metadata about the
	detected file format.

*/

typedef struct {

    const char* mime_type;      /* DETECTED MIME TYPE */
    const char* extension;      /* PRIMARY FILE EXTENSION */
    const char* description;    /* FORMAT DESCRIPTION */
    int         confidence;     /* CONFIDENCE LEVEL (0-100) */
    int         match_offset;   /* BYTE OFFSET OF MATCH */
    int         match_length;   /* LENGTH OF MATCHED PATTERN */

} commc_magic_result_t;

/*

	commc_magic_context_t
	---
	file format detection context with database
	reference, analysis options, and caching for
	efficient repeated detection operations on
	similar file types.

*/

typedef struct {

    commc_magic_database_t* database;      /* SIGNATURE DATABASE */
    
    int                     analyze_depth; /* BYTES TO ANALYZE (0 = ALL) */
    int                     require_exact; /* EXACT MATCH REQUIRED FLAG */
    int                     use_filename;  /* CONSIDER FILENAME HINTS */
    
    commc_error_context_t   error;         /* ERROR CONTEXT INFORMATION */

} commc_magic_context_t;

/* 
	==================================
         --- DATABASE API ---
	==================================
*/

/*

         commc_magic_database_create()
	       ---
	       creates new magic number database with
	       built-in signatures for common file formats.
	       initializes with standard format detection
	       patterns for immediate use.

*/

commc_magic_database_t* commc_magic_database_create(void);

/*

         commc_magic_database_destroy()
	       ---
	       releases all memory associated with magic
	       database including signatures and pattern
	       data. prevents memory leaks during database
	       cleanup operations.

*/

void commc_magic_database_destroy(commc_magic_database_t* database);

/*

         commc_magic_database_add_signature()
	       ---
	       adds custom file format signature to database
	       with pattern matching configuration. supports
	       extensible format detection with user-defined
	       magic numbers and metadata.

*/

int commc_magic_database_add_signature(commc_magic_database_t* database,
                                       const unsigned char* pattern,
                                       int pattern_len,
                                       int offset,
                                       const unsigned char* mask,
                                       const char* mime_type,
                                       const char* extension,
                                       const char* description,
                                       int confidence);

/*

         commc_magic_database_remove_signature()
	       ---
	       removes signature from database by MIME type
	       or extension. handles memory cleanup and
	       database reorganization to maintain structure
	       integrity after signature removal.

*/

int commc_magic_database_remove_signature(commc_magic_database_t* database, const char* mime_type);

/*

         commc_magic_database_load_from_file()
	       ---
	       loads magic number database from configuration
	       file with standard magic format. supports
	       external signature databases and custom
	       format definitions for extensibility.

*/

int commc_magic_database_load_from_file(commc_magic_database_t* database, const char* filename);

/*

         commc_magic_database_save_to_file()
	       ---
	       saves current database to configuration file
	       for persistence and sharing. generates
	       standard magic format output for external
	       tools and database management.

*/

int commc_magic_database_save_to_file(const commc_magic_database_t* database, const char* filename);

/*

         commc_magic_database_get_signature_count()
	       ---
	       returns number of signatures in database
	       for iteration and statistics. useful for
	       database management and capacity planning
	       during signature operations.

*/

int commc_magic_database_get_signature_count(const commc_magic_database_t* database);

/* 
	==================================
         --- DETECTION API ---
	==================================
*/

/*

         commc_magic_context_create()
	       ---
	       creates file format detection context with
	       database reference and analysis configuration.
	       sets up detection parameters and caching
	       for efficient format identification.

*/

commc_magic_context_t* commc_magic_context_create(commc_magic_database_t* database);

/*

         commc_magic_context_destroy()
	       ---
	       releases memory associated with detection
	       context including cached data and configuration.
	       prevents memory leaks during context
	       cleanup operations.

*/

void commc_magic_context_destroy(commc_magic_context_t* context);

/*

         commc_magic_detect_file()
	       ---
	       detects file format by analyzing magic numbers
	       in specified file. returns best match result
	       with confidence level and format metadata
	       for file type identification.

*/

commc_magic_result_t* commc_magic_detect_file(commc_magic_context_t* context, const char* filename);

/*

         commc_magic_detect_buffer()
	       ---
	       detects file format from memory buffer by
	       analyzing magic number patterns. supports
	       in-memory format detection without file
	       system access for stream processing.

*/

commc_magic_result_t* commc_magic_detect_buffer(commc_magic_context_t* context, const unsigned char* buffer, int buffer_len);

/*

         commc_magic_detect_stream()
	       ---
	       detects file format from open file stream
	       with automatic position restoration. analyzes
	       magic numbers without affecting stream state
	       for non-destructive format detection.

*/

commc_magic_result_t* commc_magic_detect_stream(commc_magic_context_t* context, FILE* stream);

/*

         commc_magic_result_destroy()
	       ---
	       releases memory associated with detection
	       result structure including string data and
	       metadata. prevents memory leaks after
	       format detection operations.

*/

void commc_magic_result_destroy(commc_magic_result_t* result);

/* 
	==================================
         --- UTILITY API ---
	==================================
*/

/*

         commc_magic_get_mime_type()
	       ---
	       convenience function to get MIME type for
	       file without full result structure. returns
	       allocated string with detected MIME type
	       or NULL if format unknown.

*/

char* commc_magic_get_mime_type(commc_magic_context_t* context, const char* filename);

/*

         commc_magic_get_extension()
	       ---
	       convenience function to get primary file
	       extension for detected format. returns
	       allocated string with extension or NULL
	       if format unknown.

*/

char* commc_magic_get_extension(commc_magic_context_t* context, const char* filename);

/*

         commc_magic_is_text_file()
	       ---
	       determines if file contains text data based
	       on content analysis and magic number detection.
	       returns 1 for text files, 0 for binary files,
	       -1 for detection errors.

*/

int commc_magic_is_text_file(commc_magic_context_t* context, const char* filename);

/*

         commc_magic_is_binary_file()
	       ---
	       determines if file contains binary data based
	       on content analysis and format detection.
	       returns 1 for binary files, 0 for text files,
	       -1 for detection errors.

*/

int commc_magic_is_binary_file(commc_magic_context_t* context, const char* filename);

/*

         commc_magic_get_charset()
	       ---
	       analyzes file content to determine character
	       encoding for text files. returns allocated
	       string with charset name (UTF-8, ASCII, etc.)
	       or NULL if undetermined.

*/

char* commc_magic_get_charset(commc_magic_context_t* context, const char* filename);

/*

         commc_magic_validate_format()
	       ---
	       validates file format consistency by checking
	       magic numbers against file extension and
	       content analysis. returns confidence level
	       of format validation (0-100).

*/

int commc_magic_validate_format(commc_magic_context_t* context, const char* filename, const char* expected_mime_type);

/* 
	==================================
      --- BUILT-IN FORMATS ---
	==================================
*/

/*

         commc_magic_database_add_standard_signatures()
	       ---
	       adds built-in signatures for common file
	       formats including images, documents, archives,
	       executables, and media files to database
	       for immediate format detection capability.

*/

int commc_magic_database_add_standard_signatures(commc_magic_database_t* database);

/*

         commc_magic_database_add_image_signatures()
	       ---
	       adds image format signatures (JPEG, PNG, GIF,
	       BMP, TIFF, WebP, etc.) to database for
	       comprehensive image file format detection
	       and validation.

*/

int commc_magic_database_add_image_signatures(commc_magic_database_t* database);

/*

         commc_magic_database_add_document_signatures()
	       ---
	       adds document format signatures (PDF, DOC,
	       RTF, PostScript, etc.) to database for
	       office document and text format detection
	       and identification.

*/

int commc_magic_database_add_document_signatures(commc_magic_database_t* database);

/*

         commc_magic_database_add_archive_signatures()
	       ---
	       adds archive format signatures (ZIP, RAR,
	       7Z, TAR, GZIP, etc.) to database for
	       compressed file and archive format
	       detection and validation.

*/

int commc_magic_database_add_archive_signatures(commc_magic_database_t* database);

/*

         commc_magic_database_add_executable_signatures()
	       ---
	       adds executable format signatures (PE, ELF,
	       Mach-O, etc.) to database for binary
	       executable and library format detection
	       across different platforms.

*/

int commc_magic_database_add_executable_signatures(commc_magic_database_t* database);

/*

         commc_magic_database_add_media_signatures()
	       ---
	       adds media format signatures (MP3, MP4, AVI,
	       WAV, FLAC, etc.) to database for audio
	       and video file format detection and
	       media type identification.

*/

int commc_magic_database_add_media_signatures(commc_magic_database_t* database);

/* 
	==================================
           --- EOF ---
	==================================
*/

#endif /* COMMC_MAGIC_H */