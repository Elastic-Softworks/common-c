/*
   ===================================
   C O M M O N - C
   ENDIANNESS UTILITIES IMPLEMENTATION
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- ENDIAN MODULE ---

    implementation of comprehensive byte order conversion
    utilities for cross-platform binary compatibility.
    see include/commc/endian.h for function
    prototypes and documentation.

    this module handles the complexities of different
    byte ordering schemes across computer architectures,
    ensuring binary data remains interpretable when
    transferred between systems.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#include  <string.h>      /* for memcpy */

#include  "commc/endian.h"

/*
	==================================
             --- STATIC GLOBALS ---
	==================================
*/

/* cached endianness result to avoid repeated detection */

static commc_endian_type_t cached_endian = COMMC_ENDIAN_UNKNOWN;

/*
	==================================
             --- STATIC HELPERS ---
	==================================
*/

/*

         detect_endianness()
	       ---
	       performs runtime endianness detection by creating
	       a multi-byte integer and examining the byte order
	       in memory. this is done only once and cached for
	       subsequent calls.

*/

static commc_endian_type_t detect_endianness(void) {

  unsigned short test_value = 0x0001;
  
  unsigned char* byte_ptr = (unsigned char*)&test_value;

  /* if the first byte contains 0x01, we're little-endian */
  /* if it contains 0x00, we're big-endian */

  if  (byte_ptr[0] == 0x01) {

    return COMMC_ENDIAN_LITTLE;

  } else if  (byte_ptr[0] == 0x00) {

    return COMMC_ENDIAN_BIG;

  } else {

    return COMMC_ENDIAN_UNKNOWN;

  }

}

/*
	==================================
             --- PUBLIC FUNCS ---
	==================================
*/

/*

         commc_endian_get_host()
	       ---
	       determines the endianness of the current system.

*/

commc_endian_type_t commc_endian_get_host(void) {

  if  (cached_endian == COMMC_ENDIAN_UNKNOWN) {

    cached_endian = detect_endianness();

  }

  return cached_endian;

}

/*

         commc_endian_is_little()
	       ---
	       returns 1 if the host system is little-endian.

*/

int commc_endian_is_little(void) {

  return commc_endian_get_host() == COMMC_ENDIAN_LITTLE;

}

/*

         commc_endian_is_big()
	       ---
	       returns 1 if the host system is big-endian.

*/

int commc_endian_is_big(void) {

  return commc_endian_get_host() == COMMC_ENDIAN_BIG;

}

/*

         commc_endian_swap_16()
	       ---
	       reverses the byte order of a 16-bit value.

*/

unsigned short commc_endian_swap_16(unsigned short value) {

  return (unsigned short)(
    ((value & 0x00FF) << 8) |   /* move low byte to high position */
    ((value & 0xFF00) >> 8)     /* move high byte to low position */
  );

}

/*

         commc_endian_swap_32()
	       ---
	       reverses the byte order of a 32-bit value.

*/

unsigned int commc_endian_swap_32(unsigned int value) {

  return (
    ((value & 0x000000FF) << 24) |   /* byte 0 to byte 3 */
    ((value & 0x0000FF00) << 8)  |   /* byte 1 to byte 2 */
    ((value & 0x00FF0000) >> 8)  |   /* byte 2 to byte 1 */
    ((value & 0xFF000000) >> 24)     /* byte 3 to byte 0 */
  );

}

/*

         commc_endian_swap_64()
	       ---
	       reverses the byte order of a 64-bit value using
	       two 32-bit operations for C89 compatibility.

*/

void commc_endian_swap_64(void* value) {

  unsigned char* bytes = (unsigned char*)value;
  
  unsigned char temp;
  
  int i;

  /* swap bytes symmetrically around the center */

  for  (i = 0; i < 4; i++) {

    temp = bytes[i];
    bytes[i] = bytes[7 - i];
    bytes[7 - i] = temp;

  }

}

/*

         commc_endian_swap_bytes()
	       ---
	       reverses the byte order of an arbitrary memory block.

*/

void commc_endian_swap_bytes(void* data, size_t size) {

  unsigned char* bytes = (unsigned char*)data;
  
  unsigned char temp;
  
  size_t i;

  if  (!data || size <= 1) {

    return;  /* nothing to swap */

  }

  /* swap bytes symmetrically around the center */

  for  (i = 0; i < size / 2; i++) {

    temp = bytes[i];
    bytes[i] = bytes[size - 1 - i];
    bytes[size - 1 - i] = temp;

  }

}

/*

         commc_endian_htole_16()
	       ---
	       converts a 16-bit value from host to little-endian.

*/

unsigned short commc_endian_htole_16(unsigned short value) {

  if  (commc_endian_is_big()) {

    return commc_endian_swap_16(value);

  }

  return value;

}

/*

         commc_endian_letoh_16()
	       ---
	       converts a 16-bit value from little-endian to host.

*/

unsigned short commc_endian_letoh_16(unsigned short value) {

  if  (commc_endian_is_big()) {

    return commc_endian_swap_16(value);

  }

  return value;

}

/*

         commc_endian_htobe_16()
	       ---
	       converts a 16-bit value from host to big-endian.

*/

unsigned short commc_endian_htobe_16(unsigned short value) {

  if  (commc_endian_is_little()) {

    return commc_endian_swap_16(value);

  }

  return value;

}

/*

         commc_endian_betoh_16()
	       ---
	       converts a 16-bit value from big-endian to host.

*/

unsigned short commc_endian_betoh_16(unsigned short value) {

  if  (commc_endian_is_little()) {

    return commc_endian_swap_16(value);

  }

  return value;

}

/*

         commc_endian_htole_32()
	       ---
	       converts a 32-bit value from host to little-endian.

*/

unsigned int commc_endian_htole_32(unsigned int value) {

  if  (commc_endian_is_big()) {

    return commc_endian_swap_32(value);

  }

  return value;

}

/*

         commc_endian_letoh_32()
	       ---
	       converts a 32-bit value from little-endian to host.

*/

unsigned int commc_endian_letoh_32(unsigned int value) {

  if  (commc_endian_is_big()) {

    return commc_endian_swap_32(value);

  }

  return value;

}

/*

         commc_endian_htobe_32()
	       ---
	       converts a 32-bit value from host to big-endian.

*/

unsigned int commc_endian_htobe_32(unsigned int value) {

  if  (commc_endian_is_little()) {

    return commc_endian_swap_32(value);

  }

  return value;

}

/*

         commc_endian_betoh_32()
	       ---
	       converts a 32-bit value from big-endian to host.

*/

unsigned int commc_endian_betoh_32(unsigned int value) {

  if  (commc_endian_is_little()) {

    return commc_endian_swap_32(value);

  }

  return value;

}

/*

         commc_endian_htole_64()
	       ---
	       converts a 64-bit value from host to little-endian.

*/

void commc_endian_htole_64(void* value) {

  if  (!value) {

    return;

  }

  if  (commc_endian_is_big()) {

    commc_endian_swap_64(value);

  }

}

/*

         commc_endian_letoh_64()
	       ---
	       converts a 64-bit value from little-endian to host.

*/

void commc_endian_letoh_64(void* value) {

  if  (!value) {

    return;

  }

  if  (commc_endian_is_big()) {

    commc_endian_swap_64(value);

  }

}

/*

         commc_endian_htobe_64()
	       ---
	       converts a 64-bit value from host to big-endian.

*/

void commc_endian_htobe_64(void* value) {

  if  (!value) {

    return;

  }

  if  (commc_endian_is_little()) {

    commc_endian_swap_64(value);

  }

}

/*

         commc_endian_betoh_64()
	       ---
	       converts a 64-bit value from big-endian to host.

*/

void commc_endian_betoh_64(void* value) {

  if  (!value) {

    return;

  }

  if  (commc_endian_is_little()) {

    commc_endian_swap_64(value);

  }

}

/*

         commc_endian_read_le_16()
	       ---
	       reads a 16-bit little-endian value from a byte buffer.

*/

unsigned short commc_endian_read_le_16(const unsigned char* buffer) {

  if  (!buffer) {

    return 0;

  }

  /* construct value from individual bytes in little-endian order */

  return (unsigned short)(
    ((unsigned short)buffer[0])       |   /* low byte */
    ((unsigned short)buffer[1] << 8)      /* high byte */
  );

}

/*

         commc_endian_read_be_16()
	       ---
	       reads a 16-bit big-endian value from a byte buffer.

*/

unsigned short commc_endian_read_be_16(const unsigned char* buffer) {

  if  (!buffer) {

    return 0;

  }

  /* construct value from individual bytes in big-endian order */

  return (unsigned short)(
    ((unsigned short)buffer[0] << 8)  |   /* high byte */
    ((unsigned short)buffer[1])           /* low byte */
  );

}

/*

         commc_endian_read_le_32()
	       ---
	       reads a 32-bit little-endian value from a byte buffer.

*/

unsigned int commc_endian_read_le_32(const unsigned char* buffer) {

  if  (!buffer) {

    return 0;

  }

  /* construct value from individual bytes in little-endian order */

  return (
    ((unsigned int)buffer[0])        |    /* byte 0 (LSB) */
    ((unsigned int)buffer[1] << 8)   |    /* byte 1 */
    ((unsigned int)buffer[2] << 16)  |    /* byte 2 */
    ((unsigned int)buffer[3] << 24)       /* byte 3 (MSB) */
  );

}

/*

         commc_endian_read_be_32()
	       ---
	       reads a 32-bit big-endian value from a byte buffer.

*/

unsigned int commc_endian_read_be_32(const unsigned char* buffer) {

  if  (!buffer) {

    return 0;

  }

  /* construct value from individual bytes in big-endian order */

  return (
    ((unsigned int)buffer[0] << 24)  |    /* byte 0 (MSB) */
    ((unsigned int)buffer[1] << 16)  |    /* byte 1 */
    ((unsigned int)buffer[2] << 8)   |    /* byte 2 */
    ((unsigned int)buffer[3])             /* byte 3 (LSB) */
  );

}

/*

         commc_endian_write_le_16()
	       ---
	       writes a 16-bit value to a buffer in little-endian format.

*/

void commc_endian_write_le_16(unsigned char* buffer, unsigned short value) {

  if  (!buffer) {

    return;

  }

  /* store bytes in little-endian order */

  buffer[0] = (unsigned char)(value & 0x00FF);        /* low byte */
  buffer[1] = (unsigned char)((value & 0xFF00) >> 8); /* high byte */

}

/*

         commc_endian_write_be_16()
	       ---
	       writes a 16-bit value to a buffer in big-endian format.

*/

void commc_endian_write_be_16(unsigned char* buffer, unsigned short value) {

  if  (!buffer) {

    return;

  }

  /* store bytes in big-endian order */

  buffer[0] = (unsigned char)((value & 0xFF00) >> 8); /* high byte */
  buffer[1] = (unsigned char)(value & 0x00FF);        /* low byte */

}

/*

         commc_endian_write_le_32()
	       ---
	       writes a 32-bit value to a buffer in little-endian format.

*/

void commc_endian_write_le_32(unsigned char* buffer, unsigned int value) {

  if  (!buffer) {

    return;

  }

  /* store bytes in little-endian order */

  buffer[0] = (unsigned char)(value & 0x000000FF);         /* byte 0 (LSB) */
  buffer[1] = (unsigned char)((value & 0x0000FF00) >> 8);  /* byte 1 */
  buffer[2] = (unsigned char)((value & 0x00FF0000) >> 16); /* byte 2 */
  buffer[3] = (unsigned char)((value & 0xFF000000) >> 24); /* byte 3 (MSB) */

}

/*

         commc_endian_write_be_32()
	       ---
	       writes a 32-bit value to a buffer in big-endian format.

*/

void commc_endian_write_be_32(unsigned char* buffer, unsigned int value) {

  if  (!buffer) {

    return;

  }

  /* store bytes in big-endian order */

  buffer[0] = (unsigned char)((value & 0xFF000000) >> 24); /* byte 0 (MSB) */
  buffer[1] = (unsigned char)((value & 0x00FF0000) >> 16); /* byte 1 */
  buffer[2] = (unsigned char)((value & 0x0000FF00) >> 8);  /* byte 2 */
  buffer[3] = (unsigned char)(value & 0x000000FF);         /* byte 3 (LSB) */

}

/*
	==================================
             --- EOF ---
	==================================
*/