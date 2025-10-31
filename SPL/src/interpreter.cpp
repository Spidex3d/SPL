#include <stdexcept>
#include <string>
#include <vector>
#include "../headers/interpreter.h"
//#include "../headers/parsing.h"
//#include "../headers/lexer.h"
#include <iostream>

Interpreter::Interpreter()
{

}

//std::unordered_map<std::string, Var> variables;

void Interpreter::execute(Statement* stmt) {
	if (auto* decl = dynamic_cast<Declaration*>(stmt)) {
		Var v{};
		v.varType = decl->varType;

		if (decl->value != nullptr) {
			// int
			if (decl->varType == TOKEN_DEC_I) {
				v.intValue = evalInt(decl->value);
			}
			// string
			else if (decl->varType == TOKEN_DEC_S) {
				v.stringValue = evalString(decl->value);
			}
			// float
			else if (decl->varType == TOKEN_DEC_F) {
				v.floatValue = evalFloat(decl->value);
			}
		}

		variables[decl->name] = v;
	}
	else if (auto* assign = dynamic_cast<Assignment*>(stmt)) {
		auto& v = variables[assign->name]; // must already exist
		// int
		if (v.varType == TOKEN_DEC_I) {
			v.intValue = evalInt(assign->value);
		}
		// string
		else if (v.varType == TOKEN_DEC_S) {
			v.stringValue = evalString(assign->value);
		}
		// float
		else if (v.varType == TOKEN_DEC_F) {
			v.floatValue = evalFloat(assign->value);
		}
	}
	// string
	else if (auto* pout = dynamic_cast<Pout*>(stmt)) {
		if (auto* str = dynamic_cast<StringLiteral*>(pout->value)) {
			std::cout << str->value << std::endl;
		}
		else if (auto* id = dynamic_cast<Identifier*>(pout->value)) {
			auto& v = variables[id->name];
			if (v.varType == TOKEN_DEC_S) std::cout << v.stringValue << std::endl;
			else if (v.varType == TOKEN_DEC_I) std::cout << v.intValue << std::endl;
		}
	}
}



int Interpreter::evalInt(Expression* expr) {
	if (auto* lit = dynamic_cast<IntLiteral*>(expr)) return lit->value;
	if (auto* id = dynamic_cast<Identifier*>(expr)) return variables[id->name].intValue;
	if (auto* bin = dynamic_cast<BinaryExpr*>(expr)) {
		int leftVal = evalInt(bin->left);
		int rightVal = evalInt(bin->right);
		switch (bin->op) {
		case TOKEN_PLUS:  return leftVal + rightVal;
		case TOKEN_MINUS: return leftVal - rightVal;
		case TOKEN_STAR:  return leftVal * rightVal;
		case TOKEN_SLASH: return leftVal / rightVal;
		}
	}

	throw std::runtime_error("Invalid int expression");
}

// Evaluate expressions strings
std::string Interpreter::evalString(Expression* expr) {
	if (auto* std = dynamic_cast<StringLiteral*>(expr)) {
		return std->value;
	}
	if (auto* id = dynamic_cast<Identifier*>(expr)) {
		return variables[id->name].stringValue;
	}
	throw std::runtime_error("Invalid string expression");
}

// Evaluate expressions floats
float Interpreter::evalFloat(Expression* expr) {
	// For now treat ints as floats
	if (auto* lit = dynamic_cast<IntLiteral*>(expr)) return (float)lit->value;
	if (auto* id = dynamic_cast<Identifier*>(expr)) return variables[id->name].floatValue;
	throw std::runtime_error("Invalid float expression");
}