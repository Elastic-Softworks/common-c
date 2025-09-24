/*
   ===================================
   C O M M O N - C
   ENDIANNESS UTILITIES MODULE
   ELASTIC SOFTWORKS 2025
   ===================================
*/

/*

            --- ENDIAN MODULE ---

    this module provides comprehensive byte order conversion
    utilities for cross-platform binary compatibility.
    it detects host endianness and converts between
    little-endian and big-endian formats.

    endianness refers to the order of bytes within multi-byte
    data types. little-endian stores the least significant
    byte first, while big-endian stores the most significant
    byte first. network protocols typically use big-endian
    (network byte order), while most x86/x64 systems use
    little-endian.

    proper endianness handling ensures binary data can be
    exchanged between different architectures without
    corruption or misinterpretation.

*/

/*
	==================================
             --- SETUP ---
	==================================
*/

#ifndef   COMMC_ENDIAN_H
#define   COMMC_ENDIAN_H

#include  <stddef.h>      /* for size_t */

/*
	==================================
             --- DEFINES ---
	==================================
*/

/* endianness constants for clarity */

#define   COMMC_LITTLE_ENDIAN   1234
#define   COMMC_BIG_ENDIAN      4321

/*
	==================================
             --- ENUMS ---
	==================================
*/

/* endianness type enumeration */

typedef enum {

  COMMC_ENDIAN_LITTLE,    /* least significant byte first */
  COMMC_ENDIAN_BIG,       /* most significant byte first */
  COMMC_ENDIAN_UNKNOWN    /* could not determine endianness */

} commc_endian_type_t;

/*
	==================================
             --- FUNCTIONS ---
	==================================
*/

/*

         commc_endian_get_host()
	       ---
	       determines the endianness of the current system
	       at runtime. returns the endianness type or
	       COMMC_ENDIAN_UNKNOWN if detection fails.
	       
	       this function creates a multi-byte integer and
	       examines the byte order in memory to determine
	       whether the system uses little-endian or
	       big-endian byte ordering.

*/

commc_endian_type_t commc_endian_get_host(void);

/*

         commc_endian_is_little()
	       ---
	       returns 1 if the host system is little-endian,
	       0 otherwise. this is a convenience function for
	       quick endianness checks without enum comparison.

*/

int commc_endian_is_little(void);

/*

         commc_endian_is_big()
	       ---
	       returns 1 if the host system is big-endian,
	       0 otherwise. this is a convenience function for
	       quick endianness checks without enum comparison.

*/

int commc_endian_is_big(void);

/*

         commc_endian_swap_16()
	       ---
	       reverses the byte order of a 16-bit value.
	       converts between little-endian and big-endian
	       representations. the operation is its own inverse.

*/

unsigned short commc_endian_swap_16(unsigned short value);

/*

         commc_endian_swap_32()
	       ---
	       reverses the byte order of a 32-bit value.
	       converts between little-endian and big-endian
	       representations. the operation is its own inverse.

*/

unsigned int commc_endian_swap_32(unsigned int value);

/*

         commc_endian_swap_64()
	       ---
	       reverses the byte order of a 64-bit value using
	       two 32-bit operations to maintain C89 compatibility.
	       converts between little-endian and big-endian
	       representations.

*/

void commc_endian_swap_64(void* value);

/*

         commc_endian_swap_bytes()
	       ---
	       reverses the byte order of an arbitrary memory block.
	       useful for swapping custom data types or arrays
	       of bytes. operates in-place on the provided buffer.

*/

void commc_endian_swap_bytes(void* data, size_t size);

/*

         commc_endian_htole_16()
	       ---
	       converts a 16-bit value from host byte order to
	       little-endian. if the host is already little-endian,
	       no conversion is performed.

*/

unsigned short commc_endian_htole_16(unsigned short value);

/*

         commc_endian_letoh_16()
	       ---
	       converts a 16-bit value from little-endian to
	       host byte order. if the host is already little-endian,
	       no conversion is performed.

*/

unsigned short commc_endian_letoh_16(unsigned short value);

/*

         commc_endian_htobe_16()
	       ---
	       converts a 16-bit value from host byte order to
	       big-endian. if the host is already big-endian,
	       no conversion is performed.

*/

unsigned short commc_endian_htobe_16(unsigned short value);

/*

         commc_endian_betoh_16()
	       ---
	       converts a 16-bit value from big-endian to
	       host byte order. if the host is already big-endian,
	       no conversion is performed.

*/

unsigned short commc_endian_betoh_16(unsigned short value);

/*

         commc_endian_htole_32()
	       ---
	       converts a 32-bit value from host byte order to
	       little-endian. if the host is already little-endian,
	       no conversion is performed.

*/

unsigned int commc_endian_htole_32(unsigned int value);

/*

         commc_endian_letoh_32()
	       ---
	       converts a 32-bit value from little-endian to
	       host byte order. if the host is already little-endian,
	       no conversion is performed.

*/

unsigned int commc_endian_letoh_32(unsigned int value);

/*

         commc_endian_htobe_32()
	       ---
	       converts a 32-bit value from host byte order to
	       big-endian. if the host is already big-endian,
	       no conversion is performed.

*/

unsigned int commc_endian_htobe_32(unsigned int value);

/*

         commc_endian_betoh_32()
	       ---
	       converts a 32-bit value from big-endian to
	       host byte order. if the host is already big-endian,
	       no conversion is performed.

*/

unsigned int commc_endian_betoh_32(unsigned int value);

/*

         commc_endian_htole_64()
	       ---
	       converts a 64-bit value from host byte order to
	       little-endian using in-place byte swapping to
	       maintain C89 compatibility.

*/

void commc_endian_htole_64(void* value);

/*

         commc_endian_letoh_64()
	       ---
	       converts a 64-bit value from little-endian to
	       host byte order using in-place byte swapping to
	       maintain C89 compatibility.

*/

void commc_endian_letoh_64(void* value);

/*

         commc_endian_htobe_64()
	       ---
	       converts a 64-bit value from host byte order to
	       big-endian using in-place byte swapping to
	       maintain C89 compatibility.

*/

void commc_endian_htobe_64(void* value);

/*

         commc_endian_betoh_64()
	       ---
	       converts a 64-bit value from big-endian to
	       host byte order using in-place byte swapping to
	       maintain C89 compatibility.

*/

void commc_endian_betoh_64(void* value);

/*

         commc_endian_read_le_16()
	       ---
	       reads a 16-bit little-endian value from a byte buffer
	       and returns it in host byte order. the buffer must
	       contain at least 2 bytes.

*/

unsigned short commc_endian_read_le_16(const unsigned char* buffer);

/*

         commc_endian_read_be_16()
	       ---
	       reads a 16-bit big-endian value from a byte buffer
	       and returns it in host byte order. the buffer must
	       contain at least 2 bytes.

*/

unsigned short commc_endian_read_be_16(const unsigned char* buffer);

/*

         commc_endian_read_le_32()
	       ---
	       reads a 32-bit little-endian value from a byte buffer
	       and returns it in host byte order. the buffer must
	       contain at least 4 bytes.

*/

unsigned int commc_endian_read_le_32(const unsigned char* buffer);

/*

         commc_endian_read_be_32()
	       ---
	       reads a 32-bit big-endian value from a byte buffer
	       and returns it in host byte order. the buffer must
	       contain at least 4 bytes.

*/

unsigned int commc_endian_read_be_32(const unsigned char* buffer);

/*

         commc_endian_write_le_16()
	       ---
	       writes a 16-bit value to a byte buffer in little-endian
	       format. the buffer must have space for at least 2 bytes.

*/

void commc_endian_write_le_16(unsigned char* buffer, unsigned short value);

/*

         commc_endian_write_be_16()
	       ---
	       writes a 16-bit value to a byte buffer in big-endian
	       format. the buffer must have space for at least 2 bytes.

*/

void commc_endian_write_be_16(unsigned char* buffer, unsigned short value);

/*

         commc_endian_write_le_32()
	       ---
	       writes a 32-bit value to a byte buffer in little-endian
	       format. the buffer must have space for at least 4 bytes.

*/

void commc_endian_write_le_32(unsigned char* buffer, unsigned int value);

/*

         commc_endian_write_be_32()
	       ---
	       writes a 32-bit value to a byte buffer in big-endian
	       format. the buffer must have space for at least 4 bytes.

*/

void commc_endian_write_be_32(unsigned char* buffer, unsigned int value);

#endif /* COMMC_ENDIAN_H */

/*
	==================================
             --- EOF ---
	==================================
*/