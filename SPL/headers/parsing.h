#pragma once
#include "tokens.h"
#include "ast.h"
#include <stdexcept>
#include <vector>
#include <iostream>



class Parser {
public:
    explicit  Parser(std::vector<Token*>& toks)
        : tokens(toks), pos(0) {}

    // Parse one statement
    Statement* parseStatement();

    // Parse expressions (supports + - * /)
    Expression* parseExpression();

    // Parse literals or identifiers
    Expression* parsePrimary();

    // Utility helpers
    Token* peek();
    Token* advance();
    Token* consume(type expected, const std::string& msg);
    bool match(type t);

private:
    std::vector<Token*>& tokens;
    size_t pos;


//public:
//	Parser(std::vector<Token*>& toks) : tokens(toks), pos(0) {}
//
//	Statement* parseStatement() {
//
//		if (peek()->TYPE == TOKEN_DEC_I ||
//			peek()->TYPE == TOKEN_DEC_S ||
//			peek()->TYPE == TOKEN_DEC_F) {
//			// Variable declaration
//			
//			type declType = advance()->TYPE; // consume dec_i/dec_s/dec_f
//			Token* id = consume(TOKEN_IDENTIFIER, "Expected variable name");
//
//			Expression* expr = nullptr;
//			if (peek()->TYPE == TOKEN_EQUALS) {
//				advance(); // consume '='
//				expr = parseExpression();
//			}
//
//			consume(TOKEN_SEMICOLON, "Expected ';' after declaration");
//			return new Declaration(id->VALUE, declType, expr);
//		}
//
//	
//		// Expect IDENTIFIER
//	else if (peek()->TYPE == TOKEN_IDENTIFIER) {
//			
//		Token* id = advance();
//		
//		consume(TOKEN_EQUALS, "Expected '='");
//		Expression* expr = parseExpression();
//		consume(TOKEN_SEMICOLON, "Expected ';'");
//		return new Assignment(id->VALUE, expr);
//	}
//
//	else if (peek()->TYPE == TOKEN_POUT) {
//		advance(); // consume 'pout'
//		Expression* expr = parseExpression();
//		consume(TOKEN_SEMICOLON, "Expected ';' after pout statement");
//		return new Pout(expr);
//	}
//		// comments
//	else if (peek()->TYPE == TOKEN_COMMENT)	{
//			advance(); // consume comment
//			return nullptr; // Ignore comments
//		}
//
//	else if (peek()->TYPE == TOKEN_EOF) {
//			return nullptr; // End of input
//	}
//	else {
//		throw std::runtime_error("Unexpected token in statement: " + peek()->VALUE);
//		//throw std::runtime_error("Unexpected token '" + t->VALUE + "' of type " + std::to_string(t->TYPE));
//
//		}
//	}
//
//	Expression* parseExpression() {
//		Expression* left = parsePrimary();
//		// plus , minus, times, divide
//		while (peek()->TYPE == TOKEN_PLUS ||
//			peek()->TYPE == TOKEN_MINUS ||
//			peek()->TYPE == TOKEN_STAR ||
//			peek()->TYPE == TOKEN_SLASH)
//		{
//			type op = advance()->TYPE; // consume operator
//			Expression* right = parsePrimary();
//			left = new BinaryExpr(left, op, right);
//		}
//
//		return left;
//	}
//
//	Expression* parsePrimary() {
//
//		Token* t = advance();
//		if (t->TYPE == TOKEN_INT) {
//			return new IntLiteral(std::stoi(t->VALUE));
//		}
//		else if (t->TYPE == TOKEN_IDENTIFIER) {
//			return new Identifier(t->VALUE);
//
//		}
//		if (t->TYPE == TOKEN_STRING) {
//			return new StringLiteral(t->VALUE);
//		}
//		throw std::runtime_error("Unexpected token in expression: " + t->VALUE);
//	}
//	Token* peek() { return tokens[pos]; }
//
//private:
//	std::vector<Token*>& tokens;
//	size_t pos;
//
//	Token* advance() { return tokens[pos++]; }
//
//	Token* consume(type expected, const std::string& msg) {
//		Token* t = advance();
//		if (t->TYPE != expected) {
//			throw std::runtime_error(msg);
//		}
//		return t;
//	}
};

