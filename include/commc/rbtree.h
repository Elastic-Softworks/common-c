/*
   ===================================
   C O M M O N - C
   RED-BLACK TREE MODULE
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- RED-BLACK TREE MODULE ---

    this module provides a self-balancing binary search tree
    implementation using the Red-Black tree algorithm. Red-Black
    trees guarantee O(log n) performance for insertion, deletion,
    and search operations by maintaining color properties and
    structural balance.

    a Red-Black tree is a binary search tree where each node
    has a color (red or black) and the tree satisfies these
    five critical properties:

    1. every node is either red or black
    2. the root node is always black
    3. all leaf nodes (NIL) are black
    4. red nodes cannot have red children (no two red nodes adjacent)
    5. every path from root to leaf contains the same number of black nodes

    when these properties are violated during insertion or deletion,
    the tree performs rotations and recoloring to restore balance.
    this ensures the tree height remains logarithmic.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#ifndef   COMMC_RB_TREE_H
#define   COMMC_RB_TREE_H

#include  <stddef.h>
#include  "error.h"

/*
	==================================
             --- ENUMS ---
	==================================
*/

/*

         commc_rb_color_t
	       ---
	       enumeration for node colors in Red-Black trees.
	       only two colors are needed: red and black.

*/

typedef enum {
  COMMC_RB_RED = 0,                              /* red nodes allow for flexibility */
  COMMC_RB_BLACK = 1                             /* black nodes provide structure */
} commc_rb_color_t;

/*
	==================================
             --- STRUCTS ---
	==================================
*/

typedef struct commc_rb_tree_t commc_rb_tree_t;
typedef struct commc_rb_tree_node_t commc_rb_tree_node_t;

/*
	==================================
             --- TYPEDEFS ---
	==================================
*/

/*

         commc_rb_compare_func
	       ---
	       comparison function interface for Red-Black tree keys.
	       returns negative for a < b, zero for a == b,
	       positive for a > b.

	       this allows the Red-Black tree to work with any data
	       type by providing an appropriate comparison function.

*/

typedef int (*commc_rb_compare_func)(const void* a, const void* b);

/*
	==================================
             --- FUNCTIONS ---
	==================================
*/

/*

         commc_rb_tree_create()
	       ---
	       creates a new empty Red-Black tree with the specified
	       comparison function. the comparison function is used
	       to maintain proper ordering of keys.

*/

commc_rb_tree_t* commc_rb_tree_create(commc_rb_compare_func compare_func);

/*

         commc_rb_tree_destroy()
	       ---
	       frees all nodes in the Red-Black tree and the tree
	       structure itself. does not free user data stored in
	       nodes - the caller is responsible for managing that memory.

*/

void commc_rb_tree_destroy(commc_rb_tree_t* tree);

/*

         commc_rb_tree_insert()
	       ---
	       inserts a key-value pair into the Red-Black tree. if the
	       key already exists, its value is updated. after insertion,
	       the tree automatically rebalances itself using rotations
	       and recoloring if necessary.

	       returns COMMC_SUCCESS on success, appropriate error
	       code on failure (typically COMMC_MEMORY_ERROR).

*/

commc_error_t commc_rb_tree_insert(commc_rb_tree_t* tree, void* key, void* value);

/*

         commc_rb_tree_get()
	       ---
	       retrieves the value associated with the given key.
	       returns NULL if the key is not found in the tree.

	       this operation is guaranteed O(log n) due to the
	       balanced nature of Red-Black trees.

*/

void* commc_rb_tree_get(commc_rb_tree_t* tree, const void* key);

/*

         commc_rb_tree_remove()
	       ---
	       removes the key-value pair with the specified key
	       from the tree. after removal, the tree automatically
	       rebalances itself using rotations and recoloring if
	       necessary.

	       returns COMMC_SUCCESS if the key was found and removed,
	       COMMC_ARGUMENT_ERROR if the key was not found.

*/

commc_error_t commc_rb_tree_remove(commc_rb_tree_t* tree, const void* key);

/*

         commc_rb_tree_size()
	       ---
	       returns the number of key-value pairs stored in the tree.

*/

size_t commc_rb_tree_size(commc_rb_tree_t* tree);

/*

         commc_rb_tree_height()
	       ---
	       returns the height of the tree (number of levels).
	       an empty tree has height 0, a single node has height 1.

	       Red-Black trees guarantee the height will be at most
	       2 * log2(n + 1) where n is the number of nodes.

*/

size_t commc_rb_tree_height(commc_rb_tree_t* tree);

/*

         commc_rb_tree_clear()
	       ---
	       removes all nodes from the tree, leaving it empty.
	       does not free user data - caller must handle that.

*/

void commc_rb_tree_clear(commc_rb_tree_t* tree);

/*

         commc_rb_tree_contains()
	       ---
	       checks whether the tree contains a node with the
	       specified key. returns 1 if found, 0 if not found.

*/

int commc_rb_tree_contains(commc_rb_tree_t* tree, const void* key);

/*

         commc_rb_tree_min_key()
	       ---
	       returns the smallest key in the tree, or NULL if
	       the tree is empty. useful for range operations.

*/

void* commc_rb_tree_min_key(commc_rb_tree_t* tree);

/*

         commc_rb_tree_max_key()
	       ---
	       returns the largest key in the tree, or NULL if
	       the tree is empty. useful for range operations.

*/

void* commc_rb_tree_max_key(commc_rb_tree_t* tree);

/*

         commc_rb_tree_validate()
	       ---
	       validates that the tree satisfies all Red-Black
	       properties. returns 1 if valid, 0 if invalid.
	       primarily used for testing and debugging.

*/

int commc_rb_tree_validate(commc_rb_tree_t* tree);

#endif              /* COMMC_RB_TREE_H */

/*
	==================================
             --- EOF ---
	==================================
*/