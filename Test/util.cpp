#include "util.h"

int32 get_op_code(instruction instr)
{
	return instr >> 26;
}

int32 get_rs(instruction instr)
{
	return (instr >> 11) & 0x1F;
}

int32 get_rt(instruction instr)
{
	return (instr >> 16) & 0x1F;
}

int32 get_rd(instruction instr)
{
	return (instr >> 21) & 0x1F;
}

int32 get_imm(instruction instr)
{
	return instr & 0xFFFF;
}

bool is_branch_instr(int32 op_code)
{
	switch (op_code)
	{
		case B:
			return true;
			break;

		case BEQZ:
			return true;
			break;

		case BGE:
			return true;
			break;

		case BNE:
			return true;
			break;
	}

	return false;
}