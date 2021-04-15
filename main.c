/* Copyright Anton Tchekov */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "readfile.h"
#include "main.h"

const char *err_msgs[] =
{
	"ERROR_UNEXPECTED_EOF",
	"ERROR_UNEXPECTED_TOKEN",
	"ERROR_EXPECTED_L_BRACE",
	"ERROR_EXPECTED_R_BRACE",
	"ERROR_EXPECTED_L_PAREN",
	"ERROR_EXPECTED_R_PAREN",
	"ERROR_EXPECTED_R_BRACKET",
	"ERROR_EXPECTED_SEMICOLON",
	"ERROR_EXPECTED_WHILE",
	"ERROR_EXPECTED_IDENTIFIER",
	"ERROR_MISMATCHED_PAREN",
	"ERROR_DUP_MAP_ELEM",
	"ERROR_INV_VAR_DECL",
	"ERROR_INV_BREAK",
	"ERROR_INV_CONTINUE",
	"ERROR_INV_EXPR",
	"ERROR_FN_NUM_ARGS",
	"ERROR_UNDEFINED_FN",
	"ERROR_UNDEFINED_VAR",
	"ERROR_UNDEFINED_MAIN",
	"ERROR_STACK_OVERFLOW",
	"ERROR_STACK_UNDERFLOW",
	"ERROR_INV_INSTR",
	"ERROR_INV_MEM_ACCESS"
};

#define TEXT_RED "\033[31;1m"
#define TEXT_WHITE "\033[0m"

int main(int argc, char **argv)
{
	u8 *program;
	i32 ret;
	int length;
	char *src;
	FILE *fp;
	if(argc < 2)
	{
		fprintf(stderr, "usage: ./ci filename\n");
		return 1;
	}

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

	if(!(program = calloc(1024, 1)))
	{
		fprintf(stderr, "error allocating memory for bytecode\n");
		return 1;
	}

	fclose(fp);
	if((ret = parser_parse(src, program)))
	{
		int i;
		char *s;
		printf("parse error: %s\n"
				"row: %d, col: %d\n",
				err_msgs[ABS(ret) - 1],
				_token.Pos.Row, _token.Pos.Col);

		for(i = 0, s = _lexer.LineBegin; *s != '\n' && s - src < length; ++s, ++i)
		{
			if(i == _token.Pos.Col)
			{
				fputs(TEXT_RED, stdout);
			}

			if(i == _lexer.Pos.Col)
			{
				fputs(TEXT_WHITE, stdout);
			}

			fputc(*s, stdout);
		}

		fputs(TEXT_WHITE, stdout);
		fputc('\n', stdout);
		return 1;
	}

	if((ret = interpreter_run(program)))
	{
		printf("runtime error: %s\n", err_msgs[ABS(ret) - 1]);
	}

	free(program);
	free(src);
	return 0;
}

