#include "map.h"
#include "main.h"
#include <string.h>

u64 hash(char *s, u32 len)
{
	u32 i;
	u64 h;
	for(i = 0, h = 5381; i < len; ++i, ++s)
	{
		h = ((h << 5) + h) + *s;
	}

	return h;
}

i32 map_find_hash(Map *map, char *key, u32 len, u64 h)
{
	u32 i;
	for(i = 0; i < map->Count; ++i)
	{
		if(map->Elements[i].Hash != h)
		{
			continue;
		}

		if(map->Elements[i].Length != len)
		{
			continue;
		}

		if(!strncmp(map->Elements[i].Key, key, len))
		{
			return i;
		}
	}

	return -1;
}

i32 map_find(Map *map, char *key, u32 len)
{
	u64 h;
	h = hash(key, len);
	return map_find_hash(map, key, len, h);
}

i32 map_insert(Map *map, char *key, u32 len)
{
	u64 h;
	if(map->Count >= map->Size)
	{
		return -ERROR_STACK_OVERFLOW;
	}

	h = hash(key, len);
	if(map_find_hash(map, key, len, h) != -1)
	{
		return -ERROR_DUP_MAP_ELEM;
	}

	map->Elements[map->Count].Key = key;
	map->Elements[map->Count].Length = len;
	map->Elements[map->Count].Hash = h;
	return map->Count++;
}

void map_init(Map *map, MapElement *buf, u32 size)
{
	map->Elements = buf;
	map->Count = 0;
	map->Size = size;
}

