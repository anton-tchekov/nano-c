/* Copyright Anton Tchekov */
#include "main.h"
#include <stdio.h>
#include <stdlib.h>

#define CALL_STACK_SIZE 4048
#define OP_STACK_SIZE   1024
#define HEAP_SIZE       4096

i32 interpreter_run(u8 *program)
{
	i32 op_stack[OP_STACK_SIZE];
	u32 heap[HEAP_SIZE],
		call_stack[CALL_STACK_SIZE],
		op = 0, /* op stack top */
		sp = 0, /* call stack pointer */
		fp = 0, /* frame pointer */
		ip = 0; /* instruction pointer */

	while(program[ip] != INSTR_HALT)
	{
		switch(program[ip])
		{
		case INSTR_PUSHI:
			if(op >= OP_STACK_SIZE - 1)
			{
				return -ERROR_STACK_OVERFLOW;
			}

			++ip;
			op_stack[op++] = read_32(program + ip);
			ip += 4;
			break;

		case INSTR_PUSHL:
			if(op >= OP_STACK_SIZE - 1)
			{
				return -ERROR_STACK_OVERFLOW;
			}

			++ip;
			op_stack[op++] = call_stack[fp + program[ip++]];
			break;

		case INSTR_POPL:
			if(op < 1)
			{
				return -ERROR_STACK_UNDERFLOW;
			}

			++ip;
			call_stack[fp + program[ip++]] = op_stack[--op];
			break;

		case INSTR_PUSHA:
		{
			i32 addr;
			if(op < 1)
			{
				return -ERROR_STACK_UNDERFLOW;
			}

			++ip;
			if((addr = op_stack[op - 1]) < 0 && addr >= HEAP_SIZE)
			{
				printf("pusha addr = %d\n", addr);
				return -ERROR_INV_MEM_ACCESS;
			}

			op_stack[op - 1] = heap[addr];
			break;
		}

		case INSTR_POPA:
		{
			i32 addr;
			if(op < 2)
			{
				return -ERROR_STACK_UNDERFLOW;
			}

			++ip;
			if((addr = op_stack[op - 2]) < 0 || addr >= HEAP_SIZE)
			{
				printf("popa addr = %d\n", addr);
				return -ERROR_INV_MEM_ACCESS;
			}

			heap[addr] = op_stack[op - 1];
			--op;
			break;
		}

		case INSTR_DUP:
			if(op < 1)
			{
				return -ERROR_STACK_UNDERFLOW;
			}

			++ip;
			op_stack[op] = op_stack[op - 1];
			++op;
			break;

		case INSTR_JZ:
			if(op < 1)
			{
				return -ERROR_STACK_UNDERFLOW;
			}

			++ip;
			if(!op_stack[--op])
			{
				ip = read_32(program + ip);
				continue;
			}

			ip += 4;
			break;

		case INSTR_JNZ:
			if(op < 1)
			{
				return -ERROR_STACK_UNDERFLOW;
			}

			++ip;
			if(op_stack[--op])
			{
				ip = read_32(program + ip);
				continue;
			}

			ip += 4;
			break;

		case INSTR_JMP:
			++ip;
			ip = read_32(program + ip);
			continue;

		case INSTR_POPP:
			if(op < 1)
			{
				return -ERROR_STACK_UNDERFLOW;
			}

			++ip;
			call_stack[sp + program[ip] + 2] = op_stack[--op];
			++ip;
			break;

		case INSTR_POP:
			if(op < 1)
			{
				return -ERROR_STACK_UNDERFLOW;
			}

			++ip;
			--op;
			break;

		case INSTR_CALL:
		{
			u32 func;
			++ip;
			func = read_32(program + ip);
			ip += 4;
			call_stack[sp++] = ip;
			call_stack[sp++] = fp;
			fp = sp;
			ip = func;
			break;
		}

		case INSTR_RET:
			sp = fp;
			fp = call_stack[--sp];
			ip = call_stack[--sp];
			break;

		case INSTR_PRINTI:
			if(op < 1)
			{
				return -ERROR_STACK_UNDERFLOW;
			}

			printf("> %d\n", op_stack[--op]);
			++ip;
			break;

		case INSTR_GETI:
		{
			u32 v, c;
			char buf[32], *ep;
			if(op >= OP_STACK_SIZE - 1)
			{
				return -ERROR_STACK_OVERFLOW;
			}

			c = 0;
			do
			{
				printf(c ? "?? " : "? ");
				ep = NULL;
				fgets(buf, sizeof(buf), stdin);
				v = strtol(buf, &ep, 10);
				++c;
			}
			while(*ep != '\n');
			op_stack[op++] = v;
			++ip;
			break;
		}

		case INSTR_ISP:
			++ip;
			sp += program[ip];
			++ip;
			break;

		case TT_U_MINUS:
			if(op < 1)
			{
				return -ERROR_STACK_UNDERFLOW;
			}

			op_stack[op - 1] = -op_stack[op - 1];
			++ip;
			break;

		case TT_L_NOT:
			if(op < 1)
			{
				return -ERROR_STACK_UNDERFLOW;
			}

			op_stack[op - 1] = !op_stack[op - 1];
			++ip;
			break;

		case TT_B_NOT:
			if(op < 1)
			{
				return -ERROR_STACK_UNDERFLOW;
			}

			op_stack[op - 1] = ~op_stack[op - 1];
			++ip;
			break;

		case TT_L_OR:
			if(op < 2)
			{
				return -ERROR_STACK_UNDERFLOW;
			}

			op_stack[op - 2] |= op_stack[op - 1];
			--op;
			++ip;
			break;

		case TT_L_AND:
			if(op < 2)
			{
				return -ERROR_STACK_UNDERFLOW;
			}

			op_stack[op - 2] &= op_stack[op - 1];
			--op;
			++ip;
			break;

		case TT_B_OR:
			if(op < 2)
			{
				return -ERROR_STACK_UNDERFLOW;
			}

			op_stack[op - 2] |= op_stack[op - 1];
			--op;
			++ip;
			break;

		case TT_B_XOR:
			if(op < 2)
			{
				return -ERROR_STACK_UNDERFLOW;
			}

			op_stack[op - 2] ^= op_stack[op - 1];
			--op;
			++ip;
			break;

		case TT_B_AND:
			if(op < 2)
			{
				return -ERROR_STACK_UNDERFLOW;
			}

			op_stack[op - 2] &= op_stack[op - 1];
			--op;
			++ip;
			break;

		case TT_EQ:
			if(op < 2)
			{
				return -ERROR_STACK_UNDERFLOW;
			}

			op_stack[op - 2] = op_stack[op - 2] == op_stack[op - 1];
			--op;
			++ip;
			break;

		case TT_NE:
			if(op < 2)
			{
				return -ERROR_STACK_UNDERFLOW;
			}

			op_stack[op - 2] = op_stack[op - 2] != op_stack[op - 1];
			--op;
			++ip;
			break;

		case TT_LT:
			if(op < 2)
			{
				return -ERROR_STACK_UNDERFLOW;
			}

			op_stack[op - 2] = op_stack[op - 2] < op_stack[op - 1];
			--op;
			++ip;
			break;

		case TT_GT:
			if(op < 2)
			{
				return -ERROR_STACK_UNDERFLOW;
			}

			op_stack[op - 2] = op_stack[op - 2] > op_stack[op - 1];
			--op;
			++ip;
			break;

		case TT_LE:
			if(op < 2)
			{
				return -ERROR_STACK_UNDERFLOW;
			}

			op_stack[op - 2] = op_stack[op - 2] <= op_stack[op - 1];
			--op;
			++ip;
			break;

		case TT_GE:
			if(op < 2)
			{
				return -ERROR_STACK_UNDERFLOW;
			}

			op_stack[op - 2] = op_stack[op - 2] >= op_stack[op - 1];
			--op;
			++ip;
			break;

		case TT_B_SHL:
			if(op < 2)
			{
				return -ERROR_STACK_UNDERFLOW;
			}

			op_stack[op - 2] <<= op_stack[op - 1];
			--op;
			++ip;
			break;

		case TT_B_SHR:
			if(op < 2)
			{
				return -ERROR_STACK_UNDERFLOW;
			}

			op_stack[op - 2] >>= op_stack[op - 1];
			--op;
			++ip;
			break;

		case TT_ADD:
			if(op < 2)
			{
				return -ERROR_STACK_UNDERFLOW;
			}

			op_stack[op - 2] += op_stack[op - 1];
			--op;
			++ip;
			break;

		case TT_SUB:
			if(op < 2)
			{
				return -ERROR_STACK_UNDERFLOW;
			}

			op_stack[op - 2] -= op_stack[op - 1];
			--op;
			++ip;
			break;

		case TT_MUL:
			if(op < 2)
			{
				return -ERROR_STACK_UNDERFLOW;
			}

			op_stack[op - 2] *= op_stack[op - 1];
			--op;
			++ip;
			break;

		case TT_DIV:
			if(op < 2)
			{
				return -ERROR_STACK_UNDERFLOW;
			}

			op_stack[op - 2] /= op_stack[op - 1];
			--op;
			++ip;
			break;

		case TT_MOD:
			if(op < 2)
			{
				return -ERROR_STACK_UNDERFLOW;
			}

			op_stack[op - 2] %= op_stack[op - 1];
			--op;
			++ip;
			break;

		default:
			return -ERROR_INV_INSTR;
		}
	}

	return 0;
}

