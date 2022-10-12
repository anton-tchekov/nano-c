#include "nanoc.h"
#include <stdio.h>
#include <stdlib.h>

#if defined(NANOC_ENABLE_INTERPRETER) && NANOC_ENABLE_INTERPRETER != 0

void nanoc_disasm(u8 *program, i32 len)
{
	i32 ip;
	ip = 0;
	while(ip < len)
	{
		printf("%04d: ", ip);
		switch(program[ip])
		{
		case INSTR_HALT:
			printf("HALT\n");
			break;

		case INSTR_PUSHI8:
			printf("PUSHI8 %d\n", program[ip + 1]);
			++ip;
			break;

		case INSTR_PUSHI16:
			printf("PUSHI16 %d\n", _read_16(program + ip + 1));
			ip += 2;
			break;

		case INSTR_PUSHI32:
			printf("PUSHI32 %d\n", _read_32(program + ip + 1));
			ip += 4;
			break;

		case INSTR_PUSHL:
			++ip;
			printf("PUSHL %d\n", program[ip]);
			break;

		case INSTR_POPL:
			++ip;
			printf("POPL %d\n", program[ip]);
			break;

		case INSTR_PUSHA8:
			printf("PUSHA8\n");
			break;

		case INSTR_PUSHA16:
			printf("PUSHA16\n");
			break;

		case INSTR_PUSHA32:
			printf("PUSHA32\n");
			break;

		case INSTR_POPA8:
			printf("POPA8\n");
			break;

		case INSTR_POPA16:
			printf("POPA16\n");
			break;

		case INSTR_POPA32:
			printf("POPA32\n");
			break;

		case INSTR_DUP:
			printf("DUP\n");
			break;

		case INSTR_POP:
			printf("POP\n");
			break;

		case INSTR_JZ:
			printf("JZ %d\n", _read_16(program + ip + 1));
			ip += 2;
			break;

		case INSTR_JNZ:
			printf("JNZ %d\n", _read_16(program + ip + 1));
			ip += 2;
			break;

		case INSTR_JMP:
			printf("JMP %d\n", _read_16(program + ip + 1));
			ip += 2;
			break;

		case INSTR_CALL:
			printf("CALL %d %d\n", program[ip + 1], (i16)_read_16(program + ip + 2));
			ip += 3;
			break;

		case INSTR_RET:
			printf("RET\n");
			break;

		case INSTR_ISP:
			++ip;
			printf("ISP %d\n", program[ip]);
			break;

		case TT_U_MINUS:
			printf("U_MINUS\n");
			break;

		case TT_L_NOT:
			printf("L_NOT\n");
			break;

		case TT_B_NOT:
			printf("B_NOT\n");
			break;

		case TT_L_OR:
			printf("L_OR\n");
			break;

		case TT_L_AND:
			printf("L_AND\n");
			break;

		case TT_B_OR:
			printf("B_OR\n");
			break;

		case TT_B_XOR:
			printf("B_XOR\n");
			break;

		case TT_B_AND:
			printf("B_AND\n");
			break;

		case TT_EQ:
			printf("EQ\n");
			break;

		case TT_NE:
			printf("NE\n");
			break;

		case TT_LT:
			printf("LT\n");
			break;

		case TT_GT:
			printf("GT\n");
			break;

		case TT_LE:
			printf("LE\n");
			break;

		case TT_GE:
			printf("GE\n");
			break;

		case TT_B_SHL:
			printf("B_SHL\n");
			break;

		case TT_B_SHR:
			printf("B_SHR\n");
			break;

		case TT_ADD:
			printf("ADD\n");
			break;

		case TT_SUB:
			printf("SUB\n");
			break;

		case TT_MUL:
			printf("MUL\n");
			break;

		case TT_DIV:
			printf("DIV\n");
			break;

		case TT_MOD:
			printf("MOD\n");
			break;

		default:
			printf("INVALID\n");
			break;
		}

		++ip;
	}
}

i32 nanoc_run(NanoC *n, u8 *program, u8 *data)
{
	Interpreter *i = &n->Interpreter;

	i->Heap = data;

	i->OP = 0;
	i->SP = 0;
	i->FP = 0;
	i->IP = 0;

	while(program[i->IP] != INSTR_HALT)
	{
		switch(program[i->IP])
		{
		case INSTR_PUSHI8:
			if(i->OP >= NANOC_OP_STACK_SIZE - 1)
			{
				TRACE(ERROR_STACK_OVERFLOW);
			}

			++i->IP;
			i->OperatorStack[i->OP++] = program[i->IP++];
			break;

		case INSTR_PUSHI16:
			if(i->OP >= NANOC_OP_STACK_SIZE - 1)
			{
				TRACE(ERROR_STACK_OVERFLOW);
			}

			++i->IP;
			i->OperatorStack[i->OP++] = _read_16(program + i->IP);
			i->IP += 2;
			break;

		case INSTR_PUSHI32:
			if(i->OP >= NANOC_OP_STACK_SIZE - 1)
			{
				TRACE(ERROR_STACK_OVERFLOW);
			}

			++i->IP;
			i->OperatorStack[i->OP++] = _read_32(program + i->IP);
			i->IP += 4;
			break;

		case INSTR_PUSHL:
			if(i->OP >= NANOC_OP_STACK_SIZE - 1)
			{
				TRACE(ERROR_STACK_OVERFLOW);
			}

			++i->IP;
			i->OperatorStack[i->OP++] = i->CallStack[i->FP + program[i->IP++]];
			break;

		case INSTR_POPL:
			if(i->OP < 1)
			{
				TRACE(ERROR_STACK_UNDERFLOW);
			}

			++i->IP;
			i->CallStack[i->FP + program[i->IP++]] = i->OperatorStack[--i->OP];
			break;

		case INSTR_PUSHA8:
		{
			i32 addr;
			if(i->OP < 1)
			{
				TRACE(ERROR_STACK_UNDERFLOW);
			}

			++i->IP;
			addr = i->OperatorStack[i->OP - 1];
			if(addr < 0 && addr >= NANOC_HEAP_SIZE)
			{
				TRACE(ERROR_INV_MEM_ACCESS);
			}

			i->OperatorStack[i->OP - 1] = i->Heap[addr];
			break;
		}

		case INSTR_PUSHA16:
		{
			i32 addr;
			if(i->OP < 1)
			{
				TRACE(ERROR_STACK_UNDERFLOW);
			}

			++i->IP;
			addr = i->OperatorStack[i->OP - 1];
			if(addr < 0 && addr >= NANOC_HEAP_SIZE)
			{
				TRACE(ERROR_INV_MEM_ACCESS);
			}

			i->OperatorStack[i->OP - 1] = _read_16(i->Heap + addr);
			break;
		}

		case INSTR_PUSHA32:
		{
			i32 addr;
			if(i->OP < 1)
			{
				TRACE(ERROR_STACK_UNDERFLOW);
			}

			++i->IP;
			addr = i->OperatorStack[i->OP - 1];
			if(addr < 0 && addr >= NANOC_HEAP_SIZE)
			{
				TRACE(ERROR_INV_MEM_ACCESS);
			}

			i->OperatorStack[i->OP - 1] = _read_32(i->Heap + addr);
			break;
		}

		case INSTR_POPA8:
		{
			i32 addr;
			if(i->OP < 2)
			{
				TRACE(ERROR_STACK_UNDERFLOW);
			}

			++i->IP;
			addr = i->OperatorStack[i->OP - 2];
			if(addr < 0 || addr >= NANOC_HEAP_SIZE)
			{
				TRACE(ERROR_INV_MEM_ACCESS);
			}

			i->Heap[addr] = i->OperatorStack[i->OP - 1];
			--i->OP;
			break;
		}

		case INSTR_POPA16:
		{
			i32 addr;
			if(i->OP < 2)
			{
				TRACE(ERROR_STACK_UNDERFLOW);
			}

			++i->IP;
			addr = i->OperatorStack[i->OP - 2];
			if(addr < 0 || addr >= NANOC_HEAP_SIZE)
			{
				TRACE(ERROR_INV_MEM_ACCESS);
			}

			_write_16(i->Heap + addr, i->OperatorStack[i->OP - 1]);
			--i->OP;
			break;
		}

		case INSTR_POPA32:
		{
			i32 addr;
			if(i->OP < 2)
			{
				TRACE(ERROR_STACK_UNDERFLOW);
			}

			++i->IP;
			addr = i->OperatorStack[i->OP - 2];
			if(addr < 0 || addr >= NANOC_HEAP_SIZE)
			{
				TRACE(ERROR_INV_MEM_ACCESS);
			}

			_write_32(i->Heap + addr, i->OperatorStack[i->OP - 1]);
			--i->OP;
			break;
		}

		case INSTR_DUP:
			if(i->OP < 1)
			{
				TRACE(ERROR_STACK_UNDERFLOW);
			}

			++i->IP;
			i->OperatorStack[i->OP] = i->OperatorStack[i->OP - 1];
			++i->OP;
			break;

		case INSTR_JZ:
			if(i->OP < 1)
			{
				TRACE(ERROR_STACK_UNDERFLOW);
			}

			++i->IP;
			if(!i->OperatorStack[--i->OP])
			{
				i->IP = _read_16(program + i->IP);
				continue;
			}

			i->IP += 2;
			break;

		case INSTR_JNZ:
			if(i->OP < 1)
			{
				TRACE(ERROR_STACK_UNDERFLOW);
			}

			++i->IP;
			if(i->OperatorStack[--i->OP])
			{
				i->IP = _read_16(program + i->IP);
				continue;
			}

			i->IP += 2;
			break;

		case INSTR_JMP:
			++i->IP;
			i->IP = _read_16(program + i->IP);
			continue;

		case INSTR_CALL:
		{
			i16 func;
			i32 j, args;
			++i->IP;
			args = program[i->IP];
			++i->IP;
			func = _read_16(program + i->IP);
			i->IP += 2;
			i->OP -= args;
			if(i->OP < 0)
			{
				TRACE(ERROR_STACK_UNDERFLOW);
			}

			for(j = 0; j < args; ++j)
			{
				i->CallStack[i->SP + 2 + j] = i->OperatorStack[i->OP + j];
			}

			if(func < 0)
			{
				/* native function */
				i32 ret;
				func = -func - 1;
				ret = i->Functions[func]((i32 *)(i->CallStack + i->SP + 2), i->Heap);
				if(i->OP >= NANOC_OP_STACK_SIZE - 1)
				{
					TRACE(ERROR_STACK_OVERFLOW);
				}

				i->OperatorStack[i->OP++] = ret;
				break;
			}

			i->CallStack[i->SP++] = i->IP;
			i->CallStack[i->SP++] = i->FP;
			i->FP = i->SP;
			i->IP = func;
			break;
		}

		case INSTR_RET:
			i->SP = i->FP;
			i->FP = i->CallStack[--i->SP];
			i->IP = i->CallStack[--i->SP];
			break;

		case INSTR_ISP:
			++i->IP;
			i->SP += program[i->IP];
			++i->IP;
			break;

		case INSTR_POP:
			if(i->OP < 1)
			{
				TRACE(ERROR_STACK_UNDERFLOW);
			}

			++i->IP;
			--i->OP;
			break;

		case TT_U_MINUS:
			if(i->OP < 1)
			{
				TRACE(ERROR_STACK_UNDERFLOW);
			}

			i->OperatorStack[i->OP - 1] = -i->OperatorStack[i->OP - 1];
			++i->IP;
			break;

		case TT_L_NOT:
			if(i->OP < 1)
			{
				TRACE(ERROR_STACK_UNDERFLOW);
			}

			i->OperatorStack[i->OP - 1] = !i->OperatorStack[i->OP - 1];
			++i->IP;
			break;

		case TT_B_NOT:
			if(i->OP < 1)
			{
				TRACE(ERROR_STACK_UNDERFLOW);
			}

			i->OperatorStack[i->OP - 1] = ~i->OperatorStack[i->OP - 1];
			++i->IP;
			break;

		case TT_L_OR:
			if(i->OP < 2)
			{
				TRACE(ERROR_STACK_UNDERFLOW);
			}

			i->OperatorStack[i->OP - 2] |= i->OperatorStack[i->OP - 1];
			--i->OP;
			++i->IP;
			break;

		case TT_L_AND:
			if(i->OP < 2)
			{
				TRACE(ERROR_STACK_UNDERFLOW);
			}

			i->OperatorStack[i->OP - 2] &= i->OperatorStack[i->OP - 1];
			--i->OP;
			++i->IP;
			break;

		case TT_B_OR:
			if(i->OP < 2)
			{
				TRACE(ERROR_STACK_UNDERFLOW);
			}

			i->OperatorStack[i->OP - 2] |= i->OperatorStack[i->OP - 1];
			--i->OP;
			++i->IP;
			break;

		case TT_B_XOR:
			if(i->OP < 2)
			{
				TRACE(ERROR_STACK_UNDERFLOW);
			}

			i->OperatorStack[i->OP - 2] ^= i->OperatorStack[i->OP - 1];
			--i->OP;
			++i->IP;
			break;

		case TT_B_AND:
			if(i->OP < 2)
			{
				TRACE(ERROR_STACK_UNDERFLOW);
			}

			i->OperatorStack[i->OP - 2] &= i->OperatorStack[i->OP - 1];
			--i->OP;
			++i->IP;
			break;

		case TT_EQ:
			if(i->OP < 2)
			{
				TRACE(ERROR_STACK_UNDERFLOW);
			}

			i->OperatorStack[i->OP - 2] = i->OperatorStack[i->OP - 2] == i->OperatorStack[i->OP - 1];
			--i->OP;
			++i->IP;
			break;

		case TT_NE:
			if(i->OP < 2)
			{
				TRACE(ERROR_STACK_UNDERFLOW);
			}

			i->OperatorStack[i->OP - 2] = i->OperatorStack[i->OP - 2] != i->OperatorStack[i->OP - 1];
			--i->OP;
			++i->IP;
			break;

		case TT_LT:
			if(i->OP < 2)
			{
				TRACE(ERROR_STACK_UNDERFLOW);
			}

			i->OperatorStack[i->OP - 2] = i->OperatorStack[i->OP - 2] < i->OperatorStack[i->OP - 1];
			--i->OP;
			++i->IP;
			break;

		case TT_GT:
			if(i->OP < 2)
			{
				TRACE(ERROR_STACK_UNDERFLOW);
			}

			i->OperatorStack[i->OP - 2] = i->OperatorStack[i->OP - 2] > i->OperatorStack[i->OP - 1];
			--i->OP;
			++i->IP;
			break;

		case TT_LE:
			if(i->OP < 2)
			{
				TRACE(ERROR_STACK_UNDERFLOW);
			}

			i->OperatorStack[i->OP - 2] = i->OperatorStack[i->OP - 2] <= i->OperatorStack[i->OP - 1];
			--i->OP;
			++i->IP;
			break;

		case TT_GE:
			if(i->OP < 2)
			{
				TRACE(ERROR_STACK_UNDERFLOW);
			}

			i->OperatorStack[i->OP - 2] = i->OperatorStack[i->OP - 2] >= i->OperatorStack[i->OP - 1];
			--i->OP;
			++i->IP;
			break;

		case TT_B_SHL:
			if(i->OP < 2)
			{
				TRACE(ERROR_STACK_UNDERFLOW);
			}

			i->OperatorStack[i->OP - 2] <<= i->OperatorStack[i->OP - 1];
			--i->OP;
			++i->IP;
			break;

		case TT_B_SHR:
			if(i->OP < 2)
			{
				TRACE(ERROR_STACK_UNDERFLOW);
			}

			i->OperatorStack[i->OP - 2] >>= i->OperatorStack[i->OP - 1];
			--i->OP;
			++i->IP;
			break;

		case TT_ADD:
			if(i->OP < 2)
			{
				TRACE(ERROR_STACK_UNDERFLOW);
			}

			i->OperatorStack[i->OP - 2] += i->OperatorStack[i->OP - 1];
			--i->OP;
			++i->IP;
			break;

		case TT_SUB:
			if(i->OP < 2)
			{
				TRACE(ERROR_STACK_UNDERFLOW);
			}

			i->OperatorStack[i->OP - 2] -= i->OperatorStack[i->OP - 1];
			--i->OP;
			++i->IP;
			break;

		case TT_MUL:
			if(i->OP < 2)
			{
				TRACE(ERROR_STACK_UNDERFLOW);
			}

			i->OperatorStack[i->OP - 2] *= i->OperatorStack[i->OP - 1];
			--i->OP;
			++i->IP;
			break;

		case TT_DIV:
			if(i->OP < 2)
			{
				TRACE(ERROR_STACK_UNDERFLOW);
			}

			i->OperatorStack[i->OP - 2] /= i->OperatorStack[i->OP - 1];
			--i->OP;
			++i->IP;
			break;

		case TT_MOD:
			if(i->OP < 2)
			{
				TRACE(ERROR_STACK_UNDERFLOW);
			}

			i->OperatorStack[i->OP - 2] %= i->OperatorStack[i->OP - 1];
			--i->OP;
			++i->IP;
			break;

		default:
			TRACE(ERROR_INV_INSTR);
		}
	}

	return 0;
}

#endif

