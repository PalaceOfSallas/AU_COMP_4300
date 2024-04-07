// Stephen Sallas
// sms0107
// 09-22-2020
// COMP 4300

// Including necessary libraries
#include <cstdlib>
#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

//***********************************************************************************************************
//********************************* Initializers & Main *****************************************************
//***********************************************************************************************************

// Defining text, data, and stack segment length
#define T_SEG_LEN  100
#define	D_SEG_LEN  50
#define S_SEG_LEN  50

// Defining memory address and instruction
typedef uint32_t mem_addr;
typedef uint32_t instruction;

// Creating text, data, and stack arrays
instruction textSeg[T_SEG_LEN];
mem_addr dataSeg[D_SEG_LEN];
mem_addr stackSeg[S_SEG_LEN];

// Defining the top address of each array in hex
mem_addr textTop = 0x00100000;
mem_addr dataTop = 0x00200000;
mem_addr stackTop = 0x00300000;

// Initializing memory class and all of its functions
class Memory {
public:
	Memory();
	void readText(mem_addr address);						
	void readData(mem_addr address, mem_addr data);		
    void writeStack(mem_addr address, mem_addr data);			
    mem_addr *read(mem_addr address);					
	int decodeBin(mem_addr address);				
	int decodeIndex(mem_addr address);			
	int textSegCounter;								
};

class Simulator {
public:
	Simulator();									
	void run();										
	int opCode();							
	mem_addr setMemoryAddress();			
	mem_addr stackTop2;							
	mem_addr pc;									
	instruction *currentInstruction;				
	Memory *mem;
};

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

	//Setting text segement counter to the bottom
	textSegCounter = -1;

	// Opening hex file
	ifstream file ("stackCode.txt");
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
			
			// Rading in data
			if (i == 1) {												
				for (int c = 0; c < 10; c++)
				{
					line2[c] = line[c];
					line3[0] = line[11];
				}
				dataHex = std::stoi(line2.c_str(),0, 16);
				dataHex2 = atoi(line3.c_str());
				readData(dataHex,dataHex2);
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

// Reads data and adds it to data segment array
void Memory::readData(mem_addr address, mem_addr data) {
	int memoryIndex = (int) decodeIndex(address);
	if (textSegCounter < D_SEG_LEN)	{
		dataSeg[memoryIndex] = data;									
	}
}

// Reads data and adds it to stack segment array
void Memory::writeStack(mem_addr address, mem_addr data) {	
	int memoryIndex = (int) decodeIndex(address);
	if (textSegCounter < S_SEG_LEN)	{
		stackSeg[memoryIndex] = data;									
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
	address = address << 7;
	address = address >> 27;
	return address;
}

// Decodes memory address to remove bin and op code and reset memory
int Memory::decodeIndex(mem_addr address) {																
	address = address << 15;	
	address = address >> 15;
	return address;
}


//***********************************************************************************************************
//********************************* Simulator Functions *****************************************************
//***********************************************************************************************************

Simulator::Simulator()
{
	pc = textTop;
	stackTop2 = stackTop;
	mem = new Memory();
}

// Simulating stack machine
void Simulator::run() {	

	// Setting instructions true until ned of file												
	bool instructions = true;					
	while(instructions) {
		
		// Loading next instruction and incrementing pc
		currentInstruction = mem->read(pc);
		pc++;
		
		// Running each instruction
		switch(opCode()) {

			// Push function
			case 1: {
				mem_addr *data = mem->read(setMemoryAddress());
				mem->writeStack(stackTop2, *data);
				stackTop2++;
				break;
			}

			// Pop function
			case 2: {
				stackTop2--;
				mem_addr *data = mem->read(stackTop2);
				cout << std::dec << *data << " popped from stack." << endl;
				break;
			}

			// Add function
			case 3: {
				stackTop2--;
				mem_addr *first_data = mem->read(stackTop2);
				stackTop2--;
				mem_addr *second_data = mem->read(stackTop2);
				mem_addr result = *first_data + *second_data;
				mem->writeStack(stackTop2, result);
				stackTop2++;
				break;
			}

			// Multiply function
			case 4:	{
				stackTop2--;
				mem_addr *first_data = mem->read(stackTop2);
				stackTop2--;
				mem_addr *second_data = mem->read(stackTop2);
				mem_addr result = *first_data * *second_data;
				mem->writeStack(stackTop2, result);
				stackTop2++;
				break;
			}

			// End program function
			case 5:	{
				instructions = false;
				break;
			}
			default:
				break;
		}
	}
}

// Removing bin from front of stack
int Simulator::opCode() {															
	instruction op;
	op = *currentInstruction;
	op = op >> 24;
	return op;
}

// Removing bin from front of stack
mem_addr Simulator::setMemoryAddress() {															
	instruction memoryAddress;
	memoryAddress = *currentInstruction;
	memoryAddress = memoryAddress << 8;
	memoryAddress = memoryAddress >> 8;
	return memoryAddress;
}


