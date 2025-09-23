/*
   ===================================
   C O M M O N - C
   LOCK-FREE QUEUE API HEADER
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- LOCK-FREE QUEUE HEADER ---

    thread-safe, wait-free FIFO queue implementation based on the 
    Michael & Scott algorithm. provides high-performance concurrent
    access without locks, using atomic operations and memory ordering
    to ensure correctness.
    
    key features:
    - non-blocking enqueue/dequeue operations
    - ABA problem prevention through tagged pointers
    - memory-safe reclamation using hazard pointers
    - linearizable operations with progress guarantees
    - scalable performance under contention
    
    this implementation is designed for C89 compatibility by providing
    platform-specific atomic operation abstractions.

*/

#ifndef COMMC_LF_QUEUE_H
#define COMMC_LF_QUEUE_H

/*
	==================================
             --- INCLUDES ---
	==================================
*/

#include "commc/error.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
	==================================
             --- PLATFORM ATOMIC ABSTRACTION ---
	==================================
*/

/*

    atomic operation macros for C89 compatibility.
    these provide a uniform interface across different
    compilers and platforms for atomic pointer operations.

*/

/* detect compiler and platform for atomic operations */

#if defined(__GNUC__) && ((__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1))

  /* GCC built-in atomics (GCC 4.1+) */
  
  #define COMMC_ATOMIC_LOAD(ptr)           __sync_add_and_fetch((ptr), 0)
  #define COMMC_ATOMIC_STORE(ptr, val)     __sync_lock_test_and_set((ptr), (val))
  #define COMMC_ATOMIC_CAS(ptr, old, new)  __sync_bool_compare_and_swap((ptr), (old), (new))
  #define COMMC_ATOMIC_INC(ptr)            __sync_add_and_fetch((ptr), 1)
  #define COMMC_ATOMIC_DEC(ptr)            __sync_sub_and_fetch((ptr), 1)
  #define COMMC_MEMORY_BARRIER()           __sync_synchronize()
  
  #define COMMC_HAS_ATOMICS 1
  
#elif defined(_MSC_VER) && (_MSC_VER >= 1300)

  /* Microsoft Visual C++ intrinsics */
  
  #include <intrin.h>
  
  #define COMMC_ATOMIC_LOAD(ptr)           (*(volatile long*)(ptr))
  #define COMMC_ATOMIC_STORE(ptr, val)     _InterlockedExchange((volatile long*)(ptr), (long)(val))
  #define COMMC_ATOMIC_CAS(ptr, old, new)  (_InterlockedCompareExchange((volatile long*)(ptr), (long)(new), (long)(old)) == (long)(old))
  #define COMMC_ATOMIC_INC(ptr)            _InterlockedIncrement((volatile long*)(ptr))
  #define COMMC_ATOMIC_DEC(ptr)            _InterlockedDecrement((volatile long*)(ptr))
  #define COMMC_MEMORY_BARRIER()           _ReadWriteBarrier()
  
  #define COMMC_HAS_ATOMICS 1
  
#else

  /* fallback: use volatile and hope for the best */
  /* note: this is not truly atomic and should not be used in production */
  
  #define COMMC_ATOMIC_LOAD(ptr)           (*(volatile void**)(ptr))
  #define COMMC_ATOMIC_STORE(ptr, val)     (*(volatile void**)(ptr) = (val))
  #define COMMC_ATOMIC_CAS(ptr, old, new) ((*(volatile void**)(ptr) == (old)) ? (*(volatile void**)(ptr) = (new), 1) : 0)
  #define COMMC_ATOMIC_INC(ptr)            (++(*(volatile long*)(ptr)))
  #define COMMC_ATOMIC_DEC(ptr)            (--(*(volatile long*)(ptr)))
  #define COMMC_MEMORY_BARRIER()           /* no-op */
  
  #define COMMC_HAS_ATOMICS 0
  
  #ifdef __STDC_VERSION__
    #if __STDC_VERSION__ >= 201112L
      #warning "Lock-free queue fallback: using volatile operations (not truly atomic)"
    #endif
  #endif
  
#endif

/*
	==================================
             --- DATA STRUCTURES ---
	==================================
*/

/*

    tagged pointer structure to prevent ABA problems.
    combines a pointer with a generation counter that
    is incremented on each modification.

*/

typedef struct {

  void*         ptr;      /* actual pointer value */
  unsigned long tag;      /* generation/version counter */
  
} commc_lf_queue_tagged_ptr_t;

/*

    lock-free queue node structure.
    each node contains data and an atomic next pointer
    with ABA protection through tagging.

*/

typedef struct commc_lf_queue_node {

  void*                           data;           /* stored element data */
  commc_lf_queue_tagged_ptr_t     next;           /* atomic next pointer with tag */
  volatile unsigned long          ref_count;      /* reference count for safe reclamation */
  
} commc_lf_queue_node_t;

/*

    hazard pointer structure for memory-safe reclamation.
    allows threads to announce which nodes they are accessing
    to prevent premature deallocation by other threads.

*/

typedef struct commc_lf_queue_hazard {

  volatile commc_lf_queue_node_t*  node;          /* hazard protected node */
  volatile unsigned long           active;        /* hazard is currently active */
  
} commc_lf_queue_hazard_t;

/*

    thread-local storage for hazard pointers.
    each thread gets a small set of hazard pointers
    for protecting nodes during operations.

*/

#define COMMC_LF_QUEUE_HAZARDS_PER_THREAD  4

typedef struct {

  commc_lf_queue_hazard_t  hazards[COMMC_LF_QUEUE_HAZARDS_PER_THREAD];
  unsigned long            thread_id;             /* unique thread identifier */
  
} commc_lf_queue_thread_data_t;

/*

    main lock-free queue structure.
    uses Michael & Scott algorithm with atomic head/tail pointers
    and hazard pointer-based memory management.

*/

typedef struct {

  commc_lf_queue_tagged_ptr_t      head;          /* atomic head pointer with tag */
  commc_lf_queue_tagged_ptr_t      tail;          /* atomic tail pointer with tag */
  
  /* memory management */
  
  volatile unsigned long           size;          /* approximate queue size */
  volatile unsigned long           next_thread_id;/* thread ID counter */
  
  /* hazard pointer management */
  
  commc_lf_queue_thread_data_t*    thread_data;   /* per-thread hazard storage */
  volatile unsigned long           max_threads;   /* maximum concurrent threads */
  
  /* deferred cleanup */
  
  commc_lf_queue_node_t*           retired_nodes; /* nodes pending cleanup */
  volatile unsigned long           retired_count; /* number of retired nodes */
  
} commc_lf_queue_t;

/*
	==================================
             --- CORE API ---
	==================================
*/

/*

         commc_lf_queue_create()
	       ---
	       creates a new lock-free queue with specified maximum thread count.
	       
	       the queue uses hazard pointers for memory-safe reclamation,
	       so the maximum number of threads that will access the queue
	       concurrently must be specified at creation time.

*/

commc_lf_queue_t* commc_lf_queue_create(unsigned long max_threads);

/*

         commc_lf_queue_destroy()
	       ---
	       destroys the lock-free queue and cleans up all resources.
	       
	       this function is NOT thread-safe and should only be called
	       when no other threads are accessing the queue.

*/

void commc_lf_queue_destroy(commc_lf_queue_t* queue);

/*

         commc_lf_queue_enqueue()
	       ---
	       adds an element to the tail of the queue atomically.
	       
	       this operation is wait-free and will never block,
	       though it may retry internally due to contention.
	       returns COMMC_SUCCESS on successful insertion.

*/

commc_error_t commc_lf_queue_enqueue(commc_lf_queue_t* queue, void* data);

/*

         commc_lf_queue_dequeue()
	       ---
	       removes and returns the element from the head of the queue atomically.
	       
	       this operation is wait-free and will never block.
	       returns COMMC_SUCCESS if an element was dequeued,
	       COMMC_FAILURE if the queue is empty.

*/

commc_error_t commc_lf_queue_dequeue(commc_lf_queue_t* queue, void** data);

/*

         commc_lf_queue_is_empty()
	       ---
	       checks if the queue appears to be empty at this moment.
	       
	       due to concurrent modifications, this is only a snapshot
	       and may not reflect the actual state by the time the
	       function returns.

*/

int commc_lf_queue_is_empty(const commc_lf_queue_t* queue);

/*

         commc_lf_queue_size()
	       ---
	       returns the approximate current size of the queue.
	       
	       this is an estimate and may not be precise due to
	       concurrent modifications by other threads.

*/

unsigned long commc_lf_queue_size(const commc_lf_queue_t* queue);

/*
	==================================
             --- HAZARD POINTER API ---
	==================================
*/

/*

         commc_lf_queue_acquire_hazard()
	       ---
	       acquires a hazard pointer to protect the given node from
	       being freed by other threads during critical operations.
	       
	       returns a hazard pointer handle that must be released
	       with commc_lf_queue_release_hazard().

*/

commc_lf_queue_hazard_t* commc_lf_queue_acquire_hazard(commc_lf_queue_t* queue, 
                                                        commc_lf_queue_node_t* node);

/*

         commc_lf_queue_release_hazard()
	       ---
	       releases a previously acquired hazard pointer,
	       allowing the protected node to be reclaimed if
	       no other threads are referencing it.

*/

void commc_lf_queue_release_hazard(commc_lf_queue_hazard_t* hazard);

/*

         commc_lf_queue_retire_node()
	       ---
	       marks a node for deferred cleanup. the node will be
	       safely freed once no hazard pointers reference it.
	       
	       this function is used internally during dequeue operations
	       to safely reclaim memory in the presence of concurrent access.

*/

void commc_lf_queue_retire_node(commc_lf_queue_t* queue, commc_lf_queue_node_t* node);

/*

         commc_lf_queue_cleanup_retired()
	       ---
	       scans retired nodes and safely frees those that are
	       no longer referenced by any hazard pointers.
	       
	       this is called periodically during queue operations
	       to prevent unbounded memory usage.

*/

void commc_lf_queue_cleanup_retired(commc_lf_queue_t* queue);

/*
	==================================
             --- UTILITY API ---
	==================================
*/

/*

         commc_lf_queue_get_thread_data()
	       ---
	       returns the thread-local data structure for the calling thread,
	       creating it if necessary. this manages hazard pointer allocation
	       on a per-thread basis.

*/

commc_lf_queue_thread_data_t* commc_lf_queue_get_thread_data(commc_lf_queue_t* queue);

/*

         commc_lf_queue_memory_usage()
	       ---
	       returns the approximate memory usage of the queue structure,
	       including nodes, hazard pointer data, and administrative overhead.
	       
	       this is useful for monitoring and debugging purposes.

*/

size_t commc_lf_queue_memory_usage(const commc_lf_queue_t* queue);

/*

         commc_lf_queue_validate_atomics()
	       ---
	       performs runtime validation that atomic operations are
	       working correctly on the current platform.
	       
	       returns 1 if atomics are supported and working,
	       0 if falling back to non-atomic operations.

*/

int commc_lf_queue_validate_atomics(void);

/*
	==================================
             --- TAGGED POINTER OPERATIONS ---
	==================================
*/

/*

         commc_lf_queue_tagged_ptr_create()
	       ---
	       creates a new tagged pointer with the given pointer value
	       and initial tag of 0.

*/

commc_lf_queue_tagged_ptr_t commc_lf_queue_tagged_ptr_create(void* ptr);

/*

         commc_lf_queue_tagged_ptr_advance()
	       ---
	       creates a new tagged pointer with the same pointer value
	       but with an incremented tag to prevent ABA problems.

*/

commc_lf_queue_tagged_ptr_t commc_lf_queue_tagged_ptr_advance(commc_lf_queue_tagged_ptr_t tagged_ptr);

/*

         commc_lf_queue_tagged_ptr_cas()
	       ---
	       performs an atomic compare-and-swap operation on a tagged pointer.
	       returns 1 if the swap was successful, 0 otherwise.
	       
	       this is the fundamental operation underlying all queue modifications.

*/

int commc_lf_queue_tagged_ptr_cas(volatile commc_lf_queue_tagged_ptr_t* target,
                                  commc_lf_queue_tagged_ptr_t expected,
                                  commc_lf_queue_tagged_ptr_t new_value);

/*

         commc_lf_queue_tagged_ptr_load()
	       ---
	       atomically loads a tagged pointer value with proper memory ordering.

*/

commc_lf_queue_tagged_ptr_t commc_lf_queue_tagged_ptr_load(volatile commc_lf_queue_tagged_ptr_t* target);

#ifdef __cplusplus
}
#endif

#endif /* COMMC_LF_QUEUE_H */

/* 
	==================================
             --- EOF ---
	==================================
*/