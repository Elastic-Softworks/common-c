/*
   ===================================
   C O M M O N - C
   QUEUE MODULE
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- QUEUE MODULE ---

    this module provides a generic queue data structure,
    which follows the First-In, First-Out (FIFO) principle.

    it is implemented as a wrapper around the commc_list_t,
    providing a clear and simple interface for queue operations.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#ifndef COMMC_QUEUE_H
#define COMMC_QUEUE_H

#include <stddef.h>

#include "commc/list.h"

/*
	==================================
             --- STRUCTS ---
	==================================
*/

typedef commc_list_t commc_queue_t;

/*
	==================================
             --- FUNCTIONS ---
	==================================
*/

/*

         commc_queue_create()
	       ---
	       creates a new, empty queue.

*/

commc_queue_t* commc_queue_create(void);

/*

         commc_queue_destroy()
	       ---
	       frees all memory associated with the queue.

*/

void commc_queue_destroy(commc_queue_t* queue);

/*

         commc_queue_enqueue()
	       ---
	       adds an element to the back of the queue.

*/

void commc_queue_enqueue(commc_queue_t* queue, void* data);

/*

         commc_queue_dequeue()
	       ---
	       removes and returns the element from the front
	       of the queue.

*/

void* commc_queue_dequeue(commc_queue_t* queue);

/*

         commc_queue_front()
	       ---
	       returns the element at the front of the queue
	       without removing it.

*/

void* commc_queue_front(commc_queue_t* queue);

/*

         commc_queue_back()
	       ---
	       returns the element at the back of the queue
	       without removing it.

*/

void* commc_queue_back(commc_queue_t* queue);

/*

         commc_queue_size()
	       ---
	       returns the number of elements in the queue.

*/

size_t commc_queue_size(commc_queue_t* queue);

/*

         commc_queue_is_empty()
	       ---
	       returns 1 if the queue is empty, otherwise 0.

*/

int commc_queue_is_empty(commc_queue_t* queue);

#endif /* COMMC_QUEUE_H */

/*
	==================================
             --- EOF ---
	==================================
*/
