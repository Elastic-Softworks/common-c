/* 	
   ===================================
   C O M M C / B L O O M _ F I L T E R . H
   PROBABILISTIC MEMBERSHIP TESTING API
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

	                --- BLOOM FILTER API ---

	    bloom filters provide space-efficient probabilistic testing
	    for set membership. they can tell you definitively that
	    an element is NOT in a set, but may give false positives
	    for membership testing.
	    
	    this implementation uses multiple hash functions and allows
	    tuning of false positive rates through bit array sizing
	    and hash function count selection.

*/

#ifndef COMMC_BLOOM_FILTER_H
#define COMMC_BLOOM_FILTER_H

/* 
	==================================
             --- INCLUDES ---
	==================================
*/

#include    "commc/error.h"      /* ERROR HANDLING */
#include    <stddef.h>           /* SIZE_T */

#ifdef      __cplusplus

extern "C" {

#endif

/* 
	==================================
             --- TYPES ---
	==================================
*/

/*

         commc_bloom_filter_t
	     ---
	     represents a bloom filter with configurable parameters.
	     contains bit array, hash function count, and sizing info.

*/

typedef struct {

  unsigned char* bit_array;        /* bit storage for filter */
  size_t         bit_count;        /* total bits in filter */
  size_t         hash_count;       /* number of hash functions */
  size_t         inserted_count;   /* items inserted so far */
  
} commc_bloom_filter_t;

/*

         commc_bloom_hash_function_t
	     ---
	     function pointer type for hash functions used by bloom filter.

*/

typedef size_t (*commc_bloom_hash_function_t)(const void* data, size_t length, size_t seed);

/* 
	==================================
             --- CONSTANTS ---
	==================================
*/

/*

         COMMC_BLOOM_DEFAULT_FALSE_POSITIVE_RATE
	     ---
	     default acceptable false positive rate for bloom filters.
	     1% provides good balance of space efficiency and accuracy.

*/

#define COMMC_BLOOM_DEFAULT_FALSE_POSITIVE_RATE 0.01

/*

         COMMC_BLOOM_MIN_HASH_FUNCTIONS
	     ---
	     minimum number of hash functions for meaningful filtering.

*/

#define COMMC_BLOOM_MIN_HASH_FUNCTIONS 1

/*

         COMMC_BLOOM_MAX_HASH_FUNCTIONS  
	     ---
	     maximum number of hash functions to prevent performance issues.

*/

#define COMMC_BLOOM_MAX_HASH_FUNCTIONS 16

/* 
	==================================
             --- CORE API ---
	==================================
*/

/*

         commc_bloom_filter_create()
	     ---
	       creates a bloom filter optimized for expected number of elements
	       and desired false positive rate. automatically calculates
	       optimal bit array size and hash function count.
	       
	       parameters:
	       - expected_elements: anticipated number of items to insert
	       - false_positive_rate: desired probability of false positives
	       
	       returns:
	       - pointer to new bloom filter, or NULL on error

*/

commc_bloom_filter_t* commc_bloom_filter_create(size_t expected_elements,
                                                 double false_positive_rate);

/*

         commc_bloom_filter_create_with_parameters()
	       ---
	       creates bloom filter with explicit bit count and hash function count.
	       provides direct control over filter characteristics for advanced use.
	       
	       parameters:
	       - bit_count: size of bit array in bits
	       - hash_count: number of hash functions to use
	       
	       returns:
	       - pointer to new bloom filter, or NULL on error

*/

commc_bloom_filter_t* commc_bloom_filter_create_with_parameters(size_t bit_count,
                                                                 size_t hash_count);

/*

         commc_bloom_filter_destroy()
	       ---
	       destroys bloom filter and frees all associated memory.

*/

void commc_bloom_filter_destroy(commc_bloom_filter_t* filter);

/*

         commc_bloom_filter_insert()
	       ---
	       adds an element to the bloom filter by setting appropriate bits.
	       elements are represented as byte arrays with specified length.
	       
	       parameters:
	       - filter: bloom filter to modify
	       - data: pointer to element data
	       - length: size of element data in bytes
	       
	       returns:
	       - COMMC_SUCCESS on successful insertion
	       - COMMC_ARGUMENT_ERROR for invalid parameters

*/

commc_error_t commc_bloom_filter_insert(commc_bloom_filter_t* filter,
                                         const void* data,
                                         size_t length);

/*

         commc_bloom_filter_contains()
	       ---
	       tests whether an element might be in the bloom filter.
	       returns 1 if element might be present (can be false positive).
	       returns 0 if element is definitely not present (never false negative).
	       
	       parameters:
	       - filter: bloom filter to query  
	       - data: pointer to element data to test
	       - length: size of element data in bytes
	       
	       returns:
	       - 1 if element might be present
	       - 0 if element is definitely not present

*/

int commc_bloom_filter_contains(commc_bloom_filter_t* filter,
                                const void* data,
                                size_t length);

/* 
	==================================
             --- INFO API ---
	==================================
*/

/*

         commc_bloom_filter_size()
	       ---
	       returns the size of the bit array in bits.

*/

size_t commc_bloom_filter_size(commc_bloom_filter_t* filter);

/*

         commc_bloom_filter_hash_count()
	       ---
	       returns the number of hash functions being used.

*/

size_t commc_bloom_filter_hash_count(commc_bloom_filter_t* filter);

/*

         commc_bloom_filter_inserted_count()
	       ---
	       returns the number of elements that have been inserted.

*/

size_t commc_bloom_filter_inserted_count(commc_bloom_filter_t* filter);

/*

         commc_bloom_filter_false_positive_rate()
	       ---
	       calculates the current false positive rate based on insertions.
	       rate increases as more elements are added to the filter.

*/

double commc_bloom_filter_false_positive_rate(commc_bloom_filter_t* filter);

/*

         commc_bloom_filter_memory_usage()
	       ---
	       returns total memory usage of the bloom filter in bytes.

*/

size_t commc_bloom_filter_memory_usage(commc_bloom_filter_t* filter);

/* 
	==================================
             --- UTILITY API ---
	==================================
*/

/*

         commc_bloom_filter_clear()
	       ---
	       resets the bloom filter by clearing all bits.
	       maintains filter parameters but removes all inserted elements.

*/

void commc_bloom_filter_clear(commc_bloom_filter_t* filter);

/*

         commc_bloom_filter_union()
	       ---
	       combines two bloom filters using bitwise OR operation.
	       both filters must have identical parameters (bit count, hash count).
	       result contains elements that were in either filter.
	       
	       parameters:
	       - dest: destination filter (modified in place)
	       - src: source filter to combine with dest
	       
	       returns:
	       - COMMC_SUCCESS on successful union
	       - COMMC_ARGUMENT_ERROR if filters have mismatched parameters

*/

commc_error_t commc_bloom_filter_union(commc_bloom_filter_t* dest,
                                       commc_bloom_filter_t* src);

/*

         commc_bloom_filter_intersection() 
	       ---
	       combines two bloom filters using bitwise AND operation.
	       result might contain elements that were in both filters.
	       note: may produce false negatives due to AND operation.

*/

commc_error_t commc_bloom_filter_intersection(commc_bloom_filter_t* dest,
                                              commc_bloom_filter_t* src);

/* 
	==================================
             --- CALC API ---
	==================================
*/

/*

         commc_bloom_filter_optimal_bit_count()
	       ---
	       calculates optimal bit array size for given parameters.
	       uses formula: m = -n * ln(p) / (ln(2)^2)
	       where n=elements, p=false positive rate, m=bits.

*/

size_t commc_bloom_filter_optimal_bit_count(size_t expected_elements,
                                             double false_positive_rate);

/*

         commc_bloom_filter_optimal_hash_count()
	       ---
	       calculates optimal number of hash functions.
	       uses formula: k = (m/n) * ln(2)
	       where m=bits, n=elements, k=hash functions.

*/

size_t commc_bloom_filter_optimal_hash_count(size_t bit_count,
                                              size_t expected_elements);

/* 
	==================================
             --- HASH API ---
	==================================
*/

/*

         commc_bloom_filter_hash_djb2()
	       ---
	       djb2 hash function optimized for bloom filter use.
	       provides good distribution with low collision rates.

*/

size_t commc_bloom_filter_hash_djb2(const void* data, size_t length, size_t seed);

/*

         commc_bloom_filter_hash_fnv1a()
	       ---
	       fnv-1a hash function for bloom filter secondary hashing.
	       complements djb2 to provide independent hash values.

*/

size_t commc_bloom_filter_hash_fnv1a(const void* data, size_t length, size_t seed);

/*

         commc_bloom_filter_hash_murmur()
	       ---
	       simplified murmur hash for bloom filter use.
	       provides excellent distribution for general-purpose filtering.

*/

size_t commc_bloom_filter_hash_murmur(const void* data, size_t length, size_t seed);

#ifdef __cplusplus
}
#endif

#endif /* COMMC_BLOOM_FILTER_H */

/*
	==================================
             --- EOF ---
	==================================
*/