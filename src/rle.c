/* 	
   ===================================
   R U N - L E N G T H  E N C O D I N G
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

	                --- RLE ETHOS ---

	    run-length encoding compresses repetitive data by 
	    replacing consecutive identical bytes with count-value 
	    pairs. this implementation handles edge cases through 
	    configurable escape sequences and provides adaptive 
	    threshold tuning to avoid data expansion when 
	    compression would be counterproductive.

*/

/* 
	==================================
             --- SETUP ---
	==================================
*/

#include  <stdio.h>       /* for printf debugging */
#include  <stdlib.h>      /* for malloc, free */
#include  <string.h>      /* for memcpy, memset */

#include  "commc/rle.h"
#include  "commc/error.h"

/*
	==================================
             --- INTERNAL ---
	==================================
*/

/*

         find_next_escape_safe_byte()
	       ---
	       finds a byte value that doesn't appear in the input
	       data, suitable for use as an escape sequence.

*/

static uint8_t find_next_escape_safe_byte(const unsigned char* input,
                                          size_t input_size,
                                          uint8_t start_byte) {

  uint8_t candidate;
  size_t i;
  int found;
  
  for  (candidate = start_byte; candidate != start_byte - 1; candidate++) {
  
    found = 0;
    
    for  (i = 0; i < input_size; i++) {
    
      if  (input[i] == candidate) {
      
        found = 1;
        break;
      }
    }
    
    if  (!found) {
    
      return candidate;
    }
  }
  
  /* if we can't find a safe byte, use the original */
  
  return start_byte;

}

/*

         calculate_run_statistics()
	       ---
	       analyzes input data to calculate run statistics
	       for compression efficiency estimation.

*/

static void calculate_run_statistics(const unsigned char* input,
                                     size_t input_size,
                                     commc_rle_stats_t* stats) {

  size_t i;
  size_t run_length;
  size_t total_run_length;
  unsigned char current_byte;

  if  (!input || !stats || input_size == 0) {

    return;

  }
  
  memset(stats, 0, sizeof(commc_rle_stats_t));
  
  stats->input_size = input_size;
  total_run_length = 0;
  
  i = 0;
  
  while  (i < input_size) {
  
    current_byte = input[i];
    run_length = 1;
    
    /* find the end of this run */
    
    while  (i + run_length < input_size && 
            input[i + run_length] == current_byte &&
            run_length < COMMC_RLE_MAX_RUN_LENGTH) {
            
      run_length++;
    }
    
    if  (run_length > 1) {
    
      stats->runs_encoded++;
      total_run_length += run_length;
      
      if  (run_length > stats->max_run_length) {
      
        stats->max_run_length = run_length;
      }
      
    } else {
    
      stats->literals_copied++;
    }
    
    i += run_length;
  }
  
  if  (stats->runs_encoded > 0) {
  
    stats->avg_run_length = total_run_length / stats->runs_encoded;
  }

}

/*
	==================================
             --- CONFIGURATION ---
	==================================
*/

/*

         commc_rle_config_create()
	       ---
	       creates a new RLE configuration with default
	       settings suitable for general-purpose compression.

*/

commc_rle_config_t* commc_rle_config_create(void) {

  commc_rle_config_t* config;

  config = (commc_rle_config_t*)malloc(sizeof(commc_rle_config_t));
  
  if  (!config) {

    return NULL;

  }
  
  commc_rle_config_set_defaults(config);

  return config;

}

/*

         commc_rle_config_destroy()
	       ---
	       releases memory used by an RLE configuration.

*/

void commc_rle_config_destroy(commc_rle_config_t* config) {

  if  (config) {

    free(config);

  }

}

/*

         commc_rle_config_set_defaults()
	       ---
	       initializes configuration with standard defaults
	       suitable for most compression scenarios.

*/

void commc_rle_config_set_defaults(commc_rle_config_t* config) {

  if  (!config) {

    return;

  }
  
  config->escape_byte = COMMC_RLE_DEFAULT_ESCAPE;
  config->mode = COMMC_RLE_MODE_STANDARD;
  config->threshold = COMMC_RLE_DEFAULT_THRESHOLD;
  config->efficiency_target = 0.7;  /* 70% compression ratio target */
  config->sample_size = 1024;       /* 1KB sample for adaptation */

}

/*

         commc_rle_config_set_escape()
	       ---
	       sets the escape byte used for encoding special
	       sequences in the compressed data.

*/

void commc_rle_config_set_escape(commc_rle_config_t* config, 
                                 uint8_t escape_byte) {

  if  (config) {

    config->escape_byte = escape_byte;

  }

}

/*

         commc_rle_config_set_threshold()
	       ---
	       sets the minimum run length threshold for
	       compression to avoid expanding short runs.

*/

void commc_rle_config_set_threshold(commc_rle_config_t* config, 
                                    size_t threshold) {

  if  (config && threshold > 0 && threshold <= COMMC_RLE_MAX_RUN_LENGTH) {

    config->threshold = threshold;

  }

}

/*

         commc_rle_config_set_mode()
	       ---
	       sets the compression mode affecting how the
	       algorithm handles different data patterns.

*/

void commc_rle_config_set_mode(commc_rle_config_t* config, 
                               int mode) {

  if  (config && 
       (mode == COMMC_RLE_MODE_STANDARD || 
        mode == COMMC_RLE_MODE_ADAPTIVE ||
        mode == COMMC_RLE_MODE_SAFE)) {

    config->mode = mode;

  }

}

/*
	==================================
             --- COMPRESSION ---
	==================================
*/

/*

         commc_rle_compress()
	       ---
	       compresses input data using run-length encoding
	       with the specified configuration parameters.

*/

commc_rle_compressed_t* commc_rle_compress(const unsigned char* input,
                                           size_t input_size,
                                           const commc_rle_config_t* config) {

  commc_rle_compressed_t* result;
  commc_rle_config_t default_config;
  const commc_rle_config_t* active_config;
  size_t estimated_size;
  size_t output_size;
  int compress_result;

  if  (!input || input_size == 0) {

    return NULL;

  }
  
  /* use provided config or create default */
  
  if  (config) {
  
    active_config = config;
    
  } else {
  
    commc_rle_config_set_defaults(&default_config);
    active_config = &default_config;
  }
  
  /* allocate result structure */
  
  result = (commc_rle_compressed_t*)malloc(sizeof(commc_rle_compressed_t));
  
  if  (!result) {

    return NULL;

  }
  
  /* estimate output size */
  
  estimated_size = commc_rle_estimate_compressed_size(input, input_size, active_config);
  
  /* allocate output buffer */
  
  result->data = (unsigned char*)malloc(estimated_size);
  
  if  (!result->data) {

    free(result);
    return NULL;

  }
  
  /* perform compression */
  
  compress_result = commc_rle_compress_buffer(input, input_size,
                                              result->data, estimated_size,
                                              &output_size, active_config);
  
  if  (compress_result != COMMC_SUCCESS) {

    free(result->data);
    free(result);
    return NULL;

  }
  
  /* fill in result metadata */
  
  result->data_size = output_size;
  result->config = *active_config;
  result->original_size = input_size;
  
  /* calculate final statistics */
  
  calculate_run_statistics(input, input_size, &result->stats);
  result->stats.output_size = output_size;
  
  if  (input_size > 0) {
  
    result->stats.compression_ratio = (double)output_size / input_size;
  }

  return result;

}

/*

         commc_rle_compress_buffer()
	       ---
	       compresses input data into a provided output buffer
	       using run-length encoding with escape handling.

*/

int commc_rle_compress_buffer(const unsigned char* input,
                              size_t input_size,
                              unsigned char* output,
                              size_t output_capacity,
                              size_t* output_size,
                              const commc_rle_config_t* config) {

  commc_rle_config_t default_config;
  const commc_rle_config_t* active_config;
  size_t input_pos;
  size_t output_pos;
  size_t run_length;
  unsigned char current_byte;
  uint8_t escape_byte;

  if  (!input || !output || !output_size || input_size == 0) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }
  
  /* use provided config or create default */
  
  if  (config) {
  
    active_config = config;
    
  } else {
  
    commc_rle_config_set_defaults(&default_config);
    active_config = &default_config;
  }
  
  /* determine escape byte */
  
  if  (active_config->mode == COMMC_RLE_MODE_SAFE) {
  
    escape_byte = find_next_escape_safe_byte(input, input_size, 
                                             active_config->escape_byte);
  } else {
  
    escape_byte = active_config->escape_byte;
  }
  
  input_pos = 0;
  output_pos = 0;
  
  while  (input_pos < input_size) {
  
    current_byte = input[input_pos];
    run_length = 1;
    
    /* find the end of this run */
    
    while  (input_pos + run_length < input_size && 
            input[input_pos + run_length] == current_byte &&
            run_length < COMMC_RLE_MAX_RUN_LENGTH) {
            
      run_length++;
    }
    
    /* decide whether to compress this run */
    
    if  (run_length >= active_config->threshold) {
    
      /* check output buffer space for escape sequence */
      
      if  (output_pos + 3 > output_capacity) {
      
        return COMMC_ERROR_BUFFER_TOO_SMALL;
      }
      
      /* output: escape_byte, count, value */
      
      output[output_pos++] = escape_byte;
      output[output_pos++] = (unsigned char)run_length;
      output[output_pos++] = current_byte;
      
    } else {
    
      /* copy bytes literally */
      
      if  (output_pos + run_length > output_capacity) {
      
        return COMMC_ERROR_BUFFER_TOO_SMALL;
      }
      
      /* handle escape byte in literal data */
      
      if  (current_byte == escape_byte) {
      
        if  (output_pos + 2 > output_capacity) {
        
          return COMMC_ERROR_BUFFER_TOO_SMALL;
        }
        
        /* escape the escape byte: escape_byte, 0 */
        
        output[output_pos++] = escape_byte;
        output[output_pos++] = 0;
        input_pos++;
        
      } else {
      
        /* copy run literally */
        
        while  (run_length > 0) {
        
          output[output_pos++] = current_byte;
          run_length--;
        }
        
        input_pos += (input_pos + run_length < input_size) ? run_length : 
                     input_size - input_pos;
      }
      
      continue;  /* skip the normal input_pos increment */
    }
    
    input_pos += run_length;
  }
  
  *output_size = output_pos;

  return COMMC_SUCCESS;

}

/*
	==================================
             --- DECOMPRESSION ---
	==================================
*/

/*

         commc_rle_decompress()
	       ---
	       decompresses RLE-encoded data back to its original
	       form using the embedded configuration.

*/

unsigned char* commc_rle_decompress(const commc_rle_compressed_t* compressed,
                                    size_t* output_size) {

  unsigned char* output;
  size_t actual_size;
  int result;

  if  (!compressed || !compressed->data || !output_size) {

    return NULL;

  }
  
  /* allocate output buffer */
  
  output = (unsigned char*)malloc(compressed->original_size);
  
  if  (!output) {

    return NULL;

  }
  
  /* decompress */
  
  result = commc_rle_decompress_buffer(compressed->data, compressed->data_size,
                                       output, compressed->original_size,
                                       &actual_size, &compressed->config);
  
  if  (result != COMMC_SUCCESS || actual_size != compressed->original_size) {

    free(output);
    return NULL;

  }
  
  *output_size = actual_size;

  return output;

}

/*

         commc_rle_decompress_buffer()
	       ---
	       decompresses RLE-encoded data into a provided
	       output buffer with escape sequence handling.

*/

int commc_rle_decompress_buffer(const unsigned char* input,
                                size_t input_size,
                                unsigned char* output,
                                size_t output_capacity,
                                size_t* output_size,
                                const commc_rle_config_t* config) {

  commc_rle_config_t default_config;
  const commc_rle_config_t* active_config;
  size_t input_pos;
  size_t output_pos;
  uint8_t escape_byte;
  unsigned char count;
  unsigned char value;
  size_t i;

  if  (!input || !output || !output_size || input_size == 0) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }
  
  /* use provided config or create default */
  
  if  (config) {
  
    active_config = config;
    
  } else {
  
    commc_rle_config_set_defaults(&default_config);
    active_config = &default_config;
  }
  
  escape_byte = active_config->escape_byte;
  input_pos = 0;
  output_pos = 0;
  
  while  (input_pos < input_size) {
  
    if  (input[input_pos] == escape_byte) {
    
      /* process escape sequence */
      
      if  (input_pos + 2 >= input_size) {
      
        return COMMC_ERROR_INVALID_DATA;
      }
      
      count = input[input_pos + 1];
      
      if  (count == 0) {
      
        /* escaped escape byte */
        
        if  (output_pos >= output_capacity) {
        
          return COMMC_ERROR_BUFFER_TOO_SMALL;
        }
        
        output[output_pos++] = escape_byte;
        input_pos += 2;
        
      } else {
      
        /* run-length sequence */
        
        if  (input_pos + 2 >= input_size) {
        
          return COMMC_ERROR_INVALID_DATA;
        }
        
        value = input[input_pos + 2];
        
        if  (output_pos + count > output_capacity) {
        
          return COMMC_ERROR_BUFFER_TOO_SMALL;
        }
        
        /* expand the run */
        
        for  (i = 0; i < count; i++) {
        
          output[output_pos++] = value;
        }
        
        input_pos += 3;
      }
      
    } else {
    
      /* literal byte */
      
      if  (output_pos >= output_capacity) {
      
        return COMMC_ERROR_BUFFER_TOO_SMALL;
      }
      
      output[output_pos++] = input[input_pos++];
    }
  }
  
  *output_size = output_pos;

  return COMMC_SUCCESS;

}

/*
	==================================
             --- UTILITIES ---
	==================================
*/

/*

         commc_rle_estimate_compressed_size()
	       ---
	       estimates the compressed size for input data
	       to help with buffer allocation.

*/

size_t commc_rle_estimate_compressed_size(const unsigned char* input,
                                          size_t input_size,
                                          const commc_rle_config_t* config) {

  size_t estimated_size;
  size_t i;
  size_t run_length;
  const commc_rle_config_t* active_config;
  commc_rle_config_t default_config;

  if  (!input || input_size == 0) {

    return 0;

  }
  
  if  (config) {
  
    active_config = config;
    
  } else {
  
    commc_rle_config_set_defaults(&default_config);
    active_config = &default_config;
  }
  
  estimated_size = 0;
  i = 0;
  
  while  (i < input_size) {
  
    run_length = 1;
    
    while  (i + run_length < input_size && 
            input[i + run_length] == input[i] &&
            run_length < COMMC_RLE_MAX_RUN_LENGTH) {
            
      run_length++;
    }
    
    if  (run_length >= active_config->threshold) {
    
      /* compressed: escape + count + value */
      
      estimated_size += 3;
      
    } else {
    
      /* literal bytes */
      
      estimated_size += run_length;
      
      /* account for potential escape sequences */
      
      if  (input[i] == active_config->escape_byte) {
      
        estimated_size += run_length;  /* double for escaping */
      }
    }
    
    i += run_length;
  }
  
  /* add some safety margin */
  
  return estimated_size + (estimated_size / 10) + 16;

}

/*

         commc_rle_analyze_data()
	       ---
	       analyzes input data and fills statistics
	       structure with compression metrics.

*/

int commc_rle_analyze_data(const unsigned char* input,
                           size_t input_size,
                           commc_rle_stats_t* stats) {

  if  (!input || !stats || input_size == 0) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }
  
  calculate_run_statistics(input, input_size, stats);

  return COMMC_SUCCESS;

}

/*

         commc_rle_calculate_efficiency()
	       ---
	       calculates the expected compression efficiency
	       for the input data with given configuration.

*/

double commc_rle_calculate_efficiency(const unsigned char* input,
                                      size_t input_size,
                                      const commc_rle_config_t* config) {

  size_t estimated_size;

  if  (!input || input_size == 0) {

    return 1.0;  /* no compression */

  }
  
  estimated_size = commc_rle_estimate_compressed_size(input, input_size, config);
  
  return (double)estimated_size / input_size;

}

/*
	==================================
             --- MEMORY MGMT ---
	==================================
*/

/*

         commc_rle_compressed_destroy()
	       ---
	       releases all memory used by a compressed
	       RLE data structure.

*/

void commc_rle_compressed_destroy(commc_rle_compressed_t* compressed) {

  if  (compressed) {

    if  (compressed->data) {

      free(compressed->data);

    }
    
    free(compressed);

  }

}

/*
	==================================
             --- VALIDATION ---
	==================================
*/

/*

         commc_rle_validate_config()
	       ---
	       validates RLE configuration parameters for
	       correctness and consistency.

*/

int commc_rle_validate_config(const commc_rle_config_t* config) {

  if  (!config) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }
  
  if  (config->threshold == 0 || config->threshold > COMMC_RLE_MAX_RUN_LENGTH) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }
  
  if  (config->mode < COMMC_RLE_MODE_STANDARD || config->mode > COMMC_RLE_MODE_SAFE) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }
  
  if  (config->efficiency_target < 0.0 || config->efficiency_target > 1.0) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }

  return COMMC_SUCCESS;

}

/*

         commc_rle_is_compressible()
	       ---
	       analyzes input data to determine if RLE
	       compression would be beneficial.

*/

int commc_rle_is_compressible(const unsigned char* input,
                              size_t input_size,
                              const commc_rle_config_t* config) {

  double efficiency;
  const commc_rle_config_t* active_config;
  commc_rle_config_t default_config;

  if  (!input || input_size == 0) {

    return 0;

  }
  
  if  (config) {
  
    active_config = config;
    
  } else {
  
    commc_rle_config_set_defaults(&default_config);
    active_config = &default_config;
  }
  
  efficiency = commc_rle_calculate_efficiency(input, input_size, active_config);
  
  return efficiency < 0.9;  /* worth compressing if we save at least 10% */

}

/* 
	==================================
             --- EOF ---
	==================================
*/