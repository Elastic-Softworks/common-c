/*
   ===================================
   C O M M O N - C
   VECTOR IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- VECTOR MODULE ---

    implementation of the vector data structure.
    see include/commc/vector.h for function
    prototypes and documentation.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#include <stdlib.h>
#include <string.h>

#include "commc/vector.h"

/*
	==================================
             --- STRUCTS ---
	==================================
*/

/* internal vector structure. */

struct commc_vector_t {

  unsigned char* data;          /* RAW DATA BUFFER */
  size_t         size;          /* NUMBER OF ELEMENTS */
  size_t         capacity;      /* ALLOCATED CAPACITY */
  size_t         element_size;  /* SIZE OF EACH ELEMENT */

};

/*
	==================================
             --- FUNCS ---
	==================================
*/

/*

         commc_vector_create()
	       ---
	       allocates and initializes a new vector.

*/

commc_vector_t* commc_vector_create(size_t initial_capacity, size_t element_size) {

  commc_vector_t* vector;
  
  vector = (commc_vector_t*)malloc(sizeof(commc_vector_t));

  if  (!vector) {

    return NULL;

  }

  vector->data         = (unsigned char*)malloc(initial_capacity * element_size);
  vector->size         = 0;
  vector->capacity     = initial_capacity;
  vector->element_size = element_size;

  if  (!vector->data) {

    free(vector);
    return NULL;

  }

  return vector;

}

/*

         commc_vector_destroy()
	       ---
	       frees the vector and its data buffer.

*/

void commc_vector_destroy(commc_vector_t* vector) {

  if  (vector) {

    free(vector->data);
    free(vector);

  }

}

/*

         commc_vector_reserve()
	       ---
	       resizes the vector's internal buffer to at
	       least the new capacity.

*/

int commc_vector_reserve(commc_vector_t* vector, size_t new_capacity) {

  unsigned char* new_data;

  if  (!vector || new_capacity <= vector->capacity) {

    return 1; /* success (no-op) */

  }

  new_data = (unsigned char*)realloc(vector->data, new_capacity * vector->element_size);

  if  (!new_data) {

    return 0; /* failure */

  }

  vector->data     = new_data;
  vector->capacity = new_capacity;

  return 1;

}

/*

         commc_vector_push_back()
	       ---
	       adds an element to the end. if the vector is
	       full, it doubles its capacity.

*/

int commc_vector_push_back(commc_vector_t* vector, const void* element) {

  if  (!vector) {

    return 0;

  }

  if  (vector->size >= vector->capacity) {

    size_t new_capacity = vector->capacity > 0 ? vector->capacity * 2 : 1;

    if  (!commc_vector_reserve(vector, new_capacity)) {

      return 0;

    }

  }

  memcpy(vector->data + (vector->size * vector->element_size), element, vector->element_size);
  vector->size++;

  return 1;

}

/*

         commc_vector_pop_back()
	       ---
	       removes the last element by decrementing the size.

*/

void commc_vector_pop_back(commc_vector_t* vector) {

  if  (vector && vector->size > 0) {

    vector->size--;

  }

}

/*

         commc_vector_get()
	       ---
	       returns a pointer to the element at the given index.

*/

void* commc_vector_get(commc_vector_t* vector, size_t index) {

  if  (!vector || index >= vector->size) {

    return NULL;

  }

  return vector->data + (index * vector->element_size);

}

/*

         commc_vector_set()
	       ---
	       overwrites the element at the given index.

*/

int commc_vector_set(commc_vector_t* vector, size_t index, const void* element) {

  if  (!vector || index >= vector->size) {

    return 0;

  }

  memcpy(vector->data + (index * vector->element_size), element, vector->element_size);

  return 1;

}

/*

         commc_vector_size()
	       ---
	       returns the current number of elements.

*/

size_t commc_vector_size(commc_vector_t* vector) {

  return vector ? vector->size : 0;

}

/*

         commc_vector_capacity()
	       ---
	       returns the allocated capacity.

*/

size_t commc_vector_capacity(commc_vector_t* vector) {

  return vector ? vector->capacity : 0;

}

/*

         commc_vector_is_empty()
	       ---
	       checks if the vector is empty.

*/

int commc_vector_is_empty(commc_vector_t* vector) {

  return vector ? (vector->size == 0) : 1;

}

/*

         commc_vector_clear()
	       ---
	       resets the size to 0, clearing the vector.

*/

void commc_vector_clear(commc_vector_t* vector) {

  if  (vector) {

    vector->size = 0;

  }

}

/*

         commc_vector_insert()
	       ---
	       inserts an element at a specific index, shifting
	       other elements to make space.

*/

int commc_vector_insert(commc_vector_t* vector, size_t index, const void* element) {

  if  (!vector || index > vector->size) {

    return 0;

  }

  if  (vector->size >= vector->capacity) {

    size_t new_capacity = vector->capacity > 0 ? vector->capacity * 2 : 1;

    if  (!commc_vector_reserve(vector, new_capacity)) {

      return 0;

    }

  }

  /* shift elements */

  memmove(vector->data + ((index + 1) * vector->element_size),
          vector->data + (index * vector->element_size),
          (vector->size - index) * vector->element_size);

  /* insert new element */
  
  memcpy(vector->data + (index * vector->element_size), element, vector->element_size);
  vector->size++;

  return 1;

}

/*

         commc_vector_erase()
	       ---
	       removes an element at a specific index, shifting
	       elements to fill the gap.

*/

void commc_vector_erase(commc_vector_t* vector, size_t index) {

  if  (!vector || index >= vector->size) {

    return;

  }

  /* shift elements */
  memmove(vector->data + (index * vector->element_size),
          vector->data + ((index + 1) * vector->element_size),
          (vector->size - index - 1) * vector->element_size);

  vector->size--;

}

/*

         commc_vector_front()
	       ---
	       returns the first element.

*/

void* commc_vector_front(commc_vector_t* vector) {

  return commc_vector_get(vector, 0);

}

/*

         commc_vector_back()
	       ---
	       returns the last element.

*/

void* commc_vector_back(commc_vector_t* vector) {

  if  (!vector || vector->size == 0) {

    return NULL;

  }

  return commc_vector_get(vector, vector->size - 1);

}

/*
	==================================
             --- EOF ---
	==================================
*/
