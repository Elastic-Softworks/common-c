/*
   ===================================
   C O M M O N - C
   HASH TABLE MODULE
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- HASH TABLE MODULE ---

    this module provides a generic hash table (hash map)
    implementation for storing key-value pairs. the keys
    are strings, and the values are void pointers.

    it uses the djb2 hashing algorithm and resolves
    collisions with separate chaining (using commc_list_t).

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#ifndef  COMMC_HASH_TABLE_H
#define  COMMC_HASH_TABLE_H

#include  <stddef.h> 			/* for size_t */
#include  "error.h"             /* for commc_error_t */

/*
	==================================
             --- STRUCTS ---
	==================================
*/

typedef struct commc_hash_table_t commc_hash_table_t;

/*
	==================================
             --- FUNCTIONS ---
	==================================
*/

/*

         commc_hash_table_create()
	       ---
	       creates a new hash table with a given capacity.
	       capacity should ideally be a prime number.

*/

commc_hash_table_t* commc_hash_table_create(size_t capacity);

/*

         commc_hash_table_destroy()
	       ---
	       frees all memory associated with the hash table.
	       does not free the data pointed to by the values.

*/

void commc_hash_table_destroy(commc_hash_table_t* table);

/*

         commc_hash_table_insert()
	       ---
	       inserts a key-value pair into the hash table.
	       if the key already exists, its value is updated.
	       returns COMMC_SUCCESS on success, appropriate error code on failure.

*/

commc_error_t commc_hash_table_insert(commc_hash_table_t* table, const char* key, void* value);

/*

         commc_hash_table_get()
	       ---
	       retrieves the value associated with a given key.
	       returns null if the key is not found.

*/

void* commc_hash_table_get(commc_hash_table_t* table, const char* key);

/*

         commc_hash_table_remove()
	       ---
	       removes a key-value pair from the hash table.

*/

void commc_hash_table_remove(commc_hash_table_t* table, const char* key);

/*

         commc_hash_table_size()
	       ---
	       returns the number of elements in the hash table.

*/

size_t commc_hash_table_size(commc_hash_table_t* table);

/*

         commc_hash_table_capacity()
	       ---
	       returns the number of buckets in the hash table.
	       this represents the internal capacity for distribution
	       of elements across the hash table structure.

*/

size_t commc_hash_table_capacity(commc_hash_table_t* table);

/*

         commc_hash_table_clear()
	       ---
	       removes all elements from the hash table.
	       the table remains valid and can be reused.
	       does not deallocate the bucket array.

*/

void commc_hash_table_clear(commc_hash_table_t* table);

/*

         commc_hash_table_rehash()
	       ---
	       resizes the hash table to improve load factor distribution.
	       rehashes all existing elements into the new bucket structure.
	       new_capacity should ideally be a prime number.

*/

commc_error_t commc_hash_table_rehash(commc_hash_table_t* table, size_t new_capacity);

/*

         commc_hash_table_load_factor()
	       ---
	       returns the current load factor (size / capacity) as a float.
	       useful for determining when to trigger automatic resizing.
	       typical load factors range from 0.5 to 0.75 for good performance.

*/

float commc_hash_table_load_factor(commc_hash_table_t* table);

/*

         commc_hash_table_set_hash_function()
	       ---
	       sets a custom hash function for the table.
	       the function should accept a string and return an unsigned long.
	       if set to NULL, reverts to the default djb2 hash function.

*/

typedef unsigned long (*commc_hash_function_t)(const char* key);

void commc_hash_table_set_hash_function(commc_hash_table_t* table, commc_hash_function_t hash_func);

/*

         commc_hash_table_auto_resize()
	       ---
	       enables or disables automatic resizing when load factor
	       exceeds 0.75. when enabled, the table will double in size
	       and rehash all elements when the threshold is reached.

*/

void commc_hash_table_set_auto_resize(commc_hash_table_t* table, int enable);

#endif /* COMMC_HASH_TABLE_H */

/*
	==================================
             --- EOF ---
	==================================
*/
