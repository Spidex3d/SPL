#pragma once
#include "lexer.h"
#include <stdexcept>
// AST tree
struct ASTNode {
	virtual ~ASTNode() {}
};
// Base class for expressions
struct Expression : ASTNode {};

struct IntLiteral : Expression {
	int value;
	IntLiteral(int val) : value(val) {}
};

struct Identifier : Expression {
	std::string name;
	Identifier(const std::string& n) : name(n) {}
};

struct Statement : ASTNode {};

struct Assignment : Statement {
	std::string name;
	Expression* value;
	Assignment(const std::string& n, Expression* val) : name(n), value(val) {}

};

struct Pout : Statement {
	Expression* value;
	Pout(Expression* v) : value(v) {}
};

struct Declaration : Statement {
	std::string name;
	type varType; // Token_type for variable type
	Expression* value; // Optional initial value
	Declaration(const std::string& n, type t, Expression* v = nullptr)
	: name(n), varType(t), value(v) {}
};

struct BinaryExpr : Expression {
	Expression* left;
	type op; // TOKEN_PLUS, TOKEN_MINUS, etc.
	Expression* right;
	BinaryExpr(Expression* l, type o, Expression* r) : left(l), op(o), right(r) {}
};

class Parser {
public:
	Parser(std::vector<Token*>& toks) : tokens(toks), pos(0) {}

	Statement* parseStatement() {

		if (peek()->TYPE == TOKEN_DEC_I ||
			peek()->TYPE == TOKEN_DEC_S ||
			peek()->TYPE == TOKEN_DEC_F) {
			// Variable declaration
			
			type declType = advance()->TYPE; // consume dec_i/dec_s/dec_f
			Token* id = consume(TOKEN_IDENTIFIER, "Expected variable name");

			Expression* expr = nullptr;
			if (peek()->TYPE == TOKEN_EQUALS) {
				advance(); // consume '='
				expr = parseExpression();
			}

			consume(TOKEN_SEMICOLON, "Expected ';' after declaration");
			return new Declaration(id->VALUE, declType, expr);
		}

	
		// Expect IDENTIFIER
	else if (peek()->TYPE == TOKEN_IDENTIFIER) {
			
		Token* id = advance();
		
		consume(TOKEN_EQUALS, "Expected '='");
		Expression* expr = parseExpression();
		consume(TOKEN_SEMICOLON, "Expected ';'");
		return new Assignment(id->VALUE, expr);
	}

	else if (peek()->TYPE == TOKEN_POUT) {
		advance(); // consume 'pout'
		Expression* expr = parseExpression();
		consume(TOKEN_SEMICOLON, "Expected ';' after pout statement");
		return new Pout(expr);
	}
		// comments
	else if (peek()->TYPE == TOKEN_COMMENT)	{
			advance(); // consume comment
			return nullptr; // Ignore comments
		}

	else if (peek()->TYPE == TOKEN_EOF) {
			return nullptr; // End of input
	}
	else {
		throw std::runtime_error("Unexpected token in statement: " + peek()->VALUE);
		}
	}

	Expression* parseExpression() {
		Expression* left = parsePrimary();
		// plus , minus, times, divide
		while (peek()->TYPE == TOKEN_PLUS ||
			peek()->TYPE == TOKEN_MINUS ||
			peek()->TYPE == TOKEN_STAR ||
			peek()->TYPE == TOKEN_SLASH)
		{
			type op = advance()->TYPE; // consume operator
			Expression* right = parsePrimary();
			left = new BinaryExpr(left, op, right);
		}

		return left;
	}

	Expression* parsePrimary() {

		Token* t = advance();
		if (t->TYPE == TOKEN_INT) {
			return new IntLiteral(std::stoi(t->VALUE));
		}
		else if (t->TYPE == TOKEN_IDENTIFIER) {
			return new Identifier(t->VALUE);

		}
		throw std::runtime_error("Unexpected token in expression: " + t->VALUE);
	}
	Token* peek() { return tokens[pos]; }

private:
	std::vector<Token*>& tokens;
	size_t pos;

	Token* advance() { return tokens[pos++]; }

	Token* consume(type expected, const std::string& msg) {
		Token* t = advance();
		if (t->TYPE != expected) {
			throw std::runtime_error(msg);
		}
		return t;
	}
};

struct Var {
	type varType;
	std::string stringValue;
	int intValue; 
	float floatValue;
};

class Interpreter {
public:
	void execute(Statement* stmt) {
		if (auto* decl = dynamic_cast<Declaration*>(stmt)) {
			Var v{};
			v.varType = decl->varType;

			if (decl->value != nullptr) {
				if (decl->varType == TOKEN_DEC_I) {
					v.intValue = evalInt(decl->value);
				}
				else if (decl->varType == TOKEN_DEC_F) {
					v.floatValue = evalFloat(decl->value);
				}
				else if (decl->varType == TOKEN_DEC_S) {
					v.stringValue = evalString(decl->value);
				}
			}

			variables[decl->name] = v;
		}
		else if (auto* assign = dynamic_cast<Assignment*>(stmt)) {
			auto& v = variables[assign->name]; // must already exist
			if (v.varType == TOKEN_DEC_I) {
				v.intValue = evalInt(assign->value);
			}
			else if (v.varType == TOKEN_DEC_F) {
				v.floatValue = evalFloat(assign->value);
			}
			else if (v.varType == TOKEN_DEC_S) {
				v.stringValue = evalString(assign->value);
			}
		}
		else if (auto* pout = dynamic_cast<Pout*>(stmt)) {
			Expression* e = pout->value;
			// For simplicity, just assume ints for now
			std::cout << evalInt(e) << std::endl;
		}
	}

private:
	std::unordered_map<std::string, Var> variables;

	int evalInt(Expression* expr) {
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

	float evalFloat(Expression* expr) {
		// For now treat ints as floats
		if (auto* lit = dynamic_cast<IntLiteral*>(expr)) return (float)lit->value;
		if (auto* id = dynamic_cast<Identifier*>(expr)) return variables[id->name].floatValue;
		throw std::runtime_error("Invalid float expression");
	}

	std::string evalString(Expression* expr) {
		if (auto* id = dynamic_cast<Identifier*>(expr)) return variables[id->name].stringValue;
		throw std::runtime_error("Invalid string expression");
	}
};

