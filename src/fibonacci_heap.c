/*
   ===================================
   C O M M O N - C
   FIBONACCI HEAP IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- FIBONACCI HEAP MODULE ---

    implementation of the fibonacci heap data structure.
    see include/commc/fibonacci_heap.h for function
    prototypes and documentation.

    this implementation uses circular doubly-linked lists
    to maintain the root list and child lists efficiently.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#include "commc/fibonacci_heap.h"
#include "commc/error.h"
#include <stdlib.h>
#include <math.h>

/*
	==================================
             --- STRUCTS ---
	==================================
*/

/* fibonacci heap node structure. */

struct commc_fibonacci_node_t {

  void*                      element;  /* user data */
  commc_fibonacci_node_t*    parent;   /* parent node */
  commc_fibonacci_node_t*    child;    /* first child */
  commc_fibonacci_node_t*    left;     /* left sibling */
  commc_fibonacci_node_t*    right;    /* right sibling */
  int                        degree;   /* number of children */
  int                        marked;   /* marked for cascading cuts */

};

/* fibonacci heap structure. */

struct commc_fibonacci_heap_t {

  commc_fibonacci_node_t*             min_node;   /* pointer to minimum node */
  size_t                              size;       /* number of nodes */
  commc_fibonacci_heap_compare_func_t compare;    /* comparison function */

};

/*
	==================================
             --- STATIC FUNCS ---
	==================================
*/

/*

         create_node()
	       ---
	       creates a new fibonacci heap node with the given element.
	       initializes all pointers to create a single-node list.

*/

static commc_fibonacci_node_t* create_node(void* element) {

  commc_fibonacci_node_t* node = (commc_fibonacci_node_t*)malloc(sizeof(commc_fibonacci_node_t));

  if  (!node) {

    return NULL;

  }

  node->element = element;
  node->parent  = NULL;
  node->child   = NULL;
  node->left    = node;
  node->right   = node;
  node->degree  = 0;
  node->marked  = 0;

  return node;

}

/*

         add_to_root_list()
	       ---
	       adds a node to the root list of the fibonacci heap.
	       maintains the circular doubly-linked list structure.

*/

static void add_to_root_list(commc_fibonacci_heap_t* heap, commc_fibonacci_node_t* node) {

  if  (!heap->min_node) {

    heap->min_node = node;
    node->left = node->right = node;

  } else {

    /* insert node between min_node and min_node->right */
    node->left = heap->min_node;
    node->right = heap->min_node->right;
    heap->min_node->right->left = node;
    heap->min_node->right = node;

    /* update min_node if necessary */

    if  (heap->compare(node->element, heap->min_node->element) < 0) {

      heap->min_node = node;

    }

  }

}

/*

         remove_from_list()
	       ---
	       removes a node from its current doubly-linked list.
	       handles both root list and child list removal.

*/

static void remove_from_list(commc_fibonacci_node_t* node) {

  if  (node->left == node) {

    /* single node in list */
    return;

  }

  node->left->right = node->right;
  node->right->left = node->left;

}

/*

         link_nodes()
	       ---
	       makes node2 a child of node1. used during heap consolidation
	       to combine trees of the same degree. maintains heap order.

*/

static void link_nodes(commc_fibonacci_heap_t* heap, commc_fibonacci_node_t* node1, commc_fibonacci_node_t* node2) {

  /* suppress unused parameter warning */
  (void)heap;

  /* remove node2 from root list */
  remove_from_list(node2);

  /* make node2 a child of node1 */

  if  (!node1->child) {

    node1->child = node2;
    node2->left = node2->right = node2;

  } else {

    node2->left = node1->child;
    node2->right = node1->child->right;
    node1->child->right->left = node2;
    node1->child->right = node2;

  }

  node2->parent = node1;
  node1->degree++;
  node2->marked = 0; /* clear mark when making child */

}

/*

         consolidate()
	       ---
	       consolidates the heap by combining trees of equal degree.
	       this maintains the logarithmic bound on tree degrees.
	       called after extract_min operations.

*/

static void consolidate(commc_fibonacci_heap_t* heap) {

  int max_degree = (int)(log(heap->size) / log(2)) + 1;
  commc_fibonacci_node_t** degree_table = (commc_fibonacci_node_t**)calloc(max_degree + 1, sizeof(commc_fibonacci_node_t*));
  commc_fibonacci_node_t* current = heap->min_node;
  commc_fibonacci_node_t* start;
  int i;

  if  (!degree_table) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return;

  }

  if  (!current) {

    free(degree_table);
    return;

  }

  /* mark starting point for root list traversal */
  start = current;

  do {

    commc_fibonacci_node_t* next = current->right;
    int degree = current->degree;

    while  (degree_table[degree] != NULL) {

      commc_fibonacci_node_t* conflict = degree_table[degree];

      /* ensure current has smaller key than conflict */

      if  (heap->compare(current->element, conflict->element) > 0) {

        commc_fibonacci_node_t* temp = current;
        current = conflict;
        conflict = temp;

      }

      link_nodes(heap, current, conflict);
      degree_table[degree] = NULL;
      degree++;

    }

    degree_table[degree] = current;
    current = next;

  } while (current != start);

  /* rebuild root list and find new minimum */
  heap->min_node = NULL;

  for  (i = 0; i <= max_degree; i++) {

    if  (degree_table[i] != NULL) {

      add_to_root_list(heap, degree_table[i]);

    }

  }

  free(degree_table);

}

/*

         cut()
	       ---
	       cuts a child from its parent and adds it to the root list.
	       used in decrease_key operations to maintain heap property.

*/

static void cut(commc_fibonacci_heap_t* heap, commc_fibonacci_node_t* node, commc_fibonacci_node_t* parent) {

  /* remove node from parent's child list */

  if  (parent->child == node) {

    if  (node->right == node) {

      parent->child = NULL; /* node was only child */

    } else {

      parent->child = node->right;

    }

  }

  remove_from_list(node);
  parent->degree--;

  /* add node to root list */
  node->parent = NULL;
  node->marked = 0;
  add_to_root_list(heap, node);

}

/*

         cascading_cut()
	       ---
	       performs cascading cuts up the tree to maintain
	       the fibonacci heap's degree bounds. cuts marked
	       nodes and marks unmarked ones.

*/

static void cascading_cut(commc_fibonacci_heap_t* heap, commc_fibonacci_node_t* node) {

  commc_fibonacci_node_t* parent = node->parent;

  if  (parent) {

    if  (!node->marked) {

      node->marked = 1; /* mark for future cut */

    } else {

      cut(heap, node, parent);
      cascading_cut(heap, parent); /* continue up the tree */

    }

  }

}

/*

         destroy_subtree()
	       ---
	       recursively destroys all nodes in a subtree.
	       used during heap destruction and clearing.

*/

static void destroy_subtree(commc_fibonacci_node_t* node) {

  commc_fibonacci_node_t* current;
  commc_fibonacci_node_t* start;

  if  (!node) {

    return;

  }

  /* destroy all children first */

  if  (node->child) {

    current = node->child;
    start = current;

    do {

      commc_fibonacci_node_t* next = current->right;
      destroy_subtree(current);
      current = next;

    } while (current != start);

  }

  free(node);

}

/*
	==================================
             --- FUNCS ---
	==================================
*/

/*

         commc_fibonacci_heap_create()
	       ---
	       allocates and initializes a new fibonacci heap.

*/

commc_fibonacci_heap_t* commc_fibonacci_heap_create(commc_fibonacci_heap_compare_func_t compare) {

  commc_fibonacci_heap_t* heap;

  if  (!compare) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  heap = (commc_fibonacci_heap_t*)malloc(sizeof(commc_fibonacci_heap_t));

  if  (!heap) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  heap->min_node = NULL;
  heap->size     = 0;
  heap->compare  = compare;

  return heap;

}

/*

         commc_fibonacci_heap_destroy()
	       ---
	       frees all memory used by the fibonacci heap.

*/

void commc_fibonacci_heap_destroy(commc_fibonacci_heap_t* heap) {

  if  (!heap) {

    return;

  }

  commc_fibonacci_heap_clear(heap);
  free(heap);

}

/*

         commc_fibonacci_heap_insert()
	       ---
	       creates a new node and adds it to the root list.
	       returns the node pointer for future reference.

*/

commc_fibonacci_node_t* commc_fibonacci_heap_insert(commc_fibonacci_heap_t* heap, void* element) {

  commc_fibonacci_node_t* node;

  if  (!heap) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  node = create_node(element);

  if  (!node) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  add_to_root_list(heap, node);
  heap->size++;

  return node;

}

/*

         commc_fibonacci_heap_extract_min()
	       ---
	       removes the minimum element and consolidates the heap.

*/

void* commc_fibonacci_heap_extract_min(commc_fibonacci_heap_t* heap) {

  commc_fibonacci_node_t* min_node;
  void* min_element;

  if  (!heap || !heap->min_node) {

    if  (!heap) {
      commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    }
    return NULL;

  }

  min_node = heap->min_node;
  min_element = min_node->element;

  /* add all children of min_node to root list */

  if  (min_node->child) {

    commc_fibonacci_node_t* child = min_node->child;
    commc_fibonacci_node_t* start = child;

    do {

      commc_fibonacci_node_t* next = child->right;
      child->parent = NULL;
      add_to_root_list(heap, child);
      child = next;

    } while (child != start);

  }

  /* remove min_node from root list */

  if  (min_node->right == min_node) {

    heap->min_node = NULL; /* heap is now empty */

  } else {

    heap->min_node = min_node->right;
    remove_from_list(min_node);
    consolidate(heap);

  }

  heap->size--;
  free(min_node);

  return min_element;

}

/*

         commc_fibonacci_heap_peek_min()
	       ---
	       returns the minimum element without removing it.

*/

void* commc_fibonacci_heap_peek_min(commc_fibonacci_heap_t* heap) {

  if  (!heap || !heap->min_node) {

    if  (!heap) {
      commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    }
    return NULL;

  }

  return heap->min_node->element;

}

/*

         commc_fibonacci_heap_decrease_key()
	       ---
	       decreases the key of a node and maintains heap properties.

*/

commc_error_t commc_fibonacci_heap_decrease_key(commc_fibonacci_heap_t* heap, commc_fibonacci_node_t* node, void* new_element) {

  commc_fibonacci_node_t* parent;

  if  (!heap || !node) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;

  }

  /* verify that new key is actually smaller */

  if  (heap->compare(new_element, node->element) > 0) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;

  }

  node->element = new_element;
  parent = node->parent;

  /* check if heap order is violated */

  if  (parent && heap->compare(node->element, parent->element) < 0) {

    cut(heap, node, parent);
    cascading_cut(heap, parent);

  }

  /* update minimum if necessary */

  if  (heap->compare(node->element, heap->min_node->element) < 0) {

    heap->min_node = node;

  }

  return COMMC_SUCCESS;

}

/*

         commc_fibonacci_heap_delete()
	       ---
	       deletes a specific node by decreasing to minimum value.
	       NOTE: This is a simplified implementation that requires
	       the comparison function to handle a special sentinel value.

*/

commc_error_t commc_fibonacci_heap_delete(commc_fibonacci_heap_t* heap, commc_fibonacci_node_t* node) {

  if  (!heap || !node) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;

  }

  /* set to special minimum value (implementation-dependent) */
  node->element = NULL; /* assuming NULL sorts to minimum */

  /* move to root and make minimum */

  if  (node->parent) {

    cut(heap, node, node->parent);
    cascading_cut(heap, node->parent);

  }

  heap->min_node = node;

  /* extract the minimum (which is now our target node) */
  commc_fibonacci_heap_extract_min(heap);

  return COMMC_SUCCESS;

}

/*

         commc_fibonacci_heap_merge()
	       ---
	       merges two fibonacci heaps by combining root lists.

*/

commc_error_t commc_fibonacci_heap_merge(commc_fibonacci_heap_t* heap1, commc_fibonacci_heap_t* heap2) {

  if  (!heap1 || !heap2) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;

  }

  if  (!heap2->min_node) {

    return COMMC_SUCCESS; /* heap2 is empty */

  }

  if  (!heap1->min_node) {

    heap1->min_node = heap2->min_node;

  } else {

    /* merge the circular lists */
    commc_fibonacci_node_t* temp = heap1->min_node->right;
    heap1->min_node->right = heap2->min_node->right;
    heap2->min_node->right->left = heap1->min_node;
    heap2->min_node->right = temp;
    temp->left = heap2->min_node;

    /* update minimum if necessary */

    if  (heap1->compare(heap2->min_node->element, heap1->min_node->element) < 0) {

      heap1->min_node = heap2->min_node;

    }

  }

  heap1->size += heap2->size;

  /* invalidate heap2 */
  heap2->min_node = NULL;
  heap2->size = 0;

  return COMMC_SUCCESS;

}

/*

         commc_fibonacci_heap_size()
	       ---
	       returns the number of elements in the fibonacci heap.

*/

size_t commc_fibonacci_heap_size(commc_fibonacci_heap_t* heap) {

  return heap ? heap->size : 0;

}

/*

         commc_fibonacci_heap_is_empty()
	       ---
	       checks if the fibonacci heap contains any elements.

*/

int commc_fibonacci_heap_is_empty(commc_fibonacci_heap_t* heap) {

  return heap ? (heap->size == 0) : 1;

}

/*

         commc_fibonacci_heap_clear()
	       ---
	       removes all elements from the fibonacci heap.

*/

void commc_fibonacci_heap_clear(commc_fibonacci_heap_t* heap) {

  commc_fibonacci_node_t* current;
  commc_fibonacci_node_t* start;

  if  (!heap || !heap->min_node) {

    if  (heap) {
      heap->size = 0;
    }
    return;

  }

  current = heap->min_node;
  start = current;

  do {

    commc_fibonacci_node_t* next = current->right;
    destroy_subtree(current);
    current = next;

  } while (current != start);

  heap->min_node = NULL;
  heap->size = 0;

}

/*
	==================================
             --- EOF ---
	==================================
*/