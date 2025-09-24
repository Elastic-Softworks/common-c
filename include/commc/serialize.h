/*
   ===================================
   C O M M O N - C
   BINARY SERIALIZATION MODULE
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- SERIALIZE MODULE ---

    this module provides comprehensive binary serialization
    and deserialization capabilities for cross-platform
    data exchange. it handles primitive types, arrays,
    nested structures, and version compatibility.

    serialization is the process of converting data structures
    into a linear sequence of bytes that can be stored to
    files or transmitted over networks. deserialization
    reverses this process, reconstructing the original data.

    the framework provides type safety, endianness handling,
    and format versioning to ensure reliable data exchange
    across different systems and architectures.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#ifndef   COMMC_SERIALIZE_H
#define   COMMC_SERIALIZE_H

#include  <stddef.h>      /* for size_t */

/*
	==================================
             --- DEFINES ---
	==================================
*/

/* serialization format version for compatibility tracking */

#define   COMMC_SERIALIZE_VERSION     1

/* magic number to identify serialized data format */

#define   COMMC_SERIALIZE_MAGIC       0x434D4D43   /* "CMMC" */

/* maximum nesting depth for structures to prevent infinite recursion */

#define   COMMC_SERIALIZE_MAX_DEPTH   64

/*
	==================================
             --- ENUMS ---
	==================================
*/

/* supported data types for serialization */

typedef enum {

  COMMC_SERIALIZE_CHAR,           /* 8-bit signed character */
  COMMC_SERIALIZE_UCHAR,          /* 8-bit unsigned character */
  COMMC_SERIALIZE_SHORT,          /* 16-bit signed integer */
  COMMC_SERIALIZE_USHORT,         /* 16-bit unsigned integer */
  COMMC_SERIALIZE_INT,            /* 32-bit signed integer */
  COMMC_SERIALIZE_UINT,           /* 32-bit unsigned integer */
  COMMC_SERIALIZE_LONG,           /* platform-dependent signed long */
  COMMC_SERIALIZE_ULONG,          /* platform-dependent unsigned long */
  COMMC_SERIALIZE_FLOAT,          /* 32-bit floating point */
  COMMC_SERIALIZE_DOUBLE,         /* 64-bit floating point */
  COMMC_SERIALIZE_STRING,         /* null-terminated character string */
  COMMC_SERIALIZE_ARRAY,          /* homogeneous array of elements */
  COMMC_SERIALIZE_STRUCT          /* heterogeneous structure */

} commc_serialize_type_t;

/*
	==================================
             --- STRUCTS ---
	==================================
*/

/* serialization context for maintaining state during operations */

typedef struct {

  unsigned char*  buffer;         /* output/input buffer for data */
  size_t          capacity;       /* total buffer capacity */
  size_t          position;       /* current read/write position */
  size_t          size;           /* actual data size in buffer */
  unsigned int    version;        /* format version for compatibility */
  int             depth;          /* current nesting depth */
  int             error_code;     /* last error that occurred */

} commc_serialize_context_t;

/* type descriptor for complex data structures */

typedef struct {

  commc_serialize_type_t  type;      /* base type of the element */
  size_t                  size;      /* size of individual element */
  size_t                  count;     /* number of elements (for arrays) */
  const char*             name;      /* field name (for debugging) */

} commc_serialize_descriptor_t;

/*
	==================================
             --- FUNCTIONS ---
	==================================
*/

/*

         commc_serialize_context_create()
	       ---
	       creates and initializes a serialization context
	       with the specified buffer capacity. returns a
	       pointer to the context on success, null on failure.
	       
	       the context manages the state of serialization
	       operations including the buffer, current position,
	       version information, and error tracking.

*/

commc_serialize_context_t* commc_serialize_context_create(size_t capacity);

/*

         commc_serialize_context_destroy()
	       ---
	       frees memory associated with a serialization context
	       and its internal buffer. the context pointer becomes
	       invalid after this call.

*/

void commc_serialize_context_destroy(commc_serialize_context_t* ctx);

/*

         commc_serialize_context_reset()
	       ---
	       resets a serialization context to its initial state,
	       clearing the buffer position and any error flags.
	       allows reusing the same context for multiple operations.

*/

void commc_serialize_context_reset(commc_serialize_context_t* ctx);

/*

         commc_serialize_write_header()
	       ---
	       writes the serialization format header including
	       magic number and version information. this should
	       be called before serializing any data to ensure
	       format compatibility during deserialization.

*/

int commc_serialize_write_header(commc_serialize_context_t* ctx);

/*

         commc_serialize_read_header()
	       ---
	       reads and validates the serialization format header.
	       returns 1 if the header is valid and compatible,
	       0 if invalid or incompatible. this should be called
	       before deserializing any data.

*/

int commc_serialize_read_header(commc_serialize_context_t* ctx);

/*

         commc_serialize_write_primitive()
	       ---
	       serializes a primitive data type (char, int, float, etc.)
	       into the context buffer. handles endianness conversion
	       automatically for cross-platform compatibility.

*/

int commc_serialize_write_primitive(commc_serialize_context_t* ctx, 
                                   commc_serialize_type_t type, 
                                   const void* data);

/*

         commc_serialize_read_primitive()
	       ---
	       deserializes a primitive data type from the context
	       buffer. performs endianness conversion and type
	       validation to ensure data integrity.

*/

int commc_serialize_read_primitive(commc_serialize_context_t* ctx,
                                  commc_serialize_type_t type,
                                  void* data);

/*

         commc_serialize_write_string()
	       ---
	       serializes a null-terminated string by writing its
	       length followed by the character data. handles null
	       strings gracefully by storing a zero length.

*/

int commc_serialize_write_string(commc_serialize_context_t* ctx, 
                                const char* str);

/*

         commc_serialize_read_string()
	       ---
	       deserializes a string from the buffer. allocates
	       memory for the string data which must be freed by
	       the caller. returns null on error or for null strings.

*/

char* commc_serialize_read_string(commc_serialize_context_t* ctx);

/*

         commc_serialize_write_array()
	       ---
	       serializes an array of homogeneous elements by writing
	       the element count, type information, and data. handles
	       arrays of primitives and structures.

*/

int commc_serialize_write_array(commc_serialize_context_t* ctx,
                               commc_serialize_type_t element_type,
                               const void* array,
                               size_t element_size,
                               size_t count);

/*

         commc_serialize_read_array()
	       ---
	       deserializes an array from the buffer. allocates
	       memory for the array data and stores the element
	       count. the caller must free the returned array.

*/

void* commc_serialize_read_array(commc_serialize_context_t* ctx,
                                commc_serialize_type_t expected_type,
                                size_t element_size,
                                size_t* out_count);

/*

         commc_serialize_write_struct()
	       ---
	       serializes a structure using an array of field
	       descriptors. each descriptor specifies the field
	       type, size, and offset within the structure.

*/

int commc_serialize_write_struct(commc_serialize_context_t* ctx,
                                const void* data,
                                const commc_serialize_descriptor_t* descriptors,
                                size_t descriptor_count);

/*

         commc_serialize_read_struct()
	       ---
	       deserializes a structure using field descriptors.
	       allocates memory for the structure which must be
	       freed by the caller. validates field types and
	       versions for compatibility.

*/

void* commc_serialize_read_struct(commc_serialize_context_t* ctx,
                                 const commc_serialize_descriptor_t* descriptors,
                                 size_t descriptor_count,
                                 size_t struct_size);

/*

         commc_serialize_get_buffer()
	       ---
	       returns a pointer to the internal serialization
	       buffer for direct access. useful for writing
	       the serialized data to files or network sockets.

*/

const unsigned char* commc_serialize_get_buffer(const commc_serialize_context_t* ctx);

/*

         commc_serialize_get_size()
	       ---
	       returns the current size of serialized data in
	       the context buffer. this is the number of bytes
	       that should be written or transmitted.

*/

size_t commc_serialize_get_size(const commc_serialize_context_t* ctx);

/*

         commc_serialize_get_error()
	       ---
	       returns the last error code that occurred during
	       serialization operations. zero indicates no error.

*/

int commc_serialize_get_error(const commc_serialize_context_t* ctx);

/*

         commc_serialize_set_buffer()
	       ---
	       sets the context buffer to point to external data
	       for deserialization. the buffer must remain valid
	       for the lifetime of the context operations.

*/

int commc_serialize_set_buffer(commc_serialize_context_t* ctx,
                              const unsigned char* buffer,
                              size_t size);

#endif /* COMMC_SERIALIZE_H */

/*
	==================================
             --- EOF ---
	==================================
*/
