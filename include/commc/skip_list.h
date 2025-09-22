/* 	
   ===================================
   C O M M C / S K I P _ L I S T . H
   PROBABILISTIC SKIP LIST API
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

	                --- SKIP LIST API ---

	    skip lists provide probabilistic O(log n) search, insertion, and
	    deletion operations in sorted sequences through multiple levels
	    of linked lists. higher levels contain fewer elements, creating
	    an express lane effect for faster traversal.
	    
	    this implementation uses randomization to maintain balance without
	    complex tree rotation operations, making it simpler than balanced
	    binary search trees while providing similar performance.

*/

#ifndef COMMC_SKIP_LIST_H
#define COMMC_SKIP_LIST_H

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

         commc_skip_list_node_t
	       ---
	       represents a single node in the skip list with multiple
	       forward pointers for different levels.

*/

typedef struct commc_skip_list_node {

  void*                           key;        /* stored key */
  void*                           value;      /* stored value */
  size_t                          key_size;   /* key size in bytes */
  size_t                          value_size; /* value size in bytes */
  size_t                          level;      /* height of this node */
  struct commc_skip_list_node**   forward;    /* array of forward pointers */
  
} commc_skip_list_node_t;

/*

         commc_skip_list_t
	       ---
	       represents the skip list structure with header node and
	       current maximum level information.

*/

typedef struct {

  commc_skip_list_node_t*  header;      /* header node with max levels */
  size_t                   max_level;   /* maximum level in current list */
  size_t                   size;        /* number of elements */
  double                   probability; /* level promotion probability */
  
} commc_skip_list_t;

/*

         commc_skip_list_compare_t
	       ---
	       function pointer type for key comparison.
	       should return: < 0 if key1 < key2, 0 if equal, > 0 if key1 > key2

*/

typedef int (*commc_skip_list_compare_t)(const void* key1, size_t key1_size,
                                         const void* key2, size_t key2_size);

/* 
	==================================
             --- CONSTANTS ---
	==================================
*/

/*

         COMMC_SKIP_LIST_MAX_LEVEL
	       ---
	       maximum number of levels in skip list.

*/

#define COMMC_SKIP_LIST_MAX_LEVEL 16

/*

         COMMC_SKIP_LIST_DEFAULT_PROBABILITY
	       ---
	       default probability for level promotion (0.5 is optimal).

*/

#define COMMC_SKIP_LIST_DEFAULT_PROBABILITY 0.5

/* 
	==================================
             --- CORE API ---
	==================================
*/

/*

         commc_skip_list_create()
	       ---
	       creates a new skip list with default probability.
	       
	       returns:
	       - pointer to new skip list, or NULL on error

*/

commc_skip_list_t* commc_skip_list_create(void);

/*

         commc_skip_list_create_with_probability()
	       ---
	       creates skip list with custom level promotion probability.
	       
	       parameters:
	       - probability: chance of promoting to next level (0.0 to 1.0)
	       
	       returns:
	       - pointer to new skip list, or NULL on error

*/

commc_skip_list_t* commc_skip_list_create_with_probability(double probability);

/*

         commc_skip_list_destroy()
	       ---
	       destroys skip list and frees all associated memory.

*/

void commc_skip_list_destroy(commc_skip_list_t* list);

/*

         commc_skip_list_insert()
	       ---
	       inserts a key-value pair into the skip list.
	       if key already exists, updates the value.
	       
	       parameters:
	       - list: skip list to modify
	       - key: pointer to key data
	       - key_size: size of key data in bytes
	       - value: pointer to value data
	       - value_size: size of value data in bytes
	       - compare: key comparison function
	       
	       returns:
	       - COMMC_SUCCESS on successful insertion
	       - COMMC_ARGUMENT_ERROR for invalid parameters
	       - COMMC_MEMORY_ERROR if memory allocation fails

*/

commc_error_t commc_skip_list_insert(commc_skip_list_t* list,
                                      const void* key, size_t key_size,
                                      const void* value, size_t value_size,
                                      commc_skip_list_compare_t compare);

/*

         commc_skip_list_search()
	       ---
	       searches for a key in the skip list.
	       
	       parameters:
	       - list: skip list to search
	       - key: pointer to key data to find
	       - key_size: size of key data in bytes
	       - value: pointer to store found value (output)
	       - value_size: pointer to store value size (output)
	       - compare: key comparison function
	       
	       returns:
	       - COMMC_SUCCESS if key found
	       - COMMC_FAILURE if key not found
	       - COMMC_ARGUMENT_ERROR for invalid parameters

*/

commc_error_t commc_skip_list_search(commc_skip_list_t* list,
                                      const void* key, size_t key_size,
                                      void** value, size_t* value_size,
                                      commc_skip_list_compare_t compare);

/*

         commc_skip_list_delete()
	       ---
	       removes a key-value pair from the skip list.
	       
	       parameters:
	       - list: skip list to modify
	       - key: pointer to key data to remove
	       - key_size: size of key data in bytes
	       - compare: key comparison function
	       
	       returns:
	       - COMMC_SUCCESS if key found and removed
	       - COMMC_FAILURE if key not found
	       - COMMC_ARGUMENT_ERROR for invalid parameters

*/

commc_error_t commc_skip_list_delete(commc_skip_list_t* list,
                                      const void* key, size_t key_size,
                                      commc_skip_list_compare_t compare);

/* 
	==================================
             --- INFO API ---
	==================================
*/

/*

         commc_skip_list_size()
	       ---
	       returns the number of key-value pairs in the skip list.

*/

size_t commc_skip_list_size(commc_skip_list_t* list);

/*

         commc_skip_list_is_empty()
	       ---
	       returns 1 if skip list is empty, 0 otherwise.

*/

int commc_skip_list_is_empty(commc_skip_list_t* list);

/*

         commc_skip_list_contains()
	       ---
	       returns 1 if key exists in skip list, 0 otherwise.

*/

int commc_skip_list_contains(commc_skip_list_t* list,
                             const void* key, size_t key_size,
                             commc_skip_list_compare_t compare);

/*

         commc_skip_list_get_max_level()
	       ---
	       returns the current maximum level in the skip list.

*/

size_t commc_skip_list_get_max_level(commc_skip_list_t* list);

/*

         commc_skip_list_memory_usage()
	       ---
	       estimates total memory usage in bytes.

*/

size_t commc_skip_list_memory_usage(commc_skip_list_t* list);

/* 
	==================================
             --- ITERATION API ---
	==================================
*/

/*

         commc_skip_list_iterator_t
	       ---
	       iterator for traversing skip list in sorted key order.

*/

typedef struct {

  commc_skip_list_t*       list;     /* skip list being iterated */
  commc_skip_list_node_t*  current;  /* current node */
  
} commc_skip_list_iterator_t;

/*

         commc_skip_list_iterator_begin()
	       ---
	       creates iterator starting from smallest key.

*/

commc_skip_list_iterator_t commc_skip_list_iterator_begin(commc_skip_list_t* list);

/*

         commc_skip_list_iterator_next()
	       ---
	       advances iterator to next key in sorted order.

*/

commc_error_t commc_skip_list_iterator_next(commc_skip_list_iterator_t* iterator);

/*

         commc_skip_list_iterator_data()
	       ---
	       retrieves key-value pair from current iterator position.

*/

commc_error_t commc_skip_list_iterator_data(commc_skip_list_iterator_t* iterator,
                                             void** key, size_t* key_size,
                                             void** value, size_t* value_size);

/*

         commc_skip_list_iterator_has_next()
	       ---
	       returns 1 if iterator has more elements.

*/

int commc_skip_list_iterator_has_next(commc_skip_list_iterator_t* iterator);

/* 
	==================================
             --- UTILITY API ---
	==================================
*/

/*

         commc_skip_list_clear()
	       ---
	       removes all elements from skip list.

*/

void commc_skip_list_clear(commc_skip_list_t* list);

/*

         commc_skip_list_find_min()
	       ---
	       finds the minimum key in the skip list.

*/

commc_error_t commc_skip_list_find_min(commc_skip_list_t* list,
                                        void** key, size_t* key_size,
                                        void** value, size_t* value_size);

/*

         commc_skip_list_find_max()
	       ---
	       finds the maximum key in the skip list.

*/

commc_error_t commc_skip_list_find_max(commc_skip_list_t* list,
                                        void** key, size_t* key_size,
                                        void** value, size_t* value_size);

/*

         commc_skip_list_get_level_counts()
	       ---
	       returns array with count of nodes at each level.
	       useful for analyzing skip list distribution.
	       
	       parameters:
	       - list: skip list to analyze
	       - level_counts: output array (must have COMMC_SKIP_LIST_MAX_LEVEL elements)
	       
	       returns:
	       - COMMC_SUCCESS if analysis completed
	       - COMMC_ARGUMENT_ERROR for invalid parameters

*/

commc_error_t commc_skip_list_get_level_counts(commc_skip_list_t* list,
                                                size_t* level_counts);

/* 
	==================================
             --- RANGE API ---
	==================================
*/

/*

         commc_skip_list_range_search()
	       ---
	       finds all keys within a specified range [min_key, max_key].
	       
	       parameters:
	       - list: skip list to search
	       - min_key: minimum key in range
	       - min_key_size: size of minimum key
	       - max_key: maximum key in range  
	       - max_key_size: size of maximum key
	       - results: output array for matching key-value pairs
	       - max_results: maximum number of results to return
	       - compare: key comparison function
	       
	       returns:
	       - number of results found (may be 0)

*/

size_t commc_skip_list_range_search(commc_skip_list_t* list,
                                     const void* min_key, size_t min_key_size,
                                     const void* max_key, size_t max_key_size,
                                     commc_skip_list_node_t** results,
                                     size_t max_results,
                                     commc_skip_list_compare_t compare);

/* 
	==================================
             --- COMPARE API ---
	==================================
*/

/*

         commc_skip_list_compare_int()
	       ---
	       comparison function for integer keys.

*/

int commc_skip_list_compare_int(const void* key1, size_t key1_size,
                                const void* key2, size_t key2_size);

/*

         commc_skip_list_compare_string()
	       ---
	       comparison function for null-terminated string keys.

*/

int commc_skip_list_compare_string(const void* key1, size_t key1_size,
                                   const void* key2, size_t key2_size);

/*

         commc_skip_list_compare_memcmp()
	       ---
	       comparison function using memcmp for arbitrary byte keys.

*/

int commc_skip_list_compare_memcmp(const void* key1, size_t key1_size,
                                   const void* key2, size_t key2_size);

#ifdef __cplusplus
}
#endif

#endif /* COMMC_SKIP_LIST_H */

/*
	==================================
             --- EOF ---
	==================================
*/