/* 	
   ===================================
   C O M M O N - C
   CIRCULAR BUFFER MODULE
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- CIRCULAR BUFFER MODULE ---

    this module provides a fixed-size circular buffer data structure
    (also known as ring buffer) that efficiently manages data
    in a first-in, first-out (FIFO) manner using a single contiguous
    memory block.
    
    the buffer automatically wraps around when it reaches the end,
    providing constant-time O(1) operations for all basic operations.
    this makes it ideal for streaming data, producer-consumer patterns,
    and situations where memory usage must be bounded.
    
    key advantages:
    - fixed memory footprint
    - cache-friendly sequential access  
    - lock-free single producer/single consumer
    - automatic overflow handling

*/

#ifndef COMMC_CIRCULAR_BUFFER_H
#define COMMC_CIRCULAR_BUFFER_H

/* 
	==================================
             --- INCLUDES ---
	==================================
*/

#include "commc/error.h"      /* ERROR HANDLING */
#include <stddef.h>           /* SIZE_T */

#ifdef __cplusplus
extern "C" {
#endif

/* 
	==================================
             --- CONSTANTS ---
	==================================
*/

/*

         COMMC_CIRCULAR_BUFFER_DEFAULT_CAPACITY
	       ---
	       default buffer capacity if not specified.
	       chosen to be power of 2 for efficient modulo operations.

*/

#define COMMC_CIRCULAR_BUFFER_DEFAULT_CAPACITY 1024

/* 
	==================================
             --- TYPES ---
	==================================
*/

/*

         commc_circular_buffer_overflow_policy_t
	       ---
	       defines behavior when buffer becomes full:
	       
	       OVERWRITE: new data overwrites oldest data
	       REJECT:    new data is rejected, operation fails
	       BLOCK:     operation waits for space (future use)

*/

typedef enum {

  COMMC_CIRCULAR_BUFFER_OVERWRITE = 0,
  COMMC_CIRCULAR_BUFFER_REJECT    = 1,
  COMMC_CIRCULAR_BUFFER_BLOCK     = 2
  
} commc_circular_buffer_overflow_policy_t;

/*

         commc_circular_buffer_t
	       ---
	       opaque circular buffer structure.
	       
	       implementation uses head/tail indices to track
	       buffer state without moving data, providing
	       efficient wraparound behavior.

*/

typedef struct commc_circular_buffer_t commc_circular_buffer_t;

/*

         commc_circular_buffer_iterator_t
	       ---
	       iterator for traversing buffer contents.
	       
	       iterates from oldest to newest element,
	       handling wraparound automatically.

*/

typedef struct {

  const commc_circular_buffer_t* buffer;    /* BUFFER REFERENCE */
  size_t                         position;  /* CURRENT POSITION */
  size_t                         count;     /* ELEMENTS REMAINING */
  
} commc_circular_buffer_iterator_t;

/* 
	==================================
             --- CORE API ---
	==================================
*/

/*

         commc_circular_buffer_create()
	       ---
	       creates circular buffer with default capacity.
	       
	       returns:
	       - pointer to new buffer, or NULL on error

*/

commc_circular_buffer_t* commc_circular_buffer_create(void);

/*

         commc_circular_buffer_create_with_capacity()
	       ---
	       creates circular buffer with specified capacity.
	       
	       parameters:
	       - capacity: maximum number of elements
	       - element_size: size of each element in bytes
	       
	       returns:
	       - pointer to new buffer, or NULL on error
	       
	       note: actual capacity may be rounded up to next power of 2
	       for performance optimization

*/

commc_circular_buffer_t* commc_circular_buffer_create_with_capacity(size_t capacity, 
                                                                    size_t element_size);

/*

         commc_circular_buffer_create_with_policy()
	       ---
	       creates buffer with custom overflow handling.
	       
	       parameters:
	       - capacity: maximum number of elements
	       - element_size: size of each element in bytes  
	       - policy: overflow behavior when buffer is full
	       
	       returns:
	       - pointer to new buffer, or NULL on error

*/

commc_circular_buffer_t* commc_circular_buffer_create_with_policy(size_t capacity,
                                                                  size_t element_size,
                                                                  commc_circular_buffer_overflow_policy_t policy);

/*

         commc_circular_buffer_destroy()
	       ---
	       destroys buffer and frees all associated memory.

*/

void commc_circular_buffer_destroy(commc_circular_buffer_t* buffer);

/*

         commc_circular_buffer_capacity()
	       ---
	       returns maximum buffer capacity.

*/

size_t commc_circular_buffer_capacity(const commc_circular_buffer_t* buffer);

/*

         commc_circular_buffer_size()
	       ---
	       returns current number of elements in buffer.

*/

size_t commc_circular_buffer_size(const commc_circular_buffer_t* buffer);

/*

         commc_circular_buffer_available_space()
	       ---
	       returns number of elements that can be added before full.

*/

size_t commc_circular_buffer_available_space(const commc_circular_buffer_t* buffer);

/*

         commc_circular_buffer_is_empty()
	       ---
	       returns 1 if buffer contains no elements, 0 otherwise.

*/

int commc_circular_buffer_is_empty(const commc_circular_buffer_t* buffer);

/*

         commc_circular_buffer_is_full()
	       ---
	       returns 1 if buffer is at maximum capacity, 0 otherwise.

*/

int commc_circular_buffer_is_full(const commc_circular_buffer_t* buffer);

/* 
	==================================
             --- MODIFICATION API ---
	==================================
*/

/*

         commc_circular_buffer_push()
	       ---
	       adds element to buffer tail.
	       
	       parameters:
	       - buffer: circular buffer instance
	       - data: pointer to element data to copy
	       
	       returns:
	       - COMMC_SUCCESS if element added
	       - COMMC_BUFFER_FULL if buffer full and policy is REJECT
	       - COMMC_ARGUMENT_ERROR for invalid parameters
	       
	       note: with OVERWRITE policy, always succeeds but may 
	       overwrite oldest data

*/

commc_error_t commc_circular_buffer_push(commc_circular_buffer_t* buffer, const void* data);

/*

         commc_circular_buffer_pop()
	       ---
	       removes element from buffer head.
	       
	       parameters:
	       - buffer: circular buffer instance
	       - data: pointer to storage for removed element (optional)
	       
	       returns:
	       - COMMC_SUCCESS if element removed
	       - COMMC_BUFFER_EMPTY if buffer is empty
	       - COMMC_ARGUMENT_ERROR for invalid parameters
	       
	       note: if data is NULL, element is discarded

*/

commc_error_t commc_circular_buffer_pop(commc_circular_buffer_t* buffer, void* data);

/*

         commc_circular_buffer_peek()
	       ---
	       examines element at buffer head without removing.
	       
	       parameters:
	       - buffer: circular buffer instance
	       - data: pointer to storage for peeked element
	       
	       returns:
	       - COMMC_SUCCESS if element copied
	       - COMMC_BUFFER_EMPTY if buffer is empty
	       - COMMC_ARGUMENT_ERROR for invalid parameters

*/

commc_error_t commc_circular_buffer_peek(const commc_circular_buffer_t* buffer, void* data);

/*

         commc_circular_buffer_peek_at()
	       ---
	       examines element at specific offset from head.
	       
	       parameters:
	       - buffer: circular buffer instance
	       - offset: position from head (0 = head element)
	       - data: pointer to storage for element
	       
	       returns:
	       - COMMC_SUCCESS if element copied
	       - COMMC_ARGUMENT_ERROR if offset out of bounds
	       
	       note: offset 0 is equivalent to peek()

*/

commc_error_t commc_circular_buffer_peek_at(const commc_circular_buffer_t* buffer,
                                            size_t offset, void* data);

/*

         commc_circular_buffer_clear()
	       ---
	       removes all elements from buffer.
	       resets to empty state without reallocating memory.

*/

void commc_circular_buffer_clear(commc_circular_buffer_t* buffer);

/* 
	==================================
             --- BULK OPERATIONS API ---
	==================================
*/

/*

         commc_circular_buffer_push_bulk()
	       ---
	       adds multiple elements efficiently.
	       
	       parameters:
	       - buffer: circular buffer instance
	       - data: pointer to array of elements
	       - count: number of elements to add
	       
	       returns:
	       - number of elements actually added
	       
	       note: may add fewer than requested if buffer becomes full
	       and overflow policy is REJECT

*/

size_t commc_circular_buffer_push_bulk(commc_circular_buffer_t* buffer, 
                                       const void* data, size_t count);

/*

         commc_circular_buffer_pop_bulk()
	       ---
	       removes multiple elements efficiently.
	       
	       parameters:
	       - buffer: circular buffer instance
	       - data: pointer to storage array (optional)
	       - count: maximum number of elements to remove
	       
	       returns:
	       - number of elements actually removed
	       
	       note: if data is NULL, elements are discarded

*/

size_t commc_circular_buffer_pop_bulk(commc_circular_buffer_t* buffer,
                                      void* data, size_t count);

/* 
	==================================
             --- ITERATION API ---
	==================================
*/

/*

         commc_circular_buffer_iterator_begin()
	       ---
	       creates iterator starting at buffer head (oldest element).

*/

commc_circular_buffer_iterator_t commc_circular_buffer_iterator_begin(const commc_circular_buffer_t* buffer);

/*

         commc_circular_buffer_iterator_next()
	       ---
	       advances iterator to next element.
	       
	       returns:
	       - COMMC_SUCCESS if advanced successfully
	       - COMMC_ITERATOR_END if no more elements

*/

commc_error_t commc_circular_buffer_iterator_next(commc_circular_buffer_iterator_t* iterator);

/*

         commc_circular_buffer_iterator_get()
	       ---
	       retrieves current element from iterator.
	       
	       parameters:
	       - iterator: iterator instance
	       - data: pointer to storage for element
	       
	       returns:
	       - COMMC_SUCCESS if element copied
	       - COMMC_ITERATOR_END if iterator exhausted

*/

commc_error_t commc_circular_buffer_iterator_get(const commc_circular_buffer_iterator_t* iterator, 
                                                  void* data);

/*

         commc_circular_buffer_iterator_has_next()
	       ---
	       returns 1 if iterator has more elements, 0 otherwise.

*/

int commc_circular_buffer_iterator_has_next(const commc_circular_buffer_iterator_t* iterator);

/* 
	==================================
             --- UTILITY API ---
	==================================
*/

/*

         commc_circular_buffer_get_overflow_policy()
	       ---
	       returns current overflow handling policy.

*/

commc_circular_buffer_overflow_policy_t commc_circular_buffer_get_overflow_policy(const commc_circular_buffer_t* buffer);

/*

         commc_circular_buffer_set_overflow_policy()
	       ---
	       updates overflow handling policy.
	       
	       returns:
	       - COMMC_SUCCESS if policy updated
	       - COMMC_ARGUMENT_ERROR for invalid policy

*/

commc_error_t commc_circular_buffer_set_overflow_policy(commc_circular_buffer_t* buffer,
                                                        commc_circular_buffer_overflow_policy_t policy);

/*

         commc_circular_buffer_element_size()
	       ---
	       returns size of each element in bytes.

*/

size_t commc_circular_buffer_element_size(const commc_circular_buffer_t* buffer);

/*

         commc_circular_buffer_memory_usage()
	       ---
	       estimates total memory usage in bytes.
	       includes buffer structure and data storage.

*/

size_t commc_circular_buffer_memory_usage(const commc_circular_buffer_t* buffer);

/*

         commc_circular_buffer_reset_statistics()
	       ---
	       resets internal performance counters (if enabled).
	       useful for benchmarking and profiling.

*/

void commc_circular_buffer_reset_statistics(commc_circular_buffer_t* buffer);

#ifdef __cplusplus
}
#endif

#endif /* COMMC_CIRCULAR_BUFFER_H */

/* 
	==================================
             --- EOF ---
	==================================
*/