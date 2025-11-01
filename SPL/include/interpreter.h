#pragma once
#include "parsing.h"

struct Var {
	type varType; // TOKEN_DEC_I, TOKEN_DEC_S, TOKEN_DEC_F
	std::string stringValue; // for string variables
	int intValue;	   // for int variables
	float floatValue;	 // for float variables
};

class Interpreter {

public:
	Interpreter(); // constructor

	// store variables in a map
	std::unordered_map<std::string, Var> variables;
	// execute a statement from the AST
	void execute(Statement* stmt);
	// evaluate an Int expression from the AST
	int evalInt(Expression* expr);
	// evaluate a String expression from the AST
	std::string evalString(Expression* expr);
	// evaluate a Float expression from the AST
	float evalFloat(Expression* expr);

	bool isStringExpr(Expression* expr);
	
};