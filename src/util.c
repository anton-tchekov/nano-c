#include "util.h"

u16 _read_16(u8 *p)
{
	u16 v;
	v = *p | (*(p + 1) << 8);
	return v;
}

void _write_16(u8 *b, u16 v)
{
	*b = (u8)(v & 0xFF);
	*(b + 1) = (u8)((v >> 8) & 0xFF);
}

u32 _read_32(u8 *p)
{
	u32 v;
	v = *p | (*(p + 1) << 8) | (*(p + 2) << 16) | (*(p + 3) << 24);
	return v;
}

void _write_32(u8 *b, u32 v)
{
	*b = (u8)(v & 0xFF);
	*(b + 1) = (u8)((v >> 8) & 0xFF);
	*(b + 2) = (u8)((v >> 16) & 0xFF);
	*(b + 3) = (u8)((v >> 24) & 0xFF);
}
