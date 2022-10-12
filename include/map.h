#ifndef __NANOC_MAP_H__
#define __NANOC_MAP_H__

#include "types.h"
#include "error.h"

typedef struct MAP_ELEMENT
{
	const char *Key;
	u32 Length, Hash;
} MapElement;

typedef struct MAP
{
	MapElement *Elements;
	u32 Count, Size;
} Map;

i32 _map_find(Map *map, const char *key, u32 len);
i32 _map_insert(Map *map, const char *key, u32 len);
void _map_init(Map *map, MapElement *buf, u32 size);

#endif /* __NANOC_MAP_H__ */
