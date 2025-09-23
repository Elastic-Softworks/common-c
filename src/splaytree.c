/*
   ===================================
   C O M M O N - C
   SPLAY TREE IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- SPLAY TREE MODULE ---

    implementation of the self-adjusting splay tree.
    see include/commc/splay_tree.h for function
    prototypes and documentation.

    splay trees achieve excellent amortized performance
    through the splaying operation, which moves accessed
    nodes to the root via rotations. this provides
    optimal performance for access patterns with temporal
    locality.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#include "commc/splaytree.h"
#include "commc/error.h"
#include <stdlib.h>
#include <string.h>

/*
	==================================
             --- STATIC FUNCS ---
	==================================
*/

/*

         commc_splay_node_create()
	       ---
	       creates a new splay tree node with the specified key
	       and data. the node is initialized with null pointers
	       for all child and parent relationships.

*/

static commc_splay_node_t* commc_splay_node_create(int key, void* data) {

  commc_splay_node_t* node;

  node = (commc_splay_node_t*)malloc(sizeof(commc_splay_node_t));

  if  (!node) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  node->key    = key;
  node->data   = data;
  node->left   = NULL;
  node->right  = NULL;
  node->parent = NULL;

  return node;

}

/*

         commc_splay_node_destroy()
	       ---
	       recursively destroys a splay tree node and all its
	       descendants. calls the destroy_data function if
	       provided to clean up user data.

*/

static void commc_splay_node_destroy(
  commc_splay_node_t*  node,
  void (*destroy_data)(void* data)
) {

  if  (!node) {

    return;

  }

  /* recursively destroy children */

  commc_splay_node_destroy(node->left, destroy_data);
  commc_splay_node_destroy(node->right, destroy_data);

  /* clean up user data if function provided */

  if  (destroy_data && node->data) {

    destroy_data(node->data);

  }

  free(node);

}

/*

         commc_splay_default_compare()
	       ---
	       default integer comparison function used when no
	       custom comparison function is provided. returns
	       negative for a < b, zero for a == b, positive for a > b.

*/

static int commc_splay_default_compare(int a, int b) {

  if  (a < b) {

    return -1;

  }

  if  (a > b) {

    return 1;

  }

  return 0;

}

/*

         commc_splay_rotate_left()
	       ---
	       performs a left rotation around the given node.
	       this is a fundamental operation used during splaying
	       to move nodes up the tree toward the root.
	       
	       the rotation preserves the binary search tree property
	       while changing the tree structure to improve balance
	       for the access pattern.

*/

static void commc_splay_rotate_left(commc_splay_tree_t* tree, commc_splay_node_t* x) {

  commc_splay_node_t* y;

  if  (!x || !x->right) {

    return;

  }

  y = x->right;

  /* update x's right child to y's left subtree */

  x->right = y->left;

  if  (y->left) {

    y->left->parent = x;

  }

  /* update y's parent to x's parent */

  y->parent = x->parent;

  if  (!x->parent) {

    tree->root = y;

  } else if  (x == x->parent->left) {

    x->parent->left = y;

  } else {

    x->parent->right = y;

  }

  /* make x the left child of y */

  y->left   = x;
  x->parent = y;

}

/*

         commc_splay_rotate_right()
	       ---
	       performs a right rotation around the given node.
	       this is the symmetric operation to left rotation,
	       used during splaying to restructure the tree.
	       
	       right rotations promote the left child of a node
	       to take its position, moving the original node
	       down and to the right.

*/

static void commc_splay_rotate_right(commc_splay_tree_t* tree, commc_splay_node_t* y) {

  commc_splay_node_t* x;

  if  (!y || !y->left) {

    return;

  }

  x = y->left;

  /* update y's left child to x's right subtree */

  y->left = x->right;

  if  (x->right) {

    x->right->parent = y;

  }

  /* update x's parent to y's parent */

  x->parent = y->parent;

  if  (!y->parent) {

    tree->root = x;

  } else if  (y == y->parent->left) {

    y->parent->left = x;

  } else {

    y->parent->right = x;

  }

  /* make y the right child of x */

  x->right  = y;
  y->parent = x;

}

/*

         commc_splay()
	       ---
	       performs the splay operation to move the specified node
	       to the root of the tree. this is the core operation
	       that gives splay trees their self-adjusting property.
	       
	       the splaying process uses three types of operations:
	       - zig: single rotation when node is child of root
	       - zig-zag: double rotation for opposite directions
	       - zig-zig: double rotation for same directions
	       
	       after splaying, the accessed node becomes the root,
	       improving future access times for recently used elements.

*/

static void commc_splay(commc_splay_tree_t* tree, commc_splay_node_t* node) {

  if  (!node) {

    return;

  }

  /* continue splaying until node reaches the root */

  while  (node->parent) {

    /* case 1: zig (node is child of root) */

    if  (!node->parent->parent) {

      if  (node == node->parent->left) {

        commc_splay_rotate_right(tree, node->parent);

      } else {

        commc_splay_rotate_left(tree, node->parent);

      }

    /* case 2: zig-zig (node and parent have same direction) */

    } else if  (node == node->parent->left && node->parent == node->parent->parent->left) {

      commc_splay_rotate_right(tree, node->parent->parent);
      commc_splay_rotate_right(tree, node->parent);

    } else if  (node == node->parent->right && node->parent == node->parent->parent->right) {

      commc_splay_rotate_left(tree, node->parent->parent);
      commc_splay_rotate_left(tree, node->parent);

    /* case 3: zig-zag (node and parent have opposite directions) */

    } else if  (node == node->parent->left && node->parent == node->parent->parent->right) {

      commc_splay_rotate_right(tree, node->parent);
      commc_splay_rotate_left(tree, node->parent);

    } else {

      commc_splay_rotate_left(tree, node->parent);
      commc_splay_rotate_right(tree, node->parent);

    }

  }

}

/*

         commc_splay_find_node()
	       ---
	       searches for a node with the specified key in the tree.
	       returns the node if found, or the last accessed node
	       during the search if not found.
	       
	       this function does not perform splaying - that is done
	       by the caller based on whether they want to splay the
	       found node or the last accessed node.

*/

static commc_splay_node_t* commc_splay_find_node(
  commc_splay_tree_t*  tree,
  int                  key,
  commc_splay_node_t** last_accessed
) {

  commc_splay_node_t*  current;
  commc_splay_node_t*  last;
  int                  cmp_result;

  current = tree->root;
  last    = NULL;

  while  (current) {

    last       = current;
    cmp_result = tree->compare(key, current->key);

    if  (cmp_result == 0) {

      *last_accessed = current;
      return current;

    } else if  (cmp_result < 0) {

      current = current->left;

    } else {

      current = current->right;

    }

  }

  *last_accessed = last;
  return NULL;

}

/*

         commc_splay_join()
	       ---
	       joins two splay trees where all keys in the left tree
	       are smaller than all keys in the right tree. this is
	       used during deletion operations.
	       
	       the join operation finds the maximum node in the left
	       tree, splays it to the root, then attaches the right
	       tree as its right subtree.

*/

static commc_splay_node_t* commc_splay_join(
  commc_splay_tree_t*  tree,
  commc_splay_node_t*  left_root,
  commc_splay_node_t*  right_root
) {

  commc_splay_node_t* max_node;

  if  (!left_root) {

    return right_root;

  }

  if  (!right_root) {

    return left_root;

  }

  /* find maximum node in left subtree */

  max_node = left_root;

  while  (max_node->right) {

    max_node = max_node->right;

  }

  /* temporarily set up tree structure for splaying */

  tree->root = left_root;

  if  (left_root) {

    left_root->parent = NULL;

  }

  /* splay the maximum node to become root */

  commc_splay(tree, max_node);

  /* attach right subtree */

  max_node->right = right_root;

  if  (right_root) {

    right_root->parent = max_node;

  }

  return max_node;

}

/*

         commc_splay_traverse_helper()
	       ---
	       recursive helper function for in-order tree traversal.
	       calls the visitor function for each node in sorted order.

*/

static int commc_splay_traverse_helper(
  const commc_splay_node_t*  node,
  int  (*visitor)(int key, void* data, void* context),
  void*  context
) {

  int result;

  if  (!node) {

    return 0;

  }

  /* traverse left subtree */

  result = commc_splay_traverse_helper(node->left, visitor, context);

  if  (result != 0) {

    return result;

  }

  /* visit current node */

  result = visitor(node->key, node->data, context);

  if  (result != 0) {

    return result;

  }

  /* traverse right subtree */

  return commc_splay_traverse_helper(node->right, visitor, context);

}

/*
	==================================
             --- PUBLIC FUNCS ---
	==================================
*/

/*

         commc_splay_tree_create()
	       ---
	       creates a new splay tree with the specified comparison
	       and data destruction functions. see header for details.

*/

commc_error_t commc_splay_tree_create(
  commc_splay_tree_t**  tree,
  int   (*compare)(int a, int b),
  void  (*destroy_data)(void* data)
) {

  commc_splay_tree_t* new_tree;

  if  (!tree) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;

  }

  new_tree = (commc_splay_tree_t*)malloc(sizeof(commc_splay_tree_t));

  if  (!new_tree) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return COMMC_MEMORY_ERROR;

  }

  new_tree->root         = NULL;
  new_tree->size         = 0;
  new_tree->compare      = compare ? compare : commc_splay_default_compare;
  new_tree->destroy_data = destroy_data;

  *tree = new_tree;

  return COMMC_SUCCESS;

}

/*

         commc_splay_tree_destroy()
	       ---
	       destroys the splay tree and frees all memory.
	       see header for details.

*/

commc_error_t commc_splay_tree_destroy(
  commc_splay_tree_t**  tree
) {

  if  (!tree || !*tree) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;

  }

  commc_splay_node_destroy((*tree)->root, (*tree)->destroy_data);

  free(*tree);
  *tree = NULL;

  return COMMC_SUCCESS;

}

/*

         commc_splay_tree_insert()
	       ---
	       inserts a key-value pair and splays the node to root.
	       see header for details.

*/

commc_error_t commc_splay_tree_insert(
  commc_splay_tree_t*  tree,
  int                  key,
  void*                data
) {

  commc_splay_node_t*  new_node;
  commc_splay_node_t*  parent;
  commc_splay_node_t*  current;
  int                  cmp_result;

  if  (!tree) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;

  }

  /* handle empty tree */

  if  (!tree->root) {

    new_node = commc_splay_node_create(key, data);

    if  (!new_node) {

      return COMMC_MEMORY_ERROR;

    }

    tree->root = new_node;
    tree->size = 1;

    return COMMC_SUCCESS;

  }

  /* find insertion point */

  current = tree->root;
  parent  = NULL;

  while  (current) {

    parent     = current;
    cmp_result = tree->compare(key, current->key);

    if  (cmp_result == 0) {

      /* key exists, update data and splay */

      if  (tree->destroy_data && current->data) {

        tree->destroy_data(current->data);

      }

      current->data = data;
      commc_splay(tree, current);

      return COMMC_SUCCESS;

    } else if  (cmp_result < 0) {

      current = current->left;

    } else {

      current = current->right;

    }

  }

  /* create new node */

  new_node = commc_splay_node_create(key, data);

  if  (!new_node) {

    return COMMC_MEMORY_ERROR;

  }

  /* link to parent */

  new_node->parent = parent;
  cmp_result       = tree->compare(key, parent->key);

  if  (cmp_result < 0) {

    parent->left = new_node;

  } else {

    parent->right = new_node;

  }

  /* splay new node to root */

  commc_splay(tree, new_node);
  tree->size++;

  return COMMC_SUCCESS;

}

/*

         commc_splay_tree_delete()
	       ---
	       deletes a node with the specified key and restructures
	       the tree. see header for details.

*/

commc_error_t commc_splay_tree_delete(
  commc_splay_tree_t*  tree,
  int                  key
) {

  commc_splay_node_t*  node;
  commc_splay_node_t*  last_accessed;
  commc_splay_node_t*  left_subtree;
  commc_splay_node_t*  right_subtree;

  if  (!tree) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;

  }

  if  (!tree->root) {

    return COMMC_SUCCESS;

  }

  /* find the node to delete */

  node = commc_splay_find_node(tree, key, &last_accessed);

  /* splay the found node or last accessed node */

  if  (node) {

    commc_splay(tree, node);

  } else if  (last_accessed) {

    commc_splay(tree, last_accessed);
    return COMMC_SUCCESS;

  } else {

    return COMMC_SUCCESS;

  }

  /* node is now at root, remove it */

  left_subtree  = tree->root->left;
  right_subtree = tree->root->right;

  /* clean up data if function provided */

  if  (tree->destroy_data && tree->root->data) {

    tree->destroy_data(tree->root->data);

  }

  free(tree->root);
  tree->size--;

  /* join the subtrees */

  if  (left_subtree) {

    left_subtree->parent = NULL;

  }

  if  (right_subtree) {

    right_subtree->parent = NULL;

  }

  tree->root = commc_splay_join(tree, left_subtree, right_subtree);

  if  (tree->root) {

    tree->root->parent = NULL;

  }

  return COMMC_SUCCESS;

}

/*

         commc_splay_tree_search()
	       ---
	       searches for a key and splays the accessed node.
	       see header for details.

*/

commc_error_t commc_splay_tree_search(
  commc_splay_tree_t*  tree,
  int                  key,
  void**               data
) {

  commc_splay_node_t*  node;
  commc_splay_node_t*  last_accessed;

  if  (!tree || !data) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;

  }

  *data = NULL;

  if  (!tree->root) {

    return COMMC_SUCCESS;

  }

  /* find the node */

  node = commc_splay_find_node(tree, key, &last_accessed);

  /* splay the found node or last accessed node */

  if  (node) {

    commc_splay(tree, node);
    *data = node->data;

  } else if  (last_accessed) {

    commc_splay(tree, last_accessed);

  }

  return COMMC_SUCCESS;

}

/*

         commc_splay_tree_min()
	       ---
	       finds the minimum key and splays it to root.
	       see header for details.

*/

commc_error_t commc_splay_tree_min(
  commc_splay_tree_t*  tree,
  int*                 key,
  void**               data
) {

  commc_splay_node_t*  min_node;

  if  (!tree || !key || !data) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;

  }

  if  (!tree->root) {

    commc_report_error(COMMC_FAILURE, __FILE__, __LINE__);
    return COMMC_FAILURE;

  }

  /* find minimum node */

  min_node = tree->root;

  while  (min_node->left) {

    min_node = min_node->left;

  }

  /* splay minimum to root */

  commc_splay(tree, min_node);

  *key  = min_node->key;
  *data = min_node->data;

  return COMMC_SUCCESS;

}

/*

         commc_splay_tree_max()
	       ---
	       finds the maximum key and splays it to root.
	       see header for details.

*/

commc_error_t commc_splay_tree_max(
  commc_splay_tree_t*  tree,
  int*                 key,
  void**               data
) {

  commc_splay_node_t*  max_node;

  if  (!tree || !key || !data) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;

  }

  if  (!tree->root) {

    commc_report_error(COMMC_FAILURE, __FILE__, __LINE__);
    return COMMC_FAILURE;

  }

  /* find maximum node */

  max_node = tree->root;

  while  (max_node->right) {

    max_node = max_node->right;

  }

  /* splay maximum to root */

  commc_splay(tree, max_node);

  *key  = max_node->key;
  *data = max_node->data;

  return COMMC_SUCCESS;

}

/*

         commc_splay_tree_size()
	       ---
	       returns the number of nodes in the tree.
	       see header for details.

*/

size_t commc_splay_tree_size(
  const commc_splay_tree_t*  tree
) {

  if  (!tree) {

    return 0;

  }

  return tree->size;

}

/*

         commc_splay_tree_empty()
	       ---
	       checks if the tree is empty.
	       see header for details.

*/

int commc_splay_tree_empty(
  const commc_splay_tree_t*  tree
) {

  if  (!tree) {

    return 1;

  }

  return (tree->size == 0) ? 1 : 0;

}

/*

         commc_splay_tree_traverse()
	       ---
	       performs in-order traversal of the tree.
	       see header for details.

*/

commc_error_t commc_splay_tree_traverse(
  const commc_splay_tree_t*  tree,
  int  (*visitor)(int key, void* data, void* context),
  void*  context
) {

  if  (!tree || !visitor) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;

  }

  commc_splay_traverse_helper(tree->root, visitor, context);

  return COMMC_SUCCESS;

}

/*
	==================================
             --- EOF ---
	==================================
*/