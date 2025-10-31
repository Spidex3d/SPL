#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>


#include "../headers/lexer.h"
#include "../headers/parsing.h"
#include "../headers/interpreter.h"
#include "../headers/splHelper.h"


int main() {

	std::cout << "Reading from the .spl file!" << std::endl;

	// hard-coded test file (must be in the same folder as your .exe or give full path)
	std::ifstream sourceFileStream("test.spl");
	if (!sourceFileStream) {
		std::cerr << "Error: could not open file test.spl" << std::endl;
		return 1;
	}

	std::stringstream buffer;
	buffer << sourceFileStream.rdbuf(); // simpler way to read entire file

	std::string sourceCode = buffer.str();
	// Print the source code
	//std::cout << "Source Code:\n" << sourceCode << std::endl;
	// Tokenize the source code
	Lexer lexer(sourceCode);
	std::vector<Token *> tokens = lexer.tokenize();

	// Parse the tokens
	Parser parser(tokens);
	Interpreter interp;
	
	try {
		while (true)
		{
			Statement* stmt = parser.parseStatement();
			if (stmt == nullptr) {
				if (parser.peek()->TYPE == TOKEN_EOF) break;
				else continue;
				}
			 interp.execute(stmt);
		}
			//if (stmt == nullptr) break; // End of input
			//interp.execute(stmt);
		
	} 
	catch (std::runtime_error& e) {
			// Assuming the error is due to end of input
			std::cout << "Parsing completed." << e.what() << std::endl;
	}
	// Debug: print all tokens
	//printTokens(tokens);

	std::cout << "This is the end of the program" << std::endl;

	freeTokens(tokens);

	return 0;

	
}






