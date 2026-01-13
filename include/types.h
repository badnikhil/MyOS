#pragma once

// sanity check
#if __CHAR_BIT__ != 8
#error "Unsupported platform: char is not 8 bits"
#endif

// 8  bit definitions
typedef unsigned char  u8;
typedef signed char    s8;

// 16  bit definitions
#if __SIZEOF_SHORT__ == 2
typedef unsigned short u16;
typedef signed short   s16;
#else
#error "No 16-bit type available"
#endif

// 32  bit definitions
#if __SIZEOF_INT__ == 4
typedef unsigned int  u32;
typedef signed int    s32;
#else
#error "No 32-bit type available"
#endif

// 64  bit definitions
#if __SIZEOF_LONG_LONG__ == 8
typedef unsigned long long u64;
typedef signed long long   s64;
#else
#error "No 64-bit type available"
#endif
