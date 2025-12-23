#pragma once
#include "tokens.h"
#include "ast.h"
#include <stdexcept>
#include <vector>
#include <iostream>

class Parser {
public:
    // Constructor that takes a vector of tokens
    explicit  Parser(std::vector<Token*>& toks)
        : tokens(toks), pos(0) {
    }

	Module* parseModule();
	std::vector<Module*> parseProgram();
	std::vector<Expression*> parseArgList(); // helper

    // Parse one statement
    Statement* parseStatement();

	Expression* parseOr();
	Expression* parseAnd();
	Expression* parseUnary();

    // Parse expressions (supports + - * /)
    Expression* parseExpression();


    // Parse literals or identifiers 
    Expression* parsePrimary();

	IfItsStmt* parseIfIts();
	SelectStmt* parseSelect();

    // Utility helpers see lexer
    Token* peek();
    Token* peekNext();
    Token* advance();
    Token* consume(type expected, const std::string& msg);
    // match helper used in error handling
    bool match(type t);

private:
    std::vector<Token*>& tokens;    // reference to the token list
    size_t pos; 			        // current position in the token list

};




