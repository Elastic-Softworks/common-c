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

  commc_list_t** buckets; /* array of linked lists (buckets) */
  size_t         capacity;/* number of buckets */
  size_t         size;    /* number of elements stored */

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

    return NULL;

  }

  table->buckets = (commc_list_t**)malloc(sizeof(commc_list_t*) * capacity);

  if  (!table->buckets) {

    free(table);
    return NULL;

  }

  table->capacity = capacity;
  table->size     = 0;

  for  (i = 0; i < capacity; i++) {

    table->buckets[i] = commc_list_create();

    if  (!table->buckets[i]) {

      /* cleanup partial init */

      size_t j;
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
	       returns 1 on success, 0 on failure.

*/

int commc_hash_table_insert(commc_hash_table_t* table, const char* key, void* value) {

  unsigned long       hash_val;
  size_t              bucket_index;
  commc_list_node_t*  current_node;
  commc_hash_entry_t* entry;

  if  (!table || !key) {

    return 0;

  }

  hash_val     = hash_string_djb2(key);
  bucket_index = hash_val % table->capacity;

  /* check if key already exists */

  current_node = table->buckets[bucket_index]->head;

  while  (current_node) {

    entry = (commc_hash_entry_t*)current_node->data;

    if  (strcmp(entry->key, key) == 0) {

      entry->value = value; /* update value */
      return 1;

    }

    current_node = current_node->next;

  }

  /* key not found, create new entry */
  
  entry = (commc_hash_entry_t*)malloc(sizeof(commc_hash_entry_t));

  if  (!entry) {

    return 0;

  }

  entry->key = strdup(key); /* duplicate key string */

  if  (!entry->key) {

    free(entry);
    return 0;

  }

  entry->value = value;

  if  (!commc_list_push_back(table->buckets[bucket_index], entry)) {

    free(entry->key);
    free(entry);
    return 0;

  }

  table->size++;

  return 1;

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

    return NULL;

  }

  hash_val     = hash_string_djb2(key);
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
  int                 found = 0;

  if  (!table || !key) {

    return;

  }

  hash_val     = hash_string_djb2(key);
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
      found = 1;
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
	==================================
             --- EOF ---
	==================================
*/
