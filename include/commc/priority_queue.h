/*
   ===================================
   C O M M O N - C
   PRIORITY QUEUE MODULE
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- PRIORITY QUEUE MODULE ---

    this module provides a priority queue data structure
    using a binary min-heap implementation. elements with
    smaller values have higher priority and are dequeued first.

    the binary heap is implemented as a dynamic array where
    for any node at index i:
    - left child is at index (2*i + 1)
    - right child is at index (2*i + 2)
    - parent is at index (i-1)/2

    this structure guarantees O(log n) insertion and extraction
    while maintaining the heap property: each parent node has
    a value less than or equal to its children.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#ifndef  COMMC_PRIORITY_QUEUE_H
#define  COMMC_PRIORITY_QUEUE_H

#include  <stddef.h>
#include  "error.h"

/*
	==================================
             --- STRUCTS ---
	==================================
*/

typedef struct commc_priority_queue_t commc_priority_queue_t;

/*

         commc_priority_queue_compare_func_t
	       ---
	       function pointer type for comparing priority queue elements.
	       returns negative if a < b, zero if a == b, positive if a > b.
	       this determines the priority ordering in the heap.

*/

typedef int (*commc_priority_queue_compare_func_t)(const void* a, const void* b);

/*
	==================================
             --- FUNCTIONS ---
	==================================
*/

/*

         commc_priority_queue_create()
	       ---
	       creates a new priority queue with the specified initial capacity
	       and comparison function. the queue will expand automatically
	       as needed when elements are added.

*/

commc_priority_queue_t* commc_priority_queue_create(size_t initial_capacity, commc_priority_queue_compare_func_t compare);

/*

         commc_priority_queue_destroy()
	       ---
	       frees all memory associated with the priority queue.
	       does not free the data pointed to by the elements.

*/

void commc_priority_queue_destroy(commc_priority_queue_t* pq);

/*

         commc_priority_queue_insert()
	       ---
	       inserts an element into the priority queue, maintaining
	       the heap property. the element is placed according to
	       its priority as determined by the comparison function.

*/

commc_error_t commc_priority_queue_insert(commc_priority_queue_t* pq, void* element);

/*

         commc_priority_queue_extract()
	       ---
	       removes and returns the highest priority element (minimum value)
	       from the priority queue. returns NULL if the queue is empty.

*/

void* commc_priority_queue_extract(commc_priority_queue_t* pq);

/*

         commc_priority_queue_peek()
	       ---
	       returns the highest priority element without removing it
	       from the queue. returns NULL if the queue is empty.

*/

void* commc_priority_queue_peek(commc_priority_queue_t* pq);

/*

         commc_priority_queue_size()
	       ---
	       returns the number of elements currently in the priority queue.

*/

size_t commc_priority_queue_size(commc_priority_queue_t* pq);

/*

         commc_priority_queue_capacity()
	       ---
	       returns the current capacity of the priority queue's
	       internal array. this may be larger than the size.

*/

size_t commc_priority_queue_capacity(commc_priority_queue_t* pq);

/*

         commc_priority_queue_is_empty()
	       ---
	       returns 1 if the priority queue is empty, 0 otherwise.

*/

int commc_priority_queue_is_empty(commc_priority_queue_t* pq);

/*

         commc_priority_queue_clear()
	       ---
	       removes all elements from the priority queue.
	       the queue remains valid and can be reused.

*/

void commc_priority_queue_clear(commc_priority_queue_t* pq);

#endif /* COMMC_PRIORITY_QUEUE_H */

/*
	==================================
             --- EOF ---
	==================================
*/