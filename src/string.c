/*
   ===================================
   C O M M O N - C
   STRING IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- STRING MODULE ---

    implementation of the string utilities.
    see include/commc/string.h for function
    prototypes and documentation.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#include "commc/string.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
	==================================
             --- FUNCS ---
	==================================
*/

/*

         commc_string_length()
	       ---
	       calculates string length. returns 0
	       if the pointer is null, providing safety
	       against crashes.

*/

size_t commc_string_length(const char* str) {

  const char* s;

  if  (!str) {

    return 0;

  }

  for  (s = str; *s; ++s) {

    /* loop does the work ;) */

  }

  return (s - str);

}

/*

         commc_string_copy()
	       ---
	       safely copies a string, ensuring null
	       termination. it prevents buffer overflows by
	       respecting the destination size.

*/

size_t commc_string_copy(char* dst, const char* src, size_t dst_size) {

  size_t i = 0;

  if  (!dst || !src || dst_size == 0) {

    return 0;

  }

  while  (i < dst_size - 1 && src[i] != '\0') {

    dst[i] = src[i];
    i++;

  }

  dst[i] = '\0'; /* ensure null termination */

  return i;

}

/*

         commc_string_concat()
	       ---
	       safely appends one string to another. it checks
	       the available space in the destination buffer
	       to avoid writing out of bounds.

*/

size_t commc_string_concat(char* dst, const char* src, size_t dst_size) {

  size_t dst_len;
  size_t i;

  if  (!dst || !src || dst_size == 0) {

    return 0;

  }

  dst_len = commc_string_length(dst);
  i       = 0;

  while  (dst_len + i < dst_size - 1 && src[i] != '\0') {

    dst[dst_len + i] = src[i];
    i++;

  }

  dst[dst_len + i] = '\0'; /* ensure null termination */

  return dst_len + i;

}

/*

         commc_string_starts_with()
	       ---
	       checks if a string begins with a specific
	       prefix. useful for parsing commands or file types.

*/

int commc_string_starts_with(const char* str, const char* prefix) {

  if  (!str || !prefix) {

    return 0;

  }

  while  (*prefix) {

    if  (*prefix++ != *str++) {

      return 0;

    }

  }

  return 1;

}

/*

         commc_string_ends_with()
	       ---
	       checks if a string ends with a specific suffix.

*/

int commc_string_ends_with(const char* str, const char* suffix) {

  size_t str_len;
  size_t suffix_len;

  if  (!str || !suffix) {

    return 0;

  }

  str_len    = commc_string_length(str);
  suffix_len = commc_string_length(suffix);

  if  (suffix_len > str_len) {

    return 0;

  }

  return strncmp(str + str_len - suffix_len, suffix, suffix_len) == 0;

}

/*

         commc_string_trim()
	       ---
	       allocates a new string with leading and trailing
	       whitespace removed. the caller must free the result.

*/

char* commc_string_trim(const char* str) {

  const char* end;
  size_t      len;
  char*       trimmed_str;

  if  (!str) {

    return NULL;

  }

  /* trim leading space */
  while  (isspace((unsigned char)*str)) {

    str++;

  }

  if  (*str == 0) { /* all spaces */

    trimmed_str = (char*)malloc(1);
    trimmed_str[0] = '\0';
    return trimmed_str;

  }

  /* trim trailing space */

  end = str + commc_string_length(str) - 1;

  while  (end > str && isspace((unsigned char)*end)) {

    end--;

  }

  len = end - str + 1;

  trimmed_str = (char*)malloc(len + 1);

  if  (!trimmed_str) {

    return NULL;

  }

  memcpy(trimmed_str, str, len);
  trimmed_str[len] = '\0';

  return trimmed_str;

}

/*

         commc_string_split()
	       ---
	       splits a string by a delimiter into a null-terminated
	       array of strings. this is a complex operation involving
	       multiple allocations.

*/

char** commc_string_split(const char* str, const char* delimiter) {

  char*  token;
  char*  str_copy;
  char*  temp_ptr;
  char** result;
  int    count;
  int    i;

  if  (!str) {

    return NULL;

  }

  /* count tokens first */

  str_copy = strdup(str);
  count    = 0;
  token    = strtok(str_copy, delimiter);

  while  (token != NULL) {

    count++;
    token = strtok(NULL, delimiter);

  }

  free(str_copy);

  /* allocate result array */

  result = (char**)malloc(sizeof(char*) * (count + 1));

  if  (!result) {

    return NULL;

  }

  /* populate result array */
  
  str_copy = strdup(str);
  i        = 0;
  token    = strtok_r(str_copy, delimiter, &temp_ptr);

  while  (token != NULL) {

    result[i] = strdup(token);
    i++;
    token = strtok_r(NULL, delimiter, &temp_ptr);

  }

  result[i] = NULL; /* null-terminate the array */
  free(str_copy);

  return result;

}

/*
	==================================
             --- EOF ---
	==================================
*/
