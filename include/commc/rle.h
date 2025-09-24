/* 	
   ===================================
   R U N - L E N G T H  E N C O D I N G
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

	                --- RLE ETHOS ---

	    run-length encoding (RLE) is one of the simplest 
	    compression algorithms. it works by replacing 
	    consecutive identical bytes with a count and the byte 
	    value. for example, "AAAABBBB" becomes "4A4B".
	    
	    this implementation provides configurable escape 
	    sequences to handle edge cases and threshold tuning 
	    to avoid expanding data when compression would be 
	    ineffective.

*/

/* 
	==================================
             --- SETUP ---
	==================================
*/

#ifndef COMMC_RLE_H
#define COMMC_RLE_H

#include  <stddef.h>   /* for size_t */
#include  <stdint.h>   /* for uint8_t */

#include  "commc/error.h"

/*
	==================================
             --- CONSTANTS ---
	==================================
*/

/* RLE algorithm configuration */

#define COMMC_RLE_MAX_RUN_LENGTH           255      /* maximum run length */
#define COMMC_RLE_DEFAULT_ESCAPE           0xFE    /* default escape byte */
#define COMMC_RLE_DEFAULT_THRESHOLD        3       /* minimum run for compression */

/* RLE modes for different data types */

#define COMMC_RLE_MODE_STANDARD           0        /* standard byte-wise RLE */
#define COMMC_RLE_MODE_ADAPTIVE           1        /* adaptive threshold RLE */
#define COMMC_RLE_MODE_SAFE               2        /* safe mode with escape handling */

/*
	==================================
             --- DATA TYPES ---
	==================================
*/

/*

         commc_rle_config_t
	       ---
	       configuration parameters for run-length encoding.
	       allows customization of escape sequences, thresholds,
	       and compression modes for different data types.

*/

typedef struct {

  uint8_t escape_byte;              /* escape sequence marker */
  int mode;                         /* compression mode */
  size_t threshold;                 /* minimum run length to compress */
  
  /* adaptive threshold parameters */
  
  double efficiency_target;         /* target compression efficiency */
  size_t sample_size;               /* bytes to sample for adaptation */

} commc_rle_config_t;

/*

         commc_rle_stats_t
	       ---
	       compression statistics and analysis data
	       for performance monitoring and optimization.

*/

typedef struct {

  size_t input_size;                /* original data size */
  size_t output_size;               /* compressed data size */
  double compression_ratio;         /* compression ratio achieved */
  
  size_t runs_encoded;              /* number of runs compressed */
  size_t literals_copied;           /* number of literal bytes */
  size_t escapes_added;             /* number of escape sequences */
  
  size_t max_run_length;            /* longest run found */
  size_t avg_run_length;            /* average run length */

} commc_rle_stats_t;

/*

         commc_rle_compressed_t
	       ---
	       represents compressed RLE data with metadata
	       and statistics for decompression and analysis.

*/

typedef struct {

  unsigned char* data;              /* compressed data stream */
  size_t data_size;                 /* size of compressed data */
  
  commc_rle_config_t config;        /* compression configuration used */
  commc_rle_stats_t stats;          /* compression statistics */
  
  size_t original_size;             /* size of original data */

} commc_rle_compressed_t;

/*
	==================================
             --- PROTOTYPES ---
	==================================
*/

/* configuration functions */

commc_rle_config_t* commc_rle_config_create(void);

void commc_rle_config_destroy(commc_rle_config_t* config);

void commc_rle_config_set_defaults(commc_rle_config_t* config);

void commc_rle_config_set_escape(commc_rle_config_t* config, 
                                 uint8_t escape_byte);

void commc_rle_config_set_threshold(commc_rle_config_t* config, 
                                    size_t threshold);

void commc_rle_config_set_mode(commc_rle_config_t* config, 
                               int mode);

/* compression functions */

commc_rle_compressed_t* commc_rle_compress(const unsigned char* input,
                                           size_t input_size,
                                           const commc_rle_config_t* config);

int commc_rle_compress_buffer(const unsigned char* input,
                              size_t input_size,
                              unsigned char* output,
                              size_t output_capacity,
                              size_t* output_size,
                              const commc_rle_config_t* config);

/* decompression functions */

unsigned char* commc_rle_decompress(const commc_rle_compressed_t* compressed,
                                    size_t* output_size);

int commc_rle_decompress_buffer(const unsigned char* input,
                                size_t input_size,
                                unsigned char* output,
                                size_t output_capacity,
                                size_t* output_size,
                                const commc_rle_config_t* config);

/* utility functions */

size_t commc_rle_estimate_compressed_size(const unsigned char* input,
                                          size_t input_size,
                                          const commc_rle_config_t* config);

int commc_rle_analyze_data(const unsigned char* input,
                           size_t input_size,
                           commc_rle_stats_t* stats);

double commc_rle_calculate_efficiency(const unsigned char* input,
                                      size_t input_size,
                                      const commc_rle_config_t* config);

/* memory management */

void commc_rle_compressed_destroy(commc_rle_compressed_t* compressed);

/* validation functions */

int commc_rle_validate_config(const commc_rle_config_t* config);

int commc_rle_is_compressible(const unsigned char* input,
                              size_t input_size,
                              const commc_rle_config_t* config);

/* 
	==================================
             --- EOF ---
	==================================
*/

#endif /* COMMC_RLE_H */