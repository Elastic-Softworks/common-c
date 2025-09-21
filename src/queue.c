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
#include "commc/error.h"

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

  if  (!queue) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return;

  }

  commc_list_destroy(queue);

}

/*

         commc_queue_enqueue()
	       ---
	       adds an element to the back of the queue.

*/

void commc_queue_enqueue(commc_queue_t* queue, void* data) {

  if  (!queue) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return;

  }

  commc_list_push_back(queue, data);

}

/*

         commc_queue_dequeue()
	       ---
	       removes and returns the front element.

*/

void* commc_queue_dequeue(commc_queue_t* queue) {

  void* data;

  if  (!queue) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  data = commc_list_front(queue);
  commc_list_pop_front(queue);
  return data;

}

/*

         commc_queue_front()
	       ---
	       peeks at the front element.

*/

void* commc_queue_front(commc_queue_t* queue) {

  if  (!queue) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

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
             --- ITERATORS ---
	==================================
*/

/*

         commc_queue_begin()
	       ---
	       delegates to the underlying list iterator for
	       consistent traversal behavior across data structures.

*/

commc_queue_iterator_t commc_queue_begin(const commc_queue_t* queue) {

  return commc_list_begin(queue);

}

/*

         commc_queue_next()
	       ---
	       advances queue iterator using list iterator logic.

*/

int commc_queue_next(commc_queue_iterator_t* iterator) {

  return commc_list_next(iterator);

}

/*

         commc_queue_iterator_data()
	       ---
	       retrieves current iterator data via list implementation.

*/

void* commc_queue_iterator_data(commc_queue_iterator_t* iterator) {

  return commc_list_iterator_data(iterator);

}

/*
	==================================
             --- EOF ---
	==================================
*/
