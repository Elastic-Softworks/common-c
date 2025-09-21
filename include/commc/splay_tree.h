/*
   ===================================
   C O M M O N - C
   SPLAY TREE MODULE
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- SPLAY TREE MODULE ---

    this module provides a self-adjusting binary search tree
    implementation using the splay tree algorithm. splay trees
    achieve amortized O(log n) performance by moving frequently
    accessed elements toward the root through splaying operations.

    a splay tree is a binary search tree that performs a "splay"
    operation whenever a node is accessed (insert, delete, search).
    splaying moves the accessed node to the root through a series
    of rotations, improving future access times for recently used
    elements.

    the splaying operation consists of three cases:
    - zig: single rotation when node is child of root
    - zig-zag: double rotation for different-direction grandparent
    - zig-zig: double rotation for same-direction grandparent

    splay trees excel in applications with temporal locality,
    where recently accessed elements are likely to be accessed
    again. examples include caches, memory management, and
    compression algorithms.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#ifndef   COMMC_SPLAY_TREE_H
#define   COMMC_SPLAY_TREE_H

#include  <stddef.h>
#include  "error.h"

/*
	==================================
             --- STRUCTS ---
	==================================
*/

typedef struct commc_splay_tree_t commc_splay_tree_t;

typedef struct commc_splay_node_t {

  int    key;                            /* the search key value */
  void*  data;                           /* user data pointer */

  struct commc_splay_node_t*  left;      /* left child pointer */
  struct commc_splay_node_t*  right;     /* right child pointer */
  struct commc_splay_node_t*  parent;    /* parent pointer for splaying */

} commc_splay_node_t;

struct commc_splay_tree_t {

  commc_splay_node_t*  root;             /* root node of the tree */
  size_t               size;             /* total number of nodes */

  /* comparison and cleanup functions */

  int   (*compare)(int a, int b);        /* key comparison function */
  void  (*destroy_data)(void* data);     /* data cleanup function */

};

/*
	==================================
             --- FUNCS ---
	==================================
*/

/*

         commc_splay_tree_create()
	       ---
	       creates a new splay tree structure with the specified
	       comparison and data destruction functions.
	       
	       the compare function should return negative for a < b,
	       zero for a == b, and positive for a > b. if null is
	       passed, a default integer comparison is used.
	       
	       the destroy_data function is called when nodes are
	       removed. pass null if no cleanup is needed.

*/

commc_error_t commc_splay_tree_create(
  commc_splay_tree_t**  tree,
  int   (*compare)(int a, int b),
  void  (*destroy_data)(void* data)
);

/*

         commc_splay_tree_destroy()
	       ---
	       destroys the splay tree and frees all associated memory.
	       calls the destroy_data function for each node if one
	       was provided during creation.
	       
	       all node data is cleaned up and the tree structure
	       itself is freed. the tree pointer is set to null.

*/

commc_error_t commc_splay_tree_destroy(
  commc_splay_tree_t**  tree
);

/*

         commc_splay_tree_insert()
	       ---
	       inserts a new key-value pair into the splay tree.
	       after insertion, the new node is splayed to the root,
	       optimizing future access to recently inserted elements.
	       
	       if the key already exists, the data is updated and
	       the node is splayed to the root. this ensures that
	       duplicate insertions don't create multiple nodes.

*/

commc_error_t commc_splay_tree_insert(
  commc_splay_tree_t*  tree,
  int                  key,
  void*                data
);

/*

         commc_splay_tree_delete()
	       ---
	       removes the node with the specified key from the tree.
	       the deletion process involves splaying the node to the
	       root, then joining its left and right subtrees.
	       
	       if the node is not found, no error is returned. the
	       splay operation still occurs on the last accessed node
	       during the search, maintaining the self-adjusting property.

*/

commc_error_t commc_splay_tree_delete(
  commc_splay_tree_t*  tree,
  int                  key
);

/*

         commc_splay_tree_search()
	       ---
	       searches for a node with the specified key in the tree.
	       regardless of whether the key is found, a splay operation
	       is performed on the last accessed node.
	       
	       if found, the node is splayed to the root and its data
	       is returned. if not found, the last accessed node during
	       the search is splayed, and null is returned.

*/

commc_error_t commc_splay_tree_search(
  commc_splay_tree_t*  tree,
  int                  key,
  void**               data
);

/*

         commc_splay_tree_min()
	       ---
	       finds and returns the minimum key in the tree.
	       the minimum node is splayed to the root after access,
	       optimizing future minimum queries.
	       
	       returns an error if the tree is empty. otherwise,
	       the minimum node becomes the root and its data is
	       returned through the output parameter.

*/

commc_error_t commc_splay_tree_min(
  commc_splay_tree_t*  tree,
  int*                 key,
  void**               data
);

/*

         commc_splay_tree_max()
	       ---
	       finds and returns the maximum key in the tree.
	       the maximum node is splayed to the root after access,
	       optimizing future maximum queries.
	       
	       returns an error if the tree is empty. otherwise,
	       the maximum node becomes the root and its data is
	       returned through the output parameter.

*/

commc_error_t commc_splay_tree_max(
  commc_splay_tree_t*  tree,
  int*                 key,
  void**               data
);

/*

         commc_splay_tree_size()
	       ---
	       returns the current number of nodes in the tree.
	       this operation does not modify the tree structure
	       or perform any splaying operations.

*/

size_t commc_splay_tree_size(
  const commc_splay_tree_t*  tree
);

/*

         commc_splay_tree_empty()
	       ---
	       checks whether the tree contains any nodes.
	       returns 1 if empty, 0 if non-empty. no splaying
	       operations are performed.

*/

int commc_splay_tree_empty(
  const commc_splay_tree_t*  tree
);

/*

         commc_splay_tree_traverse()
	       ---
	       performs an in-order traversal of the tree, calling
	       the visitor function for each node. the traversal
	       does not perform splaying operations to maintain
	       the tree structure during iteration.
	       
	       the visitor function receives the key, data, and
	       user context for each node. if the visitor returns
	       non-zero, the traversal stops early.

*/

commc_error_t commc_splay_tree_traverse(
  const commc_splay_tree_t*  tree,
  int  (*visitor)(int key, void* data, void* context),
  void*  context
);

#endif /* COMMC_SPLAY_TREE_H */

/*
	==================================
             --- EOF ---
	==================================
*/