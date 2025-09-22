/* 	
   ===================================
   C O M M C / B L O O M _ F I L T E R . C
   PROBABILISTIC MEMBERSHIP TESTING IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

	                --- BLOOM FILTER IMPLEMENTATION ---

	    this implementation provides a complete probabilistic membership
	    testing system using bit arrays and multiple hash functions.
	    bloom filters excel at answering "definitely not in set" vs
	    "might be in set" queries with tunable false positive rates.
	    
	    the implementation uses mathematical optimization for bit array
	    sizing and hash function selection, ensuring optimal space
	    efficiency and performance characteristics.

*/

/* 
	==================================
             --- SETUP ---
	==================================
*/

#include "commc/bloom_filter.h"  /* BLOOM FILTER API */
#include "commc/error.h"         /* ERROR HANDLING */
#include <math.h>                /* MATHEMATICAL FUNCTIONS */
#include <stdlib.h>              /* STANDARD LIBRARY FUNCTIONS */
#include <string.h>              /* MEMORY OPERATIONS */

/* 
	==================================
             --- CONSTANTS ---
	==================================
*/

#define COMMC_BLOOM_BYTE_BITS 8  /* bits per byte for indexing */

/* 
	==================================
             --- HELPERS ---
	==================================
*/

/*

         get_bit()
	       ---
	       retrieves the value of a specific bit in the bit array.

*/

static int get_bit(const unsigned char* bit_array, size_t bit_index) {

  size_t byte_index = bit_index / COMMC_BLOOM_BYTE_BITS;
  size_t bit_offset = bit_index % COMMC_BLOOM_BYTE_BITS;
  
  return (bit_array[byte_index] >> bit_offset) & 1;

}

/*

         set_bit()
	       ---
	       sets a specific bit in the bit array to 1.

*/

static void set_bit(unsigned char* bit_array, size_t bit_index) {

  size_t byte_index = bit_index / COMMC_BLOOM_BYTE_BITS;
  size_t bit_offset = bit_index % COMMC_BLOOM_BYTE_BITS;
  
  bit_array[byte_index] |= (1 << bit_offset);

}

/*

         calculate_byte_count()
	       ---
	       calculates the number of bytes needed to store the bit array.

*/

static size_t calculate_byte_count(size_t bit_count) {

  return (bit_count + COMMC_BLOOM_BYTE_BITS - 1) / COMMC_BLOOM_BYTE_BITS;

}

/*

         generate_hash_values()
	       ---
	       generates multiple hash values for an element using different
	       hash functions. uses double hashing technique for efficiency.

*/

static void generate_hash_values(const void* data, size_t length, 
                                 size_t hash_count, size_t bit_count,
                                 size_t* hash_values) {

  size_t hash1, hash2;
  size_t i;

  /* use two base hash functions for double hashing */
  hash1 = commc_bloom_filter_hash_djb2(data, length, 0) % bit_count;
  hash2 = commc_bloom_filter_hash_fnv1a(data, length, 1) % bit_count;

  /* make hash2 odd to ensure it's coprime with bit_count powers of 2 */
  if  (hash2 % 2 == 0) {
    hash2 = (hash2 + 1) % bit_count;
  }

  /* generate hash_count different hash values */
  for  (i = 0; i < hash_count; i++) {
    hash_values[i] = (hash1 + i * hash2) % bit_count;
  }

}

/*
	==================================
             --- CORE API ---
	==================================
*/

/*

         commc_bloom_filter_create()
	       ---
	       creates a bloom filter optimized for expected elements and
	       false positive rate using mathematical optimization formulas.

*/

commc_bloom_filter_t* commc_bloom_filter_create(size_t expected_elements,
                                                 double false_positive_rate) {

  size_t                bit_count;
  size_t                hash_count;

  if  (expected_elements == 0 || false_positive_rate <= 0.0 || false_positive_rate >= 1.0) {
    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;
  }

  /* calculate optimal parameters */
  bit_count = commc_bloom_filter_optimal_bit_count(expected_elements, false_positive_rate);
  hash_count = commc_bloom_filter_optimal_hash_count(bit_count, expected_elements);

  /* enforce limits */
  if  (hash_count < COMMC_BLOOM_MIN_HASH_FUNCTIONS) {
    hash_count = COMMC_BLOOM_MIN_HASH_FUNCTIONS;
  }
  
  if  (hash_count > COMMC_BLOOM_MAX_HASH_FUNCTIONS) {
    hash_count = COMMC_BLOOM_MAX_HASH_FUNCTIONS;
  }

  return commc_bloom_filter_create_with_parameters(bit_count, hash_count);

}

/*

         commc_bloom_filter_create_with_parameters()
	       ---
	       creates bloom filter with explicit parameters for advanced control.

*/

commc_bloom_filter_t* commc_bloom_filter_create_with_parameters(size_t bit_count,
                                                                 size_t hash_count) {

  commc_bloom_filter_t* filter;
  size_t                byte_count;

  if  (bit_count == 0 || hash_count == 0) {
    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return NULL;
  }

  filter = (commc_bloom_filter_t*)malloc(sizeof(commc_bloom_filter_t));
  
  if  (!filter) {
    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;
  }

  byte_count = calculate_byte_count(bit_count);
  filter->bit_array = (unsigned char*)calloc(byte_count, 1);
  
  if  (!filter->bit_array) {
    free(filter);
    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return NULL;
  }

  filter->bit_count = bit_count;
  filter->hash_count = hash_count;
  filter->inserted_count = 0;

  return filter;

}

/*

         commc_bloom_filter_destroy()
	       ---
	       destroys bloom filter and frees all memory.

*/

void commc_bloom_filter_destroy(commc_bloom_filter_t* filter) {

  if  (!filter) {
    return;
  }

  if  (filter->bit_array) {
    free(filter->bit_array);
  }

  free(filter);

}

/*

         commc_bloom_filter_insert()
	       ---
	       adds an element to the bloom filter by setting appropriate bits.

*/

commc_error_t commc_bloom_filter_insert(commc_bloom_filter_t* filter,
                                         const void* data,
                                         size_t length) {

  size_t* hash_values;
  size_t  i;

  if  (!filter || !data) {
    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;
  }

  hash_values = (size_t*)malloc(sizeof(size_t) * filter->hash_count);
  
  if  (!hash_values) {
    commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);
    return COMMC_MEMORY_ERROR;
  }

  /* generate hash values */
  generate_hash_values(data, length, filter->hash_count, filter->bit_count, hash_values);

  /* set corresponding bits */
  for  (i = 0; i < filter->hash_count; i++) {
    set_bit(filter->bit_array, hash_values[i]);
  }

  filter->inserted_count++;
  free(hash_values);

  return COMMC_SUCCESS;

}

/*

         commc_bloom_filter_contains()
	       ---
	       tests whether an element might be in the bloom filter.

*/

int commc_bloom_filter_contains(commc_bloom_filter_t* filter,
                                const void* data,
                                size_t length) {

  size_t* hash_values;
  size_t  i;
  int     result = 1;

  if  (!filter || !data) {
    return 0;
  }

  hash_values = (size_t*)malloc(sizeof(size_t) * filter->hash_count);
  
  if  (!hash_values) {
    return 0;
  }

  /* generate hash values */
  generate_hash_values(data, length, filter->hash_count, filter->bit_count, hash_values);

  /* check if all corresponding bits are set */
  for  (i = 0; i < filter->hash_count; i++) {

    if  (!get_bit(filter->bit_array, hash_values[i])) {
      result = 0;
      break;
    }

  }

  free(hash_values);
  return result;

}

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

size_t commc_bloom_filter_size(commc_bloom_filter_t* filter) {

  if  (!filter) {
    return 0;
  }

  return filter->bit_count;

}

/*

         commc_bloom_filter_hash_count()
	       ---
	       returns the number of hash functions being used.

*/

size_t commc_bloom_filter_hash_count(commc_bloom_filter_t* filter) {

  if  (!filter) {
    return 0;
  }

  return filter->hash_count;

}

/*

         commc_bloom_filter_inserted_count()
	       ---
	       returns the number of elements inserted.

*/

size_t commc_bloom_filter_inserted_count(commc_bloom_filter_t* filter) {

  if  (!filter) {
    return 0;
  }

  return filter->inserted_count;

}

/*

         commc_bloom_filter_false_positive_rate()
	       ---
	       calculates current false positive rate based on insertions.

*/

double commc_bloom_filter_false_positive_rate(commc_bloom_filter_t* filter) {

  double ratio;

  if  (!filter || filter->inserted_count == 0) {
    return 0.0;
  }

  /* formula: (1 - e^(-k*n/m))^k 
     where k=hash_count, n=inserted_count, m=bit_count */
  ratio = -(double)filter->hash_count * filter->inserted_count / filter->bit_count;

  return pow(1.0 - exp(ratio), (double)filter->hash_count);

}

/*

         commc_bloom_filter_memory_usage()
	       ---
	       returns total memory usage in bytes.

*/

size_t commc_bloom_filter_memory_usage(commc_bloom_filter_t* filter) {

  size_t total_bytes;

  if  (!filter) {
    return 0;
  }

  total_bytes = sizeof(commc_bloom_filter_t);
  total_bytes += calculate_byte_count(filter->bit_count);

  return total_bytes;

}

/*
	==================================
             --- UTILITY API ---
	==================================
*/

/*

         commc_bloom_filter_clear()
	       ---
	       resets the bloom filter by clearing all bits.

*/

void commc_bloom_filter_clear(commc_bloom_filter_t* filter) {

  size_t byte_count;

  if  (!filter) {
    return;
  }

  byte_count = calculate_byte_count(filter->bit_count);
  memset(filter->bit_array, 0, byte_count);
  filter->inserted_count = 0;

}

/*

         commc_bloom_filter_union()
	       ---
	       combines two bloom filters using bitwise OR.

*/

commc_error_t commc_bloom_filter_union(commc_bloom_filter_t* dest,
                                       commc_bloom_filter_t* src) {

  size_t byte_count;
  size_t i;

  if  (!dest || !src) {
    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;
  }

  if  (dest->bit_count != src->bit_count || dest->hash_count != src->hash_count) {
    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;
  }

  byte_count = calculate_byte_count(dest->bit_count);

  for  (i = 0; i < byte_count; i++) {
    dest->bit_array[i] |= src->bit_array[i];
  }

  /* estimate combined insertion count (conservative) */
  dest->inserted_count += src->inserted_count;

  return COMMC_SUCCESS;

}

/*

         commc_bloom_filter_intersection()
	       ---
	       combines two bloom filters using bitwise AND.

*/

commc_error_t commc_bloom_filter_intersection(commc_bloom_filter_t* dest,
                                              commc_bloom_filter_t* src) {

  size_t byte_count;
  size_t i;

  if  (!dest || !src) {
    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;
  }

  if  (dest->bit_count != src->bit_count || dest->hash_count != src->hash_count) {
    commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);
    return COMMC_ARGUMENT_ERROR;
  }

  byte_count = calculate_byte_count(dest->bit_count);

  for  (i = 0; i < byte_count; i++) {
    dest->bit_array[i] &= src->bit_array[i];
  }

  /* intersection reduces effective insertion count */
  if  (dest->inserted_count > src->inserted_count) {
    dest->inserted_count = src->inserted_count;
  }

  return COMMC_SUCCESS;

}

/*
	==================================
             --- CALC API ---
	==================================
*/

/*

         commc_bloom_filter_optimal_bit_count()
	       ---
	       calculates optimal bit array size using formula:
	       m = -n * ln(p) / (ln(2)^2)

*/

size_t commc_bloom_filter_optimal_bit_count(size_t expected_elements,
                                             double false_positive_rate) {

  double result;

  if  (expected_elements == 0 || false_positive_rate <= 0.0 || false_positive_rate >= 1.0) {
    return 0;
  }

  result = -(double)expected_elements * log(false_positive_rate) / (log(2.0) * log(2.0));

  /* ensure minimum size */
  if  (result < 8.0) {
    result = 8.0;
  }

  return (size_t)result;

}

/*

         commc_bloom_filter_optimal_hash_count()
	       ---
	       calculates optimal number of hash functions using formula:
	       k = (m/n) * ln(2)

*/

size_t commc_bloom_filter_optimal_hash_count(size_t bit_count,
                                              size_t expected_elements) {

  double result;

  if  (bit_count == 0 || expected_elements == 0) {
    return COMMC_BLOOM_MIN_HASH_FUNCTIONS;
  }

  result = ((double)bit_count / expected_elements) * log(2.0);

  /* enforce limits */
  if  (result < COMMC_BLOOM_MIN_HASH_FUNCTIONS) {
    result = COMMC_BLOOM_MIN_HASH_FUNCTIONS;
  }
  
  if  (result > COMMC_BLOOM_MAX_HASH_FUNCTIONS) {
    result = COMMC_BLOOM_MAX_HASH_FUNCTIONS;
  }

  return (size_t)result;

}

/*
	==================================
             --- HASH API ---
	==================================
*/

/*

         commc_bloom_filter_hash_djb2()
	       ---
	       djb2 hash function optimized for bloom filter use.

*/

size_t commc_bloom_filter_hash_djb2(const void* data, size_t length, size_t seed) {

  const unsigned char* bytes = (const unsigned char*)data;
  size_t               hash = 5381 + seed;
  size_t               i;

  for  (i = 0; i < length; i++) {
    hash = ((hash << 5) + hash) + bytes[i];
  }

  return hash;

}

/*

         commc_bloom_filter_hash_fnv1a()
	       ---
	       fnv-1a hash function for secondary hashing.

*/

size_t commc_bloom_filter_hash_fnv1a(const void* data, size_t length, size_t seed) {

  const unsigned char* bytes = (const unsigned char*)data;
  size_t               hash = 2166136261UL + seed;
  size_t               i;

  for  (i = 0; i < length; i++) {
    hash ^= bytes[i];
    hash *= 16777619UL;
  }

  return hash;

}

/*

         commc_bloom_filter_hash_murmur()
	       ---
	       simplified murmur hash for general-purpose filtering.

*/

size_t commc_bloom_filter_hash_murmur(const void* data, size_t length, size_t seed) {

  const unsigned char* bytes = (const unsigned char*)data;
  size_t               hash = seed;
  size_t               i;

  for  (i = 0; i < length; i++) {

    hash ^= bytes[i];
    hash *= 0x5bd1e995;
    hash ^= hash >> 15;

  }

  return hash;

}

/*
	==================================
             --- EOF ---
	==================================
*/