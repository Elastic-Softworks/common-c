/*
   ===================================
   C O M M O N - C
   FIBONACCI HEAP MODULE
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- FIBONACCI HEAP MODULE ---

    this module provides a fibonacci heap data structure,
    which is an advanced priority queue with excellent
    amortized complexity for decrease-key operations.

    fibonacci heaps are particularly useful in algorithms
    like dijkstra's shortest path and prim's minimum spanning
    tree where frequent decrease-key operations are needed.

    key properties:
    - extract-min: O(log n) amortized
    - insert: O(1) amortized
    - decrease-key: O(1) amortized
    - merge: O(1) actual

    the structure consists of a collection of min-heap-ordered
    trees organized in a circular doubly-linked list at the
    root level, with marked nodes to control cascading cuts.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#ifndef  COMMC_FIBONACCI_HEAP_H
#define  COMMC_FIBONACCI_HEAP_H

#include  <stddef.h>
#include  "error.h"

/*
	==================================
             --- STRUCTS ---
	==================================
*/

typedef struct commc_fibonacci_heap_t commc_fibonacci_heap_t;
typedef struct commc_fibonacci_node_t commc_fibonacci_node_t;

/*

         commc_fibonacci_heap_compare_func_t
	       ---
	       function pointer type for comparing fibonacci heap elements.
	       returns negative if a < b, zero if a == b, positive if a > b.
	       this determines the priority ordering in the heap.

*/

typedef int (*commc_fibonacci_heap_compare_func_t)(const void* a, const void* b);

/*
	==================================
             --- FUNCTIONS ---
	==================================
*/

/*

         commc_fibonacci_heap_create()
	       ---
	       creates a new empty fibonacci heap with the specified
	       comparison function for determining element priority.

*/

commc_fibonacci_heap_t* commc_fibonacci_heap_create(commc_fibonacci_heap_compare_func_t compare);

/*

         commc_fibonacci_heap_destroy()
	       ---
	       frees all memory associated with the fibonacci heap.
	       does not free the data pointed to by the elements.

*/

void commc_fibonacci_heap_destroy(commc_fibonacci_heap_t* heap);

/*

         commc_fibonacci_heap_insert()
	       ---
	       inserts an element into the fibonacci heap and returns
	       a node pointer that can be used for decrease-key operations.
	       this operation runs in O(1) amortized time.

*/

commc_fibonacci_node_t* commc_fibonacci_heap_insert(commc_fibonacci_heap_t* heap, void* element);

/*

         commc_fibonacci_heap_extract_min()
	       ---
	       removes and returns the minimum element from the heap.
	       this triggers heap consolidation and runs in O(log n)
	       amortized time.

*/

void* commc_fibonacci_heap_extract_min(commc_fibonacci_heap_t* heap);

/*

         commc_fibonacci_heap_peek_min()
	       ---
	       returns the minimum element without removing it from
	       the heap. runs in O(1) time.

*/

void* commc_fibonacci_heap_peek_min(commc_fibonacci_heap_t* heap);

/*

         commc_fibonacci_heap_decrease_key()
	       ---
	       decreases the key of a specific node and maintains
	       heap properties through cascading cuts if necessary.
	       this is the key advantage of fibonacci heaps, running
	       in O(1) amortized time.

*/

commc_error_t commc_fibonacci_heap_decrease_key(commc_fibonacci_heap_t* heap, commc_fibonacci_node_t* node, void* new_element);

/*

         commc_fibonacci_heap_delete()
	       ---
	       deletes a specific node from the fibonacci heap.
	       implemented by decreasing the key to negative infinity
	       (using a special sentinel value) then extracting min.

*/

commc_error_t commc_fibonacci_heap_delete(commc_fibonacci_heap_t* heap, commc_fibonacci_node_t* node);

/*

         commc_fibonacci_heap_merge()
	       ---
	       merges two fibonacci heaps into one. the second heap
	       becomes invalid after this operation. runs in O(1) time.

*/

commc_error_t commc_fibonacci_heap_merge(commc_fibonacci_heap_t* heap1, commc_fibonacci_heap_t* heap2);

/*

         commc_fibonacci_heap_size()
	       ---
	       returns the number of elements in the fibonacci heap.

*/

size_t commc_fibonacci_heap_size(commc_fibonacci_heap_t* heap);

/*

         commc_fibonacci_heap_is_empty()
	       ---
	       returns 1 if the fibonacci heap is empty, 0 otherwise.

*/

int commc_fibonacci_heap_is_empty(commc_fibonacci_heap_t* heap);

/*

         commc_fibonacci_heap_clear()
	       ---
	       removes all elements from the fibonacci heap.
	       the heap remains valid and can be reused.

*/

void commc_fibonacci_heap_clear(commc_fibonacci_heap_t* heap);

#endif /* COMMC_FIBONACCI_HEAP_H */

/*
	==================================
             --- EOF ---
	==================================
*/