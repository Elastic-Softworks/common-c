/*
   ===================================
   C O M M O N - C
   HUFFMAN CODING IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- HUFFMAN MODULE ---

    implementation of Huffman coding compression with
    frequency analysis, tree construction, and optimal
    variable-length encoding. provides both compression
    and decompression capabilities.

    see include/commc/huffman.h for function prototypes
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

#include  "commc/huffman.h"
#include  "commc/error.h"

/*
	==================================
             --- STATIC HELPERS ---
	==================================
*/

/*

         compare_nodes()
	       ---
	       comparison function for sorting nodes by frequency
	       used in priority queue for tree construction.

*/

static int compare_nodes(const void* a, const void* b) {

  const commc_huffman_node_t* node_a = *(const commc_huffman_node_t**)a;
  const commc_huffman_node_t* node_b = *(const commc_huffman_node_t**)b;

  if  (node_a->frequency < node_b->frequency) {

    return -1;

  } else if  (node_a->frequency > node_b->frequency) {

    return 1;

  }

  return 0;

}

/*

         generate_codes_recursive()
	       ---
	       recursively generates Huffman codes by traversing
	       the tree and building bit patterns.

*/

static void generate_codes_recursive(commc_huffman_node_t* node,
                                     commc_huffman_code_t* codes,
                                     unsigned int code,
                                     int depth) {

  if  (!node) {

    return;

  }

  /* leaf node - store the code */

  if  (node->symbol >= 0) {

    codes[node->symbol].bits = code;
    codes[node->symbol].length = depth;
    return;

  }

  /* internal node - recurse with extended codes */

  if  (node->left) {

    generate_codes_recursive(node->left, codes, code << 1, depth + 1);

  }

  if  (node->right) {

    generate_codes_recursive(node->right, codes, (code << 1) | 1, depth + 1);

  }

}

/*

         calculate_tree_depth()
	       ---
	       recursively calculates the maximum depth of the tree.

*/

static int calculate_tree_depth(const commc_huffman_node_t* node) {

  int left_depth;
  int right_depth;

  if  (!node) {

    return 0;

  }

  if  (node->symbol >= 0) {

    return 1;  /* leaf node */

  }

  left_depth = calculate_tree_depth(node->left);
  right_depth = calculate_tree_depth(node->right);

  return 1 + (left_depth > right_depth ? left_depth : right_depth);

}

/*

         ensure_buffer_space()
	       ---
	       ensures the bit buffer has enough space for
	       additional data, expanding if necessary.

*/

static int ensure_buffer_space(commc_huffman_context_t* ctx, size_t needed_bytes) {

  unsigned char* new_buffer;
  size_t new_size;

  if  (!ctx) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }

  if  (ctx->buffer_pos + needed_bytes <= ctx->buffer_size) {

    return COMMC_SUCCESS;  /* sufficient space */

  }

  /* expand buffer */

  new_size = ctx->buffer_size * 2;
  
  while  (new_size < ctx->buffer_pos + needed_bytes) {

    new_size *= 2;

  }

  new_buffer = (unsigned char*)realloc(ctx->bit_buffer, new_size);
  
  if  (!new_buffer) {

    return COMMC_MEMORY_ERROR;

  }

  ctx->bit_buffer = new_buffer;
  ctx->buffer_size = new_size;

  return COMMC_SUCCESS;

}

/*
	==================================
             --- PUBLIC FUNCS ---
	==================================
*/

/*

         commc_huffman_context_create()
	       ---
	       creates and initializes a new Huffman context.

*/

commc_huffman_context_t* commc_huffman_context_create(size_t buffer_size) {

  commc_huffman_context_t* ctx;

  if  (buffer_size == 0) {

    buffer_size = 4096;  /* default buffer size */

  }

  ctx = (commc_huffman_context_t*)malloc(sizeof(commc_huffman_context_t));
  
  if  (!ctx) {

    return NULL;

  }

  /* initialize basic fields */

  ctx->tree = NULL;
  ctx->buffer_size = buffer_size;
  ctx->buffer_pos = 0;
  ctx->bit_pos = 0;
  ctx->input_size = 0;
  ctx->output_size = 0;
  ctx->compression_ratio = 0.0;

  /* allocate bit buffer */

  ctx->bit_buffer = (unsigned char*)malloc(buffer_size);
  
  if  (!ctx->bit_buffer) {

    free(ctx);
    return NULL;

  }

  memset(ctx->bit_buffer, 0, buffer_size);

  return ctx;

}

/*

         commc_huffman_context_destroy()
	       ---
	       releases memory associated with a Huffman context.

*/

void commc_huffman_context_destroy(commc_huffman_context_t* ctx) {

  if  (!ctx) {

    return;

  }

  if  (ctx->tree) {

    commc_huffman_tree_destroy(ctx->tree);

  }

  if  (ctx->bit_buffer) {

    free(ctx->bit_buffer);

  }

  free(ctx);

}

/*

         commc_huffman_tree_create()
	       ---
	       creates a new empty Huffman tree.

*/

commc_huffman_tree_t* commc_huffman_tree_create(void) {

  commc_huffman_tree_t* tree;
  int i;

  tree = (commc_huffman_tree_t*)malloc(sizeof(commc_huffman_tree_t));
  
  if  (!tree) {

    return NULL;

  }

  /* initialize tree fields */

  tree->root = NULL;
  tree->num_symbols = 0;
  tree->total_symbols = 0;

  /* clear frequency and code tables */

  for  (i = 0; i < COMMC_HUFFMAN_ALPHABET_SIZE; i++) {

    tree->frequencies[i] = 0;
    tree->codes[i].bits = 0;
    tree->codes[i].length = 0;

  }

  return tree;

}

/*

         commc_huffman_tree_destroy()
	       ---
	       releases memory associated with a Huffman tree.

*/

void commc_huffman_tree_destroy(commc_huffman_tree_t* tree) {

  if  (!tree) {

    return;

  }

  if  (tree->root) {

    commc_huffman_node_destroy(tree->root);

  }

  free(tree);

}

/*

         commc_huffman_node_create()
	       ---
	       creates a new Huffman tree node.

*/

commc_huffman_node_t* commc_huffman_node_create(int symbol, unsigned int frequency) {

  commc_huffman_node_t* node;

  node = (commc_huffman_node_t*)malloc(sizeof(commc_huffman_node_t));
  
  if  (!node) {

    return NULL;

  }

  node->symbol = symbol;
  node->frequency = frequency;
  node->left = NULL;
  node->right = NULL;
  node->parent = NULL;

  return node;

}

/*

         commc_huffman_node_destroy()
	       ---
	       recursively destroys a Huffman tree node.

*/

void commc_huffman_node_destroy(commc_huffman_node_t* node) {

  if  (!node) {

    return;

  }

  commc_huffman_node_destroy(node->left);
  commc_huffman_node_destroy(node->right);
  
  free(node);

}

/*

         commc_huffman_analyze_frequencies()
	       ---
	       analyzes input data to count symbol frequencies.

*/

int commc_huffman_analyze_frequencies(commc_huffman_tree_t* tree,
                                      const unsigned char* data,
                                      size_t size) {

  size_t i;
  int symbol;

  if  (!tree || !data) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }

  /* clear existing frequency data */

  tree->num_symbols = 0;
  tree->total_symbols = size;

  for  (i = 0; i < COMMC_HUFFMAN_ALPHABET_SIZE; i++) {

    tree->frequencies[i] = 0;

  }

  /* count frequencies */

  for  (i = 0; i < size; i++) {

    symbol = (int)data[i];
    
    if  (tree->frequencies[symbol] == 0) {

      tree->num_symbols++;

    }
    
    tree->frequencies[symbol]++;

  }

  return COMMC_SUCCESS;

}

/*

         commc_huffman_build_tree()
	       ---
	       constructs the Huffman tree using priority queue algorithm.

*/

int commc_huffman_build_tree(commc_huffman_tree_t* tree) {

  commc_huffman_node_t** nodes;
  commc_huffman_node_t* left;
  commc_huffman_node_t* right;
  commc_huffman_node_t* internal;
  
  int num_nodes = 0;
  int i;

  if  (!tree || tree->num_symbols == 0) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }

  /* special case: single symbol */

  if  (tree->num_symbols == 1) {

    for  (i = 0; i < COMMC_HUFFMAN_ALPHABET_SIZE; i++) {

      if  (tree->frequencies[i] > 0) {

        tree->root = commc_huffman_node_create(i, tree->frequencies[i]);
        
        if  (!tree->root) {

          return COMMC_MEMORY_ERROR;

        }
        
        break;

      }

    }
    
    return COMMC_SUCCESS;

  }

  /* allocate node array for priority queue */

  nodes = (commc_huffman_node_t**)malloc(tree->num_symbols * sizeof(commc_huffman_node_t*));
  
  if  (!nodes) {

    return COMMC_MEMORY_ERROR;

  }

  /* create leaf nodes for each symbol */

  for  (i = 0; i < COMMC_HUFFMAN_ALPHABET_SIZE; i++) {

    if  (tree->frequencies[i] > 0) {

      nodes[num_nodes] = commc_huffman_node_create(i, tree->frequencies[i]);
      
      if  (!nodes[num_nodes]) {

        /* cleanup on failure */
        
        while  (num_nodes > 0) {

          commc_huffman_node_destroy(nodes[--num_nodes]);

        }
        
        free(nodes);
        return COMMC_MEMORY_ERROR;

      }
      
      num_nodes++;

    }

  }

  /* build tree using priority queue */

  while  (num_nodes > 1) {

    /* sort nodes by frequency */

    qsort(nodes, (size_t)num_nodes, sizeof(commc_huffman_node_t*), compare_nodes);

    /* take two nodes with lowest frequency */

    left = nodes[0];
    right = nodes[1];

    /* create internal node */

    internal = commc_huffman_node_create(-1, left->frequency + right->frequency);
    
    if  (!internal) {

      /* cleanup on failure */
      
      for  (i = 0; i < num_nodes; i++) {

        commc_huffman_node_destroy(nodes[i]);

      }
      
      free(nodes);
      return COMMC_MEMORY_ERROR;

    }

    internal->left = left;
    internal->right = right;
    left->parent = internal;
    right->parent = internal;

    /* replace first two nodes with internal node */

    nodes[0] = internal;
    
    for  (i = 1; i < num_nodes - 1; i++) {

      nodes[i] = nodes[i + 1];

    }
    
    num_nodes--;

  }

  /* set root and cleanup */

  tree->root = nodes[0];
  free(nodes);

  return COMMC_SUCCESS;

}

/*

         commc_huffman_generate_codes()
	       ---
	       generates Huffman codes for all symbols.

*/

int commc_huffman_generate_codes(commc_huffman_tree_t* tree) {

  int i;

  if  (!tree || !tree->root) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }

  /* clear existing codes */

  for  (i = 0; i < COMMC_HUFFMAN_ALPHABET_SIZE; i++) {

    tree->codes[i].bits = 0;
    tree->codes[i].length = 0;

  }

  /* special case: single symbol gets 1-bit code */

  if  (tree->num_symbols == 1) {

    for  (i = 0; i < COMMC_HUFFMAN_ALPHABET_SIZE; i++) {

      if  (tree->frequencies[i] > 0) {

        tree->codes[i].bits = 0;
        tree->codes[i].length = 1;
        break;

      }

    }
    
    return COMMC_SUCCESS;

  }

  /* generate codes by tree traversal */

  generate_codes_recursive(tree->root, tree->codes, 0, 0);

  return COMMC_SUCCESS;

}

/*

         commc_huffman_write_bit()
	       ---
	       writes a single bit to the encoding buffer.

*/

int commc_huffman_write_bit(commc_huffman_context_t* ctx, int bit) {

  int result;

  if  (!ctx) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }

  /* ensure buffer space */

  result = ensure_buffer_space(ctx, 1);
  
  if  (result != COMMC_SUCCESS) {

    return result;

  }

  /* write bit to current position */

  if  (bit) {

    ctx->bit_buffer[ctx->buffer_pos] |= (1 << (7 - ctx->bit_pos));

  }

  ctx->bit_pos++;

  /* advance to next byte if needed */

  if  (ctx->bit_pos >= COMMC_HUFFMAN_BITS_PER_BYTE) {

    ctx->bit_pos = 0;
    ctx->buffer_pos++;

  }

  return COMMC_SUCCESS;

}

/*

         commc_huffman_write_bits()
	       ---
	       writes multiple bits to the encoding buffer.

*/

int commc_huffman_write_bits(commc_huffman_context_t* ctx, 
                             unsigned int bits, 
                             int count) {

  int i;
  int bit;
  int result;

  if  (!ctx || count < 0) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }

  /* write bits from most significant to least significant */

  for  (i = count - 1; i >= 0; i--) {

    bit = (bits >> i) & 1;
    
    result = commc_huffman_write_bit(ctx, bit);
    
    if  (result != COMMC_SUCCESS) {

      return result;

    }

  }

  return COMMC_SUCCESS;

}

/*

         commc_huffman_flush_bits()
	       ---
	       flushes remaining bits by padding with zeros.

*/

int commc_huffman_flush_bits(commc_huffman_context_t* ctx) {

  if  (!ctx) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }

  /* advance to byte boundary if needed */

  if  (ctx->bit_pos > 0) {

    ctx->buffer_pos++;
    ctx->bit_pos = 0;

  }

  return COMMC_SUCCESS;

}

/*

         commc_huffman_encode()
	       ---
	       encodes input data using Huffman coding.

*/

int commc_huffman_encode(commc_huffman_context_t* ctx,
                         const unsigned char* input,
                         size_t input_size,
                         commc_huffman_encoded_t* output) {

  size_t i;
  int symbol;
  int result;

  if  (!ctx || !input || !output) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }

  /* create and initialize tree */

  ctx->tree = commc_huffman_tree_create();
  
  if  (!ctx->tree) {

    return COMMC_MEMORY_ERROR;

  }

  /* analyze frequencies */

  result = commc_huffman_analyze_frequencies(ctx->tree, input, input_size);
  
  if  (result != COMMC_SUCCESS) {

    return result;

  }

  /* build tree */

  result = commc_huffman_build_tree(ctx->tree);
  
  if  (result != COMMC_SUCCESS) {

    return result;

  }

  /* generate codes */

  result = commc_huffman_generate_codes(ctx->tree);
  
  if  (result != COMMC_SUCCESS) {

    return result;

  }

  /* reset buffer position */

  ctx->buffer_pos = 0;
  ctx->bit_pos = 0;
  memset(ctx->bit_buffer, 0, ctx->buffer_size);

  /* encode data */

  for  (i = 0; i < input_size; i++) {

    symbol = (int)input[i];
    
    result = commc_huffman_write_bits(ctx, 
                                     ctx->tree->codes[symbol].bits,
                                     ctx->tree->codes[symbol].length);
    
    if  (result != COMMC_SUCCESS) {

      return result;

    }

  }

  /* flush remaining bits */

  result = commc_huffman_flush_bits(ctx);
  
  if  (result != COMMC_SUCCESS) {

    return result;

  }

  /* copy results to output structure */

  output->data = (unsigned char*)malloc(ctx->buffer_pos);
  
  if  (!output->data) {

    return COMMC_MEMORY_ERROR;

  }

  memcpy(output->data, ctx->bit_buffer, ctx->buffer_pos);
  output->data_size = ctx->buffer_pos;
  output->bit_count = ctx->buffer_pos * 8 - (8 - ctx->bit_pos);
  output->tree = ctx->tree;  /* transfer ownership */
  
  ctx->tree = NULL;  /* prevent double-free */

  /* update statistics */

  ctx->input_size = input_size;
  ctx->output_size = ctx->buffer_pos;
  ctx->compression_ratio = commc_huffman_get_compression_ratio(input_size, ctx->buffer_pos);

  return COMMC_SUCCESS;

}

/*

         commc_huffman_get_compression_ratio()
	       ---
	       calculates compression ratio from sizes.

*/

double commc_huffman_get_compression_ratio(size_t input_size, 
                                           size_t output_size) {

  if  (input_size == 0) {

    return 0.0;

  }

  return (double)output_size / (double)input_size * 100.0;

}

/*

         commc_huffman_estimate_output_size()
	       ---
	       estimates maximum output size for encoding.

*/

size_t commc_huffman_estimate_output_size(size_t input_size) {

  /* worst case: each byte needs maximum bits plus tree overhead */

  return input_size * COMMC_HUFFMAN_MAX_CODE_LENGTH / 8 + 1024;

}

/*

         commc_huffman_validate_tree()
	       ---
	       validates Huffman tree structure.

*/

int commc_huffman_validate_tree(const commc_huffman_tree_t* tree) {

  int i;
  int valid_codes = 0;

  if  (!tree || !tree->root) {

    return COMMC_ERROR_INVALID_ARGUMENT;

  }

  /* check that symbols have valid codes */

  for  (i = 0; i < COMMC_HUFFMAN_ALPHABET_SIZE; i++) {

    if  (tree->frequencies[i] > 0) {

      if  (tree->codes[i].length <= 0 || 
           tree->codes[i].length > COMMC_HUFFMAN_MAX_CODE_LENGTH) {

        return COMMC_ERROR_INVALID_DATA;

      }
      
      valid_codes++;

    }

  }

  if  (valid_codes != tree->num_symbols) {

    return COMMC_ERROR_INVALID_DATA;

  }

  return COMMC_SUCCESS;

}

/*

         commc_huffman_get_tree_depth()
	       ---
	       calculates maximum depth of Huffman tree.

*/

int commc_huffman_get_tree_depth(const commc_huffman_tree_t* tree) {

  if  (!tree || !tree->root) {

    return 0;

  }

  return calculate_tree_depth(tree->root);

}

/*

         commc_huffman_encoded_create()
	       ---
	       creates a new encoded data structure.

*/

commc_huffman_encoded_t* commc_huffman_encoded_create(void) {

  commc_huffman_encoded_t* encoded;

  encoded = (commc_huffman_encoded_t*)malloc(sizeof(commc_huffman_encoded_t));
  
  if  (!encoded) {

    return NULL;

  }

  encoded->data = NULL;
  encoded->data_size = 0;
  encoded->bit_count = 0;
  encoded->tree = NULL;

  return encoded;

}

/*

         commc_huffman_encoded_destroy()
	       ---
	       releases memory associated with encoded data.

*/

void commc_huffman_encoded_destroy(commc_huffman_encoded_t* encoded) {

  if  (!encoded) {

    return;

  }

  if  (encoded->data) {

    free(encoded->data);

  }

  if  (encoded->tree) {

    commc_huffman_tree_destroy(encoded->tree);

  }

  free(encoded);

}

/*

         commc_huffman_print_codes()
	       ---
	       prints Huffman codes for debugging.

*/

void commc_huffman_print_codes(const commc_huffman_tree_t* tree) {

  int i;
  int j;

  if  (!tree) {

    return;

  }

  printf("OUTPUT: HUFFMAN CODES...\n");

  for  (i = 0; i < COMMC_HUFFMAN_ALPHABET_SIZE; i++) {

    if  (tree->frequencies[i] > 0) {

      printf("OUTPUT: SYMBOL %3d (", i);
      
      if  (i >= 32 && i <= 126) {

        printf("'%c'", i);

      } else {

        printf("0x%02X", i);

      }
      
      printf("): ");

      /* print binary code */

      for  (j = tree->codes[i].length - 1; j >= 0; j--) {

        printf("%d", (tree->codes[i].bits >> j) & 1);

      }
      
      printf(" (freq: %u)\n", tree->frequencies[i]);

    }

  }

}

/*
	==================================
             --- EOF ---
	==================================
*/