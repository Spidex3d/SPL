#include "../include/parsing.h"

Statement* Parser::parseStatement()
{
    // Ignore comments
    if (peek()->TYPE == TOKEN_COMMENT) {
        advance();
        return nullptr;
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
    if (peek()->TYPE == TOKEN_IDENTIFIER) {
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

    // End of file
    if (peek()->TYPE == TOKEN_EOF) {
        return nullptr;
    }

    // Unexpected token
    throw std::runtime_error("Unexpected token in statement: '" + peek()->VALUE + "'");

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

    if (t->TYPE == TOKEN_INT) {
        return new IntLiteral(std::stoi(t->VALUE));
    }
    if (t->TYPE == TOKEN_STRING) {
        return new StringLiteral(t->VALUE);
    }
    if (t->TYPE == TOKEN_IDENTIFIER) {
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
