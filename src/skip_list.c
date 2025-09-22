/* 	
   ===================================
   C O M M C / S K I P _ L I S T . C
   PROBABILISTIC SKIP LIST IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

	                --- SKIP LIST IMPLEMENTATION ---

	    skip lists use multiple levels of linked lists to provide
	    probabilistic O(log n) search, insert, and delete operations.
	    the key insight is that higher levels contain fewer elements,
	    creating an express lane effect.
	    
	    level 0: all elements
	    level 1: ~50% of elements 
	    level 2: ~25% of elements
	    level 3: ~12.5% of elements
	    
	    randomization maintains balance without complex rotations.

*/

/* 
	==================================
             --- INCLUDES ---
	==================================
*/

#include "commc/skip_list.h"
#include <stdlib.h>         /* MALLOC, FREE, RAND, SRAND */
#include <string.h>         /* MEMCMP, MEMCPY, STRLEN, STRCMP */
#include <time.h>           /* TIME */

/* 
	==================================
             --- STATIC VARS ---
	==================================
*/

static int skip_list_random_initialized = 0;

/* 
	==================================
             --- HELPERS ---
	==================================
*/

/*

         skip_list_initialize_random()
	       ---
	       initializes random number generator for level generation.
	       called once on first skip list creation.

*/

static void skip_list_initialize_random(void) {

  if (!skip_list_random_initialized) {

    srand((unsigned int)time(NULL));
    skip_list_random_initialized = 1;
    
  }
  
}

/*

         skip_list_random_level()
	       ---
	       generates random level for new node based on probability.
	       uses geometric distribution - each level has probability^level
	       chance of being selected.

*/

static size_t skip_list_random_level(double probability) {

  size_t level = 0;
  
  while (level < COMMC_SKIP_LIST_MAX_LEVEL - 1 && 
         (double)rand() / RAND_MAX < probability) {

    level++;
    
  }
  
  return level;
  
}

/*

         skip_list_create_node()
	       ---
	       creates a new skip list node with specified level.
	       allocates memory for forward pointers array.

*/

static commc_skip_list_node_t* skip_list_create_node(const void* key, size_t key_size,
                                                      const void* value, size_t value_size,
                                                      size_t level) {

  commc_skip_list_node_t* node;
  size_t                  i;
  
  node = (commc_skip_list_node_t*)malloc(sizeof(commc_skip_list_node_t));
  
  if (!node) {

    return NULL;
    
  }

  /* allocate memory for key and value */
  
  node->key = malloc(key_size);
  
  if (!node->key) {

    free(node);
    return NULL;
    
  }
  
  node->value = malloc(value_size);
  
  if (!node->value) {

    free(node->key);
    free(node);
    return NULL;
    
  }

  /* copy key and value data */
  
  memcpy(node->key, key, key_size);
  memcpy(node->value, value, value_size);
  
  node->key_size   = key_size;
  node->value_size = value_size;
  node->level      = level;

  /* allocate forward pointers array */
  
  node->forward = (commc_skip_list_node_t**)malloc(
    (level + 1) * sizeof(commc_skip_list_node_t*));
    
  if (!node->forward) {

    free(node->value);
    free(node->key);
    free(node);
    return NULL;
    
  }

  /* initialize all forward pointers to NULL */
  
  for (i = 0; i <= level; i++) {

    node->forward[i] = NULL;
    
  }
  
  return node;
  
}

/*

         skip_list_destroy_node()
	       ---
	       destroys a skip list node and frees all associated memory.

*/

static void skip_list_destroy_node(commc_skip_list_node_t* node) {

  if (!node) {

    return;
    
  }
  
  free(node->key);
  free(node->value);
  free(node->forward);
  free(node);
  
}

/*

         skip_list_find_update_array()
	       ---
	       finds the update array for insertion/deletion operations.
	       update[i] contains the rightmost node at level i that is
	       less than the search key.

*/

static commc_skip_list_node_t** skip_list_find_update_array(commc_skip_list_t* list,
                                                             const void* key, size_t key_size,
                                                             commc_skip_list_compare_t compare) {

  commc_skip_list_node_t**  update;
  commc_skip_list_node_t*   current;
  size_t                    i;
  int                       cmp_result;
  
  update = (commc_skip_list_node_t**)malloc(
    COMMC_SKIP_LIST_MAX_LEVEL * sizeof(commc_skip_list_node_t*));
    
  if (!update) {

    return NULL;
    
  }
  
  current = list->header;

  /* traverse from highest to lowest level */
  
  for (i = list->max_level; i != SIZE_MAX; i--) {

    /* move forward while next node key is less than search key */
    
    while (current->forward[i] != NULL) {

      cmp_result = compare(current->forward[i]->key, current->forward[i]->key_size,
                           key, key_size);
                           
      if (cmp_result >= 0) {

        break; /* found insertion point */
        
      }
      
      current = current->forward[i];
      
    }
    
    update[i] = current;
    
  }
  
  return update;
  
}

/* 
	==================================
             --- CORE API ---
	==================================
*/

/*

         commc_skip_list_create()
	       ---
	       creates a new skip list with default probability (0.5).

*/

commc_skip_list_t* commc_skip_list_create(void) {

  return commc_skip_list_create_with_probability(COMMC_SKIP_LIST_DEFAULT_PROBABILITY);
  
}

/*

         commc_skip_list_create_with_probability()
	       ---
	       creates skip list with custom level promotion probability.

*/

commc_skip_list_t* commc_skip_list_create_with_probability(double probability) {

  commc_skip_list_t* list;
  size_t             i;
  
  if (probability <= 0.0 || probability >= 1.0) {

    return NULL; /* invalid probability */
    
  }
  
  skip_list_initialize_random();
  
  list = (commc_skip_list_t*)malloc(sizeof(commc_skip_list_t));
  
  if (!list) {

    return NULL;
    
  }

  /* create header node with maximum level */
  
  list->header = skip_list_create_node(NULL, 0, NULL, 0, COMMC_SKIP_LIST_MAX_LEVEL - 1);
  
  if (!list->header) {

    free(list);
    return NULL;
    
  }

  /* initialize header forward pointers to NULL */
  
  for (i = 0; i < COMMC_SKIP_LIST_MAX_LEVEL; i++) {

    list->header->forward[i] = NULL;
    
  }
  
  list->max_level  = 0;
  list->size       = 0;
  list->probability = probability;
  
  return list;
  
}

/*

         commc_skip_list_destroy()
	       ---
	       destroys skip list and frees all associated memory.

*/

void commc_skip_list_destroy(commc_skip_list_t* list) {

  commc_skip_list_node_t* current;
  commc_skip_list_node_t* next;
  
  if (!list) {

    return;
    
  }
  
  current = list->header;
  
  while (current != NULL) {

    next = current->forward[0];
    skip_list_destroy_node(current);
    current = next;
    
  }
  
  free(list);
  
}

/*

         commc_skip_list_insert()
	       ---
	       inserts a key-value pair into the skip list.
	       if key already exists, updates the value.

*/

commc_error_t commc_skip_list_insert(commc_skip_list_t* list,
                                      const void* key, size_t key_size,
                                      const void* value, size_t value_size,
                                      commc_skip_list_compare_t compare) {

  commc_skip_list_node_t**  update;
  commc_skip_list_node_t*   current;
  commc_skip_list_node_t*   new_node;
  size_t                    new_level;
  size_t                    i;
  int                       cmp_result;
  
  if (!list || !key || !value || !compare || key_size == 0 || value_size == 0) {

    return COMMC_ARGUMENT_ERROR;
    
  }
  
  update = skip_list_find_update_array(list, key, key_size, compare);
  
  if (!update) {

    return COMMC_MEMORY_ERROR;
    
  }
  
  current = update[0]->forward[0];

  /* check if key already exists */
  
  if (current != NULL) {

    cmp_result = compare(current->key, current->key_size, key, key_size);
    
    if (cmp_result == 0) {

      /* key exists - update value */
      
      void* new_value = malloc(value_size);
      
      if (!new_value) {

        free(update);
        return COMMC_MEMORY_ERROR;
        
      }
      
      memcpy(new_value, value, value_size);
      free(current->value);
      
      current->value      = new_value;
      current->value_size = value_size;
      
      free(update);
      return COMMC_SUCCESS;
      
    }
    
  }

  /* create new node with random level */
  
  new_level = skip_list_random_level(list->probability);
  new_node  = skip_list_create_node(key, key_size, value, value_size, new_level);
  
  if (!new_node) {

    free(update);
    return COMMC_MEMORY_ERROR;
    
  }

  /* update max_level if necessary */
  
  if (new_level > list->max_level) {

    for (i = list->max_level + 1; i <= new_level; i++) {

      update[i] = list->header;
      
    }
    
    list->max_level = new_level;
    
  }

  /* insert node by updating forward pointers */
  
  for (i = 0; i <= new_level; i++) {

    new_node->forward[i] = update[i]->forward[i];
    update[i]->forward[i] = new_node;
    
  }
  
  list->size++;
  
  free(update);
  return COMMC_SUCCESS;
  
}

/*

         commc_skip_list_search()
	       ---
	       searches for a key in the skip list.

*/

commc_error_t commc_skip_list_search(commc_skip_list_t* list,
                                      const void* key, size_t key_size,
                                      void** value, size_t* value_size,
                                      commc_skip_list_compare_t compare) {

  commc_skip_list_node_t* current;
  size_t                  i;
  int                     cmp_result;
  
  if (!list || !key || !value || !value_size || !compare || key_size == 0) {

    return COMMC_ARGUMENT_ERROR;
    
  }
  
  current = list->header;

  /* traverse from highest to lowest level */
  
  for (i = list->max_level; i != SIZE_MAX; i--) {

    while (current->forward[i] != NULL) {

      cmp_result = compare(current->forward[i]->key, current->forward[i]->key_size,
                           key, key_size);
                           
      if (cmp_result == 0) {

        /* key found */
        
        *value      = current->forward[i]->value;
        *value_size = current->forward[i]->value_size;
        return COMMC_SUCCESS;
        
      } else if (cmp_result > 0) {

        break; /* key would be before this node */
        
      }
      
      current = current->forward[i];
      
    }
    
  }
  
  return COMMC_FAILURE; /* key not found */
  
}

/*

         commc_skip_list_delete()
	       ---
	       removes a key-value pair from the skip list.

*/

commc_error_t commc_skip_list_delete(commc_skip_list_t* list,
                                      const void* key, size_t key_size,
                                      commc_skip_list_compare_t compare) {

  commc_skip_list_node_t**  update;
  commc_skip_list_node_t*   current;
  size_t                    i;
  
  if (!list || !key || !compare || key_size == 0) {

    return COMMC_ARGUMENT_ERROR;
    
  }
  
  update = skip_list_find_update_array(list, key, key_size, compare);
  
  if (!update) {

    return COMMC_MEMORY_ERROR;
    
  }
  
  current = update[0]->forward[0];

  /* check if key exists */
  
  if (current == NULL || compare(current->key, current->key_size, key, key_size) != 0) {

    free(update);
    return COMMC_FAILURE; /* key not found */
    
  }

  /* remove node by updating forward pointers */
  
  for (i = 0; i <= current->level; i++) {

    update[i]->forward[i] = current->forward[i];
    
  }

  /* update max_level if necessary */
  
  while (list->max_level > 0 && list->header->forward[list->max_level] == NULL) {

    list->max_level--;
    
  }
  
  skip_list_destroy_node(current);
  list->size--;
  
  free(update);
  return COMMC_SUCCESS;
  
}

/* 
	==================================
             --- INFO API ---
	==================================
*/

/*

         commc_skip_list_size()
	       ---
	       returns the number of key-value pairs in the skip list.

*/

size_t commc_skip_list_size(commc_skip_list_t* list) {

  return list ? list->size : 0;
  
}

/*

         commc_skip_list_is_empty()
	       ---
	       returns 1 if skip list is empty, 0 otherwise.

*/

int commc_skip_list_is_empty(commc_skip_list_t* list) {

  return list ? (list->size == 0) : 1;
  
}

/*

         commc_skip_list_contains()
	       ---
	       returns 1 if key exists in skip list, 0 otherwise.

*/

int commc_skip_list_contains(commc_skip_list_t* list,
                             const void* key, size_t key_size,
                             commc_skip_list_compare_t compare) {

  void*  value;
  size_t value_size;
  
  return (commc_skip_list_search(list, key, key_size, &value, &value_size, compare) 
          == COMMC_SUCCESS) ? 1 : 0;
  
}

/*

         commc_skip_list_get_max_level()
	       ---
	       returns the current maximum level in the skip list.

*/

size_t commc_skip_list_get_max_level(commc_skip_list_t* list) {

  return list ? list->max_level : 0;
  
}

/*

         commc_skip_list_memory_usage()
	       ---
	       estimates total memory usage in bytes.

*/

size_t commc_skip_list_memory_usage(commc_skip_list_t* list) {

  commc_skip_list_node_t* current;
  size_t                  total;
  
  if (!list) {

    return 0;
    
  }
  
  total = sizeof(commc_skip_list_t);
  current = list->header;
  
  while (current != NULL) {

    total += sizeof(commc_skip_list_node_t);
    total += current->key_size;
    total += current->value_size;
    total += (current->level + 1) * sizeof(commc_skip_list_node_t*);
    
    current = current->forward[0];
    
  }
  
  return total;
  
}

/* 
	==================================
             --- ITERATION API ---
	==================================
*/

/*

         commc_skip_list_iterator_begin()
	       ---
	       creates iterator starting from smallest key.

*/

commc_skip_list_iterator_t commc_skip_list_iterator_begin(commc_skip_list_t* list) {

  commc_skip_list_iterator_t iterator;
  
  iterator.list    = list;
  iterator.current = list ? list->header->forward[0] : NULL;
  
  return iterator;
  
}

/*

         commc_skip_list_iterator_next()
	       ---
	       advances iterator to next key in sorted order.

*/

commc_error_t commc_skip_list_iterator_next(commc_skip_list_iterator_t* iterator) {

  if (!iterator || !iterator->current) {

    return COMMC_ARGUMENT_ERROR;
    
  }
  
  iterator->current = iterator->current->forward[0];
  
  return COMMC_SUCCESS;
  
}

/*

         commc_skip_list_iterator_data()
	       ---
	       retrieves key-value pair from current iterator position.

*/

commc_error_t commc_skip_list_iterator_data(commc_skip_list_iterator_t* iterator,
                                             void** key, size_t* key_size,
                                             void** value, size_t* value_size) {

  if (!iterator || !iterator->current || !key || !key_size || !value || !value_size) {

    return COMMC_ARGUMENT_ERROR;
    
  }
  
  *key       = iterator->current->key;
  *key_size  = iterator->current->key_size;
  *value     = iterator->current->value;
  *value_size = iterator->current->value_size;
  
  return COMMC_SUCCESS;
  
}

/*

         commc_skip_list_iterator_has_next()
	       ---
	       returns 1 if iterator has more elements.

*/

int commc_skip_list_iterator_has_next(commc_skip_list_iterator_t* iterator) {

  return (iterator && iterator->current) ? 1 : 0;
  
}

/* 
	==================================
             --- UTILITY API ---
	==================================
*/

/*

         commc_skip_list_clear()
	       ---
	       removes all elements from skip list.

*/

void commc_skip_list_clear(commc_skip_list_t* list) {

  commc_skip_list_node_t* current;
  commc_skip_list_node_t* next;
  size_t                  i;
  
  if (!list) {

    return;
    
  }

  /* free all nodes except header */
  
  current = list->header->forward[0];
  
  while (current != NULL) {

    next = current->forward[0];
    skip_list_destroy_node(current);
    current = next;
    
  }

  /* reset header forward pointers */
  
  for (i = 0; i < COMMC_SKIP_LIST_MAX_LEVEL; i++) {

    list->header->forward[i] = NULL;
    
  }
  
  list->max_level = 0;
  list->size      = 0;
  
}

/*

         commc_skip_list_find_min()
	       ---
	       finds the minimum key in the skip list.

*/

commc_error_t commc_skip_list_find_min(commc_skip_list_t* list,
                                        void** key, size_t* key_size,
                                        void** value, size_t* value_size) {

  commc_skip_list_node_t* first_node;
  
  if (!list || !key || !key_size || !value || !value_size) {

    return COMMC_ARGUMENT_ERROR;
    
  }
  
  first_node = list->header->forward[0];
  
  if (!first_node) {

    return COMMC_FAILURE; /* list is empty */
    
  }
  
  *key       = first_node->key;
  *key_size  = first_node->key_size;
  *value     = first_node->value;
  *value_size = first_node->value_size;
  
  return COMMC_SUCCESS;
  
}

/*

         commc_skip_list_find_max()
	       ---
	       finds the maximum key in the skip list.

*/

commc_error_t commc_skip_list_find_max(commc_skip_list_t* list,
                                        void** key, size_t* key_size,
                                        void** value, size_t* value_size) {

  commc_skip_list_node_t* current;
  
  if (!list || !key || !key_size || !value || !value_size) {

    return COMMC_ARGUMENT_ERROR;
    
  }

  if (list->size == 0) {

    return COMMC_FAILURE; /* list is empty */
    
  }

  /* traverse level 0 to find last node */
  
  current = list->header;
  
  while (current->forward[0] != NULL) {

    current = current->forward[0];
    
  }
  
  *key       = current->key;
  *key_size  = current->key_size;
  *value     = current->value;
  *value_size = current->value_size;
  
  return COMMC_SUCCESS;
  
}

/*

         commc_skip_list_get_level_counts()
	       ---
	       returns array with count of nodes at each level.

*/

commc_error_t commc_skip_list_get_level_counts(commc_skip_list_t* list,
                                                size_t* level_counts) {

  commc_skip_list_node_t* current;
  size_t                  i;
  size_t                  level;
  
  if (!list || !level_counts) {

    return COMMC_ARGUMENT_ERROR;
    
  }

  /* initialize counts to zero */
  
  for (i = 0; i < COMMC_SKIP_LIST_MAX_LEVEL; i++) {

    level_counts[i] = 0;
    
  }

  /* count nodes at each level */
  
  current = list->header->forward[0];
  
  while (current != NULL) {

    for (level = 0; level <= current->level; level++) {

      level_counts[level]++;
      
    }
    
    current = current->forward[0];
    
  }
  
  return COMMC_SUCCESS;
  
}

/* 
	==================================
             --- RANGE API ---
	==================================
*/

/*

         commc_skip_list_range_search()
	       ---
	       finds all keys within a specified range [min_key, max_key].

*/

size_t commc_skip_list_range_search(commc_skip_list_t* list,
                                     const void* min_key, size_t min_key_size,
                                     const void* max_key, size_t max_key_size,
                                     commc_skip_list_node_t** results,
                                     size_t max_results,
                                     commc_skip_list_compare_t compare) {

  commc_skip_list_node_t* current;
  size_t                  count;
  size_t                  i;
  int                     min_cmp;
  int                     max_cmp;
  
  if (!list || !min_key || !max_key || !results || !compare || max_results == 0) {

    return 0;
    
  }

  /* find starting position (first node >= min_key) */
  
  current = list->header;
  
  for (i = list->max_level; i != SIZE_MAX; i--) {

    while (current->forward[i] != NULL) {

      min_cmp = compare(current->forward[i]->key, current->forward[i]->key_size,
                        min_key, min_key_size);
                        
      if (min_cmp >= 0) {

        break;
        
      }
      
      current = current->forward[i];
      
    }
    
  }
  
  current = current->forward[0];
  count   = 0;

  /* collect nodes in range */
  
  while (current != NULL && count < max_results) {

    min_cmp = compare(current->key, current->key_size, min_key, min_key_size);
    max_cmp = compare(current->key, current->key_size, max_key, max_key_size);
    
    if (min_cmp >= 0 && max_cmp <= 0) {

      results[count] = current;
      count++;
      
    } else if (max_cmp > 0) {

      break; /* beyond range */
      
    }
    
    current = current->forward[0];
    
  }
  
  return count;
  
}

/* 
	==================================
             --- COMPARE API ---
	==================================
*/

/*

         commc_skip_list_compare_int()
	       ---
	       comparison function for integer keys.

*/

int commc_skip_list_compare_int(const void* key1, size_t key1_size,
                                const void* key2, size_t key2_size) {

  int val1;
  int val2;
  
  if (key1_size != sizeof(int) || key2_size != sizeof(int)) {

    return 0; /* invalid comparison */
    
  }
  
  val1 = *(const int*)key1;
  val2 = *(const int*)key2;
  
  if (val1 < val2) {

    return -1;
    
  } else if (val1 > val2) {

    return 1;
    
  } else {

    return 0;
    
  }
  
}

/*

         commc_skip_list_compare_string()
	       ---
	       comparison function for null-terminated string keys.

*/

int commc_skip_list_compare_string(const void* key1, size_t key1_size,
                                   const void* key2, size_t key2_size) {

  /* unused parameters in string comparison */
  (void)key1_size;
  (void)key2_size;
  
  return strcmp((const char*)key1, (const char*)key2);
  
}

/*

         commc_skip_list_compare_memcmp()
	       ---
	       comparison function using memcmp for arbitrary byte keys.

*/

int commc_skip_list_compare_memcmp(const void* key1, size_t key1_size,
                                   const void* key2, size_t key2_size) {

  size_t min_size;
  int    result;
  
  min_size = (key1_size < key2_size) ? key1_size : key2_size;
  result   = memcmp(key1, key2, min_size);
  
  if (result != 0) {

    return result;
    
  }

  /* if prefixes are equal, compare sizes */
  
  if (key1_size < key2_size) {

    return -1;
    
  } else if (key1_size > key2_size) {

    return 1;
    
  } else {

    return 0;
    
  }
  
}

/*
	==================================
             --- EOF ---
	==================================
*/