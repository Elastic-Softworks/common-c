/*
   ===================================
   C O M M O N - C
   DEFLATE-STYLE COMPRESSION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- DEFLATE MODULE ---

    implementation of deflate-style compression combining
    LZ77 and Huffman coding in a two-stage pipeline for
    maximum compression efficiency.

    this module follows the deflate algorithm specification
    used in gzip, zip, and PNG formats, providing industry-
    standard compression with excellent ratios and speed.

    stage 1: LZ77 compression reduces redundancy by finding
             repeated sequences and replacing them with
             back-references.

    stage 2: Huffman coding applies optimal entropy coding
             to the LZ77 symbol stream, further reducing
             the data size.

*/

#ifndef COMMC_DEFLATE_H
#define COMMC_DEFLATE_H

/*
	==================================
             --- INCLUDES ---
	==================================
*/

#include  <stddef.h>   /* for size_t */

#include  "commc/lz77.h"
#include  "commc/huffman.h"

/*
	==================================
             --- CONSTANTS ---
	==================================
*/

/* deflate algorithm configuration */

#define COMMC_DEFLATE_MAX_WINDOW_SIZE       32768    /* 32KB sliding window */
#define COMMC_DEFLATE_DEFAULT_WINDOW_SIZE   4096     /* 4KB default window */
#define COMMC_DEFLATE_MIN_WINDOW_SIZE       256      /* 256B minimum window */

/* compression levels */

#define COMMC_DEFLATE_LEVEL_STORE_VALUE     0        /* no compression */
#define COMMC_DEFLATE_LEVEL_FAST_VALUE      1        /* fast compression */
#define COMMC_DEFLATE_LEVEL_DEFAULT_VALUE   6        /* balanced speed/ratio */
#define COMMC_DEFLATE_LEVEL_BEST_VALUE      9        /* maximum compression */

/* block types for deflate format */

#define COMMC_DEFLATE_BLOCK_RAW             0        /* uncompressed block */
#define COMMC_DEFLATE_BLOCK_FIXED           1        /* fixed Huffman codes */
#define COMMC_DEFLATE_BLOCK_DYNAMIC         2        /* dynamic Huffman codes */

/* deflate symbol alphabet extensions */

#define COMMC_DEFLATE_LITERAL_SYMBOLS       256      /* 0-255 literal bytes */
#define COMMC_DEFLATE_LENGTH_SYMBOLS        29       /* 257-285 length codes */
#define COMMC_DEFLATE_DISTANCE_SYMBOLS      30       /* distance codes */
#define COMMC_DEFLATE_END_OF_BLOCK          256      /* end-of-block symbol */

#define COMMC_DEFLATE_TOTAL_SYMBOLS         (COMMC_DEFLATE_LITERAL_SYMBOLS + \
                                            COMMC_DEFLATE_LENGTH_SYMBOLS + 1)

/*
	==================================
             --- DATA TYPES ---
	==================================
*/

/*

         commc_deflate_level_t
	       ---
	       compression level enumeration affecting the
	       trade-off between speed and compression ratio.

*/

typedef enum {

  COMMC_DEFLATE_LEVEL_STORE    = COMMC_DEFLATE_LEVEL_STORE_VALUE,     /* no compression, store only */
  COMMC_DEFLATE_LEVEL_FAST     = COMMC_DEFLATE_LEVEL_FAST_VALUE,      /* fast compression */
  COMMC_DEFLATE_LEVEL_DEFAULT  = COMMC_DEFLATE_LEVEL_DEFAULT_VALUE,   /* balanced compression */
  COMMC_DEFLATE_LEVEL_BEST     = COMMC_DEFLATE_LEVEL_BEST_VALUE       /* maximum compression */

} commc_deflate_level_t;

/*

         commc_deflate_strategy_t
	       ---
	       compression strategy affecting how the algorithm
	       handles different types of data.

*/

typedef enum {

  COMMC_DEFLATE_STRATEGY_DEFAULT,     /* general-purpose compression */
  COMMC_DEFLATE_STRATEGY_FILTERED,    /* optimized for filtered data */
  COMMC_DEFLATE_STRATEGY_HUFFMAN_ONLY, /* Huffman coding only */
  COMMC_DEFLATE_STRATEGY_RLE          /* run-length encoding focused */

} commc_deflate_strategy_t;

/*

         commc_deflate_block_t
	       ---
	       represents a compressed block with its metadata
	       and compressed data stream.

*/

typedef struct {

  int type;                       /* block type (raw/fixed/dynamic) */
  int final_block;                /* 1 if this is the final block */
  
  size_t uncompressed_size;       /* size before compression */
  size_t compressed_size;         /* size after compression */
  
  unsigned char* data;            /* compressed block data */
  size_t data_capacity;           /* allocated size of data buffer */

} commc_deflate_block_t;

/*

         commc_deflate_context_t
	       ---
	       comprehensive compression context maintaining
	       both LZ77 and Huffman states for the two-stage
	       compression pipeline.

*/

typedef struct {

  /* compression parameters */
  
  commc_deflate_level_t level;        /* compression level */
  commc_deflate_strategy_t strategy;  /* compression strategy */
  size_t window_size;                 /* LZ77 sliding window size */
  
  /* algorithm contexts */
  
  commc_lz77_context_t* lz77_ctx;     /* LZ77 compression context */
  commc_huffman_context_t* huff_ctx;  /* Huffman coding context */
  
  /* intermediate data */
  
  commc_lz77_symbol_t* lz77_symbols;  /* LZ77 output symbols */
  size_t symbol_count;                /* number of symbols */
  size_t symbol_capacity;             /* allocated symbol buffer size */
  
  /* block management */
  
  commc_deflate_block_t* blocks;      /* compressed blocks */
  size_t block_count;                 /* number of blocks */
  size_t block_capacity;              /* allocated blocks capacity */
  
  /* statistics */
  
  size_t input_size;                  /* total input size */
  size_t output_size;                 /* total compressed size */
  double compression_ratio;           /* achieved compression ratio */
  double lz77_ratio;                  /* LZ77 stage ratio */
  double huffman_ratio;               /* Huffman stage ratio */

} commc_deflate_context_t;

/*

         commc_deflate_compressed_t
	       ---
	       represents the final compressed output with
	       metadata and the complete compressed stream.

*/

typedef struct {

  unsigned char* data;                /* compressed data stream */
  size_t data_size;                   /* size of compressed data */
  
  commc_deflate_block_t* blocks;      /* block structure information */
  size_t block_count;                 /* number of blocks */
  
  size_t original_size;               /* size of original data */
  double compression_ratio;           /* compression ratio achieved */
  
  /* algorithm statistics */
  
  size_t lz77_symbols;                /* number of LZ77 symbols */
  size_t huffman_bits;                /* number of Huffman bits */

} commc_deflate_compressed_t;

/*
	==================================
             --- PROTOTYPES ---
	==================================
*/

/*

         commc_deflate_context_create()
	       ---
	       creates and initializes a deflate compression
	       context with specified parameters.
	       
	       parameters:
	         window_size - LZ77 sliding window size
	         level - compression level
	         strategy - compression strategy
	       
	       returns:
	         pointer to initialized context, or NULL on failure

*/

commc_deflate_context_t* commc_deflate_context_create(size_t window_size,
                                                      commc_deflate_level_t level,
                                                      commc_deflate_strategy_t strategy);

/*

         commc_deflate_context_destroy()
	       ---
	       releases all memory associated with a deflate
	       compression context.
	       
	       parameters:
	         ctx - pointer to context to destroy

*/

void commc_deflate_context_destroy(commc_deflate_context_t* ctx);

/*

         commc_deflate_compress()
	       ---
	       compresses input data using the two-stage
	       LZ77+Huffman deflate algorithm.
	       
	       parameters:
	         ctx - compression context
	         input - input data to compress
	         input_size - size of input data
	         output - pointer to store compressed result
	       
	       returns:
	         COMMC_SUCCESS or error code

*/

int commc_deflate_compress(commc_deflate_context_t* ctx,
                           const unsigned char* input,
                           size_t input_size,
                           commc_deflate_compressed_t* output);

/*

         commc_deflate_decompress()
	       ---
	       decompresses deflate-compressed data back to
	       the original uncompressed form.
	       
	       parameters:
	         ctx - decompression context
	         compressed - compressed data structure
	         output - buffer for decompressed data
	         output_size - size of output buffer
	         bytes_written - pointer to store output size
	       
	       returns:
	         COMMC_SUCCESS or error code

*/

int commc_deflate_decompress(commc_deflate_context_t* ctx,
                             const commc_deflate_compressed_t* compressed,
                             unsigned char* output,
                             size_t output_size,
                             size_t* bytes_written);

/*

         commc_deflate_compress_block()
	       ---
	       compresses a single block of data using the
	       specified block type and compression parameters.
	       
	       parameters:
	         ctx - compression context
	         input - input data block
	         input_size - size of input block
	         block_type - type of compression block
	         final_block - 1 if this is the final block
	         output - pointer to store compressed block
	       
	       returns:
	         COMMC_SUCCESS or error code

*/

int commc_deflate_compress_block(commc_deflate_context_t* ctx,
                                 const unsigned char* input,
                                 size_t input_size,
                                 int block_type,
                                 int final_block,
                                 commc_deflate_block_t* output);

/*

         commc_deflate_choose_block_type()
	       ---
	       analyzes input data and chooses the optimal
	       block type for compression.
	       
	       parameters:
	         ctx - compression context
	         input - input data to analyze
	         input_size - size of input data
	       
	       returns:
	         optimal block type for this data

*/

int commc_deflate_choose_block_type(commc_deflate_context_t* ctx,
                                    const unsigned char* input,
                                    size_t input_size);

/*

         commc_deflate_estimate_compressed_size()
	       ---
	       estimates the compressed size for input data
	       using the current compression parameters.
	       
	       parameters:
	         ctx - compression context
	         input_size - size of input data
	       
	       returns:
	         estimated compressed size in bytes

*/

size_t commc_deflate_estimate_compressed_size(commc_deflate_context_t* ctx,
                                              size_t input_size);

/*

         commc_deflate_get_compression_ratio()
	       ---
	       calculates the overall compression ratio achieved
	       by the deflate algorithm.
	       
	       parameters:
	         original_size - size of original data
	         compressed_size - size of compressed data
	       
	       returns:
	         compression ratio as percentage

*/

double commc_deflate_get_compression_ratio(size_t original_size,
                                           size_t compressed_size);

/*

         commc_deflate_validate_parameters()
	       ---
	       validates deflate compression parameters and
	       ensures they are within acceptable ranges.
	       
	       parameters:
	         window_size - sliding window size
	         level - compression level
	         strategy - compression strategy
	       
	       returns:
	         COMMC_SUCCESS if parameters are valid

*/

int commc_deflate_validate_parameters(size_t window_size,
                                      commc_deflate_level_t level,
                                      commc_deflate_strategy_t strategy);

/*

         commc_deflate_get_statistics()
	       ---
	       retrieves comprehensive compression statistics
	       including ratios for each stage.
	       
	       parameters:
	         ctx - compression context
	         lz77_ratio - pointer to store LZ77 ratio
	         huffman_ratio - pointer to store Huffman ratio
	         total_ratio - pointer to store total ratio
	       
	       returns:
	         COMMC_SUCCESS or error code

*/

int commc_deflate_get_statistics(const commc_deflate_context_t* ctx,
                                 double* lz77_ratio,
                                 double* huffman_ratio,
                                 double* total_ratio);

/*

         commc_deflate_block_create()
	       ---
	       creates a new deflate block structure.
	       
	       returns:
	         pointer to initialized block, or NULL on failure

*/

commc_deflate_block_t* commc_deflate_block_create(void);

/*

         commc_deflate_block_destroy()
	       ---
	       releases memory associated with a deflate block.
	       
	       parameters:
	         block - pointer to block to destroy

*/

void commc_deflate_block_destroy(commc_deflate_block_t* block);

/*

         commc_deflate_compressed_create()
	       ---
	       creates a new compressed data structure.
	       
	       returns:
	         pointer to initialized structure, or NULL on failure

*/

commc_deflate_compressed_t* commc_deflate_compressed_create(void);

/*

         commc_deflate_compressed_destroy()
	       ---
	       releases memory associated with compressed data.
	       
	       parameters:
	         compressed - pointer to compressed data to destroy

*/

void commc_deflate_compressed_destroy(commc_deflate_compressed_t* compressed);

/*

         commc_deflate_reset_context()
	       ---
	       resets the compression context for processing
	       a new data stream.
	       
	       parameters:
	         ctx - compression context to reset
	       
	       returns:
	         COMMC_SUCCESS or error code

*/

int commc_deflate_reset_context(commc_deflate_context_t* ctx);

/*

         commc_deflate_set_level()
	       ---
	       changes the compression level for subsequent
	       operations.
	       
	       parameters:
	         ctx - compression context
	         level - new compression level
	       
	       returns:
	         COMMC_SUCCESS or error code

*/

int commc_deflate_set_level(commc_deflate_context_t* ctx,
                            commc_deflate_level_t level);

/*

         commc_deflate_set_strategy()
	       ---
	       changes the compression strategy for subsequent
	       operations.
	       
	       parameters:
	         ctx - compression context
	         strategy - new compression strategy
	       
	       returns:
	         COMMC_SUCCESS or error code

*/

int commc_deflate_set_strategy(commc_deflate_context_t* ctx,
                               commc_deflate_strategy_t strategy);

#endif /* COMMC_DEFLATE_H */

/*
	==================================
             --- EOF ---
	==================================
*/