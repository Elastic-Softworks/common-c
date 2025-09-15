/*
   ===================================
   C O M M O N - C
   STACK MODULE
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- STACK MODULE ---

    this module provides a generic stack data structure,
    which follows the Last-In, First-Out (LIFO) principle.

    it is implemented as a wrapper around the commc_list_t,
    providing a clear and simple interface for stack operations.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#ifndef   COMMC_STACK_H
#define   COMMC_STACK_H

#include  <stddef.h>

#include  "commc/list.h"

/*
	==================================
             --- STRUCTS ---
	==================================
*/

typedef  commc_list_t commc_stack_t;

/*
	==================================
             --- FUNCTIONS ---
	==================================
*/

/*

         commc_stack_create()
	       ---
	       creates a new, empty stack.

*/

commc_stack_t* commc_stack_create(void);

/*

         commc_stack_destroy()
	       ---
	       frees all memory associated with the stack.

*/

void commc_stack_destroy(commc_stack_t* stack);

/*

         commc_stack_push()
	       ---
	       adds an element to the top of the stack.

*/

void commc_stack_push(commc_stack_t* stack, void* data);

/*

         commc_stack_pop()
	       ---
	       removes and returns the element from the top
	       of the stack.

*/

void* commc_stack_pop(commc_stack_t* stack);

/*

         commc_stack_peek()
	       ---
	       returns the element at the top of the stack
	       without removing it.

*/

void* commc_stack_peek(commc_stack_t* stack);

/*

         commc_stack_size()
	       ---
	       returns the number of elements in the stack.

*/

size_t commc_stack_size(commc_stack_t* stack);

/*

         commc_stack_is_empty()
	       ---
	       returns 1 if the stack is empty, otherwise 0.

*/

int commc_stack_is_empty(commc_stack_t* stack);

#endif /* COMMC_STACK_H */

/*
	==================================
             --- EOF ---
	==================================
*/
