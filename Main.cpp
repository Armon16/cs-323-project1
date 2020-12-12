// Assignment 3: Symbol Table & Assembly Code
// CPSC 323-02, CSUF Fall 2020
// Gage Dimapindan, Richard Gobea, Armon Rahimi

#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <vector>
#include <algorithm>
#include <iomanip>
#include "class.h"
#include "lexer.h"
#include "Machine.h"

int main(int argument, const char* argument_2[]) {
	std::string testName;
	std::cout << "Welcome to Assignment 3 for CPSC 323" << std::endl;
	std::cout << "This code is brought to you by Richard Gobea, Armon Rahimi, and Gage Dimapindan" << std::endl;
	std::cout << "Please enter the name of the file you wish to process" << std::endl;
	std::cin >> testName;
	std::ifstream source(testName);
	std::ofstream out("output1.txt");
	if (!out.is_open()) {
		std::cout << "Output file failed to open\n";
		return 2;
	}

	if (!source.is_open()) {
		std::cout << "Input file failed to open\n";
		return 2;
	}

	Rat20F(out, source);

	out << "Symbol Table:\n\n";
	out << "Identifier\t" << "MemoryLocation\t\t\tType\n";
	std::vector<Character>::iterator i = table.begin();
	while (i != table.end()) {
		out << "\t" << i->getSym().getLexeme() << "\t\t\t\t\t\t" << i->getAddress() << "\t\t\t\t\t" << i->getSym().getToken() << "\n";
		i++;
	}

	print_instr(out);

	out.close();
	source.close();
	std::cout << "Main is now finished. Please view results inside file labeled 'output1.txt'" << std::endl;
	std::cout << "Main will now return 0. Goodbye" << std::endl;
	return 0;
}
