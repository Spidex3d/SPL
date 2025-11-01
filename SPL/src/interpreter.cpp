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
	if (!stmt) return; // Ignore null statements (e.g., comments)

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
		return; // new
	}
	else if (auto* assign = dynamic_cast<Assignment*>(stmt)) {
		// auto& v = variables[assign->name]; // must already exist
		// new
		auto it = variables.find(assign->name);
		if (it == variables.end()) {
			throw std::runtime_error("Undifined Variable '" + assign->name + "' not declared.");
		}
		Var& v = it->second;
		
		// ------------------------------------
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
		return;
	}
	// new
	if (auto* p = dynamic_cast<Pout*>(stmt)) {
		Expression* e = p->value;
		// Check if the expression is a string expression
		if (isStringExpr(e)) {
			std::cout << evalString(e) << std::endl;
		}
		else {
			// Assume it's an int expression for now
			std::cout << evalInt(e) << std::endl;
		}
		return;
	}	
	// string
	/*else if (auto* pout = dynamic_cast<Pout*>(stmt)) {
		if (auto* str = dynamic_cast<StringLiteral*>(pout->value)) {
			std::cout << str->value << std::endl;
		}
		else if (auto* id = dynamic_cast<Identifier*>(pout->value)) {
			auto& v = variables[id->name];
			if (v.varType == TOKEN_DEC_S) std::cout << v.stringValue << std::endl;
			else if (v.varType == TOKEN_DEC_I) std::cout << v.intValue << std::endl;
		}
	} */
}


int Interpreter::evalInt(Expression* expr) {
	if (auto* lit = dynamic_cast<IntLiteral*>(expr)) {
		return lit->value;
	}
	if (auto* id = dynamic_cast<Identifier*>(expr)) {
		auto it = variables.find(id->name);
		if (it == variables.end())
			throw std::runtime_error("Undefined variable: " + id->name);
		if (it->second.varType != TOKEN_DEC_I)
			throw std::runtime_error("Type error: '" + id->name + "' is not an int");
		return it->second.intValue;
	}
	if (auto* bin = dynamic_cast<BinaryExpr*>(expr)) {
		// If either side is string and op is PLUS, this is string concat → not an int
		if (bin->op == TOKEN_PLUS && (isStringExpr(bin->left) || isStringExpr(bin->right))) {
			throw std::runtime_error("Type error: '+' on strings used in int context");
		}
		int l = evalInt(bin->left);
		int r = evalInt(bin->right);
		switch (bin->op) {
		case TOKEN_PLUS:  return l + r;
		case TOKEN_MINUS: return l - r;
		case TOKEN_STAR:  return l * r;
		case TOKEN_SLASH: return r == 0 ? 0 : l / r; // simple guard
		default: break;
		}
	}
	// String in an int position:
	if (dynamic_cast<StringLiteral*>(expr)) {
		throw std::runtime_error("Type error: string where int expected");
	}
	throw std::runtime_error("Invalid int expression");
}

/*
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
*/

std::string Interpreter::evalString(Expression* expr) {
	if (auto* s = dynamic_cast<StringLiteral*>(expr)) {
		return s->value;
	}
	if (auto* id = dynamic_cast<Identifier*>(expr)) {
		auto it = variables.find(id->name);
		if (it == variables.end())
			throw std::runtime_error("Undefined variable: " + id->name);
		const Var& v = it->second;
		if (v.varType == TOKEN_DEC_S) return v.stringValue;
		if (v.varType == TOKEN_DEC_I) return std::to_string(v.intValue);
		// simple fallback for future float
		if (v.varType == TOKEN_DEC_F) return std::to_string(v.floatValue);
		return "";
	}
	if (auto* lit = dynamic_cast<IntLiteral*>(expr)) {
		return std::to_string(lit->value);
	}
	if (auto* bin = dynamic_cast<BinaryExpr*>(expr)) {
		if (bin->op == TOKEN_PLUS) {
			// Concatenate with coercion to string on both sides
			return evalString(bin->left) + evalString(bin->right);
		}
		// Other operators on strings are not supported
		throw std::runtime_error("Type error: invalid operator on string");
	}
	throw std::runtime_error("Invalid string expression");
}

// Evaluate expressions strings
/*
std::string Interpreter::evalString(Expression* expr) {
	if (auto* std = dynamic_cast<StringLiteral*>(expr)) {
		return std->value;
	}
	if (auto* id = dynamic_cast<Identifier*>(expr)) {
		return variables[id->name].stringValue;
	}
	throw std::runtime_error("Invalid string expression");
}
*/

// Evaluate expressions floats
float Interpreter::evalFloat(Expression* expr) {
	// For now treat ints as floats
	if (auto* lit = dynamic_cast<IntLiteral*>(expr)) return (float)lit->value;
	if (auto* id = dynamic_cast<Identifier*>(expr)) return variables[id->name].floatValue;
	throw std::runtime_error("Invalid float expression");
}
// Determine if an expression evaluates to a string --- string concatenation ---
bool Interpreter::isStringExpr(Expression* expr)
{
	if (dynamic_cast<StringLiteral*>(expr)) return true;

	if (auto* id = dynamic_cast<Identifier*>(expr)) {
		auto it = variables.find(id->name);
		return it != variables.end() && it->second.varType == TOKEN_DEC_S;
	}

	if (auto* bin = dynamic_cast<BinaryExpr*>(expr)) {
		// String if either side is string and op is PLUS (concat)
		if (bin->op == TOKEN_PLUS) {
			return isStringExpr(bin->left) || isStringExpr(bin->right);
		}
		return false;
	}

	// IntLiteral or anything else defaults to non-string
	return false;
}
