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

  commc_list_destroy(stack);

}

/*

         commc_stack_push()
	       ---
	       adds an element to the top of the stack.

*/

void commc_stack_push(commc_stack_t* stack, void* data) {

  commc_list_push_back(stack, data);

}

/*

         commc_stack_pop()
	       ---
	       removes and returns the top element.

*/

void* commc_stack_pop(commc_stack_t* stack) {

  void* data = commc_list_back(stack);
  commc_list_pop_back(stack);
  return data;

}

/*

         commc_stack_peek()
	       ---
	       peeks at the top element.

*/

void* commc_stack_peek(commc_stack_t* stack) {

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
             --- EOF ---
	==================================
*/
