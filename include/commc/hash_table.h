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

#ifndef COMMC_HASH_TABLE_H
#define COMMC_HASH_TABLE_H

#include <stddef.h> /* for size_t */

/*
	==================================
             --- STRUCTS ---
	==================================
*/

/* opaque hash table structure. */

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

*/

int commc_hash_table_insert(commc_hash_table_t* table, const char* key, void* value);

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

#endif /* COMMC_HASH_TABLE_H */

/*
	==================================
             --- EOF ---
	==================================
*/
