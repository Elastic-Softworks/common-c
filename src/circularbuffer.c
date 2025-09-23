/* 	
   ===================================
   C O M M O N - C
   CIRCULAR BUFFER IMPLEMENTATION  
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- CIRCULAR BUFFER IMPLEMENTATION ---

    this implementation uses a contiguous memory block with head/tail
    indices to manage a fixed-size circular buffer. the buffer
    automatically wraps around using modulo arithmetic for efficient
    constant-time operations.
    
    key design decisions:
    - power-of-2 capacity for fast modulo using bitwise AND
    - separate read/write indices to avoid ambiguity  
    - overflow policies handle full buffer scenarios
    - bulk operations optimize memory copying

*/

/* 
	==================================
             --- INCLUDES ---
	==================================
*/

#include "commc/circularbuffer.h"
#include <stdlib.h>         /* MALLOC, FREE */
#include <string.h>         /* MEMCPY, MEMSET */

/* 
	==================================
             --- INTERNAL STRUCTURE ---
	==================================
*/

/*

         commc_circular_buffer_t
	       ---
	       internal circular buffer structure.
	       
	       uses head/tail indices instead of pointers to avoid
	       complications with wraparound and memory management.

*/

struct commc_circular_buffer_t {

  void*                                    data;           /* BUFFER STORAGE */
  size_t                                   capacity;       /* MAX ELEMENTS */
  size_t                                   element_size;   /* BYTES PER ELEMENT */
  size_t                                   head;           /* READ INDEX */
  size_t                                   tail;           /* WRITE INDEX */
  size_t                                   count;          /* CURRENT SIZE */
  size_t                                   mask;           /* CAPACITY - 1 FOR FAST MODULO */
  commc_circular_buffer_overflow_policy_t  policy;         /* OVERFLOW BEHAVIOR */
  
};

/* 
	==================================
             --- HELPER FUNCTIONS ---
	==================================
*/

/*

         next_power_of_2()
	       ---
	       rounds up to next power of 2 for efficient modulo.
	       uses bit manipulation for fast calculation.

*/

static size_t next_power_of_2(size_t n) {

  size_t power;
  
  if (n == 0) {

    return 1;
    
  }
  
  if ((n & (n - 1)) == 0) {

    return n; /* already power of 2 */
    
  }
  
  power = 1;
  
  while (power < n) {

    power <<= 1;
    
  }
  
  return power;
  
}

/*

         buffer_get_element_ptr()
	       ---
	       calculates pointer to element at given index.
	       handles wraparound using mask for efficiency.

*/

static void* buffer_get_element_ptr(const commc_circular_buffer_t* buffer, size_t index) {

  char* base_ptr;
  size_t wrapped_index;
  
  if (!buffer || !buffer->data) {

    return NULL;
    
  }
  
  base_ptr      = (char*)buffer->data;
  wrapped_index = index & buffer->mask;
  
  return base_ptr + (wrapped_index * buffer->element_size);
  
}

/* 
	==================================
             --- CORE API ---
	==================================
*/

/*

         commc_circular_buffer_create()
	       ---
	       creates circular buffer with default capacity.

*/

commc_circular_buffer_t* commc_circular_buffer_create(void) {

  return commc_circular_buffer_create_with_capacity(COMMC_CIRCULAR_BUFFER_DEFAULT_CAPACITY,
                                                    sizeof(void*));
  
}

/*

         commc_circular_buffer_create_with_capacity()
	       ---
	       creates circular buffer with specified capacity and element size.

*/

commc_circular_buffer_t* commc_circular_buffer_create_with_capacity(size_t capacity, 
                                                                    size_t element_size) {

  return commc_circular_buffer_create_with_policy(capacity, element_size,
                                                  COMMC_CIRCULAR_BUFFER_OVERWRITE);
  
}

/*

         commc_circular_buffer_create_with_policy()
	       ---
	       creates buffer with custom overflow handling.

*/

commc_circular_buffer_t* commc_circular_buffer_create_with_policy(size_t capacity,
                                                                  size_t element_size,
                                                                  commc_circular_buffer_overflow_policy_t policy) {

  commc_circular_buffer_t* buffer;
  size_t                   actual_capacity;
  
  if (capacity == 0 || element_size == 0) {

    return NULL;
    
  }
  
  buffer = (commc_circular_buffer_t*)malloc(sizeof(commc_circular_buffer_t));
  
  if (!buffer) {

    return NULL;
    
  }

  /* round capacity up to next power of 2 for efficient modulo */
  
  actual_capacity = next_power_of_2(capacity);
  
  buffer->data = malloc(actual_capacity * element_size);
  
  if (!buffer->data) {

    free(buffer);
    return NULL;
    
  }
  
  buffer->capacity     = actual_capacity;
  buffer->element_size = element_size;
  buffer->head         = 0;
  buffer->tail         = 0;
  buffer->count        = 0;
  buffer->mask         = actual_capacity - 1; /* for fast modulo with powers of 2 */
  buffer->policy       = policy;
  
  return buffer;
  
}

/*

         commc_circular_buffer_destroy()
	       ---
	       destroys buffer and frees all memory.

*/

void commc_circular_buffer_destroy(commc_circular_buffer_t* buffer) {

  if (!buffer) {

    return;
    
  }
  
  free(buffer->data);
  free(buffer);
  
}

/*

         commc_circular_buffer_capacity()
	       ---
	       returns maximum buffer capacity.

*/

size_t commc_circular_buffer_capacity(const commc_circular_buffer_t* buffer) {

  return buffer ? buffer->capacity : 0;
  
}

/*

         commc_circular_buffer_size()
	       ---
	       returns current number of elements.

*/

size_t commc_circular_buffer_size(const commc_circular_buffer_t* buffer) {

  return buffer ? buffer->count : 0;
  
}

/*

         commc_circular_buffer_available_space()
	       ---
	       returns available space before buffer becomes full.

*/

size_t commc_circular_buffer_available_space(const commc_circular_buffer_t* buffer) {

  if (!buffer) {

    return 0;
    
  }
  
  return buffer->capacity - buffer->count;
  
}

/*

         commc_circular_buffer_is_empty()
	       ---
	       returns 1 if buffer contains no elements.

*/

int commc_circular_buffer_is_empty(const commc_circular_buffer_t* buffer) {

  return buffer ? (buffer->count == 0) : 1;
  
}

/*

         commc_circular_buffer_is_full()
	       ---
	       returns 1 if buffer is at maximum capacity.

*/

int commc_circular_buffer_is_full(const commc_circular_buffer_t* buffer) {

  return buffer ? (buffer->count == buffer->capacity) : 0;
  
}

/* 
	==================================
             --- MODIFICATION API ---
	==================================
*/

/*

         commc_circular_buffer_push()
	       ---
	       adds element to buffer tail with overflow policy handling.

*/

commc_error_t commc_circular_buffer_push(commc_circular_buffer_t* buffer, const void* data) {

  void* dest_ptr;
  
  if (!buffer || !data) {

    return COMMC_ARGUMENT_ERROR;
    
  }

  /* check if buffer is full */
  
  if (buffer->count == buffer->capacity) {

    switch (buffer->policy) {

    case COMMC_CIRCULAR_BUFFER_REJECT:
      return COMMC_FAILURE; /* buffer full, operation rejected */
      
    case COMMC_CIRCULAR_BUFFER_OVERWRITE:

      /* advance head to overwrite oldest element */

      buffer->head = (buffer->head + 1) & buffer->mask;
      buffer->count--;
      break;
      
    case COMMC_CIRCULAR_BUFFER_BLOCK:

      /* future implementation for blocking behavior */
      
      return COMMC_FAILURE;
      
    default:
      return COMMC_ARGUMENT_ERROR;
      
    }
    
  }

  /* copy data to tail position */
  
  dest_ptr = buffer_get_element_ptr(buffer, buffer->tail);
  
  if (!dest_ptr) {

    return COMMC_MEMORY_ERROR;
    
  }
  
  memcpy(dest_ptr, data, buffer->element_size);

  /* advance tail and increment count */
  
  buffer->tail = (buffer->tail + 1) & buffer->mask;
  buffer->count++;
  
  return COMMC_SUCCESS;
  
}

/*

         commc_circular_buffer_pop()
	       ---
	       removes element from buffer head.

*/

commc_error_t commc_circular_buffer_pop(commc_circular_buffer_t* buffer, void* data) {

  void* src_ptr;
  
  if (!buffer) {

    return COMMC_ARGUMENT_ERROR;
    
  }
  
  if (buffer->count == 0) {

    return COMMC_FAILURE; /* buffer empty */
    
  }

  /* copy data from head position if storage provided */
  
  if (data) {

    src_ptr = buffer_get_element_ptr(buffer, buffer->head);
    
    if (!src_ptr) {

      return COMMC_MEMORY_ERROR;
      
    }
    
    memcpy(data, src_ptr, buffer->element_size);
    
  }

  /* advance head and decrement count */
  
  buffer->head = (buffer->head + 1) & buffer->mask;
  buffer->count--;
  
  return COMMC_SUCCESS;
  
}

/*

         commc_circular_buffer_peek()
	       ---
	       examines head element without removing it.

*/

commc_error_t commc_circular_buffer_peek(const commc_circular_buffer_t* buffer, void* data) {

  void* src_ptr;
  
  if (!buffer || !data) {

    return COMMC_ARGUMENT_ERROR;
    
  }
  
  if (buffer->count == 0) {

    return COMMC_FAILURE; /* buffer empty */
    
  }
  
  src_ptr = buffer_get_element_ptr(buffer, buffer->head);
  
  if (!src_ptr) {

    return COMMC_MEMORY_ERROR;
    
  }
  
  memcpy(data, src_ptr, buffer->element_size);
  
  return COMMC_SUCCESS;
  
}

/*

         commc_circular_buffer_peek_at()
	       ---
	       examines element at specific offset from head.

*/

commc_error_t commc_circular_buffer_peek_at(const commc_circular_buffer_t* buffer,
                                            size_t offset, void* data) {

  void* src_ptr;
  size_t actual_index;
  
  if (!buffer || !data) {

    return COMMC_ARGUMENT_ERROR;
    
  }
  
  if (offset >= buffer->count) {

    return COMMC_ARGUMENT_ERROR;
    
  }
  
  actual_index = (buffer->head + offset) & buffer->mask;
  src_ptr = buffer_get_element_ptr(buffer, actual_index);
  
  if (!src_ptr) {

    return COMMC_MEMORY_ERROR;
    
  }
  
  memcpy(data, src_ptr, buffer->element_size);
  
  return COMMC_SUCCESS;
  
}

/*

         commc_circular_buffer_clear()
	       ---
	       removes all elements by resetting indices.

*/

void commc_circular_buffer_clear(commc_circular_buffer_t* buffer) {

  if (!buffer) {

    return;
    
  }
  
  buffer->head  = 0;
  buffer->tail  = 0;
  buffer->count = 0;
  
}

/* 
	==================================
             --- BULK OPERATIONS API ---
	==================================
*/

/*

         commc_circular_buffer_push_bulk()
	       ---
	       adds multiple elements efficiently.

*/

size_t commc_circular_buffer_push_bulk(commc_circular_buffer_t* buffer, 
                                       const void* data, size_t count) {

  const char* src_ptr;
  void*       dest_ptr;
  size_t      elements_added;
  size_t      available;
  size_t      i;
  
  if (!buffer || !data || count == 0) {

    return 0;
    
  }
  
  src_ptr        = (const char*)data;
  elements_added = 0;

  /* process elements one by one to handle capacity correctly */
  
  for (i = 0; i < count; i++) {

    available = buffer->capacity - buffer->count;
    
    /* check if we can add another element */
    
    if (available == 0 && buffer->policy == COMMC_CIRCULAR_BUFFER_REJECT) {

      break; /* no more space with REJECT policy */
      
    }

    /* handle overwrite policy */
    
    if (available == 0 && buffer->policy == COMMC_CIRCULAR_BUFFER_OVERWRITE) {

      buffer->head = (buffer->head + 1) & buffer->mask;
      buffer->count--;
      
    }

    dest_ptr = buffer_get_element_ptr(buffer, buffer->tail);
    
    if (!dest_ptr) {

      break; /* stop on error */
      
    }
    
    memcpy(dest_ptr, src_ptr, buffer->element_size);
    
    src_ptr += buffer->element_size;
    buffer->tail = (buffer->tail + 1) & buffer->mask;
    buffer->count++;
    elements_added++;
    
    if (buffer->count > buffer->capacity) {

      buffer->count = buffer->capacity; /* safety check */
      
    }
    
  }
  
  return elements_added;
  
}

/*

         commc_circular_buffer_pop_bulk()
	       ---
	       removes multiple elements efficiently.

*/

size_t commc_circular_buffer_pop_bulk(commc_circular_buffer_t* buffer,
                                      void* data, size_t count) {

  char*  dest_ptr;
  void*  src_ptr;
  size_t elements_removed;
  size_t to_remove;
  size_t i;
  
  if (!buffer || count == 0) {

    return 0;
    
  }
  
  dest_ptr        = (char*)data;
  elements_removed = 0;
  to_remove        = (count <= buffer->count) ? count : buffer->count;

  /* remove elements one by one to handle wraparound */
  
  for (i = 0; i < to_remove; i++) {

    src_ptr = buffer_get_element_ptr(buffer, buffer->head);
    
    if (!src_ptr) {

      break; /* stop on error */
      
    }

    /* copy data if storage provided */
    
    if (dest_ptr) {

      memcpy(dest_ptr, src_ptr, buffer->element_size);
      dest_ptr += buffer->element_size;
      
    }
    
    buffer->head = (buffer->head + 1) & buffer->mask;
    buffer->count--;
    elements_removed++;
    
  }
  
  return elements_removed;
  
}

/* 
	==================================
             --- ITERATION API ---
	==================================
*/

/*

         commc_circular_buffer_iterator_begin()
	       ---
	       creates iterator starting at buffer head.

*/

commc_circular_buffer_iterator_t commc_circular_buffer_iterator_begin(const commc_circular_buffer_t* buffer) {

  commc_circular_buffer_iterator_t iterator;
  
  iterator.buffer   = buffer;
  iterator.position = buffer ? buffer->head : 0;
  iterator.count    = buffer ? buffer->count : 0;
  
  return iterator;
  
}

/*

         commc_circular_buffer_iterator_next()
	       ---
	       advances iterator to next element.

*/

commc_error_t commc_circular_buffer_iterator_next(commc_circular_buffer_iterator_t* iterator) {

  if (!iterator || !iterator->buffer) {

    return COMMC_FAILURE; /* invalid iterator */
    
  }
  
  if (iterator->count == 0) {

    return COMMC_FAILURE; /* iterator already exhausted */
    
  }
  
  iterator->position = (iterator->position + 1) & iterator->buffer->mask;
  iterator->count--;
  
  /* return failure if we've exhausted the iterator */
  
  return (iterator->count == 0) ? COMMC_FAILURE : COMMC_SUCCESS;
  
}

/*

         commc_circular_buffer_iterator_get()
	       ---
	       retrieves current element from iterator.

*/

commc_error_t commc_circular_buffer_iterator_get(const commc_circular_buffer_iterator_t* iterator, 
                                                  void* data) {

  void* src_ptr;
  
  if (!iterator || !iterator->buffer || !data || iterator->count == 0) {

    return COMMC_FAILURE; /* iterator exhausted or invalid */
    
  }
  
  src_ptr = buffer_get_element_ptr(iterator->buffer, iterator->position);
  
  if (!src_ptr) {

    return COMMC_MEMORY_ERROR;
    
  }
  
  memcpy(data, src_ptr, iterator->buffer->element_size);
  
  return COMMC_SUCCESS;
  
}

/*

         commc_circular_buffer_iterator_has_next()
	       ---
	       returns 1 if iterator has more elements.

*/

int commc_circular_buffer_iterator_has_next(const commc_circular_buffer_iterator_t* iterator) {

  return (iterator && iterator->buffer && iterator->count > 0) ? 1 : 0;
  
}

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

commc_circular_buffer_overflow_policy_t commc_circular_buffer_get_overflow_policy(const commc_circular_buffer_t* buffer) {

  return buffer ? buffer->policy : COMMC_CIRCULAR_BUFFER_REJECT;
  
}

/*

         commc_circular_buffer_set_overflow_policy()
	       ---
	       updates overflow handling policy.

*/

commc_error_t commc_circular_buffer_set_overflow_policy(commc_circular_buffer_t* buffer,
                                                        commc_circular_buffer_overflow_policy_t policy) {

  if (!buffer) {

    return COMMC_ARGUMENT_ERROR;
    
  }
  
  if (policy != COMMC_CIRCULAR_BUFFER_OVERWRITE && 
      policy != COMMC_CIRCULAR_BUFFER_REJECT &&
      policy != COMMC_CIRCULAR_BUFFER_BLOCK) {

    return COMMC_ARGUMENT_ERROR;
    
  }
  
  buffer->policy = policy;
  
  return COMMC_SUCCESS;
  
}

/*

         commc_circular_buffer_element_size()
	       ---
	       returns size of each element in bytes.

*/

size_t commc_circular_buffer_element_size(const commc_circular_buffer_t* buffer) {

  return buffer ? buffer->element_size : 0;
  
}

/*

         commc_circular_buffer_memory_usage()
	       ---
	       estimates total memory usage.

*/

size_t commc_circular_buffer_memory_usage(const commc_circular_buffer_t* buffer) {

  if (!buffer) {

    return 0;
    
  }
  
  return sizeof(commc_circular_buffer_t) + 
         (buffer->capacity * buffer->element_size);
  
}

/*

         commc_circular_buffer_reset_statistics()
	       ---
	       resets performance counters (placeholder for future use).

*/

void commc_circular_buffer_reset_statistics(commc_circular_buffer_t* buffer) {

  /* placeholder for future statistics tracking */
  
  (void)buffer; /* suppress unused parameter warning */
  
}

/* 
	==================================
             --- EOF ---
	==================================
*/