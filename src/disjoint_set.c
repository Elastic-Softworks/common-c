/* 	
   ===================================
   C O M M C / D I S J O I N T _ S E T . C
   UNION-FIND DATA STRUCTURE IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

	                --- DISJOINT SET IMPLEMENTATION ---

	    this implementation provides near-constant time union and find
	    operations through two key optimizations: path compression during
	    find operations flattens trees, and union by rank keeps trees
	    balanced by always making the shorter tree a subtree of the taller.
	    
	    the amortized time complexity approaches O(α(n)) where α is the
	    inverse ackermann function, which is effectively constant for all
	    practical values of n.

*/

/* 
	==================================
             --- SETUP ---
	==================================
*/

#include "commc/disjoint_set.h"  /* DISJOINT SET API */
#include "commc/error.h"         /* ERROR HANDLING */
#include <stdlib.h>              /* STANDARD LIBRARY FUNCTIONS */

/* 
	==================================
             --- CONSTANTS ---
	==================================
*/

#define COMMC_DISJOINT_SET_INVALID_ELEMENT SIZE_MAX  /* invalid element marker */

/* 
	==================================
             --- HELPERS ---
	==================================
*/

/*

         is_valid_element()
	       ---
	       validates that element is within the valid range.

*/

static int is_valid_element(commc_disjoint_set_t* ds, size_t element) {

  return ds && element < ds->capacity;

}

/*

         find_with_compression()
	       ---
	       internal find with path compression implementation.
	       makes all nodes on path point directly to root.

*/

static commc_disjoint_set_node_t* find_with_compression(commc_disjoint_set_t* ds, size_t element) {

  commc_disjoint_set_node_t* node;
  commc_disjoint_set_node_t* root;

  if  (!is_valid_element(ds, element)) {
    return NULL;
  }

  node = &ds->nodes[element];

  /* find root */
  root = node;
  
  while  (root->parent != root) {
    root = root->parent;
  }

  /* path compression - make all nodes on path point to root */
  while  (node->parent != node) {

    commc_disjoint_set_node_t* parent = node->parent;
    node->parent = root;
    node = parent;

  }

  return root;

}

/*

         calculate_tree_depth()
	       ---
	       calculates actual tree depth starting from given node.

*/

static size_t calculate_tree_depth(commc_disjoint_set_node_t* node) {

  size_t depth = 0;

  while  (node->parent != node) {
    depth++;
    node = node->parent;
  }

  return depth;

}

/*
	==================================
             --- CORE API ---
	==================================
*/

/*

         commc_disjoint_set_create()
	       ---
	       creates disjoint set with each element as its own singleton set.

*/

commc_disjoint_set_t* commc_disjoint_set_create(size_t capacity) {

  commc_disjoint_set_t* ds;
  size_t                i;

  if  (capacity < COMMC_DISJOINT_SET_MIN_CAPACITY) {
    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;
  }

  ds = (commc_disjoint_set_t*)malloc(sizeof(commc_disjoint_set_t));
  
  if  (!ds) {
    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;
  }

  ds->nodes = (commc_disjoint_set_node_t*)malloc(sizeof(commc_disjoint_set_node_t) * capacity);
  
  if  (!ds->nodes) {
    free(ds);
    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;
  }

  ds->capacity = capacity;
  ds->set_count = capacity;

  /* initialize each element as its own set */
  for  (i = 0; i < capacity; i++) {
    ds->nodes[i].element = i;
    ds->nodes[i].parent = &ds->nodes[i];  /* self-parent makes it root */
    ds->nodes[i].rank = 0;
  }

  return ds;

}

/*

         commc_disjoint_set_destroy()
	       ---
	       destroys disjoint set and frees memory.

*/

void commc_disjoint_set_destroy(commc_disjoint_set_t* ds) {

  if  (!ds) {
    return;
  }

  if  (ds->nodes) {
    free(ds->nodes);
  }

  free(ds);

}

/*

         commc_disjoint_set_find()
	       ---
	       finds representative of set containing element with path compression.

*/

size_t commc_disjoint_set_find(commc_disjoint_set_t* ds, size_t element) {

  commc_disjoint_set_node_t* root;

  root = find_with_compression(ds, element);
  
  if  (!root) {
    return COMMC_DISJOINT_SET_INVALID_ELEMENT;
  }

  return root->element;

}

/*

         commc_disjoint_set_union()
	       ---
	       unites two sets using union by rank optimization.

*/

commc_error_t commc_disjoint_set_union(commc_disjoint_set_t* ds, size_t element1, size_t element2) {

  commc_disjoint_set_node_t* root1;
  commc_disjoint_set_node_t* root2;

  if  (!is_valid_element(ds, element1) || !is_valid_element(ds, element2)) {
    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;
  }

  root1 = find_with_compression(ds, element1);
  root2 = find_with_compression(ds, element2);

  if  (!root1 || !root2) {
    return COMMC_ARGUMENT_ERROR;
  }

  /* already in same set */
  if  (root1 == root2) {
    return COMMC_FAILURE;
  }

  /* union by rank - attach smaller tree under root of larger tree */
  if  (root1->rank < root2->rank) {

    root1->parent = root2;

  } else if  (root1->rank > root2->rank) {

    root2->parent = root1;

  } else {

    /* same rank - make root2 child of root1 and increment root1's rank */
    root2->parent = root1;
    root1->rank++;

  }

  ds->set_count--;
  return COMMC_SUCCESS;

}

/*

         commc_disjoint_set_connected()
	       ---
	       tests if two elements belong to same set.

*/

int commc_disjoint_set_connected(commc_disjoint_set_t* ds, size_t element1, size_t element2) {

  commc_disjoint_set_node_t* root1;
  commc_disjoint_set_node_t* root2;

  if  (!is_valid_element(ds, element1) || !is_valid_element(ds, element2)) {
    return 0;
  }

  root1 = find_with_compression(ds, element1);
  root2 = find_with_compression(ds, element2);

  return root1 && root2 && root1 == root2;

}

/*
	==================================
             --- INFO API ---
	==================================
*/

/*

         commc_disjoint_set_capacity()
	       ---
	       returns maximum number of elements.

*/

size_t commc_disjoint_set_capacity(commc_disjoint_set_t* ds) {

  if  (!ds) {
    return 0;
  }

  return ds->capacity;

}

/*

         commc_disjoint_set_count()
	       ---
	       returns current number of disjoint sets.

*/

size_t commc_disjoint_set_count(commc_disjoint_set_t* ds) {

  if  (!ds) {
    return 0;
  }

  return ds->set_count;

}

/*

         commc_disjoint_set_size()
	       ---
	       returns size of set containing given element.

*/

size_t commc_disjoint_set_size(commc_disjoint_set_t* ds, size_t element) {

  commc_disjoint_set_node_t* target_root;
  size_t                     set_size = 0;
  size_t                     i;

  if  (!is_valid_element(ds, element)) {
    return 0;
  }

  target_root = find_with_compression(ds, element);
  
  if  (!target_root) {
    return 0;
  }

  /* count all elements with same root */
  for  (i = 0; i < ds->capacity; i++) {

    if  (find_with_compression(ds, i) == target_root) {
      set_size++;
    }

  }

  return set_size;

}

/*

         commc_disjoint_set_get_representatives()
	       ---
	       fills array with set representatives.

*/

size_t commc_disjoint_set_get_representatives(commc_disjoint_set_t* ds,
                                               size_t* representatives,
                                               size_t max_count) {

  size_t count = 0;
  size_t i;

  if  (!ds || !representatives || max_count == 0) {
    return 0;
  }

  /* find all root elements */
  for  (i = 0; i < ds->capacity && count < max_count; i++) {

    if  (ds->nodes[i].parent == &ds->nodes[i]) {
      representatives[count] = i;
      count++;
    }

  }

  return count;

}

/*

         commc_disjoint_set_get_set_members()
	       ---
	       fills array with all members of element's set.

*/

size_t commc_disjoint_set_get_set_members(commc_disjoint_set_t* ds,
                                           size_t element,
                                           size_t* members,
                                           size_t max_count) {

  commc_disjoint_set_node_t* target_root;
  size_t                     count = 0;
  size_t                     i;

  if  (!is_valid_element(ds, element) || !members || max_count == 0) {
    return 0;
  }

  target_root = find_with_compression(ds, element);
  
  if  (!target_root) {
    return 0;
  }

  /* collect all elements with same root */
  for  (i = 0; i < ds->capacity && count < max_count; i++) {

    if  (find_with_compression(ds, i) == target_root) {
      members[count] = i;
      count++;
    }

  }

  return count;

}

/*
	==================================
             --- UTILITY API ---
	==================================
*/

/*

         commc_disjoint_set_reset()
	       ---
	       resets all elements to singleton sets.

*/

void commc_disjoint_set_reset(commc_disjoint_set_t* ds) {

  size_t i;

  if  (!ds) {
    return;
  }

  /* reinitialize each element as its own set */
  for  (i = 0; i < ds->capacity; i++) {
    ds->nodes[i].element = i;
    ds->nodes[i].parent = &ds->nodes[i];
    ds->nodes[i].rank = 0;
  }

  ds->set_count = ds->capacity;

}

/*

         commc_disjoint_set_compress_paths()
	       ---
	       explicitly applies path compression to all elements.

*/

void commc_disjoint_set_compress_paths(commc_disjoint_set_t* ds) {

  size_t i;

  if  (!ds) {
    return;
  }

  /* apply find to all elements to trigger path compression */
  for  (i = 0; i < ds->capacity; i++) {
    find_with_compression(ds, i);
  }

}

/*

         commc_disjoint_set_is_representative()
	       ---
	       tests if element is a set representative.

*/

int commc_disjoint_set_is_representative(commc_disjoint_set_t* ds, size_t element) {

  if  (!is_valid_element(ds, element)) {
    return 0;
  }

  return ds->nodes[element].parent == &ds->nodes[element];

}

/*

         commc_disjoint_set_get_rank()
	       ---
	       returns rank of element (meaningful for representatives only).

*/

size_t commc_disjoint_set_get_rank(commc_disjoint_set_t* ds, size_t element) {

  if  (!is_valid_element(ds, element)) {
    return 0;
  }

  return ds->nodes[element].rank;

}

/*
	==================================
             --- ITERATION API ---
	==================================
*/

/*

         commc_disjoint_set_iterator_create()
	       ---
	       creates iterator for set containing given element.

*/

commc_disjoint_set_iterator_t commc_disjoint_set_iterator_create(commc_disjoint_set_t* ds,
                                                                 size_t element) {

  commc_disjoint_set_iterator_t iterator;
  commc_disjoint_set_node_t*    root;

  iterator.ds = ds;
  iterator.current = 0;
  iterator.representative = COMMC_DISJOINT_SET_INVALID_ELEMENT;

  if  (is_valid_element(ds, element)) {

    root = find_with_compression(ds, element);
    
    if  (root) {
      iterator.representative = root->element;
    }

  }

  /* advance to first valid element */
  while  (iterator.current < ds->capacity &&
          commc_disjoint_set_find(ds, iterator.current) != iterator.representative) {
    iterator.current++;
  }

  return iterator;

}

/*

         commc_disjoint_set_iterator_next()
	       ---
	       advances iterator to next element in set.

*/

commc_error_t commc_disjoint_set_iterator_next(commc_disjoint_set_iterator_t* iterator) {

  if  (!iterator || !iterator->ds) {
    return COMMC_ARGUMENT_ERROR;
  }

  iterator->current++;

  /* find next element in same set */
  while  (iterator->current < iterator->ds->capacity &&
          commc_disjoint_set_find(iterator->ds, iterator->current) != iterator->representative) {
    iterator->current++;
  }

  return COMMC_SUCCESS;

}

/*

         commc_disjoint_set_iterator_current()
	       ---
	       returns current element from iterator.

*/

size_t commc_disjoint_set_iterator_current(commc_disjoint_set_iterator_t* iterator) {

  if  (!iterator || iterator->current >= iterator->ds->capacity) {
    return COMMC_DISJOINT_SET_INVALID_ELEMENT;
  }

  return iterator->current;

}

/*

         commc_disjoint_set_iterator_has_next()
	       ---
	       returns 1 if iterator has more elements.

*/

int commc_disjoint_set_iterator_has_next(commc_disjoint_set_iterator_t* iterator) {

  if  (!iterator || !iterator->ds) {
    return 0;
  }

  return iterator->current < iterator->ds->capacity;

}

/*
	==================================
             --- ANALYSIS API ---
	==================================
*/

/*

         commc_disjoint_set_analyze()
	       ---
	       computes comprehensive performance statistics.

*/

commc_disjoint_set_statistics_t commc_disjoint_set_analyze(commc_disjoint_set_t* ds) {

  commc_disjoint_set_statistics_t stats;
  size_t*                         set_sizes;
  size_t                          i;
  size_t                          depth;
  size_t                          total_depth = 0;

  /* initialize statistics */
  stats.total_sets = 0;
  stats.largest_set_size = 0;
  stats.smallest_set_size = ds ? ds->capacity : 0;
  stats.average_set_size = 0.0;
  stats.max_tree_depth = 0;
  stats.average_tree_depth = 0.0;

  if  (!ds) {
    return stats;
  }

  stats.total_sets = ds->set_count;

  if  (ds->set_count == 0) {
    return stats;
  }

  set_sizes = (size_t*)malloc(sizeof(size_t) * ds->set_count);
  
  if  (!set_sizes) {
    return stats;
  }

  /* analyze each representative's set */
  {
    size_t rep_count = 0;
    size_t total_elements = 0;

    for  (i = 0; i < ds->capacity; i++) {

      if  (commc_disjoint_set_is_representative(ds, i)) {

        size_t set_size = commc_disjoint_set_size(ds, i);
        
        set_sizes[rep_count] = set_size;
        rep_count++;

        total_elements += set_size;

        if  (set_size > stats.largest_set_size) {
          stats.largest_set_size = set_size;
        }

        if  (set_size < stats.smallest_set_size) {
          stats.smallest_set_size = set_size;
        }

      }

    }

    stats.average_set_size = (double)total_elements / ds->set_count;

  }

  /* analyze tree depths */
  for  (i = 0; i < ds->capacity; i++) {

    depth = calculate_tree_depth(&ds->nodes[i]);
    total_depth += depth;

    if  (depth > stats.max_tree_depth) {
      stats.max_tree_depth = depth;
    }

  }

  stats.average_tree_depth = (double)total_depth / ds->capacity;

  free(set_sizes);
  return stats;

}

/*

         commc_disjoint_set_memory_usage()
	       ---
	       returns total memory usage in bytes.

*/

size_t commc_disjoint_set_memory_usage(commc_disjoint_set_t* ds) {

  size_t total_bytes;

  if  (!ds) {
    return 0;
  }

  total_bytes = sizeof(commc_disjoint_set_t);
  total_bytes += sizeof(commc_disjoint_set_node_t) * ds->capacity;

  return total_bytes;

}

/*

         commc_disjoint_set_validate()
	       ---
	       validates internal data structure consistency.

*/

commc_error_t commc_disjoint_set_validate(commc_disjoint_set_t* ds) {

  size_t i;
  size_t actual_set_count = 0;

  if  (!ds || !ds->nodes) {
    return COMMC_ARGUMENT_ERROR;
  }

  /* check all elements are valid */
  for  (i = 0; i < ds->capacity; i++) {

    /* element should match its index */
    if  (ds->nodes[i].element != i) {
      return COMMC_FAILURE;
    }

    /* parent should be valid */
    if  (ds->nodes[i].parent < ds->nodes || 
         ds->nodes[i].parent >= ds->nodes + ds->capacity) {
      return COMMC_FAILURE;
    }

    /* count representatives */
    if  (ds->nodes[i].parent == &ds->nodes[i]) {
      actual_set_count++;
    }

  }

  /* validate set count */
  if  (actual_set_count != ds->set_count) {
    return COMMC_FAILURE;
  }

  return COMMC_SUCCESS;

}

/*
	==================================
             --- EOF ---
	==================================
*/