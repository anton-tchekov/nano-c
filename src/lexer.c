#include "nanoc.h"

#if defined(NANOC_ENABLE_COMPILER) && NANOC_ENABLE_COMPILER != 0

#include <stdio.h>
#include <string.h>

typedef struct KEYWORD
{
	const char *Name;
	TokenType Type;
} Keyword;

static const Keyword _keywords[] =
{
	{ "if",       TT_IF       },
	{ "else",     TT_ELSE     },
	{ "do",       TT_DO       },
	{ "while",    TT_WHILE    },
	{ "for",      TT_FOR      },
	{ "int",      TT_INT      },
	{ "i8",       TT_I8       },
	{ "i16",      TT_I16      },
	{ "i32",      TT_I32      },
	{ "break",    TT_BREAK    },
	{ "continue", TT_CONTINUE },
	{ "return",   TT_RETURN   }
};

void _lexer_init(Lexer *lexer, const char *src, char *strings, i32 offset)
{
	lexer->Ptr = src;
	lexer->StringsOffset = offset;
	lexer->Strings = strings;
	lexer->StringsPtr = strings;
	lexer->LineBegin = src;
	lexer->Pos.Col = 0;
	lexer->Pos.Row = 1;
}

void _lexer_advance(Lexer *lexer)
{
	if(*lexer->Ptr == '\0')
	{
		return;
	}

	++lexer->Ptr;
	++lexer->Pos.Col;
	if(*lexer->Ptr == '\n')
	{
		++lexer->Pos.Row;
		lexer->Pos.Col = -1;
		lexer->LineBegin = lexer->Ptr + 1;
	}
}

static i32 _lexer_char(Lexer *lexer)
{
	char c;
	i32 v;
	if((c = *lexer->Ptr) == '\\')
	{
		_lexer_advance(lexer);
		switch(*lexer->Ptr)
		{
			case '\\':
				v = '\\';
				break;

			case '\'':
				v = '\'';
				break;

			case '\"':
				v = '\"';
				break;

			case 't':
				v = '\t';
				break;

			case 'v':
				v = '\v';
				break;

			case 'n':
				v = '\n';
				break;

			case 'r':
				v = '\r';
				break;

			case 'b':
				v = '\b';
				break;

			case '0':
				v = '\0';
				break;

			default:
				TRACE(ERROR_INV_ESCAPE_SEQUENCE);
		}
	}
	else if(c >= 32 && c <= 126)
	{
		v = c;
	}
	else
	{
		TRACE(ERROR_UNEXPECTED_CHARACTER);
	}

	return v;
}

i32 _lexer_next(Lexer *lexer, Token *token)
{
	char c;
	token->Pos = lexer->Pos;
	for(;;)
	{
		if((c = *lexer->Ptr) == '\0')
		{
			token->Type = TT_NULL;
			return 0;
		}

		switch(c)
		{
			case '/':
			{
				_lexer_advance(lexer);
				if(*lexer->Ptr == '/')
				{
					/* single line comment */
					while(*lexer->Ptr && *lexer->Ptr != '\n')
					{
						_lexer_advance(lexer);
					}

					token->Pos = lexer->Pos;
				}
				else if(*lexer->Ptr == '*')
				{
					/* multi line comment */
					_lexer_advance(lexer);
					while(*lexer->Ptr && !(*lexer->Ptr == '*' && *(lexer->Ptr + 1) == '/'))
					{
						_lexer_advance(lexer);
					}

					_lexer_advance(lexer);
					_lexer_advance(lexer);
					token->Pos = lexer->Pos;
				}
				else
				{
					token->Type = TT_DIV;
					return 0;
				}

				break;
			}

			case '=':
			{
				token->Type = TT_ASSIGN;
				_lexer_advance(lexer);
				if(*lexer->Ptr == '=')
				{
					token->Type = TT_EQ;
					_lexer_advance(lexer);
				}

				return 0;
			}

			case '+':
			{
				token->Type = TT_ADD;
				_lexer_advance(lexer);
				return 0;
			}

			case '-':
			{
				token->Type = TT_SUB;
				_lexer_advance(lexer);
				return 0;
			}

			case '!':
			{
				token->Type = TT_L_NOT;
				_lexer_advance(lexer);
				if(*lexer->Ptr == '=')
				{
					token->Type = TT_NE;
					_lexer_advance(lexer);
				}

				return 0;
			}

			case '<':
			{
				token->Type = TT_LT;
				_lexer_advance(lexer);
				if(*lexer->Ptr == '=')
				{
					token->Type = TT_LE;
					_lexer_advance(lexer);
				}
				else if(*lexer->Ptr == '<')
				{
					token->Type = TT_B_SHL;
					_lexer_advance(lexer);
				}

				return 0;
			}

			case '>':
			{
				token->Type = TT_GT;
				_lexer_advance(lexer);
				if(*lexer->Ptr == '=')
				{
					token->Type = TT_GE;
					_lexer_advance(lexer);
				}
				else if(*lexer->Ptr == '>')
				{
					token->Type = TT_B_SHR;
					_lexer_advance(lexer);
				}

				return 0;
			}

			case '|':
			{
				token->Type = TT_B_OR;
				_lexer_advance(lexer);
				if(*lexer->Ptr == '|')
				{
					token->Type = TT_L_OR;
					_lexer_advance(lexer);
				}

				return 0;
			}

			case '&':
			{
				token->Type = TT_B_AND;
				_lexer_advance(lexer);
				if(*lexer->Ptr == '&')
				{
					token->Type = TT_L_AND;
					_lexer_advance(lexer);
				}

				return 0;
			}

			case '^':
			{
				token->Type = TT_B_XOR;
				_lexer_advance(lexer);
				return 0;
			}

			case '%':
			{
				token->Type = TT_MOD;
				_lexer_advance(lexer);
				return 0;
			}

			case '*':
			{
				token->Type = TT_MUL;
				_lexer_advance(lexer);
				return 0;
			}

			case '(':
			{
				token->Type = TT_L_PAREN;
				_lexer_advance(lexer);
				return 0;
			}

			case ')':
			{
				token->Type = TT_R_PAREN;
				_lexer_advance(lexer);
				return 0;
			}

			case '[':
			{
				token->Type = TT_L_BRACKET;
				_lexer_advance(lexer);
				return 0;
			}

			case ']':
			{
				token->Type = TT_R_BRACKET;
				_lexer_advance(lexer);
				return 0;
			}

			case '{':
			{
				token->Type = TT_L_BRACE;
				_lexer_advance(lexer);
				return 0;
			}

			case '}':
			{
				token->Type = TT_R_BRACE;
				_lexer_advance(lexer);
				return 0;
			}

			case '~':
			{
				token->Type = TT_B_NOT;
				_lexer_advance(lexer);
				return 0;
			}

			case ';':
			{
				token->Type = TT_SEMICOLON;
				_lexer_advance(lexer);
				return 0;
			}

			case ':':
			{
				token->Type = TT_COLON;
				_lexer_advance(lexer);
				return 0;
			}

			case ',':
			{
				token->Type = TT_COMMA;
				_lexer_advance(lexer);
				return 0;
			}

			case '\\':
			{
				token->Type = TT_BACKSLASH;
				_lexer_advance(lexer);
				return 0;
			}

			default:
			{
				if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
						(c == '_'))
				{
					/* parse identifier */
					u32 i;
					const char *name;

					name = lexer->Ptr;
					_lexer_advance(lexer);
					while(((c = *lexer->Ptr) >= 'a' && c <= 'z') ||
							(c >= 'A' && c <= 'Z') ||
							(c >= '0' && c <= '9') ||
							(c == '_'))
					{
						_lexer_advance(lexer);
					}

					for(i = 0; i < sizeof(_keywords) / sizeof(*_keywords); ++i)
					{
						if(strlen(_keywords[i].Name) != (u32)(lexer->Ptr - name))
						{
							continue;
						}

						if(!strncmp(_keywords[i].Name, name, lexer->Ptr - name))
						{
							token->Type = _keywords[i].Type;
							return 0;
						}
					}

					token->Type = TT_IDENTIFIER;
					token->Value.Identifier.Name = name;
					token->Value.Identifier.Length = lexer->Ptr - name;
					return 0;
				}
				else if(c >= '0' && c <= '9')
				{
					/* parse number */
					i32 n;
					n = c - '0';
					_lexer_advance(lexer);
					if(n > 0)
					{
						/* decimal */
						while((c = *(lexer->Ptr)) >= '0' && c <= '9')
						{
							n = n * 10 + (c - '0');
							_lexer_advance(lexer);
						}
					}
					else
					{
						if((c = *(lexer->Ptr)) == 'x' || c == 'X')
						{
							/* hexadecimal */
							_lexer_advance(lexer);
							while(((c = *(lexer->Ptr)) >= '0' && c <= '9') ||
									(c >= 'a' && c <= 'f') ||
									(c >= 'A' && c <= 'F'))
							{
								n = n * 16 + (c & 15) + (c >= 'A' ? 9 : 0);
								_lexer_advance(lexer);
							}
						}
						else if((c = *(lexer->Ptr)) == 'b' || c == 'B')
						{
							/* binary */
							_lexer_advance(lexer);
							while((c = *(lexer->Ptr)) == '0' || c == '1')
							{
								n = n * 2 + (c - '0');
								_lexer_advance(lexer);
							}
						}
						else
						{
							/* octal */
							while((c = *lexer->Ptr) >= '0' && c <= '7')
							{
								n = n * 8 + (c - '0');
								_lexer_advance(lexer);
							}
						}
					}

					token->Type = TT_NUMBER;
					token->Value.Number = n;
					return 0;
				}
				else if(c == '\"')
				{
					i32 v, offset;

					offset = lexer->StringsPtr - lexer->Strings;
					_lexer_advance(lexer);
					for(;;)
					{
						return_if((v = _lexer_char(lexer)));
						if(v == '\"')
						{
							break;
						}

						*lexer->StringsPtr++ = v;
						_lexer_advance(lexer);
					}

					*lexer->StringsPtr++ = '\0';
					token->Type = TT_NUMBER;
					token->Value.Number = lexer->StringsOffset + offset;
					_lexer_advance(lexer);
					return 0;
				}
				else if(c == '\'')
				{
					i32 v;
					_lexer_advance(lexer);
					return_if((v = _lexer_char(lexer)));
					if(v == '\'')
					{
						TRACE(ERROR_UNEXPECTED_CHARACTER);
					}

					_lexer_advance(lexer);

					if(*lexer->Ptr != '\'')
					{
						TRACE(ERROR_UNTERMINATED_CHAR_LITERAL);
					}

					_lexer_advance(lexer);
					token->Type = TT_NUMBER;
					token->Value.Number = v;
					return 0;
				}
				else
				{
					_lexer_advance(lexer);
					token->Pos = lexer->Pos;
				}
			}
		}
	}
}

#endif
