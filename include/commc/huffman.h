/*
   ===================================
   C O M M O N - C
   HUFFMAN CODING COMPRESSION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- HUFFMAN MODULE ---

    implementation of Huffman coding compression with
    frequency analysis, tree construction, and optimal
    variable-length encoding for text and binary data.

    Huffman coding creates an optimal prefix-free code
    where more frequent symbols get shorter bit sequences,
    achieving excellent compression ratios for data with
    non-uniform symbol distributions.

*/

#ifndef COMMC_HUFFMAN_H
#define COMMC_HUFFMAN_H

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

/* alphabet and tree configuration */

#define COMMC_HUFFMAN_ALPHABET_SIZE     256      /* 8-bit symbol alphabet */
#define COMMC_HUFFMAN_MAX_CODE_LENGTH   32       /* maximum code bit length */
#define COMMC_HUFFMAN_EOF_SYMBOL        256      /* end-of-file marker symbol */

/* compression limits */

#define COMMC_HUFFMAN_MIN_FREQUENCY     1        /* minimum symbol frequency */
#define COMMC_HUFFMAN_MAX_NODES         512      /* maximum tree nodes */

/* bit manipulation constants */

#define COMMC_HUFFMAN_BITS_PER_BYTE     8        /* bits in a byte */
#define COMMC_HUFFMAN_BYTE_MASK         0xFF     /* byte mask for bit ops */

/*
	==================================
             --- DATA TYPES ---
	==================================
*/

/*

         commc_huffman_code_t
	       ---
	       represents a Huffman code for a single symbol,
	       containing the bit sequence and its length.

*/

typedef struct {

  unsigned int bits;           /* bit sequence for this symbol */
  int length;                  /* number of bits in the code */

} commc_huffman_code_t;

/*

         commc_huffman_node_t
	       ---
	       represents a node in the Huffman tree, either
	       a leaf containing a symbol or an internal node
	       with left and right children.

*/

typedef struct commc_huffman_node {

  unsigned int frequency;              /* frequency of this node */
  int symbol;                          /* symbol (-1 for internal nodes) */
  
  struct commc_huffman_node* left;     /* left child node */
  struct commc_huffman_node* right;    /* right child node */
  struct commc_huffman_node* parent;   /* parent node for tree traversal */

} commc_huffman_node_t;

/*

         commc_huffman_tree_t
	       ---
	       represents the complete Huffman coding tree
	       with root node and symbol-to-code mapping.

*/

typedef struct {

  commc_huffman_node_t* root;                              /* root of the tree */
  commc_huffman_code_t codes[COMMC_HUFFMAN_ALPHABET_SIZE]; /* symbol codes */
  unsigned int frequencies[COMMC_HUFFMAN_ALPHABET_SIZE];   /* symbol frequencies */
  
  int num_symbols;                                         /* number of unique symbols */
  size_t total_symbols;                                    /* total symbol count */

} commc_huffman_tree_t;

/*

         commc_huffman_context_t
	       ---
	       compression/decompression context maintaining
	       tree state and encoding/decoding buffers.

*/

typedef struct {

  commc_huffman_tree_t* tree;     /* Huffman coding tree */
  
  /* bit buffer for encoding/decoding */
  
  unsigned char* bit_buffer;      /* buffer for bit-level operations */
  size_t buffer_size;             /* size of bit buffer */
  size_t buffer_pos;              /* current position in buffer */
  int bit_pos;                    /* current bit position (0-7) */
  
  /* statistics and configuration */
  
  size_t input_size;              /* size of input data */
  size_t output_size;             /* size of compressed output */
  double compression_ratio;       /* achieved compression ratio */

} commc_huffman_context_t;

/*

         commc_huffman_encoded_t
	       ---
	       represents the output of Huffman encoding,
	       containing the encoded bit stream and tree.

*/

typedef struct {

  unsigned char* data;            /* encoded bit stream */
  size_t data_size;               /* size of encoded data in bytes */
  size_t bit_count;               /* total number of bits used */
  
  commc_huffman_tree_t* tree;     /* tree needed for decoding */

} commc_huffman_encoded_t;

/*
	==================================
             --- PROTOTYPES ---
	==================================
*/

/*

         commc_huffman_context_create()
	       ---
	       creates and initializes a new Huffman coding
	       context with the specified buffer size.
	       
	       parameters:
	         buffer_size - size of internal bit buffer
	       
	       returns:
	         pointer to initialized context, or NULL on failure

*/

commc_huffman_context_t* commc_huffman_context_create(size_t buffer_size);

/*

         commc_huffman_context_destroy()
	       ---
	       releases all memory associated with a Huffman
	       coding context.
	       
	       parameters:
	         ctx - pointer to context to destroy

*/

void commc_huffman_context_destroy(commc_huffman_context_t* ctx);

/*

         commc_huffman_tree_create()
	       ---
	       creates a new empty Huffman tree structure.
	       
	       returns:
	         pointer to initialized tree, or NULL on failure

*/

commc_huffman_tree_t* commc_huffman_tree_create(void);

/*

         commc_huffman_tree_destroy()
	       ---
	       releases all memory associated with a Huffman tree.
	       
	       parameters:
	         tree - pointer to tree to destroy

*/

void commc_huffman_tree_destroy(commc_huffman_tree_t* tree);

/*

         commc_huffman_analyze_frequencies()
	       ---
	       analyzes input data to count symbol frequencies
	       and stores results in the tree structure.
	       
	       parameters:
	         tree - tree to store frequency data
	         data - input data to analyze
	         size - size of input data
	       
	       returns:
	         COMMC_SUCCESS or error code

*/

int commc_huffman_analyze_frequencies(commc_huffman_tree_t* tree,
                                      const unsigned char* data,
                                      size_t size);

/*

         commc_huffman_build_tree()
	       ---
	       constructs the Huffman tree from frequency data
	       using the standard greedy algorithm.
	       
	       parameters:
	         tree - tree containing frequency data
	       
	       returns:
	         COMMC_SUCCESS or error code

*/

int commc_huffman_build_tree(commc_huffman_tree_t* tree);

/*

         commc_huffman_generate_codes()
	       ---
	       generates Huffman codes for all symbols by
	       traversing the tree and building bit sequences.
	       
	       parameters:
	         tree - tree to generate codes for
	       
	       returns:
	         COMMC_SUCCESS or error code

*/

int commc_huffman_generate_codes(commc_huffman_tree_t* tree);

/*

         commc_huffman_encode()
	       ---
	       encodes input data using Huffman coding,
	       producing a compressed bit stream.
	       
	       parameters:
	         ctx - compression context
	         input - input data to compress
	         input_size - size of input data
	         output - pointer to store encoded result
	       
	       returns:
	         COMMC_SUCCESS or error code

*/

int commc_huffman_encode(commc_huffman_context_t* ctx,
                         const unsigned char* input,
                         size_t input_size,
                         commc_huffman_encoded_t* output);

/*

         commc_huffman_decode()
	       ---
	       decodes Huffman-encoded data back to the
	       original uncompressed form.
	       
	       parameters:
	         ctx - decompression context
	         encoded - encoded data structure
	         output - buffer for decoded data
	         output_size - size of output buffer
	         bytes_written - pointer to store output size
	       
	       returns:
	         COMMC_SUCCESS or error code

*/

int commc_huffman_decode(commc_huffman_context_t* ctx,
                         const commc_huffman_encoded_t* encoded,
                         unsigned char* output,
                         size_t output_size,
                         size_t* bytes_written);

/*

         commc_huffman_node_create()
	       ---
	       creates a new Huffman tree node with the
	       specified symbol and frequency.
	       
	       parameters:
	         symbol - symbol value (-1 for internal nodes)
	         frequency - frequency count for this symbol
	       
	       returns:
	         pointer to new node, or NULL on failure

*/

commc_huffman_node_t* commc_huffman_node_create(int symbol, 
                                                 unsigned int frequency);

/*

         commc_huffman_node_destroy()
	       ---
	       recursively destroys a Huffman tree node and
	       all its children.
	       
	       parameters:
	         node - root node to destroy

*/

void commc_huffman_node_destroy(commc_huffman_node_t* node);

/*

         commc_huffman_write_bit()
	       ---
	       writes a single bit to the encoding buffer,
	       handling byte boundaries and buffer expansion.
	       
	       parameters:
	         ctx - compression context
	         bit - bit value to write (0 or 1)
	       
	       returns:
	         COMMC_SUCCESS or error code

*/

int commc_huffman_write_bit(commc_huffman_context_t* ctx, int bit);

/*

         commc_huffman_write_bits()
	       ---
	       writes multiple bits to the encoding buffer
	       from an integer value.
	       
	       parameters:
	         ctx - compression context
	         bits - bit pattern to write
	         count - number of bits to write
	       
	       returns:
	         COMMC_SUCCESS or error code

*/

int commc_huffman_write_bits(commc_huffman_context_t* ctx, 
                             unsigned int bits, 
                             int count);

/*

         commc_huffman_read_bit()
	       ---
	       reads a single bit from the decoding buffer,
	       handling byte boundaries and buffer management.
	       
	       parameters:
	         ctx - decompression context
	         bit - pointer to store bit value
	       
	       returns:
	         COMMC_SUCCESS or error code

*/

int commc_huffman_read_bit(commc_huffman_context_t* ctx, int* bit);

/*

         commc_huffman_flush_bits()
	       ---
	       flushes any remaining bits in the encoding
	       buffer by padding with zeros.
	       
	       parameters:
	         ctx - compression context
	       
	       returns:
	         COMMC_SUCCESS or error code

*/

int commc_huffman_flush_bits(commc_huffman_context_t* ctx);

/*

         commc_huffman_get_compression_ratio()
	       ---
	       calculates the compression ratio achieved
	       from input and output sizes.
	       
	       parameters:
	         input_size - size of original data
	         output_size - size of compressed data
	       
	       returns:
	         compression ratio as percentage

*/

double commc_huffman_get_compression_ratio(size_t input_size, 
                                           size_t output_size);

/*

         commc_huffman_estimate_output_size()
	       ---
	       estimates the maximum output size needed
	       for encoding data of a given size.
	       
	       parameters:
	         input_size - size of input data
	       
	       returns:
	         estimated maximum output size in bytes

*/

size_t commc_huffman_estimate_output_size(size_t input_size);

/*

         commc_huffman_validate_tree()
	       ---
	       validates that a Huffman tree is properly
	       constructed and contains valid codes.
	       
	       parameters:
	         tree - tree to validate
	       
	       returns:
	         COMMC_SUCCESS if tree is valid

*/

int commc_huffman_validate_tree(const commc_huffman_tree_t* tree);

/*

         commc_huffman_get_tree_depth()
	       ---
	       calculates the maximum depth of the Huffman
	       tree for performance analysis.
	       
	       parameters:
	         tree - tree to analyze
	       
	       returns:
	         maximum depth of the tree

*/

int commc_huffman_get_tree_depth(const commc_huffman_tree_t* tree);

/*

         commc_huffman_print_codes()
	       ---
	       prints the generated Huffman codes for debugging
	       and analysis purposes.
	       
	       parameters:
	         tree - tree containing codes to print

*/

void commc_huffman_print_codes(const commc_huffman_tree_t* tree);

/*

         commc_huffman_encoded_create()
	       ---
	       creates a new encoded data structure.
	       
	       returns:
	         pointer to initialized structure, or NULL on failure

*/

commc_huffman_encoded_t* commc_huffman_encoded_create(void);

/*

         commc_huffman_encoded_destroy()
	       ---
	       releases memory associated with encoded data.
	       
	       parameters:
	         encoded - pointer to encoded data to destroy

*/

void commc_huffman_encoded_destroy(commc_huffman_encoded_t* encoded);

#endif /* COMMC_HUFFMAN_H */

/*
	==================================
             --- EOF ---
	==================================
*/