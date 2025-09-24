/*
   ===================================
   C O M M O N - C
   LZ77 COMPRESSION IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- LZ77 MODULE ---

    implementation of LZ77 lossless data compression
    algorithm with sliding window dictionary and
    efficient match finding for high-performance
    data compression.

    see include/commc/lz77.h for function prototypes
    and detailed documentation.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#include  <stdlib.h>      /* for malloc, free */
#include  <string.h>      /* for memcpy, memset */

#include  "commc/lz77.h"
#include  "commc/error.h"

/*
	==================================
             --- STATIC HELPERS ---
	==================================
*/

/*

         validate_context()
	       ---
	       validates that a compression context is valid
	       and properly initialized.

*/

static int validate_context(const commc_lz77_context_t* ctx) {

  if  (!ctx) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }

  if  (!ctx->window || !ctx->hash_table || !ctx->prev_table) {

    return COMMC_ERROR_INVALID_STATE;

  }

  return COMMC_SUCCESS;

}

/*

         compare_sequences()
	       ---
	       compares two byte sequences and returns the
	       length of the common prefix.

*/

static size_t compare_sequences(const unsigned char* seq1,
                               const unsigned char* seq2,
                               size_t max_length) {

  size_t length = 0;

  if  (!seq1 || !seq2) {

    return 0;

  }

  while  (length < max_length && seq1[length] == seq2[length]) {

    length++;

  }

  return length;

}

/*

         circular_distance()
	       ---
	       calculates distance between two positions in
	       the circular sliding window buffer.

*/

static size_t circular_distance(size_t from, size_t to, size_t window_size) {

  if  (from <= to) {

    return to - from;

  } else {

    return window_size - from + to;

  }

}

/*

         get_level_parameters()
	       ---
	       sets compression parameters based on the
	       specified compression level.

*/

static void get_level_parameters(commc_lz77_level_t level,
                                size_t* max_chain_length,
                                size_t* good_match_length,
                                size_t* lazy_threshold) {

  switch  (level) {

    case COMMC_LZ77_LEVEL_FAST:
      *max_chain_length = 4;
      *good_match_length = 8;
      *lazy_threshold = 4;
      break;

    case COMMC_LZ77_LEVEL_DEFAULT:
      *max_chain_length = 32;
      *good_match_length = 32;
      *lazy_threshold = 16;
      break;

    case COMMC_LZ77_LEVEL_BEST:
      *max_chain_length = 128;
      *good_match_length = 128;
      *lazy_threshold = 32;
      break;

    default:
      /* use default parameters for unknown levels */
      *max_chain_length = 32;
      *good_match_length = 32;
      *lazy_threshold = 16;
      break;

  }

}

/*
	==================================
             --- PUBLIC FUNCS ---
	==================================
*/

/*

         commc_lz77_context_create()
	       ---
	       creates and initializes a new LZ77 compression context.

*/

commc_lz77_context_t* commc_lz77_context_create(size_t window_size,
                                                 commc_lz77_level_t level) {

  commc_lz77_context_t* ctx;
  
  size_t max_chain, good_match, lazy_threshold;

  /* validate parameters */

  if  (commc_lz77_validate_parameters(window_size, level) != COMMC_SUCCESS) {

    return NULL;

  }

  /* allocate context structure */

  ctx = (commc_lz77_context_t*)malloc(sizeof(commc_lz77_context_t));
  
  if  (!ctx) {

    return NULL;

  }

  /* initialize basic parameters */

  ctx->window_size = window_size;
  ctx->level = level;
  ctx->window_pos = 0;
  ctx->window_filled = 0;

  /* allocate sliding window buffer */

  ctx->window = (unsigned char*)malloc(window_size);
  
  if  (!ctx->window) {

    free(ctx);
    return NULL;

  }

  /* allocate hash table */

  ctx->hash_table = (int*)malloc(COMMC_LZ77_HASH_SIZE * sizeof(int));
  
  if  (!ctx->hash_table) {

    free(ctx->window);
    free(ctx);
    return NULL;

  }

  /* allocate previous occurrence table */

  ctx->prev_table = (int*)malloc(window_size * sizeof(int));
  
  if  (!ctx->prev_table) {

    free(ctx->hash_table);
    free(ctx->window);
    free(ctx);
    return NULL;

  }

  /* set compression level parameters */

  get_level_parameters(level, &max_chain, &good_match, &lazy_threshold);
  
  ctx->max_chain_length = max_chain;
  ctx->good_match_length = good_match;
  ctx->lazy_match_threshold = lazy_threshold;

  /* initialize tables */

  memset(ctx->hash_table, -1, COMMC_LZ77_HASH_SIZE * sizeof(int));
  memset(ctx->prev_table, -1, window_size * sizeof(int));
  memset(ctx->window, 0, window_size);

  return ctx;

}

/*

         commc_lz77_context_destroy()
	       ---
	       releases all memory associated with an LZ77 context.

*/

void commc_lz77_context_destroy(commc_lz77_context_t* ctx) {

  if  (!ctx) {

    return;

  }

  if  (ctx->prev_table) {

    free(ctx->prev_table);

  }

  if  (ctx->hash_table) {

    free(ctx->hash_table);

  }

  if  (ctx->window) {

    free(ctx->window);

  }

  free(ctx);

}

/*

         commc_lz77_context_reset()
	       ---
	       resets the compression context to initial state.

*/

int commc_lz77_context_reset(commc_lz77_context_t* ctx) {

  int result = validate_context(ctx);

  if  (result != COMMC_SUCCESS) {

    return result;

  }

  /* reset position and fill counters */

  ctx->window_pos = 0;
  ctx->window_filled = 0;

  /* clear all tables */

  memset(ctx->hash_table, -1, COMMC_LZ77_HASH_SIZE * sizeof(int));
  memset(ctx->prev_table, -1, ctx->window_size * sizeof(int));
  memset(ctx->window, 0, ctx->window_size);

  return COMMC_SUCCESS;

}

/*

         commc_lz77_hash_string()
	       ---
	       computes hash value for a 3-byte string.

*/

unsigned int commc_lz77_hash_string(const unsigned char* data) {

  unsigned int hash;

  if  (!data) {

    return 0;

  }

  /* simple polynomial rolling hash */

  hash = (unsigned int)data[0];
  hash = (hash << 5) ^ (unsigned int)data[1];
  hash = (hash << 5) ^ (unsigned int)data[2];

  return hash & COMMC_LZ77_HASH_MASK;

}

/*

         commc_lz77_update_hash()
	       ---
	       updates hash tables when adding new data.

*/

void commc_lz77_update_hash(commc_lz77_context_t* ctx,
                           size_t pos,
                           unsigned char byte) {

  unsigned int hash;
  
  size_t window_pos;

  if  (!ctx || pos < 2) {

    return;  /* need at least 3 bytes for hashing */

  }

  /* add byte to window */

  window_pos = pos % ctx->window_size;
  ctx->window[window_pos] = byte;

  /* update window fill tracking */

  if  (ctx->window_filled < ctx->window_size) {

    ctx->window_filled++;

  }

  /* compute hash for 3-byte string ending at this position */

  if  (pos >= 2) {

    hash = commc_lz77_hash_string(&ctx->window[(pos - 2) % ctx->window_size]);

    /* link previous occurrence */

    ctx->prev_table[window_pos] = ctx->hash_table[hash];

    /* update hash table head */

    ctx->hash_table[hash] = (int)window_pos;

  }

}

/*

         commc_lz77_find_match()
	       ---
	       searches for the longest match in the sliding window.

*/

size_t commc_lz77_find_match(commc_lz77_context_t* ctx,
                            size_t current_pos,
                            const unsigned char* input,
                            const unsigned char* input_end,
                            commc_lz77_match_t* match) {

  unsigned int hash;
  
  int prev_pos;
  
  size_t chain_length;
  size_t best_length = 0;
  size_t best_distance = 0;
  size_t current_length;
  size_t max_length;
  size_t remaining_input;

  if  (!ctx || !input || !input_end || !match || current_pos + 2 >= (size_t)(input_end - input)) {

    return 0;

  }

  remaining_input = (size_t)(input_end - input) - current_pos;
  max_length = remaining_input < COMMC_LZ77_MAX_MATCH_LENGTH ? 
               remaining_input : COMMC_LZ77_MAX_MATCH_LENGTH;

  /* compute hash for current 3-byte string */

  hash = commc_lz77_hash_string(&input[current_pos]);

  /* start hash chain traversal */

  prev_pos = ctx->hash_table[hash];
  chain_length = 0;

  while  (prev_pos >= 0 && chain_length < ctx->max_chain_length) {

    size_t distance = circular_distance((size_t)prev_pos, 
                                       current_pos % ctx->window_size, 
                                       ctx->window_size);

    /* check if distance is within valid range */

    if  (distance > COMMC_LZ77_MAX_DISTANCE || distance == 0) {

      break;

    }

    /* compare sequences */

    current_length = compare_sequences(&input[current_pos],
                                     &ctx->window[prev_pos],
                                     max_length);

    /* check if this is our best match so far */

    if  (current_length > best_length && current_length >= COMMC_LZ77_MIN_MATCH_LENGTH) {

      best_length = current_length;
      best_distance = distance;

      /* early termination for good matches */

      if  (current_length >= ctx->good_match_length) {

        break;

      }

    }

    /* follow hash chain */

    prev_pos = ctx->prev_table[prev_pos];
    chain_length++;

  }

  /* store best match found */

  if  (best_length >= COMMC_LZ77_MIN_MATCH_LENGTH) {

    match->length = (unsigned int)best_length;
    match->distance = (unsigned int)best_distance;
    return best_length;

  }

  return 0;

}

/*

         commc_lz77_compress()
	       ---
	       compresses input data using LZ77 algorithm.

*/

int commc_lz77_compress(commc_lz77_context_t* ctx,
                        const unsigned char* input,
                        size_t input_size,
                        commc_lz77_symbol_t* output,
                        size_t output_capacity,
                        size_t* symbols_written) {

  size_t input_pos = 0;
  size_t output_pos = 0;
  
  commc_lz77_match_t current_match;
  commc_lz77_match_t next_match;
  
  size_t match_length;
  size_t next_match_length;
  
  int result;

  /* validate parameters */

  result = validate_context(ctx);
  
  if  (result != COMMC_SUCCESS) {

    return result;

  }

  if  (!input || !output || !symbols_written) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }

  *symbols_written = 0;

  /* fill initial window */

  while  (input_pos < input_size && input_pos < ctx->window_size) {

    commc_lz77_update_hash(ctx, input_pos, input[input_pos]);
    input_pos++;

  }

  input_pos = 0;  /* reset for actual compression */

  /* main compression loop */

  while  (input_pos < input_size && output_pos < output_capacity) {

    /* find match at current position */

    match_length = commc_lz77_find_match(ctx, input_pos, input, 
                                        input + input_size, &current_match);

    /* check for lazy matching (look ahead one position) */

    if  (match_length >= COMMC_LZ77_MIN_MATCH_LENGTH && 
         match_length < ctx->lazy_match_threshold &&
         input_pos + 1 < input_size) {

      next_match_length = commc_lz77_find_match(ctx, input_pos + 1, input,
                                               input + input_size, &next_match);

      /* if next position has significantly better match, use literal */

      if  (next_match_length > match_length + 1) {

        match_length = 0;  /* force literal output */

      }

    }

    /* output symbol based on match result */

    if  (match_length >= COMMC_LZ77_MIN_MATCH_LENGTH) {

      /* output match symbol */

      output[output_pos].type = COMMC_LZ77_SYMBOL_MATCH;
      output[output_pos].data.match = current_match;
      output_pos++;

      /* update hash for all consumed bytes */

      while  (match_length > 0 && input_pos < input_size) {

        commc_lz77_update_hash(ctx, input_pos, input[input_pos]);
        input_pos++;
        match_length--;

      }

    } else {

      /* output literal symbol */

      output[output_pos].type = COMMC_LZ77_SYMBOL_LITERAL;
      output[output_pos].data.literal = input[input_pos];
      output_pos++;

      /* update hash for this byte */

      commc_lz77_update_hash(ctx, input_pos, input[input_pos]);
      input_pos++;

    }

  }

  *symbols_written = output_pos;

  /* check if we processed all input */

  if  (input_pos < input_size) {

    return COMMC_ERROR_BUFFER_TOO_SMALL;

  }

  return COMMC_SUCCESS;

}

/*

         commc_lz77_decompress()
	       ---
	       decompresses LZ77 symbols back to original data.

*/

int commc_lz77_decompress(commc_lz77_context_t* ctx,
                          const commc_lz77_symbol_t* symbols,
                          size_t symbol_count,
                          unsigned char* output,
                          size_t output_capacity,
                          size_t* bytes_written) {

  size_t symbol_pos = 0;
  size_t output_pos = 0;
  
  size_t match_start;
  size_t copy_length;
  size_t i;
  
  int result;

  /* validate parameters */

  result = validate_context(ctx);
  
  if  (result != COMMC_SUCCESS) {

    return result;

  }

  if  (!symbols || !output || !bytes_written) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }

  *bytes_written = 0;

  /* reset context for decompression */

  result = commc_lz77_context_reset(ctx);
  
  if  (result != COMMC_SUCCESS) {

    return result;

  }

  /* process each symbol */

  for  (symbol_pos = 0; symbol_pos < symbol_count; symbol_pos++) {

    if  (symbols[symbol_pos].type == COMMC_LZ77_SYMBOL_LITERAL) {

      /* output literal byte */

      if  (output_pos >= output_capacity) {

        return COMMC_ERROR_BUFFER_TOO_SMALL;

      }

      output[output_pos] = symbols[symbol_pos].data.literal;
      ctx->window[output_pos % ctx->window_size] = output[output_pos];
      output_pos++;

    } else if  (symbols[symbol_pos].type == COMMC_LZ77_SYMBOL_MATCH) {

      /* handle match symbol */

      copy_length = symbols[symbol_pos].data.match.length;

      if  (output_pos + copy_length > output_capacity) {

        return COMMC_ERROR_BUFFER_TOO_SMALL;

      }

      /* calculate start position for copy */

      if  (symbols[symbol_pos].data.match.distance > output_pos) {

        return COMMC_ERROR_INVALID_DATA;

      }

      match_start = output_pos - symbols[symbol_pos].data.match.distance;

      /* copy data from sliding window */

      for  (i = 0; i < copy_length; i++) {

        output[output_pos] = output[match_start + i];
        ctx->window[output_pos % ctx->window_size] = output[output_pos];
        output_pos++;

      }

    } else {

      return COMMC_ERROR_INVALID_DATA;

    }

  }

  *bytes_written = output_pos;
  return COMMC_SUCCESS;

}

/*

         commc_lz77_validate_parameters()
	       ---
	       validates LZ77 compression parameters.

*/

int commc_lz77_validate_parameters(size_t window_size,
                                   commc_lz77_level_t level) {

  if  (window_size < COMMC_LZ77_MIN_WINDOW_SIZE || 
       window_size > COMMC_LZ77_MAX_WINDOW_SIZE) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }

  if  (level < COMMC_LZ77_LEVEL_FAST || level > COMMC_LZ77_LEVEL_BEST) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }

  return COMMC_SUCCESS;

}

/*

         commc_lz77_estimate_output_size()
	       ---
	       estimates maximum number of output symbols needed.

*/

size_t commc_lz77_estimate_output_size(size_t input_size) {

  /* worst case: every byte becomes a literal symbol */
  /* add 10% safety margin for overhead */

  return input_size + (input_size / 10) + 256;

}

/*

         commc_lz77_get_compression_ratio()
	       ---
	       calculates compression ratio from sizes.

*/

double commc_lz77_get_compression_ratio(size_t original_size,
                                       size_t symbol_count) {

  if  (original_size == 0) {

    return 0.0;

  }

  /* rough estimate: assume each symbol represents ~1.5 bytes on average */

  return (double)(symbol_count * 1.5) / (double)original_size * 100.0;

}

/*

         commc_lz77_get_statistics()
	       ---
	       retrieves compression statistics from symbol stream.

*/

int commc_lz77_get_statistics(const commc_lz77_symbol_t* symbols,
                              size_t symbol_count,
                              size_t* literal_count,
                              size_t* match_count,
                              double* avg_match_length) {

  size_t literals = 0;
  size_t matches = 0;
  size_t total_match_length = 0;
  size_t i;

  if  (!symbols || !literal_count || !match_count || !avg_match_length) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }

  /* analyze symbol stream */

  for  (i = 0; i < symbol_count; i++) {

    if  (symbols[i].type == COMMC_LZ77_SYMBOL_LITERAL) {

      literals++;

    } else if  (symbols[i].type == COMMC_LZ77_SYMBOL_MATCH) {

      matches++;
      total_match_length += symbols[i].data.match.length;

    }

  }

  *literal_count = literals;
  *match_count = matches;
  *avg_match_length = matches > 0 ? (double)total_match_length / (double)matches : 0.0;

  return COMMC_SUCCESS;

}

/*
	==================================
             --- EOF ---
	==================================
*/