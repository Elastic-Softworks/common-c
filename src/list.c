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

#include <stdlib.h>
#include "commc/list.h"

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

    return NULL;

  }

  new_node = (commc_list_node_t*)malloc(sizeof(commc_list_node_t));

  if  (!new_node) {

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

    return NULL;

  }

  new_node = (commc_list_node_t*)malloc(sizeof(commc_list_node_t));

  if  (!new_node) {

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

void* commc_list_front(commc_list_t* list) {

  if  (!list || !list->head) {

    return NULL;

  }

  return list->head->data;

}

/*

         commc_list_back()
	       ---
	       returns the data of the last node.

*/

void* commc_list_back(commc_list_t* list) {

  if  (!list || !list->tail) {

    return NULL;

  }

  return list->tail->data;

}

/*

         commc_list_size()
	       ---
	       returns the number of nodes in the list.

*/

size_t commc_list_size(commc_list_t* list) {

  return list ? list->size : 0;

}

/*

         commc_list_is_empty()
	       ---
	       checks if the list is empty.

*/

int commc_list_is_empty(commc_list_t* list) {

  return list ? (list->size == 0) : 1;

}

/*
	==================================
             --- EOF ---
	==================================
*/
