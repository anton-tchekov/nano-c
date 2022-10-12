#ifndef __NANOC_ERROR_H__
#define __NANOC_ERROR_H__

#if defined(NANOC_DEBUG) && NANOC_DEBUG != 0

#define TRACE(e) \
	do \
	{ \
		i32 ret = -e; \
		fprintf(stderr, "[ Trace ] %s - %s:%d (%s)\n", nanoc_error_message(ret), __FILE__, __LINE__, __func__); \
		return ret; \
	} while(0)

#define return_if(E) \
	do \
	{ \
		i32 ret; \
		if((ret = (E)) < 0) \
		{ \
			fprintf(stderr, "[ Trace ] %s - %s:%d (%s)\n", nanoc_error_message(ret), __FILE__, __LINE__, __func__); \
			return ret; \
		} \
	} while(0)

#else

#define TRACE(e) \
	do \
	{ \
		return -e; \
	} while(0)

#define return_if(E) \
	do \
	{ \
		i32 ret; \
		if((ret = (E)) < 0) \
		{ \
			return ret; \
		} \
	} while(0)

#endif

enum NANOC_ERROR
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
	ERROR_EXPECTED_TYPE,
	ERROR_EXPECTED_L_BRACKET,
	ERROR_EXPECTED_INT,
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
	ERROR_INV_MEM_ACCESS,
	ERROR_INV_ESCAPE_SEQUENCE,
	ERROR_UNTERMINATED_CHAR_LITERAL,
	ERROR_UNEXPECTED_CHARACTER,
	ERROR_UNTERMINATED_STRING_LITERAL
};

#endif /* __NANOC_ERROR_H__ */
