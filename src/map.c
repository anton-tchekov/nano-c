#include "map.h"
#include <string.h>

static u32 _map_hash(const char *s, u32 len)
{
	u32 i, h;
	for(i = 0, h = 0; i < len; ++i)
	{
		h = 37 * h + s[i];
	}

	return h;
}

static i32 _map_find_hash(Map *map, const char *key, u32 len, u32 h)
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

i32 _map_find(Map *map, const char *key, u32 len)
{
	u32 h;
	h = _map_hash(key, len);
	return _map_find_hash(map, key, len, h);
}

i32 _map_insert(Map *map, const char *key, u32 len)
{
	u32 h;
	if(map->Count >= map->Size)
	{
		return -ERROR_STACK_OVERFLOW;
	}

	h = _map_hash(key, len);
	if(_map_find_hash(map, key, len, h) != -1)
	{
		return -ERROR_DUP_MAP_ELEM;
	}

	map->Elements[map->Count].Key = key;
	map->Elements[map->Count].Length = len;
	map->Elements[map->Count].Hash = h;
	return map->Count++;
}

void _map_init(Map *map, MapElement *buf, u32 size)
{
	map->Elements = buf;
	map->Count = 0;
	map->Size = size;
}
