#pragma once
#include "parsing.h"

struct Var {
	type varType;
	std::string stringValue;
	int intValue;
	float floatValue;
};

class Interpreter {

public:
	Interpreter();
	
	std::unordered_map<std::string, Var> variables;

	void execute(Statement* stmt);

	int evalInt(Expression* expr);

	std::string evalString(Expression* expr);

	float evalFloat(Expression* expr);
	
};