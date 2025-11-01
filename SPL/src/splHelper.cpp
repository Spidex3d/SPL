#include "../include/splHelper.h"
#include <iostream>`


void freeTokens(std::vector<Token*>& tokens)

{
	for (Token* token : tokens) {
		delete token;
	}
	tokens.clear();
}

void printTokens(const std::vector<Token*>& tokens)
{
	for (const Token* t : tokens) {
		std::cout << "Token Type: " << t->TYPE
			<< ", Value: '" << t->VALUE << "'" << std::endl;
	}
}
