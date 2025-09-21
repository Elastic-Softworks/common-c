/*
   ===================================
   C O M M O N - C
   STACK IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- STACK MODULE ---

    implementation of the stack data structure.
    this is a thin wrapper around the commc_list_t.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#include "commc/stack.h"
#include "commc/error.h"

/*
	==================================
             --- FUNCS ---
	==================================
*/

/*

         commc_stack_create()
	       ---
	       creates a new stack by creating a list.

*/

commc_stack_t* commc_stack_create(void) {

  return commc_list_create();

}

/*

         commc_stack_destroy()
	       ---
	       destroys the stack by destroying the list.

*/

void commc_stack_destroy(commc_stack_t* stack) {

  if  (!stack) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return;

  }

  commc_list_destroy(stack);

}

/*

         commc_stack_push()
	       ---
	       adds an element to the top of the stack.

*/

void commc_stack_push(commc_stack_t* stack, void* data) {

  if  (!stack) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return;

  }

  commc_list_push_back(stack, data);

}

/*

         commc_stack_pop()
	       ---
	       removes and returns the top element.

*/

void* commc_stack_pop(commc_stack_t* stack) {

  void* data;

  if  (!stack) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  data = commc_list_back(stack);
  commc_list_pop_back(stack);
  return data;

}

/*

         commc_stack_peek()
	       ---
	       peeks at the top element.

*/

void* commc_stack_peek(commc_stack_t* stack) {

  if  (!stack) {

    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;

  }

  return commc_list_back(stack);

}

/*

         commc_stack_size()
	       ---
	       returns the number of elements.

*/

size_t commc_stack_size(commc_stack_t* stack) {

  return commc_list_size(stack);

}

/*

         commc_stack_is_empty()
	       ---
	       checks if the stack is empty.

*/

int commc_stack_is_empty(commc_stack_t* stack) {

  return commc_list_is_empty(stack);

}

/*
	==================================
             --- ITERATORS ---
	==================================
*/

/*

         commc_stack_begin()
	       ---
	       delegates to list iterator, providing access to
	       stack elements from bottom (first pushed) to top.

*/

commc_stack_iterator_t commc_stack_begin(const commc_stack_t* stack) {

  return commc_list_begin(stack);

}

/*

         commc_stack_next()
	       ---
	       advances iterator using list traversal logic.

*/

int commc_stack_next(commc_stack_iterator_t* iterator) {

  return commc_list_next(iterator);

}

/*

         commc_stack_iterator_data()
	       ---
	       retrieves current iterator data via list implementation.

*/

void* commc_stack_iterator_data(commc_stack_iterator_t* iterator) {

  return commc_list_iterator_data(iterator);

}

/*
	==================================
             --- EOF ---
	==================================
*/
