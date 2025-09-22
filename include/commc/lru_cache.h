/* 	
   ===================================
   C O M M C / L R U _ C A C H E . H
   LEAST RECENTLY USED CACHE API
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

	                --- LRU CACHE API ---

	    lru (least recently used) caches provide O(1) access and eviction
	    for fixed-capacity key-value storage. this implementation combines
	    a hash table for fast lookups with a doubly linked list for
	    efficient access order tracking and eviction.
	    
	    when capacity is reached, the least recently accessed item is
	    automatically evicted to make room for new entries.

*/

#ifndef COMMC_LRU_CACHE_H
#define COMMC_LRU_CACHE_H

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
             --- TYPES ---
	==================================
*/

/*

         commc_lru_cache_node_t
	       ---
	       internal node structure containing key-value pair and
	       doubly linked list pointers for access order tracking.

*/

typedef struct commc_lru_cache_node {

  void*                          key;        /* stored key */
  void*                          value;      /* stored value */
  size_t                         key_size;   /* key size in bytes */
  size_t                         value_size; /* value size in bytes */
  struct commc_lru_cache_node*   prev;       /* previous in access order */
  struct commc_lru_cache_node*   next;       /* next in access order */
  
} commc_lru_cache_node_t;

/*

         commc_lru_cache_t
	       ---
	       represents an lru cache with hash table for lookups and
	       doubly linked list for access order management.

*/

typedef struct {

  commc_lru_cache_node_t**  hash_table;       /* hash table for O(1) lookup */
  commc_lru_cache_node_t*   head;             /* most recently accessed */  
  commc_lru_cache_node_t*   tail;             /* least recently accessed */
  size_t                    capacity;         /* maximum number of entries */
  size_t                    size;             /* current number of entries */
  size_t                    hash_table_size;  /* size of hash table array */
  
} commc_lru_cache_t;

/*

         commc_lru_cache_hash_function_t
	       ---
	       function pointer type for custom hash functions.

*/

typedef size_t (*commc_lru_cache_hash_function_t)(const void* key, size_t key_size);

/*

         commc_lru_cache_key_compare_t
	       ---
	       function pointer type for custom key comparison functions.
	       should return 0 for equal keys, non-zero for different keys.

*/

typedef int (*commc_lru_cache_key_compare_t)(const void* key1, size_t key1_size,
                                             const void* key2, size_t key2_size);

/*

         commc_lru_cache_eviction_callback_t
	       ---
	       function pointer type for eviction notifications.
	       called when an item is evicted from the cache.

*/

typedef void (*commc_lru_cache_eviction_callback_t)(const void* key, size_t key_size,
                                                     const void* value, size_t value_size,
                                                     void* user_data);

/* 
	==================================
             --- CONSTANTS ---
	==================================
*/

/*

         COMMC_LRU_CACHE_DEFAULT_CAPACITY
	       ---
	       default cache capacity if none specified.

*/

#define COMMC_LRU_CACHE_DEFAULT_CAPACITY 100

/*

         COMMC_LRU_CACHE_DEFAULT_HASH_SIZE
	       ---
	       default hash table size for good distribution.

*/

#define COMMC_LRU_CACHE_DEFAULT_HASH_SIZE 127

/*

         COMMC_LRU_CACHE_MIN_CAPACITY
	       ---
	       minimum allowable cache capacity.

*/

#define COMMC_LRU_CACHE_MIN_CAPACITY 1

/* 
	==================================
             --- CORE API ---
	==================================
*/

/*

         commc_lru_cache_create()
	       ---
	       creates a new lru cache with specified capacity.
	       uses default hash function and key comparison.
	       
	       parameters:
	       - capacity: maximum number of key-value pairs to store
	       
	       returns:
	       - pointer to new cache, or NULL on error

*/

commc_lru_cache_t* commc_lru_cache_create(size_t capacity);

/*

         commc_lru_cache_create_with_hash_size()
	       ---
	       creates lru cache with custom hash table size for performance tuning.
	       
	       parameters:
	       - capacity: maximum number of key-value pairs
	       - hash_table_size: size of internal hash table
	       
	       returns:
	       - pointer to new cache, or NULL on error

*/

commc_lru_cache_t* commc_lru_cache_create_with_hash_size(size_t capacity,
                                                          size_t hash_table_size);

/*

         commc_lru_cache_destroy()
	       ---
	       destroys lru cache and frees all associated memory.

*/

void commc_lru_cache_destroy(commc_lru_cache_t* cache);

/*

         commc_lru_cache_put()
	       ---
	       inserts or updates a key-value pair in the cache.
	       if key exists, updates value and moves to front.
	       if cache is full, evicts least recently used item.
	       
	       parameters:
	       - cache: lru cache to modify
	       - key: pointer to key data
	       - key_size: size of key data in bytes
	       - value: pointer to value data  
	       - value_size: size of value data in bytes
	       
	       returns:
	       - COMMC_SUCCESS on successful insertion
	       - COMMC_ARGUMENT_ERROR for invalid parameters
	       - COMMC_MEMORY_ERROR if memory allocation fails

*/

commc_error_t commc_lru_cache_put(commc_lru_cache_t* cache,
                                   const void* key, size_t key_size,
                                   const void* value, size_t value_size);

/*

         commc_lru_cache_get()
	       ---
	       retrieves a value by key and moves it to front (most recently used).
	       
	       parameters:
	       - cache: lru cache to query
	       - key: pointer to key data to search for
	       - key_size: size of key data in bytes
	       - value: pointer to store retrieved value (output)
	       - value_size: pointer to store value size (output)
	       
	       returns:
	       - COMMC_SUCCESS if key found
	       - COMMC_KEY_NOT_FOUND if key not in cache
	       - COMMC_ARGUMENT_ERROR for invalid parameters

*/

commc_error_t commc_lru_cache_get(commc_lru_cache_t* cache,
                                   const void* key, size_t key_size,
                                   void** value, size_t* value_size);

/*

         commc_lru_cache_peek()
	       ---
	       retrieves a value by key without updating access order.
	       useful for checking cache contents without affecting eviction.

*/

commc_error_t commc_lru_cache_peek(commc_lru_cache_t* cache,
                                    const void* key, size_t key_size,
                                    void** value, size_t* value_size);

/*

         commc_lru_cache_remove()
	       ---
	       removes a key-value pair from the cache.
	       
	       parameters:
	       - cache: lru cache to modify
	       - key: pointer to key data to remove
	       - key_size: size of key data in bytes
	       
	       returns:
	       - COMMC_SUCCESS if key found and removed
	       - COMMC_KEY_NOT_FOUND if key not in cache
	       - COMMC_ARGUMENT_ERROR for invalid parameters

*/

commc_error_t commc_lru_cache_remove(commc_lru_cache_t* cache,
                                      const void* key, size_t key_size);

/* 
	==================================
             --- INFO API ---
	==================================
*/

/*

         commc_lru_cache_size()
	       ---
	       returns the current number of items in the cache.

*/

size_t commc_lru_cache_size(commc_lru_cache_t* cache);

/*

         commc_lru_cache_capacity()
	       ---
	       returns the maximum capacity of the cache.

*/

size_t commc_lru_cache_capacity(commc_lru_cache_t* cache);

/*

         commc_lru_cache_is_empty()
	       ---
	       returns 1 if cache is empty, 0 otherwise.

*/

int commc_lru_cache_is_empty(commc_lru_cache_t* cache);

/*

         commc_lru_cache_is_full()
	       ---
	       returns 1 if cache is at capacity, 0 otherwise.

*/

int commc_lru_cache_is_full(commc_lru_cache_t* cache);

/*

         commc_lru_cache_contains()
	       ---
	       returns 1 if key exists in cache, 0 otherwise.
	       does not affect access order.

*/

int commc_lru_cache_contains(commc_lru_cache_t* cache,
                             const void* key, size_t key_size);

/*

         commc_lru_cache_hit_rate()
	       ---
	       calculates cache hit rate as percentage.
	       requires tracking hits/misses during cache lifetime.

*/

double commc_lru_cache_hit_rate(commc_lru_cache_t* cache);

/*

         commc_lru_cache_memory_usage()
	       ---
	       estimates total memory usage of the cache in bytes.

*/

size_t commc_lru_cache_memory_usage(commc_lru_cache_t* cache);

/* 
	==================================
             --- UTILITY API ---
	==================================
*/

/*

         commc_lru_cache_clear()
	       ---
	       removes all items from the cache without changing capacity.

*/

void commc_lru_cache_clear(commc_lru_cache_t* cache);

/*

         commc_lru_cache_set_eviction_callback()
	       ---
	       sets callback function to be called when items are evicted.
	       useful for cleanup or logging of evicted items.

*/

void commc_lru_cache_set_eviction_callback(commc_lru_cache_t* cache,
                                            commc_lru_cache_eviction_callback_t callback,
                                            void* user_data);

/*

         commc_lru_cache_get_lru_key()
	       ---
	       returns the least recently used key without removing it.
	       useful for external eviction policies or monitoring.

*/

commc_error_t commc_lru_cache_get_lru_key(commc_lru_cache_t* cache,
                                           void** key, size_t* key_size);

/*

         commc_lru_cache_get_mru_key()
	       ---
	       returns the most recently used key.

*/

commc_error_t commc_lru_cache_get_mru_key(commc_lru_cache_t* cache,
                                           void** key, size_t* key_size);

/* 
	==================================
             --- ITERATION API ---
	==================================
*/

/*

         commc_lru_cache_iterator_t
	       ---
	       iterator for traversing cache contents in access order.

*/

typedef struct {

  commc_lru_cache_node_t* current;    /* current node */
  commc_lru_cache_t*      cache;      /* cache being iterated */
  
} commc_lru_cache_iterator_t;

/*

         commc_lru_cache_iterator_begin()
	       ---
	       creates iterator starting from most recently used item.

*/

commc_lru_cache_iterator_t commc_lru_cache_iterator_begin(commc_lru_cache_t* cache);

/*

         commc_lru_cache_iterator_rbegin()
	       ---
	       creates iterator starting from least recently used item.

*/

commc_lru_cache_iterator_t commc_lru_cache_iterator_rbegin(commc_lru_cache_t* cache);

/*

         commc_lru_cache_iterator_next()
	       ---
	       advances iterator to next item in access order.

*/

commc_error_t commc_lru_cache_iterator_next(commc_lru_cache_iterator_t* iterator);

/*

         commc_lru_cache_iterator_data()
	       ---
	       retrieves key-value pair from current iterator position.

*/

commc_error_t commc_lru_cache_iterator_data(commc_lru_cache_iterator_t* iterator,
                                             void** key, size_t* key_size,
                                             void** value, size_t* value_size);

/*

         commc_lru_cache_iterator_has_next()
	       ---
	       returns 1 if iterator has more items, 0 otherwise.

*/

int commc_lru_cache_iterator_has_next(commc_lru_cache_iterator_t* iterator);

/* 
	==================================
             --- HASH API ---
	==================================
*/

/*

         commc_lru_cache_hash_djb2()
	       ---
	       default djb2 hash function for general-purpose cache keys.

*/

size_t commc_lru_cache_hash_djb2(const void* key, size_t key_size);

/*

         commc_lru_cache_hash_fnv1a()
	       ---
	       fnv-1a hash function alternative for different distribution.

*/

size_t commc_lru_cache_hash_fnv1a(const void* key, size_t key_size);

/*

         commc_lru_cache_key_compare_memcmp()
	       ---
	       default key comparison using memcmp for arbitrary byte keys.

*/

int commc_lru_cache_key_compare_memcmp(const void* key1, size_t key1_size,
                                       const void* key2, size_t key2_size);

/*

         commc_lru_cache_key_compare_string()
	       ---
	       string key comparison for null-terminated string keys.

*/

int commc_lru_cache_key_compare_string(const void* key1, size_t key1_size,
                                       const void* key2, size_t key2_size);

#ifdef __cplusplus
}
#endif

#endif /* COMMC_LRU_CACHE_H */

/*
	==================================
             --- EOF ---
	==================================
*/