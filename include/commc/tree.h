/*
   ===================================
   C O M M O N - C
   BINARY SEARCH TREE MODULE
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- TREE MODULE ---

    this module provides a generic binary search tree (BST)
    implementation. it stores key-value pairs and allows
    efficient searching, insertion, and deletion.

    the keys are generic (void*), requiring a comparison
    function provided by the user.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#ifndef   COMMC_TREE_H
#define   COMMC_TREE_H

#include  <stddef.h>

/*
	==================================
             --- STRUCTS ---
	==================================
*/

typedef struct commc_tree_t commc_tree_t;
typedef struct commc_tree_node_t commc_tree_node_t;

/*
	==================================
             --- TYPEDEFS ---
	==================================
*/

/* function pointer for comparing two keys.
   returns < 0 if a < b, 0 if a == b, > 0 if a > b. */

typedef int (*commc_tree_compare_func)(const void* a, const void* b);

/*
	==================================
             --- FUNCTIONS ---
	==================================
*/

/*

         commc_tree_create()
	       ---
	       creates a new, empty binary search tree.
	       requires a comparison function for keys.

*/

commc_tree_t* commc_tree_create(commc_tree_compare_func compare_func);

/*

         commc_tree_destroy()
	       ---
	       frees all memory associated with the tree.
	       note: does not free the data held by the nodes.

*/

void commc_tree_destroy(commc_tree_t* tree);

/*

         commc_tree_insert()
	       ---
	       inserts a key-value pair into the tree.
	       if the key already exists, its value is updated.

*/

int commc_tree_insert(commc_tree_t* tree, void* key, void* value);

/*

         commc_tree_get()
	       ---
	       retrieves the value associated with a given key.
	       returns null if the key is not found.

*/

void* commc_tree_get(commc_tree_t* tree, const void* key);

/*

         commc_tree_remove()
	       ---
	       removes a key-value pair from the tree.

*/

void commc_tree_remove(commc_tree_t* tree, const void* key);

/*

         commc_tree_size()
	       ---
	       returns the number of elements in the tree.

*/

size_t commc_tree_size(commc_tree_t* tree);

/*

         commc_tree_is_empty()
	       ---
	       returns 1 if the tree is empty, otherwise 0.

*/

int commc_tree_is_empty(commc_tree_t* tree);

#endif /* COMMC_TREE_H */

/*
	==================================
             --- EOF ---
	==================================
*/
