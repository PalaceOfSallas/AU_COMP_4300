#include "util.cpp"
#include "util.h"
#include "reg.cpp"
#include "reg.h"
#include <cstddef>
#include <fstream>
#include <map>
#include <stdexcept>
#include <string>

#define DATA_SEG_LENGTH 200
#define TEXT_SEG_LENGTH 200
#define DATA_SEG_BASE 0x1000
#define TEXT_SEG_BASE 0x2000

typedef int32 mem_address;
typedef int32 mem_word;

extern std::map<std::string, int32> data_symbol_table;
extern std::map<std::string, int32> text_symbol_table;
extern std::map<int32, int32> instruction_totals;
extern std::map<int32, std::string> string_table;
extern mem_address PC;
extern mem_address DATA_TOP;
extern mem_address TEXT_TOP;
extern mem_address DATA_SEG_END;
extern mem_address TEXT_SEG_END;
extern mem_word DATA_SEG[];
extern mem_word TEXT_SEG[];

void init_segs(std::string filename);
void index_symbols();
void init_instr_table();
instruction encode_instruction(std::string line);
mem_word read_mem(mem_address address);
void write_mem(mem_address address, mem_word data, bool increment_top = true);