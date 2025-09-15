/*
   ===================================
   C O M M O N - C
   FILE I/O IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- FILE MODULE ---

    implementation of enhanced file input/output
    and directory utilities.
    see include/commc/file.h for function
    prototypes and documentation.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#include <stdlib.h>      /* for malloc, free */
#include <string.h>      /* for memcpy, strlen */

#include "commc/file.h"
#include "commc/error.h" /* for error handling */

#ifdef _WIN32
#include <direct.h>      /* for _mkdir, _rmdir */
#include <io.h>          /* for _access */
#define F_OK 0           /* for _access compatibility */
#else
#include <sys/stat.h>    /* for mkdir */
#include <unistd.h>      /* for rmdir, access */
#endif

/*
	==================================
             --- STATIC FUNCS ---
	==================================
*/

/*

         get_file_mode_string()
	       ---
	       converts commc_file_mode_t to a standard C
	       file mode string (e.g., "rb", "w+b").

*/

static const char* get_file_mode_string(commc_file_mode_t mode) {

  switch  (mode) {

    case COMMC_FILE_READ:        return "rb";
    case COMMC_FILE_WRITE:       return "wb";
    case COMMC_FILE_APPEND:      return "ab";
    case COMMC_FILE_READ_WRITE:  return "r+b";
    case COMMC_FILE_READ_APPEND: return "a+b";
    default:                     return NULL;

  }

}

/*
	==================================
             --- FUNCTIONS ---
	==================================
*/

/*

         commc_file_open()
	       ---
	       opens a file with the specified path and mode.

*/

FILE* commc_file_open(const char* path, commc_file_mode_t mode) {

  const char* mode_str;

  if  (!path) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  mode_str = get_file_mode_string(mode);

  if  (!mode_str) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  return fopen(path, mode_str);

}

/*

         commc_file_close()
	       ---
	       closes an open file.

*/

void commc_file_close(FILE* file) {

  if  (file) {

    fclose(file);

  }

}

/*

         commc_file_read_all_text()
	       ---
	       reads the entire content of a text file into a
	       dynamically allocated string.

*/

char* commc_file_read_all_text(const char* path) {

  FILE*  file;
  long   file_size;
  char*  buffer;
  size_t read_count;

  if  (!path) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  file = fopen(path, "r"); /* open in text read mode */

  if  (!file) {

    commc_report_error(COMMC_IO_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  fseek(file, 0, SEEK_END);
  file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  if  (file_size == -1L) {

    commc_report_error(COMMC_IO_ERROR, __FILE__, __LINE__);
    fclose(file);
    return NULL;

  }

  buffer = (char*)malloc(file_size + 1); /* +1 for null terminator */

  if  (!buffer) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    fclose(file);
    return NULL;

  }

  read_count = fread(buffer, 1, file_size, file);

  if  (read_count != (size_t)file_size) {

    commc_report_error(COMMC_IO_ERROR, __FILE__, __LINE__);
    free(buffer);
    fclose(file);
    return NULL;

  }

  buffer[file_size] = '\0'; /* null-terminate the string */
  fclose(file);

  return buffer;

}

/*

         commc_file_write_all_text()
	       ---
	       writes a string to a file, overwriting existing
	       content.

*/

int commc_file_write_all_text(const char* path, const char* content) {

  FILE*  file;
  size_t content_len;
  size_t write_count;

  if  (!path || !content) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return 0;

  }

  /* open in text write mode */

  file = fopen(path, "w");

  if  (!file) {

    commc_report_error(COMMC_IO_ERROR, __FILE__, __LINE__);
    return 0;

  }

  content_len = strlen(content);
  write_count = fwrite(content, 1, content_len, file);

  fclose(file);

  if  (write_count != content_len) {

    commc_report_error(COMMC_IO_ERROR, __FILE__, __LINE__);
    return 0;

  }

  return 1;

}

/*

         commc_file_read_all_bytes()
	       ---
	       reads the entire content of a binary file into a
	       dynamically allocated buffer.

*/

void* commc_file_read_all_bytes(const char* path, size_t* out_size) {

  FILE*  file;
  long   file_size;
  void*  buffer;
  size_t read_count;

  if  (!path || !out_size) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  file = fopen(path, "rb"); /* open in binary read mode */

  if  (!file) {

    commc_report_error(COMMC_IO_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  fseek(file, 0, SEEK_END);
  file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  if  (file_size == -1L) {

    commc_report_error(COMMC_IO_ERROR, __FILE__, __LINE__);
    fclose(file);
    return NULL;

  }

  buffer = malloc(file_size);

  if  (!buffer) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    fclose(file);
    return NULL;

  }

  read_count = fread(buffer, 1, file_size, file);

  if  (read_count != (size_t)file_size) {

    commc_report_error(COMMC_IO_ERROR, __FILE__, __LINE__);
    free(buffer);
    fclose(file);
    return NULL;

  }

  *out_size = (size_t)file_size;
  fclose(file);

  return buffer;

}

/*

         commc_file_write_all_bytes()
	       ---
	       writes a buffer of bytes to a file, overwriting
	       existing content.

*/

int commc_file_write_all_bytes(const char* path, const void* buffer, size_t size) {

  FILE*  file;
  size_t write_count;

  if  (!path || !buffer) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return 0;

  }

  file = fopen(path, "wb");             /* open in binary write mode */

  if  (!file) {

    commc_report_error(COMMC_IO_ERROR, __FILE__, __LINE__);
    return 0;

  }

  write_count = fwrite(buffer, 1, size, file);

  fclose(file);

  if  (write_count != size) {

    commc_report_error(COMMC_IO_ERROR, __FILE__, __LINE__);
    return 0;

  }

  return 1;

}

/*

         commc_file_exists()
	       ---
	       checks if a file exists at the given path.

*/

int commc_file_exists(const char* path) {

  if  (!path) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return 0;

  }

#ifdef _WIN32
  return _access(path, F_OK) == 0;
#else
  return access(path, F_OK) == 0;
#endif

}

/*

         commc_file_delete()
	       ---
	       deletes a file at the given path.

*/

int commc_file_delete(const char* path) {

  if  (!path) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return 0;

  }

  return remove(path) == 0;

}

/*

         commc_dir_create()
	       ---
	       creates a directory at the given path.

*/

int commc_dir_create(const char* path) {

  if  (!path) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return 0;

  }

#ifdef _WIN32
  return _mkdir(path) == 0;
#else
  return mkdir(path, 0777) == 0;          /* 0777 for full permissions */
#endif

}

/*

         commc_dir_delete()
	       ---
	       deletes an empty directory at the given path.

*/

int commc_dir_delete(const char* path) {

  if  (!path) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return 0;

  }

#ifdef _WIN32
  return _rmdir(path) == 0;
#else
  return rmdir(path) == 0;
#endif

}

/*
	==================================
             --- EOF ---
	==================================
*/
