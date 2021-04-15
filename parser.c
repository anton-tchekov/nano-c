/* Copyright Anton Tchekov */
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "map.h"

#define TRACE(e) \
	do \
	{ \
		fprintf(stderr, "[ Trace ] %s - %s:%d (%s)\n", err_msgs[e - 1], __FILE__, __LINE__, __func__); \
		return -e; \
	} while(0)

#define return_if(E) do \
{ \
	i32 ret; \
	if((ret = (E)) < 0) \
	{ \
		fprintf(stderr, "[ Trace ] %s - %s:%d (%s)\n", err_msgs[ABS(ret) - 1], __FILE__, __LINE__, __func__); \
		return ret; \
	} \
} while(0);

#define IS_OPERATOR(T) ((T) > OPERATOR_START && (T) < OPERATOR_END)

u32 read_32(u8 *p)
{
	u32 v;
	v = *p | (*(p + 1) << 8) | (*(p + 2) << 16) | (*(p + 3) << 24);
	return v;
}

void write_32(u8 *b, u32 v)
{
	*b = (u8)(v & 0xFF);
	*(b + 1) = (u8)((v >> 8) & 0xFF);
	*(b + 2) = (u8)((v >> 16) & 0xFF);
	*(b + 3) = (u8)((v >> 24) & 0xFF);
}

static MapElement var_buf[MAX_VARS], fn_buf[MAX_FNS];
static Map vars, fns;

/* bytecode output */
static u8 *_bc;
static u32 _idx;

/* break jump address stack. continue jump address stack */
static u32 bas[MAX_BC], cas[MAX_BC];

/* break address stack top, continue address stack top */
static u32 bt, ct;

/* break nesting, continue nesting */
static u32 bn, cn;

static u32 map_fn_argcnt[MAX_FNS];
static u32 map_fn_addr[MAX_FNS];

static i32 get_precedence(TokenType tt);
static i32 parser_function(char *name, u32 len);
static i32 parser_block(void);
static i32 parser_statement(void);
static i32 parser_substmt(TokenType term);
static i32 parser_fncall(i32 i);
static i32 parser_expression(void);
static void handle_break(u32 prev, u32 addr_break);
static void handle_continue(u32 prev, u32 addr_continue);

static i32 get_precedence(TokenType tt)
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

i32 parser_parse(char *src, u8 *out)
{
	i32 i;
	u32 idx_main, len;
	char *name;
	_bc = out;
	lexer_init(src);
	map_init(&vars, var_buf, sizeof(var_buf) / sizeof(*var_buf));
	map_init(&fns, fn_buf, sizeof(fn_buf) / sizeof(*fn_buf));
	_bc[_idx++] = INSTR_CALL;
	idx_main = _idx;
	_idx += 4;
	_bc[_idx++] = INSTR_HALT;
	for(;;)
	{
		lexer_next();
		if(_token.Type != TT_INT)
		{
			break;
		}

		lexer_next();
		if(_token.Type != TT_IDENTIFIER)
		{
			TRACE(ERROR_EXPECTED_IDENTIFIER);
		}

		name = _token.Value.Identifier.Name;
		len = _token.Value.Identifier.Length;
		lexer_next();
		if(_token.Type != TT_L_PAREN)
		{
			TRACE(ERROR_EXPECTED_L_PAREN);
		}

		return_if(parser_function(name, len));
	}

	if((i = map_find(&fns, "main", 4)) < 0)
	{
		TRACE(ERROR_UNDEFINED_MAIN);
	}

	write_32(_bc + idx_main, map_fn_addr[i]);
	return 0;
}

static i32 parser_function(char *name, u32 len)
{
	i32 i;
	u32 args;
	args = 0;
	return_if(i = map_insert(&fns, name, len));
	map_fn_addr[i] = _idx;
	lexer_next();
	for(;;)
	{
		if(_token.Type == TT_INT)
		{
			lexer_next();
			if(_token.Type != TT_IDENTIFIER)
			{
				TRACE(ERROR_EXPECTED_IDENTIFIER);
			}

			return_if(map_insert(&vars, _token.Value.Identifier.Name,
				_token.Value.Identifier.Length));

			++args;
			lexer_next();
			if(_token.Type == TT_COMMA)
			{
				lexer_next();
			}
		}
		else if(_token.Type == TT_R_PAREN)
		{
			break;
		}
		else
		{
			TRACE(ERROR_UNEXPECTED_TOKEN);
		}
	}

	lexer_next();
	if(_token.Type != TT_L_BRACE)
	{
		TRACE(ERROR_EXPECTED_L_BRACE);
	}

	map_fn_argcnt[i] = args;
	if(args)
	{
		_bc[_idx++] = INSTR_ISP;
		_bc[_idx++] = args;
	}

	return_if(parser_block());
	_bc[_idx++] = INSTR_RET;
	vars.Count -= args;
	return 0;
}

static i32 parser_block(void)
{
	i32 lvc;
	lvc = 0;
	for(;;)
	{
		lexer_next();
		if(_token.Type == TT_INT)
		{
			do
			{
				i32 i;
				lexer_next();
				if(_token.Type != TT_IDENTIFIER)
				{
					TRACE(ERROR_INV_VAR_DECL);
				}

				return_if(i = map_insert(&vars, _token.Value.Identifier.Name,
					_token.Value.Identifier.Length));

				++lvc;
				lexer_next();
				if(_token.Type != TT_ASSIGN)
				{
					continue;
				}

				lexer_next();
				return_if(parser_expression());
				_bc[_idx++] = INSTR_POPL;
				_bc[_idx++] = (u8)(i & 0xFF);
			}
			while(_token.Type == TT_COMMA);
			if(_token.Type != TT_SEMICOLON)
			{
				TRACE(ERROR_EXPECTED_SEMICOLON);
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
		_bc[_idx++] = INSTR_ISP;
		_bc[_idx++] = lvc;
	}

	while(_token.Type != TT_R_BRACE)
	{
		return_if(parser_statement());
		lexer_next();
	}

	vars.Count -= lvc;
	return 0;
}

static void handle_break(u32 prev, u32 addr_break)
{
	while(bt > prev)
	{
		--bt;
		write_32(_bc + bas[bt], addr_break);
	}
}

static void handle_continue(u32 prev, u32 addr_continue)
{
	while(ct > prev)
	{
		--ct;
		write_32(_bc + cas[ct], addr_continue);
	}
}

static i32 parser_statement(void)
{
	switch(_token.Type)
	{
	case TT_NULL:
		TRACE(ERROR_UNEXPECTED_EOF);

	case TT_SEMICOLON:
		break;

	case TT_PRINT:
		lexer_next();
		if(_token.Type != TT_L_PAREN)
		{
			TRACE(ERROR_EXPECTED_L_PAREN);
		}

		lexer_next();
		return_if(parser_expression());
		lexer_next();
		if(_token.Type != TT_SEMICOLON)
		{
			TRACE(ERROR_EXPECTED_SEMICOLON);
		}

		_bc[_idx++] = INSTR_PRINTI;
		break;

	case TT_IF:
	{
		Lexer state;
		u32 idx_after, idx_end;
		lexer_next();
		if(_token.Type != TT_L_PAREN)
		{
			TRACE(ERROR_EXPECTED_L_PAREN);
		}

		lexer_next();
		return_if(parser_expression());
		_bc[_idx++] = INSTR_JZ;
		idx_after = _idx;
		_idx += 4;
		lexer_next();
		return_if(parser_statement());
		state = _lexer;
		lexer_next();
		if(_token.Type == TT_ELSE)
		{
			_bc[_idx++] = INSTR_JMP;
			idx_end = _idx;
			_idx += 4;
			write_32(_bc + idx_after, _idx);
			lexer_next();
			return_if(parser_statement());
			write_32(_bc + idx_end, _idx);
		}
		else
		{
			_lexer = state;
			write_32(_bc + idx_after, _idx);
		}
		break;
	}

	case TT_WHILE:
	{
		u32 idx_cp, idx_after, prev_break, prev_continue;
		idx_cp = _idx;
		lexer_next();
		if(_token.Type != TT_L_PAREN)
		{
			TRACE(ERROR_EXPECTED_L_PAREN);
		}

		lexer_next();
		return_if(parser_expression());
		_bc[_idx++] = INSTR_JZ;
		idx_after = _idx;
		_idx += 4;
		lexer_next();
		prev_break = bt;
		prev_continue = ct;
		++bn;
		++cn;
		return_if(parser_statement());
		--bn;
		--cn;
		_bc[_idx++] = INSTR_JMP;
		write_32(_bc + _idx, idx_cp);
		_idx += 4;
		write_32(_bc + idx_after, _idx);
		handle_break(prev_break, _idx);
		handle_continue(prev_continue, idx_cp);
		break;
	}

	case TT_DO:
	{
		u32 idx_begin, idx_cp, prev_break, prev_continue;
		idx_begin = _idx;
		lexer_next();
		prev_break = bt;
		prev_continue = ct;
		++bn;
		++cn;
		return_if(parser_statement());
		--bn;
		--cn;
		lexer_next();
		if(_token.Type != TT_WHILE)
		{
			TRACE(ERROR_EXPECTED_WHILE);
		}

		lexer_next();
		if(_token.Type != TT_L_PAREN)
		{
			TRACE(ERROR_EXPECTED_L_PAREN);
		}

		lexer_next();
		return_if(parser_expression());
		if(_token.Type != TT_R_PAREN)
		{
			TRACE(ERROR_EXPECTED_R_PAREN);
		}

		lexer_next();
		if(_token.Type != TT_SEMICOLON)
		{
			TRACE(ERROR_EXPECTED_SEMICOLON);
		}

		idx_cp = _idx;
		_bc[_idx++] = INSTR_JNZ;
		write_32(_bc + _idx, idx_begin);
		_idx += 4;
		handle_break(prev_break, _idx);
		handle_continue(prev_continue, idx_cp);
		break;
	}

	case TT_FOR:
	{
		Lexer save, save2;
		u32 idx_inc, idx_cp, idx_end, prev_break, prev_continue;
		lexer_next();
		if(_token.Type != TT_L_PAREN)
		{
			TRACE(ERROR_EXPECTED_L_PAREN);
		}

		lexer_next();
		return_if(parser_substmt(TT_SEMICOLON));
		lexer_next();
		idx_cp = _idx;
		if(_token.Type == TT_SEMICOLON)
		{
			_bc[_idx++] = INSTR_PUSHI;
			write_32(_bc + _idx, 1);
			_idx += 4;
		}
		else
		{
			return_if(parser_expression());
		}

		_bc[_idx++] = INSTR_JZ;
		idx_end = _idx;
		_idx += 4;
		save = _lexer;
		lexer_next();
		while(_token.Type != TT_R_PAREN)
		{
			lexer_next();
			if(_token.Type == TT_NULL)
			{
				TRACE(ERROR_UNEXPECTED_EOF);
			}
		}

		lexer_next();
		prev_break = bt;
		prev_continue = ct;
		++bn;
		++cn;
		return_if(parser_statement());
		--bn;
		--cn;
		idx_inc = _idx;
		save2 = _lexer;
		_lexer = save;
		lexer_next();
		return_if(parser_substmt(TT_R_PAREN));
		_lexer = save2;
		_bc[_idx++] = INSTR_JMP;
		write_32(_bc + _idx, idx_cp);
		_idx += 4;
		write_32(_bc + idx_end, _idx);
		handle_break(prev_break, _idx);
		handle_continue(prev_continue, idx_inc);
		break;
	}

	case TT_BREAK:
		lexer_next();
		if(_token.Type != TT_SEMICOLON)
		{
			TRACE(ERROR_EXPECTED_SEMICOLON);
		}

		if(!bn)
		{
			TRACE(ERROR_INV_BREAK);
		}

		_bc[_idx++] = INSTR_JMP;
		bas[bt++] = _idx;
		_idx += 4;
		break;

	case TT_CONTINUE:
		lexer_next();
		if(_token.Type != TT_SEMICOLON)
		{
			TRACE(ERROR_EXPECTED_SEMICOLON);
		}

		if(!cn)
		{
			TRACE(ERROR_INV_CONTINUE);
		}

		_bc[_idx++] = INSTR_JMP;
		cas[ct++] = _idx;
		_idx += 4;
		break;

	case TT_RETURN:
		lexer_next();
		return_if(parser_expression());
		_bc[_idx++] = INSTR_RET;
		break;

	case TT_L_BRACE:
		return_if(parser_block());
		break;

	default:
		return_if(parser_substmt(TT_SEMICOLON));
	}

	return 0;
}

static i32 parser_substmt(TokenType term)
{
	i32 i;
	if(_token.Type == term)
	{
		return 0;
	}

	for(;;)
	{
		Token prev;
		if(_token.Type != TT_IDENTIFIER)
		{
			TRACE(ERROR_UNEXPECTED_TOKEN);
		}

		prev = _token;
		lexer_next();
		if(_token.Type == TT_L_PAREN)
		{
			if((i = map_find(&fns, prev.Value.Identifier.Name,
				prev.Value.Identifier.Length)) < 0)
			{
				TRACE(ERROR_UNDEFINED_FN);
			}

			return_if(parser_fncall(i));
			lexer_next();
		}
		else
		{
			u32 arr;
			TokenType at;
			if((i = map_find(&vars, prev.Value.Identifier.Name,
				prev.Value.Identifier.Length)) < 0)
			{
				TRACE(ERROR_UNDEFINED_VAR);
			}

			arr = 0;
			at = TT_NULL;
			if(_token.Type == TT_L_BRACKET)
			{
				lexer_next();
				return_if(parser_expression());
				if(_token.Type != TT_R_BRACKET)
				{
					TRACE(ERROR_EXPECTED_R_BRACKET);
				}

				lexer_next();
				_bc[_idx++] = INSTR_PUSHL;
				_bc[_idx++] = (u8)(i & 0xFF);
				_bc[_idx++] = TT_ADD;
				arr = 1;
			}

			if(_token.Type == TT_ADD || _token.Type == TT_SUB ||
				_token.Type == TT_MUL || _token.Type == TT_DIV ||
				_token.Type == TT_MOD || _token.Type == TT_B_SHL ||
				_token.Type == TT_B_SHR || _token.Type == TT_B_AND ||
				_token.Type == TT_B_OR || _token.Type == TT_B_XOR)
			{
				at = _token.Type;
				lexer_next();
				if(arr)
				{
					_bc[_idx++] = INSTR_DUP;
					_bc[_idx++] = INSTR_PUSHA;
				}
				else
				{
					_bc[_idx++] = INSTR_PUSHL;
					_bc[_idx++] = (u8)(i & 0xFF);
				}
			}
			else if(_token.Type != TT_ASSIGN)
			{
				TRACE(ERROR_UNEXPECTED_TOKEN);
			}

			lexer_next();
			return_if(parser_expression());
			if(at)
			{
				_bc[_idx++] = at;
			}

			if(arr)
			{
				_bc[_idx++] = INSTR_POPA;
			}
			else
			{
				_bc[_idx++] = INSTR_POPL;
				_bc[_idx++] = (u8)(i & 0xFF);
			}
		}

		if(_token.Type == TT_COMMA)
		{
			lexer_next();
			continue;
		}
		else if(_token.Type == term)
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

static i32 parser_fncall(i32 i)
{
	u32 arg_cnt;
	arg_cnt = 0;
	lexer_next();
	if(_token.Type != TT_R_PAREN)
	{
		for(;;)
		{
			return_if(parser_expression());
			_bc[_idx++] = INSTR_POPP;
			_bc[_idx++] = (u8)(arg_cnt & 0xFF);
			++arg_cnt;
			if(_token.Type == TT_R_PAREN)
			{
				break;
			}
			else if(_token.Type == TT_COMMA)
			{
				lexer_next();
			}
			else
			{
				TRACE(ERROR_UNEXPECTED_TOKEN);
			}
		}
	}

	if(arg_cnt != map_fn_argcnt[i])
	{
		TRACE(ERROR_FN_NUM_ARGS);
	}

	_bc[_idx++] = INSTR_CALL;
	write_32(_bc + _idx, map_fn_addr[i]);
	_idx += 4;
	return 0;
}

static i32 parser_expression(void)
{
	TokenType prev, op_stack[32];
	u32 idx_old, paren_cnt, cnt;
	size_t si;

	prev = TT_NULL;
	paren_cnt = 1;
	si = 0;
	idx_old = _idx;
	cnt = 0;

	for(;;)
	{
		switch(_token.Type)
		{
			case TT_NUMBER:
			{
				if(prev == TT_NUMBER || prev == TT_IDENTIFIER ||
						prev == TT_GET || prev == TT_R_PAREN)
				{
					TRACE(ERROR_UNEXPECTED_TOKEN);
				}

				++cnt;
				_bc[_idx++] = INSTR_PUSHI;
				write_32(_bc + _idx, _token.Value.Number);
				_idx += 4;
				break;
			}

			case TT_GET:
			{
				if(prev == TT_NUMBER || prev == TT_IDENTIFIER ||
						prev == TT_GET || prev == TT_R_PAREN)
				{
					TRACE(ERROR_UNEXPECTED_TOKEN);
				}

				lexer_next();
				if(_token.Type != TT_L_PAREN)
				{
					TRACE(ERROR_EXPECTED_L_PAREN);
				}

				lexer_next();
				if(_token.Type != TT_R_PAREN)
				{
					TRACE(ERROR_EXPECTED_R_PAREN);
				}

				lexer_next();
				_bc[_idx++] = INSTR_GETI;
				prev = TT_GET;
				continue;
			}

			case TT_IDENTIFIER:
			{
				Token tok;
				Lexer lex;
				i32 i;
				if(prev == TT_NUMBER || prev == TT_IDENTIFIER ||
						prev == TT_GET || prev == TT_R_PAREN)
				{
					TRACE(ERROR_UNEXPECTED_TOKEN);
				}

				tok = _token;
				prev = _token.Type;
				lex = _lexer;
				lexer_next();
				if(_token.Type == TT_L_PAREN)
				{
					if((i = map_find(&fns, tok.Value.Identifier.Name,
						tok.Value.Identifier.Length)) >= 0)
					{
						return_if(parser_fncall(i));
					}
				}
				else
				{
					_lexer = lex;
					_token = tok;
					if((i = map_find(&vars, tok.Value.Identifier.Name,
						tok.Value.Identifier.Length)) < 0)
					{
						TRACE(ERROR_UNDEFINED_VAR);
					}

					++cnt;
					_bc[_idx++] = INSTR_PUSHL;
					_bc[_idx++] = (u8)(i & 0xFF);
				}

				break;
			}

			case TT_L_BRACKET:
			{
				if(prev != TT_IDENTIFIER)
				{
					TRACE(ERROR_UNEXPECTED_TOKEN);
				}

				lexer_next();
				return_if(parser_expression());
				if(_token.Type != TT_R_BRACKET)
				{
					TRACE(ERROR_EXPECTED_R_BRACKET);
				}

				_bc[_idx++] = TT_ADD;
				_bc[_idx++] = INSTR_PUSHA;
				_token.Type = TT_IDENTIFIER;
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
				op_stack[si++] = _token.Type;
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

					_bc[_idx++] = op_stack[si - 1];
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
					_token.Type = TT_U_MINUS;
				}
			}
			/* fall through */

			default:
			{
				if(IS_OPERATOR(_token.Type))
				{
					int prec;
					if(IS_OPERATOR(prev) && _token.Type != TT_U_MINUS)
					{
						TRACE(ERROR_UNEXPECTED_TOKEN);
					}

					prec = get_precedence(_token.Type);
					for(; si; --si)
					{
						if((get_precedence(op_stack[si - 1]) > prec) ||
							(op_stack[si - 1] == TT_L_PAREN))
						{
							break;
						}

						_bc[_idx++] = op_stack[si - 1];
					}

					if(si >= sizeof(op_stack) / sizeof(*op_stack) - 1)
					{
						TRACE(ERROR_STACK_OVERFLOW);
					}

					op_stack[si++] = _token.Type;
				}
				else
				{
					TRACE(ERROR_UNEXPECTED_TOKEN);
				}
				break;
			}
		}

		prev = _token.Type;
		lexer_next();
		if(IS_OPERATOR(prev) &&
				_token.Type != TT_NUMBER &&
				_token.Type != TT_L_PAREN &&
				_token.Type != TT_IDENTIFIER &&
				_token.Type != TT_GET)
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

		_bc[_idx++] = op_stack[si - 1];
	}

	if(_idx == idx_old)
	{
		TRACE(ERROR_INV_EXPR);
	}

	return 0;
}

