/*
   ===================================
   C O M M O N - C
   AVL TREE MODULE
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- AVL TREE MODULE ---

    this module provides a self-balancing binary search tree
    implementation using the AVL (Adelson-Velsky and Landis)
    algorithm. AVL trees guarantee O(log n) performance for
    insertion, deletion, and search operations by maintaining
    strict height balance.

    an AVL tree is a binary search tree where the heights of
    the two child subtrees of any node differ by at most one.
    when this balance condition is violated, the tree performs
    rotations to restore balance.

    the four rotation types are:
    - left rotation: corrects right-heavy imbalance
    - right rotation: corrects left-heavy imbalance  
    - left-right rotation: corrects left-right imbalance
    - right-left rotation: corrects right-left imbalance

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#ifndef   COMMC_AVL_TREE_H
#define   COMMC_AVL_TREE_H

#include  <stddef.h>
#include  "error.h"

/*
	==================================
             --- STRUCTS ---
	==================================
*/

typedef struct commc_avl_tree_t commc_avl_tree_t;
typedef struct commc_avl_tree_node_t commc_avl_tree_node_t;

/*
	==================================
             --- TYPEDEFS ---
	==================================
*/

/*

         commc_avl_compare_func
	       ---
	       comparison function interface for AVL tree keys.
	       returns negative for a < b, zero for a == b,
	       positive for a > b.

	       this allows the AVL tree to work with any data type
	       by providing an appropriate comparison function.

*/

typedef int (*commc_avl_compare_func)(const void* a, const void* b);

/*
	==================================
             --- FUNCTIONS ---
	==================================
*/

/*

         commc_avl_tree_create()
	       ---
	       creates a new empty AVL tree with the specified
	       comparison function. the comparison function is
	       used to maintain proper ordering of keys.

*/

commc_avl_tree_t* commc_avl_tree_create(commc_avl_compare_func compare_func);

/*

         commc_avl_tree_destroy()
	       ---
	       frees all nodes in the AVL tree and the tree structure
	       itself. does not free user data stored in nodes - the
	       caller is responsible for managing that memory.

*/

void commc_avl_tree_destroy(commc_avl_tree_t* tree);

/*

         commc_avl_tree_insert()
	       ---
	       inserts a key-value pair into the AVL tree. if the
	       key already exists, its value is updated. after
	       insertion, the tree automatically rebalances itself
	       using rotations if necessary.

	       returns COMMC_SUCCESS on success, appropriate error
	       code on failure (typically COMMC_MEMORY_ERROR).

*/

commc_error_t commc_avl_tree_insert(commc_avl_tree_t* tree, void* key, void* value);

/*

         commc_avl_tree_get()
	       ---
	       retrieves the value associated with the given key.
	       returns NULL if the key is not found in the tree.

	       this operation is guaranteed O(log n) due to the
	       balanced nature of AVL trees.

*/

void* commc_avl_tree_get(commc_avl_tree_t* tree, const void* key);

/*

         commc_avl_tree_remove()
	       ---
	       removes the key-value pair with the specified key
	       from the tree. after removal, the tree automatically
	       rebalances itself using rotations if necessary.

	       returns COMMC_SUCCESS if the key was found and removed,
	       COMMC_ARGUMENT_ERROR if the key was not found.

*/

commc_error_t commc_avl_tree_remove(commc_avl_tree_t* tree, const void* key);

/*

         commc_avl_tree_size()
	       ---
	       returns the number of key-value pairs stored in the tree.

*/

size_t commc_avl_tree_size(commc_avl_tree_t* tree);

/*

         commc_avl_tree_height()
	       ---
	       returns the height of the tree (number of levels).
	       an empty tree has height 0, a single node has height 1.

	       this function demonstrates the balanced nature of AVL
	       trees - the height will be approximately log2(n) where
	       n is the number of nodes.

*/

size_t commc_avl_tree_height(commc_avl_tree_t* tree);

/*

         commc_avl_tree_clear()
	       ---
	       removes all nodes from the tree, leaving it empty.
	       does not free user data - caller must handle that.

*/

void commc_avl_tree_clear(commc_avl_tree_t* tree);

/*

         commc_avl_tree_contains()
	       ---
	       checks whether the tree contains a node with the
	       specified key. returns 1 if found, 0 if not found.

*/

int commc_avl_tree_contains(commc_avl_tree_t* tree, const void* key);

/*

         commc_avl_tree_min_key()
	       ---
	       returns the smallest key in the tree, or NULL if
	       the tree is empty. useful for range operations.

*/

void* commc_avl_tree_min_key(commc_avl_tree_t* tree);

/*

         commc_avl_tree_max_key()
	       ---
	       returns the largest key in the tree, or NULL if
	       the tree is empty. useful for range operations.

*/

void* commc_avl_tree_max_key(commc_avl_tree_t* tree);

/*
	==================================
             --- EOF ---
	==================================
*/

#endif /* COMMC_AVL_TREE_H */