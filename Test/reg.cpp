#include "reg.h"

/* Initialize externs */
reg_word R[R_LENGTH];

void init_regs()
{
	
}

reg_word read_reg(int index)
{
	if (index >= R_LENGTH)
		throw std::runtime_error("*** REGISTER ERROR *** : Invalid register index");

	return R[index];
}

void write_reg(int index, reg_word val)
{
	if (index >= R_LENGTH)
		throw std::runtime_error("*** REGISTER ERROR *** : Invalid register index");

	R[index] = val;
}