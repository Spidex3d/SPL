#include "../include/parsing.h"

std::vector<Module*> Parser::parseProgram()
{
    std::vector<Module*> mods;
    while (peek()->TYPE != TOKEN_EOF) {
        if (peek()->TYPE == TOKEN_COMMENT) {
            advance(); // consume comment
            continue; // Ignore comments
        }
        if (peek()->TYPE != TOKEN_MOD) {
            throw std::runtime_error("Expected 'mod' keyword at the beginning of a module, got '" + peek()->VALUE + "'");
        }
        mods.push_back(parseModule());
        
    }
    return mods;
}

std::vector<Expression*> Parser::parseArgList()
{
    std::vector<Expression*> args;
    if (peek()->TYPE == TOKEN_RIGHT_PAREN) return args;
    do {
        args.push_back(parseExpression());
    } while (match(TOKEN_COMMA));
    return args;
}


Module* Parser::parseModule()
{
	consume(TOKEN_MOD, "Expected 'mod'");
	Token* nameTok = consume(TOKEN_IDENTIFIER, "Expected module name");
	consume(TOKEN_LEFT_PAREN, "Expected '(' after module name");
	std::vector<std::string> params;

	if (peek()->TYPE != TOKEN_RIGHT_PAREN) {
		do {
			Token* paramTok = consume(TOKEN_IDENTIFIER, "Expected parameter name");
			params.push_back(paramTok->VALUE);
		} while (match(TOKEN_COMMA));
	}

	consume(TOKEN_RIGHT_PAREN, "Expected ')' after module name");
	consume(TOKEN_SEMICOLON, "Expected ';' after module declaration");

	std::vector<Statement*> body;
	while (peek()->TYPE != TOKEN_MODEND && peek()->TYPE != TOKEN_EOF) {
		if (peek()->TYPE == TOKEN_COMMENT) {
			advance(); // consume comment
			continue; // Ignore comments
		}
		Statement* s = parseStatement();
		if (s) body.push_back(s); // can return nullptr for comments; gauard if needed
	}

	consume(TOKEN_MODEND, "Expected 'modEnd' at the end of module");
	consume(TOKEN_LEFT_PAREN, "Expected '(' after 'modEnd'");
	consume(TOKEN_RIGHT_PAREN, "Expected ')' after 'modEnd'");
	consume(TOKEN_SEMICOLON, "Expected ';' after 'modEnd()'");

	return new Module(nameTok->VALUE, std::move(params), std::move(body));
}



Statement* Parser::parseStatement()
{
    // Ignore comments
    if (peek()->TYPE == TOKEN_COMMENT) { advance(); return nullptr; }

    if (peek()->TYPE == TOKEN_RETURN) { advance();
	Expression* v = parseExpression();
    consume(TOKEN_SEMICOLON, "Expected ';' after return");
		return new ReturnStmt(v);
	}

    // Variable declaration
    if (peek()->TYPE == TOKEN_DEC_I ||
        peek()->TYPE == TOKEN_DEC_S ||
        peek()->TYPE == TOKEN_DEC_F) {

        type declType = advance()->TYPE; // consume dec_i/dec_s/dec_f
        Token* id = consume(TOKEN_IDENTIFIER, "Expected variable name");

        Expression* expr = nullptr;
        if (match(TOKEN_EQUALS)) {
            expr = parseExpression();
        }

        consume(TOKEN_SEMICOLON, "Expected ';' after declaration");
        return new Declaration(id->VALUE, declType, expr);
    }

    // Assignment
    if (peek()->TYPE == TOKEN_IDENTIFIER && tokens[pos + 1]->TYPE == TOKEN_EQUALS) {
    //if (peek()->TYPE == TOKEN_IDENTIFIER) {
        Token* id = advance();
        consume(TOKEN_EQUALS, "Expected '='");
        Expression* expr = parseExpression();
        consume(TOKEN_SEMICOLON, "Expected ';'");
        return new Assignment(id->VALUE, expr);
    }

    // Pout (print)
    if (peek()->TYPE == TOKEN_POUT) {
        advance(); // consume 'pout'
        Expression* expr = parseExpression();
        consume(TOKEN_SEMICOLON, "Expected ';' after pout statement");
        return new Pout(expr);
    }

	Expression* e = parseExpression();
	consume(TOKEN_SEMICOLON, "Expected ';' after expression statement");
	return new ExprStmt(e);

}

Expression* Parser::parseExpression()
{
    Expression* left = parsePrimary();

    while (peek()->TYPE == TOKEN_PLUS ||
        peek()->TYPE == TOKEN_MINUS ||
        peek()->TYPE == TOKEN_STAR ||
        peek()->TYPE == TOKEN_SLASH)
    {
        type op = advance()->TYPE;
        Expression* right = parsePrimary();
        left = new BinaryExpr(left, op, right);
    }

    return left;
}

Expression* Parser::parsePrimary()
{
    Token* t = advance();

    if (t->TYPE == TOKEN_INT)  return new IntLiteral(std::stoi(t->VALUE));
    
    if (t->TYPE == TOKEN_STRING) return new StringLiteral(t->VALUE);
    
    if (t->TYPE == TOKEN_IDENTIFIER) { 
        if (peek()->TYPE == TOKEN_LEFT_PAREN) {
            advance(); // consume '('
            
			auto args = parseArgList();

            consume(TOKEN_RIGHT_PAREN, "Expected ')'");
            return new CallExpr(t->VALUE, std::move(args));
        }


        return new Identifier(t->VALUE);
    }

    throw std::runtime_error("Unexpected token in expression: '" + t->VALUE + "'");

}

Token* Parser::peek()
{
	if (pos >= tokens.size()) return tokens.back(); // assume EOF
	return tokens[pos];
}

Token* Parser::advance()
{
	if (pos < tokens.size()) return tokens[pos++];
	return tokens.back();
}

Token* Parser::consume(type expected, const std::string& msg)
{
	Token* t = advance();
	if (t->TYPE != expected) {
		throw std::runtime_error(msg + " (got '" + t->VALUE + "')");
	}
	return t;
}

bool Parser::match(type t)
{
	if (peek()->TYPE == t) {
		advance();
		return true;
	}
	return false;
}
