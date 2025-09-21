/*
   ===================================
   C O M M O N - C
   AVL TREE IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- AVL TREE MODULE ---

    implementation of the self-balancing AVL tree.
    see include/commc/avl_tree.h for function
    prototypes and documentation.

    AVL trees maintain balance through rotations whenever
    the height difference between left and right subtrees
    exceeds 1. this guarantees O(log n) operations.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#include "commc/avl_tree.h"
#include "commc/error.h"
#include <stdlib.h>
#include <string.h>

/*
	==================================
             --- STRUCTS ---
	==================================
*/

/* internal node structure definition */

struct commc_avl_tree_node_t {

  void*  key;                                    /* key for comparison */
  void*  value;                                  /* user-provided data */

  struct commc_avl_tree_node_t*  left;           /* left child */
  struct commc_avl_tree_node_t*  right;          /* right child */

  int    height;                                 /* height of subtree rooted at this node */

};

/* internal tree structure */

struct commc_avl_tree_t {

  commc_avl_tree_node_t*     root;               /* root of the tree */
  size_t                     size;               /* number of nodes */
  commc_avl_compare_func     compare_func;       /* function to compare keys */

};

/*
	==================================
             --- STATIC FUNCS ---
	==================================
*/

/*

         commc_avl_node_create()
	       ---
	       creates a new AVL tree node with height 1 (leaf node).

*/

static commc_avl_tree_node_t* commc_avl_node_create(void* key, void* value) {

  commc_avl_tree_node_t* node;
  
  node = (commc_avl_tree_node_t*)malloc(sizeof(commc_avl_tree_node_t));

  if  (!node) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  node->key    = key;
  node->value  = value;
  node->left   = NULL;
  node->right  = NULL;
  node->height = 1;                              /* new nodes start at height 1 */

  return node;

}

/*

         commc_avl_node_destroy()
	       ---
	       recursively destroys a node and its children.

*/

static void commc_avl_node_destroy(commc_avl_tree_node_t* node) {

  if  (!node) {

    return;

  }

  commc_avl_node_destroy(node->left);
  commc_avl_node_destroy(node->right);
  free(node);

}

/*

         commc_avl_node_height()
	       ---
	       returns the height of a node. NULL nodes have height 0.
	       this helper function prevents null pointer dereferences.

*/

static int commc_avl_node_height(commc_avl_tree_node_t* node) {

  if  (!node) {

    return 0;

  }

  return node->height;

}

/*

         commc_avl_max()
	       ---
	       utility function to return the maximum of two integers.
	       used in height calculations.

*/

static int commc_avl_max(int a, int b) {

  return (a > b) ? a : b;

}

/*

         commc_avl_update_height()
	       ---
	       recalculates and updates the height of a node based
	       on the heights of its children. height is defined as
	       1 + max(left_height, right_height).

*/

static void commc_avl_update_height(commc_avl_tree_node_t* node) {

  if  (!node) {

    return;

  }

  node->height = 1 + commc_avl_max(commc_avl_node_height(node->left),
                                   commc_avl_node_height(node->right));

}

/*

         commc_avl_balance_factor()
	       ---
	       calculates the balance factor of a node, which is
	       left_height - right_height. in AVL trees:
	       - balance factor > 1: left-heavy
	       - balance factor < -1: right-heavy
	       - balance factor between -1 and 1: balanced

*/

static int commc_avl_balance_factor(commc_avl_tree_node_t* node) {

  if  (!node) {

    return 0;

  }

  return commc_avl_node_height(node->left) - commc_avl_node_height(node->right);

}

/*

         commc_avl_rotate_right()
	       ---
	       performs a right rotation on the given node.
	       
	       before rotation:    after rotation:
	           y                   x
	          / \                 / \
	         x   C               A   y
	        / \                     / \
	       A   B                   B   C
	       
	       this rotation corrects a left-heavy imbalance.

*/

static commc_avl_tree_node_t* commc_avl_rotate_right(commc_avl_tree_node_t* y) {

  commc_avl_tree_node_t* x;
  commc_avl_tree_node_t* B;

  if  (!y || !y->left) {

    return y;

  }

  x = y->left;
  B = x->right;

  /* perform rotation */

  x->right = y;
  y->left  = B;

  /* update heights (order matters: update y first, then x) */

  commc_avl_update_height(y);
  commc_avl_update_height(x);

  return x;                                      /* x is now the root of this subtree */

}

/*

         commc_avl_rotate_left()
	       ---
	       performs a left rotation on the given node.
	       
	       before rotation:    after rotation:
	           x                   y
	          / \                 / \
	         A   y               x   C
	            / \             / \
	           B   C           A   B
	       
	       this rotation corrects a right-heavy imbalance.

*/

static commc_avl_tree_node_t* commc_avl_rotate_left(commc_avl_tree_node_t* x) {

  commc_avl_tree_node_t* y;
  commc_avl_tree_node_t* B;

  if  (!x || !x->right) {

    return x;

  }

  y = x->right;
  B = y->left;

  /* perform rotation */

  y->left  = x;
  x->right = B;

  /* update heights (order matters: update x first, then y) */

  commc_avl_update_height(x);
  commc_avl_update_height(y);

  return y;                                      /* y is now the root of this subtree */

}

/*

         commc_avl_insert_recursive()
	       ---
	       recursive helper for inserting a node and rebalancing.
	       this is where the AVL magic happens - after each
	       insertion, we check balance factors and perform
	       rotations as needed.

*/

static commc_avl_tree_node_t* commc_avl_insert_recursive(commc_avl_tree_node_t* node,
                                                          void* key,
                                                          void* value,
                                                          commc_avl_compare_func compare_func,
                                                          size_t* tree_size) {

  int cmp;
  int balance;

  /* step 1: perform standard BST insertion */

  if  (!node) {

    (*tree_size)++;
    return commc_avl_node_create(key, value);

  }

  cmp = compare_func(key, node->key);

  if  (cmp < 0) {

    node->left = commc_avl_insert_recursive(node->left, key, value, compare_func, tree_size);

  } else if  (cmp > 0) {

    node->right = commc_avl_insert_recursive(node->right, key, value, compare_func, tree_size);

  } else {

    /* key already exists - update value */

    node->value = value;
    return node;

  }

  /* step 2: update height of current node */

  commc_avl_update_height(node);

  /* step 3: get balance factor and perform rotations if needed */

  balance = commc_avl_balance_factor(node);

  /* case 1: left-left heavy (single right rotation) */

  if  (balance > 1 && compare_func(key, node->left->key) < 0) {

    return commc_avl_rotate_right(node);

  }

  /* case 2: right-right heavy (single left rotation) */

  if  (balance < -1 && compare_func(key, node->right->key) > 0) {

    return commc_avl_rotate_left(node);

  }

  /* case 3: left-right heavy (double rotation: left then right) */

  if  (balance > 1 && compare_func(key, node->left->key) > 0) {

    node->left = commc_avl_rotate_left(node->left);
    return commc_avl_rotate_right(node);

  }

  /* case 4: right-left heavy (double rotation: right then left) */

  if  (balance < -1 && compare_func(key, node->right->key) < 0) {

    node->right = commc_avl_rotate_right(node->right);
    return commc_avl_rotate_left(node);

  }

  /* node is balanced - return unchanged */

  return node;

}

/*

         commc_avl_get_recursive()
	       ---
	       recursive helper for finding a value by key.

*/

static void* commc_avl_get_recursive(commc_avl_tree_node_t* node,
                                     const void* key,
                                     commc_avl_compare_func compare_func) {

  int cmp;

  if  (!node) {

    return NULL;

  }

  cmp = compare_func(key, node->key);

  if  (cmp < 0) {

    return commc_avl_get_recursive(node->left, key, compare_func);

  } else if  (cmp > 0) {

    return commc_avl_get_recursive(node->right, key, compare_func);

  } else {

    return node->value;

  }

}

/*

         commc_avl_min_node()
	       ---
	       finds the node with the smallest key in the subtree.
	       used in deletion operations.

*/

static commc_avl_tree_node_t* commc_avl_min_node(commc_avl_tree_node_t* node) {

  if  (!node) {

    return NULL;

  }

  while  (node->left) {

    node = node->left;

  }

  return node;

}

/*

         commc_avl_max_node()
	       ---
	       finds the node with the largest key in the subtree.

*/

static commc_avl_tree_node_t* commc_avl_max_node(commc_avl_tree_node_t* node) {

  if  (!node) {

    return NULL;

  }

  while  (node->right) {

    node = node->right;

  }

  return node;

}

/*

         commc_avl_remove_recursive()
	       ---
	       recursive helper for removing a node and rebalancing.
	       handles three cases: node with 0, 1, or 2 children.

*/

static commc_avl_tree_node_t* commc_avl_remove_recursive(commc_avl_tree_node_t* node,
                                                          const void* key,
                                                          commc_avl_compare_func compare_func,
                                                          size_t* tree_size,
                                                          int* found) {

  int cmp;
  int balance;
  commc_avl_tree_node_t* temp;

  if  (!node) {

    *found = 0;                                  /* key not found */
    return NULL;

  }

  cmp = compare_func(key, node->key);

  if  (cmp < 0) {

    node->left = commc_avl_remove_recursive(node->left, key, compare_func, tree_size, found);

  } else if  (cmp > 0) {

    node->right = commc_avl_remove_recursive(node->right, key, compare_func, tree_size, found);

  } else {

    /* found the node to delete */

    *found = 1;
    (*tree_size)--;

    /* case 1: node with only right child or no children */

    if  (!node->left) {

      temp = node->right;
      free(node);
      return temp;

    }

    /* case 2: node with only left child */

    else if  (!node->right) {

      temp = node->left;
      free(node);
      return temp;

    }

    /* case 3: node with two children */
    /* find inorder successor (smallest in right subtree) */

    temp = commc_avl_min_node(node->right);

    /* copy the inorder successor's data to this node */

    node->key   = temp->key;
    node->value = temp->value;

    /* delete the inorder successor recursively */
    /* note: size is already decremented once above */

    {
      int successor_found = 0;
      size_t temp_size = 0; /* use dummy size to prevent double decrement */
      node->right = commc_avl_remove_recursive(node->right, temp->key, compare_func, &temp_size, &successor_found);
    }

  }

  /* if tree had only one node, return */

  if  (!node) {

    return node;

  }

  /* update height of current node */

  commc_avl_update_height(node);

  /* get balance factor and perform rotations if needed */

  balance = commc_avl_balance_factor(node);

  /* case 1: left-left heavy */

  if  (balance > 1 && commc_avl_balance_factor(node->left) >= 0) {

    return commc_avl_rotate_right(node);

  }

  /* case 2: right-right heavy */

  if  (balance < -1 && commc_avl_balance_factor(node->right) <= 0) {

    return commc_avl_rotate_left(node);

  }

  /* case 3: left-right heavy */

  if  (balance > 1 && commc_avl_balance_factor(node->left) < 0) {

    node->left = commc_avl_rotate_left(node->left);
    return commc_avl_rotate_right(node);

  }

  /* case 4: right-left heavy */

  if  (balance < -1 && commc_avl_balance_factor(node->right) > 0) {

    node->right = commc_avl_rotate_right(node->right);
    return commc_avl_rotate_left(node);

  }

  return node;

}

/*
	==================================
             --- FUNCS ---
	==================================
*/

/*

         commc_avl_tree_create()
	       ---
	       allocates and initializes a new AVL tree.

*/

commc_avl_tree_t* commc_avl_tree_create(commc_avl_compare_func compare_func) {

  commc_avl_tree_t* tree;

  if  (!compare_func) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  tree = (commc_avl_tree_t*)malloc(sizeof(commc_avl_tree_t));

  if  (!tree) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  tree->root         = NULL;
  tree->size         = 0;
  tree->compare_func = compare_func;

  return tree;

}

/*

         commc_avl_tree_destroy()
	       ---
	       frees all nodes and the tree structure itself.

*/

void commc_avl_tree_destroy(commc_avl_tree_t* tree) {

  if  (!tree) {

    return;

  }

  commc_avl_node_destroy(tree->root);
  free(tree);

}

/*

         commc_avl_tree_insert()
	       ---
	       inserts a key-value pair with automatic rebalancing.

*/

commc_error_t commc_avl_tree_insert(commc_avl_tree_t* tree, void* key, void* value) {

  if  (!tree) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;

  }

  tree->root = commc_avl_insert_recursive(tree->root, key, value, tree->compare_func, &tree->size);

  return tree->root ? COMMC_SUCCESS : COMMC_MEMORY_ERROR;

}

/*

         commc_avl_tree_get()
	       ---
	       retrieves the value for a given key.

*/

void* commc_avl_tree_get(commc_avl_tree_t* tree, const void* key) {

  if  (!tree) {

    return NULL;

  }

  return commc_avl_get_recursive(tree->root, key, tree->compare_func);

}

/*

         commc_avl_tree_remove()
	       ---
	       removes a key-value pair with automatic rebalancing.

*/

commc_error_t commc_avl_tree_remove(commc_avl_tree_t* tree, const void* key) {

  int found = 0;

  if  (!tree) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;

  }

  tree->root = commc_avl_remove_recursive(tree->root, key, tree->compare_func, &tree->size, &found);

  return found ? COMMC_SUCCESS : COMMC_ARGUMENT_ERROR;

}

/*

         commc_avl_tree_size()
	       ---
	       returns the number of nodes in the tree.

*/

size_t commc_avl_tree_size(commc_avl_tree_t* tree) {

  if  (!tree) {

    return 0;

  }

  return tree->size;

}

/*

         commc_avl_tree_height()
	       ---
	       returns the height of the tree.

*/

size_t commc_avl_tree_height(commc_avl_tree_t* tree) {

  if  (!tree) {

    return 0;

  }

  return (size_t)commc_avl_node_height(tree->root);

}

/*

         commc_avl_tree_clear()
	       ---
	       removes all nodes from the tree.

*/

void commc_avl_tree_clear(commc_avl_tree_t* tree) {

  if  (!tree) {

    return;

  }

  commc_avl_node_destroy(tree->root);
  tree->root = NULL;
  tree->size = 0;

}

/*

         commc_avl_tree_contains()
	       ---
	       checks if the tree contains a key.

*/

int commc_avl_tree_contains(commc_avl_tree_t* tree, const void* key) {

  return commc_avl_tree_get(tree, key) != NULL;

}

/*

         commc_avl_tree_min_key()
	       ---
	       returns the smallest key in the tree.

*/

void* commc_avl_tree_min_key(commc_avl_tree_t* tree) {

  commc_avl_tree_node_t* min_node;

  if  (!tree) {

    return NULL;

  }

  min_node = commc_avl_min_node(tree->root);

  return min_node ? min_node->key : NULL;

}

/*

         commc_avl_tree_max_key()
	       ---
	       returns the largest key in the tree.

*/

void* commc_avl_tree_max_key(commc_avl_tree_t* tree) {

  commc_avl_tree_node_t* max_node;

  if  (!tree) {

    return NULL;

  }

  max_node = commc_avl_max_node(tree->root);

  return max_node ? max_node->key : NULL;

}

/*
	==================================
             --- EOF ---
	==================================
*/