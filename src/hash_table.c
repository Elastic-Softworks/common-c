/*
   ===================================
   C O M M O N - C
   HASH TABLE IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- HASH TABLE MODULE ---

    implementation of the hash table data structure.
    see include/commc/hash_table.h for function
    prototypes and documentation.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#include "commc/hash_table.h"
#include "commc/list.h"
#include "commc/error.h"
#include <stdlib.h>
#include <string.h>

/*
	==================================
             --- STRUCTS ---
	==================================
*/

/* a single key-value pair stored in a hash table node. */

typedef struct {

  char* key;   /* string key */
  void* value; /* pointer to user data */

} commc_hash_entry_t;

/* internal hash table structure. */

struct commc_hash_table_t {

  commc_list_t**         buckets;       /* array of linked lists (buckets) */
  size_t                 capacity;      /* number of buckets */
  size_t                 size;          /* number of elements stored */
  commc_hash_function_t  hash_function; /* custom hash function pointer */
  int                    auto_resize;   /* enable automatic resizing */

};

/*
	==================================
             --- STATIC FUNCS ---
	==================================
*/

/*

         hash_string_djb2()
	       ---
	       a simple and effective string hashing algorithm.
	       it produces a hash value for a given string.

*/

static unsigned long hash_string_djb2(const char* str) {

  unsigned long hash = 5381;
  int           c;

  while  ((c = *str++)) {

    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  }

  return hash;

}

/*

         get_hash_value()
	       ---
	       gets the hash value for a key using either the custom
	       hash function (if set) or the default djb2 algorithm.

*/

static unsigned long get_hash_value(commc_hash_table_t* table, const char* key) {

  if  (table->hash_function) {

    return table->hash_function(key);

  }

  return hash_string_djb2(key);

}

/*
	==================================
             --- FUNCS ---
	==================================
*/

/*

         commc_hash_table_create()
	       ---
	       allocates and initializes a new hash table.
	       each bucket is an empty linked list.

*/

commc_hash_table_t* commc_hash_table_create(size_t capacity) {

  commc_hash_table_t* table;
  size_t              i;

  table = (commc_hash_table_t*)malloc(sizeof(commc_hash_table_t));

  if  (!table) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  table->buckets = (commc_list_t**)malloc(sizeof(commc_list_t*) * capacity);

  if  (!table->buckets) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    free(table);
    return NULL;

  }

  table->capacity      = capacity;
  table->size          = 0;
  table->hash_function = NULL;     /* NULL means use default djb2 */
  table->auto_resize   = 0;        /* disabled by default */

  for  (i = 0; i < capacity; i++) {

    table->buckets[i] = commc_list_create();

    if  (!table->buckets[i]) {

      /* cleanup partial init */

      size_t j;
      
      commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
      
      for  (j = 0; j < i; j++) {
        commc_list_destroy(table->buckets[j]);
      }
      free(table->buckets);
      free(table);
      return NULL;

    }

  }

  return table;

}

/*

         commc_hash_table_destroy()
	       ---
	       frees all memory used by the hash table,
	       including keys and bucket lists.

*/

void commc_hash_table_destroy(commc_hash_table_t* table) {

  size_t i;

  if  (!table) {

    return;

  }

  for  (i = 0; i < table->capacity; i++) {

    commc_list_node_t* current_node = table->buckets[i]->head;

    while  (current_node) {

      commc_hash_entry_t* entry = (commc_hash_entry_t*)current_node->data;
      free(entry->key); /* free the key string */
      free(entry);      /* free the entry struct */
      current_node = current_node->next;

    }

    commc_list_destroy(table->buckets[i]);

  }

  free(table->buckets);
  free(table);

}

/*

         commc_hash_table_insert()
	       ---
	       adds or updates a key-value pair.
	       returns COMMC_SUCCESS on success, appropriate error code on failure.

*/

commc_error_t commc_hash_table_insert(commc_hash_table_t* table, const char* key, void* value) {

  unsigned long       hash_val;
  size_t              bucket_index;
  commc_list_node_t*  current_node;
  commc_hash_entry_t* entry;

  if  (!table || !key) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;

  }

  /* check for auto-resize if load factor would exceed 0.75 */

  if  (table->auto_resize && ((float)(table->size + 1) / table->capacity) > 0.75f) {

    commc_error_t resize_result = commc_hash_table_rehash(table, table->capacity * 2);

    if  (resize_result != COMMC_SUCCESS) {

      return resize_result;

    }

  }

  hash_val     = get_hash_value(table, key);
  bucket_index = hash_val % table->capacity;

  /* check if key already exists */

  current_node = table->buckets[bucket_index]->head;

  while  (current_node) {

    entry = (commc_hash_entry_t*)current_node->data;

    if  (strcmp(entry->key, key) == 0) {

      entry->value = value; /* update value */
      return COMMC_SUCCESS;

    }

    current_node = current_node->next;

  }

  /* key not found, create new entry */
  
  entry = (commc_hash_entry_t*)malloc(sizeof(commc_hash_entry_t));

  if  (!entry) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return COMMC_MEMORY_ERROR;

  }

  entry->key = strdup(key); /* duplicate key string */

  if  (!entry->key) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    free(entry);
    return COMMC_MEMORY_ERROR;

  }

  entry->value = value;

  if  (!commc_list_push_back(table->buckets[bucket_index], entry)) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    free(entry->key);
    free(entry);
    return COMMC_MEMORY_ERROR;

  }

  table->size++;

  return COMMC_SUCCESS;

}

/*

         commc_hash_table_get()
	       ---
	       retrieves the value for a given key.
	       returns null if key not found.

*/

void* commc_hash_table_get(commc_hash_table_t* table, const char* key) {

  unsigned long       hash_val;
  size_t              bucket_index;
  commc_list_node_t*  current_node;
  commc_hash_entry_t* entry;

  if  (!table || !key) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  hash_val     = get_hash_value(table, key);
  bucket_index = hash_val % table->capacity;

  current_node = table->buckets[bucket_index]->head;

  while  (current_node) {

    entry = (commc_hash_entry_t*)current_node->data;

    if  (strcmp(entry->key, key) == 0) {

      return entry->value;

    }

    current_node = current_node->next;

  }

  return NULL; /* key not found */

}

/*

         commc_hash_table_remove()
	       ---
	       removes a key-value pair from the hash table.

*/

void commc_hash_table_remove(commc_hash_table_t* table, const char* key) {

  unsigned long       hash_val;
  size_t              bucket_index;
  commc_list_node_t*  current_node;
  commc_hash_entry_t* entry;

  if  (!table || !key) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return;

  }

  hash_val     = get_hash_value(table, key);
  bucket_index = hash_val % table->capacity;

  current_node = table->buckets[bucket_index]->head;

  while  (current_node) {

    entry = (commc_hash_entry_t*)current_node->data;

    if  (strcmp(entry->key, key) == 0) {

      /* remove node from list */
      if  (current_node->prev) {
        current_node->prev->next = current_node->next;
      } else {
        table->buckets[bucket_index]->head = current_node->next;
      }

      if  (current_node->next) {
        current_node->next->prev = current_node->prev;
      } else {
        table->buckets[bucket_index]->tail = current_node->prev;
      }

      table->buckets[bucket_index]->size--;

      free(entry->key);
      free(entry);
      free(current_node); /* free the list node itself */
      table->size--;
      break;

    }

    current_node = current_node->next;

  }

}

/*

         commc_hash_table_size()
	       ---
	       returns the number of elements in the hash table.

*/

size_t commc_hash_table_size(commc_hash_table_t* table) {

  return table ? table->size : 0;

}

/*

         commc_hash_table_capacity()
	       ---
	       returns the number of buckets allocated for the hash table.
	       this indicates the internal capacity for element distribution
	       and can help assess hash table performance characteristics.

*/

size_t commc_hash_table_capacity(commc_hash_table_t* table) {

  return table ? table->capacity : 0;

}

/*

         commc_hash_table_clear()
	       ---
	       removes all entries from the hash table while
	       preserving the bucket structure for efficient reuse.
	       all keys and bucket contents are freed.

*/

void commc_hash_table_clear(commc_hash_table_t* table) {

  size_t i;

  if  (!table) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return;

  }

  for  (i = 0; i < table->capacity; i++) {

    commc_list_node_t* current;

    if  (!table->buckets[i]) {

      continue;

    }

    current = table->buckets[i]->head;

    while  (current) {

      commc_hash_entry_t* entry = (commc_hash_entry_t*)current->data;
      
      if  (entry && entry->key) {

        free(entry->key);

      }

      if  (entry) {

        free(entry);

      }

      current = current->next;

    }

    commc_list_clear(table->buckets[i]);

  }

  table->size = 0;

}

/*

         commc_hash_table_rehash()
	       ---
	       resizes the hash table to a new capacity and rehashes
	       all existing elements. this is used both for manual
	       resizing and automatic load factor management.

*/

commc_error_t commc_hash_table_rehash(commc_hash_table_t* table, size_t new_capacity) {

  commc_list_t**      old_buckets;
  size_t              old_capacity;
  size_t              i;
  commc_list_node_t*  current;

  if  (!table || new_capacity == 0) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;

  }

  /* save old bucket array */
  old_buckets  = table->buckets;
  old_capacity = table->capacity;

  /* allocate new bucket array */
  table->buckets = (commc_list_t**)malloc(sizeof(commc_list_t*) * new_capacity);

  if  (!table->buckets) {

    table->buckets = old_buckets; /* restore old buckets on failure */
    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return COMMC_MEMORY_ERROR;

  }

  table->capacity = new_capacity;
  table->size     = 0; /* will be re-counted during rehashing */

  /* initialize new buckets */

  for  (i = 0; i < new_capacity; i++) {

    table->buckets[i] = commc_list_create();

    if  (!table->buckets[i]) {

      size_t j;

      commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
      
      /* cleanup partial init and restore old state */

      for  (j = 0; j < i; j++) {
        commc_list_destroy(table->buckets[j]);
      }
      free(table->buckets);
      table->buckets  = old_buckets;
      table->capacity = old_capacity;
      return COMMC_MEMORY_ERROR;

    }

  }

  /* rehash all existing entries from old buckets to new buckets */

  for  (i = 0; i < old_capacity; i++) {

    current = old_buckets[i]->head;

    while  (current) {

      commc_hash_entry_t* entry       = (commc_hash_entry_t*)current->data;
      unsigned long       hash_val    = get_hash_value(table, entry->key);
      size_t              bucket_idx  = hash_val % table->capacity;

      /* insert into new bucket (we know the key doesn't exist yet) */

      if  (!commc_list_push_back(table->buckets[bucket_idx], entry)) {

        size_t j;

        commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
        
        /* cleanup and restore old state on failure */

        for  (j = 0; j < new_capacity; j++) {
          commc_list_destroy(table->buckets[j]);
        }
        free(table->buckets);
        table->buckets  = old_buckets;
        table->capacity = old_capacity;
        return COMMC_MEMORY_ERROR;

      }

      table->size++;
      current = current->next;

    }

  }

  /* destroy old bucket structure (but not the entries, which were moved) */

  for  (i = 0; i < old_capacity; i++) {

    commc_list_node_t* node = old_buckets[i]->head;

    while  (node) {

      commc_list_node_t* next = node->next;
      free(node); /* free list node but not the entry data */
      node = next;

    }

    free(old_buckets[i]); /* free list structure */

  }

  free(old_buckets);

  return COMMC_SUCCESS;

}

/*

         commc_hash_table_load_factor()
	       ---
	       calculates and returns the current load factor.
	       load factor = number of elements / number of buckets.
	       values approaching 1.0 indicate potential performance degradation.

*/

float commc_hash_table_load_factor(commc_hash_table_t* table) {

  if  (!table || table->capacity == 0) {

    return 0.0f;

  }

  return (float)table->size / (float)table->capacity;

}

/*

         commc_hash_table_set_hash_function()
	       ---
	       sets a custom hash function for future hash operations.
	       existing elements are NOT rehashed automatically.
	       set to NULL to revert to default djb2 algorithm.

*/

void commc_hash_table_set_hash_function(commc_hash_table_t* table, commc_hash_function_t hash_func) {

  if  (!table) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return;

  }

  table->hash_function = hash_func;

}

/*

         commc_hash_table_set_auto_resize()
	       ---
	       enables or disables automatic resizing.
	       when enabled, the table automatically doubles in size
	       when load factor exceeds 0.75 during insertions.

*/

void commc_hash_table_set_auto_resize(commc_hash_table_t* table, int enable) {

  if  (!table) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return;

  }

  table->auto_resize = enable ? 1 : 0;

}

/*
	==================================
             --- EOF ---
	==================================
*/
