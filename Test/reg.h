#include "util.cpp"
#include "util.h"
#include <stdexcept>

#define R_LENGTH 32

typedef int32 reg_word;

extern reg_word R[];

void init_regs();
reg_word read_reg(int index);
void write_reg(int index, reg_word val);