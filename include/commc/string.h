/*
   ===================================
   C O M M O N - C
   STRING UTILITIES MODULE
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- STRING MODULE ---

    this module provides safe and extended string
    manipulation functions. it includes safer alternatives
    to standard library functions to prevent common issues
    like buffer overflows, as well as new functionality
    not found in C89.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#ifndef   COMMC_STRING_H
#define   COMMC_STRING_H

#include  <stddef.h> /* for size_t */

/*
	==================================
          --- FUNCTIONS ---
	==================================
*/

/*

         commc_string_copy()
	       ---
	       safely copies a string from src to dst,
	       ensuring null termination and preventing
	       buffer overflows. returns the number of
	       bytes copied.

*/

size_t commc_string_copy(char* dst, const char* src, size_t dst_size);

/*

         commc_string_concat()
	       ---
	       safely concatenates src onto the end of dst.
	       it respects the total size of the destination
	       buffer to prevent overflows.

*/

size_t commc_string_concat(char* dst, const char* src, size_t dst_size);

/*

         commc_string_length()
	       ---
	       calculates the length of a string, not including
	       the null terminator. returns 0 for a null pointer.

*/

size_t commc_string_length(const char* str);

/*

         commc_string_split()
	       ---
	       splits a string into an array of substrings
	       based on a delimiter. the caller is responsible
	       for freeing the returned array and its contents.
	       the last element of the returned array is NULL.

*/

char** commc_string_split(const char* str, const char* delimiter);

/*

         commc_string_trim()
	       ---
	       removes leading and trailing whitespace from a
	       string. returns a new, allocated string that
	       the caller must free.

*/

char* commc_string_trim(const char* str);

/*

         commc_string_starts_with()
	       ---
	       checks if a string starts with a given prefix.
	       returns 1 if true, 0 if false.

*/

int commc_string_starts_with(const char* str, const char* prefix);

/*

         commc_string_ends_with()
	       ---
	       checks if a string ends with a given suffix.
	       returns 1 if true, 0 if false.

*/

int commc_string_ends_with(const char* str, const char* suffix);

#endif /* COMMC_STRING_H */

/*
	==================================
             --- EOF ---
	==================================
*/
