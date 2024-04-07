- Stephen Sallas
- sms0107
- 09/22/2020
- COMP 4300

PROJECT 1

Overview:
Created two simulators, a stack machine and an accumulator.
They both read hexidecimal code from their respective .txt files.
They evaluate the expression AX^2 + BX + C.
X = 3
A = 7
B = 5
C = 4
ALL PROGRAMS WERE CREATED AND COMPILED IN VISUAL STUDIO CODE.

How to run Stack Simulator:
In a linux terminal type the following commands in order... 
g++ stackSim.cpp -o stackSim -std=c++11
./stackSim

How to run Accumulator Simulator:
In a linux terminal type the following commands in order... 
g++ accumSim.cpp -o accumSim -std=c++11
./accumSim

Issues:
Had to research to find that both programs need the "stdint.h" library to correctly implement uint32_t data types.
Found out that this code will not compile without a c++ 11 compiler due to the std::stoi function.
This is why the -std=c++11 command is necessary when compiling the code.


Please email me at sms0107@auburn.edu if you have any questions.
War eagle!





