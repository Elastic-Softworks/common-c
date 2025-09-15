/*
   ===================================
   C O M M O N - C
   QUEUE IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- QUEUE MODULE ---

    implementation of the queue data structure.
    this is a thin wrapper around the commc_list_t.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#include "commc/queue.h"

/*
	==================================
             --- FUNCS ---
	==================================
*/

/*

         commc_queue_create()
	       ---
	       creates a new queue by creating a list.

*/

commc_queue_t* commc_queue_create(void) {

  return commc_list_create();

}

/*

         commc_queue_destroy()
	       ---
	       destroys the queue by destroying the list.

*/

void commc_queue_destroy(commc_queue_t* queue) {

  commc_list_destroy(queue);

}

/*

         commc_queue_enqueue()
	       ---
	       adds an element to the back of the queue.

*/

void commc_queue_enqueue(commc_queue_t* queue, void* data) {

  commc_list_push_back(queue, data);

}

/*

         commc_queue_dequeue()
	       ---
	       removes and returns the front element.

*/

void* commc_queue_dequeue(commc_queue_t* queue) {

  void* data = commc_list_front(queue);
  commc_list_pop_front(queue);
  return data;

}

/*

         commc_queue_front()
	       ---
	       peeks at the front element.

*/

void* commc_queue_front(commc_queue_t* queue) {

  return commc_list_front(queue);

}

/*

         commc_queue_back()
	       ---
	       peeks at the back element.

*/

void* commc_queue_back(commc_queue_t* queue) {

  return commc_list_back(queue);

}

/*

         commc_queue_size()
	       ---
	       returns the number of elements.

*/

size_t commc_queue_size(commc_queue_t* queue) {

  return commc_list_size(queue);

}

/*

         commc_queue_is_empty()
	       ---
	       checks if the queue is empty.

*/

int commc_queue_is_empty(commc_queue_t* queue) {

  return commc_list_is_empty(queue);

}

/*
	==================================
             --- EOF ---
	==================================
*/
