/*
   ===================================
   C O M M O N - C
   RED-BLACK TREE IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- RED-BLACK TREE MODULE ---

    implementation of the self-balancing Red-Black tree.
    see include/commc/rb_tree.h for function
    prototypes and documentation.

    Red-Black trees maintain balance through color properties
    and structural rules rather than explicit height tracking.
    this makes them slightly more complex but often more
    efficient in practice than AVL trees.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#include "commc/rb_tree.h"
#include "commc/error.h"
#include <stdlib.h>
#include <string.h>

/*
	==================================
             --- STRUCTS ---
	==================================
*/

/* internal node structure definition */

struct commc_rb_tree_node_t {

  void*                        key;              /* key for comparison */
  void*                        value;            /* user-provided data */

  struct commc_rb_tree_node_t* left;             /* left child */
  struct commc_rb_tree_node_t* right;            /* right child */
  struct commc_rb_tree_node_t* parent;           /* parent node (needed for RB operations) */

  commc_rb_color_t             color;            /* red or black color */

};

/* internal tree structure */

struct commc_rb_tree_t {

  commc_rb_tree_node_t*     root;                /* root of the tree */
  commc_rb_tree_node_t*     nil;                 /* sentinel NIL node */
  size_t                    size;                /* number of nodes */
  commc_rb_compare_func     compare_func;        /* function to compare keys */

};

/*
	==================================
             --- STATIC FUNCS ---
	==================================
*/

/*

         commc_rb_node_create()
	       ---
	       creates a new Red-Black tree node. new nodes start
	       as red to minimize black-height violations.

*/

static commc_rb_tree_node_t* commc_rb_node_create(commc_rb_tree_t* tree, void* key, void* value) {

  commc_rb_tree_node_t* node;
  
  node = (commc_rb_tree_node_t*)malloc(sizeof(commc_rb_tree_node_t));

  if  (!node) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  node->key    = key;
  node->value  = value;
  node->left   = tree->nil;                      /* children point to sentinel NIL */
  node->right  = tree->nil;
  node->parent = tree->nil;
  node->color  = COMMC_RB_RED;                   /* new nodes are red initially */

  return node;

}

/*

         commc_rb_node_destroy_recursive()
	       ---
	       recursively destroys a node and its children.
	       skips the sentinel NIL node.

*/

static void commc_rb_node_destroy_recursive(commc_rb_tree_t* tree, commc_rb_tree_node_t* node) {

  if  (!node || node == tree->nil) {

    return;

  }

  commc_rb_node_destroy_recursive(tree, node->left);
  commc_rb_node_destroy_recursive(tree, node->right);
  free(node);

}

/*

         commc_rb_rotate_left()
	       ---
	       performs a left rotation on the given node.
	       
	       before rotation:    after rotation:
	           x                   y
	          / \                 / \
	         A   y               x   C
	            / \             / \
	           B   C           A   B
	       
	       this rotation is used during rebalancing operations.

*/

static void commc_rb_rotate_left(commc_rb_tree_t* tree, commc_rb_tree_node_t* x) {

  commc_rb_tree_node_t* y;

  if  (!x || x->right == tree->nil) {

    return;

  }

  /* set y */

  y = x->right;

  /* turn y's left subtree into x's right subtree */

  x->right = y->left;

  if  (y->left != tree->nil) {

    y->left->parent = x;

  }

  /* link x's parent to y */

  y->parent = x->parent;

  if  (x->parent == tree->nil) {

    tree->root = y;                              /* y becomes new root */

  } else if  (x == x->parent->left) {

    x->parent->left = y;

  } else {

    x->parent->right = y;

  }

  /* put x on y's left */

  y->left = x;
  x->parent = y;

}

/*

         commc_rb_rotate_right()
	       ---
	       performs a right rotation on the given node.
	       
	       before rotation:    after rotation:
	           y                   x
	          / \                 / \
	         x   C               A   y
	        / \                     / \
	       A   B                   B   C
	       
	       this rotation is used during rebalancing operations.

*/

static void commc_rb_rotate_right(commc_rb_tree_t* tree, commc_rb_tree_node_t* y) {

  commc_rb_tree_node_t* x;

  if  (!y || y->left == tree->nil) {

    return;

  }

  x = y->left;                                   /* set x */

  /* turn x's right subtree into y's left subtree */

  y->left = x->right;

  if  (x->right != tree->nil) {

    x->right->parent = y;

  }

  /* link y's parent to x */

  x->parent = y->parent;

  if  (y->parent == tree->nil) {

    tree->root = x;                              /* x becomes new root */

  } else if  (y == y->parent->left) {

    y->parent->left = x;

  } else {

    y->parent->right = x;

  }

  /* put y on x's right */

  x->right = y;
  y->parent = x;

}

/*

         commc_rb_insert_fixup()
	       ---
	       restores Red-Black properties after insertion.
	       this is where the Red-Black magic happens - we fix
	       violations of the red-red rule through rotations
	       and recoloring.

*/

static void commc_rb_insert_fixup(commc_rb_tree_t* tree, commc_rb_tree_node_t* z) {

  commc_rb_tree_node_t* uncle;

  /* while parent is red (violates red-red property) */

  while  (z->parent->color == COMMC_RB_RED) {

    /* case: parent is left child of grandparent */

    if  (z->parent == z->parent->parent->left) {

      uncle = z->parent->parent->right;          /* uncle is right child */

      /* case 1: uncle is red - recolor and move up */

      if  (uncle->color == COMMC_RB_RED) {

        z->parent->color = COMMC_RB_BLACK;
        uncle->color = COMMC_RB_BLACK;
        z->parent->parent->color = COMMC_RB_RED;
        z = z->parent->parent;                   /* move up tree */

      } else {

        /* case 2: uncle is black, z is right child - left rotate */

        if  (z == z->parent->right) {

          z = z->parent;
          commc_rb_rotate_left(tree, z);

        }

        /* case 3: uncle is black, z is left child - recolor and right rotate */

        z->parent->color = COMMC_RB_BLACK;
        z->parent->parent->color = COMMC_RB_RED;
        commc_rb_rotate_right(tree, z->parent->parent);

      }

    } else {

      /* symmetric case: parent is right child of grandparent */

      uncle = z->parent->parent->left;           /* uncle is left child */

      /* case 1: uncle is red - recolor and move up */

      if  (uncle->color == COMMC_RB_RED) {

        z->parent->color = COMMC_RB_BLACK;
        uncle->color = COMMC_RB_BLACK;
        z->parent->parent->color = COMMC_RB_RED;
        z = z->parent->parent;                   /* move up tree */

      } else {

        /* case 2: uncle is black, z is left child - right rotate */

        if  (z == z->parent->left) {

          z = z->parent;
          commc_rb_rotate_right(tree, z);

        }

        /* case 3: uncle is black, z is right child - recolor and left rotate */

        z->parent->color = COMMC_RB_BLACK;
        z->parent->parent->color = COMMC_RB_RED;
        commc_rb_rotate_left(tree, z->parent->parent);

      }

    }

  }

  /* root must always be black */

  tree->root->color = COMMC_RB_BLACK;

}

/*

         commc_rb_transplant()
	       ---
	       replaces subtree rooted at node u with subtree
	       rooted at node v. used during deletion operations.

*/

static void commc_rb_transplant(commc_rb_tree_t* tree, commc_rb_tree_node_t* u, commc_rb_tree_node_t* v) {

  if  (u->parent == tree->nil) {

    tree->root = v;

  } else if  (u == u->parent->left) {

    u->parent->left = v;

  } else {

    u->parent->right = v;

  }

  v->parent = u->parent;

}

/*

         commc_rb_min_node()
	       ---
	       finds the node with the smallest key in the subtree.

*/

static commc_rb_tree_node_t* commc_rb_min_node(commc_rb_tree_t* tree, commc_rb_tree_node_t* node) {

  if  (!node || node == tree->nil) {

    return tree->nil;

  }

  while  (node->left != tree->nil) {

    node = node->left;

  }

  return node;

}

/*

         commc_rb_max_node()
	       ---
	       finds the node with the largest key in the subtree.

*/

static commc_rb_tree_node_t* commc_rb_max_node(commc_rb_tree_t* tree, commc_rb_tree_node_t* node) {

  if  (!node || node == tree->nil) {

    return tree->nil;

  }

  while  (node->right != tree->nil) {

    node = node->right;

  }

  return node;

}

/*

         commc_rb_delete_fixup()
	       ---
	       restores Red-Black properties after deletion.
	       this is the most complex part of Red-Black tree
	       operations, handling various cases of black-height
	       violations through rotations and recoloring.

*/

static void commc_rb_delete_fixup(commc_rb_tree_t* tree, commc_rb_tree_node_t* x) {

  commc_rb_tree_node_t* sibling;

  /* continue until x is root or x is red */

  while  (x != tree->root && x->color == COMMC_RB_BLACK) {

    /* case: x is left child */

    if  (x == x->parent->left) {

      sibling = x->parent->right;

      /* case 1: sibling is red - recolor and rotate */

      if  (sibling->color == COMMC_RB_RED) {

        sibling->color = COMMC_RB_BLACK;
        x->parent->color = COMMC_RB_RED;
        commc_rb_rotate_left(tree, x->parent);
        sibling = x->parent->right;

      }

      /* case 2: sibling is black with black children - recolor and move up */

      if  (sibling->left->color == COMMC_RB_BLACK && sibling->right->color == COMMC_RB_BLACK) {

        sibling->color = COMMC_RB_RED;
        x = x->parent;

      } else {

        /* case 3: sibling is black, left child is red, right child is black */

        if  (sibling->right->color == COMMC_RB_BLACK) {

          sibling->left->color = COMMC_RB_BLACK;
          sibling->color = COMMC_RB_RED;
          commc_rb_rotate_right(tree, sibling);
          sibling = x->parent->right;

        }

        /* case 4: sibling is black with red right child */

        sibling->color = x->parent->color;
        x->parent->color = COMMC_RB_BLACK;
        sibling->right->color = COMMC_RB_BLACK;
        commc_rb_rotate_left(tree, x->parent);
        x = tree->root;                          /* terminate loop */

      }

    } else {

      /* symmetric case: x is right child */

      sibling = x->parent->left;

      /* case 1: sibling is red - recolor and rotate */

      if  (sibling->color == COMMC_RB_RED) {

        sibling->color = COMMC_RB_BLACK;
        x->parent->color = COMMC_RB_RED;
        commc_rb_rotate_right(tree, x->parent);
        sibling = x->parent->left;

      }

      /* case 2: sibling is black with black children - recolor and move up */

      if  (sibling->right->color == COMMC_RB_BLACK && sibling->left->color == COMMC_RB_BLACK) {

        sibling->color = COMMC_RB_RED;
        x = x->parent;

      } else {

        /* case 3: sibling is black, right child is red, left child is black */

        if  (sibling->left->color == COMMC_RB_BLACK) {

          sibling->right->color = COMMC_RB_BLACK;
          sibling->color = COMMC_RB_RED;
          commc_rb_rotate_left(tree, sibling);
          sibling = x->parent->left;

        }

        /* case 4: sibling is black with red left child */

        sibling->color = x->parent->color;
        x->parent->color = COMMC_RB_BLACK;
        sibling->left->color = COMMC_RB_BLACK;
        commc_rb_rotate_right(tree, x->parent);
        x = tree->root;                          /* terminate loop */

      }

    }

  }

  /* ensure x is black */

  x->color = COMMC_RB_BLACK;

}

/*

         commc_rb_get_recursive()
	       ---
	       recursive helper for finding a value by key.

*/

static void* commc_rb_get_recursive(commc_rb_tree_t* tree,
                                   commc_rb_tree_node_t* node,
                                   const void* key) {

  int cmp;

  if  (!node || node == tree->nil) {

    return NULL;

  }

  cmp = tree->compare_func(key, node->key);

  if  (cmp < 0) {

    return commc_rb_get_recursive(tree, node->left, key);

  } else if  (cmp > 0) {

    return commc_rb_get_recursive(tree, node->right, key);

  } else {

    return node->value;

  }

}

/*

         commc_rb_height_recursive()
	       ---
	       recursive helper for calculating tree height.

*/

static size_t commc_rb_height_recursive(commc_rb_tree_t* tree, commc_rb_tree_node_t* node) {

  size_t left_height;
  size_t right_height;

  if  (!node || node == tree->nil) {

    return 0;

  }

  left_height = commc_rb_height_recursive(tree, node->left);
  right_height = commc_rb_height_recursive(tree, node->right);

  return 1 + ((left_height > right_height) ? left_height : right_height);

}

/*

         commc_rb_validate_recursive()
	       ---
	       recursive helper for validating Red-Black properties.
	       returns the black height of the subtree, or -1 if invalid.

*/

static int commc_rb_validate_recursive(commc_rb_tree_t* tree, commc_rb_tree_node_t* node) {

  int left_black_height;
  int right_black_height;

  if  (!node || node == tree->nil) {

    return 1;                                    /* NIL nodes count as black with height 1 */

  }

  /* check red-red violation */

  if  (node->color == COMMC_RB_RED) {

    if  ((node->left != tree->nil && node->left->color == COMMC_RB_RED) ||
         (node->right != tree->nil && node->right->color == COMMC_RB_RED)) {

      return -1;                                 /* red-red violation */

    }

  }

  /* recursively validate children */

  left_black_height = commc_rb_validate_recursive(tree, node->left);
  right_black_height = commc_rb_validate_recursive(tree, node->right);

  if  (left_black_height == -1 || right_black_height == -1) {

    return -1;                                   /* child is invalid */

  }

  /* check black height consistency */

  if  (left_black_height != right_black_height) {

    return -1;                                   /* black height violation */

  }

  /* return black height of this subtree */

  return left_black_height + ((node->color == COMMC_RB_BLACK) ? 1 : 0);

}

/*
	==================================
             --- FUNCS ---
	==================================
*/

/*

         commc_rb_tree_create()
	       ---
	       allocates and initializes a new Red-Black tree.

*/

commc_rb_tree_t* commc_rb_tree_create(commc_rb_compare_func compare_func) {

  commc_rb_tree_t* tree;

  if  (!compare_func) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  tree = (commc_rb_tree_t*)malloc(sizeof(commc_rb_tree_t));

  if  (!tree) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  /* create sentinel NIL node */

  tree->nil = (commc_rb_tree_node_t*)malloc(sizeof(commc_rb_tree_node_t));

  if  (!tree->nil) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    free(tree);
    return NULL;

  }

  /* initialize NIL node */

  tree->nil->color = COMMC_RB_BLACK;             /* NIL is always black */
  tree->nil->left = tree->nil;
  tree->nil->right = tree->nil;
  tree->nil->parent = tree->nil;
  tree->nil->key = NULL;
  tree->nil->value = NULL;

  /* initialize tree */

  tree->root = tree->nil;
  tree->size = 0;
  tree->compare_func = compare_func;

  return tree;

}

/*

         commc_rb_tree_destroy()
	       ---
	       frees all nodes and the tree structure itself.

*/

void commc_rb_tree_destroy(commc_rb_tree_t* tree) {

  if  (!tree) {

    return;

  }

  commc_rb_node_destroy_recursive(tree, tree->root);
  free(tree->nil);
  free(tree);

}

/*

         commc_rb_tree_insert()
	       ---
	       inserts a key-value pair with automatic rebalancing.

*/

commc_error_t commc_rb_tree_insert(commc_rb_tree_t* tree, void* key, void* value) {

  commc_rb_tree_node_t* z;
  commc_rb_tree_node_t* y;
  commc_rb_tree_node_t* x;
  int cmp;

  if  (!tree) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;

  }

  z = commc_rb_node_create(tree, key, value);

  if  (!z) {

    return COMMC_MEMORY_ERROR;

  }

  /* standard BST insertion */

  y = tree->nil;
  x = tree->root;

  while  (x != tree->nil) {

    y = x;
    cmp = tree->compare_func(z->key, x->key);

    if  (cmp < 0) {

      x = x->left;

    } else if  (cmp > 0) {

      x = x->right;

    } else {

      /* key exists - update value and free new node */

      x->value = value;
      free(z);
      return COMMC_SUCCESS;

    }

  }

  z->parent = y;

  if  (y == tree->nil) {

    tree->root = z;                              /* tree was empty */

  } else {

    cmp = tree->compare_func(z->key, y->key);

    if  (cmp < 0) {

      y->left = z;

    } else {

      y->right = z;

    }

  }

  tree->size++;

  /* fix Red-Black properties */

  commc_rb_insert_fixup(tree, z);

  return COMMC_SUCCESS;

}

/*

         commc_rb_tree_get()
	       ---
	       retrieves the value for a given key.

*/

void* commc_rb_tree_get(commc_rb_tree_t* tree, const void* key) {

  if  (!tree) {

    return NULL;

  }

  return commc_rb_get_recursive(tree, tree->root, key);

}

/*

         commc_rb_tree_remove()
	       ---
	       removes a key-value pair with automatic rebalancing.

*/

commc_error_t commc_rb_tree_remove(commc_rb_tree_t* tree, const void* key) {

  commc_rb_tree_node_t* z;
  commc_rb_tree_node_t* y;
  commc_rb_tree_node_t* x;
  commc_rb_color_t y_original_color;
  int cmp;

  if  (!tree) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;

  }

  /* find node to delete */

  z = tree->root;

  while  (z != tree->nil) {

    cmp = tree->compare_func(key, z->key);

    if  (cmp < 0) {

      z = z->left;

    } else if  (cmp > 0) {

      z = z->right;

    } else {

      break;                                     /* found it */

    }

  }

  if  (z == tree->nil) {

    return COMMC_ARGUMENT_ERROR;                 /* key not found */

  }

  y = z;
  y_original_color = y->color;

  if  (z->left == tree->nil) {

    x = z->right;
    commc_rb_transplant(tree, z, z->right);

  } else if  (z->right == tree->nil) {

    x = z->left;
    commc_rb_transplant(tree, z, z->left);

  } else {

    y = commc_rb_min_node(tree, z->right);
    y_original_color = y->color;
    x = y->right;

    if  (y->parent == z) {

      x->parent = y;

    } else {

      commc_rb_transplant(tree, y, y->right);
      y->right = z->right;
      y->right->parent = y;

    }

    commc_rb_transplant(tree, z, y);
    y->left = z->left;
    y->left->parent = y;
    y->color = z->color;

  }

  tree->size--;
  free(z);

  /* fix Red-Black properties if we removed a black node */

  if  (y_original_color == COMMC_RB_BLACK) {

    commc_rb_delete_fixup(tree, x);

  }

  return COMMC_SUCCESS;

}

/*

         commc_rb_tree_size()
	       ---
	       returns the number of nodes in the tree.

*/

size_t commc_rb_tree_size(commc_rb_tree_t* tree) {

  if  (!tree) {

    return 0;

  }

  return tree->size;

}

/*

         commc_rb_tree_height()
	       ---
	       returns the height of the tree.

*/

size_t commc_rb_tree_height(commc_rb_tree_t* tree) {

  if  (!tree) {

    return 0;

  }

  return commc_rb_height_recursive(tree, tree->root);

}

/*

         commc_rb_tree_clear()
	       ---
	       removes all nodes from the tree.

*/

void commc_rb_tree_clear(commc_rb_tree_t* tree) {

  if  (!tree) {

    return;

  }

  commc_rb_node_destroy_recursive(tree, tree->root);
  tree->root = tree->nil;
  tree->size = 0;

}

/*

         commc_rb_tree_contains()
	       ---
	       checks if the tree contains a key.

*/

int commc_rb_tree_contains(commc_rb_tree_t* tree, const void* key) {

  return commc_rb_tree_get(tree, key) != NULL;

}

/*

         commc_rb_tree_min_key()
	       ---
	       returns the smallest key in the tree.

*/

void* commc_rb_tree_min_key(commc_rb_tree_t* tree) {

  commc_rb_tree_node_t* min_node;

  if  (!tree) {

    return NULL;

  }

  min_node = commc_rb_min_node(tree, tree->root);

  return (min_node == tree->nil) ? NULL : min_node->key;

}

/*

         commc_rb_tree_max_key()
	       ---
	       returns the largest key in the tree.

*/

void* commc_rb_tree_max_key(commc_rb_tree_t* tree) {

  commc_rb_tree_node_t* max_node;

  if  (!tree) {

    return NULL;

  }

  max_node = commc_rb_max_node(tree, tree->root);

  return (max_node == tree->nil) ? NULL : max_node->key;

}

/*

         commc_rb_tree_validate()
	       ---
	       validates Red-Black properties of the tree.

*/

int commc_rb_tree_validate(commc_rb_tree_t* tree) {

  if  (!tree) {

    return 0;

  }

  /* check that root is black */

  if  (tree->root != tree->nil && tree->root->color != COMMC_RB_BLACK) {

    return 0;

  }

  /* validate recursive properties */

  return commc_rb_validate_recursive(tree, tree->root) != -1;

}

/*
	==================================
             --- EOF ---
	==================================
*/