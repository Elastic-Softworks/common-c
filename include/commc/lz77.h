/*
   ===================================
   C O M M O N - C
   LZ77 COMPRESSION ALGORITHM
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- LZ77 MODULE ---

    implementation of the LZ77 lossless data compression
    algorithm with sliding window dictionary, efficient
    match finding, and high-performance encoding/decoding.

    this module provides the foundation for deflate-style
    compression used in gzip, zip, and PNG formats.

    LZ77 works by finding repeated sequences in the data
    stream and replacing them with references to previous
    occurrences within a sliding window buffer.

*/

#ifndef COMMC_LZ77_H
#define COMMC_LZ77_H

/*
	==================================
             --- INCLUDES ---
	==================================
*/

#include  <stddef.h>   /* for size_t */

/*
	==================================
             --- CONSTANTS ---
	==================================
*/

/* compression configuration limits */

#define COMMC_LZ77_MAX_WINDOW_SIZE      32768    /* 32KB sliding window */
#define COMMC_LZ77_MIN_WINDOW_SIZE      256      /* 256B minimum window */
#define COMMC_LZ77_DEFAULT_WINDOW_SIZE  4096     /* 4KB default window */

#define COMMC_LZ77_MAX_MATCH_LENGTH     258      /* maximum match length */
#define COMMC_LZ77_MIN_MATCH_LENGTH     3        /* minimum match length */

#define COMMC_LZ77_MAX_DISTANCE         32768    /* maximum back-reference distance */

/* encoding symbol types */

#define COMMC_LZ77_SYMBOL_LITERAL       0        /* literal byte value */
#define COMMC_LZ77_SYMBOL_MATCH         1        /* length/distance pair */

/* hash table configuration for fast string matching */

#define COMMC_LZ77_HASH_BITS            12       /* 12-bit hash = 4096 entries */
#define COMMC_LZ77_HASH_SIZE            (1 << COMMC_LZ77_HASH_BITS)
#define COMMC_LZ77_HASH_MASK            (COMMC_LZ77_HASH_SIZE - 1)

/* compression levels */

typedef enum {

  COMMC_LZ77_LEVEL_FAST    = 1,   /* fast compression, larger output */
  COMMC_LZ77_LEVEL_DEFAULT = 6,   /* balanced compression and speed */
  COMMC_LZ77_LEVEL_BEST    = 9    /* maximum compression, slower */

} commc_lz77_level_t;

/*
	==================================
             --- DATA TYPES ---
	==================================
*/

/*

         commc_lz77_match_t
	       ---
	       represents a back-reference match found in the
	       sliding window dictionary.

*/

typedef struct {

  unsigned int length;        /* length of the matched sequence */
  unsigned int distance;      /* distance back to the match */

} commc_lz77_match_t;

/*

         commc_lz77_symbol_t
	       ---
	       represents an output symbol which can be either
	       a literal byte or a length/distance pair.

*/

typedef struct {

  int type;                   /* COMMC_LZ77_SYMBOL_LITERAL or COMMC_LZ77_SYMBOL_MATCH */
  
  union {
  
    unsigned char literal;    /* literal byte value */
    commc_lz77_match_t match; /* length/distance pair */
    
  } data;

} commc_lz77_symbol_t;

/*

         commc_lz77_context_t
	       ---
	       compression/decompression context maintaining
	       sliding window state and hash tables.

*/

typedef struct {

  /* sliding window buffer */
  
  unsigned char* window;      /* circular buffer for sliding window */
  size_t window_size;         /* size of the sliding window */
  size_t window_pos;          /* current position in window */
  size_t window_filled;       /* amount of data in window */
  
  /* hash table for fast string matching */
  
  int* hash_table;            /* hash table for 3-byte strings */
  int* prev_table;            /* previous occurrence chain */
  
  /* compression parameters */
  
  commc_lz77_level_t level;   /* compression level */
  size_t max_chain_length;    /* maximum hash chain traversal */
  size_t good_match_length;   /* length threshold for early termination */
  size_t lazy_match_threshold; /* threshold for lazy matching */

} commc_lz77_context_t;

/*
	==================================
             --- PROTOTYPES ---
	==================================
*/

/*

         commc_lz77_context_create()
	       ---
	       creates and initializes a new LZ77 compression
	       context with the specified window size and
	       compression level.
	       
	       parameters:
	         window_size - size of sliding window buffer
	         level - compression level (affects performance)
	       
	       returns:
	         pointer to initialized context, or NULL on failure

*/

commc_lz77_context_t* commc_lz77_context_create(size_t window_size, 
                                                 commc_lz77_level_t level);

/*

         commc_lz77_context_destroy()
	       ---
	       releases all memory associated with an LZ77
	       compression context.
	       
	       parameters:
	         ctx - pointer to context to destroy

*/

void commc_lz77_context_destroy(commc_lz77_context_t* ctx);

/*

         commc_lz77_context_reset()
	       ---
	       resets the compression context to initial state
	       for processing a new data stream.
	       
	       parameters:
	         ctx - pointer to context to reset
	       
	       returns:
	         COMMC_SUCCESS or error code

*/

int commc_lz77_context_reset(commc_lz77_context_t* ctx);

/*

         commc_lz77_compress()
	       ---
	       compresses input data using LZ77 algorithm,
	       producing a stream of literal and match symbols.
	       
	       parameters:
	         ctx - compression context
	         input - input data buffer
	         input_size - size of input data
	         output - array to store output symbols
	         output_capacity - maximum number of symbols
	         symbols_written - pointer to store symbol count
	       
	       returns:
	         COMMC_SUCCESS or error code

*/

int commc_lz77_compress(commc_lz77_context_t* ctx,
                        const unsigned char* input,
                        size_t input_size,
                        commc_lz77_symbol_t* output,
                        size_t output_capacity,
                        size_t* symbols_written);

/*

         commc_lz77_decompress()
	       ---
	       decompresses a stream of LZ77 symbols back to
	       the original data using sliding window reconstruction.
	       
	       parameters:
	         ctx - decompression context
	         symbols - input symbol stream
	         symbol_count - number of symbols to process
	         output - output data buffer
	         output_capacity - maximum output size
	         bytes_written - pointer to store output size
	       
	       returns:
	         COMMC_SUCCESS or error code

*/

int commc_lz77_decompress(commc_lz77_context_t* ctx,
                          const commc_lz77_symbol_t* symbols,
                          size_t symbol_count,
                          unsigned char* output,
                          size_t output_capacity,
                          size_t* bytes_written);

/*

         commc_lz77_find_match()
	       ---
	       searches the sliding window for the longest
	       match starting at the current position.
	       
	       parameters:
	         ctx - compression context
	         current_pos - current position in input
	         input - input data buffer
	         input_end - end of input data
	         match - pointer to store match information
	       
	       returns:
	         length of match found (0 if no match)

*/

size_t commc_lz77_find_match(commc_lz77_context_t* ctx,
                             size_t current_pos,
                             const unsigned char* input,
                             const unsigned char* input_end,
                             commc_lz77_match_t* match);

/*

         commc_lz77_hash_string()
	       ---
	       computes hash value for a 3-byte string used
	       in the hash table for fast match finding.
	       
	       parameters:
	         data - pointer to 3-byte sequence
	       
	       returns:
	         hash value masked to hash table size

*/

unsigned int commc_lz77_hash_string(const unsigned char* data);

/*

         commc_lz77_update_hash()
	       ---
	       updates hash tables when adding new data to
	       the sliding window buffer.
	       
	       parameters:
	         ctx - compression context
	         pos - position in sliding window
	         byte - byte value being added

*/

void commc_lz77_update_hash(commc_lz77_context_t* ctx, 
                           size_t pos, 
                           unsigned char byte);

/*

         commc_lz77_get_compression_ratio()
	       ---
	       calculates compression ratio from original
	       and compressed symbol stream sizes.
	       
	       parameters:
	         original_size - size of original data
	         symbol_count - number of compressed symbols
	       
	       returns:
	         compression ratio as percentage (0.0 - 100.0)

*/

double commc_lz77_get_compression_ratio(size_t original_size, 
                                       size_t symbol_count);

/*

         commc_lz77_estimate_output_size()
	       ---
	       estimates the maximum number of symbols needed
	       to compress data of a given size.
	       
	       parameters:
	         input_size - size of input data
	       
	       returns:
	         estimated maximum number of output symbols

*/

size_t commc_lz77_estimate_output_size(size_t input_size);

/*

         commc_lz77_validate_parameters()
	       ---
	       validates LZ77 compression parameters and
	       ensures they are within acceptable ranges.
	       
	       parameters:
	         window_size - sliding window size to validate
	         level - compression level to validate
	       
	       returns:
	         COMMC_SUCCESS if parameters are valid

*/

int commc_lz77_validate_parameters(size_t window_size, 
                                   commc_lz77_level_t level);

/*

         commc_lz77_get_statistics()
	       ---
	       retrieves compression statistics including
	       match counts, literal counts, and ratios.
	       
	       parameters:
	         symbols - compressed symbol stream
	         symbol_count - number of symbols
	         literal_count - pointer to store literal count
	         match_count - pointer to store match count
	         avg_match_length - pointer to store average match length
	       
	       returns:
	         COMMC_SUCCESS or error code

*/

int commc_lz77_get_statistics(const commc_lz77_symbol_t* symbols,
                              size_t symbol_count,
                              size_t* literal_count,
                              size_t* match_count,
                              double* avg_match_length);

#endif /* COMMC_LZ77_H */

/*
	==================================
             --- EOF ---
	==================================
*/