#include "mem.h"

/* Initialize externs */
std::map<std::string, int32> data_symbol_table;
std::map<std::string, int32> text_symbol_table;
std::map<int32, int32> instruction_totals;
std::map<int32, std::string> string_table;
mem_address PC = TEXT_SEG_BASE;
mem_address DATA_TOP = DATA_SEG_BASE;
mem_address TEXT_TOP = TEXT_SEG_BASE;
mem_address DATA_SEG_END = DATA_SEG_BASE + DATA_SEG_LENGTH;
mem_address TEXT_SEG_END = TEXT_SEG_BASE + TEXT_SEG_LENGTH;
mem_word DATA_SEG[DATA_SEG_LENGTH];
mem_word TEXT_SEG[TEXT_SEG_LENGTH];

void init_instr_table()
{
	instruction_totals[ADDI] = 6;
	instruction_totals[B] = 4;
	instruction_totals[BEQZ] = 5;
	instruction_totals[BGE] = 5;
	instruction_totals[BNE] = 5;
	instruction_totals[LA] = 5;
	instruction_totals[LB] = 6;
	instruction_totals[LI] = 3;
	instruction_totals[SUBI] = 6;
	instruction_totals[SYSCALL] = 8;
}

void index_symbols(std::string filename)
{
	printf("\tIndexing symbols...\n");

	std::ifstream file (filename, std::ios::in);
	std::string line;
	bool in_text_seg = false, in_data_seg = false;
	int offset = 0;

	if (file.is_open())
	{
		while (getline(file, line))
		{
			if (line.find_first_of("abcdefghijklmnopqrstuvwxyz0123456789") == std::string::npos)
			{
				// Do nothing
			}

			else if (line.find(".text") != std::string::npos)
			{
				in_text_seg = true;
				in_data_seg = true;
			}
			else if (line.find(".data") != std::string::npos)
			{
				in_data_seg = true;
				in_text_seg = false;
			}

			else
			{
				// If this is a symbol in the text segment
				if (line.find(":") != std::string::npos && in_text_seg)
				{
					std::string symbol_name = line.substr(0, line.find(":"));
					printf("\t\tFound symbol %s at offset %d\n", symbol_name.c_str(), offset);
					text_symbol_table[symbol_name] = offset;
				}

				// If this is a symbol in the data segment
				else if (line.find(":") != std::string::npos && in_data_seg)
				{
					std::string symbol_name = line.substr(0, line.find(":"));
					printf("\t\tFound symbol %s. Assigned to address %x\n", symbol_name.c_str(), DATA_TOP);
					data_symbol_table[symbol_name] = DATA_TOP;
					line = line.substr(line.find("."));
					std::string data_type = line.substr(0, line.find(" "));
					if (data_type == ".asciiz")
					{
						line = line.substr(line.find("\"") + 1);
						std::string msg = line.substr(0, line.find("\""));
						string_table[DATA_TOP] = msg;
						DATA_TOP++;
						printf("\t\t\tAllocating string: %s\n", msg.c_str());
					}
					else if (data_type == ".space")
					{
						line = line.substr(line.find(" ") + 1);
						int size = stoi(line.substr(0, line.find("\r")), 0, 0);
						DATA_TOP += size;
						printf("\t\t\tAllocating space of size %d\n", size);
					}
				}

				// This is just a regular instruction
				else if (in_text_seg)
				{
					offset++;
				}
			}
		}
	}

	printf("\tIndexing COMPLETE... Found %d symbols!\n", (int)text_symbol_table.size());
}

/*
*	This is the "all-powerful initialization routine".
*	Reads in and initializes the data segment and the
*	text segment from the file given as an argument on
*	the command line
*/
void init_segs(std::string filename)
{
	printf("Initializing...\n");

	index_symbols(filename);
	
	std::string line;
	std::ifstream file (filename, std::ios::in);
	bool in_text_seg = false;
	bool in_data_seg = false;
	
	if (file.is_open())
	{
		std::string segment;
		
		while (getline(file, line))
		{
			if (line.find(".text") != std::string::npos)
			{
				printf("\tSetting segment to text\n");
				in_text_seg = true;
				in_data_seg = false;
			}
				
			else if (line.find(".data") != std::string::npos)
			{
				printf("\tSetting segment to data\n");
				in_data_seg = true;
				in_text_seg = false;
			}
				
			else if (line.empty())
			{
				// Do nothing
			}
			
			/*
			*	If this is a line within the data block,
			*	we need to store the data at the given
			*	address in DATA_SEG
			*/
			else if (in_data_seg)
			{
				//printf("\tProcessing data segment line\n");
			}
			
			/*
			*	If this is a line within the text block,
			*	we need to store the instruction at the next
			*	position in TEXT_SEG
			*/
			else if (in_text_seg)
			{
				//printf("\tProcessing text segment line\n");
				

				if(line.find(":") == std::string::npos)
				{
					// We don't want to encode an empty line
					if(line.find("\r") != 0)
					{
						instruction instr = encode_instruction(line);
						write_mem(TEXT_TOP, instr);
					}
				}
				else
				{
					// This is a label
				}
			}		
		
		}
				
		file.close();
	}
	else
	{
		throw std::runtime_error("*** RUNTIME ERROR *** : Failed to open input file");
	}

	init_instr_table();

	printf("Initialization complete!\n");
}

instruction encode_instruction(std::string line)
{
	instruction instr;
	std::string op;
	
	// If there is a space, we can use it to
	// find the op, params follow
	if(line.find(" ") != std::string::npos)
	{
		op = line.substr(0, line.find(" "));
	}

	// Without space, we must use something else
	// to find the op - since there are no params
	// the \r immediate follows the op. If the \r
	// is at index 0, this line contains no op
	else
	{
		op = line.substr(0, line.find("\r"));
	}

	printf("\tEncoding instruction\n");
	
	if(op == "add")
	{
		// TODO: Implement this
		line = line.substr(line.find("$") + 1);

		int dest = stoi(line.substr(0, line.find(",")), 0, 0);

		line = line.substr(line.find("$") + 1);

		int tar = stoi(line.substr(0, line.find(",")), 0, 0);

		line = line.substr(line.find("$") + 1);

		int src = stoi(line.substr(0, line.find("\r")), 0, 0);

		printf("\t\tADD: params - %d %d %d\n", dest, tar, src);

		// Encode the instruction
		instr = ADD;
		instr = instr << 26; // first 6 bits are op code
		instr |= dest << 21; // second 5 bits are dest
		instr |= tar << 16; // third 5 bits are tar
		instr |= src << 11; // fourth 5 bits are src

		printf("\t\t\tOp Code: %x\n", instr >> 26);
		printf("\t\t\tDest: %d\n", (instr >> 21) & 0x1F);
		printf("\t\t\tTar: %d\n", (instr >> 16) & 0x1F);
		printf("\t\t\tSrc: %d\n", (instr >> 11) & 0x1F);
	}

	/*
	 *	ADDI Rdest, Rsrc1, Imm
	 */
	else if(op == "addi")
	{	
		line = line.substr(line.find("$") + 1);

		int dest = stoi(line.substr(0, line.find(",")), 0, 0);

		line = line.substr(line.find("$") + 1);
		
		int src = stoi(line.substr(0, line.find(",")), 0, 0);

		line = line.substr(line.find(" ") + 1);

		int imm = stoi(line.substr(0, line.find("\r")), 0, 0);

		printf("\t\tADDI: params - %d %d %d\n", dest, src, imm);

		// Encode the instruction
		instr = ADDI;
		instr = instr << 26; // first 6 bits are op code
		instr |= dest << 21; // second 5 bits are dest
		instr |= src << 16; // third 5 bits are src
		instr |= (imm & 0xFFFF); // last 16 bits are immediate

		printf("\t\t\tOp Code: %x\n", instr >> 26);
		printf("\t\t\tDest: %d\n", (instr >> 21) & 0x1F);
		printf("\t\t\tSrc: %d\n", (instr >> 16) & 0x1F);
		printf("\t\t\tImm: %d\n", instr & 0xFFFF);
	}
	
	/*
	 *	B label
	 */
	else if(op == "b")
	{
		line = line.substr(line.find(" ") + 1);

		std::string label = line.substr(0, line.find("\r"));

		printf("\t\tB: params - %s\n", label.c_str());

		// Encode the instruction
		instr = B;
		instr = instr << 26; // first 6 bits are op code
		instr |= (text_symbol_table[label] & 0xFFFF); // last 16 bits are offset

		printf("\t\t\tOp Code: %x\n", instr >> 26);
		printf("\t\t\tOffset: %d\n", instr & 0xFFFF);
	}

	/*
	 *	BEQZ Rsrc1, label
	 */
	else if(op == "beqz")
	{
		line = line.substr(line.find("$") + 1);

		int src = stoi(line.substr(0, line.find(" ")), 0, 0);

		line = line.substr(line.find(" ") + 1);

		std::string label = line.substr(0, line.find("\r"));

		printf("\t\tBEQZ: params - %d %s\n", src, label.c_str());

		// Encode the instruction
		instr = BEQZ;
		instr = instr << 26; // first 6 bits are op code
		instr |= src << 21; // second 5 bits are src
		instr |= (text_symbol_table[label] & 0xFFFF); // last 16 bits are offset

		

		printf("\t\t\tOp Code: %x\n", instr >> 26);
		printf("\t\t\tSrc: %d\n", (instr >> 21) & 0x1F);
		printf("\t\t\tOffset: %d\n", instr & 0xFFFF);
	}

	/*
	 *	BGE Rsrc1, Rsrc2, label
	 */
	else if(op == "bge")
	{
		line = line.substr(line.find("$") + 1);

		int src1 = stoi(line.substr(0, line.find(",")), 0, 0);

		line = line.substr(line.find("$") + 1);

		int src2 = stoi(line.substr(0, line.find(",")), 0, 0);

		line = line.substr(line.find(" ") + 1);

		std::string label = line.substr(0, line.find("\r"));

		printf("\t\tBGE: params - %d %d %s\n", src1, src2, label.c_str());

		// Encode the instruction
		instr = BGE;
		instr = instr << 26; // first 6 bits are op code
		instr |= src1 << 21; // second 5 bits are src1
		instr |= src2 << 16; // third 5 bits are src2
		instr |= (text_symbol_table[label] & 0xFFFF); // last 16 bits are offset

		// TODO: Replace the label name with the actual value

		printf("\t\t\tOp Code: %x\n", instr >> 26);
		printf("\t\t\tSrc1: %d\n", (instr >> 21) & 0x1F);
		printf("\t\t\tSrc2: %d\n", (instr >> 16) & 0x1F);
		printf("\t\t\tOffset: %d\n", instr & 0xFFFF);
	}

	/*
	 *	BNE Rsrc1, Rsrc2, label
	 */
	else if(op == "bne")
	{
		line = line.substr(line.find("$") + 1);

		int src1 = stoi(line.substr(0, line.find(",")), 0, 0);

		line = line.substr(line.find("$") + 1);

		int src2 = stoi(line.substr(0, line.find(",")), 0, 0);

		line = line.substr(line.find(" ") + 1);

		std::string label = line.substr(0, line.find("\r"));

		printf("\t\tBNE: params - %d %d %s\n", src1, src2, label.c_str());

		// Encode the instruction
		instr = BNE;
		instr = instr << 26; // first 6 bits are op code
		instr |= src1 << 21; // second 5 bits are src1
		instr |= src2 << 16; // third 5 bits are src2
		instr |= (text_symbol_table[label] & 0xFFFF); // last 16 bits are offset

		// TODO: Replace the label name with the actual value

		printf("\t\t\tOp Code: %x\n", instr >> 26);
		printf("\t\t\tSrc1: %d\n", (instr >> 21) & 0x1F);
		printf("\t\t\tSrc2: %d\n", (instr >> 16) & 0x1F);
		printf("\t\t\tOffset: %d\n", instr & 0xFFFF);
	}

	/*
	 *	LA Rdest, label
	 */
	else if(op == "la")
	{
		line = line.substr(line.find("$") + 1);

		int dest = stoi(line.substr(0, line.find(",")), 0, 0);

		line = line.substr(line.find(" ") + 1);

		std::string label = line.substr(0, line.find("\r"));

		printf("\t\tLA: params - %d %s\n", dest, label.c_str());

		// Encode the instruction
		instr = LA;
		instr = instr << 26; // first 6 bits are op code
		instr |= dest << 21; // second 5 bits are dest
		instr |= (data_symbol_table[label] & 0xFFFF); // last 16 bits are offset

		// TODO: Replace the label name with the actual value

		printf("\t\t\tOp Code: %x\n", instr >> 26);
		printf("\t\t\tDest: %d\n", (instr >> 21) & 0x1F);
		printf("\t\t\tOffset: %x\n", instr & 0xFFFF);
	}

	/*
	 *	LB Rdest, offset(address)
	 */
	else if(op == "lb")
	{
		line = line.substr(line.find("$") + 1);

		int dest = stoi(line.substr(0, line.find(",")), 0, 0);

		line = line.substr(line.find("$") + 1);

		// Here we get the src for the address of the byte we
		// want to load, then retrieve the address from R[src]

		int src = stoi(line.substr(0, line.find(")")), 0, 0);

		mem_address addr = (mem_address) R[src];

		printf("\t\tLB: params - %d %x -> %d\n", dest, src, addr);

		// Encode the instruction
		instr = LB;
		instr = instr << 26; // first 6 bits are op code
		instr |= dest << 21; // second 5 bits are dest
		instr |= src << 16; // third 5 bits are src

		// TODO: Replace the label name with the actual value

		printf("\t\t\tOp Code: %x\n", instr >> 26);
		printf("\t\t\tDest: %d\n", (instr >> 21) & 0x1F);
	}

	/*
	 *	LI Rdest, Imm
	 */
	else if(op == "li")
	{
		line = line.substr(line.find("$") + 1);

		int dest = stoi(line.substr(0, line.find(",")), 0, 0);

		line = line.substr(line.find(" ") + 1);

		int imm = stoi(line.substr(0, line.find("\r")), 0, 0);

		printf("\t\tLI: params - %d %d\n", dest, imm);

		// Encode the instruction
		instr = LI;
		instr = instr << 26; // first 6 bits are op code
		instr |= dest << 21; // second 5 bits are dest
		instr |= (imm & 0xFFFF); // last 16 bits are immediate

		printf("\t\t\tOp Code: %x\n", instr >> 26);
		printf("\t\t\tDest: %d\n", (instr >> 21) & 0x1F);
		printf("\t\t\tImm: %d\n", instr & 0xFFFF);
	}

	/*
	 *	SUBI Rdest, Rsrc1, Imm
	 */
	else if(op == "subi")
	{
		line = line.substr(line.find("$") + 1);

		int dest = stoi(line.substr(0, line.find(",")), 0, 0);

		line = line.substr(line.find("$") + 1);

		int src = stoi(line.substr(0, line.find(",")), 0, 0);

		line = line.substr(line.find(" ") + 1);

		int imm = stoi(line.substr(0, line.find("\r")), 0, 0);

		printf("\t\tSUBI: params - %d %d %d\n", dest, src, imm);

		// Encode the instruction
		instr = SUBI;
		instr = instr << 26; // first 6 bits are op code
		instr |= dest << 21; // seconds 5 bits are the dest
		instr |= src << 16; // third 5 bits are the src
		instr |= (imm & 0xFFFF); // last 16 bits are immediate

		printf("\t\t\tOp Code: %x\n", instr >> 26);
		printf("\t\t\tDest: %d\n", (instr >> 21) & 0x1F);
		printf("\t\t\tSrc: %d\n", (instr >> 16) & 0x1F);
		printf("\t\t\tImm: %d\n", instr & 0xFFFF);
	}

	else if(op == "syscall")
	{
		printf("\t\tSYSCALL\n");

		// Encode the instruction
		instr = SYSCALL;
		instr = instr << 26; // first 6 bits are op code
	}

	else if(op == "nop")
	{
		printf("\t\tNOP\n");

		// Encode the isntruction
		instr = NOP;
		instr = instr << 26; // first 6 bits are op code
	}

	return instr;
}

mem_word read_mem(mem_address address)
{
	if ((DATA_SEG_BASE <= address) && (address < DATA_SEG_END))
	{
		mem_address local_address = address - DATA_SEG_BASE;
		return DATA_SEG[local_address];
	}

	else if ((TEXT_SEG_BASE <= address) && (address < TEXT_SEG_END))
	{
		mem_address local_address = address - TEXT_SEG_BASE;
		return TEXT_SEG[local_address];
	}

	else
	{
		throw std::runtime_error("*** MEMORY ERROR *** : Address outside designated segment space");
	}

	return 0;
}

void write_mem(mem_address address, mem_word data, bool increment_top)
{
	if ((DATA_SEG_BASE <= address) && (address < DATA_SEG_END))
	{
		printf("\t\tWriting to Data segment\n");
		mem_address local_address = address - DATA_SEG_BASE;
		DATA_SEG[local_address] = data;

		if (increment_top)
			DATA_TOP++;
	}

	else if ((TEXT_SEG_BASE <= address) && (address < TEXT_SEG_END))
	{
		printf("\t\tWriting to Text segment\n");
		mem_address local_address = address - TEXT_SEG_BASE;
		TEXT_SEG[local_address] = data;

		if (increment_top)
			TEXT_TOP++;
	}

	else
	{
		throw std::runtime_error("*** MEMORY ERROR *** : Address outside designated segment space");	
	}
}