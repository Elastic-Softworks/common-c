/*
   ===================================
   C O M M O N - C
   FILE I/O UTILITIES MODULE
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- FILE MODULE ---

    this module provides enhanced file input/output
    utilities, including binary file handling and
    basic directory operations, which are not
    standardized in C89.

    it aims to offer a more robust and cross-platform
    approach to file system interactions.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#ifndef COMMC_FILE_H
#define COMMC_FILE_H

#include <stddef.h>      /* for size_t */
#include <stdio.h>       /* for FILE */

/*
	==================================
             --- DEFINES ---
	==================================
*/

/* file open modes. */

typedef enum {

  COMMC_FILE_READ,        /* read-only */
  COMMC_FILE_WRITE,       /* write-only, creates/truncates */
  COMMC_FILE_APPEND,      /* append-only, creates if not exists */
  COMMC_FILE_READ_WRITE,  /* read and write, creates/truncates */
  COMMC_FILE_READ_APPEND  /* read and append, creates if not exists */

} commc_file_mode_t;

/*
	==================================
             --- FUNCTIONS ---
	==================================
*/

/*

         commc_file_open()
	       ---
	       opens a file with the specified path and mode.
	       returns a FILE pointer on success, NULL on failure.

*/

FILE* commc_file_open(const char* path, commc_file_mode_t mode);

/*

         commc_file_close()
	       ---
	       closes an open file.

*/

void commc_file_close(FILE* file);

/*

         commc_file_read_all_text()
	       ---
	       reads the entire content of a text file into a
	       dynamically allocated string. caller must free.

*/

char* commc_file_read_all_text(const char* path);

/*

         commc_file_write_all_text()
	       ---
	       writes a string to a file, overwriting existing
	       content. returns 1 on success, 0 on failure.

*/

int commc_file_write_all_text(const char* path, const char* content);

/*

         commc_file_read_all_bytes()
	       ---
	       reads the entire content of a binary file into a
	       dynamically allocated buffer. caller must free.
	       stores the size in 'out_size'.

*/

void* commc_file_read_all_bytes(const char* path, size_t* out_size);

/*

         commc_file_write_all_bytes()
	       ---
	       writes a buffer of bytes to a file, overwriting
	       existing content. returns 1 on success, 0 on failure.

*/

int commc_file_write_all_bytes(const char* path, const void* buffer, size_t size);

/*

         commc_file_exists()
	       ---
	       checks if a file exists at the given path.
	       returns 1 if exists, 0 otherwise.

*/

int commc_file_exists(const char* path);

/*

         commc_file_delete()
	       ---
	       deletes a file at the given path.
	       returns 1 on success, 0 on failure.

*/

int commc_file_delete(const char* path);

/*

         commc_dir_create()
	       ---
	       creates a directory at the given path.
	       returns 1 on success, 0 on failure.

*/

int commc_dir_create(const char* path);

/*

         commc_dir_delete()
	       ---
	       deletes an empty directory at the given path.
	       returns 1 on success, 0 on failure.

*/

int commc_dir_delete(const char* path);

#endif /* COMMC_FILE_H */

/*
	==================================
             --- EOF ---
	==================================
*/
