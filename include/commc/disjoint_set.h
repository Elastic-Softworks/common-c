/* 	
   ===================================
   C O M M C / D I S J O I N T _ S E T . H
   UNION-FIND DATA STRUCTURE API
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

	                --- DISJOINT SET API ---

	    disjoint sets (union-find) track partitions of elements where each
	    element belongs to exactly one set. this implementation provides
	    near-constant time union and find operations through path compression
	    and union by rank optimizations.
	    
	    commonly used for kruskal's minimum spanning tree, connected
	    components in graphs, and equivalence class management.

*/

#ifndef COMMC_DISJOINT_SET_H
#define COMMC_DISJOINT_SET_H

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

         commc_disjoint_set_node_t
	       ---
	       internal node structure representing a single element in
	       the union-find forest with parent pointer and rank.

*/

typedef struct commc_disjoint_set_node {

  size_t                            element;   /* element identifier */
  struct commc_disjoint_set_node*   parent;    /* parent in tree (self if root) */
  size_t                            rank;      /* approximate tree depth for union by rank */
  
} commc_disjoint_set_node_t;

/*

         commc_disjoint_set_t
	       ---
	       represents a disjoint set data structure managing partitions
	       of numbered elements from 0 to capacity-1.

*/

typedef struct {

  commc_disjoint_set_node_t*  nodes;           /* array of nodes */
  size_t                      capacity;        /* maximum number of elements */
  size_t                      set_count;       /* current number of disjoint sets */
  
} commc_disjoint_set_t;

/* 
	==================================
             --- CONSTANTS ---
	==================================
*/

/*

         COMMC_DISJOINT_SET_DEFAULT_CAPACITY
	       ---
	       default capacity if none specified.

*/

#define COMMC_DISJOINT_SET_DEFAULT_CAPACITY 1000

/*

         COMMC_DISJOINT_SET_MIN_CAPACITY
	       ---
	       minimum allowable capacity.

*/

#define COMMC_DISJOINT_SET_MIN_CAPACITY 1

/* 
	==================================
             --- CORE API ---
	==================================
*/

/*

         commc_disjoint_set_create()
	       ---
	       creates a new disjoint set structure managing elements 0 to capacity-1.
	       initially each element forms its own singleton set.
	       
	       parameters:
	       - capacity: maximum number of elements (0 to capacity-1)
	       
	       returns:
	       - pointer to new disjoint set, or NULL on error

*/

commc_disjoint_set_t* commc_disjoint_set_create(size_t capacity);

/*

         commc_disjoint_set_destroy()
	       ---
	       destroys disjoint set and frees all associated memory.

*/

void commc_disjoint_set_destroy(commc_disjoint_set_t* ds);

/*

         commc_disjoint_set_find()
	       ---
	       finds the representative (root) of the set containing the element.
	       applies path compression for optimization - all nodes on the path
	       to root are made direct children of root.
	       
	       parameters:
	       - ds: disjoint set structure
	       - element: element to find (must be < capacity)
	       
	       returns:
	       - representative element of the set containing element
	       - SIZE_MAX on error (invalid element or NULL ds)

*/

size_t commc_disjoint_set_find(commc_disjoint_set_t* ds, size_t element);

/*

         commc_disjoint_set_union()
	       ---
	       unites the sets containing two elements using union by rank.
	       the tree with smaller rank becomes a subtree of the other.
	       
	       parameters:
	       - ds: disjoint set structure  
	       - element1: first element (must be < capacity)
	       - element2: second element (must be < capacity)
	       
	       returns:
	       - COMMC_SUCCESS if union performed
	       - COMMC_FAILURE if elements already in same set
	       - COMMC_ARGUMENT_ERROR for invalid parameters

*/

commc_error_t commc_disjoint_set_union(commc_disjoint_set_t* ds, size_t element1, size_t element2);

/*

         commc_disjoint_set_connected()
	       ---
	       tests whether two elements belong to the same set.
	       equivalent to comparing find(element1) == find(element2)
	       but more semantically clear.
	       
	       parameters:
	       - ds: disjoint set structure
	       - element1: first element
	       - element2: second element
	       
	       returns:
	       - 1 if elements are in same set
	       - 0 if elements are in different sets or on error

*/

int commc_disjoint_set_connected(commc_disjoint_set_t* ds, size_t element1, size_t element2);

/* 
	==================================
             --- INFO API ---
	==================================
*/

/*

         commc_disjoint_set_capacity()
	       ---
	       returns the capacity (maximum number of elements).

*/

size_t commc_disjoint_set_capacity(commc_disjoint_set_t* ds);

/*

         commc_disjoint_set_count()
	       ---
	       returns the current number of disjoint sets.
	       starts at capacity (each element in its own set)
	       and decreases with each successful union operation.

*/

size_t commc_disjoint_set_count(commc_disjoint_set_t* ds);

/*

         commc_disjoint_set_size()
	       ---
	       returns the size of the set containing the given element.
	       uses find to locate the set and counts all elements with same root.
	       
	       time complexity: O(n * α(n)) where α is inverse ackermann function

*/

size_t commc_disjoint_set_size(commc_disjoint_set_t* ds, size_t element);

/*

         commc_disjoint_set_get_representatives()
	       ---
	       fills array with representative elements of all disjoint sets.
	       caller must provide array with at least commc_disjoint_set_count() elements.
	       
	       parameters:
	       - ds: disjoint set structure
	       - representatives: output array to fill with set representatives
	       - max_count: maximum elements to write to array
	       
	       returns:
	       - actual number of representatives written
	       - 0 on error

*/

size_t commc_disjoint_set_get_representatives(commc_disjoint_set_t* ds,
                                               size_t* representatives,
                                               size_t max_count);

/*

         commc_disjoint_set_get_set_members()
	       ---
	       fills array with all members of the set containing the given element.
	       caller must provide sufficient array space.
	       
	       parameters:
	       - ds: disjoint set structure
	       - element: element whose set members to retrieve
	       - members: output array to fill with set members
	       - max_count: maximum elements to write to array
	       
	       returns:
	       - actual number of members written
	       - 0 on error

*/

size_t commc_disjoint_set_get_set_members(commc_disjoint_set_t* ds,
                                           size_t element,
                                           size_t* members,
                                           size_t max_count);

/* 
	==================================
             --- UTILITY API ---
	==================================
*/

/*

         commc_disjoint_set_reset()
	       ---
	       resets all elements to singleton sets (initial state).
	       each element becomes its own set representative.

*/

void commc_disjoint_set_reset(commc_disjoint_set_t* ds);

/*

         commc_disjoint_set_compress_paths()
	       ---
	       explicitly applies path compression to all elements.
	       normally path compression happens during find operations,
	       but this function can be used to pre-optimize the structure.

*/

void commc_disjoint_set_compress_paths(commc_disjoint_set_t* ds);

/*

         commc_disjoint_set_is_representative()
	       ---
	       tests whether element is a set representative (root node).
	       
	       returns:
	       - 1 if element is root of its tree
	       - 0 if element is not root or on error

*/

int commc_disjoint_set_is_representative(commc_disjoint_set_t* ds, size_t element);

/*

         commc_disjoint_set_get_rank()
	       ---
	       returns the rank (approximate depth) of element's tree.
	       only meaningful for representative elements.

*/

size_t commc_disjoint_set_get_rank(commc_disjoint_set_t* ds, size_t element);

/* 
	==================================
             --- ITERATION API ---
	==================================
*/

/*

         commc_disjoint_set_iterator_t
	       ---
	       iterator for traversing all elements in a specific set.

*/

typedef struct {

  commc_disjoint_set_t* ds;              /* disjoint set being iterated */
  size_t                representative;  /* representative of target set */
  size_t                current;         /* current element index */
  
} commc_disjoint_set_iterator_t;

/*

         commc_disjoint_set_iterator_create()
	       ---
	       creates iterator for all elements in the set containing given element.

*/

commc_disjoint_set_iterator_t commc_disjoint_set_iterator_create(commc_disjoint_set_t* ds,
                                                                 size_t element);

/*

         commc_disjoint_set_iterator_next()
	       ---
	       advances iterator to next element in the set.

*/

commc_error_t commc_disjoint_set_iterator_next(commc_disjoint_set_iterator_t* iterator);

/*

         commc_disjoint_set_iterator_current()
	       ---
	       returns current element from iterator.

*/

size_t commc_disjoint_set_iterator_current(commc_disjoint_set_iterator_t* iterator);

/*

         commc_disjoint_set_iterator_has_next()
	       ---
	       returns 1 if iterator has more elements.

*/

int commc_disjoint_set_iterator_has_next(commc_disjoint_set_iterator_t* iterator);

/* 
	==================================
             --- ANALYSIS API ---
	==================================
*/

/*

         commc_disjoint_set_get_statistics()
	       ---
	       structure for disjoint set performance statistics.

*/

typedef struct {

  size_t  total_sets;           /* current number of disjoint sets */
  size_t  largest_set_size;     /* size of largest set */
  size_t  smallest_set_size;    /* size of smallest set */
  double  average_set_size;     /* average set size */
  size_t  max_tree_depth;       /* maximum tree depth */
  double  average_tree_depth;   /* average tree depth */
  
} commc_disjoint_set_statistics_t;

/*

         commc_disjoint_set_analyze()
	       ---
	       computes comprehensive statistics about the disjoint set structure.
	       useful for performance analysis and debugging.

*/

commc_disjoint_set_statistics_t commc_disjoint_set_analyze(commc_disjoint_set_t* ds);

/*

         commc_disjoint_set_memory_usage()
	       ---
	       returns total memory usage in bytes.

*/

size_t commc_disjoint_set_memory_usage(commc_disjoint_set_t* ds);

/*

         commc_disjoint_set_validate()
	       ---
	       validates internal data structure consistency.
	       checks for cycles, proper parent relationships, etc.
	       
	       returns:
	       - COMMC_SUCCESS if structure is valid
	       - COMMC_FAILURE if corruption detected

*/

commc_error_t commc_disjoint_set_validate(commc_disjoint_set_t* ds);

#ifdef __cplusplus
}
#endif

#endif /* COMMC_DISJOINT_SET_H */

/*
	==================================
             --- EOF ---
	==================================
*/