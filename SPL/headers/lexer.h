#pragma once
#include <string>
#include <vector>
#include "tokens.h"

class Lexer {	
public:
	explicit Lexer(const std::string& sourceCode);
	
	char advance();
		
	char peek(int offset = 0);
	std::vector<Token*> tokenize();
	
private:
	std::string source;
	int cursor;
	int size;
	char current;

};

//void freeTokens(std::vector<Token*>& tokens);




