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

// type limits
#define INT8_MAX  (__INT8_MAX__)
#define INT16_MAX (__INT16_MAX__)
#define INT32_MAX (__INT32_MAX__)
#define INT64_MAX (__INT64_MAX__)

#define INT8_MIN  (INT8_MAX + 1)
#define INT16_MIN (INT16_MAX + 1)
#define INT32_MIN (INT32_MAX + 1)
#define INT64_MIN (INT64_MAX + 1)

#define UINT8_MAX  (__UINT8_MAX__)
#define UINT16_MAX (__UINT16_MAX__)
#define UINT32_MAX (__UINT32_MAX__)
#define UINT64_MAX (__UINT64_MAX__)

// process exit codes constants
typedef byte status_code;
#define OK (0)
#define ERR (1)

#endif
