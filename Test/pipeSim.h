#include "mem.cpp"
#include "mem.h"
#include "reg.cpp"
#include "reg.h"
#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <stdexcept>
#include <string>

using namespace std;

extern std::string FILENAME;

void init();
void run();

if_id_latch instr_fetch();
id_ex_latch instr_decode(if_id_latch if_id);
ex_mem_latch instr_execute(id_ex_latch id_ex, bool *user_mode);
mem_wb_latch mem_access(ex_mem_latch ex_mem);
void write_back(mem_wb_latch mem_wb);
void update_PC();