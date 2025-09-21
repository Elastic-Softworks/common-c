/*
   ===================================
   C O M M O N - C
   LINKED LIST IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- LIST MODULE ---

    implementation of the doubly linked list.
    see include/commc/list.h for function
    prototypes and documentation.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#include    <stdlib.h>

#include    "commc/list.h"
#include    "commc/error.h"

/*
	==================================
             --- STRUCTS ---
	==================================
*/


/*
	==================================
             --- FUNCS ---
	==================================
*/

/*

         commc_list_create()
	       ---
	       allocates and initializes an empty list.

*/

commc_list_t* commc_list_create(void) {

  commc_list_t* list;
  
  list = (commc_list_t*)malloc(sizeof(commc_list_t));

  if  (!list) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  list->head = NULL;
  list->tail = NULL;
  list->size = 0;

  return list;

}

/*

         commc_list_destroy()
	       ---
	       frees all nodes in the list, then the list itself.

*/

void commc_list_destroy(commc_list_t* list) {

  commc_list_node_t* current;

  if  (!list) {

    return;

  }

  current = list->head;

  while  (current) {

    commc_list_node_t* next = current->next;
    free(current);
    current = next;

  }

  free(list);

}

/*

         commc_list_push_front()
	       ---
	       creates a new node and adds it to the front.

*/

commc_list_node_t* commc_list_push_front(commc_list_t* list, void* data) {

  commc_list_node_t* new_node;

  if  (!list) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  new_node = (commc_list_node_t*)malloc(sizeof(commc_list_node_t));

  if  (!new_node) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  new_node->data = data;
  new_node->prev = NULL;
  new_node->next = list->head;

  if  (list->head) {

    list->head->prev = new_node;

  } else {

    list->tail = new_node;

  }

  list->head = new_node;
  list->size++;

  return new_node;

}

/*

         commc_list_push_back()
	       ---
	       creates a new node and adds it to the end.

*/

commc_list_node_t* commc_list_push_back(commc_list_t* list, void* data) {

  commc_list_node_t* new_node;

  if  (!list) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  new_node = (commc_list_node_t*)malloc(sizeof(commc_list_node_t));

  if  (!new_node) {

    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  new_node->data = data;
  new_node->prev = list->tail;
  new_node->next = NULL;

  if  (list->tail) {

    list->tail->next = new_node;

  } else {

    list->head = new_node;

  }

  list->tail = new_node;
  list->size++;

  return new_node;

}

/*

         commc_list_pop_front()
	       ---
	       removes and frees the first node.

*/

void commc_list_pop_front(commc_list_t* list) {

  commc_list_node_t* old_head;

  if  (!list || !list->head) {

    return;

  }

  old_head   = list->head;
  list->head = old_head->next;

  if  (list->head) {

    list->head->prev = NULL;

  } else {

    list->tail = NULL;

  }

  free(old_head);
  list->size--;

}

/*

         commc_list_pop_back()
	       ---
	       removes and frees the last node.

*/

void commc_list_pop_back(commc_list_t* list) {

  commc_list_node_t* old_tail;

  if  (!list || !list->tail) {

    return;

  }

  old_tail   = list->tail;
  list->tail = old_tail->prev;

  if  (list->tail) {

    list->tail->next = NULL;

  } else {

    list->head = NULL;

  }

  free(old_tail);
  list->size--;

}

/*

         commc_list_front()
	       ---
	       returns the data of the first node.

*/

void* commc_list_front(const commc_list_t* list) {

  if  (!list || !list->head) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  return list->head->data;

}

/*

         commc_list_back()
	       ---
	       returns the data of the last node.

*/

void* commc_list_back(const commc_list_t* list) {

  if  (!list || !list->tail) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  return list->tail->data;

}

/*

         commc_list_size()
	       ---
	       returns the number of nodes in the list.

*/

size_t commc_list_size(const commc_list_t* list) {

  return list ? list->size : 0;

}

/*

         commc_list_is_empty()
	       ---
	       checks if the list is empty.

*/

int commc_list_is_empty(const commc_list_t* list) {

  return list ? (list->size == 0) : 1;

}

/*

         commc_list_clear()
	       ---
	       removes all elements from the list without
	       deallocating the list structure itself. the
	       list can be reused after clearing.

*/

void commc_list_clear(commc_list_t* list) {

  commc_list_node_t* current;

  if  (!list) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return;

  }

  current = list->head;

  while  (current) {

    commc_list_node_t* next = current->next;
    free(current);
    current = next;

  }

  list->head = NULL;
  list->tail = NULL;
  list->size = 0;

}

/*
	==================================
             --- ITERATORS ---
	==================================
*/

/*

         commc_list_begin()
	       ---
	       initializes iterator to the first element of the list.
	       provides educational example of iterator pattern
	       implementation using C89-compliant structures.

*/

commc_list_iterator_t commc_list_begin(const commc_list_t* list) {

  commc_list_iterator_t iterator;

  if  (!list) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    iterator.current = NULL;
    iterator.list = NULL;
    return iterator;

  }

  iterator.current = list->head;
  iterator.list = list;

  return iterator;

}

/*

         commc_list_next()
	       ---
	       advances iterator to the next element in the list.
	       demonstrates pointer-based traversal patterns
	       commonly used in systems programming.

*/

int commc_list_next(commc_list_iterator_t* iterator) {

  if  (!iterator || !iterator->current) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return 0;

  }

  iterator->current = iterator->current->next;
  return iterator->current != NULL;

}

/*

         commc_list_iterator_data()
	       ---
	       retrieves data from the current iterator position.
	       safe accessor that handles invalid states gracefully
	       while teaching proper error checking patterns.

*/

void* commc_list_iterator_data(commc_list_iterator_t* iterator) {

  if  (!iterator || !iterator->current) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  return iterator->current->data;

}

/*
	==================================
             --- SEARCH ---
	==================================
*/

/*

         commc_list_find()
	       ---
	       demonstrates linear search algorithm implementation
	       with function pointer pattern for flexible comparison.

*/

void* commc_list_find(const commc_list_t* list, const void* data, commc_compare_func compare) {

  commc_list_node_t* current;

  if  (!list || !data || !compare) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  current = list->head;

  while  (current) {

    if  (compare(current->data, data) == 0) {

      return current->data;

    }

    current = current->next;

  }

  return NULL;

}

/*

         commc_list_find_index()
	       ---
	       returns the position index of the first matching element.
	       useful for positional operations after successful search.

*/

int commc_list_find_index(const commc_list_t* list, const void* data, commc_compare_func compare) {

  commc_list_node_t* current;
  int                index;

  if  (!list || !data || !compare) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return -1;

  }

  current = list->head;
  index = 0;

  while  (current) {

    if  (compare(current->data, data) == 0) {

      return index;

    }

    current = current->next;
    index++;

  }

  return -1;

}

/*
	==================================
             --- EOF ---
	==================================
*/
