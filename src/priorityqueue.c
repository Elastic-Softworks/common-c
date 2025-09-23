/*
   ===================================
   C O M M O N - C
   PRIORITY QUEUE IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- PRIORITY QUEUE MODULE ---

    implementation of the priority queue data structure.
    see include/commc/priority_queue.h for function
    prototypes and documentation.

    this implementation uses a binary min-heap stored
    in a dynamic array for optimal cache performance.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#include "commc/priorityqueue.h"
#include "commc/error.h"
#include <stdlib.h>
#include <string.h>

/*
	==================================
             --- CONSTANTS ---
	==================================
*/

#define PRIORITY_QUEUE_DEFAULT_CAPACITY 16
#define PRIORITY_QUEUE_GROWTH_FACTOR    2

/*
	==================================
             --- STRUCTS ---
	==================================
*/

/* internal priority queue structure. */

struct commc_priority_queue_t {

  void**                             elements;    /* array of element pointers */
  size_t                             size;        /* current number of elements */
  size_t                             capacity;    /* maximum elements without reallocation */
  commc_priority_queue_compare_func_t compare;    /* comparison function */

};

/*
	==================================
             --- STATIC FUNCS ---
	==================================
*/

/*

         parent_index()
	       ---
	       calculates the parent index of a given node in the heap.
	       in a zero-indexed array, parent of node i is at (i-1)/2.

*/

static size_t parent_index(size_t index) {

  return (index - 1) / 2;

}

/*

         left_child_index()
	       ---
	       calculates the left child index of a given node.
	       left child of node i is at (2*i + 1).

*/

static size_t left_child_index(size_t index) {

  return (2 * index) + 1;

}

/*

         right_child_index()
	       ---
	       calculates the right child index of a given node.
	       right child of node i is at (2*i + 2).

*/

static size_t right_child_index(size_t index) {

  return (2 * index) + 2;

}

/*

         swap_elements()
	       ---
	       swaps two elements in the priority queue array.
	       this is used during heap operations to maintain
	       the heap property.

*/

static void swap_elements(commc_priority_queue_t* pq, size_t index1, size_t index2) {

  void* temp = pq->elements[index1];
  pq->elements[index1] = pq->elements[index2];
  pq->elements[index2] = temp;

}

/*

         heapify_up()
	       ---
	       maintains the min-heap property by moving an element
	       up the tree until it finds its correct position.
	       used after insertion operations.

*/

static void heapify_up(commc_priority_queue_t* pq, size_t index) {

  size_t parent_idx;

  if  (index == 0) {

    return; /* already at root */

  }

  parent_idx = parent_index(index);

  /* if current element has higher priority than parent, swap them */

  if  (pq->compare(pq->elements[index], pq->elements[parent_idx]) < 0) {

    swap_elements(pq, index, parent_idx);
    heapify_up(pq, parent_idx); /* continue up the tree */

  }

}

/*

         heapify_down()
	       ---
	       maintains the min-heap property by moving an element
	       down the tree until it finds its correct position.
	       used after extraction operations.

*/

static void heapify_down(commc_priority_queue_t* pq, size_t index) {

  size_t left_idx  = left_child_index(index);
  size_t right_idx = right_child_index(index);
  size_t smallest  = index;

  /* find the element with highest priority among parent and children */

  if  (left_idx < pq->size && pq->compare(pq->elements[left_idx], pq->elements[smallest]) < 0) {

    smallest = left_idx;

  }

  if  (right_idx < pq->size && pq->compare(pq->elements[right_idx], pq->elements[smallest]) < 0) {

    smallest = right_idx;

  }

  /* if smallest is not the current index, swap and continue down */

  if  (smallest != index) {

    swap_elements(pq, index, smallest);
    heapify_down(pq, smallest);

  }

}

/*

         resize_if_needed()
	       ---
	       expands the priority queue capacity if needed.
	       doubles the capacity when the array becomes full.

*/

static commc_error_t resize_if_needed(commc_priority_queue_t* pq) {

  void** new_elements;

  if  (pq->size < pq->capacity) {

    return COMMC_SUCCESS; /* no resize needed */

  }

  new_elements = (void**)realloc(pq->elements, sizeof(void*) * pq->capacity * PRIORITY_QUEUE_GROWTH_FACTOR);

  if  (!new_elements) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return COMMC_MEMORY_ERROR;

  }

  pq->elements = new_elements;
  pq->capacity *= PRIORITY_QUEUE_GROWTH_FACTOR;

  return COMMC_SUCCESS;

}

/*
	==================================
             --- FUNCS ---
	==================================
*/

/*

         commc_priority_queue_create()
	       ---
	       allocates and initializes a new priority queue.
	       the comparison function determines element ordering.

*/

commc_priority_queue_t* commc_priority_queue_create(size_t initial_capacity, commc_priority_queue_compare_func_t compare) {

  commc_priority_queue_t* pq;

  if  (!compare || initial_capacity == 0) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  pq = (commc_priority_queue_t*)malloc(sizeof(commc_priority_queue_t));

  if  (!pq) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  pq->elements = (void**)malloc(sizeof(void*) * initial_capacity);

  if  (!pq->elements) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    free(pq);
    return NULL;

  }

  pq->size     = 0;
  pq->capacity = initial_capacity;
  pq->compare  = compare;

  return pq;

}

/*

         commc_priority_queue_destroy()
	       ---
	       frees all memory used by the priority queue structure.
	       does not free the actual elements, only the container.

*/

void commc_priority_queue_destroy(commc_priority_queue_t* pq) {

  if  (!pq) {

    return;

  }

  free(pq->elements);
  free(pq);

}

/*

         commc_priority_queue_insert()
	       ---
	       adds a new element to the priority queue and maintains
	       the heap property through heapify-up operations.

*/

commc_error_t commc_priority_queue_insert(commc_priority_queue_t* pq, void* element) {

  commc_error_t result;

  if  (!pq) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;

  }

  result = resize_if_needed(pq);

  if  (result != COMMC_SUCCESS) {

    return result;

  }

  /* insert element at the end and heapify up */
  pq->elements[pq->size] = element;
  heapify_up(pq, pq->size);
  pq->size++;

  return COMMC_SUCCESS;

}

/*

         commc_priority_queue_extract()
	       ---
	       removes and returns the highest priority element.
	       replaces root with last element and heapifies down.

*/

void* commc_priority_queue_extract(commc_priority_queue_t* pq) {

  void* root_element;

  if  (!pq || pq->size == 0) {

    if  (!pq) {
      commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    }
    return NULL;

  }

  root_element = pq->elements[0];

  /* move last element to root and decrease size */
  pq->size--;
  pq->elements[0] = pq->elements[pq->size];

  /* heapify down from root if heap is not empty */

  if  (pq->size > 0) {

    heapify_down(pq, 0);

  }

  return root_element;

}

/*

         commc_priority_queue_peek()
	       ---
	       returns the highest priority element without removing it.
	       the root of a min-heap always contains the minimum value.

*/

void* commc_priority_queue_peek(commc_priority_queue_t* pq) {

  if  (!pq || pq->size == 0) {

    if  (!pq) {
      commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    }
    return NULL;

  }

  return pq->elements[0];

}

/*

         commc_priority_queue_size()
	       ---
	       returns the current number of elements in the queue.

*/

size_t commc_priority_queue_size(commc_priority_queue_t* pq) {

  return pq ? pq->size : 0;

}

/*

         commc_priority_queue_capacity()
	       ---
	       returns the current capacity of the internal array.

*/

size_t commc_priority_queue_capacity(commc_priority_queue_t* pq) {

  return pq ? pq->capacity : 0;

}

/*

         commc_priority_queue_is_empty()
	       ---
	       checks if the priority queue contains any elements.

*/

int commc_priority_queue_is_empty(commc_priority_queue_t* pq) {

  return pq ? (pq->size == 0) : 1;

}

/*

         commc_priority_queue_clear()
	       ---
	       removes all elements from the priority queue.
	       capacity remains unchanged for efficient reuse.

*/

void commc_priority_queue_clear(commc_priority_queue_t* pq) {

  if  (!pq) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return;

  }

  pq->size = 0;

}

/*
	==================================
             --- EOF ---
	==================================
*/