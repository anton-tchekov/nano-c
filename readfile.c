#include <stdio.h>
#include <stdlib.h>
#include "readfile.h"

char *readfile(FILE *in, int *length)
{
	char *data, *temp;
	int size, used, n;
	size = 0;
	used = 0;
	data = 0;

	if(ferror(in))
	{
		return 0;
	}

	for(;;)
	{
		if(used + READFILE_CHUNK + 1 > size)
		{
			size = used + READFILE_CHUNK + 1;
			if(!(temp = realloc(data, size)))
			{
				free(data);
				return 0;
			}

			data = temp;
		}

		if(!(n = fread(data + used, 1, READFILE_CHUNK, in)))
		{
			break;
		}

		used += n;
	}

	if(ferror(in))
	{
		free(data);
		return 0;
	}

	if(!(temp = realloc(data, used + 1)))
	{
		free(data);
		return 0 ;
	}

	data = temp;
	data[used] = '\0';
	*length = used;
	return data;
}

