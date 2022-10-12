#ifndef __UTIL_H__
#define __UTIL_H__

#include "types.h"

#define ARRLEN(a) (sizeof(a) / sizeof(*a))
#define ABS(n) (((n) < 0) ? -(n) : (n))

u16 _read_16(u8 *p);
void _write_16(u8 *b, u16 v);

u32 _read_32(u8 *p);
void _write_32(u8 *b, u32 v);

#endif
