/*
   ===================================
   C O M M O N - C
   MEMORY IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- MEMORY MODULE ---

    implementation of memory pool functions.
    see include/commc/memory.h for prototypes.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#include "commc/memory.h"
#include <stdlib.h>
#include <string.h>

/*
	==================================
             --- STRUCTS ---
	==================================
*/

/* internal pool structure. */

/* internal definition of memory pool structure */

struct commc_memory_pool_t {

  void**                          free_blocks;    /* intrusive freelist head */
  size_t                          block_size;     /* size of each block */
  size_t                          total_size;     /* total buffer size */
  unsigned char*                  buffer;         /* large allocation */

};

/*
	==================================
             --- FUNCS ---
	==================================
*/

/*

         commc_memory_pool_create()
	   	   ---
	   	   allocates a large buffer and sets up the freelist
	   	   with all blocks initially free.

*/

commc_memory_pool_t* commc_memory_pool_create(size_t block_size, size_t block_count) {

  commc_memory_pool_t*  pool;
  unsigned char*        current;  /* C89 compliance: declare all variables at top */
  size_t                i;

  /* ensure block size is at least sizeof(void*) for intrusive freelist */

  if  (block_size < sizeof(void*)) {

    block_size = sizeof(void*);

  }

  pool = (commc_memory_pool_t*) malloc(sizeof(commc_memory_pool_t));

  if  (!pool) {

    return NULL;

  }

  pool->total_size =  block_size * block_count;
  pool->block_size =  block_size;
  pool->buffer     =  (unsigned char*) malloc(pool->total_size);

  if  (!pool->buffer) {

    free(pool);
    return NULL;

  }

  pool->free_blocks = NULL;

  /* init intrusive freelist - each block points to next */

  current = pool->buffer;

  for  (i = 0; i < block_count; i++) {

    *((void**)current) = pool->free_blocks;  /* store next pointer in block */
    pool->free_blocks = (void**)current;     /* update freelist head */
    current += block_size;

  }

  return pool;

}

/*

         commc_memory_pool_alloc()
	   	   ---
	   	   pops the top free block from the list.

*/

void* commc_memory_pool_alloc(commc_memory_pool_t* pool) {

  void** block; /* C89 compliance: declare variables at top */

  if  (!pool) {

    return NULL;

  }

  if  (!pool->free_blocks) {

    return NULL;

  }

  block = pool->free_blocks;
  pool->free_blocks = (void**)*pool->free_blocks;  /* update freelist to next block */

  return (void*)block;

}

/*

         commc_memory_pool_free()
	   	   ---
	   	   pushes the block back onto the freelist.
	   	   note: does not check if block belongs to pool.

*/

void commc_memory_pool_free(commc_memory_pool_t* pool, void* block) {

  if  (!pool || !block) {

    return; /* handle null parameters gracefully */

  }

  /* add block back to intrusive freelist */

  *((void**)block) = pool->free_blocks;  /* store current head in block */
  pool->free_blocks = (void**)block;     /* block becomes new head */

}

/*

         commc_memory_pool_destroy()
	   	   ---
	   	   frees all memory associated with the pool.

*/

void commc_memory_pool_destroy(commc_memory_pool_t* pool) {

  if  (!pool) {

    return;

  }

  /* no need to free individual freelist nodes - they're intrusive */

  free(pool->buffer);
  free(pool);

}

/*
	==================================
             --- EOF ---
	==================================
*/
