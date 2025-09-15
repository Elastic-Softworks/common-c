/*
   ===================================
   C O M M O N - C
   LINKED LIST MODULE
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- LIST MODULE ---

    this module provides a generic doubly linked list
    implementation. it is useful for ordered collections
    where insertions and deletions are frequent.

    it operates on void pointers, so the user is
    responsible for managing the data stored in each node.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#ifndef  COMMC_LIST_H
#define  COMMC_LIST_H

#include  <stddef.h>

/*
	==================================
             --- STRUCTS ---
	==================================
*/

/* internal node structure. */

typedef struct commc_list_node_t {

  void* data;                      /* user-provided data */

  struct commc_list_node_t* prev;  /* previous node */
  struct commc_list_node_t* next;  /* next node */

} commc_list_node_t;

/* internal list structure. */

typedef struct commc_list_t {

  commc_list_node_t* head; /* first node */
  commc_list_node_t* tail; /* last node */

  size_t size; /* number of nodes */

} commc_list_t;

/*
	==================================
             --- FUNCTIONS ---
	==================================
*/

/*

         commc_list_create()
	       ---
	       creates a new, empty linked list.

*/

commc_list_t* commc_list_create(void);

/*

         commc_list_destroy()
	       ---
	       frees all memory associated with the list.
	       note: does not free the data held by the nodes.

*/

void commc_list_destroy(commc_list_t* list);

/*

         commc_list_push_front()
	       ---
	       adds an element to the beginning of the list.

*/

commc_list_node_t* commc_list_push_front(commc_list_t* list, void* data);

/*

         commc_list_push_back()
	       ---
	       adds an element to the end of the list.

*/

commc_list_node_t* commc_list_push_back(commc_list_t* list, void* data);

/*

         commc_list_pop_front()
	       ---
	       removes the first element from the list.

*/

void commc_list_pop_front(commc_list_t* list);

/*

         commc_list_pop_back()
	       ---
	       removes the last element from the list.

*/

void commc_list_pop_back(commc_list_t* list);

/*

         commc_list_front()
	       ---
	       returns a pointer to the first node's data.

*/

void* commc_list_front(commc_list_t* list);

/*

         commc_list_back()
	       ---
	       returns a pointer to the last node's data.

*/

void* commc_list_back(commc_list_t* list);

/*

         commc_list_size()
	       ---
	       returns the number of elements in the list.

*/

size_t commc_list_size(commc_list_t* list);

/*

         commc_list_is_empty()
	       ---
	       returns 1 if the list is empty, otherwise 0.

*/

int commc_list_is_empty(commc_list_t* list);

#endif /* COMMC_LIST_H */

/*
	==================================
             --- EOF ---
	==================================
*/
