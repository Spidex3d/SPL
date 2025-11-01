#pragma once
#include <string>
#include <vector>
#include "tokens.h"

class Lexer {	
public:
	// Initialize lexer with source code
	explicit Lexer(const std::string& sourceCode);
	// Get next character and advance cursor
	char advance();
	// Get current character without advancing
	char peek(int offset = 0);
	// Tokenize the entire source code
	std::vector<Token*> tokenize();
	
private:
	std::string source; // entire source code to be tokenized
	int cursor;			// current position in source code
	int size;			// size of source code	
	char current;		// current character being analyzed

};






