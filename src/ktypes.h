#ifndef KTYPES_H
#define KTYPES_H

/**
 * ktypes.h
 *
 * Defines the types to be used in the RIO kernel
 */

// kernel data types
typedef __INT8_TYPE__   int8; 
typedef __INT16_TYPE__  int16; 
typedef __INT32_TYPE__  int32; 
typedef __INT64_TYPE__  int64; 

typedef __UINT8_TYPE__  uint8; 
typedef __UINT16_TYPE__ uint16; 
typedef __UINT32_TYPE__ uint32; 
typedef __UINT64_TYPE__ uint64; 

typedef uint8 byte;
#define NULL ((void *) 0x0)

// process exit codes constants
typedef byte status_code;
#define OK (0)
#define ERR (1)

#endif
