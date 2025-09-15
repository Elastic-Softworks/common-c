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

typedef struct commc_memory_block_t {

  void*                           address;   /* pointer to block */
  struct commc_memory_block_t*    next;      /* next free block */

} commc_memory_block_t;

typedef struct commc_memory_pool_t {

  commc_memory_block_t*           free_blocks;    /* freelist head */
  size_t                          block_size;     /* size of each block */
  size_t                          total_size;     /* total buffer size */
  unsigned char*                  buffer;         /* large allocation */

} commc_memory_pool_t;

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

  commc_memory_pool_t* pool;

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

  /* init freelist */

  unsigned char* current = pool->buffer;

  for  (size_t i = 0; i < block_count; i++) {

    commc_memory_block_t* node;
    node = (commc_memory_block_t*) malloc(sizeof(commc_memory_block_t));

    if  (!node) {

      /* cleanup partial init */

      free(pool->buffer);
      free(pool);
      return NULL;

    }

    node->address = current;
    node->next    = pool->free_blocks;
    pool->free_blocks = node;
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

  if  (!pool->free_blocks) {

    return NULL;

  }

  commc_memory_block_t* block = pool->free_blocks;
  pool->free_blocks = block->next;

  return block->address;

}

/*

         commc_memory_pool_free()
	   	   ---
	   	   pushes the block back onto the freelist.
	   	   note: does not check if block belongs to pool.

*/

void commc_memory_pool_free(commc_memory_pool_t* pool, void* block) {

  commc_memory_block_t* node;
  node = (commc_memory_block_t*) malloc(sizeof(commc_memory_block_t));

  if  (node) {

    node->address = block;
    node->next    = pool->free_blocks;
    pool->free_blocks = node;

  }
  /* if alloc fails, silently leak or handle differently. */

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

  commc_memory_block_t* current = pool->free_blocks;

  while  (current) {

    commc_memory_block_t* next = current->next;
    free(current);
    current = next;

  }

  free(pool->buffer);
  free(pool);

}

/*
	==================================
             --- EOF ---
	==================================
*/
