/*
   ===================================
   C O M M O N - C
   B-TREE HEADER
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- B-TREE MODULE ---

    multi-way search tree optimized for systems with large
    block sizes, particularly disk storage and database systems.
    
    B-Trees maintain balance through controlled node splitting
    and merging operations, ensuring logarithmic height while
    maximizing data per node to minimize disk accesses.
    
    unlike binary trees, B-Tree nodes can have many children
    (determined by the 'order' or 'degree'), making them ideal
    for database indexes and file system structures.

*/

#ifndef COMMC_B_TREE_H
#define COMMC_B_TREE_H

/*
	==================================
             --- INCLUDES ---
	==================================
*/

#include "commc/error.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
	==================================
             --- TYPES ---
	==================================
*/

/* forward declarations */

typedef struct commc_b_tree_t      commc_b_tree_t;
typedef struct commc_b_tree_node_t commc_b_tree_node_t;

/* comparison function type for B-Tree keys */

typedef int (*commc_b_compare_func)(const void* a, const void* b);

/*
	==================================
             --- CORE FUNCS ---
	==================================
*/

/*

         commc_b_tree_create()
	       ---
	       creates a new B-Tree with the specified minimum degree.
	       
	       the minimum degree 't' determines the capacity:
	       - each node can have at most (2t-1) keys
	       - each internal node can have at most 2t children
	       - each node (except root) must have at least (t-1) keys
	       - each internal node (except root) has at least t children
	       
	       typical values: t=3 (order 6), t=4 (order 8), etc.
	       larger values reduce tree height but increase node size.

*/

commc_b_tree_t* commc_b_tree_create(int min_degree, commc_b_compare_func compare_func);

/*

         commc_b_tree_destroy()
	       ---
	       frees all nodes and the B-Tree structure itself.
	       performs recursive traversal to deallocate all memory.

*/

void commc_b_tree_destroy(commc_b_tree_t* tree);

/*

         commc_b_tree_insert()
	       ---
	       inserts a key-value pair into the B-Tree.
	       
	       automatically handles node splitting when nodes become
	       full (contain 2t-1 keys). splitting propagates upward
	       and may create a new root, increasing tree height by one.
	       
	       if the key already exists, updates the value.

*/

commc_error_t commc_b_tree_insert(commc_b_tree_t* tree, void* key, void* value);

/*

         commc_b_tree_get()
	       ---
	       retrieves the value associated with a given key.
	       returns NULL if the key is not found.
	       
	       search proceeds from root to leaf, examining keys
	       within each node and following appropriate child
	       pointers based on comparison results.

*/

void* commc_b_tree_get(commc_b_tree_t* tree, const void* key);

/*

         commc_b_tree_remove()
	       ---
	       removes a key-value pair from the B-Tree.
	       
	       automatically handles node merging and key redistribution
	       when nodes become underfull (have fewer than t-1 keys).
	       may decrease tree height if root becomes empty.

*/

commc_error_t commc_b_tree_remove(commc_b_tree_t* tree, const void* key);

/*

         commc_b_tree_contains()
	       ---
	       checks if the B-Tree contains a specific key.
	       more efficient than get() when only existence matters.

*/

int commc_b_tree_contains(commc_b_tree_t* tree, const void* key);

/*

         commc_b_tree_size()
	       ---
	       returns the total number of key-value pairs in the tree.

*/

size_t commc_b_tree_size(commc_b_tree_t* tree);

/*

         commc_b_tree_height()
	       ---
	       returns the height of the B-Tree.
	       height is logarithmic in base t (minimum degree).

*/

size_t commc_b_tree_height(commc_b_tree_t* tree);

/*

         commc_b_tree_clear()
	       ---
	       removes all key-value pairs from the tree.
	       leaves the tree empty but intact for reuse.

*/

void commc_b_tree_clear(commc_b_tree_t* tree);

/*

         commc_b_tree_min_key()
	       ---
	       returns the smallest key in the tree.
	       found by following leftmost path to leaf node
	       and returning the first key of that leaf.

*/

void* commc_b_tree_min_key(commc_b_tree_t* tree);

/*

         commc_b_tree_max_key()
	       ---
	       returns the largest key in the tree.
	       found by following rightmost path to leaf node
	       and returning the last key of that leaf.

*/

void* commc_b_tree_max_key(commc_b_tree_t* tree);

/*

         commc_b_tree_validate()
	       ---
	       validates B-Tree structural properties.
	       
	       checks:
	       1. key ordering within nodes and between levels
	       2. minimum and maximum key counts per node
	       3. all leaves at same level
	       4. internal node child counts match key counts
	       
	       returns 1 if valid, 0 if corrupted.

*/

int commc_b_tree_validate(commc_b_tree_t* tree);

/*
	==================================
             --- ADVANCED FUNCS ---
	==================================
*/

/*

         commc_b_tree_get_min_degree()
	       ---
	       returns the minimum degree (t value) of the B-Tree.
	       useful for understanding tree characteristics.

*/

int commc_b_tree_get_min_degree(commc_b_tree_t* tree);

/*

         commc_b_tree_get_max_keys()
	       ---
	       returns the maximum number of keys per node (2t-1).
	       calculated from minimum degree for convenience.

*/

int commc_b_tree_get_max_keys(commc_b_tree_t* tree);

/*

         commc_b_tree_print()
	       ---
	       prints the B-Tree structure for debugging.
	       shows keys in each node and tree structure visually.
	       requires a print function for keys.

*/

typedef void (*commc_b_print_key_func)(const void* key);

void commc_b_tree_print(commc_b_tree_t* tree, commc_b_print_key_func print_func);

/*

         commc_b_tree_range_search()
	       ---
	       performs range query to find all keys between min and max.
	       efficient for database-style range queries.
	       results are collected in provided array up to max_results.
	       returns actual number of results found.

*/

size_t commc_b_tree_range_search(commc_b_tree_t* tree, 
                                 const void* min_key, 
                                 const void* max_key,
                                 void** results, 
                                 size_t max_results);

#ifdef __cplusplus
}
#endif

#endif /* COMMC_B_TREE_H */

/*
	==================================
             --- EOF ---
	==================================
*/