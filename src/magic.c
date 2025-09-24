/*

   ===================================
   C O M M C   -   M A G I C
   MAGIC NUMBER FILE FORMAT DETECTION
   ELASTIC SOFTWORKS 2025
   ===================================

*/

#include "commc/magic.h"
#include <stdlib.h>
#include <string.h>

/* 
	==================================
          --- CONSTANTS ---
	==================================
*/

/* maximum bytes to read for format detection */

#define MAGIC_MAX_HEADER_SIZE  8192

/* default confidence levels */

#define MAGIC_CONFIDENCE_HIGH     95
#define MAGIC_CONFIDENCE_MEDIUM   75
#define MAGIC_CONFIDENCE_LOW      50

/* 
	==================================
         --- HELPERS ---
	==================================
*/

/*

         signature_matches()
	       ---
	       checks if signature pattern matches buffer
	       data at specified offset with optional mask
	       application. returns 1 for match, 0 for
	       no match with proper bounds checking.

*/

static int signature_matches(const unsigned char* buffer, int buffer_len, const commc_magic_signature_t* signature) {

    int i;

    /* check bounds */

    if (signature->offset + signature->pattern_len > buffer_len) {

        return 0;

    }

    /* compare pattern with optional mask */

    for (i = 0; i < signature->pattern_len; i++) {

        unsigned char buffer_byte = buffer[signature->offset + i];
        unsigned char pattern_byte = signature->pattern[i];

        if (signature->mask) {

            buffer_byte &= signature->mask[i];
            pattern_byte &= signature->mask[i];

        }

        if (buffer_byte != pattern_byte) {

            return 0;

        }
    }

    return 1;
}

/*

         create_signature()
	       ---
	       helper function to create magic signature
	       structure with proper memory allocation
	       and data copying for database storage
	       and pattern matching operations.

*/

static commc_magic_signature_t* create_signature(const unsigned char* pattern,
                                                 int pattern_len,
                                                 int offset,
                                                 const unsigned char* mask,
                                                 const char* mime_type,
                                                 const char* extension,
                                                 const char* description,
                                                 int confidence) {

    commc_magic_signature_t* signature;

    int i;

    signature = (commc_magic_signature_t*)malloc(sizeof(commc_magic_signature_t));

    if (!signature) {

        return NULL;

    }

    /* allocate and copy pattern */

    signature->pattern = (unsigned char*)malloc(pattern_len);

    if (!signature->pattern) {

        free(signature);
        return NULL;

    }

    for (i = 0; i < pattern_len; i++) {

        signature->pattern[i] = pattern[i];

    }

    signature->pattern_len = pattern_len;
    signature->offset = offset;

    /* allocate and copy mask if provided */

    if (mask) {

        signature->mask = (unsigned char*)malloc(pattern_len);

        if (signature->mask) {

            for (i = 0; i < pattern_len; i++) {

                signature->mask[i] = mask[i];

            }
        }

    } else {

        signature->mask = NULL;

    }

    /* copy string fields */

    signature->mime_type = mime_type ? strdup(mime_type) : NULL;
    signature->extension = extension ? strdup(extension) : NULL;
    signature->description = description ? strdup(description) : NULL;
    signature->confidence = confidence;

    return signature;

}

/*

         destroy_signature()
	       ---
	       releases all memory associated with magic
	       signature structure including pattern data,
	       mask, and string fields to prevent memory
	       leaks during signature cleanup.

*/

static void destroy_signature(commc_magic_signature_t* signature) {

    if (!signature) {

        return;

    }

    if (signature->pattern) {

        free(signature->pattern);

    }

    if (signature->mask) {

        free(signature->mask);

    }

    if (signature->mime_type) {

        free((void*)signature->mime_type);

    }

    if (signature->extension) {

        free((void*)signature->extension);

    }

    if (signature->description) {

        free((void*)signature->description);

    }

    free(signature);
}

/* 
	==================================
         --- DATABASE API ---
	==================================
*/

/*

         commc_magic_database_create()
	       ---
	       creates new magic number database with
	       initial capacity for signature storage.
	       initializes empty database ready for
	       signature addition and format detection.

*/

commc_magic_database_t* commc_magic_database_create(void) {

    commc_magic_database_t* database;

    database = (commc_magic_database_t*)malloc(sizeof(commc_magic_database_t));

    if (!database) {

        return NULL;

    }

    database->signatures = NULL;
    database->count = 0;
    database->capacity = 0;
    database->sorted = 1;  /* empty database is sorted */

    return database;

}

/*

         commc_magic_database_destroy()
	       ---
	       releases all memory associated with magic
	       database including signature array and
	       individual signatures to prevent memory
	       leaks during database cleanup.

*/

void commc_magic_database_destroy(commc_magic_database_t* database) {

    int i;

    if (!database) {

        return;

    }

    /* destroy all signatures */

    if (database->signatures) {

        for (i = 0; i < database->count; i++) {

            destroy_signature(database->signatures[i]);

        }

        free(database->signatures);

    }

    free(database);
}

/*

         commc_magic_database_add_signature()
	       ---
	       adds new magic signature to database with
	       automatic capacity management and signature
	       creation. maintains database for efficient
	       format detection operations.

*/

int commc_magic_database_add_signature(commc_magic_database_t* database,
                                       const unsigned char* pattern,
                                       int pattern_len,
                                       int offset,
                                       const unsigned char* mask,
                                       const char* mime_type,
                                       const char* extension,
                                       const char* description,
                                       int confidence) {

    commc_magic_signature_t* signature;
    commc_magic_signature_t** new_signatures;

    if (!database || !pattern || pattern_len <= 0) {

        return -1;

    }

    /* create signature */

    signature = create_signature(pattern, pattern_len, offset, mask,
                                mime_type, extension, description, confidence);

    if (!signature) {

        return -1;

    }

    /* expand array if needed */

    if (database->count >= database->capacity) {

        int new_capacity = database->capacity ? database->capacity * 2 : 16;

        new_signatures = (commc_magic_signature_t**)realloc(database->signatures,
                                                           new_capacity * sizeof(commc_magic_signature_t*));

        if (!new_signatures) {

            destroy_signature(signature);
            return -1;

        }

        database->signatures = new_signatures;
        database->capacity = new_capacity;
    }

    /* add signature */

    database->signatures[database->count++] = signature;
    database->sorted = 0;  /* database no longer sorted */

    return 0;
}

/*

         commc_magic_database_remove_signature()
	       ---
	       removes signature from database by MIME type
	       with memory cleanup and array compaction.
	       maintains database integrity after signature
	       removal operations.

*/

int commc_magic_database_remove_signature(commc_magic_database_t* database, const char* mime_type) {

    int i;

    if (!database || !mime_type) {

        return -1;

    }

    for (i = 0; i < database->count; i++) {

        if (database->signatures[i]->mime_type &&
            strcmp(database->signatures[i]->mime_type, mime_type) == 0) {

            /* destroy signature */

            destroy_signature(database->signatures[i]);

            /* compact array */

            for (; i < database->count - 1; i++) {

                database->signatures[i] = database->signatures[i + 1];

            }

            database->count--;
            return 0;
        }
    }

    return -1;  /* signature not found */
}

/*

         commc_magic_database_get_signature_count()
	       ---
	       returns number of signatures currently
	       stored in database for iteration and
	       statistics during database management
	       and analysis operations.

*/

int commc_magic_database_get_signature_count(const commc_magic_database_t* database) {

    if (!database) {

        return 0;

    }

    return database->count;
}

/* 
	==================================
         --- DETECTION API ---
	==================================
*/

/*

         commc_magic_context_create()
	       ---
	       creates file format detection context with
	       database reference and default analysis
	       configuration for efficient format
	       identification operations.

*/

commc_magic_context_t* commc_magic_context_create(commc_magic_database_t* database) {

    commc_magic_context_t* context;

    if (!database) {

        return NULL;

    }

    context = (commc_magic_context_t*)malloc(sizeof(commc_magic_context_t));

    if (!context) {

        return NULL;

    }

    context->database = database;
    context->analyze_depth = MAGIC_MAX_HEADER_SIZE;
    context->require_exact = 0;
    context->use_filename = 1;

    memset(&context->error, 0, sizeof(commc_error_context_t));

    return context;
}

/*

         commc_magic_context_destroy()
	       ---
	       releases memory associated with detection
	       context including configuration and cached
	       data to prevent memory leaks during
	       context cleanup operations.

*/

void commc_magic_context_destroy(commc_magic_context_t* context) {

    if (!context) {

        return;

    }

    free(context);
}

/*

         commc_magic_detect_buffer()
	       ---
	       analyzes memory buffer to detect file format
	       using magic number signatures. returns best
	       match result with confidence level and
	       format metadata for identification.

*/

commc_magic_result_t* commc_magic_detect_buffer(commc_magic_context_t* context, const unsigned char* buffer, int buffer_len) {

    commc_magic_result_t* result;
    commc_magic_signature_t* best_match;

    int i;
    int best_confidence;

    if (!context || !buffer || buffer_len <= 0) {

        return NULL;

    }

    best_match = NULL;
    best_confidence = 0;

    /* search for matching signatures */

    for (i = 0; i < context->database->count; i++) {

        commc_magic_signature_t* signature = context->database->signatures[i];

        if (signature_matches(buffer, buffer_len, signature)) {

            if (signature->confidence > best_confidence) {

                best_match = signature;
                best_confidence = signature->confidence;

            }
        }
    }

    if (!best_match) {

        return NULL;

    }

    /* create result */

    result = (commc_magic_result_t*)malloc(sizeof(commc_magic_result_t));

    if (!result) {

        return NULL;

    }

    result->mime_type = best_match->mime_type ? strdup(best_match->mime_type) : NULL;
    result->extension = best_match->extension ? strdup(best_match->extension) : NULL;
    result->description = best_match->description ? strdup(best_match->description) : NULL;
    result->confidence = best_match->confidence;
    result->match_offset = best_match->offset;
    result->match_length = best_match->pattern_len;

    return result;

}

/*

         commc_magic_detect_file()
	       ---
	       detects file format by reading and analyzing
	       magic numbers from specified file. returns
	       format detection result with confidence
	       level and metadata for identification.

*/

commc_magic_result_t* commc_magic_detect_file(commc_magic_context_t* context, const char* filename) {

    FILE* file;
    unsigned char* buffer;
    commc_magic_result_t* result;

    int bytes_read;
    int read_size;

    if (!context || !filename) {

        return NULL;

    }

    file = fopen(filename, "rb");

    if (!file) {

        context->error.error_code = COMMC_IO_ERROR;
        strcpy(context->error.custom_message, "Cannot open file for reading");
        return NULL;

    }

    /* determine read size */

    read_size = context->analyze_depth > 0 ? context->analyze_depth : MAGIC_MAX_HEADER_SIZE;

    buffer = (unsigned char*)malloc(read_size);

    if (!buffer) {

        fclose(file);
        context->error.error_code = COMMC_MEMORY_ERROR;
        strcpy(context->error.custom_message, "Cannot allocate buffer for file analysis");
        return NULL;

    }

    bytes_read = fread(buffer, 1, read_size, file);
    fclose(file);

    if (bytes_read <= 0) {

        free(buffer);
        context->error.error_code = COMMC_IO_ERROR;
        strcpy(context->error.custom_message, "Cannot read file data");
        return NULL;

    }

    result = commc_magic_detect_buffer(context, buffer, bytes_read);

    free(buffer);

    return result;
}

/*

         commc_magic_detect_stream()
	       ---
	       detects file format from open file stream
	       with automatic position restoration. performs
	       non-destructive format analysis without
	       affecting stream state.

*/

commc_magic_result_t* commc_magic_detect_stream(commc_magic_context_t* context, FILE* stream) {

    unsigned char* buffer;
    commc_magic_result_t* result;
    long original_pos;
    int bytes_read;
    int read_size;

    if (!context || !stream) {

        return NULL;

    }

    /* save current position */

    original_pos = ftell(stream);

    if (original_pos < 0) {

        context->error.error_code = COMMC_IO_ERROR;
        strcpy(context->error.custom_message, "Cannot determine stream position");
        return NULL;

    }

    /* seek to beginning */

    if (fseek(stream, 0, SEEK_SET) != 0) {

        context->error.error_code = COMMC_IO_ERROR;
        strcpy(context->error.custom_message, "Cannot seek to stream beginning");
        return NULL;

    }

    /* determine read size */

    read_size = context->analyze_depth > 0 ? context->analyze_depth : MAGIC_MAX_HEADER_SIZE;

    buffer = (unsigned char*)malloc(read_size);

    if (!buffer) {

        fseek(stream, original_pos, SEEK_SET);
        context->error.error_code = COMMC_MEMORY_ERROR;
        strcpy(context->error.custom_message, "Cannot allocate buffer for stream analysis");
        return NULL;

    }

    bytes_read = fread(buffer, 1, read_size, stream);

    /* restore original position */

    fseek(stream, original_pos, SEEK_SET);

    if (bytes_read <= 0) {

        free(buffer);
        context->error.error_code = COMMC_IO_ERROR;
        strcpy(context->error.custom_message, "Cannot read stream data");
        return NULL;

    }

    result = commc_magic_detect_buffer(context, buffer, bytes_read);

    free(buffer);

    return result;
}

/*

         commc_magic_result_destroy()
	       ---
	       releases all memory associated with detection
	       result structure including string fields and
	       metadata to prevent memory leaks after
	       format detection operations.

*/

void commc_magic_result_destroy(commc_magic_result_t* result) {

    if (!result) {

        return;

    }

    if (result->mime_type) {

        free((void*)result->mime_type);

    }

    if (result->extension) {

        free((void*)result->extension);

    }

    if (result->description) {

        free((void*)result->description);

    }

    free(result);

}

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

char* commc_magic_get_mime_type(commc_magic_context_t* context, const char* filename) {

    commc_magic_result_t* result;
    char* mime_type = NULL;

    result = commc_magic_detect_file(context, filename);

    if (result && result->mime_type) {

        mime_type = strdup(result->mime_type);

    }

    commc_magic_result_destroy(result);

    return mime_type;
}

/*

         commc_magic_get_extension()
	       ---
	       convenience function to get primary file
	       extension for detected format. returns
	       allocated string with extension or NULL
	       if format unknown.

*/

char* commc_magic_get_extension(commc_magic_context_t* context, const char* filename) {

    commc_magic_result_t* result;
    char* extension = NULL;

    result = commc_magic_detect_file(context, filename);

    if (result && result->extension) {

        extension = strdup(result->extension);

    }

    commc_magic_result_destroy(result);

    return extension;
}

/*

         commc_magic_is_text_file()
	       ---
	       determines if file contains text data based
	       on magic number detection and content
	       analysis. returns 1 for text files,
	       0 for binary files.

*/

int commc_magic_is_text_file(commc_magic_context_t* context, const char* filename) {

    commc_magic_result_t* result;
    int is_text = 0;

    result = commc_magic_detect_file(context, filename);

    if (result && result->mime_type) {

        /* check if MIME type indicates text */

        if (strncmp(result->mime_type, "text/", 5) == 0 ||
            strcmp(result->mime_type, "application/json") == 0 ||
            strcmp(result->mime_type, "application/xml") == 0 ||
            strcmp(result->mime_type, "application/javascript") == 0) {

            is_text = 1;

        }

    } else {

        /* no magic match - analyze content directly */

        FILE* file = fopen(filename, "rb");

        if (file) {

            unsigned char buffer[1024];
            int bytes_read = fread(buffer, 1, sizeof(buffer), file);
            int i;

            is_text = 1;  /* assume text unless proven otherwise */

            for (i = 0; i < bytes_read && is_text; i++) {

                /* check for non-printable characters (except whitespace) */

                if (buffer[i] < 32 && buffer[i] != '\t' && buffer[i] != '\n' && buffer[i] != '\r') {

                    if (buffer[i] != 0) {  /* allow null bytes in some text formats */

                        is_text = 0;

                    }
                }
            }

            fclose(file);
        }
    }

    commc_magic_result_destroy(result);

    return is_text;
}

/*

         commc_magic_is_binary_file()
	       ---
	       determines if file contains binary data based
	       on magic number detection and content analysis.
	       returns 1 for binary files, 0 for text files,
	       -1 for detection errors.

*/

int commc_magic_is_binary_file(commc_magic_context_t* context, const char* filename) {

    int is_text = commc_magic_is_text_file(context, filename);

    if (is_text < 0) {

        return -1;  /* error */

    }

    return !is_text;
}

/* 
	==================================
      --- BUILT-IN FORMATS ---
	==================================
*/

/*

         commc_magic_database_add_standard_signatures()
	       ---
	       adds comprehensive set of built-in signatures
	       for common file formats including images,
	       documents, archives, and executables for
	       immediate format detection capability.

*/

int commc_magic_database_add_standard_signatures(commc_magic_database_t* database) {

    if (!database) {

        return -1;

    }

    /* add all standard signature categories */

    commc_magic_database_add_image_signatures(database);
    commc_magic_database_add_document_signatures(database);
    commc_magic_database_add_archive_signatures(database);
    commc_magic_database_add_executable_signatures(database);
    commc_magic_database_add_media_signatures(database);

    return 0;

}

/*

         commc_magic_database_add_image_signatures()
	       ---
	       adds magic signatures for common image formats
	       with proper magic numbers and MIME types for
	       comprehensive image format detection and
	       validation capabilities.

*/

int commc_magic_database_add_image_signatures(commc_magic_database_t* database) {

    if (!database) {

        return -1;

    }

    /* JPEG */

    {

        unsigned char pattern[] = {0xFF, 0xD8, 0xFF};
        commc_magic_database_add_signature(database, pattern, 3, 0, NULL,
                                          "image/jpeg", "jpg", "JPEG image", MAGIC_CONFIDENCE_HIGH);

    }

    /* PNG */
    {

        unsigned char pattern[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
        commc_magic_database_add_signature(database, pattern, 8, 0, NULL,
                                          "image/png", "png", "PNG image", MAGIC_CONFIDENCE_HIGH);

    }

    /* GIF87a */

    {

        unsigned char pattern[] = {0x47, 0x49, 0x46, 0x38, 0x37, 0x61};
        commc_magic_database_add_signature(database, pattern, 6, 0, NULL,
                                          "image/gif", "gif", "GIF image (87a)", MAGIC_CONFIDENCE_HIGH);

    }

    /* GIF89a */

    {

        unsigned char pattern[] = {0x47, 0x49, 0x46, 0x38, 0x39, 0x61};
        commc_magic_database_add_signature(database, pattern, 6, 0, NULL,
                                          "image/gif", "gif", "GIF image (89a)", MAGIC_CONFIDENCE_HIGH);

    }

    /* BMP */

    {

        unsigned char pattern[] = {0x42, 0x4D};
        commc_magic_database_add_signature(database, pattern, 2, 0, NULL,
                                          "image/bmp", "bmp", "BMP image", MAGIC_CONFIDENCE_MEDIUM);

    }

    /* TIFF (little endian) */

    {

        unsigned char pattern[] = {0x49, 0x49, 0x2A, 0x00};
        commc_magic_database_add_signature(database, pattern, 4, 0, NULL,
                                          "image/tiff", "tiff", "TIFF image (little endian)", MAGIC_CONFIDENCE_HIGH);

    }

    /* TIFF (big endian) */

    {

        unsigned char pattern[] = {0x4D, 0x4D, 0x00, 0x2A};
        commc_magic_database_add_signature(database, pattern, 4, 0, NULL,
                                          "image/tiff", "tiff", "TIFF image (big endian)", MAGIC_CONFIDENCE_HIGH);

    }

    /* WebP */

    {

        unsigned char pattern[] = {0x52, 0x49, 0x46, 0x46};  /* RIFF */
        commc_magic_database_add_signature(database, pattern, 4, 0, NULL,
                                          "image/webp", "webp", "WebP image", MAGIC_CONFIDENCE_MEDIUM);

    }

    return 0;

}

/*

         commc_magic_database_add_document_signatures()
	       ---
	       adds magic signatures for document formats
	       including PDF, Microsoft Office, and text
	       formats for comprehensive document format
	       detection and identification.

*/

int commc_magic_database_add_document_signatures(commc_magic_database_t* database) {

    if (!database) {

        return -1;

    }

    /* PDF */

    {

        unsigned char pattern[] = {0x25, 0x50, 0x44, 0x46};  /* %PDF */
        commc_magic_database_add_signature(database, pattern, 4, 0, NULL,
                                          "application/pdf", "pdf", "PDF document", MAGIC_CONFIDENCE_HIGH);

    }

    /* PostScript */

    {

        unsigned char pattern[] = {0x25, 0x21, 0x50, 0x53};  /* %!PS */
        commc_magic_database_add_signature(database, pattern, 4, 0, NULL,
                                          "application/postscript", "ps", "PostScript document", MAGIC_CONFIDENCE_HIGH);

    }

    /* RTF */

    {

        unsigned char pattern[] = {0x7B, 0x5C, 0x72, 0x74, 0x66, 0x31};  /* {\rtf1 */
        commc_magic_database_add_signature(database, pattern, 6, 0, NULL,
                                          "application/rtf", "rtf", "Rich Text Format", MAGIC_CONFIDENCE_HIGH);

    }

    return 0;

}

/*

         commc_magic_database_add_archive_signatures()
	       ---
	       adds magic signatures for archive and
	       compression formats including ZIP, RAR,
	       and TAR for comprehensive archive format
	       detection and validation.

*/

int commc_magic_database_add_archive_signatures(commc_magic_database_t* database) {

    if (!database) {

        return -1;

    }

    /* ZIP */

    {

        unsigned char pattern[] = {0x50, 0x4B, 0x03, 0x04};  /* PK.. */
        commc_magic_database_add_signature(database, pattern, 4, 0, NULL,
                                          "application/zip", "zip", "ZIP archive", MAGIC_CONFIDENCE_HIGH);

    }

    /* RAR */

    {

        unsigned char pattern[] = {0x52, 0x61, 0x72, 0x21, 0x1A, 0x07, 0x00};  /* Rar! */
        commc_magic_database_add_signature(database, pattern, 7, 0, NULL,
                                          "application/x-rar-compressed", "rar", "RAR archive", MAGIC_CONFIDENCE_HIGH);

    }

    /* GZIP */

    {

        unsigned char pattern[] = {0x1F, 0x8B, 0x08};
        commc_magic_database_add_signature(database, pattern, 3, 0, NULL,
                                          "application/gzip", "gz", "GZIP compressed file", MAGIC_CONFIDENCE_HIGH);

    }

    /* 7-Zip */

    {

        unsigned char pattern[] = {0x37, 0x7A, 0xBC, 0xAF, 0x27, 0x1C};  /* 7z */
        commc_magic_database_add_signature(database, pattern, 6, 0, NULL,
                                          "application/x-7z-compressed", "7z", "7-Zip archive", MAGIC_CONFIDENCE_HIGH);
     }

    return 0;

}

/*

         commc_magic_database_add_executable_signatures()
	       ---
	       adds magic signatures for executable formats
	       including PE, ELF, and Mach-O for binary
	       executable detection across different
	       operating systems and architectures.

*/

int commc_magic_database_add_executable_signatures(commc_magic_database_t* database) {

    if (!database) {

        return -1;

    }

    /* PE (Portable Executable) - Windows */

    {
        unsigned char pattern[] = {0x4D, 0x5A};  /* MZ */
        commc_magic_database_add_signature(database, pattern, 2, 0, NULL,
                                          "application/x-msdownload", "exe", "Windows executable", MAGIC_CONFIDENCE_MEDIUM);
    }

    /* ELF - Unix/Linux */
    {
        unsigned char pattern[] = {0x7F, 0x45, 0x4C, 0x46};  /* .ELF */
        commc_magic_database_add_signature(database, pattern, 4, 0, NULL,
                                          "application/x-executable", "elf", "ELF executable", MAGIC_CONFIDENCE_HIGH);

    }

    /* Mach-O (32-bit) - macOS */

    {

        unsigned char pattern[] = {0xFE, 0xED, 0xFA, 0xCE};
        commc_magic_database_add_signature(database, pattern, 4, 0, NULL,
                                          "application/x-mach-binary", "macho", "Mach-O executable (32-bit)", MAGIC_CONFIDENCE_HIGH);

    }

    /* Mach-O (64-bit) - macOS */

    {

        unsigned char pattern[] = {0xFE, 0xED, 0xFA, 0xCF};
        commc_magic_database_add_signature(database, pattern, 4, 0, NULL,
                                          "application/x-mach-binary", "macho", "Mach-O executable (64-bit)", MAGIC_CONFIDENCE_HIGH);

    }

    return 0;

}

/*

         commc_magic_database_add_media_signatures()
	       ---
	       adds magic signatures for audio and video
	       formats including MP3, MP4, and WAV for
	       comprehensive media file format detection
	       and identification capabilities.

*/

int commc_magic_database_add_media_signatures(commc_magic_database_t* database) {

    if (!database) {

        return -1;

    }

    /* MP3 */

    {

        unsigned char pattern[] = {0xFF, 0xFB};  /* MPEG Layer 3 */
        commc_magic_database_add_signature(database, pattern, 2, 0, NULL,
                                          "audio/mpeg", "mp3", "MP3 audio", MAGIC_CONFIDENCE_MEDIUM);

    }

    /* FLAC */

    {

        unsigned char pattern[] = {0x66, 0x4C, 0x61, 0x43};  /* fLaC */
        commc_magic_database_add_signature(database, pattern, 4, 0, NULL,
                                          "audio/flac", "flac", "FLAC audio", MAGIC_CONFIDENCE_HIGH);

    }

    /* WAV */

    {

        unsigned char pattern[] = {0x52, 0x49, 0x46, 0x46};  /* RIFF */
        commc_magic_database_add_signature(database, pattern, 4, 0, NULL,
                                          "audio/wav", "wav", "WAV audio", MAGIC_CONFIDENCE_MEDIUM);

    }

    /* MP4 */

    {

        unsigned char pattern[] = {0x66, 0x74, 0x79, 0x70};  /* ftyp */
        commc_magic_database_add_signature(database, pattern, 4, 4, NULL,
                                          "video/mp4", "mp4", "MP4 video", MAGIC_CONFIDENCE_HIGH);

    }

    /* AVI */

    {

        unsigned char pattern[] = {0x52, 0x49, 0x46, 0x46};  /* RIFF */
        commc_magic_database_add_signature(database, pattern, 4, 0, NULL,
                                          "video/avi", "avi", "AVI video", MAGIC_CONFIDENCE_MEDIUM);

    }

    return 0;
    
}

/* 
	==================================
           --- EOF ---
	==================================
*/