#ifndef __MAIN_H__
#define __MAIN_H__

#include "types.h"
#include "map.h"

/* maximum number of variables */
#define MAX_VARS 16

/* maximum number of fuctions */
#define MAX_FNS  16

/* maximum number of break and continue statements */
#define MAX_BC   16

#define ABS(n) (((n) < 0) ? -(n) : (n))

typedef enum TOKEN_TYPE
{
	TT_NULL,
	TT_NUMBER,
	TT_LITERAL,
	TT_IDENTIFIER,
	TT_ASSIGN,              /* assignment            ("=")  */
	TT_L_PAREN,             /* left parenthesis      ("(")  */
	TT_R_PAREN,             /* right parenthesis     (")")  */
	TT_L_BRACKET,           /* left square bracket   ("[")  */
	TT_R_BRACKET,           /* right square bracket  ("]")  */
	TT_L_BRACE,             /* left curly brace      ("{")  */
	TT_R_BRACE,             /* right curly brace     ("}")  */
	TT_COMMA,
	TT_SEMICOLON,
	TT_COLON,

	TT_IF,
	TT_ELSE,
	TT_CASE,
	TT_DO,
	TT_WHILE,
	TT_FOR,
	TT_INT,
	TT_VOID,
	TT_BREAK,
	TT_CONTINUE,
	TT_RETURN,
	TT_PRINT,
	TT_GET,

	OPERATOR_START = 128,
	TT_L_NOT,               /* logical not           ("!")  */
	TT_L_OR,                /* logical or            ("||") */
	TT_L_AND,               /* locical and           ("&&") */
	TT_B_NOT,               /* bitwise not           ("~")  */
	TT_B_OR,                /* bitwise or            ("|")  */
	TT_B_XOR,               /* bitwise exclusive or  ("^")  */
	TT_B_AND,               /* bitwise and           ("&")  */
	TT_EQ,                  /* equal                 ("==") */
	TT_NE,                  /* not equal             ("!=") */
	TT_LT,                  /* less than             ("<")  */
	TT_GT,                  /* greater than          (">")  */
	TT_LE,                  /* less than or equal    ("<=") */
	TT_GE,                  /* greater than or equal (">=") */
	TT_B_SHL,               /* bitwise shift left    ("<<") */
	TT_B_SHR,               /* bitwise shift right   (">>") */
	TT_ADD,                 /* add                   ("+")  */
	TT_SUB,                 /* subtract              ("-")  */
	TT_MUL,                 /* multiply              ("*")  */
	TT_DIV,                 /* divide                ("/")  */
	TT_MOD,                 /* modulo                ("%")  */
	TT_U_MINUS,             /* unary minus           ("-")  */
	OPERATOR_END
} TokenType;

typedef struct POSITION
{
	i32 Col, Row;
} Position;

typedef struct TOKEN
{
	Position Pos;
	TokenType Type;
	union
	{
		i32 Number;
		struct
		{
			u32 Length;
			char *Name;
		} Identifier;
	} Value;
} Token;

typedef struct LEXER
{
	char *LineBegin, *Ptr;
	Position Pos;
} Lexer;

extern Lexer _lexer;
extern Token _token;

void lexer_init(char *src);
void lexer_advance(void);
i32 lexer_next(void);
void lexer_print_token(Token *token);

enum INSTRUCTION
{
	INSTR_HALT,
	INSTR_PUSHI,
	INSTR_PUSHL,
	INSTR_POPL,
	INSTR_PUSHA,
	INSTR_POPA,
	INSTR_JZ,
	INSTR_JNZ,
	INSTR_JMP,
	INSTR_POPP,
	INSTR_POP,
	INSTR_CALL,
	INSTR_RET,
	INSTR_PRINTI,
	INSTR_GETI,
	INSTR_ISP,
	INSTR_DUP
};

i32 interpreter_run(u8 *d);

enum
{
	ERROR_UNEXPECTED_EOF = 1,
	ERROR_UNEXPECTED_TOKEN,
	ERROR_EXPECTED_L_BRACE,
	ERROR_EXPECTED_R_BRACE,
	ERROR_EXPECTED_L_PAREN,
	ERROR_EXPECTED_R_PAREN,
	ERROR_EXPECTED_R_BRACKET,
	ERROR_EXPECTED_SEMICOLON,
	ERROR_EXPECTED_WHILE,
	ERROR_EXPECTED_IDENTIFIER,
	ERROR_MISMATCHED_PAREN,
	ERROR_DUP_MAP_ELEM,
	ERROR_INV_VAR_DECL,
	ERROR_INV_BREAK,
	ERROR_INV_CONTINUE,
	ERROR_INV_EXPR,
	ERROR_FN_NUM_ARGS,
	ERROR_UNDEFINED_FN,
	ERROR_UNDEFINED_VAR,
	ERROR_UNDEFINED_MAIN,
	ERROR_STACK_OVERFLOW,
	ERROR_STACK_UNDERFLOW,
	ERROR_INV_INSTR,
	ERROR_INV_MEM_ACCESS
};

i32 parser_parse(char *src, u8 *out);

u32 read_32(u8 *p);
void write_32(u8 *b, u32 v);

extern const char *err_msgs[];

#endif

