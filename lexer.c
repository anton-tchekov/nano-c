/* Copyright Anton Tchekov */
#include <string.h>
#include "main.h"
#include <stdio.h>

Lexer _lexer;
Token _token;

static struct KEYWORD
{
	char *Name;
	TokenType Type;
}
_keywords[] =
{
	{ "if",       TT_IF       },
	{ "else",     TT_ELSE     },
	{ "case",     TT_CASE     },
	{ "do",       TT_DO       },
	{ "while",    TT_WHILE    },
	{ "for",      TT_FOR      },
	{ "int",      TT_INT      },
	{ "break",    TT_BREAK    },
	{ "continue", TT_CONTINUE },
	{ "return",   TT_RETURN   },
	{ "print",    TT_PRINT    },
	{ "get",      TT_GET      }
};

void lexer_init(char *src)
{
	_lexer.Ptr = src;
	_lexer.LineBegin = src;
	_lexer.Pos.Col = 0;
	_lexer.Pos.Row = 1;
}

void lexer_advance(void)
{
	if(*(_lexer.Ptr) == '\0')
	{
		return;
	}

	++_lexer.Ptr;
	++_lexer.Pos.Col;
	if(*_lexer.Ptr == '\n')
	{
		++_lexer.Pos.Row;
		_lexer.Pos.Col = -1;
		_lexer.LineBegin = _lexer.Ptr + 1;
	}
}

i32 lexer_next(void)
{
	char c;
	_token.Pos = _lexer.Pos;
	for(;;)
	{
		if((c = *_lexer.Ptr) == '\0')
		{
			_token.Type = TT_NULL;
			return -1;
		}

		switch(c)
		{
			case '/':
			{
				lexer_advance();
				if(*_lexer.Ptr == '/')
				{
					/* single line comment */
					while(*_lexer.Ptr && *_lexer.Ptr != '\n')
					{
						lexer_advance();
					}

					_token.Pos = _lexer.Pos;
				}
				else if(*_lexer.Ptr == '*')
				{
					/* multi line comment */
					lexer_advance();
					while(*_lexer.Ptr && !(*_lexer.Ptr == '*' && *(_lexer.Ptr + 1) == '/'))
					{
						lexer_advance();
					}

					lexer_advance();
					lexer_advance();
					_token.Pos = _lexer.Pos;
				}
				else
				{
					_token.Type = TT_DIV;
					return 0;
				}
				break;
			}

			case '=':
			{
				_token.Type = TT_ASSIGN;
				lexer_advance();
				if(*_lexer.Ptr == '=')
				{
					_token.Type = TT_EQ;
					lexer_advance();
				}
				return 0;
			}

			case '+':
			{
				_token.Type = TT_ADD;
				lexer_advance();
				return 0;
			}

			case '-':
			{
				_token.Type = TT_SUB;
				lexer_advance();
				return 0;
			}

			case '!':
			{
				_token.Type = TT_L_NOT;
				lexer_advance();
				if(*_lexer.Ptr == '=')
				{
					_token.Type = TT_NE;
					lexer_advance();
				}
				return 0;
			}

			case '<':
			{
				_token.Type = TT_LT;
				lexer_advance();
				if(*_lexer.Ptr == '=')
				{
					_token.Type = TT_LE;
					lexer_advance();
				}
				else if(*_lexer.Ptr == '<')
				{
					_token.Type = TT_B_SHL;
					lexer_advance();
				}
				return 0;
			}

			case '>':
			{
				_token.Type = TT_GT;
				lexer_advance();
				if(*_lexer.Ptr == '=')
				{
					_token.Type = TT_GE;
					lexer_advance();
				}
				else if(*_lexer.Ptr == '>')
				{
					_token.Type = TT_B_SHR;
					lexer_advance();
				}
				return 0;
			}

			case '|':
			{
				_token.Type = TT_B_OR;
				lexer_advance();
				if(*_lexer.Ptr == '|')
				{
					_token.Type = TT_L_OR;
					lexer_advance();
				}
				return 0;
			}

			case '&':
			{
				_token.Type = TT_B_AND;
				lexer_advance();
				if(*_lexer.Ptr == '&')
				{
					_token.Type = TT_L_AND;
					lexer_advance();
				}
				return 0;
			}

			case '^':
			{
				_token.Type = TT_B_XOR;
				lexer_advance();
				return 0;
			}

			case '%':
			{
				_token.Type = TT_MOD;
				lexer_advance();
				return 0;
			}

			case '*':
			{
				_token.Type = TT_MUL;
				lexer_advance();
				return 0;
			}

			case '(':
			{
				_token.Type = TT_L_PAREN;
				lexer_advance();
				return 0;
			}

			case ')':
			{
				_token.Type = TT_R_PAREN;
				lexer_advance();
				return 0;
			}

			case '[':
			{
				_token.Type = TT_L_BRACKET;
				lexer_advance();
				return 0;
			}

			case ']':
			{
				_token.Type = TT_R_BRACKET;
				lexer_advance();
				return 0;
			}

			case '{':
			{
				_token.Type = TT_L_BRACE;
				lexer_advance();
				return 0;
			}

			case '}':
			{
				_token.Type = TT_R_BRACE;
				lexer_advance();
				return 0;
			}

			case '~':
			{
				_token.Type = TT_B_NOT;
				lexer_advance();
				return 0;
			}

			case ';':
			{
				_token.Type = TT_SEMICOLON;
				lexer_advance();
				return 0;
			}

			case ':':
			{
				_token.Type = TT_COLON;
				lexer_advance();
				return 0;
			}

			case ',':
			{
				_token.Type = TT_COMMA;
				lexer_advance();
				return 0;
			}

			default:
			{
				if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
						(c == '_'))
				{
					/* parse identifier */
					u32 i;
					char *name;
					name = _lexer.Ptr;
					lexer_advance();
					while(((c = *(_lexer.Ptr)) >= 'a' && c <= 'z') ||
							(c >= 'A' && c <= 'Z') ||
							(c >= '0' && c <= '9') ||
							(c == '_'))
					{
						lexer_advance();
					}

					for(i = 0; i < sizeof(_keywords) / sizeof(*_keywords); ++i)
					{
						if(strlen(_keywords[i].Name) != (u32)(_lexer.Ptr - name))
						{
							continue;
						}

						if(!strncmp(_keywords[i].Name, name, _lexer.Ptr - name))
						{
							_token.Type = _keywords[i].Type;
							return 0;
						}
					}

					_token.Type = TT_IDENTIFIER;
					_token.Value.Identifier.Name = name;
					_token.Value.Identifier.Length = _lexer.Ptr - name;
					return 0;
				}
				else if(c >= '0' && c <= '9')
				{
					/* parse number */
					int n;
					n = c - '0';
					lexer_advance();
					if(n > 0)
					{
						/* decimal */
						while((c = *(_lexer.Ptr)) >= '0' && c <= '9')
						{
							n = n * 10 + (c - '0');
							lexer_advance();
						}
					}
					else
					{
						if((c = *(_lexer.Ptr)) == 'x' || c == 'X')
						{
							/* hexadecimal */
							lexer_advance();
							while(((c = *(_lexer.Ptr)) >= '0' && c <= '9') ||
									(c >= 'a' && c <= 'f') ||
									(c >= 'A' && c <= 'F'))
							{
								n = n * 16 + (c & 15) + (c >= 'A' ? 9 : 0);
								lexer_advance();
							}
						}
						else if((c = *(_lexer.Ptr)) == 'b' || c == 'B')
						{
							/* binary */
							lexer_advance();
							while((c = *(_lexer.Ptr)) == '0' || c == '1')
							{
								n = n * 2 + (c - '0');
								lexer_advance();
							}
						}
						else
						{
							/* octal */
							while((c = *(_lexer.Ptr)) >= '0' && c <= '7')
							{
								n = n * 8 + (c - '0');
								lexer_advance();
							}
						}
					}

					_token.Type = TT_NUMBER;
					_token.Value.Number = n;
					return 0;
				}
				else
				{
					lexer_advance();
					_token.Pos = _lexer.Pos;
				}
			}
		}
	}
}

