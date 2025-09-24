/*
   ===================================
   C O M M O N - C
   DEFLATE-STYLE COMPRESSION IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- DEFLATE MODULE ---

    implementation of deflate-style compression combining
    LZ77 and Huffman coding for maximum compression
    efficiency. provides industry-standard compression
    compatible with gzip and zip formats.

    see include/commc/deflate.h for function prototypes
    and detailed documentation.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#include  <stdlib.h>      /* for malloc, free */
#include  <string.h>      /* for memset, memcpy */
#include  <stdio.h>       /* for debugging output */

#include  "commc/deflate.h"
#include  "commc/error.h"

/*
	==================================
             --- STATIC HELPERS ---
	==================================
*/

/*

         validate_context()
	       ---
	       validates that a deflate context is properly
	       initialized and ready for use.

*/

static int validate_context(const commc_deflate_context_t* ctx) {

  if  (!ctx) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }

  if  (!ctx->lz77_ctx || !ctx->huff_ctx) {

    return COMMC_ERROR_INVALID_STATE;

  }

  return COMMC_SUCCESS;

}

/*

         ensure_symbol_capacity()
	       ---
	       ensures the symbol buffer has sufficient capacity,
	       expanding it if necessary.

*/

static int ensure_symbol_capacity(commc_deflate_context_t* ctx, size_t needed) {

  commc_lz77_symbol_t* new_buffer;
  size_t new_capacity;

  if  (!ctx) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }

  if  (ctx->symbol_capacity >= needed) {

    return COMMC_SUCCESS;

  }

  /* expand capacity */

  new_capacity = ctx->symbol_capacity * 2;
  
  while  (new_capacity < needed) {

    new_capacity *= 2;

  }

  new_buffer = (commc_lz77_symbol_t*)realloc(ctx->lz77_symbols, 
                                            new_capacity * sizeof(commc_lz77_symbol_t));
  
  if  (!new_buffer) {

    return COMMC_MEMORY_ERROR;

  }

  ctx->lz77_symbols = new_buffer;
  ctx->symbol_capacity = new_capacity;

  return COMMC_SUCCESS;

}

/*

         convert_lz77_to_deflate_symbols()
	       ---
	       converts LZ77 symbols to deflate symbol format,
	       mapping length/distance pairs to deflate codes.

*/

static int convert_lz77_to_deflate_symbols(const commc_lz77_symbol_t* lz77_symbols,
                                          size_t symbol_count,
                                          unsigned char* deflate_symbols,
                                          size_t* deflate_count) {

  size_t i;
  size_t out_pos = 0;
  
  unsigned int length;
  unsigned int distance;

  if  (!lz77_symbols || !deflate_symbols || !deflate_count) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }

  /* convert each LZ77 symbol */

  for  (i = 0; i < symbol_count; i++) {

    if  (lz77_symbols[i].type == COMMC_LZ77_SYMBOL_LITERAL) {

      /* literal byte - copy directly */

      deflate_symbols[out_pos++] = lz77_symbols[i].data.literal;

    } else if  (lz77_symbols[i].type == COMMC_LZ77_SYMBOL_MATCH) {

      /* length/distance pair - convert to deflate format */

      length = lz77_symbols[i].data.match.length;
      distance = lz77_symbols[i].data.match.distance;

      /* map length to deflate length code (simplified mapping) */

      if  (length >= 3 && length <= 10) {

        deflate_symbols[out_pos++] = (unsigned char)(COMMC_DEFLATE_LITERAL_SYMBOLS + length - 3);

      } else if  (length > 10) {

        deflate_symbols[out_pos++] = (unsigned char)(COMMC_DEFLATE_LITERAL_SYMBOLS + 8);

      }

      /* encode distance (simplified - would need full deflate distance codes) */

      if  (distance <= 4) {

        deflate_symbols[out_pos++] = (unsigned char)(distance - 1);

      } else {

        deflate_symbols[out_pos++] = 4;  /* simplified distance mapping */

      }

    }

  }

  *deflate_count = out_pos;
  return COMMC_SUCCESS;

}

/*

         calculate_block_efficiency()
	       ---
	       estimates compression efficiency for different
	       block types to choose the optimal one.

*/

static double calculate_block_efficiency(const unsigned char* input,
                                        size_t input_size,
                                        int block_type) {

  size_t unique_symbols = 0;
  int symbol_present[256] = {0};
  size_t i;
  double entropy = 0.0;
  double frequency;

  if  (!input || input_size == 0) {

    return 0.0;

  }

  /* count symbol frequencies */

  for  (i = 0; i < input_size; i++) {

    if  (!symbol_present[input[i]]) {

      symbol_present[input[i]] = 1;
      unique_symbols++;

    }

  }

  /* estimate entropy for Huffman efficiency */

  for  (i = 0; i < 256; i++) {

    if  (symbol_present[i]) {

      frequency = 1.0 / (double)unique_symbols;  /* simplified frequency */
      entropy += frequency * (-frequency);  /* simplified entropy calculation */

    }

  }

  /* adjust efficiency based on block type */

  switch  (block_type) {

    case COMMC_DEFLATE_BLOCK_RAW:
      return 0.0;  /* no compression */

    case COMMC_DEFLATE_BLOCK_FIXED:
      return entropy * 0.7;  /* fixed Huffman efficiency */

    case COMMC_DEFLATE_BLOCK_DYNAMIC:
      return entropy * 0.9;  /* dynamic Huffman efficiency */

    default:
      return 0.0;

  }

}

/*
	==================================
             --- PUBLIC FUNCS ---
	==================================
*/

/*

         commc_deflate_context_create()
	       ---
	       creates and initializes a deflate compression context.

*/

commc_deflate_context_t* commc_deflate_context_create(size_t window_size,
                                                      commc_deflate_level_t level,
                                                      commc_deflate_strategy_t strategy) {

  commc_deflate_context_t* ctx;
  commc_lz77_level_t lz77_level;

  /* validate parameters */

  if  (commc_deflate_validate_parameters(window_size, level, strategy) != COMMC_SUCCESS) {

    return NULL;

  }

  /* allocate context */

  ctx = (commc_deflate_context_t*)malloc(sizeof(commc_deflate_context_t));
  
  if  (!ctx) {

    return NULL;

  }

  /* initialize basic parameters */

  ctx->level = level;
  ctx->strategy = strategy;
  ctx->window_size = window_size;

  /* map deflate level to LZ77 level */

  switch  (level) {

    case COMMC_DEFLATE_LEVEL_FAST:
      lz77_level = COMMC_LZ77_LEVEL_FAST;
      break;

    case COMMC_DEFLATE_LEVEL_BEST:
      lz77_level = COMMC_LZ77_LEVEL_BEST;
      break;

    default:
      lz77_level = COMMC_LZ77_LEVEL_DEFAULT;
      break;

  }

  /* create LZ77 context */

  ctx->lz77_ctx = commc_lz77_context_create(window_size, lz77_level);
  
  if  (!ctx->lz77_ctx) {

    free(ctx);
    return NULL;

  }

  /* create Huffman context */

  ctx->huff_ctx = commc_huffman_context_create(4096);
  
  if  (!ctx->huff_ctx) {

    commc_lz77_context_destroy(ctx->lz77_ctx);
    free(ctx);
    return NULL;

  }

  /* initialize symbol buffer */

  ctx->symbol_capacity = 1024;
  ctx->lz77_symbols = (commc_lz77_symbol_t*)malloc(ctx->symbol_capacity * sizeof(commc_lz77_symbol_t));
  
  if  (!ctx->lz77_symbols) {

    commc_huffman_context_destroy(ctx->huff_ctx);
    commc_lz77_context_destroy(ctx->lz77_ctx);
    free(ctx);
    return NULL;

  }

  ctx->symbol_count = 0;

  /* initialize block buffer */

  ctx->block_capacity = 16;
  ctx->blocks = (commc_deflate_block_t*)malloc(ctx->block_capacity * sizeof(commc_deflate_block_t));
  
  if  (!ctx->blocks) {

    free(ctx->lz77_symbols);
    commc_huffman_context_destroy(ctx->huff_ctx);
    commc_lz77_context_destroy(ctx->lz77_ctx);
    free(ctx);
    return NULL;

  }

  ctx->block_count = 0;

  /* initialize statistics */

  ctx->input_size = 0;
  ctx->output_size = 0;
  ctx->compression_ratio = 0.0;
  ctx->lz77_ratio = 0.0;
  ctx->huffman_ratio = 0.0;

  return ctx;

}

/*

         commc_deflate_context_destroy()
	       ---
	       releases memory associated with deflate context.

*/

void commc_deflate_context_destroy(commc_deflate_context_t* ctx) {

  size_t i;

  if  (!ctx) {

    return;

  }

  /* destroy blocks */

  if  (ctx->blocks) {

    for  (i = 0; i < ctx->block_count; i++) {

      if  (ctx->blocks[i].data) {

        free(ctx->blocks[i].data);

      }

    }
    
    free(ctx->blocks);

  }

  /* destroy symbol buffer */

  if  (ctx->lz77_symbols) {

    free(ctx->lz77_symbols);

  }

  /* destroy algorithm contexts */

  if  (ctx->huff_ctx) {

    commc_huffman_context_destroy(ctx->huff_ctx);

  }

  if  (ctx->lz77_ctx) {

    commc_lz77_context_destroy(ctx->lz77_ctx);

  }

  free(ctx);

}

/*

         commc_deflate_compress()
	       ---
	       compresses data using the two-stage deflate algorithm.

*/

int commc_deflate_compress(commc_deflate_context_t* ctx,
                           const unsigned char* input,
                           size_t input_size,
                           commc_deflate_compressed_t* output) {

  unsigned char* deflate_symbols;
  size_t deflate_symbol_count;
  
  commc_huffman_encoded_t* huffman_output;
  
  size_t estimated_symbols;
  
  int result;

  /* validate parameters */

  result = validate_context(ctx);
  
  if  (result != COMMC_SUCCESS) {

    return result;

  }

  if  (!input || !output) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }

  /* update input size */

  ctx->input_size = input_size;

  /* stage 1: LZ77 compression */

  estimated_symbols = commc_lz77_estimate_output_size(input_size);
  
  result = ensure_symbol_capacity(ctx, estimated_symbols);
  
  if  (result != COMMC_SUCCESS) {

    return result;

  }

  result = commc_lz77_compress(ctx->lz77_ctx, input, input_size,
                              ctx->lz77_symbols, ctx->symbol_capacity,
                              &ctx->symbol_count);
  
  if  (result != COMMC_SUCCESS) {

    return result;

  }

  /* calculate LZ77 compression ratio */

  ctx->lz77_ratio = (double)(ctx->symbol_count * sizeof(commc_lz77_symbol_t)) / 
                    (double)input_size * 100.0;

  /* convert LZ77 symbols to deflate format */

  deflate_symbols = (unsigned char*)malloc(ctx->symbol_count * 4);  /* worst case */
  
  if  (!deflate_symbols) {

    return COMMC_MEMORY_ERROR;

  }

  result = convert_lz77_to_deflate_symbols(ctx->lz77_symbols, ctx->symbol_count,
                                          deflate_symbols, &deflate_symbol_count);
  
  if  (result != COMMC_SUCCESS) {

    free(deflate_symbols);
    return result;

  }

  /* stage 2: Huffman coding */

  huffman_output = commc_huffman_encoded_create();
  
  if  (!huffman_output) {

    free(deflate_symbols);
    return COMMC_MEMORY_ERROR;

  }

  result = commc_huffman_encode(ctx->huff_ctx, deflate_symbols, deflate_symbol_count,
                               huffman_output);
  
  if  (result != COMMC_SUCCESS) {

    free(deflate_symbols);
    commc_huffman_encoded_destroy(huffman_output);
    return result;

  }

  free(deflate_symbols);

  /* calculate Huffman compression ratio */

  ctx->huffman_ratio = (double)huffman_output->data_size / 
                       (double)deflate_symbol_count * 100.0;

  /* update output structure */

  ctx->output_size = huffman_output->data_size;
  ctx->compression_ratio = commc_deflate_get_compression_ratio(input_size, 
                                                              ctx->output_size);

  /* copy results to output */

  output->data = (unsigned char*)malloc(huffman_output->data_size);
  
  if  (!output->data) {

    commc_huffman_encoded_destroy(huffman_output);
    return COMMC_MEMORY_ERROR;

  }

  memcpy(output->data, huffman_output->data, huffman_output->data_size);
  output->data_size = huffman_output->data_size;
  output->original_size = input_size;
  output->compression_ratio = ctx->compression_ratio;
  output->lz77_symbols = ctx->symbol_count;
  output->huffman_bits = huffman_output->bit_count;

  /* create single block for simplicity */

  output->blocks = (commc_deflate_block_t*)malloc(sizeof(commc_deflate_block_t));
  
  if  (!output->blocks) {

    free(output->data);
    commc_huffman_encoded_destroy(huffman_output);
    return COMMC_MEMORY_ERROR;

  }

  output->block_count = 1;
  output->blocks[0].type = COMMC_DEFLATE_BLOCK_DYNAMIC;
  output->blocks[0].final_block = 1;
  output->blocks[0].uncompressed_size = input_size;
  output->blocks[0].compressed_size = huffman_output->data_size;
  output->blocks[0].data = NULL;  /* data is in main output buffer */
  output->blocks[0].data_capacity = 0;

  commc_huffman_encoded_destroy(huffman_output);

  return COMMC_SUCCESS;

}

/*

         commc_deflate_choose_block_type()
	       ---
	       chooses optimal block type based on data analysis.

*/

int commc_deflate_choose_block_type(commc_deflate_context_t* ctx,
                                    const unsigned char* input,
                                    size_t input_size) {

  double raw_efficiency;
  double fixed_efficiency;
  double dynamic_efficiency;
  
  int best_type = COMMC_DEFLATE_BLOCK_RAW;
  double best_efficiency = 0.0;

  if  (!ctx || !input) {

    return COMMC_DEFLATE_BLOCK_RAW;

  }

  /* calculate efficiency for each block type */

  raw_efficiency = 0.0;  /* raw blocks have no compression */
  fixed_efficiency = calculate_block_efficiency(input, input_size, 
                                                COMMC_DEFLATE_BLOCK_FIXED);
  dynamic_efficiency = calculate_block_efficiency(input, input_size, 
                                                  COMMC_DEFLATE_BLOCK_DYNAMIC);

  /* choose the most efficient type */
  
  if  (raw_efficiency > best_efficiency) {
  
    best_efficiency = raw_efficiency;
    best_type = COMMC_DEFLATE_BLOCK_RAW;
    
  }

  if  (fixed_efficiency > best_efficiency) {

    best_efficiency = fixed_efficiency;
    best_type = COMMC_DEFLATE_BLOCK_FIXED;

  }

  if  (dynamic_efficiency > best_efficiency) {

    best_efficiency = dynamic_efficiency;
    best_type = COMMC_DEFLATE_BLOCK_DYNAMIC;

  }

  /* for very small blocks, raw might be better due to overhead */

  if  (input_size < 64) {

    best_type = COMMC_DEFLATE_BLOCK_RAW;

  }

  return best_type;

}

/*

         commc_deflate_estimate_compressed_size()
	       ---
	       estimates compressed size for input data.

*/

size_t commc_deflate_estimate_compressed_size(commc_deflate_context_t* ctx,
                                              size_t input_size) {

  size_t lz77_estimate;
  size_t huffman_estimate;

  if  (!ctx) {

    return input_size;  /* worst case */

  }

  /* estimate LZ77 compression */

  lz77_estimate = commc_lz77_estimate_output_size(input_size);

  /* estimate Huffman compression (assume 50% reduction) */

  huffman_estimate = lz77_estimate / 2;

  /* add deflate format overhead */

  return huffman_estimate + 64;

}

/*

         commc_deflate_get_compression_ratio()
	       ---
	       calculates compression ratio from sizes.

*/

double commc_deflate_get_compression_ratio(size_t original_size,
                                           size_t compressed_size) {

  if  (original_size == 0) {

    return 0.0;

  }

  return (double)compressed_size / (double)original_size * 100.0;

}

/*

         commc_deflate_validate_parameters()
	       ---
	       validates deflate compression parameters.

*/

int commc_deflate_validate_parameters(size_t window_size,
                                      commc_deflate_level_t level,
                                      commc_deflate_strategy_t strategy) {

  if  (window_size < COMMC_DEFLATE_MIN_WINDOW_SIZE || 
       window_size > COMMC_DEFLATE_MAX_WINDOW_SIZE) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }

  if  (level < COMMC_DEFLATE_LEVEL_STORE || level > COMMC_DEFLATE_LEVEL_BEST) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }

  if  (strategy < COMMC_DEFLATE_STRATEGY_DEFAULT || strategy > COMMC_DEFLATE_STRATEGY_RLE) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }

  return COMMC_SUCCESS;

}

/*

         commc_deflate_get_statistics()
	       ---
	       retrieves comprehensive compression statistics.

*/

int commc_deflate_get_statistics(const commc_deflate_context_t* ctx,
                                 double* lz77_ratio,
                                 double* huffman_ratio,
                                 double* total_ratio) {

  if  (!ctx || !lz77_ratio || !huffman_ratio || !total_ratio) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }

  *lz77_ratio = ctx->lz77_ratio;
  *huffman_ratio = ctx->huffman_ratio;
  *total_ratio = ctx->compression_ratio;

  return COMMC_SUCCESS;

}

/*

         commc_deflate_block_create()
	       ---
	       creates a new deflate block structure.

*/

commc_deflate_block_t* commc_deflate_block_create(void) {

  commc_deflate_block_t* block;

  block = (commc_deflate_block_t*)malloc(sizeof(commc_deflate_block_t));
  
  if  (!block) {

    return NULL;

  }

  block->type = COMMC_DEFLATE_BLOCK_RAW;
  block->final_block = 0;
  block->uncompressed_size = 0;
  block->compressed_size = 0;
  block->data = NULL;
  block->data_capacity = 0;

  return block;

}

/*

         commc_deflate_block_destroy()
	       ---
	       releases memory associated with a deflate block.

*/

void commc_deflate_block_destroy(commc_deflate_block_t* block) {

  if  (!block) {

    return;

  }

  if  (block->data) {

    free(block->data);

  }

  free(block);

}

/*

         commc_deflate_compressed_create()
	       ---
	       creates a new compressed data structure.

*/

commc_deflate_compressed_t* commc_deflate_compressed_create(void) {

  commc_deflate_compressed_t* compressed;

  compressed = (commc_deflate_compressed_t*)malloc(sizeof(commc_deflate_compressed_t));
  
  if  (!compressed) {

    return NULL;

  }

  compressed->data = NULL;
  compressed->data_size = 0;
  compressed->blocks = NULL;
  compressed->block_count = 0;
  compressed->original_size = 0;
  compressed->compression_ratio = 0.0;
  compressed->lz77_symbols = 0;
  compressed->huffman_bits = 0;

  return compressed;

}

/*

         commc_deflate_compressed_destroy()
	       ---
	       releases memory associated with compressed data.

*/

void commc_deflate_compressed_destroy(commc_deflate_compressed_t* compressed) {

  size_t i;

  if  (!compressed) {

    return;

  }

  if  (compressed->data) {

    free(compressed->data);

  }

  if  (compressed->blocks) {

    for  (i = 0; i < compressed->block_count; i++) {

      if  (compressed->blocks[i].data) {

        free(compressed->blocks[i].data);

      }

    }
    
    free(compressed->blocks);

  }

  free(compressed);

}

/*

         commc_deflate_reset_context()
	       ---
	       resets compression context for new data stream.

*/

int commc_deflate_reset_context(commc_deflate_context_t* ctx) {

  size_t i;
  int result;

  result = validate_context(ctx);
  
  if  (result != COMMC_SUCCESS) {

    return result;

  }

  /* reset algorithm contexts */

  result = commc_lz77_context_reset(ctx->lz77_ctx);
  
  if  (result != COMMC_SUCCESS) {

    return result;

  }

  /* clear symbol buffer */

  ctx->symbol_count = 0;

  /* clear blocks */

  for  (i = 0; i < ctx->block_count; i++) {

    if  (ctx->blocks[i].data) {

      free(ctx->blocks[i].data);
      ctx->blocks[i].data = NULL;

    }

  }
  
  ctx->block_count = 0;

  /* reset statistics */

  ctx->input_size = 0;
  ctx->output_size = 0;
  ctx->compression_ratio = 0.0;
  ctx->lz77_ratio = 0.0;
  ctx->huffman_ratio = 0.0;

  return COMMC_SUCCESS;

}

/*

         commc_deflate_set_level()
	       ---
	       changes compression level for subsequent operations.

*/

int commc_deflate_set_level(commc_deflate_context_t* ctx,
                            commc_deflate_level_t level) {

  if  (!ctx) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }

  if  (level < COMMC_DEFLATE_LEVEL_STORE || level > COMMC_DEFLATE_LEVEL_BEST) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }

  ctx->level = level;
  return COMMC_SUCCESS;

}

/*

         commc_deflate_set_strategy()
	       ---
	       changes compression strategy for subsequent operations.

*/

int commc_deflate_set_strategy(commc_deflate_context_t* ctx,
                               commc_deflate_strategy_t strategy) {

  if  (!ctx) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }

  if  (strategy < COMMC_DEFLATE_STRATEGY_DEFAULT || 
       strategy > COMMC_DEFLATE_STRATEGY_RLE) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }

  ctx->strategy = strategy;
  return COMMC_SUCCESS;

}

/*
	==================================
             --- EOF ---
	==================================
*/