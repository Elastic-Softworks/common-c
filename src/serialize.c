/*
   ===================================
   C O M M O N - C
   BINARY SERIALIZATION IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- SERIALIZE MODULE ---

    implementation of comprehensive binary serialization
    and deserialization capabilities.
    see include/commc/serialize.h for function
    prototypes and documentation.

    this module converts data structures into portable
    binary format for storage or transmission across
    different systems and architectures.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#include  <stdlib.h>      /* for malloc, free */
#include  <string.h>      /* for memcpy, strlen */

#include  "commc/serialize.h"
#include  "commc/error.h"

/*
	==================================
             --- STATIC HELPERS ---
	==================================
*/

/*

         is_little_endian()
	       ---
	       determines if the current system uses little-endian
	       byte ordering. returns 1 for little-endian, 0 for
	       big-endian. this is used for cross-platform
	       serialization compatibility.

*/

static int is_little_endian(void) {

  unsigned short test = 0x0001;
  
  return *((unsigned char*)&test) == 1;

}

/*

         swap_bytes()
	       ---
	       reverses the byte order of a memory block in-place.
	       used for endianness conversion during serialization
	       to ensure consistent data representation across
	       different architectures.

*/

static void swap_bytes(void* data, size_t size) {

  unsigned char* bytes = (unsigned char*)data;
  
  size_t  i;

  for  (i = 0; i < size / 2; i++) {

    unsigned char temp = bytes[i];
    
    bytes[i]         = bytes[size - 1 - i];
    bytes[size - 1 - i] = temp;

  }

}

/*

         ensure_buffer_space()
	       ---
	       ensures the serialization buffer has at least
	       the requested number of bytes available from
	       the current position. expands the buffer if
	       necessary using exponential growth strategy.

*/

static int ensure_buffer_space(commc_serialize_context_t* ctx, size_t needed) {

  size_t  required_size;
  
  size_t  new_capacity;
  
  unsigned char*  new_buffer;

  if  (!ctx) {

    return 0;

  }

  required_size = ctx->position + needed;

  if  (required_size <= ctx->capacity) {

    return 1;  /* sufficient space available */

  }

  /* exponential growth strategy for buffer expansion */

  new_capacity = ctx->capacity;

  while  (new_capacity < required_size) {

    new_capacity *= 2;

    if  (new_capacity < ctx->capacity) {  /* overflow check */

      ctx->error_code = COMMC_MEMORY_ERROR;
      return 0;

    }

  }

  new_buffer = (unsigned char*)realloc(ctx->buffer, new_capacity);

  if  (!new_buffer) {

    ctx->error_code = COMMC_MEMORY_ERROR;
    return 0;

  }

  ctx->buffer   = new_buffer;
  ctx->capacity = new_capacity;

  return 1;

}

/*

         get_type_size()
	       ---
	       returns the size in bytes of a primitive data type.
	       used for calculating buffer space requirements and
	       performing serialization operations.

*/

static size_t get_type_size(commc_serialize_type_t type) {

  switch  (type) {

    case COMMC_SERIALIZE_CHAR:    return sizeof(char);
    case COMMC_SERIALIZE_UCHAR:   return sizeof(unsigned char);
    case COMMC_SERIALIZE_SHORT:   return sizeof(short);
    case COMMC_SERIALIZE_USHORT:  return sizeof(unsigned short);
    case COMMC_SERIALIZE_INT:     return sizeof(int);
    case COMMC_SERIALIZE_UINT:    return sizeof(unsigned int);
    case COMMC_SERIALIZE_LONG:    return sizeof(long);
    case COMMC_SERIALIZE_ULONG:   return sizeof(unsigned long);
    case COMMC_SERIALIZE_FLOAT:   return sizeof(float);
    case COMMC_SERIALIZE_DOUBLE:  return sizeof(double);

    default:
      return 0;  /* variable-size types */

  }

}

/*
	==================================
             --- PUBLIC FUNCS ---
	==================================
*/

/*

         commc_serialize_context_create()
	       ---
	       creates and initializes a serialization context
	       with the specified buffer capacity.

*/

commc_serialize_context_t* commc_serialize_context_create(size_t capacity) {

  commc_serialize_context_t*  ctx;

  if  (capacity == 0) {

    capacity = 1024;  /* default buffer size */

  }

  ctx = (commc_serialize_context_t*)malloc(sizeof(commc_serialize_context_t));

  if  (!ctx) {

    return NULL;

  }

  ctx->buffer = (unsigned char*)malloc(capacity);

  if  (!ctx->buffer) {

    free(ctx);
    return NULL;

  }

  ctx->capacity   = capacity;
  ctx->position   = 0;
  ctx->size       = 0;
  ctx->version    = COMMC_SERIALIZE_VERSION;
  ctx->depth      = 0;
  ctx->error_code = 0;

  return ctx;

}

/*

         commc_serialize_context_destroy()
	       ---
	       frees memory associated with a serialization context.

*/

void commc_serialize_context_destroy(commc_serialize_context_t* ctx) {

  if  (!ctx) {

    return;

  }

  if  (ctx->buffer) {

    free(ctx->buffer);

  }

  free(ctx);

}

/*

         commc_serialize_context_reset()
	       ---
	       resets a serialization context to its initial state.

*/

void commc_serialize_context_reset(commc_serialize_context_t* ctx) {

  if  (!ctx) {

    return;

  }

  ctx->position   = 0;
  ctx->size       = 0;
  ctx->depth      = 0;
  ctx->error_code = 0;

}

/*

         commc_serialize_write_header()
	       ---
	       writes the serialization format header.

*/

int commc_serialize_write_header(commc_serialize_context_t* ctx) {

  unsigned int  magic   = COMMC_SERIALIZE_MAGIC;
  
  unsigned int  version = COMMC_SERIALIZE_VERSION;

  if  (!ctx) {

    return 0;

  }

  /* reset position to beginning for header */

  ctx->position = 0;

  /* write magic number in little-endian format */

  if  (!is_little_endian()) {

    swap_bytes(&magic, sizeof(magic));

  }

  if  (!ensure_buffer_space(ctx, sizeof(magic))) {

    return 0;

  }

  memcpy(ctx->buffer + ctx->position, &magic, sizeof(magic));
  ctx->position += sizeof(magic);

  /* write version number in little-endian format */

  if  (!is_little_endian()) {

    swap_bytes(&version, sizeof(version));

  }

  if  (!ensure_buffer_space(ctx, sizeof(version))) {

    return 0;

  }

  memcpy(ctx->buffer + ctx->position, &version, sizeof(version));
  ctx->position += sizeof(version);

  /* update total size */

  if  (ctx->position > ctx->size) {

    ctx->size = ctx->position;

  }

  return 1;

}

/*

         commc_serialize_read_header()
	       ---
	       reads and validates the serialization format header.

*/

int commc_serialize_read_header(commc_serialize_context_t* ctx) {

  unsigned int  magic;
  
  unsigned int  version;

  if  (!ctx || !ctx->buffer) {

    return 0;

  }

  /* ensure we have enough data for header */

  if  (ctx->size < sizeof(magic) + sizeof(version)) {

    ctx->error_code = COMMC_IO_ERROR;
    return 0;

  }

  ctx->position = 0;

  /* read and validate magic number */

  memcpy(&magic, ctx->buffer + ctx->position, sizeof(magic));
  ctx->position += sizeof(magic);

  if  (!is_little_endian()) {

    swap_bytes(&magic, sizeof(magic));

  }

  if  (magic != COMMC_SERIALIZE_MAGIC) {

    ctx->error_code = COMMC_FORMAT_ERROR;
    return 0;

  }

  /* read and check version compatibility */

  memcpy(&version, ctx->buffer + ctx->position, sizeof(version));
  ctx->position += sizeof(version);

  if  (!is_little_endian()) {

    swap_bytes(&version, sizeof(version));

  }

  if  (version > COMMC_SERIALIZE_VERSION) {

    ctx->error_code = COMMC_VERSION_ERROR;
    return 0;

  }

  ctx->version = version;

  return 1;

}

/*

         commc_serialize_write_primitive()
	       ---
	       serializes a primitive data type into the buffer.

*/

int commc_serialize_write_primitive(commc_serialize_context_t* ctx, 
                                   commc_serialize_type_t type, 
                                   const void* data) {

  size_t      type_size;
  
  unsigned char  temp_buffer[sizeof(double)];  /* largest primitive */

  if  (!ctx || !data) {

    if  (ctx) {

      ctx->error_code = COMMC_ARGUMENT_ERROR;

    }

    return 0;

  }

  type_size = get_type_size(type);

  if  (type_size == 0) {

    ctx->error_code = COMMC_ARGUMENT_ERROR;
    return 0;

  }

  if  (!ensure_buffer_space(ctx, type_size)) {

    return 0;

  }

  /* copy data to temp buffer for endianness conversion */

  memcpy(temp_buffer, data, type_size);

  /* convert multi-byte types to little-endian */

  if  (type_size > 1 && !is_little_endian()) {

    swap_bytes(temp_buffer, type_size);

  }

  /* write to buffer */

  memcpy(ctx->buffer + ctx->position, temp_buffer, type_size);
  ctx->position += type_size;

  /* update total size */

  if  (ctx->position > ctx->size) {

    ctx->size = ctx->position;

  }

  return 1;

}

/*

         commc_serialize_read_primitive()
	       ---
	       deserializes a primitive data type from the buffer.

*/

int commc_serialize_read_primitive(commc_serialize_context_t* ctx,
                                  commc_serialize_type_t type,
                                  void* data) {

  size_t  type_size;

  if  (!ctx || !ctx->buffer || !data) {

    if  (ctx) {

      ctx->error_code = COMMC_ARGUMENT_ERROR;

    }

    return 0;

  }

  type_size = get_type_size(type);

  if  (type_size == 0) {

    ctx->error_code = COMMC_ARGUMENT_ERROR;
    return 0;

  }

  /* check available data */

  if  (ctx->position + type_size > ctx->size) {

    ctx->error_code = COMMC_IO_ERROR;
    return 0;

  }

  /* read from buffer */

  memcpy(data, ctx->buffer + ctx->position, type_size);
  ctx->position += type_size;

  /* convert from little-endian if necessary */

  if  (type_size > 1 && !is_little_endian()) {

    swap_bytes(data, type_size);

  }

  return 1;

}

/*

         commc_serialize_write_string()
	       ---
	       serializes a null-terminated string.

*/

int commc_serialize_write_string(commc_serialize_context_t* ctx, 
                                const char* str) {

  size_t  length;
  
  unsigned int  length_field;

  if  (!ctx) {

    return 0;

  }

  /* handle null strings */

  if  (!str) {

    length = 0;

  } else {

    length = strlen(str);

  }

  /* write length field */

  length_field = (unsigned int)length;

  if  (!commc_serialize_write_primitive(ctx, COMMC_SERIALIZE_UINT, &length_field)) {

    return 0;

  }

  /* write string data if non-empty */

  if  (length > 0) {

    if  (!ensure_buffer_space(ctx, length)) {

      return 0;

    }

    memcpy(ctx->buffer + ctx->position, str, length);
    ctx->position += length;

    /* update total size */

    if  (ctx->position > ctx->size) {

      ctx->size = ctx->position;

    }

  }

  return 1;

}

/*

         commc_serialize_read_string()
	       ---
	       deserializes a string from the buffer.

*/

char* commc_serialize_read_string(commc_serialize_context_t* ctx) {

  unsigned int  length;
  
  char*  str;

  if  (!ctx || !ctx->buffer) {

    return NULL;

  }

  /* read length field */

  if  (!commc_serialize_read_primitive(ctx, COMMC_SERIALIZE_UINT, &length)) {

    return NULL;

  }

  /* handle empty strings */

  if  (length == 0) {

    return NULL;  /* null string indicator */

  }

  /* check available data */

  if  (ctx->position + length > ctx->size) {

    ctx->error_code = COMMC_IO_ERROR;
    return NULL;

  }

  /* allocate string buffer with null terminator */

  str = (char*)malloc(length + 1);

  if  (!str) {

    ctx->error_code = COMMC_MEMORY_ERROR;
    return NULL;

  }

  /* read string data */

  memcpy(str, ctx->buffer + ctx->position, length);
  ctx->position += length;

  /* ensure null termination */

  str[length] = '\0';

  return str;

}

/*

         commc_serialize_get_buffer()
	       ---
	       returns pointer to internal buffer.

*/

const unsigned char* commc_serialize_get_buffer(const commc_serialize_context_t* ctx) {

  if  (!ctx) {

    return NULL;

  }

  return ctx->buffer;

}

/*

         commc_serialize_get_size()
	       ---
	       returns current size of serialized data.

*/

size_t commc_serialize_get_size(const commc_serialize_context_t* ctx) {

  if  (!ctx) {

    return 0;

  }

  return ctx->size;

}

/*

         commc_serialize_get_error()
	       ---
	       returns last error code.

*/

int commc_serialize_get_error(const commc_serialize_context_t* ctx) {

  if  (!ctx) {

    return COMMC_ARGUMENT_ERROR;

  }

  return ctx->error_code;

}

/*

         commc_serialize_set_buffer()
	       ---
	       sets context buffer to external data for deserialization.

*/

int commc_serialize_set_buffer(commc_serialize_context_t* ctx,
                              const unsigned char* buffer,
                              size_t size) {

  if  (!ctx || !buffer || size == 0) {

    if  (ctx) {

      ctx->error_code = COMMC_ARGUMENT_ERROR;

    }

    return 0;

  }

  /* note: we don't free existing buffer in set_buffer since */
  /* it may be external memory that shouldn't be freed */

  /* set external buffer (read-only) */

  ctx->buffer   = (unsigned char*)buffer;  /* cast away const for internal use */
  ctx->capacity = size;
  ctx->size     = size;
  ctx->position = 0;
  ctx->error_code = 0;

  return 1;

}

/*
	==================================
             --- EOF ---
	==================================
*/