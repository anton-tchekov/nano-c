#ifndef __MAP_H__
#define __MAP_H__

#include "types.h"

typedef struct MAP_ELEMENT
{
	char *Key;
	u32 Length;
	u64 Hash;
} MapElement;

typedef struct MAP
{
	MapElement *Elements;
	u32 Count, Size;
} Map;

u64 hash(char *s, u32 len);
i32 map_find_hash(Map *map, char *key, u32 len, u64 h);
i32 map_find(Map *map, char *key, u32 len);
i32 map_insert(Map *map, char *key, u32 len);
void map_init(Map *map, MapElement *buf, u32 size);

#endif
