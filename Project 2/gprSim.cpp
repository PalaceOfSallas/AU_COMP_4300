// Stephen Sallas
// sms0107
// 10/20/2020
// COMP 4300
// Project 2 Multi-Cycle Machine Simulator

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
	ifstream file ("palindrome.s");
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
}

// Multi-cycle machine simulator
void Simulator::run()
{
	// Variables
	bool moreInstructions = true;
	int IC = 0;
	int C = 0;

	// Ofstream
	ofstream results ("result.txt");

	// Looping through all instructions
	while(moreInstructions) {
		// Incrementing instructions
		currentInstruction = mem->read(pc);
		pc++;

		// Using functions
		switch(opCode()) {
			// ADDI
			case 1: {
				int8_t immediate = signedValue(rightInstruction());
				uint32_t registerValue = registers->readRegister(centerInstruction());
				registers->writeRegister(leftInstruction(), immediate + registerValue);
				IC += 1;
				C += 6;
				break;
			}

			//B
			case 2: {
				int8_t offset = 0;
				offset = signedValue(rightInstruction());
				pc += offset;	
				IC += 1;
				C += 4;
				break;
			}
			
			//BEQZ
			case 3: {
				int8_t offset = 0;
				if (registers->readRegister(leftInstruction()) == 0) {
					offset = signedValue(rightInstruction());
					pc += offset;
				}
				IC += 1;
				C += 5;
				break;
			}

			//BGE 
			case 4: {
				int8_t offset = 0;
				if ( registers->readRegister(leftInstruction())  >=  registers->readRegister(centerInstruction())) {
					offset = signedValue(rightInstruction());
					pc += offset;
				}
				IC += 1;
				C += 5;
				break;
			}

			//BNE
			case 5: {
				int8_t offset = 0;
				if ( registers->readRegister(leftInstruction())  !=  registers->readRegister(centerInstruction())) {
					offset = signedValue(rightInstruction());
					pc += offset;
				}
				IC += 1;
				C += 5;
				break;
			}

			//LA
			case 6: {
				registers->writeRegister(leftInstruction(),immediateValue());
				IC += 1;
				C += 5;
				break;
			}

			//LB
			case 7: {
				mem_addr address_value = registers->readRegister(centerInstruction()); 		//number of bytes
				int8_t immediate = signedValue(rightInstruction());
				address_value += immediate;
				registers->writeRegister(leftInstruction(),mem->readByte(address_value, address_value%4) );
				IC += 1;
				C += 6;
				break;
			}

			//LI
			case 8: {
				registers->writeRegister(leftInstruction(), centerInstruction());
				IC += 1;
				C += 3;
				break;
			}

			//SUBI
			case 9: {
				int8_t immediate = signedValue(rightInstruction());
				uint32_t registerValue = registers->readRegister(centerInstruction());
				registers->writeRegister(leftInstruction(), registerValue - immediate);
				IC += 1;
				C += 6;
				break;
			}

			//SYSCALL
			case 10: {
				IC += 1;
				C += 8;
				switch(registers->readRegister(3)) {
					// Write string
					case 4: {
						cout << mem->readFromMemory(registers->readRegister(1)) << endl;
						cout << "See results for C, IC, and Speed-up in \"result.txt\"" << endl << endl;
						break;
					}

					// Read String
					case 8:	{
						char p[1024];
						string palindrome;

					    for (int i=0; i < 1024; i++) {
					            p[i]=0;
					    }

						cout << "Please enter a word: ";
						cin >> palindrome;
						palindrome.copy(p, 1024, 0);
						int length = strlen(p);
						p[length] = '\0';
						mem->writeToMemory(registers->readRegister(1), p);
						break;
					}

					// Ending program
					case 10: {
						moreInstructions = false;
						results << "IC: " << std::dec << IC << endl;
						results << "C: " << std::dec << C << endl;
						results << "Speed-up: " << (8.0*IC) / C ;
						break;
					}

					// Default
					default: {
						moreInstructions = false;
						break;
					}
				}
				break;
			}

			// Default
			default:
				moreInstructions = false;
				break;
		}
	}

	// Closing output stream
	results.close();
}

// Removes memory address
int Simulator::opCode() {															
	instruction op;					
	op = *currentInstruction;
	op = op >> 24;
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

		