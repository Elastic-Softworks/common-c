/*
   ===================================
   C O M M O N - C
   VECTOR (DYNAMIC ARRAY) MODULE
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- VECTOR MODULE ---

    this module provides a generic dynamic array (vector)
    implementation. it can store elements of any type
    and automatically resizes itself as needed.

    it operates on void pointers, so the user is
    responsible for managing the type of data stored.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#ifndef   COMMC_VECTOR_H
#define   COMMC_VECTOR_H

#include  <stddef.h>       /* for size_t */

/*
	==================================
             --- STRUCTS ---
	==================================
*/

/* opaque vector structure. */

typedef struct commc_vector_t commc_vector_t;

/*
	==================================
             --- FUNCTIONS ---
	==================================
*/

/*

         commc_vector_create()
	       ---
	       creates a new vector with a given initial
	       capacity and element size.

*/

commc_vector_t* commc_vector_create(size_t initial_capacity, size_t element_size);

/*

         commc_vector_destroy()
	       ---
	       frees all memory associated with the vector.

*/

void commc_vector_destroy(commc_vector_t* vector);

/*

         commc_vector_push_back()
	       ---
	       adds an element to the end of the vector,
	       resizing if necessary.

*/

int commc_vector_push_back(commc_vector_t* vector, const void* element);

/*

         commc_vector_pop_back()
	       ---
	       removes the last element from the vector.

*/

void commc_vector_pop_back(commc_vector_t* vector);

/*

         commc_vector_get()
	       ---
	       retrieves a pointer to the element at a
	       specific index. returns null if out of bounds.

*/

void* commc_vector_get(commc_vector_t* vector, size_t index);

/*

         commc_vector_set()
	       ---
	       sets the element at a specific index.
	       the vector must already contain an element
	       at this index.

*/

int commc_vector_set(commc_vector_t* vector, size_t index, const void* element);

/*

         commc_vector_size()
	       ---
	       returns the number of elements in the vector.

*/

size_t commc_vector_size(commc_vector_t* vector);

/*

         commc_vector_capacity()
	       ---
	       returns the current capacity of the vector.

*/

size_t commc_vector_capacity(commc_vector_t* vector);

/*

         commc_vector_reserve()
	       ---
	       ensures the vector has at least a given capacity.

*/

int commc_vector_reserve(commc_vector_t* vector, size_t new_capacity);

/*

         commc_vector_is_empty()
	       ---
	       returns 1 if the vector is empty, otherwise 0.

*/

int commc_vector_is_empty(commc_vector_t* vector);

/*

         commc_vector_clear()
	       ---
	       removes all elements from the vector, but does
	       not free the allocated memory.

*/

void commc_vector_clear(commc_vector_t* vector);

/*

         commc_vector_insert()
	       ---
	       inserts an element at a specific index, shifting
	       all subsequent elements.

*/

int commc_vector_insert(commc_vector_t* vector, size_t index, const void* element);

/*

         commc_vector_erase()
	       ---
	       removes an element at a specific index, shifting
	       all subsequent elements.

*/

void commc_vector_erase(commc_vector_t* vector, size_t index);

/*

         commc_vector_front()
	       ---
	       returns a pointer to the first element.

*/

void* commc_vector_front(commc_vector_t* vector);

/*

         commc_vector_back()
	       ---
	       returns a pointer to the last element.

*/

void* commc_vector_back(commc_vector_t* vector);

#endif /* COMMC_VECTOR_H */

/*
	==================================
             --- EOF ---
	==================================
*/
