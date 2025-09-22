/* 	
   ===================================
   C O M M C / R O P E . H
   ROPE DATA STRUCTURE API
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

	                --- ROPE API ---

	    ropes are binary trees of string fragments that provide
	    efficient concatenation, splitting, and editing operations
	    on large strings. they avoid the O(n) cost of string
	    concatenation by storing strings as trees of smaller pieces.
	    
	    this implementation is optimized for text editors and
	    applications that need frequent string manipulation with
	    minimal copying overhead.

*/

#ifndef COMMC_ROPE_H
#define COMMC_ROPE_H

/* 
	==================================
             --- INCLUDES ---
	==================================
*/

#include "commc/error.h"      /* ERROR HANDLING */
#include <stddef.h>           /* SIZE_T */

#ifdef __cplusplus
extern "C" {
#endif

/* 
	==================================
             --- TYPES ---
	==================================
*/

/*

         commc_rope_node_t
	       ---
	       represents a single node in the rope tree.
	       can be either a leaf (containing actual string data)
	       or internal node (pointing to left and right subtrees).

*/

typedef struct commc_rope_node {

  char*                    data;      /* string data (leaf nodes only) */
  size_t                   length;    /* length of string/subtree */
  size_t                   weight;    /* weight (length of left subtree) */
  struct commc_rope_node*  left;      /* left child */
  struct commc_rope_node*  right;     /* right child */
  
} commc_rope_node_t;

/*

         commc_rope_t
	       ---
	       represents the rope data structure with root node
	       and optional balancing parameters.

*/

typedef struct {

  commc_rope_node_t*  root;            /* root of rope tree */
  size_t              total_length;    /* total string length */
  size_t              leaf_threshold;  /* max leaf size before split */
  
} commc_rope_t;

/*

         commc_rope_iterator_t
	       ---
	       iterator for character-by-character traversal of rope.

*/

typedef struct {

  commc_rope_t*        rope;          /* rope being iterated */
  commc_rope_node_t*   current_node;  /* current leaf node */
  size_t               position;      /* position within current leaf */
  size_t               global_pos;    /* global position in rope */
  
} commc_rope_iterator_t;

/* 
	==================================
             --- CONSTANTS ---
	==================================
*/

/*

         COMMC_ROPE_DEFAULT_LEAF_THRESHOLD
	       ---
	       default maximum size for leaf nodes before splitting.

*/

#define COMMC_ROPE_DEFAULT_LEAF_THRESHOLD 1024

/*

         COMMC_ROPE_BALANCE_THRESHOLD
	       ---
	       threshold for rebalancing based on tree height.

*/

#define COMMC_ROPE_BALANCE_THRESHOLD 8

/* 
	==================================
             --- CORE API ---
	==================================
*/

/*

         commc_rope_create()
	       ---
	       creates a new empty rope.
	       
	       returns:
	       - pointer to new rope, or NULL on error

*/

commc_rope_t* commc_rope_create(void);

/*

         commc_rope_create_with_threshold()
	       ---
	       creates rope with custom leaf size threshold.
	       
	       parameters:
	       - leaf_threshold: maximum leaf node size
	       
	       returns:
	       - pointer to new rope, or NULL on error

*/

commc_rope_t* commc_rope_create_with_threshold(size_t leaf_threshold);

/*

         commc_rope_create_from_string()
	       ---
	       creates rope from existing string.
	       
	       parameters:
	       - str: null-terminated string to initialize with
	       
	       returns:
	       - pointer to new rope, or NULL on error

*/

commc_rope_t* commc_rope_create_from_string(const char* str);

/*

         commc_rope_destroy()
	       ---
	       destroys rope and frees all associated memory.

*/

void commc_rope_destroy(commc_rope_t* rope);

/*

         commc_rope_length()
	       ---
	       returns the total length of the rope string.

*/

size_t commc_rope_length(commc_rope_t* rope);

/*

         commc_rope_is_empty()
	       ---
	       returns 1 if rope is empty, 0 otherwise.

*/

int commc_rope_is_empty(commc_rope_t* rope);

/* 
	==================================
             --- ACCESS API ---
	==================================
*/

/*

         commc_rope_char_at()
	       ---
	       retrieves character at specified position.
	       
	       parameters:
	       - rope: rope to query
	       - index: character position (0-based)
	       
	       returns:
	       - character at position, or '\0' if index out of bounds

*/

char commc_rope_char_at(commc_rope_t* rope, size_t index);

/*

         commc_rope_substring()
	       ---
	       extracts substring from rope as new string.
	       caller must free returned string.
	       
	       parameters:
	       - rope: source rope
	       - start: starting position (inclusive)
	       - length: number of characters to extract
	       
	       returns:
	       - new null-terminated string, or NULL on error

*/

char* commc_rope_substring(commc_rope_t* rope, size_t start, size_t length);

/*

         commc_rope_to_string()
	       ---
	       converts entire rope to null-terminated string.
	       caller must free returned string.
	       
	       returns:
	       - new string containing full rope content, or NULL on error

*/

char* commc_rope_to_string(commc_rope_t* rope);

/* 
	==================================
             --- MODIFICATION API ---
	==================================
*/

/*

         commc_rope_concat()
	       ---
	       concatenates two ropes, creating a new rope.
	       original ropes are not modified.
	       
	       parameters:
	       - left: first rope
	       - right: second rope
	       
	       returns:
	       - new rope containing concatenated content, or NULL on error

*/

commc_rope_t* commc_rope_concat(commc_rope_t* left, commc_rope_t* right);

/*

         commc_rope_insert()
	       ---
	       inserts string at specified position.
	       modifies the rope in place.
	       
	       parameters:
	       - rope: rope to modify
	       - index: insertion position
	       - str: null-terminated string to insert
	       
	       returns:
	       - COMMC_SUCCESS on successful insertion
	       - COMMC_ARGUMENT_ERROR for invalid parameters
	       - COMMC_MEMORY_ERROR if memory allocation fails

*/

commc_error_t commc_rope_insert(commc_rope_t* rope, size_t index, const char* str);

/*

         commc_rope_delete()
	       ---
	       removes characters from specified range.
	       modifies the rope in place.
	       
	       parameters:
	       - rope: rope to modify
	       - start: starting position (inclusive)
	       - length: number of characters to remove
	       
	       returns:
	       - COMMC_SUCCESS on successful deletion
	       - COMMC_ARGUMENT_ERROR for invalid parameters

*/

commc_error_t commc_rope_delete(commc_rope_t* rope, size_t start, size_t length);

/*

         commc_rope_split()
	       ---
	       splits rope at specified position into two ropes.
	       
	       parameters:
	       - rope: rope to split
	       - index: split position
	       - left: output for left part (before index)
	       - right: output for right part (from index onwards)
	       
	       returns:
	       - COMMC_SUCCESS on successful split
	       - COMMC_ARGUMENT_ERROR for invalid parameters
	       - COMMC_MEMORY_ERROR if memory allocation fails

*/

commc_error_t commc_rope_split(commc_rope_t* rope, size_t index,
                               commc_rope_t** left, commc_rope_t** right);

/* 
	==================================
             --- SEARCH API ---
	==================================
*/

/*

         commc_rope_find()
	       ---
	       finds first occurrence of substring in rope.
	       
	       parameters:
	       - rope: rope to search
	       - needle: string to find
	       - start: starting search position
	       
	       returns:
	       - position of first match, or SIZE_MAX if not found

*/

size_t commc_rope_find(commc_rope_t* rope, const char* needle, size_t start);

/*

         commc_rope_find_char()
	       ---
	       finds first occurrence of character in rope.
	       
	       parameters:
	       - rope: rope to search
	       - ch: character to find
	       - start: starting search position
	       
	       returns:
	       - position of first match, or SIZE_MAX if not found

*/

size_t commc_rope_find_char(commc_rope_t* rope, char ch, size_t start);

/*

         commc_rope_count_lines()
	       ---
	       counts number of newline characters in rope.
	       useful for text editors.
	       
	       returns:
	       - number of line breaks (newline characters)

*/

size_t commc_rope_count_lines(commc_rope_t* rope);

/* 
	==================================
             --- ITERATION API ---
	==================================
*/

/*

         commc_rope_iterator_begin()
	       ---
	       creates iterator starting from beginning of rope.

*/

commc_rope_iterator_t commc_rope_iterator_begin(commc_rope_t* rope);

/*

         commc_rope_iterator_at()
	       ---
	       creates iterator starting at specified position.

*/

commc_rope_iterator_t commc_rope_iterator_at(commc_rope_t* rope, size_t position);

/*

         commc_rope_iterator_next()
	       ---
	       advances iterator to next character.

*/

commc_error_t commc_rope_iterator_next(commc_rope_iterator_t* iterator);

/*

         commc_rope_iterator_get_char()
	       ---
	       retrieves current character from iterator.

*/

char commc_rope_iterator_get_char(commc_rope_iterator_t* iterator);

/*

         commc_rope_iterator_get_position()
	       ---
	       retrieves current global position of iterator.

*/

size_t commc_rope_iterator_get_position(commc_rope_iterator_t* iterator);

/*

         commc_rope_iterator_has_next()
	       ---
	       returns 1 if iterator has more characters.

*/

int commc_rope_iterator_has_next(commc_rope_iterator_t* iterator);

/* 
	==================================
             --- UTILITY API ---
	==================================
*/

/*

         commc_rope_balance()
	       ---
	       rebalances rope tree for optimal performance.
	       should be called periodically after many modifications.
	       
	       returns:
	       - COMMC_SUCCESS if rebalancing completed
	       - COMMC_MEMORY_ERROR if rebalancing failed

*/

commc_error_t commc_rope_balance(commc_rope_t* rope);

/*

         commc_rope_is_balanced()
	       ---
	       checks if rope tree is reasonably balanced.
	       
	       returns:
	       - 1 if balanced, 0 if rebalancing recommended

*/

int commc_rope_is_balanced(commc_rope_t* rope);

/*

         commc_rope_depth()
	       ---
	       calculates maximum depth of rope tree.
	       useful for performance analysis.

*/

size_t commc_rope_depth(commc_rope_t* rope);

/*

         commc_rope_leaf_count()
	       ---
	       counts number of leaf nodes in rope tree.

*/

size_t commc_rope_leaf_count(commc_rope_t* rope);

/*

         commc_rope_memory_usage()
	       ---
	       estimates total memory usage in bytes.

*/

size_t commc_rope_memory_usage(commc_rope_t* rope);

/* 
	==================================
             --- COMPARISON API ---
	==================================
*/

/*

         commc_rope_compare()
	       ---
	       compares two ropes lexicographically.
	       
	       returns:
	       - < 0 if rope1 < rope2
	       - 0 if rope1 == rope2
	       - > 0 if rope1 > rope2

*/

int commc_rope_compare(commc_rope_t* rope1, commc_rope_t* rope2);

/*

         commc_rope_equals()
	       ---
	       tests if two ropes contain identical strings.

*/

int commc_rope_equals(commc_rope_t* rope1, commc_rope_t* rope2);

/*

         commc_rope_starts_with()
	       ---
	       tests if rope starts with given string.

*/

int commc_rope_starts_with(commc_rope_t* rope, const char* prefix);

/*

         commc_rope_ends_with()
	       ---
	       tests if rope ends with given string.

*/

int commc_rope_ends_with(commc_rope_t* rope, const char* suffix);

#ifdef __cplusplus
}
#endif

#endif /* COMMC_ROPE_H */

/*
	==================================
             --- EOF ---
	==================================
*/