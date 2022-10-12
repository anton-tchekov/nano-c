#include "nanoc.h"

void nanoc_init(NanoC *n)
{
#if defined(NANOC_ENABLE_COMPILER) && NANOC_ENABLE_COMPILER != 0
	_map_init(&n->Parser.Variables, n->Parser.VariableBuffer, sizeof(n->Parser.VariableBuffer) / sizeof(*n->Parser.VariableBuffer));
	_map_init(&n->Parser.Functions, n->Parser.FunctionBuffer, sizeof(n->Parser.FunctionBuffer) / sizeof(*n->Parser.FunctionBuffer));
#endif

#if defined(NANOC_ENABLE_INTERPRETER) && NANOC_ENABLE_INTERPRETER != 0

#endif
}

#if defined(NANOC_ENABLE_COMPILER) && NANOC_ENABLE_COMPILER != 0

#include <stdio.h>
#include <string.h>

#define IS_OPERATOR(T) ((T) > OPERATOR_START && (T) < OPERATOR_END)

static i32 _get_precedence(TokenType tt);
static i32 _parser_function(NanoC *n, const char *name, u32 len);
static i32 _parser_block(NanoC *n);
static i32 _parser_statement(NanoC *n);
static i32 _parser_substmt(NanoC *n, TokenType term);
static i32 _parser_fncall(NanoC *n, i32 i);
static i32 _parser_fnimpl(NanoC *n, const char *name, u32 len);
static i32 _parser_expression(NanoC *n);
static i32 _parser_type_annotation(NanoC *n);
static void _handle_break(NanoC *n, u32 prev, u32 addr_break);
static void _handle_continue(NanoC *n, u32 prev, u32 addr_continue);

i32 nanoc_compile(NanoC *n, const char *src, u8 *out, u8 *strings, i32 string_offset)
{
	i32 i;
	u32 idx_main, len;
	const char *name;

	n->Parser.Output = out;
	_lexer_init(&n->Lexer, src, (char *)strings, string_offset);
	n->Parser.Output[n->Parser.Index++] = INSTR_CALL;
	n->Parser.Output[n->Parser.Index++] = 0;
	idx_main = n->Parser.Index;
	n->Parser.Index += 2;
	n->Parser.Output[n->Parser.Index++] = INSTR_HALT;
	for(;;)
	{
		return_if(_lexer_next(&n->Lexer, &n->Token));
		if(n->Token.Type == TT_NULL)
		{
			break;
		}
		else if(n->Token.Type != TT_INT)
		{
			TRACE(ERROR_EXPECTED_INT);
		}

		return_if(_lexer_next(&n->Lexer, &n->Token));
		if(n->Token.Type != TT_IDENTIFIER)
		{
			TRACE(ERROR_EXPECTED_IDENTIFIER);
		}

		name = n->Token.Value.Identifier.Name;
		len = n->Token.Value.Identifier.Length;
		return_if(_lexer_next(&n->Lexer, &n->Token));
		if(n->Token.Type != TT_L_PAREN)
		{
			TRACE(ERROR_EXPECTED_L_PAREN);
		}

		return_if(_parser_function(n, name, len));
	}

	/* check that all functions are implemented */
	for(i = 0; i < (i32)n->Parser.Functions.Count; ++i)
	{
		if(n->Parser.FN_Addrs[i] == 0)
		{
			TRACE(ERROR_UNDEFINED_FN);
		}
	}

	if((i = _map_find(&n->Parser.Functions, "main", 4)) < 0)
	{
		TRACE(ERROR_UNDEFINED_MAIN);
	}

	_write_16(n->Parser.Output + idx_main, n->Parser.FN_Addrs[i]);
	return 0;
}

static i32 _get_precedence(TokenType tt)
{
	switch(tt)
	{
		case TT_L_OR:
			return 11;

		case TT_L_AND:
			return 10;

		case TT_B_OR:
			return 9;

		case TT_B_XOR:
			return 8;

		case TT_B_AND:
			return 7;

		case TT_EQ:
		case TT_NE:
			return 6;

		case TT_LT:
		case TT_GT:
		case TT_LE:
		case TT_GE:
			return 5;

		case TT_B_SHL:
		case TT_B_SHR:
			return 4;

		case TT_ADD:
		case TT_SUB:
			return 3;

		case TT_MUL:
		case TT_DIV:
		case TT_MOD:
			return 2;

		case TT_L_NOT:
		case TT_B_NOT:
		case TT_U_MINUS:
			return 1;

		default:
			break;
	}

	return 0;
}

static i32 _parser_function(NanoC *n, const char *name, u32 len)
{
	i32 i, impl;
	u8 args;

	impl = 0;
	args = 0;
	if((i = _map_insert(&n->Parser.Functions, name, len)) < 0)
	{
		if(i == -ERROR_DUP_MAP_ELEM)
		{
			if((i = _map_find(&n->Parser.Functions, name, len)) < 0)
			{
				TRACE(ERROR_DUP_MAP_ELEM);
			}

			if(n->Parser.FN_Addrs[i] == 0)
			{
				impl = 1;
			}
			else
			{
				TRACE(ERROR_DUP_MAP_ELEM);
			}
		}
	}

	n->Parser.FN_Addrs[i] = n->Parser.Index;
	return_if(_lexer_next(&n->Lexer, &n->Token));
	for(;;)
	{
		if(n->Token.Type == TT_INT)
		{
			return_if(_lexer_next(&n->Lexer, &n->Token));
			if(n->Token.Type != TT_IDENTIFIER)
			{
				TRACE(ERROR_EXPECTED_IDENTIFIER);
			}

			return_if(_map_insert(&n->Parser.Variables, n->Token.Value.Identifier.Name,
				n->Token.Value.Identifier.Length));

			++args;
			return_if(_lexer_next(&n->Lexer, &n->Token));
			if(n->Token.Type == TT_COMMA)
			{
				return_if(_lexer_next(&n->Lexer, &n->Token));
			}
		}
		else if(n->Token.Type == TT_R_PAREN)
		{
			break;
		}
		else
		{
			TRACE(ERROR_UNEXPECTED_TOKEN);
		}
	}

	return_if(_lexer_next(&n->Lexer, &n->Token));
	if(n->Token.Type != TT_L_BRACE)
	{
		TRACE(ERROR_EXPECTED_L_BRACE);
	}

	if(impl)
	{
		if(args != n->Parser.FN_Args[i])
		{
			TRACE(ERROR_FN_NUM_ARGS);
		}

		{
			u32 k;
			for(k = 0; k < n->Parser.UsagesCount; ++k)
			{
				if(_read_16(&n->Parser.Output[n->Parser.FunctionUsages[k] + 2]) == i)
				{
					_write_16(&n->Parser.Output[n->Parser.FunctionUsages[k] + 2], n->Parser.FN_Addrs[i]);
				}
			}
		}
	}

	n->Parser.FN_Args[i] = args;

	if(args)
	{
		n->Parser.Output[n->Parser.Index++] = INSTR_ISP;
		n->Parser.Output[n->Parser.Index++] = args;
	}

	return_if(_parser_block(n));

	if(n->Parser.Output[n->Parser.Index - 1] != INSTR_RET)
	{
		n->Parser.Output[n->Parser.Index++] = INSTR_PUSHI8;
		n->Parser.Output[n->Parser.Index++] = 0;
		n->Parser.Output[n->Parser.Index++] = INSTR_RET;
	}

	n->Parser.Variables.Count -= args;
	return 0;
}

static i32 _parser_block(NanoC *n)
{
	i32 lvc;
	lvc = 0;
	for(;;)
	{
		return_if(_lexer_next(&n->Lexer, &n->Token));
		if(n->Token.Type == TT_INT)
		{
			do
			{
				i32 i;
				return_if(_lexer_next(&n->Lexer, &n->Token));
				if(n->Token.Type != TT_IDENTIFIER)
				{
					TRACE(ERROR_INV_VAR_DECL);
				}

				return_if(i = _map_insert(&n->Parser.Variables, n->Token.Value.Identifier.Name,
					n->Token.Value.Identifier.Length));

				++lvc;
				return_if(_lexer_next(&n->Lexer, &n->Token));
			}
			while(n->Token.Type == TT_COMMA);
			if(n->Token.Type != TT_SEMICOLON)
			{
				TRACE(ERROR_EXPECTED_SEMICOLON);
			}
		}
		else
		{
			break;
		}
	}

	if(lvc)
	{
		if(n->Parser.Output[n->Parser.Index - 2] == INSTR_ISP)
		{
			n->Parser.Output[n->Parser.Index - 1] += lvc;
		}
		else
		{
			n->Parser.Output[n->Parser.Index++] = INSTR_ISP;
			n->Parser.Output[n->Parser.Index++] = lvc;
		}
	}

	while(n->Token.Type != TT_R_BRACE)
	{
		return_if(_parser_statement(n));
		return_if(_lexer_next(&n->Lexer, &n->Token));
	}

	n->Parser.Variables.Count -= lvc;
	return 0;
}

static void _handle_break(NanoC *n, u32 prev, u32 addr_break)
{
	while(n->Parser.BT > prev)
	{
		--n->Parser.BT;
		_write_16(n->Parser.Output + n->Parser.BAS[n->Parser.BT], addr_break);
	}
}

static void _handle_continue(NanoC *n, u32 prev, u32 addr_continue)
{
	while(n->Parser.CT > prev)
	{
		--n->Parser.CT;
		_write_16(n->Parser.Output + n->Parser.CAS[n->Parser.CT], addr_continue);
	}
}

static i32 _parser_statement(NanoC *n)
{
	switch(n->Token.Type)
	{
	case TT_NULL:
		TRACE(ERROR_UNEXPECTED_EOF);

	case TT_SEMICOLON:
		break;

	case TT_IF:
	{
		Lexer state;
		u32 idx_after, idx_end;
		return_if(_lexer_next(&n->Lexer, &n->Token));
		if(n->Token.Type != TT_L_PAREN)
		{
			TRACE(ERROR_EXPECTED_L_PAREN);
		}

		return_if(_lexer_next(&n->Lexer, &n->Token));
		return_if(_parser_expression(n));
		n->Parser.Output[n->Parser.Index++] = INSTR_JZ;
		idx_after = n->Parser.Index;
		n->Parser.Index += 2;
		return_if(_lexer_next(&n->Lexer, &n->Token));
		return_if(_parser_statement(n));
		state = n->Lexer;
		return_if(_lexer_next(&n->Lexer, &n->Token));
		if(n->Token.Type == TT_ELSE)
		{
			n->Parser.Output[n->Parser.Index++] = INSTR_JMP;
			idx_end = n->Parser.Index;
			n->Parser.Index += 2;
			_write_16(n->Parser.Output + idx_after, n->Parser.Index);
			return_if(_lexer_next(&n->Lexer, &n->Token));
			return_if(_parser_statement(n));
			_write_16(n->Parser.Output + idx_end, n->Parser.Index);
		}
		else
		{
			n->Lexer = state;
			_write_16(n->Parser.Output + idx_after, n->Parser.Index);
		}
		break;
	}

	case TT_WHILE:
	{
		u32 idx_cp, idx_after, prev_break, prev_continue;
		idx_cp = n->Parser.Index;
		return_if(_lexer_next(&n->Lexer, &n->Token));
		if(n->Token.Type != TT_L_PAREN)
		{
			TRACE(ERROR_EXPECTED_L_PAREN);
		}

		return_if(_lexer_next(&n->Lexer, &n->Token));
		return_if(_parser_expression(n));
		n->Parser.Output[n->Parser.Index++] = INSTR_JZ;
		idx_after = n->Parser.Index;
		n->Parser.Index += 2;
		return_if(_lexer_next(&n->Lexer, &n->Token));
		prev_break = n->Parser.BT;
		prev_continue = n->Parser.CT;
		++n->Parser.BreakNesting;
		++n->Parser.ContinueNesting;
		return_if(_parser_statement(n));
		--n->Parser.BreakNesting;
		--n->Parser.ContinueNesting;
		n->Parser.Output[n->Parser.Index++] = INSTR_JMP;
		_write_16(n->Parser.Output + n->Parser.Index, idx_cp);
		n->Parser.Index += 2;
		_write_16(n->Parser.Output + idx_after, n->Parser.Index);
		_handle_break(n, prev_break, n->Parser.Index);
		_handle_continue(n, prev_continue, idx_cp);
		break;
	}

	case TT_DO:
	{
		u32 idx_begin, idx_cp, prev_break, prev_continue;
		idx_begin = n->Parser.Index;
		return_if(_lexer_next(&n->Lexer, &n->Token));
		prev_break = n->Parser.BT;
		prev_continue = n->Parser.CT;
		++n->Parser.BreakNesting;
		++n->Parser.ContinueNesting;
		return_if(_parser_statement(n));
		--n->Parser.BreakNesting;
		--n->Parser.ContinueNesting;
		return_if(_lexer_next(&n->Lexer, &n->Token));
		if(n->Token.Type != TT_WHILE)
		{
			TRACE(ERROR_EXPECTED_WHILE);
		}

		return_if(_lexer_next(&n->Lexer, &n->Token));
		if(n->Token.Type != TT_L_PAREN)
		{
			TRACE(ERROR_EXPECTED_L_PAREN);
		}

		return_if(_lexer_next(&n->Lexer, &n->Token));
		return_if(_parser_expression(n));
		if(n->Token.Type != TT_R_PAREN)
		{
			TRACE(ERROR_EXPECTED_R_PAREN);
		}

		return_if(_lexer_next(&n->Lexer, &n->Token));
		if(n->Token.Type != TT_SEMICOLON)
		{
			TRACE(ERROR_EXPECTED_SEMICOLON);
		}

		idx_cp = n->Parser.Index;
		n->Parser.Output[n->Parser.Index++] = INSTR_JNZ;
		_write_16(n->Parser.Output + n->Parser.Index, idx_begin);
		n->Parser.Index += 2;
		_handle_break(n, prev_break, n->Parser.Index);
		_handle_continue(n, prev_continue, idx_cp);
		break;
	}

	case TT_FOR:
	{
		Lexer save, save2;
		u32 idx_inc, idx_cp, idx_end, prev_break, prev_continue;
		return_if(_lexer_next(&n->Lexer, &n->Token));
		if(n->Token.Type != TT_L_PAREN)
		{
			TRACE(ERROR_EXPECTED_L_PAREN);
		}

		return_if(_lexer_next(&n->Lexer, &n->Token));
		return_if(_parser_substmt(n, TT_SEMICOLON));
		return_if(_lexer_next(&n->Lexer, &n->Token));
		idx_cp = n->Parser.Index;
		if(n->Token.Type == TT_SEMICOLON)
		{
			n->Parser.Output[n->Parser.Index++] = INSTR_PUSHI8;
			n->Parser.Output[n->Parser.Index++] = 1;
		}
		else
		{
			return_if(_parser_expression(n));
		}

		n->Parser.Output[n->Parser.Index++] = INSTR_JZ;
		idx_end = n->Parser.Index;
		n->Parser.Index += 2;
		save = n->Lexer;
		return_if(_lexer_next(&n->Lexer, &n->Token));
		while(n->Token.Type != TT_R_PAREN)
		{
			return_if(_lexer_next(&n->Lexer, &n->Token));
			if(n->Token.Type == TT_NULL)
			{
				TRACE(ERROR_UNEXPECTED_EOF);
			}
		}

		return_if(_lexer_next(&n->Lexer, &n->Token));
		prev_break = n->Parser.BT;
		prev_continue = n->Parser.CT;
		++n->Parser.BreakNesting;
		++n->Parser.ContinueNesting;
		return_if(_parser_statement(n));
		--n->Parser.BreakNesting;
		--n->Parser.ContinueNesting;
		idx_inc = n->Parser.Index;
		save2 = n->Lexer;
		n->Lexer = save;
		return_if(_lexer_next(&n->Lexer, &n->Token));
		return_if(_parser_substmt(n, TT_R_PAREN));
		n->Lexer = save2;
		n->Parser.Output[n->Parser.Index++] = INSTR_JMP;
		_write_16(n->Parser.Output + n->Parser.Index, idx_cp);
		n->Parser.Index += 2;
		_write_16(n->Parser.Output + idx_end, n->Parser.Index);
		_handle_break(n, prev_break, n->Parser.Index);
		_handle_continue(n, prev_continue, idx_inc);
		break;
	}

	case TT_BREAK:
		return_if(_lexer_next(&n->Lexer, &n->Token));
		if(n->Token.Type != TT_SEMICOLON)
		{
			TRACE(ERROR_EXPECTED_SEMICOLON);
		}

		if(!n->Parser.BreakNesting)
		{
			TRACE(ERROR_INV_BREAK);
		}

		n->Parser.Output[n->Parser.Index++] = INSTR_JMP;
		n->Parser.BAS[n->Parser.BT++] = n->Parser.Index;
		n->Parser.Index += 2;
		break;

	case TT_CONTINUE:
		return_if(_lexer_next(&n->Lexer, &n->Token));
		if(n->Token.Type != TT_SEMICOLON)
		{
			TRACE(ERROR_EXPECTED_SEMICOLON);
		}

		if(!n->Parser.ContinueNesting)
		{
			TRACE(ERROR_INV_CONTINUE);
		}

		n->Parser.Output[n->Parser.Index++] = INSTR_JMP;
		n->Parser.CAS[n->Parser.CT++] = n->Parser.Index;
		n->Parser.Index += 2;
		break;

	case TT_RETURN:
		return_if(_lexer_next(&n->Lexer, &n->Token));
		return_if(_parser_expression(n));
		n->Parser.Output[n->Parser.Index++] = INSTR_RET;
		break;

	case TT_L_BRACE:
		return_if(_parser_block(n));
		break;

	default:
		return_if(_parser_substmt(n, TT_SEMICOLON));
	}

	return 0;
}

static i32 _parser_type_annotation(NanoC *n)
{
	i32 ret = 4;
	if(n->Token.Type == TT_BACKSLASH)
	{
		return_if(_lexer_next(&n->Lexer, &n->Token));
		switch(n->Token.Type)
		{
			case TT_I8:
				ret = 1;
				break;

			case TT_I16:
				ret = 2;
				break;

			case TT_I32:
				break;

			default:
				TRACE(ERROR_EXPECTED_TYPE);
				break;
		}

		return_if(_lexer_next(&n->Lexer, &n->Token));
		if(n->Token.Type != TT_L_BRACKET)
		{
			TRACE(ERROR_EXPECTED_L_BRACKET);
		}
	}

	return ret;
}

static i32 _parser_substmt(NanoC *n, TokenType term)
{
	i32 i;
	if(n->Token.Type == term)
	{
		return 0;
	}

	for(;;)
	{
		Token prev;
		if(n->Token.Type != TT_IDENTIFIER)
		{
			TRACE(ERROR_UNEXPECTED_TOKEN);
		}

		prev = n->Token;
		return_if(_lexer_next(&n->Lexer, &n->Token));
		if(n->Token.Type == TT_L_PAREN)
		{
			if((i = _map_find(&n->Parser.Functions, prev.Value.Identifier.Name,
				prev.Value.Identifier.Length)) < 0)
			{
				return_if(_parser_fnimpl(n, prev.Value.Identifier.Name, prev.Value.Identifier.Length));
			}
			else
			{
				return_if(_parser_fncall(n, i));
			}

			n->Parser.Output[n->Parser.Index++] = INSTR_POP;

			return_if(_lexer_next(&n->Lexer, &n->Token));
		}
		else
		{
			u8 bytes;
			u32 arr;
			TokenType at;
			if((i = _map_find(&n->Parser.Variables, prev.Value.Identifier.Name,
				prev.Value.Identifier.Length)) < 0)
			{
				TRACE(ERROR_UNDEFINED_VAR);
			}

			arr = 0;
			at = TT_NULL;
			return_if(bytes = _parser_type_annotation(n));

			if(n->Token.Type == TT_L_BRACKET)
			{
				return_if(_lexer_next(&n->Lexer, &n->Token));
				return_if(_parser_expression(n));
				if(n->Token.Type != TT_R_BRACKET)
				{
					TRACE(ERROR_EXPECTED_R_BRACKET);
				}

				return_if(_lexer_next(&n->Lexer, &n->Token));

				n->Parser.Output[n->Parser.Index++] = INSTR_PUSHI8;
				n->Parser.Output[n->Parser.Index++] = bytes;
				n->Parser.Output[n->Parser.Index++] = TT_MUL;
				n->Parser.Output[n->Parser.Index++] = INSTR_PUSHL;
				n->Parser.Output[n->Parser.Index++] = (u8)(i & 0xFF);
				n->Parser.Output[n->Parser.Index++] = TT_ADD;
				arr = 1;
			}

			if(n->Token.Type == TT_ADD || n->Token.Type == TT_SUB ||
				n->Token.Type == TT_MUL || n->Token.Type == TT_DIV ||
				n->Token.Type == TT_MOD || n->Token.Type == TT_B_SHL ||
				n->Token.Type == TT_B_SHR || n->Token.Type == TT_B_AND ||
				n->Token.Type == TT_B_OR || n->Token.Type == TT_B_XOR)
			{
				at = n->Token.Type;
				return_if(_lexer_next(&n->Lexer, &n->Token));
				if(arr)
				{
					n->Parser.Output[n->Parser.Index++] = INSTR_DUP;
					n->Parser.Output[n->Parser.Index++] = INSTR_PUSHA8 + bytes - 1;
				}
				else
				{
					n->Parser.Output[n->Parser.Index++] = INSTR_PUSHL;
					n->Parser.Output[n->Parser.Index++] = (u8)(i & 0xFF);
				}
			}

			if(n->Token.Type != TT_ASSIGN)
			{
				TRACE(ERROR_UNEXPECTED_TOKEN);
			}

			return_if(_lexer_next(&n->Lexer, &n->Token));
			return_if(_parser_expression(n));
			if(at)
			{
				n->Parser.Output[n->Parser.Index++] = at;
			}

			if(arr)
			{
				n->Parser.Output[n->Parser.Index++] = INSTR_POPA8 + bytes - 1;
			}
			else
			{
				n->Parser.Output[n->Parser.Index++] = INSTR_POPL;
				n->Parser.Output[n->Parser.Index++] = (u8)(i & 0xFF);
			}
		}

		if(n->Token.Type == TT_COMMA)
		{
			return_if(_lexer_next(&n->Lexer, &n->Token));
			continue;
		}
		else if(n->Token.Type == term)
		{
			break;
		}
		else
		{
			TRACE(ERROR_UNEXPECTED_TOKEN);
		}
	}

	return 0;
}

i32 nanoc_add_function(NanoC *n, u16 id, const char *name, u8 num_args, i32 (*function)(i32 *parameters, u8 *heap))
{
#if defined(NANOC_ENABLE_COMPILER) && NANOC_ENABLE_COMPILER != 0
	i32 i;
	return_if(i = _map_insert(&n->Parser.Functions, name, strlen(name)));
	n->Parser.FN_Addrs[i] = -id - 1;
	n->Parser.FN_Args[i] = num_args;
#endif

#if defined(NANOC_ENABLE_INTERPRETER) && NANOC_ENABLE_INTERPRETER != 0
	n->Interpreter.Functions[id] = function;
#endif

	return 0;
}

static i32 _parser_fnimpl(NanoC *n, const char *name, u32 len)
{
	i16 i;
	u32 arg_cnt;

	arg_cnt = 0;
	return_if(_lexer_next(&n->Lexer, &n->Token));
	if(n->Token.Type != TT_R_PAREN)
	{
		for(;;)
		{
			return_if(_parser_expression(n));
			++arg_cnt;
			if(n->Token.Type == TT_R_PAREN)
			{
				break;
			}
			else if(n->Token.Type == TT_COMMA)
			{
				return_if(_lexer_next(&n->Lexer, &n->Token));
			}
			else
			{
				TRACE(ERROR_UNEXPECTED_TOKEN);
			}
		}
	}

	return_if(i = _map_insert(&n->Parser.Functions, name, len));
	n->Parser.FN_Addrs[i] = 0;
	n->Parser.FN_Args[i] = arg_cnt;

	n->Parser.FunctionUsages[n->Parser.UsagesCount++] = n->Parser.Index;

	n->Parser.Output[n->Parser.Index++] = INSTR_CALL;
	n->Parser.Output[n->Parser.Index++] = arg_cnt;
	_write_16(n->Parser.Output + n->Parser.Index, i);
	n->Parser.Index += 2;
	return 0;
}

static i32 _parser_fncall(NanoC *n, i32 i)
{
	u32 arg_cnt;

	arg_cnt = 0;
	return_if(_lexer_next(&n->Lexer, &n->Token));
	if(n->Token.Type != TT_R_PAREN)
	{
		for(;;)
		{
			return_if(_parser_expression(n));
			++arg_cnt;
			if(n->Token.Type == TT_R_PAREN)
			{
				break;
			}
			else if(n->Token.Type == TT_COMMA)
			{
				return_if(_lexer_next(&n->Lexer, &n->Token));
			}
			else
			{
				TRACE(ERROR_UNEXPECTED_TOKEN);
			}
		}
	}

	if(arg_cnt != n->Parser.FN_Args[i])
	{
		TRACE(ERROR_FN_NUM_ARGS);
	}

	n->Parser.Output[n->Parser.Index++] = INSTR_CALL;
	n->Parser.Output[n->Parser.Index++] = arg_cnt;
	_write_16(n->Parser.Output + n->Parser.Index, n->Parser.FN_Addrs[i]);
	n->Parser.Index += 2;
	return 0;
}

static i32 _parser_expression(NanoC *n)
{
	TokenType prev, op_stack[NANOC_MAX_OP];
	u32 idx_old, paren_cnt, cnt;
	size_t si;

	prev = TT_NULL;
	paren_cnt = 1;
	si = 0;
	idx_old = n->Parser.Index;
	cnt = 0;

	for(;;)
	{
		switch(n->Token.Type)
		{
			case TT_NUMBER:
			{
				u32 value;
				if(prev == TT_NUMBER || prev == TT_IDENTIFIER ||
						prev == TT_R_PAREN)
				{
					TRACE(ERROR_UNEXPECTED_TOKEN);
				}

				++cnt;

				value = n->Token.Value.Number;
				if(value <= 0xFF)
				{
					n->Parser.Output[n->Parser.Index++] = INSTR_PUSHI8;
					n->Parser.Output[n->Parser.Index++] = value;
				}
				else if(value <= 0xFFFF)
				{
					n->Parser.Output[n->Parser.Index++] = INSTR_PUSHI16;
					_write_32(n->Parser.Output + n->Parser.Index, value);
					n->Parser.Index += 2;
				}
				else
				{
					n->Parser.Output[n->Parser.Index++] = INSTR_PUSHI32;
					_write_32(n->Parser.Output + n->Parser.Index, value);
					n->Parser.Index += 4;
				}

				break;
			}

			case TT_IDENTIFIER:
			{
				Token tok;
				Lexer lex;
				i32 i;
				if(prev == TT_NUMBER || prev == TT_IDENTIFIER ||
						prev == TT_R_PAREN)
				{
					TRACE(ERROR_UNEXPECTED_TOKEN);
				}

				tok = n->Token;
				prev = n->Token.Type;
				lex = n->Lexer;
				return_if(_lexer_next(&n->Lexer, &n->Token));
				if(n->Token.Type == TT_L_PAREN)
				{
					if((i = _map_find(&n->Parser.Functions, tok.Value.Identifier.Name,
						tok.Value.Identifier.Length)) < 0)
					{
						return_if(_parser_fnimpl(n, tok.Value.Identifier.Name, tok.Value.Identifier.Length));
					}
					else
					{
						return_if(_parser_fncall(n, i));
					}
				}
				else
				{
					n->Lexer = lex;
					n->Token = tok;
					if((i = _map_find(&n->Parser.Variables, tok.Value.Identifier.Name,
						tok.Value.Identifier.Length)) < 0)
					{
						TRACE(ERROR_UNDEFINED_VAR);
					}

					++cnt;
					n->Parser.Output[n->Parser.Index++] = INSTR_PUSHL;
					n->Parser.Output[n->Parser.Index++] = (u8)(i & 0xFF);
				}

				break;
			}

			case TT_BACKSLASH:
			case TT_L_BRACKET:
			{
				u8 bytes;

				if(prev != TT_IDENTIFIER)
				{
					TRACE(ERROR_UNEXPECTED_TOKEN);
				}

				return_if(bytes = _parser_type_annotation(n));

				return_if(_lexer_next(&n->Lexer, &n->Token));
				return_if(_parser_expression(n));
				if(n->Token.Type != TT_R_BRACKET)
				{
					TRACE(ERROR_EXPECTED_R_BRACKET);
				}

				n->Parser.Output[n->Parser.Index++] = INSTR_PUSHI8;
				n->Parser.Output[n->Parser.Index++] = bytes;
				n->Parser.Output[n->Parser.Index++] = TT_MUL;
				n->Parser.Output[n->Parser.Index++] = TT_ADD;
				n->Parser.Output[n->Parser.Index++] = INSTR_PUSHA8 + bytes - 1;
				n->Token.Type = TT_IDENTIFIER;
				break;
			}

			case TT_L_PAREN:
			{
				if(si >= sizeof(op_stack) / sizeof(*op_stack) - 1)
				{
					TRACE(ERROR_STACK_OVERFLOW);
				}

				cnt = 0;
				++paren_cnt;
				op_stack[si++] = n->Token.Type;
				break;
			}

			case TT_R_PAREN:
			{
				if(--paren_cnt == 0)
				{
					goto exit;
				}

				for(; si; --si)
				{
					if(op_stack[si - 1] == TT_L_PAREN)
					{
						--si;
						goto found;
					}

					n->Parser.Output[n->Parser.Index++] = op_stack[si - 1];
				}

				TRACE(ERROR_MISMATCHED_PAREN);
found:
				if(!cnt)
				{
					TRACE(ERROR_UNEXPECTED_TOKEN);
				}
				break;
			}

			case TT_COMMA:
			case TT_SEMICOLON:
			case TT_R_BRACKET:
			{
				goto exit;
			}

			case TT_SUB:
			{
				if(prev == TT_NULL || prev == TT_L_PAREN)
				{
					n->Token.Type = TT_U_MINUS;
				}
			}
			/* fall through */

			default:
			{
				if(IS_OPERATOR(n->Token.Type))
				{
					i32 prec;
					if(IS_OPERATOR(prev) && n->Token.Type != TT_U_MINUS)
					{
						TRACE(ERROR_UNEXPECTED_TOKEN);
					}

					prec = _get_precedence(n->Token.Type);
					for(; si; --si)
					{
						if((_get_precedence(op_stack[si - 1]) > prec) ||
							(op_stack[si - 1] == TT_L_PAREN))
						{
							break;
						}

						n->Parser.Output[n->Parser.Index++] = op_stack[si - 1];
					}

					if(si >= sizeof(op_stack) / sizeof(*op_stack) - 1)
					{
						TRACE(ERROR_STACK_OVERFLOW);
					}

					op_stack[si++] = n->Token.Type;
				}
				else
				{
					TRACE(ERROR_UNEXPECTED_TOKEN);
				}
				break;
			}
		}

		prev = n->Token.Type;
		return_if(_lexer_next(&n->Lexer, &n->Token));
		if(IS_OPERATOR(prev) &&
				n->Token.Type != TT_NUMBER &&
				n->Token.Type != TT_L_PAREN &&
				n->Token.Type != TT_IDENTIFIER)
		{
			if(prev != TT_U_MINUS)
			{
				TRACE(ERROR_UNEXPECTED_TOKEN);
			}
		}
	}

exit:
	for(; si; --si)
	{
		if(op_stack[si - 1] == TT_L_PAREN)
		{
			TRACE(ERROR_MISMATCHED_PAREN);
		}

		n->Parser.Output[n->Parser.Index++] = op_stack[si - 1];
	}

	if(n->Parser.Index == idx_old)
	{
		TRACE(ERROR_INV_EXPR);
	}

	return 0;
}

#endif
