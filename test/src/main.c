#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nanoc.h"
#include "readfile.h"

#include <time.h>

#define STRINGS_OFFSET (8 * 1024)

#define TEXT_RED "\033[31;1m"
#define TEXT_WHITE "\033[0m"

i32 print_number(i32 *parameters, u8 *heap)
{
	printf("%d", parameters[0]);
	return 0;
}

i32 get_number(i32 *parameters, u8 *heap)
{
	i32 number;
	scanf("%d", &number);
	return number;
}

i32 print_string(i32 *parameters, u8 *heap)
{
	printf("%s", (char *)&heap[parameters[0]]);
	return 0;
}

i32 empty(i32 *parameters, u8 *heap)
{
	return 0;
}

int main(int argc, char **argv)
{
	NanoC nanoc;
	u8 *program, *data;
	i32 ret;
	int length;
	char *src;
	FILE *fp;

	/* check args */
	if(argc != 2 && argc != 3)
	{
		fprintf(stderr, "usage: ./nanoc filename\n");
		return 1;
	}

	/* load source file */
	if(!(fp = fopen(argv[1], "r")))
	{
		fprintf(stderr, "unable to open file: \"%s\"\n", argv[1]);
		return 1;
	}

	if(!(src = readfile(fp, &length)))
	{
		fprintf(stderr, "error reading file\n");
		return 1;
	}

	fclose(fp);

	/* init NanoC */
	nanoc_init(&nanoc);
	nanoc_add_function(&nanoc,  0, "content_type",  1, empty);
	nanoc_add_function(&nanoc,  1, "response_code", 1, empty);
	nanoc_add_function(&nanoc,  2, "response_body", 1, empty);
	nanoc_add_function(&nanoc,  3, "args_count",    0, empty);
	nanoc_add_function(&nanoc,  4, "arg_name",      3, empty);
	nanoc_add_function(&nanoc,  5, "arg",           3, empty);
	nanoc_add_function(&nanoc,  6, "arg_name_len",  1, empty);
	nanoc_add_function(&nanoc,  7, "arg_len",       1, empty);

	nanoc_add_function(&nanoc,  8, "memcpy",        3, empty);
	nanoc_add_function(&nanoc,  9, "memmove",       3, empty);
	nanoc_add_function(&nanoc, 10, "memcmp",        3, empty);
	nanoc_add_function(&nanoc, 11, "memchr",        3, empty);
	nanoc_add_function(&nanoc, 12, "memset",        3, empty);

	nanoc_add_function(&nanoc, 13, "strcpy",        2, empty);
	nanoc_add_function(&nanoc, 14, "strncpy",       3, empty);
	nanoc_add_function(&nanoc, 15, "strcat",        2, empty);
	nanoc_add_function(&nanoc, 16, "strncat",       3, empty);
	nanoc_add_function(&nanoc, 17, "strcmp",        2, empty);
	nanoc_add_function(&nanoc, 18, "strncmp",       3, empty);
	nanoc_add_function(&nanoc, 19, "strchr",        2, empty);
	nanoc_add_function(&nanoc, 20, "strlen",        1, empty);

	nanoc_add_function(&nanoc, 21, "itoa",          3, empty);
	nanoc_add_function(&nanoc, 22, "atoi",          1, empty);
	nanoc_add_function(&nanoc, 23, "rand",          0, empty);
	nanoc_add_function(&nanoc, 24, "srand",         1, empty);

	nanoc_add_function(&nanoc, 25, "print_number",  1, print_number);
	nanoc_add_function(&nanoc, 26, "print_string",  1, print_string);
	nanoc_add_function(&nanoc, 27, "get_number",    0, get_number);


	/* compile to bytecode */
	if(!(program = calloc(16 * 1024, 1)))
	{
		fprintf(stderr, "error allocating memory for bytecode\n");
		return 1;
	}

	if(!(data = calloc(16 * 1024, 1)))
	{
		fprintf(stderr, "error allocating memory for data\n");
		return 1;
	}


	if((ret = nanoc_compile(&nanoc, src, program, data + STRINGS_OFFSET, STRINGS_OFFSET)))
	{
		int i;
		const char *s;
		printf("parse error: %s\n"
				"row: %d, col: %d\n",
				nanoc_error_message(ret),
				nanoc.Token.Pos.Row, nanoc.Token.Pos.Col);

		for(i = 0, s = nanoc.Lexer.LineBegin; *s != '\n' && s - src < length; ++s, ++i)
		{
			if(i == nanoc.Token.Pos.Col)
			{
				fputs(TEXT_RED, stdout);
			}

			if(i == nanoc.Lexer.Pos.Col)
			{
				fputs(TEXT_WHITE, stdout);
			}

			fputc(*s, stdout);
		}

		fputs(TEXT_WHITE, stdout);
		fputc('\n', stdout);
		return 1;
	}

	/* write output file */
	if(argc == 3)
	{
		u8 header[4];

		u16 program_size, data_size;

		if(!(fp = fopen(argv[2], "w")))
		{
			fprintf(stderr, "unable to open output file for writing: \"%s\"\n", argv[2]);
			return 1;
		}

		program_size = nanoc.Parser.Index;
		data_size = nanoc.Lexer.StringsPtr - nanoc.Lexer.Strings;

		_write_16(header, program_size);
		_write_16(header + 2, data_size);

		fwrite(header, 1, 4, fp);

		fwrite(program, 1, program_size, fp);
		fwrite(data + STRINGS_OFFSET, 1, data_size, fp);

		fclose(fp);
	}

	/* run interpreter */
	nanoc_disasm(program, nanoc.Parser.Index);

	if((ret = nanoc_run(&nanoc, program, data)))
	{
		printf("runtime error: %s\n", nanoc_error_message(ret));
	}

	free(program);
	free(src);
	return 0;
}

