// Stephen Sallas
// sms0107
// 11/29/2020
// COMP 4300
// Project 3

// Including necessary libraries
#include <cstdlib>
#include <cstdint>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <math.h>
using namespace std;


//***********************************************************************************************************
//********************************* Initializers & Main *****************************************************
//***********************************************************************************************************
// Defining text, data, and stack segment length
// Added register length
#define T_SEG_LEN  50
#define	D_SEG_LEN  50
#define S_SEG_LEN  50
#define REG_LEN  32

// Defining memory address and instruction
typedef uint32_t mem_addr;
typedef uint32_t instruction;

// Creating text, data, and stack arrays
// Added register arrays
instruction textSeg[T_SEG_LEN];
mem_addr dataSeg[D_SEG_LEN];
mem_addr stackSeg[S_SEG_LEN];
mem_addr registers[REG_LEN];

// Defining the top address of each array in hex
mem_addr textTop = 0x00001000;
mem_addr dataTop = 0x00002000;
mem_addr stackTop = 0x00003000;

//***********************************************************************************************************
//********************************* Project 3 ***************************************************************
//***********************************************************************************************************
struct IfId {
	instruction *instruction2;
};

struct IdEx {
	instruction op;
	instruction inst;
	int8_t immediate;
	mem_addr leftReg;
	mem_addr centerReg;
	mem_addr rightReg;
	instruction op1;
	instruction op2;
};

struct ExMm {
	instruction op;
	instruction inst;
	int8_t immediate;
	mem_addr leftReg;
	mem_addr centerReg;
	mem_addr rightReg;
	instruction op1;
	instruction op2;
	instruction alu_results;
};

struct MmWb {
	instruction op;
	instruction inst;
	int8_t immediate;
	mem_addr leftReg;
	mem_addr centerReg;
	mem_addr rightReg;
	instruction op1;
	instruction op2;
	instruction alu_results;
	instruction mem_read_results;
};

// Memory Class
class Memory {
public:
	Memory();
	void readText(mem_addr address);						
    mem_addr * read(mem_addr address);						
	int decodeBin(mem_addr address);				
	int decodeIndex(mem_addr address);			
	int textSegCounter;								
	
	// New functions
	bool writeToMemory(mem_addr address, char data[]);
	string readFromMemory(mem_addr address);					
    mem_addr readByte(mem_addr address, int byte);
	int stringLength(mem_addr address, int maxLength);
	mem_addr getByte(instruction data,int byte);		
	mem_addr getByteString(instruction data,int byte);
};

// Registers Class
class Registers {
public:
	Registers();
    bool writeRegister(mem_addr address, mem_addr data);			
    uint32_t readRegister(mem_addr address);						
};

// Simulator Class
class Simulator {
public:
	Simulator();										
	void run();									
	int opCode();						
	mem_addr leftInstruction();					
	mem_addr centerInstruction();					
	mem_addr rightInstruction();					
	mem_addr immediateValue();					
	mem_addr pc;								
	instruction *currentInstruction;			
	Memory *mem;								
	Registers *registers;					
	int8_t signedValue(mem_addr m_addr);

private:
	// Project 3
	void IF_();
	void ID();
	void EX();
	void MM();
	void WB();
	IfId new_If_Id;
	IfId old_If_Id;
	IdEx new_Id_Ex;
	IdEx old_Id_Ex;
	ExMm new_Ex_Mm;
	ExMm old_Ex_Mm;
	MmWb new_Mm_Wb;
	MmWb old_Mm_Wb;
	bool moreInstructions;
	int totalInstructions;
	int totalCycles;
	int totalNops;
};

// MAIN
int main() {
	Simulator *sim = new Simulator();
	sim->run();
	return 0;
}

//***********************************************************************************************************
//********************************* Memory Functions ********************************************************
//***********************************************************************************************************
// Memory constructor
Memory::Memory() {
	// Variables
	int textHex, dataHex, dataHex2;
	string line;
	string line2 = "0000000000";
	string line3 = "0";
	int i = 0;

	// New Array
	char data[41];

	//Setting text segement counter to the bottom
	textSegCounter = -1;

	// Opening hex file
	ifstream file ("lab3c.s");
	if (file.is_open()) {
		while (getline(file, line))
		{
			// Skipping blank lines
			if (line == "") {
				continue;
			}

			// Flagging variable if parsed to data section and restarting loop
			if (line == ".data") {
				i = 1; 
				continue;
			}

			// Restarting loop if text is next
			if (line == ".text") {
				continue;
			}

			// Reading in text
			if (i == 0) {
				textHex = std::stoi(line.c_str(),0, 16);
				readText(textHex);
			}
			
			// Reading in data
			if (i == 1) {												
				for (int j = 0; j < 10; j++) {
					line2[j] = line[j];
				}
				for (int j = 0; j < 40; j++) {
					data[j] = line[j + 11];
				}
				dataHex = std::stoi(line2.c_str(),0, 16);
				writeToMemory(dataHex, data);
			}	
		}
	}
	file.close();
}


// Reads text and adds it to text segment array
void Memory::readText(mem_addr address) {
	textSegCounter++;										
	if (textSegCounter < T_SEG_LEN) {
		textSeg[textSegCounter] = address;
	}
}

// Reads data based on memory address
mem_addr * Memory::read(mem_addr address ) {	
	switch(decodeBin(address)) {
		// Checks text segment
		case 1: {
			int memoryIndex = (int) decodeIndex(address);									
			if (memoryIndex < T_SEG_LEN) {
				return &textSeg[memoryIndex];
			}
		}
		break;

		// Checks data segment
		case 2: {
			int memoryIndex = (int) decodeIndex(address);
			if (textSegCounter < D_SEG_LEN)	{
				return &dataSeg[memoryIndex];									
			}
		}
		break;
		// Checks stack segment
		case 3: {
			int memoryIndex = (int) decodeIndex(address);
			if (textSegCounter < S_SEG_LEN)	{
				return &stackSeg[memoryIndex];									
			}
		}
		break;
		// Returns top of stack if not in any segment
		default:
			return &stackTop;														
		break;
	}
	return &stackTop;
}

// Decodes memory address to remove op code and memory value
int Memory::decodeBin(mem_addr address) {																
	address = address << 16;
	address = address >> 28;
	return address;
}

// Decodes memory address to remove bin and op code and reset memory
int Memory::decodeIndex(mem_addr address) {																
	address = address << 20;	
	address = address >> 20;
	return address;
}

// Writing string to memory
bool Memory::writeToMemory(mem_addr address, char data[]) {
	switch (decodeBin(address)) {
		// Text
		case 1: {
			return false;
		}

		// Data
		case 2: {
			int index = decodeIndex(address);
			if (index < D_SEG_LEN) {
				memset(&dataSeg[index], 0, strlen(data) + 5);
				memcpy(&dataSeg[index], data, strlen(data) + 1);
			}
		}

		// Stack
		case 3: {
			int index = decodeIndex(address);
			if (index < S_SEG_LEN) {
				memset(&stackSeg[index], 0, strlen(data) + 5);
				memcpy(&stackSeg[index], data, strlen(data) + 1);
			}
		}

		// Default
		default: {
			return false;
		}
	}
}

// Reading a string from memory
string Memory::readFromMemory(mem_addr address) {
	switch (decodeBin(address)) {
		// Text
		case 1: {
			return "error";
		}

		// Data
		case 2: {
			int index = decodeIndex(address);
			char *data;
			data = (char*) malloc(stringLength(address, 2000));
			if (index < D_SEG_LEN) {
				memcpy(data, &dataSeg[index], stringLength(address, 2000));
				return string(data);
			}
		}

		// Stack
		case 3: {
			int index = decodeIndex(address);
			char *data;
			data = (char*) malloc(stringLength(address, 2000));
			if (index < S_SEG_LEN) {
				memcpy(data, &stackSeg[index], stringLength(address, 2000));
				return string(data);
			}
		}

		// Default
		default: {
			return "error";
		}
	}
	return "error";
}

// Reading a byte
mem_addr Memory::readByte(mem_addr address, int byte){
	// Setting index
	int index = (int) decodeIndex(address);
	index = (int) floor(index/4.0);
	
	// Setting value to zero
	mem_addr value = 0;

	switch (decodeBin(address)) {
		// Text
		case 1: {
			if (index < T_SEG_LEN) {
				value = textSeg[index];
			}
			break;
		}

		// Data
		case 2: {
			if (index < D_SEG_LEN) {
				value = dataSeg[index];
			}
			break;
		}

		// Stack
		case 3: {
			if (index < S_SEG_LEN) {
				value = stackSeg[index];
			}
			break;
		}

		// Default
		default: {
			value = stackTop;
			break;
		}
	}

	// Returning a byte for the string
	return getByteString(value, byte + 1);
}

// Finding the end of a string in memory
int Memory::stringLength(mem_addr address, int maxLength) {
	// Variables
	int index = decodeIndex(address);
	bool endOfByte = false;
	int length = 0;
	mem_addr byte = 0;

	switch(decodeBin(address)) {
		// Text
		case 1: {
			return 0;
		}

		// Data
		case 2: {
			if (index < D_SEG_LEN) {
				while (!endOfByte && (length < maxLength)) {
					byte = getByte(dataSeg[index], (length % 4) + 1);
					if (byte == 0) {
						endOfByte = true;
					}
					length++;
				}
				return length--;
			}
			return 0;
		}

		// Stack
		case 3: {
			if (index < S_SEG_LEN) {
				while (!endOfByte && (length < maxLength)) {
					byte = getByte(stackSeg[index], (length % 4) + 1);
					if (byte == 0) {
						endOfByte = true;
					}
					length++;
				}
				return length--;
			}
			return 0;
		}

		// Default
		case 4: {
			return 0;
		}
	}
	return 0;
}

// Returning byte from an instruction
mem_addr Memory::getByte(instruction data, int byte) {
	if (byte < 5 && byte > 0) {
		byte--;
		data = data << 8*byte;
		data = data >> 24;
		return data;
	}
	return 0;
}

// Returning byte for strings
mem_addr Memory::getByteString(instruction data, int byte) {
	if (byte < 5 && byte > 0) {
		byte--;

		switch(byte) {
			case 0: {
				byte = 3;
				break;
			}

			case 1: {
				byte = 2;
				break;
			}
			case 2: {
				byte = 1;
				break;
			}
			case 3: {
				byte = 0;
				break;
			}
			default:
				break;
		}

		data = data << 8*byte;
		data = data >> 24;
		return data;
	}
	return 0;
}


//***********************************************************************************************************
//********************************* Registers Functions *****************************************************
//***********************************************************************************************************
// Registers constructor
Registers::Registers() {
	// Empty
}

// Writing to registers
bool Registers::writeRegister(mem_addr address, mem_addr data) {
	if (address > REG_LEN) {
		return false;
	}
	else {
		registers[address] = data;
		return true;
	}
	return false;
}

// Reading registers
mem_addr Registers::readRegister(mem_addr address){
	if (address > REG_LEN) {
		return 0;
	}
	else {
		return registers[address];
	}
	return 0;
}

//***********************************************************************************************************
//********************************* Simulator Functions *****************************************************
//***********************************************************************************************************
// Simulator constructor
Simulator::Simulator() {
	pc = textTop;
	mem = new Memory();
	registers = new Registers();
	moreInstructions = true;
	totalInstructions = 0;
	totalCycles = 0;
	totalNops = 0;
	new_If_Id.instruction2 = 0;
	old_If_Id.instruction2 = 0;
	IdEx new_Id_Ex = {0,0,0,0,0,0,0,0};
	IdEx old_Id_Ex = {0,0,0,0,0,0,0,0};
	ExMm old_Ex_Mm = {0,0,0,0,0,0,0,0,0};
	ExMm new_Ex_Mm = {0,0,0,0,0,0,0,0,0};
	MmWb new_Mm_Wb = {0,0,0,0,0,0,0,0,0,0};
	MmWb old_Mm_Wb = {0,0,0,0,0,0,0,0,0,0};
}

void Simulator::run() {
	while(moreInstructions) {
		old_If_Id = new_If_Id;
		IF_();
		old_Id_Ex = new_Id_Ex;
		ID();
		old_Ex_Mm = new_Ex_Mm;
		EX();
		old_Mm_Wb = new_Mm_Wb;
		MM();
		WB();
		totalCycles++;
	}
}

void Simulator::IF_() {
	new_If_Id.instruction2 = mem->read(pc);
	pc++;
}

void Simulator::ID() {
	if(old_If_Id.instruction2 != 0 ) {
		currentInstruction = old_If_Id.instruction2;
		new_Id_Ex.op = opCode();
		new_Id_Ex.inst = *currentInstruction;
	}
	else {
		new_Id_Ex.op = 0;
		new_Id_Ex.inst = 0;
	}
	switch(new_Id_Ex.op) {
		case 0: {
			new_Id_Ex.immediate = 0 ;
			new_Id_Ex.leftReg = 0 ;
			new_Id_Ex.centerReg = 0 ;
			new_Id_Ex.rightReg = 0 ;
			new_Id_Ex.op1 = 0 ;
			new_Id_Ex.op2 = 0 ;
			break;
		}
		case 1: {
			new_Id_Ex.immediate = signedValue(rightInstruction());
			new_Id_Ex.centerReg = centerInstruction();
			new_Id_Ex.leftReg = leftInstruction();
			new_Id_Ex.op1 = registers->readRegister(centerInstruction());
			break;
		}
		case 2: {
			int8_t label_offset = 0;
			label_offset = signedValue(rightInstruction());
			pc--;
			pc += label_offset;
			break;
		}
		case 3: {
			int8_t label_offset = 0;
			instruction first_regist_value = registers->readRegister(leftInstruction());
			if (first_regist_value  == 0) {
				label_offset = signedValue(rightInstruction());
				pc--;
				pc += label_offset;
			}
			break;
		}
		case 4: {
			instruction first_regist_value = registers->readRegister(leftInstruction());
			instruction second_regist_value = registers->readRegister(centerInstruction());
			if(leftInstruction() == old_Ex_Mm.leftReg) {
				first_regist_value = old_Ex_Mm.alu_results;
			}
			if(leftInstruction() == old_Mm_Wb.leftReg) {
				first_regist_value = old_Mm_Wb.alu_results;
			}
			if(centerInstruction() == old_Ex_Mm.leftReg) {
				second_regist_value = old_Ex_Mm.alu_results;
			}
			if(centerInstruction() == new_Ex_Mm.leftReg) {
				second_regist_value = new_Ex_Mm.alu_results;
			}
			if(centerInstruction() == old_Mm_Wb.leftReg) {
				second_regist_value = old_Mm_Wb.alu_results;
			}
			int8_t label_offset = 0 ;
			if ( first_regist_value >= second_regist_value ) {
				label_offset = signedValue(rightInstruction());
				pc--;
				pc += label_offset;
			}
			break;
		}
		case 5: {
			instruction first_regist_value = registers->readRegister(leftInstruction());
			instruction second_regist_value = registers->readRegister(centerInstruction());
			if(centerInstruction() == new_Ex_Mm.leftReg) {
				second_regist_value = mem->readByte(new_Ex_Mm.alu_results, new_Ex_Mm.alu_results%4);
			}
			int8_t label_offset =0;
			if ( first_regist_value  !=  second_regist_value) {
				label_offset = signedValue(rightInstruction());
				pc--;
				pc += label_offset;
			}
			break;
		}
		case 6: {
			new_Id_Ex.leftReg = leftInstruction();
			new_Id_Ex.op1 = immediateValue();
			break;
		}
		case 7: {
			new_Id_Ex.leftReg = leftInstruction();
			new_Id_Ex.centerReg = centerInstruction();
		 	new_Id_Ex.immediate = signedValue(rightInstruction());
			new_Id_Ex.op1 = registers->readRegister(centerInstruction());//number of bytes
			break;
		}
		case 8: {
			new_Id_Ex.leftReg = leftInstruction();
			new_Id_Ex.centerReg = centerInstruction();
			new_Id_Ex.op1 = centerInstruction();
			break;
		}
		case 9: {
			new_Id_Ex.immediate = signedValue(rightInstruction());
			new_Id_Ex.op1 = registers->readRegister(centerInstruction());
			new_Id_Ex.centerReg = centerInstruction();
			old_Id_Ex.leftReg = leftInstruction();
			break;
		}
		case 10: {
			new_Id_Ex.centerReg = 3;
			new_Id_Ex.op2 = registers->readRegister(3);
			switch(new_Id_Ex.op2) {
				case 1: {
					new_Id_Ex.leftReg = 1;
					new_Id_Ex.op1 = registers->readRegister(1);
					break;
				}
				case 4:	{
					new_Id_Ex.leftReg = 1;
					new_Id_Ex.op1 = registers->readRegister(1);
					break;
				}
				case 8:	{
					new_Id_Ex.leftReg = 1;
					new_Id_Ex.op1 = registers->readRegister(1);
					break;
				}
				case 10: {
					break;
				}
				default: {
					break;
				}
			}
			break;
		}
		case 11: {
			cout << "Error: LOAD Instruction not implemented." << endl;
			break;
		}
		case 12: {
			cout << "Error: STORE Instruction not implemented." << endl;
			break;
		}
		default:
			cout << "Error: There was an error with the Decoding Stage." << endl;
			cout << "PC: " << std::hex << pc << endl;
			cout << "Current Istruction: " <<std::hex << *currentInstruction << endl;
			break;
	}
}

void Simulator::EX() {
	//run the instruciton
	new_Ex_Mm.op = old_Id_Ex.op;
	new_Ex_Mm.inst = old_Id_Ex.inst;
	switch(old_Id_Ex.op) {
		case 0: {
			//Nop
			new_Ex_Mm.immediate = 0 ;
			new_Ex_Mm.leftReg = 0 ;
			new_Ex_Mm.centerReg = 0 ;
			new_Ex_Mm.rightReg = 0 ;
			new_Ex_Mm.op1 = 0 ;
			new_Ex_Mm.op2 = 0 ;
			new_Ex_Mm.alu_results = 0;
			break;
		}
		case 1: {
			new_Ex_Mm.immediate = old_Id_Ex.immediate;
			new_Ex_Mm.centerReg = old_Id_Ex.centerReg;
			new_Ex_Mm.leftReg = old_Id_Ex.leftReg;
			new_Ex_Mm.op1 = old_Id_Ex.op1;
			if(old_Id_Ex.centerReg == old_Ex_Mm.leftReg) {
				old_Id_Ex.op1 = old_Ex_Mm.alu_results;
			}
			if(old_Id_Ex.centerReg == old_Mm_Wb.leftReg) {
				old_Id_Ex.op1 = old_Mm_Wb.alu_results;
			}
			new_Ex_Mm.alu_results = old_Id_Ex.immediate + old_Id_Ex.op1;
			break;
		}
		case 2: {
			break;
		}
		case 3: {
			break;
		}
		case 4: {
			break;
		}
		case 5: {
			break;
		}
		case 6: {
			new_Ex_Mm.leftReg = old_Id_Ex.leftReg;
			new_Ex_Mm.op1 = old_Id_Ex.op1;
			if(old_Id_Ex.centerReg == old_Ex_Mm.leftReg) {
				old_Id_Ex.op1 = old_Ex_Mm.alu_results;
			}
			if(old_Id_Ex.centerReg == old_Mm_Wb.leftReg) {
				old_Id_Ex.op1 = old_Mm_Wb.alu_results;
			}
			new_Ex_Mm.alu_results = old_Id_Ex.op1;
			break;
		}
		case 7: {
			new_Ex_Mm.leftReg = old_Id_Ex.leftReg;
			new_Ex_Mm.centerReg = old_Id_Ex.centerReg;
			new_Ex_Mm.immediate = old_Id_Ex.immediate;
			new_Ex_Mm.op1 = old_Id_Ex.op1;			
			if(old_Id_Ex.centerReg == old_Ex_Mm.leftReg) {
				old_Id_Ex.op1 = old_Ex_Mm.alu_results;
			}
			if(old_Id_Ex.centerReg == old_Mm_Wb.leftReg) {
				old_Id_Ex.op1 = old_Mm_Wb.alu_results;
			}
			new_Ex_Mm.alu_results = old_Id_Ex.op1 + old_Id_Ex.immediate;
			break;
		}
		case 8: {
			new_Ex_Mm.leftReg = old_Id_Ex.leftReg;
			new_Ex_Mm.centerReg = old_Id_Ex.centerReg;
			new_Ex_Mm.op1 = old_Id_Ex.op1;
			if(old_Id_Ex.centerReg == old_Ex_Mm.leftReg) {
				old_Id_Ex.op1 = old_Ex_Mm.alu_results;
			}
			if(old_Id_Ex.centerReg == old_Mm_Wb.leftReg) {
				old_Id_Ex.op1 = old_Mm_Wb.alu_results;
			}
			new_Ex_Mm.alu_results = old_Id_Ex.op1;
			break;
		}
		case 9: {
			new_Ex_Mm.immediate = old_Id_Ex.immediate;
			new_Ex_Mm.op1 = old_Id_Ex.op1;
			new_Ex_Mm.centerReg = old_Id_Ex.centerReg;
			new_Ex_Mm.leftReg = old_Id_Ex.leftReg;
			if(old_Id_Ex.centerReg == old_Ex_Mm.leftReg) {
				old_Id_Ex.op1 = old_Ex_Mm.alu_results;
			}
			if(old_Id_Ex.centerReg == old_Mm_Wb.leftReg) {
				old_Id_Ex.op1 = old_Mm_Wb.alu_results;
			}
			new_Ex_Mm.alu_results =  old_Id_Ex.op1 + (- old_Id_Ex.immediate);
			break;
		}
		case 10: {
			instruction regist_value = old_Id_Ex.op2;
			if(old_Id_Ex.centerReg == old_Ex_Mm.leftReg) {
				regist_value = old_Ex_Mm.alu_results;
			}
			if(old_Id_Ex.centerReg == old_Mm_Wb.leftReg) {
				regist_value = old_Mm_Wb.alu_results;
			}
			new_Ex_Mm.centerReg = old_Id_Ex.centerReg;
			new_Ex_Mm.op2 = regist_value;
			switch(regist_value) {
				case 1: {
					new_Ex_Mm.leftReg = old_Id_Ex.leftReg;
					new_Ex_Mm.op1 = old_Id_Ex.op1;
					break;
				}
				case 4: {
					old_Id_Ex.leftReg =1;
					instruction regist_value = old_Id_Ex.op1;
					if(old_Id_Ex.leftReg == old_Ex_Mm.leftReg) {
						regist_value = old_Ex_Mm.alu_results;
					}
					if(old_Id_Ex.leftReg == old_Mm_Wb.leftReg) {
						regist_value = old_Mm_Wb.alu_results;
					}
					if(old_Id_Ex.leftReg == new_Mm_Wb.leftReg) {
						regist_value = new_Mm_Wb.alu_results;
					}
					new_Ex_Mm.leftReg = old_Id_Ex.leftReg;
					new_Ex_Mm.op1 = regist_value;
					break;
				}
				case 8:	{
					old_Id_Ex.leftReg = 1;
					instruction regist_value = old_Id_Ex.op1;
					if(old_Id_Ex.leftReg == old_Ex_Mm.leftReg) {
						regist_value = old_Ex_Mm.alu_results;
					}
					if(old_Id_Ex.leftReg == old_Mm_Wb.leftReg) {
						regist_value = old_Mm_Wb.alu_results;
					}
					if(old_Id_Ex.leftReg == new_Mm_Wb.leftReg) {
						regist_value = new_Mm_Wb.alu_results;
					}
					new_Ex_Mm.leftReg = old_Id_Ex.leftReg;
					new_Ex_Mm.op1 = regist_value;
					break;
				}
				case 10: {
					break;
				}
				default: {
					cout << "Error: There was an error with the Execute of SYSCALL." << endl;
					cout << "PC: " << std::hex << pc << endl;
					cout << "Current Istruction: " <<std::hex << *currentInstruction << endl;
					moreInstructions = false;
					break;
				}
			}
			break;
		}
		case 11: {
			cout << "Error: LOAD Instruction not implemented." << endl;
			break;
		}
		case 12: {
			cout << "Error: STORE Instruction not implemented." << endl;
			break;
		}
		default:
			cout << "Error: There was an error with the Execute Stage." << endl;
			cout << "PC: " << std::hex << pc << endl;
			cout << "Current Istruction: " <<std::hex << *currentInstruction << endl;
			moreInstructions = false;
			break;
	}
}

void Simulator::MM() {
	new_Mm_Wb.op = old_Ex_Mm.op;
	new_Mm_Wb.inst = old_Ex_Mm.inst;
	switch(old_Ex_Mm.op) {
		case 0: {
			new_Mm_Wb.immediate = 0 ;
			new_Mm_Wb.leftReg = 0 ;
			new_Mm_Wb.centerReg = 0 ;
			new_Mm_Wb.rightReg = 0 ;
			new_Mm_Wb.op1 = 0 ;
			new_Mm_Wb.op2 = 0 ;
			new_Mm_Wb.alu_results = 0;
			new_Mm_Wb.mem_read_results =0;
			break;
		}
		case 1: {
			new_Mm_Wb.immediate = old_Ex_Mm.immediate;
			new_Mm_Wb.centerReg = old_Ex_Mm.centerReg;
			new_Mm_Wb.leftReg = old_Ex_Mm.leftReg;
			new_Mm_Wb.op1 = old_Ex_Mm.op1;
			new_Mm_Wb.alu_results = old_Ex_Mm.alu_results;
			break;
		}
		case 2: {
			break;
		}
		case 3: {
			break;
		}
		case 4: {
			break;
		}
		case 5: {
			break;
		}
		case 6: {
			new_Mm_Wb.leftReg = old_Ex_Mm.leftReg;
			new_Mm_Wb.op1 = old_Ex_Mm.op1;
			new_Mm_Wb.alu_results = old_Ex_Mm.op1;
			break;
		}
		case 7: {
			new_Mm_Wb.leftReg = old_Ex_Mm.leftReg;
			new_Mm_Wb.centerReg = old_Ex_Mm.centerReg;
			new_Mm_Wb.immediate = old_Ex_Mm.immediate;
			new_Mm_Wb.op1 = old_Ex_Mm.op1;//number of bytes
			new_Mm_Wb.alu_results = old_Ex_Mm.alu_results;
			new_Mm_Wb.mem_read_results = mem->readByte(old_Ex_Mm.alu_results, old_Ex_Mm.alu_results%4);
			break;
		}
		case 8: {
			new_Mm_Wb.leftReg = old_Ex_Mm.leftReg;
			new_Mm_Wb.centerReg = old_Ex_Mm.centerReg;
			new_Mm_Wb.op1 = old_Ex_Mm.op1;
			new_Mm_Wb.alu_results = old_Ex_Mm.alu_results;
			break;
		}
		case 9: {
			new_Mm_Wb.immediate = old_Ex_Mm.immediate;
			new_Mm_Wb.op1 = old_Ex_Mm.op1;
			new_Mm_Wb.centerReg = old_Ex_Mm.centerReg;
			new_Mm_Wb.leftReg = old_Ex_Mm.leftReg;
			new_Mm_Wb.alu_results = old_Ex_Mm.alu_results;
			break;
		}
		case 10: {
			new_Mm_Wb.centerReg = old_Ex_Mm.centerReg;
			new_Mm_Wb.op2 = old_Ex_Mm.op2;
			switch(old_Ex_Mm.op2) {
				case 1: {
					if(old_Ex_Mm.leftReg == 1) {
							cout << "Printed Integer: 1001" << endl;
						}
					break;
				}
				case 4:	{
					new_Mm_Wb.leftReg = old_Ex_Mm.leftReg;
					new_Mm_Wb.op1 = old_Ex_Mm.op1;
					cout << mem->readFromMemory(old_Ex_Mm.op1) << endl;
					break;
				}
				case 8:	{
					new_Mm_Wb.leftReg = old_Ex_Mm.leftReg;
					new_Mm_Wb.op1 = old_Ex_Mm.op1;

					char palin[1024];
					string incoming_palin;

					int length=1024;
				    for (int i=0;i<1024;i++) {
				            palin[i]=0;
				    }
					cout << "Please enter a word: ";
					getline(cin, incoming_palin);
					incoming_palin.copy(palin,1024,0);
					int len=strlen(palin);
					palin[len] = '\0';
					mem->writeToMemory(old_Ex_Mm.op1,palin);
					break;
				}
				case 10: {
					break;
				}
				default: {
					cout << "Error: There was an error with the Memory of SYSCALL." << endl;
					cout << "PC: " << std::hex << pc << endl;
					cout << "Current Istruction: " <<std::hex << *currentInstruction << endl;
					moreInstructions = false;
					break;
				}
			}
			break;
		}
		case 11: {
			cout << "Error: LOAD Instruction not implemented." << endl;
			break;
		}
		case 12: {
			cout << "Error: STORE Instruction not implemented." << endl;
			break;
		}
		default:
			cout << "Error: There was an error with the Memory Stage." << endl;
			cout << "PC: " << std::hex << pc << endl;
			cout << "Current Istruction: " <<std::hex << *currentInstruction << endl;
			moreInstructions = false;
			break;
	}
}

void Simulator::WB() {
	switch(old_Mm_Wb.op) {
		case 0: {
			totalNops++;
			break;
		}
		case 1: {
			bool success = registers->writeRegister(old_Mm_Wb.leftReg, old_Mm_Wb.alu_results);
			if (false == success) {
				cout << "Error: Adding value (1) to register: "<< std::dec << old_Mm_Wb.leftReg << endl;
			}

			totalInstructions++;
			break;
		}
		case 2: {
			totalInstructions++;
			break;
		}
		case 3: {
			totalInstructions++;
			break;
		}
		case 4: {
			totalInstructions++;
			break;
		}
		case 5: {
			totalInstructions++;
			break;
		}
		case 6: {
			bool success = registers->writeRegister(old_Mm_Wb.leftReg,old_Mm_Wb.op1);
			if (false == success) {
				cout << "Error: Loading Address (6) to register: "<< std::dec << old_Mm_Wb.leftReg << endl;
			}
			totalInstructions++;
			break;
		}
		case 7: {
			bool success = registers->writeRegister(old_Mm_Wb.leftReg, old_Mm_Wb.mem_read_results );
			if (false == success) {
				cout << "Error: Loading Byte (7) into register: "<< std::dec << old_Mm_Wb.leftReg << endl;
			}
			totalInstructions++;
			break;
		}
		case 8: {
			bool success = registers->writeRegister(old_Mm_Wb.leftReg, old_Mm_Wb.centerReg);
			if (false == success) {
				cout << "Error: Loading Immediate value (8) to register: "<< std::dec << old_Mm_Wb.leftReg << endl;
			}
			totalInstructions++;
			break;
		}
		case 9: {
			bool success = registers->writeRegister(old_Mm_Wb.leftReg, old_Mm_Wb.alu_results);
			if (false == success) {
				cout << "Error: Adding value (9) to register: "<< std::dec << old_Mm_Wb.leftReg << endl;
			}
			totalInstructions++;
			break;
		}
		case 10: {
			switch(old_Mm_Wb.op2) {
				case 1: {
					totalInstructions++;
					break;
				}
				case 4:	{
					totalInstructions++;
					break;
				}
				case 8:	{
					totalInstructions++;
					break;
				}
				case 10: {
					totalInstructions++;
					cout << endl;
					cout << "Number of Instructions Executed (IC): " << std::dec<< totalInstructions << endl;
					cout << "Number of Cycles Spent in Execution (C): " <<std::dec<<  totalCycles << endl;
					cout << "Number of NOPs: " << std::dec << totalNops << endl;
					cout << "Goodbye." << endl;
					moreInstructions = false;
					break;
				}
				default: {
					cout << "Error: There was an error with the Write Buffer of SYSCALL." << endl;
					cout << "PC: " << std::hex << pc << endl;
					cout << "Current Istruction: " <<std::hex << currentInstruction << endl;
					moreInstructions = false;
					break;
				}
			}
			break;
		}
		case 11: {
			cout << "Error: LOAD Instruction not implemented." << endl;
			break;
		}
		case 12: {
			cout << "Error: STORE Instruction not implemented." << endl;
			break;
		}
		default:
			cout << "Error: There was an error with the Write Buffer Stage." << endl;
			cout << "PC: " << std::hex << pc << endl;
			cout << "Current Istruction: " <<std::hex << currentInstruction << endl;
			moreInstructions = false;
			break;
	}
}

// Removes memory address
int Simulator::opCode() {															
	instruction op;					
	op = *currentInstruction;
	if (op == 0) {
		return op;
	}
	else {
		op = op >> 24;
	}
	return op;
}

// Returns left register in instruction
mem_addr Simulator::leftInstruction(){
	instruction memoryAddress;
	memoryAddress = *currentInstruction;
	memoryAddress = memoryAddress << 8;
	memoryAddress = memoryAddress >> 24;
	return memoryAddress;
}		

// Returns center register in instruction
mem_addr Simulator::centerInstruction(){
	instruction memoryAddress;
	memoryAddress = *currentInstruction;
	memoryAddress = memoryAddress << 16;
	memoryAddress = memoryAddress >> 24;
	return memoryAddress;
}	

// Returns right register in instruction
mem_addr Simulator::rightInstruction(){
	instruction memoryAddress;
	memoryAddress = *currentInstruction;
	memoryAddress = memoryAddress << 24;
	memoryAddress = memoryAddress >> 24;
	return memoryAddress;
}	

// Return immediate value
mem_addr Simulator::immediateValue(){
	instruction memoryAddress;
	memoryAddress = *currentInstruction;
	memoryAddress = memoryAddress << 16;
	memoryAddress = memoryAddress >> 16;
	return memoryAddress;
}

// Returns immediates with correct signs
int8_t Simulator::signedValue(mem_addr address){
	mem_addr sign = address;
	sign = sign >> 7;
	address = address << 26;
	address = address >> 26;

	if (sign == 1){
		return (0 - address);
	}
	else {
		return address;	}

}
