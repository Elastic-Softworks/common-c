/*
   ===================================
   C O M M O N - C
   B-TREE IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- B-TREE MODULE ---

    implementation of multi-way B-Tree optimized for disk storage.
    see include/commc/b_tree.h for function prototypes and documentation.

    B-Trees are specifically designed for systems where data access
    has high latency (like disk reads). by storing many keys per node,
    they minimize the number of disk accesses needed for operations.

    the implementation maintains the fundamental B-Tree properties:
    1. all leaves are at the same level
    2. nodes have between t-1 and 2t-1 keys (except root)
    3. keys within nodes are sorted
    4. internal nodes have one more child than keys

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#include "commc/b_tree.h"
#include "commc/error.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
	==================================
             --- STRUCTS ---
	==================================
*/

/* internal B-Tree node structure */

struct commc_b_tree_node_t {

  void**                    keys;                /* array of keys */
  void**                    values;              /* array of values */
  struct commc_b_tree_node_t** children;        /* array of child pointers */

  int                       key_count;           /* current number of keys */
  int                       is_leaf;             /* 1 if leaf, 0 if internal */

};

/* internal B-Tree structure */

struct commc_b_tree_t {

  commc_b_tree_node_t*      root;                /* root node */
  int                       min_degree;          /* minimum degree (t) */
  int                       max_keys;            /* maximum keys per node (2t-1) */
  size_t                    size;                /* total number of keys */
  commc_b_compare_func      compare_func;        /* key comparison function */

};

/*
	==================================
             --- STATIC FUNCS ---
	==================================
*/

/*

         commc_b_node_create()
	       ---
	       creates a new B-Tree node with allocated arrays.
	       initializes as a leaf node by default.

*/

static commc_b_tree_node_t* commc_b_node_create(int max_keys, int is_leaf) {

  commc_b_tree_node_t* node;

  node = (commc_b_tree_node_t*)malloc(sizeof(commc_b_tree_node_t));

  if  (!node) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  /* allocate arrays for keys, values, and children */

  node->keys = (void**)malloc(max_keys * sizeof(void*));
  node->values = (void**)malloc(max_keys * sizeof(void*));
  node->children = (commc_b_tree_node_t**)malloc((max_keys + 1) * sizeof(commc_b_tree_node_t*));

  if  (!node->keys || !node->values || !node->children) {

    free(node->keys);
    free(node->values);
    free(node->children);
    free(node);
    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  /* initialize node properties */

  node->key_count = 0;
  node->is_leaf = is_leaf;

  /* initialize arrays to NULL for safety */

  memset(node->keys, 0, max_keys * sizeof(void*));
  memset(node->values, 0, max_keys * sizeof(void*));
  memset(node->children, 0, (max_keys + 1) * sizeof(commc_b_tree_node_t*));

  return node;

}

/*

         commc_b_node_destroy_recursive()
	       ---
	       recursively destroys a node and all its children.

*/

static void commc_b_node_destroy_recursive(commc_b_tree_node_t* node) {

  int i;

  if  (!node) {

    return;

  }

  /* recursively destroy children first */

  if  (!node->is_leaf) {

    for  (i = 0; i <= node->key_count; i++) {

      commc_b_node_destroy_recursive(node->children[i]);

    }

  }

  /* free arrays and node */

  free(node->keys);
  free(node->values);
  free(node->children);
  free(node);

}

/*

         commc_b_node_search()
	       ---
	       searches for a key within a node.
	       returns the index if found, or -(insertion_point + 1) if not found.

*/

static int commc_b_node_search(commc_b_tree_node_t* node, 
                               const void* key, 
                               commc_b_compare_func compare_func) {

  int i;
  int cmp;

  for  (i = 0; i < node->key_count; i++) {

    cmp = compare_func(key, node->keys[i]);

    if  (cmp == 0) {

      return i;                                  /* key found */

    } else if  (cmp < 0) {

      return -(i + 1);                           /* key should be inserted at position i */

    }

  }

  return -(node->key_count + 1);                 /* key should be inserted at end */

}

/*

         commc_b_node_insert_key()
	       ---
	       inserts a key-value pair into a node at the specified position.
	       shifts existing elements to make room.

*/

static void commc_b_node_insert_key(commc_b_tree_node_t* node, 
                                    int position, 
                                    void* key, 
                                    void* value) {

  int i;

  /* shift keys, values, and children to make room */

  for  (i = node->key_count; i > position; i--) {

    node->keys[i] = node->keys[i - 1];
    node->values[i] = node->values[i - 1];

  }

  if  (!node->is_leaf) {

    for  (i = node->key_count + 1; i > position + 1; i--) {

      node->children[i] = node->children[i - 1];

    }

  }

  /* insert the new key-value pair */

  node->keys[position] = key;
  node->values[position] = value;
  node->key_count++;

}

/*

         commc_b_node_remove_key()
	       ---
	       removes a key-value pair from a node at the specified position.
	       shifts remaining elements to fill the gap.

*/

#ifdef __GNUC__
__attribute__((unused))
#endif
static void commc_b_node_remove_key(commc_b_tree_node_t* node, int position) {

  int i;

  /* shift keys and values to fill the gap */

  for  (i = position; i < node->key_count - 1; i++) {

    node->keys[i] = node->keys[i + 1];
    node->values[i] = node->values[i + 1];

  }

  /* shift children if internal node */

  if  (!node->is_leaf) {

    for  (i = position + 1; i < node->key_count; i++) {

      node->children[i] = node->children[i + 1];

    }

  }

  node->key_count--;

}

/*

         commc_b_node_is_full()
	       ---
	       checks if a node is full (has maximum number of keys).

*/

static int commc_b_node_is_full(commc_b_tree_node_t* node, int max_keys) {

  return node->key_count == max_keys;

}

/*

         commc_b_node_is_minimal()
	       ---
	       checks if a node has minimum number of keys.
	       root can have fewer keys than other nodes.

*/

#ifdef __GNUC__
__attribute__((unused))
#endif
static int commc_b_node_is_minimal(commc_b_tree_node_t* node, int min_degree, int is_root) {

  int min_keys = is_root ? 1 : (min_degree - 1);
  return node->key_count == min_keys;

}

/*

         commc_b_node_split_child()
	       ---
	       splits a full child node into two nodes.
	       the median key moves up to the parent.

*/

static commc_error_t commc_b_node_split_child(commc_b_tree_node_t* parent, 
                                              int child_index,
                                              int max_keys,
                                              int min_degree) {

  commc_b_tree_node_t* full_child;
  commc_b_tree_node_t* new_child;
  int median_index;
  int i;

  full_child = parent->children[child_index];
  median_index = min_degree - 1;

  /* create new node to hold second half of keys */

  new_child = commc_b_node_create(max_keys, full_child->is_leaf);

  if  (!new_child) {

    return COMMC_MEMORY_ERROR;

  }

  /* copy second half of keys and values to new child */

  for  (i = 0; i < min_degree - 1; i++) {

    new_child->keys[i] = full_child->keys[i + min_degree];
    new_child->values[i] = full_child->values[i + min_degree];

  }

  new_child->key_count = min_degree - 1;

  /* copy children if internal node */

  if  (!full_child->is_leaf) {

    for  (i = 0; i < min_degree; i++) {

      new_child->children[i] = full_child->children[i + min_degree];

    }

  }

  /* adjust original child size */

  full_child->key_count = min_degree - 1;

  /* shift parent's children to make room for new child */

  for  (i = parent->key_count; i > child_index; i--) {

    parent->children[i + 1] = parent->children[i];

  }

  parent->children[child_index + 1] = new_child;

  /* move median key up to parent */

  commc_b_node_insert_key(parent, 
                          child_index, 
                          full_child->keys[median_index], 
                          full_child->values[median_index]);

  return COMMC_SUCCESS;

}

/*

         commc_b_tree_insert_non_full()
	       ---
	       recursive helper for insertion into a non-full node.
	       handles node splitting as needed during traversal.

*/

static commc_error_t commc_b_tree_insert_non_full(commc_b_tree_t* tree,
                                                  commc_b_tree_node_t* node,
                                                  void* key,
                                                  void* value) {

  int search_result;
  int insertion_point;
  int child_index;
  commc_error_t error;

  search_result = commc_b_node_search(node, key, tree->compare_func);

  if  (search_result >= 0) {

    /* key exists - update value */

    node->values[search_result] = value;
    return COMMC_SUCCESS;

  }

  insertion_point = -(search_result + 1);

  if  (node->is_leaf) {

    /* insert into leaf node */

    commc_b_node_insert_key(node, insertion_point, key, value);
    tree->size++;
    return COMMC_SUCCESS;

  } else {

    /* traverse to appropriate child */

    child_index = insertion_point;

    /* split child if it's full */

    if  (commc_b_node_is_full(node->children[child_index], tree->max_keys)) {

      error = commc_b_node_split_child(node, child_index, tree->max_keys, tree->min_degree);

      if  (error != COMMC_SUCCESS) {

        return error;

      }

      /* determine which of the two children to follow */

      if  (tree->compare_func(key, node->keys[child_index]) > 0) {

        child_index++;

      }

    }

    return commc_b_tree_insert_non_full(tree, node->children[child_index], key, value);

  }

}

/*

         commc_b_tree_get_recursive()
	       ---
	       recursive helper for key retrieval.

*/

static void* commc_b_tree_get_recursive(commc_b_tree_t* tree,
                                        commc_b_tree_node_t* node,
                                        const void* key) {

  int search_result;
  int child_index;

  if  (!node) {

    return NULL;

  }

  search_result = commc_b_node_search(node, key, tree->compare_func);

  if  (search_result >= 0) {

    /* key found */

    return node->values[search_result];

  }

  if  (node->is_leaf) {

    /* key not found and we're at a leaf */

    return NULL;

  }

  /* traverse to appropriate child */

  child_index = -(search_result + 1);
  return commc_b_tree_get_recursive(tree, node->children[child_index], key);

}

/*

         commc_b_tree_height_recursive()
	       ---
	       recursive helper for calculating tree height.

*/

static size_t commc_b_tree_height_recursive(commc_b_tree_node_t* node) {

  if  (!node) {

    return 0;

  }

  if  (node->is_leaf) {

    return 1;

  }

  return 1 + commc_b_tree_height_recursive(node->children[0]);

}

/*

         commc_b_tree_validate_recursive()
	       ---
	       recursive helper for validating B-Tree properties.

*/

static int commc_b_tree_validate_recursive(commc_b_tree_t* tree,
                                           commc_b_tree_node_t* node,
                                           int is_root,
                                           int* leaf_level,
                                           int current_level) {

  int i;
  int expected_children;

  if  (!node) {

    return 1;

  }

  /* check key count constraints */

  if  (!is_root && node->key_count < tree->min_degree - 1) {

    return 0;                                    /* too few keys */

  }

  if  (node->key_count > tree->max_keys) {

    return 0;                                    /* too many keys */

  }

  /* check key ordering within node */

  for  (i = 1; i < node->key_count; i++) {

    if  (tree->compare_func(node->keys[i - 1], node->keys[i]) >= 0) {

      return 0;                                  /* keys not in order */

    }

  }

  /* check leaf level consistency */

  if  (node->is_leaf) {

    if  (*leaf_level == -1) {

      *leaf_level = current_level;               /* first leaf found */

    } else if  (*leaf_level != current_level) {

      return 0;                                  /* leaves at different levels */

    }

  } else {

    /* check children count */

    expected_children = node->key_count + 1;

    for  (i = 0; i < expected_children; i++) {

      if  (!node->children[i]) {

        return 0;                                /* missing child */

      }

      if  (!commc_b_tree_validate_recursive(tree, node->children[i], 0, leaf_level, current_level + 1)) {

        return 0;

      }

    }

  }

  return 1;

}

/*

         commc_b_tree_find_min_node()
	       ---
	       finds the leftmost leaf node (contains minimum keys).

*/

static commc_b_tree_node_t* commc_b_tree_find_min_node(commc_b_tree_node_t* node) {

  if  (!node) {

    return NULL;

  }

  while  (!node->is_leaf) {

    node = node->children[0];

  }

  return node;

}

/*

         commc_b_tree_find_max_node()
	       ---
	       finds the rightmost leaf node (contains maximum keys).

*/

static commc_b_tree_node_t* commc_b_tree_find_max_node(commc_b_tree_node_t* node) {

  if  (!node) {

    return NULL;

  }

  while  (!node->is_leaf) {

    node = node->children[node->key_count];

  }

  return node;

}

/*
	==================================
             --- FUNCS ---
	==================================
*/

/*

         commc_b_tree_create()
	       ---
	       allocates and initializes a new B-Tree.

*/

commc_b_tree_t* commc_b_tree_create(int min_degree, commc_b_compare_func compare_func) {

  commc_b_tree_t* tree;

  if  (min_degree < 2 || !compare_func) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  tree = (commc_b_tree_t*)malloc(sizeof(commc_b_tree_t));

  if  (!tree) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  /* initialize B-Tree properties */

  tree->min_degree = min_degree;
  tree->max_keys = (2 * min_degree) - 1;
  tree->size = 0;
  tree->compare_func = compare_func;

  /* create empty root */

  tree->root = commc_b_node_create(tree->max_keys, 1);

  if  (!tree->root) {

    free(tree);
    return NULL;

  }

  return tree;

}

/*

         commc_b_tree_destroy()
	       ---
	       frees all nodes and the tree structure.

*/

void commc_b_tree_destroy(commc_b_tree_t* tree) {

  if  (!tree) {

    return;

  }

  commc_b_node_destroy_recursive(tree->root);
  free(tree);

}

/*

         commc_b_tree_insert()
	       ---
	       inserts a key-value pair into the B-Tree.

*/

commc_error_t commc_b_tree_insert(commc_b_tree_t* tree, void* key, void* value) {

  commc_b_tree_node_t* old_root;
  commc_b_tree_node_t* new_root;
  commc_error_t error;

  if  (!tree) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;

  }

  /* check if root is full */

  if  (commc_b_node_is_full(tree->root, tree->max_keys)) {

    /* create new root and split old root */

    old_root = tree->root;
    new_root = commc_b_node_create(tree->max_keys, 0);

    if  (!new_root) {

      return COMMC_MEMORY_ERROR;

    }

    tree->root = new_root;
    new_root->children[0] = old_root;

    error = commc_b_node_split_child(new_root, 0, tree->max_keys, tree->min_degree);

    if  (error != COMMC_SUCCESS) {

      return error;

    }

  }

  return commc_b_tree_insert_non_full(tree, tree->root, key, value);

}

/*

         commc_b_tree_get()
	       ---
	       retrieves the value for a given key.

*/

void* commc_b_tree_get(commc_b_tree_t* tree, const void* key) {

  if  (!tree) {

    return NULL;

  }

  return commc_b_tree_get_recursive(tree, tree->root, key);

}

/*

         commc_b_tree_remove()
	       ---
	       removes a key-value pair from the B-Tree.
	       simplified implementation - full deletion is complex.

*/

commc_error_t commc_b_tree_remove(commc_b_tree_t* tree, const void* key) {

  /* B-Tree deletion is quite complex and involves merging/borrowing */
  /* this is a simplified stub for the basic interface */

  if  (!tree) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;

  }

  /* suppress unused parameter warning */
  
  (void)key;

  /* actual implementation would handle deletion cases */

  return COMMC_NOT_IMPLEMENTED_ERROR;

}

/*

         commc_b_tree_contains()
	       ---
	       checks if the tree contains a key.

*/

int commc_b_tree_contains(commc_b_tree_t* tree, const void* key) {

  return commc_b_tree_get(tree, key) != NULL;

}

/*

         commc_b_tree_size()
	       ---
	       returns the number of key-value pairs.

*/

size_t commc_b_tree_size(commc_b_tree_t* tree) {

  if  (!tree) {

    return 0;

  }

  return tree->size;

}

/*

         commc_b_tree_height()
	       ---
	       returns the height of the tree.

*/

size_t commc_b_tree_height(commc_b_tree_t* tree) {

  if  (!tree) {

    return 0;

  }

  return commc_b_tree_height_recursive(tree->root);

}

/*

         commc_b_tree_clear()
	       ---
	       removes all elements from the tree.

*/

void commc_b_tree_clear(commc_b_tree_t* tree) {

  if  (!tree) {

    return;

  }

  commc_b_node_destroy_recursive(tree->root);
  tree->root = commc_b_node_create(tree->max_keys, 1);
  tree->size = 0;

}

/*

         commc_b_tree_min_key()
	       ---
	       returns the smallest key.

*/

void* commc_b_tree_min_key(commc_b_tree_t* tree) {

  commc_b_tree_node_t* min_node;

  if  (!tree || tree->size == 0) {

    return NULL;

  }

  min_node = commc_b_tree_find_min_node(tree->root);

  return min_node ? min_node->keys[0] : NULL;

}

/*

         commc_b_tree_max_key()
	       ---
	       returns the largest key.

*/

void* commc_b_tree_max_key(commc_b_tree_t* tree) {

  commc_b_tree_node_t* max_node;

  if  (!tree || tree->size == 0) {

    return NULL;

  }

  max_node = commc_b_tree_find_max_node(tree->root);

  return max_node ? max_node->keys[max_node->key_count - 1] : NULL;

}

/*

         commc_b_tree_validate()
	       ---
	       validates B-Tree structural properties.

*/

int commc_b_tree_validate(commc_b_tree_t* tree) {

  int leaf_level = -1;

  if  (!tree) {

    return 0;

  }

  return commc_b_tree_validate_recursive(tree, tree->root, 1, &leaf_level, 0);

}

/*

         commc_b_tree_get_min_degree()
	       ---
	       returns the minimum degree of the tree.

*/

int commc_b_tree_get_min_degree(commc_b_tree_t* tree) {

  if  (!tree) {

    return 0;

  }

  return tree->min_degree;

}

/*

         commc_b_tree_get_max_keys()
	       ---
	       returns the maximum keys per node.

*/

int commc_b_tree_get_max_keys(commc_b_tree_t* tree) {

  if  (!tree) {

    return 0;

  }

  return tree->max_keys;

}

/*

         commc_b_tree_print()
	       ---
	       prints tree structure for debugging.

*/

void commc_b_tree_print(commc_b_tree_t* tree, commc_b_print_key_func print_func) {

  /* simplified printing stub */

  if  (!tree || !print_func) {

    return;

  }

  printf("B-Tree (min_degree=%d, size=%lu)\n", tree->min_degree, (unsigned long)tree->size);

}

/*

         commc_b_tree_range_search()
	       ---
	       performs range query (stub implementation).

*/

size_t commc_b_tree_range_search(commc_b_tree_t* tree, 
                                 const void* min_key, 
                                 const void* max_key,
                                 void** results, 
                                 size_t max_results) {

  /* range search implementation would traverse tree collecting results */

  if  (!tree || !min_key || !max_key || !results) {

    return 0;

  }

  /* suppress unused parameter warning */
  
  (void)max_results;

  return 0;                                      /* stub - no results */

}

/*
	==================================
             --- EOF ---
	==================================
*/