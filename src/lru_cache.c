/* 	
   ===================================
   C O M M C / L R U _ C A C H E . C
   LEAST RECENTLY USED CACHE IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

	                --- LRU CACHE IMPLEMENTATION ---

	    this implementation provides O(1) access and eviction for fixed-capacity
	    key-value storage by combining a hash table for fast lookups with a
	    doubly linked list for efficient access order tracking.
	    
	    the hash table enables constant-time key location while the doubly
	    linked list maintains access order with the most recently used items
	    at the head and least recently used at the tail.

*/

/* 
	==================================
             --- SETUP ---
	==================================
*/

#include "commc/lru_cache.h"  /* LRU CACHE API */
#include "commc/error.h"      /* ERROR HANDLING */
#include <stdlib.h>           /* STANDARD LIBRARY FUNCTIONS */
#include <string.h>           /* MEMORY OPERATIONS */

/* 
	==================================
             --- EXTENDED TYPES ---
	==================================
*/

/*

         commc_lru_cache_internal_t
	       ---
	       internal cache structure with additional tracking fields.

*/

typedef struct {

  commc_lru_cache_t                      base;               /* public interface */
  commc_lru_cache_eviction_callback_t    eviction_callback;  /* eviction notification */
  void*                                  callback_user_data; /* user data for callback */
  size_t                                 hits;               /* cache hits for statistics */
  size_t                                 misses;             /* cache misses for statistics */
  
} commc_lru_cache_internal_t;

/* 
	==================================
             --- HELPERS ---
	==================================
*/

/*

         create_node()
	       ---
	       creates a new cache node with key-value data.

*/

static commc_lru_cache_node_t* create_node(const void* key, size_t key_size,
                                            const void* value, size_t value_size) {

  commc_lru_cache_node_t* node;

  node = (commc_lru_cache_node_t*)malloc(sizeof(commc_lru_cache_node_t));
  
  if  (!node) {
    return NULL;
  }

  /* allocate and copy key */
  node->key = malloc(key_size);
  
  if  (!node->key) {
    free(node);
    return NULL;
  }

  memcpy(node->key, key, key_size);
  node->key_size = key_size;

  /* allocate and copy value */
  node->value = malloc(value_size);
  
  if  (!node->value) {
    free(node->key);
    free(node);
    return NULL;
  }

  memcpy(node->value, value, value_size);
  node->value_size = value_size;

  node->prev = NULL;
  node->next = NULL;

  return node;

}

/*

         destroy_node()
	       ---
	       destroys a cache node and frees all associated memory.

*/

static void destroy_node(commc_lru_cache_node_t* node) {

  if  (!node) {
    return;
  }

  if  (node->key) {
    free(node->key);
  }

  if  (node->value) {
    free(node->value);
  }

  free(node);

}

/*

         hash_key()
	       ---
	       calculates hash value for a key using djb2 algorithm.

*/

static size_t hash_key(const void* key, size_t key_size) {

  return commc_lru_cache_hash_djb2(key, key_size);

}

/*

         keys_equal()
	       ---
	       compares two keys for equality using memcmp.

*/

static int keys_equal(const void* key1, size_t key1_size,
                      const void* key2, size_t key2_size) {

  return commc_lru_cache_key_compare_memcmp(key1, key1_size, key2, key2_size) == 0;

}

/*

         find_node()
	       ---
	       locates a node in the hash table by key.

*/

static commc_lru_cache_node_t* find_node(commc_lru_cache_t* cache,
                                          const void* key, size_t key_size) {

  size_t                   hash_index;
  commc_lru_cache_node_t*  current;

  hash_index = hash_key(key, key_size) % cache->hash_table_size;
  current = cache->hash_table[hash_index];

  while  (current) {

    if  (keys_equal(current->key, current->key_size, key, key_size)) {
      return current;
    }

    current = current->next;

  }

  return NULL;

}

/*

         add_to_hash_table()
	       ---
	       adds a node to the hash table using chaining for collisions.

*/

static void add_to_hash_table(commc_lru_cache_t* cache, commc_lru_cache_node_t* node) {

  size_t hash_index;

  hash_index = hash_key(node->key, node->key_size) % cache->hash_table_size;

  /* insert at head of chain */
  node->next = cache->hash_table[hash_index];
  
  if  (cache->hash_table[hash_index]) {
    cache->hash_table[hash_index]->prev = node;
  }

  cache->hash_table[hash_index] = node;
  node->prev = NULL;

}

/*

         remove_from_hash_table()
	       ---
	       removes a node from the hash table.

*/

static void remove_from_hash_table(commc_lru_cache_t* cache, commc_lru_cache_node_t* node) {

  size_t hash_index;

  hash_index = hash_key(node->key, node->key_size) % cache->hash_table_size;

  if  (node->prev) {
    node->prev->next = node->next;
  } else {
    cache->hash_table[hash_index] = node->next;
  }

  if  (node->next) {
    node->next->prev = node->prev;
  }

}

/*

         move_to_head()
	       ---
	       moves a node to the head of the access order list (most recent).

*/

static void move_to_head(commc_lru_cache_t* cache, commc_lru_cache_node_t* node) {

  if  (cache->head == node) {
    return;  /* already at head */
  }

  /* remove from current position in access list */
  if  (node->prev) {
    node->prev->next = node->next;
  }

  if  (node->next) {
    node->next->prev = node->prev;
  } else {
    cache->tail = node->prev;  /* was tail */
  }

  /* add to head */
  node->prev = NULL;
  node->next = cache->head;

  if  (cache->head) {
    cache->head->prev = node;
  }

  cache->head = node;

  if  (!cache->tail) {
    cache->tail = node;  /* first node */
  }

}

/*

         add_to_head()
	       ---
	       adds a new node to the head of the access order list.

*/

static void add_to_head(commc_lru_cache_t* cache, commc_lru_cache_node_t* node) {

  node->prev = NULL;
  node->next = cache->head;

  if  (cache->head) {
    cache->head->prev = node;
  }

  cache->head = node;

  if  (!cache->tail) {
    cache->tail = node;  /* first node */
  }

}

/*

         remove_tail()
	       ---
	       removes and returns the tail node (least recently used).

*/

static commc_lru_cache_node_t* remove_tail(commc_lru_cache_t* cache) {

  commc_lru_cache_node_t* tail_node = cache->tail;

  if  (!tail_node) {
    return NULL;
  }

  if  (tail_node->prev) {
    tail_node->prev->next = NULL;
  } else {
    cache->head = NULL;  /* was only node */
  }

  cache->tail = tail_node->prev;

  tail_node->prev = NULL;
  tail_node->next = NULL;

  return tail_node;

}

/*

         evict_lru()
	       ---
	       evicts the least recently used item to make room for new entries.

*/

static void evict_lru(commc_lru_cache_internal_t* internal_cache) {

  commc_lru_cache_t*       cache = &internal_cache->base;
  commc_lru_cache_node_t*  lru_node;

  lru_node = remove_tail(cache);
  
  if  (!lru_node) {
    return;
  }

  remove_from_hash_table(cache, lru_node);

  /* call eviction callback if set */
  if  (internal_cache->eviction_callback) {
    internal_cache->eviction_callback(lru_node->key, lru_node->key_size,
                                      lru_node->value, lru_node->value_size,
                                      internal_cache->callback_user_data);
  }

  destroy_node(lru_node);
  cache->size--;

}

/*

         get_internal_cache()
	       ---
	       casts public cache pointer to internal structure.

*/

static commc_lru_cache_internal_t* get_internal_cache(commc_lru_cache_t* cache) {

  return (commc_lru_cache_internal_t*)cache;

}

/*
	==================================
             --- CORE API ---
	==================================
*/

/*

         commc_lru_cache_create()
	       ---
	       creates a new lru cache with default hash table size.

*/

commc_lru_cache_t* commc_lru_cache_create(size_t capacity) {

  return commc_lru_cache_create_with_hash_size(capacity, COMMC_LRU_CACHE_DEFAULT_HASH_SIZE);

}

/*

         commc_lru_cache_create_with_hash_size()
	       ---
	       creates lru cache with custom hash table size.

*/

commc_lru_cache_t* commc_lru_cache_create_with_hash_size(size_t capacity,
                                                          size_t hash_table_size) {

  commc_lru_cache_internal_t* internal_cache;
  commc_lru_cache_t*          cache;

  if  (capacity < COMMC_LRU_CACHE_MIN_CAPACITY || hash_table_size == 0) {
    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;
  }

  internal_cache = (commc_lru_cache_internal_t*)malloc(sizeof(commc_lru_cache_internal_t));
  
  if  (!internal_cache) {
    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;
  }

  cache = &internal_cache->base;

  cache->hash_table = (commc_lru_cache_node_t**)calloc(hash_table_size, sizeof(commc_lru_cache_node_t*));
  
  if  (!cache->hash_table) {
    free(internal_cache);
    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;
  }

  cache->head = NULL;
  cache->tail = NULL;
  cache->capacity = capacity;
  cache->size = 0;
  cache->hash_table_size = hash_table_size;

  internal_cache->eviction_callback = NULL;
  internal_cache->callback_user_data = NULL;
  internal_cache->hits = 0;
  internal_cache->misses = 0;

  return cache;

}

/*

         commc_lru_cache_destroy()
	       ---
	       destroys lru cache and frees all memory.

*/

void commc_lru_cache_destroy(commc_lru_cache_t* cache) {

  commc_lru_cache_internal_t* internal_cache;

  if  (!cache) {
    return;
  }

  internal_cache = get_internal_cache(cache);

  commc_lru_cache_clear(cache);

  if  (cache->hash_table) {
    free(cache->hash_table);
  }

  free(internal_cache);

}

/*

         commc_lru_cache_put()
	       ---
	       inserts or updates a key-value pair with automatic eviction.

*/

commc_error_t commc_lru_cache_put(commc_lru_cache_t* cache,
                                   const void* key, size_t key_size,
                                   const void* value, size_t value_size) {

  commc_lru_cache_internal_t* internal_cache;
  commc_lru_cache_node_t*     existing_node;
  commc_lru_cache_node_t*     new_node;

  if  (!cache || !key || key_size == 0 || !value) {
    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;
  }

  internal_cache = get_internal_cache(cache);
  existing_node = find_node(cache, key, key_size);

  if  (existing_node) {

    /* update existing entry */
    free(existing_node->value);
    existing_node->value = malloc(value_size);
    
    if  (!existing_node->value) {
      commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
      return COMMC_MEMORY_ERROR;
    }

    memcpy(existing_node->value, value, value_size);
    existing_node->value_size = value_size;

    move_to_head(cache, existing_node);
    return COMMC_SUCCESS;

  }

  /* create new entry */
  new_node = create_node(key, key_size, value, value_size);
  
  if  (!new_node) {
    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return COMMC_MEMORY_ERROR;
  }

  /* evict if necessary */
  if  (cache->size >= cache->capacity) {
    evict_lru(internal_cache);
  }

  add_to_hash_table(cache, new_node);
  add_to_head(cache, new_node);
  cache->size++;

  return COMMC_SUCCESS;

}

/*

         commc_lru_cache_get()
	       ---
	       retrieves a value by key and updates access order.

*/

commc_error_t commc_lru_cache_get(commc_lru_cache_t* cache,
                                   const void* key, size_t key_size,
                                   void** value, size_t* value_size) {

  commc_lru_cache_internal_t* internal_cache;
  commc_lru_cache_node_t*     node;

  if  (!cache || !key || key_size == 0 || !value || !value_size) {
    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;
  }

  internal_cache = get_internal_cache(cache);
  node = find_node(cache, key, key_size);

  if  (!node) {
    internal_cache->misses++;
    return COMMC_FAILURE;
  }

  internal_cache->hits++;
  move_to_head(cache, node);

  *value = node->value;
  *value_size = node->value_size;

  return COMMC_SUCCESS;

}

/*

         commc_lru_cache_peek()
	       ---
	       retrieves a value without updating access order.

*/

commc_error_t commc_lru_cache_peek(commc_lru_cache_t* cache,
                                    const void* key, size_t key_size,
                                    void** value, size_t* value_size) {

  commc_lru_cache_node_t* node;

  if  (!cache || !key || key_size == 0 || !value || !value_size) {
    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;
  }

  node = find_node(cache, key, key_size);

  if  (!node) {
    return COMMC_FAILURE;
  }

  *value = node->value;
  *value_size = node->value_size;

  return COMMC_SUCCESS;

}

/*

         commc_lru_cache_remove()
	       ---
	       removes a key-value pair from the cache.

*/

commc_error_t commc_lru_cache_remove(commc_lru_cache_t* cache,
                                      const void* key, size_t key_size) {

  commc_lru_cache_node_t* node;

  if  (!cache || !key || key_size == 0) {
    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;
  }

  node = find_node(cache, key, key_size);

  if  (!node) {
    return COMMC_FAILURE;
  }

  remove_from_hash_table(cache, node);

  /* remove from access order list */
  if  (node->prev) {
    node->prev->next = node->next;
  } else {
    cache->head = node->next;
  }

  if  (node->next) {
    node->next->prev = node->prev;
  } else {
    cache->tail = node->prev;
  }

  destroy_node(node);
  cache->size--;

  return COMMC_SUCCESS;

}

/*
	==================================
             --- INFO API ---
	==================================
*/

/*

         commc_lru_cache_size()
	       ---
	       returns current number of items in cache.

*/

size_t commc_lru_cache_size(commc_lru_cache_t* cache) {

  if  (!cache) {
    return 0;
  }

  return cache->size;

}

/*

         commc_lru_cache_capacity()
	       ---
	       returns maximum capacity of cache.

*/

size_t commc_lru_cache_capacity(commc_lru_cache_t* cache) {

  if  (!cache) {
    return 0;
  }

  return cache->capacity;

}

/*

         commc_lru_cache_is_empty()
	       ---
	       returns 1 if cache is empty.

*/

int commc_lru_cache_is_empty(commc_lru_cache_t* cache) {

  if  (!cache) {
    return 1;
  }

  return cache->size == 0;

}

/*

         commc_lru_cache_is_full()
	       ---
	       returns 1 if cache is at capacity.

*/

int commc_lru_cache_is_full(commc_lru_cache_t* cache) {

  if  (!cache) {
    return 0;
  }

  return cache->size >= cache->capacity;

}

/*

         commc_lru_cache_contains()
	       ---
	       returns 1 if key exists in cache.

*/

int commc_lru_cache_contains(commc_lru_cache_t* cache,
                             const void* key, size_t key_size) {

  if  (!cache || !key || key_size == 0) {
    return 0;
  }

  return find_node(cache, key, key_size) != NULL;

}

/*

         commc_lru_cache_hit_rate()
	       ---
	       calculates cache hit rate as percentage.

*/

double commc_lru_cache_hit_rate(commc_lru_cache_t* cache) {

  commc_lru_cache_internal_t* internal_cache;
  size_t                      total_accesses;

  if  (!cache) {
    return 0.0;
  }

  internal_cache = get_internal_cache(cache);
  total_accesses = internal_cache->hits + internal_cache->misses;

  if  (total_accesses == 0) {
    return 0.0;
  }

  return (double)internal_cache->hits / total_accesses * 100.0;

}

/*

         commc_lru_cache_memory_usage()
	       ---
	       estimates total memory usage in bytes.

*/

size_t commc_lru_cache_memory_usage(commc_lru_cache_t* cache) {

  commc_lru_cache_node_t* current;
  size_t                  total_bytes;
  size_t                  i;

  if  (!cache) {
    return 0;
  }

  total_bytes = sizeof(commc_lru_cache_internal_t);
  total_bytes += cache->hash_table_size * sizeof(commc_lru_cache_node_t*);

  /* estimate node memory usage */
  for  (i = 0; i < cache->hash_table_size; i++) {

    current = cache->hash_table[i];

    while  (current) {
      total_bytes += sizeof(commc_lru_cache_node_t);
      total_bytes += current->key_size;
      total_bytes += current->value_size;
      current = current->next;
    }

  }

  return total_bytes;

}

/*
	==================================
             --- UTILITY API ---
	==================================
*/

/*

         commc_lru_cache_clear()
	       ---
	       removes all items from cache.

*/

void commc_lru_cache_clear(commc_lru_cache_t* cache) {

  commc_lru_cache_node_t* current;
  commc_lru_cache_node_t* next;

  if  (!cache) {
    return;
  }

  current = cache->head;

  while  (current) {
    next = current->next;
    destroy_node(current);
    current = next;
  }

  memset(cache->hash_table, 0, cache->hash_table_size * sizeof(commc_lru_cache_node_t*));

  cache->head = NULL;
  cache->tail = NULL;
  cache->size = 0;

}

/*

         commc_lru_cache_set_eviction_callback()
	       ---
	       sets callback for eviction notifications.

*/

void commc_lru_cache_set_eviction_callback(commc_lru_cache_t* cache,
                                            commc_lru_cache_eviction_callback_t callback,
                                            void* user_data) {

  commc_lru_cache_internal_t* internal_cache;

  if  (!cache) {
    return;
  }

  internal_cache = get_internal_cache(cache);
  internal_cache->eviction_callback = callback;
  internal_cache->callback_user_data = user_data;

}

/*

         commc_lru_cache_get_lru_key()
	       ---
	       returns least recently used key.

*/

commc_error_t commc_lru_cache_get_lru_key(commc_lru_cache_t* cache,
                                           void** key, size_t* key_size) {

  if  (!cache || !key || !key_size) {
    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;
  }

  if  (!cache->tail) {
    return COMMC_FAILURE;
  }

  *key = cache->tail->key;
  *key_size = cache->tail->key_size;

  return COMMC_SUCCESS;

}

/*

         commc_lru_cache_get_mru_key()
	       ---
	       returns most recently used key.

*/

commc_error_t commc_lru_cache_get_mru_key(commc_lru_cache_t* cache,
                                           void** key, size_t* key_size) {

  if  (!cache || !key || !key_size) {
    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;
  }

  if  (!cache->head) {
    return COMMC_FAILURE;
  }

  *key = cache->head->key;
  *key_size = cache->head->key_size;

  return COMMC_SUCCESS;

}

/*
	==================================
             --- ITERATION API ---
	==================================
*/

/*

         commc_lru_cache_iterator_begin()
	       ---
	       creates iterator from most recently used item.

*/

commc_lru_cache_iterator_t commc_lru_cache_iterator_begin(commc_lru_cache_t* cache) {

  commc_lru_cache_iterator_t iterator;

  iterator.cache = cache;
  iterator.current = cache ? cache->head : NULL;

  return iterator;

}

/*

         commc_lru_cache_iterator_rbegin()
	       ---
	       creates iterator from least recently used item.

*/

commc_lru_cache_iterator_t commc_lru_cache_iterator_rbegin(commc_lru_cache_t* cache) {

  commc_lru_cache_iterator_t iterator;

  iterator.cache = cache;
  iterator.current = cache ? cache->tail : NULL;

  return iterator;

}

/*

         commc_lru_cache_iterator_next()
	       ---
	       advances iterator to next item.

*/

commc_error_t commc_lru_cache_iterator_next(commc_lru_cache_iterator_t* iterator) {

  if  (!iterator || !iterator->current) {
    return COMMC_ARGUMENT_ERROR;
  }

  iterator->current = iterator->current->next;
  return COMMC_SUCCESS;

}

/*

         commc_lru_cache_iterator_data()
	       ---
	       retrieves data from current iterator position.

*/

commc_error_t commc_lru_cache_iterator_data(commc_lru_cache_iterator_t* iterator,
                                             void** key, size_t* key_size,
                                             void** value, size_t* value_size) {

  if  (!iterator || !iterator->current || !key || !key_size || !value || !value_size) {
    return COMMC_ARGUMENT_ERROR;
  }

  *key = iterator->current->key;
  *key_size = iterator->current->key_size;
  *value = iterator->current->value;
  *value_size = iterator->current->value_size;

  return COMMC_SUCCESS;

}

/*

         commc_lru_cache_iterator_has_next()
	       ---
	       returns 1 if iterator has more items.

*/

int commc_lru_cache_iterator_has_next(commc_lru_cache_iterator_t* iterator) {

  if  (!iterator) {
    return 0;
  }

  return iterator->current != NULL;

}

/*
	==================================
             --- HASH API ---
	==================================
*/

/*

         commc_lru_cache_hash_djb2()
	       ---
	       djb2 hash function for cache keys.

*/

size_t commc_lru_cache_hash_djb2(const void* key, size_t key_size) {

  const unsigned char* bytes = (const unsigned char*)key;
  size_t               hash = 5381;
  size_t               i;

  for  (i = 0; i < key_size; i++) {
    hash = ((hash << 5) + hash) + bytes[i];
  }

  return hash;

}

/*

         commc_lru_cache_hash_fnv1a()
	       ---
	       fnv-1a hash function alternative.

*/

size_t commc_lru_cache_hash_fnv1a(const void* key, size_t key_size) {

  const unsigned char* bytes = (const unsigned char*)key;
  size_t               hash = 2166136261UL;
  size_t               i;

  for  (i = 0; i < key_size; i++) {
    hash ^= bytes[i];
    hash *= 16777619UL;
  }

  return hash;

}

/*

         commc_lru_cache_key_compare_memcmp()
	       ---
	       default key comparison using memcmp.

*/

int commc_lru_cache_key_compare_memcmp(const void* key1, size_t key1_size,
                                       const void* key2, size_t key2_size) {

  if  (key1_size != key2_size) {
    return 1;
  }

  return memcmp(key1, key2, key1_size);

}

/*

         commc_lru_cache_key_compare_string()
	       ---
	       string key comparison for null-terminated strings.

*/

int commc_lru_cache_key_compare_string(const void* key1, size_t key1_size,
                                       const void* key2, size_t key2_size) {

  /* ignore size parameters for null-terminated strings */
  (void)key1_size;
  (void)key2_size;

  return strcmp((const char*)key1, (const char*)key2);

}

/*
	==================================
             --- EOF ---
	==================================
*/