#ifndef __NANOC_H__
#define __NANOC_H__


/* feature selection */
#define NANOC_DEBUG               1
#define NANOC_ENABLE_COMPILER     1
#define NANOC_ENABLE_INTERPRETER  1

/* maximum number of variables in scope at the same time */
#define NANOC_MAX_VARS           32

/* maximum number of fuctions */
#define NANOC_MAX_FNS            32
#define NANOC_MAX_NATIVE_FNS     32

/* maximum number of break and continue statements */
#define NANOC_MAX_BC             32

/* maximum number of operators in expression */
#define NANOC_MAX_OP             32

/* interpreter */
#define NANOC_CALL_STACK_SIZE  4096
#define NANOC_OP_STACK_SIZE    1024
#define NANOC_HEAP_SIZE        (16 * 1024)


#include "types.h"
#include "util.h"
#include "map.h"
#include "error.h"


/* token */
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
	TT_BACKSLASH,

	TT_IF,
	TT_ELSE,
	TT_DO,
	TT_WHILE,
	TT_FOR,
	TT_INT,
	TT_I8,
	TT_I16,
	TT_I32,
	TT_BREAK,
	TT_CONTINUE,
	TT_RETURN,

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

#if defined(NANOC_ENABLE_COMPILER) && NANOC_ENABLE_COMPILER != 0
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
			const char *Name;
		} Identifier;
	} Value;
} Token;

/* lexer */
typedef struct LEXER
{
	i32 StringsOffset;
	char *Strings, *StringsPtr;
	const char *LineBegin, *Ptr;
	Position Pos;
} Lexer;

void _lexer_init(Lexer *lexer, const char *src, char *strings, i32 offset);
void _lexer_advance(Lexer *lexer);
i32 _lexer_next(Lexer *lexer, Token *token);

/* parser */
typedef struct PARSER
{
	MapElement VariableBuffer[NANOC_MAX_VARS], FunctionBuffer[NANOC_MAX_FNS];
	Map Variables, Functions;

	u8 FN_Args[NANOC_MAX_FNS];
	u16 FN_Addrs[NANOC_MAX_FNS];

	u16 FunctionUsages[NANOC_MAX_FNS];
	u32 UsagesCount;

	/* bytecode output */
	u8 *Output;
	u32 Index;

	/* break jump address stack. continue jump address stack */
	u16 BAS[NANOC_MAX_BC], CAS[NANOC_MAX_BC];

	/* break address stack top, continue address stack top */
	u32 BT, CT;

	u32 BreakNesting, ContinueNesting;

} Parser;
#endif

/* instruction */
enum INSTRUCTION
{
	INSTR_HALT,
	INSTR_PUSHI8,
	INSTR_PUSHI16,
	INSTR_PUSHI24,
	INSTR_PUSHI32,
	INSTR_PUSHL,
	INSTR_POPL,
	INSTR_PUSHA8,
	INSTR_PUSHA16,
	INSTR_PUSHA24,
	INSTR_PUSHA32,
	INSTR_POPA8,
	INSTR_POPA16,
	INSTR_POPA24,
	INSTR_POPA32,
	INSTR_JZ,
	INSTR_JNZ,
	INSTR_JMP,
	INSTR_CALL,
	INSTR_RET,
	INSTR_ISP,
	INSTR_DUP,
	INSTR_POP,
};

#if defined(NANOC_ENABLE_INTERPRETER) && NANOC_ENABLE_INTERPRETER != 0
/* interpreter */
typedef struct INTERPRETER
{
	i32 (*Functions[NANOC_MAX_NATIVE_FNS])(i32 *parameter, u8 *heap);
	u8 *Heap;
	i32 OperatorStack[NANOC_OP_STACK_SIZE];
	u32 CallStack[NANOC_CALL_STACK_SIZE];
	i32 OP; /* op stack top */
	u32 SP, /* call stack pointer */
		FP, /* frame pointer */
		IP; /* instruction pointer */
} Interpreter;
#endif

typedef struct NANOC
{
#if defined(NANOC_ENABLE_COMPILER) && NANOC_ENABLE_COMPILER != 0
	Lexer Lexer;
	Token Token;
	Parser Parser;
#endif

#if defined(NANOC_ENABLE_INTERPRETER) && NANOC_ENABLE_INTERPRETER != 0
	Interpreter Interpreter;
#endif
} NanoC;

void nanoc_init(NanoC *n);
i32 nanoc_add_function(NanoC *n, u16 id, const char *name, u8 num_args, i32 (*function)(i32 *parameter, u8 *heap));

#if defined(NANOC_ENABLE_COMPILER) && NANOC_ENABLE_COMPILER != 0
i32 nanoc_compile(NanoC *n, const char *src, u8 *out, u8 *strings, i32 strings_offset);
#endif

void nanoc_disasm(u8 *program, i32 len);

#if defined(NANOC_ENABLE_INTERPRETER) && NANOC_ENABLE_INTERPRETER != 0
i32 nanoc_run(NanoC *n, u8 *program, u8 *data);
#endif

#if defined(NANOC_DEBUG) && NANOC_DEBUG != 0
const char *nanoc_error_message(i32 code);
#endif

#endif
