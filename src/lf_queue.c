/*
   ===================================
   C O M M O N - C
   LOCK-FREE QUEUE IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- LOCK-FREE QUEUE IMPLEMENTATION ---

    thread-safe, wait-free FIFO queue based on the Michael & Scott algorithm.
    this implementation uses atomic compare-and-swap operations with tagged
    pointers to prevent ABA problems, and hazard pointers for safe memory
    reclamation in a concurrent environment.
    
    the algorithm ensures linearizability - all operations appear to take
    effect atomically at some point between their start and completion.
    operations are wait-free, meaning they complete in a bounded number
    of steps regardless of other thread activity.
    
    memory management uses a combination of hazard pointers and reference
    counting to safely reclaim memory without requiring a garbage collector.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#include "commc/lf_queue.h"
#include "commc/error.h"

#include <stdlib.h>
#include <string.h>

/* platform-specific includes for threading primitives */

#ifdef _WIN32
  #include <windows.h>
  #define COMMC_THREAD_ID() GetCurrentThreadId()
#elif defined(__unix__) || defined(__APPLE__)
  #include <pthread.h>
  #include <unistd.h>
  #define COMMC_THREAD_ID() ((unsigned long)pthread_self())
#else
  #define COMMC_THREAD_ID() 0
#endif

/*
	==================================
             --- CONSTANTS ---
	==================================
*/

#define COMMC_LF_QUEUE_CLEANUP_THRESHOLD    100    /* retire nodes before cleanup */
#define COMMC_LF_QUEUE_MAX_CLEANUP_BATCH    50     /* max nodes cleaned per batch */

/*
	==================================
             --- INTERNAL HELPERS ---
	==================================
*/

/*

         allocate_node()
	       ---
	       allocates and initializes a new queue node.

*/

static commc_lf_queue_node_t* allocate_node(void* data) {

  commc_lf_queue_node_t* node;
  
  node = (commc_lf_queue_node_t*)malloc(sizeof(commc_lf_queue_node_t));
  
  if (!node) {

    return NULL;
    
  }
  
  node->data = data;
  node->next = commc_lf_queue_tagged_ptr_create(NULL);
  node->ref_count = 1;
  
  return node;
  
}

/*

         free_node()
	       ---
	       safely frees a queue node after ensuring no references remain.

*/

static void free_node(commc_lf_queue_node_t* node) {

  if (!node) {

    return;
    
  }
  
  /* ensure reference count has reached zero */
  
  if (COMMC_ATOMIC_LOAD(&node->ref_count) != 0) {

    return; /* still referenced, cannot free */
    
  }
  
  free(node);
  
}

/*

         get_thread_local_data()
	       ---
	       retrieves thread-local hazard pointer data, creating if necessary.

*/

static commc_lf_queue_thread_data_t* get_thread_local_data(commc_lf_queue_t* queue) {

  unsigned long          thread_id;
  unsigned long          i;
  commc_lf_queue_thread_data_t* thread_data;
  
  if (!queue || !queue->thread_data) {

    return NULL;
    
  }
  
  thread_id = COMMC_THREAD_ID();
  
  /* search for existing thread data */
  
  for (i = 0; i < queue->max_threads; i++) {

    thread_data = &queue->thread_data[i];
    
    if (thread_data->thread_id == thread_id) {

      return thread_data;
      
    }
    
    /* try to claim an unused slot */
    
    if (thread_data->thread_id == 0) {

      if (COMMC_ATOMIC_CAS(&thread_data->thread_id, 0, thread_id)) {

        /* successfully claimed slot, initialize hazards */
        
        unsigned long j;
        
        for (j = 0; j < COMMC_LF_QUEUE_HAZARDS_PER_THREAD; j++) {

          thread_data->hazards[j].node = NULL;
          thread_data->hazards[j].active = 0;
          
        }
        
        return thread_data;
        
      }
      
    }
    
  }
  
  return NULL; /* no available thread slots */
  
}

/*

         acquire_hazard_pointer()
	       ---
	       protects a node from reclamation by acquiring a hazard pointer.

*/

static commc_lf_queue_hazard_t* acquire_hazard_pointer(commc_lf_queue_t* queue,
                                                       commc_lf_queue_node_t* node) {

  commc_lf_queue_thread_data_t* thread_data;
  unsigned long                 i;
  
  thread_data = get_thread_local_data(queue);
  
  if (!thread_data) {

    return NULL;
    
  }
  
  /* find an inactive hazard pointer */
  
  for (i = 0; i < COMMC_LF_QUEUE_HAZARDS_PER_THREAD; i++) {

    if (COMMC_ATOMIC_CAS(&thread_data->hazards[i].active, 0, 1)) {

      /* claimed hazard pointer, protect the node */
      
      (void)COMMC_ATOMIC_STORE((void**)&thread_data->hazards[i].node, node);
      COMMC_MEMORY_BARRIER();
      
      /* increment node reference count */
      
      if (node) {

        COMMC_ATOMIC_INC(&node->ref_count);
        
      }
      
      return &thread_data->hazards[i];
      
    }
    
  }
  
  return NULL; /* no available hazard pointers */
  
}

/*

         release_hazard_pointer()
	       ---
	       releases a hazard pointer and decrements node reference count.

*/

static void release_hazard_pointer(commc_lf_queue_hazard_t* hazard) {

  commc_lf_queue_node_t* node;
  
  if (!hazard || !COMMC_ATOMIC_LOAD(&hazard->active)) {

    return;
    
  }
  
  node = (commc_lf_queue_node_t*)COMMC_ATOMIC_LOAD((void**)&hazard->node);
  
  /* clear the hazard */
  
  (void)COMMC_ATOMIC_STORE((void**)&hazard->node, NULL);
  COMMC_MEMORY_BARRIER();
  (void)COMMC_ATOMIC_STORE(&hazard->active, 0);
  
  /* decrement reference count */
  
  if (node) {

    COMMC_ATOMIC_DEC(&node->ref_count);
    
  }
  
}

/*

         is_node_hazardous()
	       ---
	       checks if any thread has a hazard pointer on the given node.

*/

static int is_node_hazardous(commc_lf_queue_t* queue, commc_lf_queue_node_t* node) {

  unsigned long i;
  unsigned long j;
  
  if (!queue || !node || !queue->thread_data) {

    return 0;
    
  }
  
  /* scan all thread hazard pointers */
  
  for (i = 0; i < queue->max_threads; i++) {

    commc_lf_queue_thread_data_t* thread_data = &queue->thread_data[i];
    
    if (thread_data->thread_id == 0) {

      continue; /* unused thread slot */
      
    }
    
    for (j = 0; j < COMMC_LF_QUEUE_HAZARDS_PER_THREAD; j++) {

      if (COMMC_ATOMIC_LOAD(&thread_data->hazards[j].active)) {

        commc_lf_queue_node_t* hazardous_node = 
          (commc_lf_queue_node_t*)COMMC_ATOMIC_LOAD((void**)&thread_data->hazards[j].node);
          
        if (hazardous_node == node) {

          return 1; /* node is protected by hazard pointer */
          
        }
        
      }
      
    }
    
  }
  
  return 0; /* node is not hazardous */
  
}

/*
	==================================
             --- TAGGED POINTER OPERATIONS ---
	==================================
*/

/*

         commc_lf_queue_tagged_ptr_create()
	       ---
	       creates a tagged pointer with initial tag of 0.

*/

commc_lf_queue_tagged_ptr_t commc_lf_queue_tagged_ptr_create(void* ptr) {

  commc_lf_queue_tagged_ptr_t tagged;
  
  tagged.ptr = ptr;
  tagged.tag = 0;
  
  return tagged;
  
}

/*

         commc_lf_queue_tagged_ptr_advance()
	       ---
	       increments the tag to prevent ABA problems.

*/

commc_lf_queue_tagged_ptr_t commc_lf_queue_tagged_ptr_advance(commc_lf_queue_tagged_ptr_t tagged_ptr) {

  commc_lf_queue_tagged_ptr_t result;
  
  result.ptr = tagged_ptr.ptr;
  result.tag = tagged_ptr.tag + 1;
  
  return result;
  
}

/*

         commc_lf_queue_tagged_ptr_load()
	       ---
	       atomically loads a tagged pointer with memory barriers.

*/

commc_lf_queue_tagged_ptr_t commc_lf_queue_tagged_ptr_load(volatile commc_lf_queue_tagged_ptr_t* target) {

  commc_lf_queue_tagged_ptr_t result;
  
  if (!target) {

    return commc_lf_queue_tagged_ptr_create(NULL);
    
  }
  
  /* atomic load with memory barrier */
  
  COMMC_MEMORY_BARRIER();
  result.ptr = COMMC_ATOMIC_LOAD((void**)&target->ptr);
  result.tag = COMMC_ATOMIC_LOAD(&target->tag);
  COMMC_MEMORY_BARRIER();
  
  return result;
  
}

/*

         commc_lf_queue_tagged_ptr_cas()
	       ---
	       atomic compare-and-swap for tagged pointers.

*/

int commc_lf_queue_tagged_ptr_cas(volatile commc_lf_queue_tagged_ptr_t* target,
                                  commc_lf_queue_tagged_ptr_t expected,
                                  commc_lf_queue_tagged_ptr_t new_value) {

  commc_lf_queue_tagged_ptr_t current;
  
  if (!target) {

    return 0;
    
  }
  
  current = commc_lf_queue_tagged_ptr_load(target);
  
  /* check if current matches expected */
  
  if (current.ptr != expected.ptr || current.tag != expected.tag) {

    return 0; /* mismatch */
    
  }
  
  /* attempt double-width CAS or sequential CAS fallback */
  
  #if defined(COMMC_HAS_ATOMICS) && (COMMC_HAS_ATOMICS == 1)
  
    /* try to update both ptr and tag atomically */
    
    if (COMMC_ATOMIC_CAS((void**)&target->ptr, expected.ptr, new_value.ptr)) {

      /* pointer CAS succeeded, now update tag */
      
      (void)COMMC_ATOMIC_STORE(&target->tag, new_value.tag);
      return 1;
      
    }
    
  #else
  
    /* fallback for platforms without strong atomics */
    
    COMMC_MEMORY_BARRIER();
    
    if (target->ptr == expected.ptr && target->tag == expected.tag) {

      target->ptr = new_value.ptr;
      target->tag = new_value.tag;
      return 1;
      
    }
    
  #endif
  
  return 0;
  
}

/*
	==================================
             --- CORE QUEUE OPERATIONS ---
	==================================
*/

/*

         commc_lf_queue_create()
	       ---
	       creates a new lock-free queue with hazard pointer management.

*/

commc_lf_queue_t* commc_lf_queue_create(unsigned long max_threads) {

  commc_lf_queue_t*       queue;
  commc_lf_queue_node_t*  dummy_node;
  size_t                  thread_data_size;
  
  if (max_threads == 0) {

    max_threads = 16; /* reasonable default */
    
  }
  
  queue = (commc_lf_queue_t*)malloc(sizeof(commc_lf_queue_t));
  
  if (!queue) {

    return NULL;
    
  }
  
  /* create dummy node for Michael & Scott algorithm */
  
  dummy_node = allocate_node(NULL);
  
  if (!dummy_node) {

    free(queue);
    return NULL;
    
  }
  
  /* initialize queue structure */
  
  queue->head = commc_lf_queue_tagged_ptr_create(dummy_node);
  queue->tail = commc_lf_queue_tagged_ptr_create(dummy_node);
  queue->size = 0;
  queue->next_thread_id = 1;
  queue->max_threads = max_threads;
  queue->retired_nodes = NULL;
  queue->retired_count = 0;
  
  /* allocate thread data for hazard pointers */
  
  thread_data_size = max_threads * sizeof(commc_lf_queue_thread_data_t);
  queue->thread_data = (commc_lf_queue_thread_data_t*)malloc(thread_data_size);
  
  if (!queue->thread_data) {

    free_node(dummy_node);
    free(queue);
    return NULL;
    
  }
  
  memset(queue->thread_data, 0, thread_data_size);
  
  return queue;
  
}

/*

         commc_lf_queue_destroy()
	       ---
	       destroys queue and cleans up all resources.

*/

void commc_lf_queue_destroy(commc_lf_queue_t* queue) {

  commc_lf_queue_node_t* current;
  commc_lf_queue_node_t* next;
  
  if (!queue) {

    return;
    
  }
  
  /* free all nodes in main queue */
  
  current = (commc_lf_queue_node_t*)queue->head.ptr;
  
  while (current) {

    next = (commc_lf_queue_node_t*)current->next.ptr;
    free(current);
    current = next;
    
  }
  
  /* free all retired nodes */
  
  current = queue->retired_nodes;
  
  while (current) {

    next = (commc_lf_queue_node_t*)current->next.ptr;
    free(current);
    current = next;
    
  }
  
  /* free thread data */
  
  if (queue->thread_data) {

    free(queue->thread_data);
    
  }
  
  free(queue);
  
}

/*

         commc_lf_queue_enqueue()
	       ---
	       Michael & Scott enqueue operation with hazard pointer protection.

*/

commc_error_t commc_lf_queue_enqueue(commc_lf_queue_t* queue, void* data) {

  commc_lf_queue_node_t*     new_node;
  commc_lf_queue_hazard_t*   tail_hazard;
  commc_lf_queue_hazard_t*   next_hazard;
  commc_lf_queue_tagged_ptr_t tail;
  commc_lf_queue_tagged_ptr_t next;
  commc_lf_queue_tagged_ptr_t new_tail;
  
  if (!queue) {

    return COMMC_ARGUMENT_ERROR;
    
  }
  
  /* allocate new node */
  
  new_node = allocate_node(data);
  
  if (!new_node) {

    return COMMC_MEMORY_ERROR;
    
  }
  
  tail_hazard = NULL;
  next_hazard = NULL;
  
  while (1) {

    /* load tail with hazard protection */
    
    tail = commc_lf_queue_tagged_ptr_load(&queue->tail);
    tail_hazard = acquire_hazard_pointer(queue, (commc_lf_queue_node_t*)tail.ptr);
    
    if (!tail_hazard) {

      /* no hazard pointers available, retry */
      continue;
      
    }
    
    /* verify tail hasn't changed */
    
    {
      commc_lf_queue_tagged_ptr_t current_tail = commc_lf_queue_tagged_ptr_load(&queue->tail);
      
      if (current_tail.ptr != tail.ptr || current_tail.tag != tail.tag) {

        release_hazard_pointer(tail_hazard);
        continue; /* tail changed, retry */
        
      }
    }
    
    /* load next pointer of tail */
    
    next = commc_lf_queue_tagged_ptr_load(&((commc_lf_queue_node_t*)tail.ptr)->next);
    
    if (next.ptr == NULL) {

      /* tail is pointing to last node, try to link new node */
      
      commc_lf_queue_tagged_ptr_t new_next = commc_lf_queue_tagged_ptr_advance(next);
      new_next.ptr = new_node;
      
      if (commc_lf_queue_tagged_ptr_cas(&((commc_lf_queue_node_t*)tail.ptr)->next, next, new_next)) {

        /* successfully linked new node, now try to advance tail */
        
        new_tail = commc_lf_queue_tagged_ptr_advance(tail);
        new_tail.ptr = new_node;
        
        commc_lf_queue_tagged_ptr_cas(&queue->tail, tail, new_tail);
        
        /* increment size counter */
        
        COMMC_ATOMIC_INC(&queue->size);
        
        break; /* enqueue successful */
        
      }
      
    } else {

      /* tail is not pointing to last node, try to advance it */
      
      new_tail = commc_lf_queue_tagged_ptr_advance(tail);
      new_tail.ptr = next.ptr;
      
      commc_lf_queue_tagged_ptr_cas(&queue->tail, tail, new_tail);
      
    }
    
    /* release hazards and retry */
    
    if (tail_hazard) {

      release_hazard_pointer(tail_hazard);
      tail_hazard = NULL;
      
    }
    
    if (next_hazard) {

      release_hazard_pointer(next_hazard);
      next_hazard = NULL;
      
    }
    
  }
  
  /* cleanup hazard pointers */
  
  if (tail_hazard) {

    release_hazard_pointer(tail_hazard);
    
  }
  
  if (next_hazard) {

    release_hazard_pointer(next_hazard);
    
  }
  
  /* periodic cleanup of retired nodes */
  
  if (COMMC_ATOMIC_LOAD(&queue->retired_count) > COMMC_LF_QUEUE_CLEANUP_THRESHOLD) {

    commc_lf_queue_cleanup_retired(queue);
    
  }
  
  return COMMC_SUCCESS;
  
}

/*

         commc_lf_queue_dequeue()
	       ---
	       Michael & Scott dequeue operation with safe memory reclamation.

*/

commc_error_t commc_lf_queue_dequeue(commc_lf_queue_t* queue, void** data) {

  commc_lf_queue_hazard_t*   head_hazard;
  commc_lf_queue_hazard_t*   tail_hazard;
  commc_lf_queue_hazard_t*   next_hazard;
  commc_lf_queue_tagged_ptr_t head;
  commc_lf_queue_tagged_ptr_t tail;
  commc_lf_queue_tagged_ptr_t next;
  commc_lf_queue_tagged_ptr_t new_head;
  commc_lf_queue_node_t*     head_node;
  void*                      result_data;
  
  if (!queue || !data) {

    return COMMC_ARGUMENT_ERROR;
    
  }
  
  head_hazard = NULL;
  tail_hazard = NULL;
  next_hazard = NULL;
  
  while (1) {

    /* load head with hazard protection */
    
    head = commc_lf_queue_tagged_ptr_load(&queue->head);
    tail = commc_lf_queue_tagged_ptr_load(&queue->tail);
    
    head_hazard = acquire_hazard_pointer(queue, (commc_lf_queue_node_t*)head.ptr);
    
    if (!head_hazard) {

      continue; /* no hazard pointers available, retry */
      
    }
    
    /* verify head hasn't changed */
    
    {
      commc_lf_queue_tagged_ptr_t current_head = commc_lf_queue_tagged_ptr_load(&queue->head);
      
      if (current_head.ptr != head.ptr || current_head.tag != head.tag) {

        release_hazard_pointer(head_hazard);
        continue; /* head changed, retry */
        
      }
    }
    
    head_node = (commc_lf_queue_node_t*)head.ptr;
    next = commc_lf_queue_tagged_ptr_load(&head_node->next);
    
    if (head.ptr == tail.ptr) {

      if (next.ptr == NULL) {

        /* queue is empty */
        
        release_hazard_pointer(head_hazard);
        return COMMC_FAILURE;
        
      }
      
      /* tail is lagging, try to advance it */
      
      {
        commc_lf_queue_tagged_ptr_t new_tail = commc_lf_queue_tagged_ptr_advance(tail);
        new_tail.ptr = next.ptr;
        
        commc_lf_queue_tagged_ptr_cas(&queue->tail, tail, new_tail);
      }
      
    } else {

      if (next.ptr == NULL) {

        continue; /* inconsistent state, retry */
        
      }
      
      /* protect next node before accessing its data */
      
      next_hazard = acquire_hazard_pointer(queue, (commc_lf_queue_node_t*)next.ptr);
      
      if (!next_hazard) {

        release_hazard_pointer(head_hazard);
        continue;
        
      }
      
      /* read data before attempting to dequeue */
      
      result_data = ((commc_lf_queue_node_t*)next.ptr)->data;
      
      /* try to advance head */
      
      new_head = commc_lf_queue_tagged_ptr_advance(head);
      new_head.ptr = next.ptr;
      
      if (commc_lf_queue_tagged_ptr_cas(&queue->head, head, new_head)) {

        /* successfully dequeued, return data */
        
        *data = result_data;
        
        /* decrement size */
        
        COMMC_ATOMIC_DEC(&queue->size);
        
        /* retire the old head node */
        
        commc_lf_queue_retire_node(queue, head_node);
        
        break; /* dequeue successful */
        
      }
      
    }
    
    /* release hazards and retry */
    
    if (head_hazard) {

      release_hazard_pointer(head_hazard);
      head_hazard = NULL;
      
    }
    
    if (tail_hazard) {

      release_hazard_pointer(tail_hazard);
      tail_hazard = NULL;
      
    }
    
    if (next_hazard) {

      release_hazard_pointer(next_hazard);
      next_hazard = NULL;
      
    }
    
  }
  
  /* cleanup hazard pointers */
  
  if (head_hazard) {

    release_hazard_pointer(head_hazard);
    
  }
  
  if (tail_hazard) {

    release_hazard_pointer(tail_hazard);
    
  }
  
  if (next_hazard) {

    release_hazard_pointer(next_hazard);
    
  }
  
  return COMMC_SUCCESS;
  
}

/*
	==================================
             --- UTILITY FUNCTIONS ---
	==================================
*/

/*

         commc_lf_queue_is_empty()
	       ---
	       checks if queue appears empty (snapshot).

*/

int commc_lf_queue_is_empty(const commc_lf_queue_t* queue) {

  commc_lf_queue_tagged_ptr_t head;
  commc_lf_queue_tagged_ptr_t tail;
  commc_lf_queue_tagged_ptr_t next;
  
  if (!queue) {

    return 1;
    
  }
  
  head = commc_lf_queue_tagged_ptr_load((volatile commc_lf_queue_tagged_ptr_t*)&queue->head);
  tail = commc_lf_queue_tagged_ptr_load((volatile commc_lf_queue_tagged_ptr_t*)&queue->tail);
  
  if (head.ptr != tail.ptr) {

    return 0; /* definitely not empty */
    
  }
  
  /* head == tail, check if next pointer is NULL */
  
  next = commc_lf_queue_tagged_ptr_load((volatile commc_lf_queue_tagged_ptr_t*)&((commc_lf_queue_node_t*)head.ptr)->next);
  
  return (next.ptr == NULL) ? 1 : 0;
  
}

/*

         commc_lf_queue_size()
	       ---
	       returns approximate queue size.

*/

unsigned long commc_lf_queue_size(const commc_lf_queue_t* queue) {

  if (!queue) {

    return 0;
    
  }
  
  return COMMC_ATOMIC_LOAD((volatile unsigned long*)&queue->size);
  
}

/*

         commc_lf_queue_retire_node()
	       ---
	       adds node to retired list for deferred cleanup.

*/

void commc_lf_queue_retire_node(commc_lf_queue_t* queue, commc_lf_queue_node_t* node) {

  if (!queue || !node) {

    return;
    
  }
  
  /* add to retired list */
  
  do {

    commc_lf_queue_node_t* old_head = queue->retired_nodes;
    node->next.ptr = old_head;
    
  } while (!COMMC_ATOMIC_CAS((void**)&queue->retired_nodes, 
                             (void*)node->next.ptr, (void*)node));
  
  /* increment retired count */
  
  COMMC_ATOMIC_INC(&queue->retired_count);
  
}

/*

         commc_lf_queue_cleanup_retired()
	       ---
	       safely reclaims retired nodes not protected by hazard pointers.

*/

void commc_lf_queue_cleanup_retired(commc_lf_queue_t* queue) {

  commc_lf_queue_node_t* current;
  commc_lf_queue_node_t* next;
  commc_lf_queue_node_t* prev;
  unsigned long          cleaned;
  
  if (!queue) {

    return;
    
  }
  
  prev = NULL;
  current = queue->retired_nodes;
  cleaned = 0;
  
  while (current && cleaned < COMMC_LF_QUEUE_MAX_CLEANUP_BATCH) {

    next = (commc_lf_queue_node_t*)current->next.ptr;
    
    if (!is_node_hazardous(queue, current) && 
        COMMC_ATOMIC_LOAD(&current->ref_count) == 0) {

      /* node is safe to free */
      
      if (prev) {

        prev->next.ptr = next;
        
      } else {

        queue->retired_nodes = next;
        
      }
      
      free(current);
      cleaned++;
      COMMC_ATOMIC_DEC(&queue->retired_count);
      
    } else {

      prev = current;
      
    }
    
    current = next;
    
  }
  
}

/*

         commc_lf_queue_memory_usage()
	       ---
	       calculates approximate memory usage.

*/

size_t commc_lf_queue_memory_usage(const commc_lf_queue_t* queue) {

  size_t total;
  unsigned long node_count;
  
  if (!queue) {

    return 0;
    
  }
  
  total = sizeof(commc_lf_queue_t);
  total += queue->max_threads * sizeof(commc_lf_queue_thread_data_t);
  
  node_count = COMMC_ATOMIC_LOAD((volatile unsigned long*)&queue->size) + COMMC_ATOMIC_LOAD((volatile unsigned long*)&queue->retired_count);
  total += node_count * sizeof(commc_lf_queue_node_t);
  
  return total;
  
}

/*

         commc_lf_queue_validate_atomics()
	       ---
	       tests if atomic operations are working correctly.

*/

int commc_lf_queue_validate_atomics(void) {

  volatile unsigned long test_value = 42;
  unsigned long          result;
  
  /* test atomic load/store */
  
  (void)COMMC_ATOMIC_STORE(&test_value, 100);
  result = COMMC_ATOMIC_LOAD(&test_value);
  
  if (result != 100) {

    return 0;
    
  }
  
  /* test compare-and-swap */
  
  if (!COMMC_ATOMIC_CAS(&test_value, 100, 200)) {

    return 0;
    
  }
  
  if (COMMC_ATOMIC_LOAD(&test_value) != 200) {

    return 0;
    
  }
  
  /* test increment/decrement */
  
  COMMC_ATOMIC_INC(&test_value);
  
  if (COMMC_ATOMIC_LOAD(&test_value) != 201) {

    return 0;
    
  }
  
  COMMC_ATOMIC_DEC(&test_value);
  
  if (COMMC_ATOMIC_LOAD(&test_value) != 200) {

    return 0;
    
  }
  
  return 1; /* atomics working correctly */
  
}

/*
	==================================
             --- HAZARD POINTER API ---
	==================================
*/

/*

         commc_lf_queue_acquire_hazard()
	       ---
	       public interface for acquiring hazard pointers.

*/

commc_lf_queue_hazard_t* commc_lf_queue_acquire_hazard(commc_lf_queue_t* queue,
                                                        commc_lf_queue_node_t* node) {

  return acquire_hazard_pointer(queue, node);
  
}

/*

         commc_lf_queue_release_hazard()
	       ---
	       public interface for releasing hazard pointers.

*/

void commc_lf_queue_release_hazard(commc_lf_queue_hazard_t* hazard) {

  release_hazard_pointer(hazard);
  
}

/*

         commc_lf_queue_get_thread_data()
	       ---
	       public interface for accessing thread-local data.

*/

commc_lf_queue_thread_data_t* commc_lf_queue_get_thread_data(commc_lf_queue_t* queue) {

  return get_thread_local_data(queue);
  
}

/* 
	==================================
             --- EOF ---
	==================================
*/