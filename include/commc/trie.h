/*

    ===================================
    T R I E   ( P R E F I X   T R E E )
    ELASTIC SOFTWORKS 2025
    ===================================

*/

/*

         commc_trie - prefix tree for string storage and retrieval
           ---
           this header defines a trie (prefix tree) data structure optimized
           for string operations including prefix search, auto-complete, and
           efficient string storage with shared prefixes.
           
           a trie is a tree-like data structure where each node represents
           a single character. strings are stored as paths from root to leaf,
           with each edge representing a character transition. this structure
           excels at prefix-based operations and provides excellent performance
           for dictionary-like applications.
           
           key benefits:
           - O(m) search time where m is string length (independent of n strings)
           - natural prefix matching capabilities  
           - space efficiency through shared prefixes
           - ideal for auto-complete and spell-check applications
           
           this implementation uses ASCII character support with 256-element
           arrays for maximum flexibility, trading some memory for simplicity
           and educational clarity.

*/

#ifndef COMMC_TRIE_H
#define COMMC_TRIE_H

/* 
	==================================
             --- INCLUDES ---
	==================================
*/

#include "error.h"
#include <stddef.h>
#include <stdbool.h>

/* 
	==================================
             --- CONSTANTS ---
	==================================
*/

#define COMMC_TRIE_ASCII_SIZE  256  /* FULL ASCII CHARACTER SET */
#define COMMC_TRIE_MAX_COMPLETIONS  1000  /* MAXIMUM AUTO-COMPLETE RESULTS */

/* 
	==================================
             --- TYPES ---
	==================================
*/

/*

         commc_trie_node_t - individual trie node structure
           ---
           represents a single node in the trie tree. each node contains
           an array of pointers to child nodes (one for each possible ASCII
           character) and a flag indicating whether this node marks the
           end of a stored string.
           
           the children array uses direct ASCII indexing - children[65]
           points to the child node for character 'A', children[97] for 'a', 
           etc. this provides O(1) character lookup at the cost of memory
           overhead for sparse character sets.
           
           the is_end_of_word flag distinguishes between intermediate nodes
           (part of longer strings) and terminal nodes (complete strings).
           this allows the trie to store both "cat" and "catch" correctly.

*/

typedef struct commc_trie_node_t {

  struct commc_trie_node_t*  children[COMMC_TRIE_ASCII_SIZE];  /* CHILD NODE POINTERS */
  
  bool                       is_end_of_word;  /* MARKS COMPLETE STRING */

} commc_trie_node_t;

/*

         commc_trie_t - main trie data structure  
           ---
           the primary trie container holding the root node and metadata
           about the stored strings. tracks the total number of unique
           strings stored for efficient size queries.
           
           the root node represents an empty string and serves as the
           starting point for all trie operations. all stored strings
           begin their path from this root node.

*/

typedef struct {

  commc_trie_node_t*  root;  /* ROOT NODE OF THE TRIE TREE */
  
  size_t              size;  /* NUMBER OF UNIQUE STRINGS STORED */

} commc_trie_t;

/*

         commc_trie_completions_t - auto-complete result container
           ---
           holds the results of prefix-based completion searches. contains
           an array of string pointers representing all strings that match
           the given prefix, along with the count of matches found.
           
           used by commc_trie_get_completions() to return multiple matching
           strings efficiently. the caller is responsible for freeing both
           the individual strings and the completions array.

*/

typedef struct {

  char**  strings;  /* ARRAY OF COMPLETION STRINGS */
  
  size_t  count;    /* NUMBER OF COMPLETIONS FOUND */

} commc_trie_completions_t;

/* 
	==================================
             --- CORE OPERATIONS ---
	==================================
*/

/*

         commc_trie_create()
           ---
           creates and initializes a new empty trie data structure.
           
           allocates memory for the trie container and its root node,
           initializing all child pointers to NULL and setting the
           initial size to zero. the root represents an empty string
           and serves as the foundation for all string storage.
           
           returns NULL if memory allocation fails, otherwise returns
           a pointer to the newly created trie ready for string operations.

*/

commc_trie_t*  commc_trie_create(void);

/*

         commc_trie_destroy()
           ---
           recursively destroys the trie and frees all allocated memory.
           
           performs a depth-first traversal of the trie tree, freeing
           each node and its associated memory. this includes both the
           trie container and all internal nodes created during string
           insertion operations.
           
           after this call, the trie pointer becomes invalid and should
           not be used. passing NULL is safe and results in no operation.

*/

void  commc_trie_destroy(commc_trie_t* trie);

/* 
	==================================
             --- STRING OPERATIONS ---
	==================================
*/

/*

         commc_trie_insert()
           ---
           inserts a string into the trie, creating nodes as necessary.
           
           traverses the trie following the character path defined by
           the input string. creates new nodes for any missing characters
           along the path. marks the final node as end_of_word to indicate
           a complete string is stored at this location.
           
           supports inserting duplicate strings - subsequent insertions
           of the same string are ignored without error. handles empty
           strings by marking the root node as end_of_word.
           
           returns COMMC_SUCCESS on successful insertion, COMMC_ARGUMENT_ERROR
           for NULL parameters, or COMMC_MEMORY_ERROR if node allocation fails.

*/

commc_error_t  commc_trie_insert(commc_trie_t* trie, const char* string);

/*

         commc_trie_search()
           ---
           searches for an exact string match within the trie.
           
           traverses the trie following the character path defined by
           the search string. returns true only if the complete path
           exists and the final node is marked as end_of_word.
           
           this distinguishes between stored strings and partial matches.
           for example, if only "catch" is stored, searching for "cat"
           returns false even though "cat" is a prefix of "catch".
           
           returns true if the exact string is found, false otherwise.
           NULL parameters result in false return value.

*/

bool  commc_trie_search(const commc_trie_t* trie, const char* string);

/*

         commc_trie_delete()
           ---
           removes a string from the trie, cleaning up unnecessary nodes.
           
           locates the string in the trie and unmarks the end_of_word flag.
           then performs cleanup by removing nodes that are no longer needed
           - nodes with no children and not marking end of other strings.
           
           this optimization prevents memory waste from deleted strings
           while preserving the structure needed for remaining strings.
           deleting a non-existent string returns success without error.
           
           returns COMMC_SUCCESS on successful deletion (or if string not found),
           or COMMC_ARGUMENT_ERROR for NULL parameters.

*/

commc_error_t  commc_trie_delete(commc_trie_t* trie, const char* string);

/* 
	==================================
             --- PREFIX OPERATIONS ---
	==================================
*/

/*

         commc_trie_has_prefix()
           ---
           checks whether any stored strings begin with the given prefix.
           
           traverses the trie following the character path of the prefix.
           returns true if the path exists, regardless of whether the
           prefix itself is a complete stored string. this indicates that
           at least one stored string shares this common prefix.
           
           useful for validation before calling expensive completion
           operations. an empty prefix returns true if the trie contains
           any strings (since all strings have the empty string as prefix).
           
           returns true if the prefix exists in the trie, false otherwise.
           NULL parameters result in false return value.

*/

bool  commc_trie_has_prefix(const commc_trie_t* trie, const char* prefix);

/*

         commc_trie_get_completions()
           ---
           finds all strings that begin with the specified prefix.
           
           traverses to the prefix location in the trie, then performs
           a depth-first search to collect all complete strings extending
           from that point. reconstructs full strings by combining the
           original prefix with the suffixes found during traversal.
           
           returns results in a commc_trie_completions_t structure containing
           an array of string pointers and the count of matches found.
           limits results to COMMC_TRIE_MAX_COMPLETIONS for memory safety.
           
           the caller is responsible for freeing both the individual strings
           and the completions array using commc_trie_free_completions().
           
           returns NULL if memory allocation fails or parameters are invalid.
           empty prefix returns all stored strings in the trie.

*/

commc_trie_completions_t*  commc_trie_get_completions(const commc_trie_t* trie, const char* prefix);

/*

         commc_trie_free_completions()
           ---
           frees memory allocated for completion results.
           
           releases all memory associated with a commc_trie_completions_t
           structure, including the individual string allocations and
           the container arrays. this cleanup is essential to prevent
           memory leaks after using commc_trie_get_completions().
           
           passing NULL is safe and results in no operation. after this
           call, the completions pointer becomes invalid and should not be used.

*/

void  commc_trie_free_completions(commc_trie_completions_t* completions);

/* 
	==================================
             --- UTILITY OPERATIONS ---
	==================================
*/

/*

         commc_trie_size()
           ---
           returns the number of unique strings stored in the trie.
           
           provides O(1) access to the count of stored strings maintained
           internally during insert and delete operations. this count
           represents distinct strings only - inserting the same string
           multiple times does not increase the size.
           
           returns 0 for empty tries or NULL parameters. useful for
           capacity planning and progress reporting in applications.

*/

size_t  commc_trie_size(const commc_trie_t* trie);

/*

         commc_trie_is_empty()
           ---
           checks whether the trie contains any stored strings.
           
           equivalent to checking if commc_trie_size() returns 0, but
           provided as a convenience function for clarity in conditional
           expressions and logical flow.
           
           returns true if the trie is empty or NULL, false if it contains
           one or more strings.

*/

bool  commc_trie_is_empty(const commc_trie_t* trie);

/*

         commc_trie_clear()
           ---
           removes all strings from the trie while preserving the structure.
           
           efficiently resets the trie to an empty state by deallocating
           all nodes except the root and resetting the size counter to zero.
           this is more efficient than individual deletions when clearing
           large tries.
           
           the trie remains valid and ready for new insertions after clearing.
           NULL parameter is handled safely without error.
           
           returns COMMC_SUCCESS on successful clearing, or COMMC_ARGUMENT_ERROR
           for NULL parameters.

*/

commc_error_t  commc_trie_clear(commc_trie_t* trie);

/* 
	==================================
             --- EOF ---
	==================================
*/

#endif /* COMMC_TRIE_H */