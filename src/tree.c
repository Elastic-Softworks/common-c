/*
   ===================================
   C O M M O N - C
   BINARY SEARCH TREE IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- TREE MODULE ---

    implementation of the binary search tree.
    see include/commc/tree.h for function
    prototypes and documentation.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#include "commc/tree.h"
#include <stdlib.h>
#include <string.h>

/*
	==================================
             --- STRUCTS ---
	==================================
*/

/* internal node structure. */

typedef struct commc_tree_node_t {

  void*  key;                             /* key for comparison */
  void*  value;                           /* user-provided data */

  struct commc_tree_node_t*  left;        /* left child */
  struct commc_tree_node_t*  right;       /* right child */

} commc_tree_node_t;

/* internal tree structure. */

struct commc_tree_t {

  commc_tree_node_t*        root;          /* root of the tree */
  size_t                    size;          /* number of nodes */
  commc_tree_compare_func   compare_func;  /* function to compare keys */

};

/*
	==================================
             --- STATIC FUNCS ---
	==================================
*/

/*

         commc_tree_node_create()
	       ---
	       creates a new tree node.

*/

static commc_tree_node_t* commc_tree_node_create(void* key, void* value) {

  commc_tree_node_t* node;
  
  node = (commc_tree_node_t*)malloc(sizeof(commc_tree_node_t));

  if  (!node) {

    return NULL;

  }

  node->key   = key;
  node->value = value;
  node->left  = NULL;
  node->right = NULL;

  return node;

}

/*

         commc_tree_node_destroy()
	       ---
	       recursively destroys a node and its children.

*/

static void commc_tree_node_destroy(commc_tree_node_t* node) {

  if  (!node) {

    return;

  }

  commc_tree_node_destroy(node->left);
  commc_tree_node_destroy(node->right);
  free(node);

}

/*

         commc_tree_insert_recursive()
	       ---
	       recursive helper for inserting a node.

*/

static commc_tree_node_t* commc_tree_insert_recursive(commc_tree_node_t* node,
                                                      void* key,
                                                      void* value,
                                                      commc_tree_compare_func compare_func) {

  int cmp;

  if  (!node) {

    return commc_tree_node_create(key, value);

  }

  cmp = compare_func(key, node->key);

  if  (cmp < 0) {

    node->left = commc_tree_insert_recursive(node->left, key, value, compare_func);

  } else if  (cmp > 0) {

    node->right = commc_tree_insert_recursive(node->right, key, value, compare_func);

  } else {

    /* update value if key exists */

    node->value = value;

  }

  return node;

}

/*

         commc_tree_get_recursive()
	       ---
	       recursive helper for getting a node's value.

*/

static void* commc_tree_get_recursive(commc_tree_node_t* node,
                                     const void* key,
                                     commc_tree_compare_func compare_func) {

  int cmp;

  if  (!node) {

    return NULL;

  }

  cmp = compare_func(key, node->key);

  if  (cmp < 0) {

    return commc_tree_get_recursive(node->left, key, compare_func);

  } else if  (cmp > 0) {

    return commc_tree_get_recursive(node->right, key, compare_func);

  } else {

    return node->value;

  }

}

/*

         commc_tree_min_node()
	       ---
	       finds the node with the minimum key in a subtree.

*/

static commc_tree_node_t* commc_tree_min_node(commc_tree_node_t* node) {

  commc_tree_node_t* current = node;

  while  (current && current->left) {

    current = current->left;

  }

  return current;

}

/*

         commc_tree_remove_recursive()
	       ---
	       recursive helper for removing a node.

*/

static commc_tree_node_t* commc_tree_remove_recursive(commc_tree_node_t* node,
                                                      const void* key,
                                                      commc_tree_compare_func compare_func) {

  int cmp;

  if  (!node) {

    return NULL;

  }

  cmp = compare_func(key, node->key);

  if  (cmp < 0) {

    node->left = commc_tree_remove_recursive(node->left, key, compare_func);

  } else if  (cmp > 0) {

    node->right = commc_tree_remove_recursive(node->right, key, compare_func);

  } else {

    /* node to be deleted found */
    if  (node->left == NULL) {

      commc_tree_node_t* temp = node->right;
      free(node);
      return temp;

    } else if  (node->right == NULL) {

      commc_tree_node_t* temp = node->left;
      free(node);
      return temp;

    }

    /* node with two children: get inorder successor */
    commc_tree_node_t* temp = commc_tree_min_node(node->right);

    node->key   = temp->key;
    node->value = temp->value;
    node->right = commc_tree_remove_recursive(node->right, temp->key, compare_func);

  }

  return node;

}

/*
	==================================
             --- FUNCS ---
	==================================
*/

/*

         commc_tree_create()
	       ---
	       allocates and initializes a new tree.

*/

commc_tree_t* commc_tree_create(commc_tree_compare_func compare_func) {

  commc_tree_t* tree;
  
  tree = (commc_tree_t*)malloc(sizeof(commc_tree_t));

  if  (!tree) {

    return NULL;

  }

  tree->root         = NULL;
  tree->size         = 0;
  tree->compare_func = compare_func;

  return tree;

}

/*

         commc_tree_destroy()
	       ---
	       frees all nodes in the tree.

*/

void commc_tree_destroy(commc_tree_t* tree) {

  if  (tree) {

    commc_tree_node_destroy(tree->root);
    free(tree);

  }

}

/*

         commc_tree_insert()
	       ---
	       inserts a key-value pair. increments size if new.

*/

int commc_tree_insert(commc_tree_t* tree, void* key, void* value) {

  size_t old_size;

  if  (!tree || !tree->compare_func) {

    return 0;

  }

  old_size   = tree->size;
  tree->root = commc_tree_insert_recursive(tree->root, key, value, tree->compare_func);

  if  (tree->root && tree->size == old_size) {
    /* if root was updated but size didn't change, it was an update */
    return 1;
  } else if  (tree->root) {
    tree->size++;
    return 1;
  }

  return 0;

}

/*

         commc_tree_get()
	       ---
	       retrieves the value for a given key.

*/

void* commc_tree_get(commc_tree_t* tree, const void* key) {

  if  (!tree || !tree->compare_func) {

    return NULL;

  }

  return commc_tree_get_recursive(tree->root, key, tree->compare_func);

}

/*

         commc_tree_remove()
	       ---
	       removes a key-value pair. decrements size if found.

*/

void commc_tree_remove(commc_tree_t* tree, const void* key) {

  size_t old_size;

  if  (!tree || !tree->compare_func) {

    return;

  }

  old_size   = tree->size;
  tree->root = commc_tree_remove_recursive(tree->root, key, tree->compare_func);

  if  (tree->root || tree->size != old_size) {
    /* if root was updated or size changed, it was a removal */
    tree->size--;
  }

}

/*

         commc_tree_size()
	       ---
	       returns the number of elements.

*/

size_t commc_tree_size(commc_tree_t* tree) {

  return tree ? tree->size : 0;

}

/*

         commc_tree_is_empty()
	       ---
	       checks if the tree is empty.

*/

int commc_tree_is_empty(commc_tree_t* tree) {

  return tree ? (tree->size == 0) : 1;

}

/*
	==================================
             --- EOF ---
	==================================
*/
