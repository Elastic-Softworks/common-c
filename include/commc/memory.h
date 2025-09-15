/*
   ===================================
   C O M M O N - C
   MEMORY MANAGEMENT MODULE
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- MEMORY MODULE ---

    this module provides advanced memory management utilities
    beyond the standard malloc/free. it includes memory pools
    for fixed-size allocations and arena allocators for scratch
    space in game loops or temporary data.

    useful for games where fragmentation is an issue.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#ifndef COMMC_MEMORY_H
#define COMMC_MEMORY_H

#include <stddef.h>

/*
	==================================
             --- DEFINES ---
	==================================
*/

/* opaque type for memory pool. */

typedef struct commc_memory_pool_t commc_memory_pool_t; 

/*
	==================================
             --- FUNCTIONS ---
	==================================
*/

/*

         commc_memory_pool_create()
     	   ---
	   	   create a memory pool for fixed-size allocations.
	   	   allocates a large block and manages sub-allocations
	   	   from it to reduce fragmentation and overhead.

*/

commc_memory_pool_t* commc_memory_pool_create(size_t block_size, size_t block_count);

/*

         commc_memory_pool_alloc()
      	 ---
	   	   allocate from the pool. returns NULL if out of space.

*/

void* commc_memory_pool_alloc(commc_memory_pool_t* pool);

/*

         commc_memory_pool_free()
      	 ---
	   	   free a block back to the pool (marks as available).

*/

void commc_memory_pool_free(commc_memory_pool_t* pool, void* block);

/*

         commc_memory_pool_destroy()
      	 ---
	   	   destroy the pool and free all underlying memory.

*/

void commc_memory_pool_destroy(commc_memory_pool_t* pool);

#endif /* COMMC_MEMORY_H */

/*
	==================================
             --- EOF ---
	==================================
*/
