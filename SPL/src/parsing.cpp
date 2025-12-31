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
    // 0) Ignore comments
    if (peek()->TYPE == TOKEN_COMMENT) { advance(); return nullptr; }

    // 1) IfIts / ElseIts / Else / EndIfIts
    if (peek()->TYPE == TOKEN_IFITS) {
        return parseIfIts();
    }

    // 2) Select
    if (peek()->TYPE == TOKEN_SELECT) {
        return parseSelect();
    }

    // 3) Return
    if (peek()->TYPE == TOKEN_RETURN) {
        advance(); // consume return
        Expression* v = parseExpression();
        consume(TOKEN_SEMICOLON, "Expected ';' after return");
        return new ReturnStmt(v);
    }

    // 4) Array assignment:  names[index] = value;
    // MUST be before normal assignment and before expression-statement fallback.
    if (peek()->TYPE == TOKEN_IDENTIFIER &&
        pos + 1 < tokens.size() &&
        tokens[pos + 1]->TYPE == TOKEN_LEFT_BRACKET)
    {
        Token* id = advance(); // name
        consume(TOKEN_LEFT_BRACKET, "Expected '[' after array name");
        Expression* index = parseExpression();
        consume(TOKEN_RIGHT_BRACKET, "Expected ']' after array index");
        consume(TOKEN_EQUALS, "Expected '=' after array index");
        Expression* value = parseExpression();
        consume(TOKEN_SEMICOLON, "Expected ';' after array assignment");
        return new ArrayAssignStmt(id->VALUE, index, value);
    }

    // 5) Declarations (scalar + array)
    if (peek()->TYPE == TOKEN_DEC_I ||
        peek()->TYPE == TOKEN_DEC_S ||
        peek()->TYPE == TOKEN_DEC_F ||
        peek()->TYPE == TOKEN_DEC_B)
    {
        type declType = advance()->TYPE;  // dec_i / dec_s / dec_f / dec_b
        Token* id = consume(TOKEN_IDENTIFIER, "Expected variable name");

        // ---- Array declaration: dec_s names[3] { ... };
		if (match(TOKEN_LEFT_BRACKET)) {   // '['
            int size = -1;

            // Optional explicit size: [3]
            if (peek()->TYPE == TOKEN_INT) {
                size = std::stoi(advance()->VALUE);
            }

            consume(TOKEN_RIGHT_BRACKET, "Expected ']' after array size");

            // Allow optional '=' : arr[3] = { ... } OR arr[3] { ... }
            match(TOKEN_EQUALS);

            // Now must see '{'
            consume(TOKEN_LEFT_CURL_PAREN, "Expected '{' for array initializer");

            std::vector<Expression*> values;

            // Parse: { expr (, expr)* }
            // also allow empty: { }
            while (peek()->TYPE != TOKEN_RIGHT_CURL_PAREN &&
                peek()->TYPE != TOKEN_EOF)
            {
                values.push_back(parseExpression());

                if (peek()->TYPE == TOKEN_COMMA) {
                    advance(); // eat the comma
                    continue;
                }

                // next must be '}' otherwise syntax error
                if (peek()->TYPE != TOKEN_RIGHT_CURL_PAREN) {
                    throw std::runtime_error(
                        "Expected ',' or '}' in array initializer, got '" + peek()->VALUE + "'");
                }
            }

            consume(TOKEN_RIGHT_CURL_PAREN, "Expected '}' after array initializer");
            consume(TOKEN_SEMICOLON, "Expected ';' after array declaration");

            return new ArrayDeclaration(declType, id->VALUE, size, std::move(values));
        }

        // ---- Normal scalar declaration: dec_i x = 5;
        Expression* expr = nullptr;
        if (match(TOKEN_EQUALS)) {
            expr = parseExpression();
        }

        consume(TOKEN_SEMICOLON, "Expected ';' after declaration");
        return new Declaration(id->VALUE, declType, expr);
    }

    // 6) While loop
    if (peek()->TYPE == TOKEN_WHILE) {
        advance(); // consume 'while'
        consume(TOKEN_LEFT_PAREN, "Expected '(' after while");
        Expression* cond = parseExpression();
        consume(TOKEN_RIGHT_PAREN, "Expected ')' after while condition");
        consume(TOKEN_LEFT_CURL_PAREN, "Expected '{' after while(...)");

        std::vector<Statement*> body;
        while (peek()->TYPE != TOKEN_RIGHT_CURL_PAREN) {
            Statement* s = parseStatement();
            if (s) body.push_back(s);
        }
        consume(TOKEN_RIGHT_CURL_PAREN, "Expected '}' after while body");
        match(TOKEN_SEMICOLON); // optional
        return new WhileStmt(cond, std::move(body));
    }

    // 7) Do loop (your 3-part loop)
    if (peek()->TYPE == TOKEN_DO) {
        advance(); // consume 'do'
        consume(TOKEN_LEFT_PAREN, "Expected '(' after do");

        // init is a full statement that must end with ';'
        Statement* init = parseStatement();

        Expression* cond = parseExpression();
        consume(TOKEN_SEMICOLON, "Expected ';' in do loop after condition");

        // step: j++ OR j = expr   (NO trailing ';' in header)
        Statement* step = nullptr;

        if (peek()->TYPE == TOKEN_IDENTIFIER &&
            pos + 1 < tokens.size() &&
            tokens[pos + 1]->TYPE == TOKEN_INCREMENT)
        {
            Token* idTok = advance();
            advance(); // '++'

            Expression* one = new IntLiteral(1);
            Expression* leftId = new Identifier(idTok->VALUE);
            Expression* plus = new BinaryExpr(leftId, TOKEN_PLUS, one);
            step = new Assignment(idTok->VALUE, plus);
        }
        else if (peek()->TYPE == TOKEN_IDENTIFIER &&
            pos + 1 < tokens.size() &&
            tokens[pos + 1]->TYPE == TOKEN_DECREASE)
        {
            Token* idTok = advance();
            advance(); // '--'

            Expression* one = new IntLiteral(1);
            Expression* leftId = new Identifier(idTok->VALUE);
            Expression* minus = new BinaryExpr(leftId, TOKEN_MINUS, one);
            step = new Assignment(idTok->VALUE, minus);
        }
        else if (peek()->TYPE == TOKEN_IDENTIFIER &&
            pos + 1 < tokens.size() &&
            tokens[pos + 1]->TYPE == TOKEN_EQUALS)
        {
            Token* idTok = advance();
            consume(TOKEN_EQUALS, "Expected '=' in do-loop step");
            Expression* rhs = parseExpression();
            step = new Assignment(idTok->VALUE, rhs);
        }
        else {
            throw std::runtime_error("Expected step statement (e.g. j++, j--, or j = expr) in do-loop");
        }

        consume(TOKEN_RIGHT_PAREN, "Expected ')' after do condition");
        consume(TOKEN_LEFT_CURL_PAREN, "Expected '{' after do (...)");

        std::vector<Statement*> body;
        while (peek()->TYPE != TOKEN_RIGHT_CURL_PAREN) {
            Statement* s = parseStatement();
            if (s) body.push_back(s);
        }
        consume(TOKEN_RIGHT_CURL_PAREN, "Expected '}' after do body");

        match(TOKEN_SEMICOLON); // optional
        return new DoStmt(init, cond, step, std::move(body));
    }

    // 8) i++ statement
    if (peek()->TYPE == TOKEN_IDENTIFIER &&
        pos + 1 < tokens.size() &&
        tokens[pos + 1]->TYPE == TOKEN_INCREMENT)
    {
        Token* idTok = advance();
        advance(); // '++'
        consume(TOKEN_SEMICOLON, "Expected ';' after ++");

        Expression* one = new IntLiteral(1);
        Expression* leftId = new Identifier(idTok->VALUE);
        Expression* plus = new BinaryExpr(leftId, TOKEN_PLUS, one);
        return new Assignment(idTok->VALUE, plus);
    }

    // 9) i-- statement
    if (peek()->TYPE == TOKEN_IDENTIFIER &&
        pos + 1 < tokens.size() &&
        tokens[pos + 1]->TYPE == TOKEN_DECREASE)
    {
        Token* idTok = advance();
        advance(); // '--'
        consume(TOKEN_SEMICOLON, "Expected ';' after --");

        Expression* one = new IntLiteral(1);
        Expression* leftId = new Identifier(idTok->VALUE);
        Expression* minus = new BinaryExpr(leftId, TOKEN_MINUS, one);
        return new Assignment(idTok->VALUE, minus);
    }

    // 10) Normal assignment: x = expr;
    if (peek()->TYPE == TOKEN_IDENTIFIER &&
        pos + 1 < tokens.size() &&
        tokens[pos + 1]->TYPE == TOKEN_EQUALS)
    {
        Token* id = advance();
        consume(TOKEN_EQUALS, "Expected '='");
        Expression* expr = parseExpression();
        consume(TOKEN_SEMICOLON, "Expected ';'");
        return new Assignment(id->VALUE, expr);
    }

    // 11) Pout
    if (peek()->TYPE == TOKEN_POUT) {
        advance(); // consume 'pout'
        Expression* expr = parseExpression();
        consume(TOKEN_SEMICOLON, "Expected ';' after pout statement");
        return new Pout(expr);
    }

    // 12) Expression statement fallback (module calls etc.)
    Expression* e = parseExpression();
    consume(TOKEN_SEMICOLON, "Expected ';' after expression statement");
    return new ExprStmt(e);
}

// ------------------------------------------------------------ OR AND NOT ------------------------------------------------
Expression* Parser::parseOr()
{
    Expression* left = parseAnd();
    while (peek()->TYPE == TOKEN_OR) {
        type op = advance()->TYPE;
        Expression* right = parseAnd();
        left = new BinaryExpr(left, op, right);
    }
    return left;
}

Expression* Parser::parseAnd()
{
    Expression* left = parseUnary();
    while (peek()->TYPE == TOKEN_AND) {
        type op = advance()->TYPE;
        Expression* right = parseUnary();
        left = new BinaryExpr(left, op, right);
    }
    return left;
}

Expression* Parser::parseUnary()
{
    // Handle logical NOT with highest precedence
    if (peek()->TYPE == TOKEN_NOT) {
        advance(); // consume '!'
        Expression* e = parseUnary();
        return new UnaryExpr(TOKEN_NOT, e);
    }

    // Base: start with a primary
    Expression* left = parsePrimary();

    // Then handle arithmetic + comparison operators (left-assoc)
    while (peek()->TYPE == TOKEN_PLUS ||
        peek()->TYPE == TOKEN_MINUS ||
        peek()->TYPE == TOKEN_STAR ||
        peek()->TYPE == TOKEN_SLASH ||
        peek()->TYPE == TOKEN_ISLESS ||
        peek()->TYPE == TOKEN_ISMORE ||
        peek()->TYPE == TOKEN_ISEQUAL ||
        peek()->TYPE == TOKEN_ISNOTEQUAL)
    {
        type op = advance()->TYPE;
        Expression* right = parsePrimary();
        left = new BinaryExpr(left, op, right);
    }

    return left;
   
}

Expression* Parser::parseExpression()
{
 
	return parseOr();
}

Expression* Parser::parsePrimary()
{
    Token* t = advance();

    // Grouping: ( expr )
    if (t->TYPE == TOKEN_LEFT_PAREN) {
        Expression* inner = parseExpression();
        consume(TOKEN_RIGHT_PAREN, "Expected ')' after expression");
        return inner;
    }

    // Literals
    if (t->TYPE == TOKEN_INT)    return new IntLiteral(std::stoi(t->VALUE));
    if (t->TYPE == TOKEN_FLOAT)  return new FloatLiteral(std::stof(t->VALUE));
    if (t->TYPE == TOKEN_STRING) return new StringLiteral(t->VALUE);
    if (t->TYPE == TOKEN_TRUE)   return new IntLiteral(1);
    if (t->TYPE == TOKEN_FALSE)  return new IntLiteral(0);

    // Array literal: { ... }
    if (t->TYPE == TOKEN_LEFT_CURL_PAREN) {
        // IMPORTANT: parseArrayLiteral assumes '{' is already consumed
        return parseArrayLiteral();
    }

    // Identifier / Call / Array index
    if (t->TYPE == TOKEN_IDENTIFIER) {

        // Function / module call:  name( ... )
        if (peek()->TYPE == TOKEN_LEFT_PAREN) {
            advance(); // consume '('

            std::vector<Expression*> args;
            if (peek()->TYPE != TOKEN_RIGHT_PAREN) {
                while (true) {
                    args.push_back(parseExpression());
                    if (peek()->TYPE == TOKEN_COMMA) {
                        advance(); // eat the comma ','
                        continue;
                    }
                    break;
                }
            }

            consume(TOKEN_RIGHT_PAREN, "Expected ')' after function call");
            return new CallExpr(t->VALUE, std::move(args));
        }

        // Array indexing expression:  name[expr]
        if (peek()->TYPE == TOKEN_LEFT_BRACKET) {
            advance(); // eat the L Bracket '['
            Expression* index = parseExpression();
            consume(TOKEN_RIGHT_BRACKET, "Expected ']' after index");
            return new ArrayAccessExpr(t->VALUE, index); // you should already have this in AST ArrayLiteral
        }

        // Plain variable reference
        return new Identifier(t->VALUE);
    }

    throw std::runtime_error("Unexpected token in expression: '" + t->VALUE + "'");
}




ArrayLiteral* Parser::parseArrayLiteral()
{
    // We assume the '{' has already been consumed by parsePrimary()
    std::vector<Expression*> elements;

    // empty array: { }
    if (peek()->TYPE == TOKEN_RIGHT_CURL_PAREN) {
        advance(); // consume '}'
        return new ArrayLiteral(std::move(elements));
    }

    // { expr, expr, expr }
    do {
        elements.push_back(parseExpression());
    } while (match(TOKEN_COMMA));

    consume(TOKEN_RIGHT_CURL_PAREN, "Expected '}' after array literal");
    return new ArrayLiteral(std::move(elements));
}
	

// ------------------------------------------------------------ select case ------------------------------------------------
SelectStmt* Parser::parseSelect()
{
	consume(TOKEN_SELECT, "Expected 'select'");
	consume(TOKEN_LEFT_PAREN, "Expected '(' after select");
	Expression* expr = parseExpression(); // select expression ie; select(x) x = index
	consume(TOKEN_RIGHT_PAREN, "Expected ')' after select expression");
	consume(TOKEN_LEFT_CURL_PAREN, "Expected '{' after select(...)");
	std::vector<SelectStmt::CaseBranch> cases;
	std::vector<Statement*> defaultBody;
	while (peek()->TYPE != TOKEN_RIGHT_CURL_PAREN && peek()->TYPE != TOKEN_EOF) {
		if (peek()->TYPE == TOKEN_CASE) {
			advance(); // consume 'case'
			Expression* caseExpr = parseExpression(); // case expression ie; case 1: 1 = index
			consume(TOKEN_COLON, "Expected ':' after case expression");
			std::vector<Statement*> caseBody;
			while (peek()->TYPE != TOKEN_CASE &&
				peek()->TYPE != TOKEN_DEFAULT &&
				peek()->TYPE != TOKEN_RIGHT_CURL_PAREN &&
				peek()->TYPE != TOKEN_EOF) {
				Statement* s = parseStatement();
				if (s) caseBody.push_back(s);
			}
			cases.push_back(SelectStmt::CaseBranch{ caseExpr, std::move(caseBody) });
		}
		else if (peek()->TYPE == TOKEN_DEFAULT) {
			advance(); // consume 'default'
			consume(TOKEN_COLON, "Expected ':' after default");
			while (peek()->TYPE != TOKEN_RIGHT_CURL_PAREN && peek()->TYPE != TOKEN_EOF) {
				Statement* s = parseStatement();
				if (s) defaultBody.push_back(s);
			}
		}
		else {
			throw std::runtime_error("Expected 'case' or 'default' in select statement");
		}
	}
	consume(TOKEN_RIGHT_CURL_PAREN, "Expected '}' after select body");
	match(TOKEN_SEMICOLON); // optional semicolon after select block
	return new SelectStmt(expr, std::move(cases), std::move(defaultBody));
}


// ------------------------------------------------------------ IF ELSEIF ELSE ------------------------------------------------

IfItsStmt* Parser::parseIfIts() {
        consume(TOKEN_IFITS, "Expected 'IfIts'");
        consume(TOKEN_LEFT_PAREN, "Expected '(' after IfIts");
        Expression* cond = parseExpression();
        consume(TOKEN_RIGHT_PAREN, "Expected ')' after IfIts condition");

        // BODY for the first IfIts branch: runs until ElseIts / Else / EndIfIts
        std::vector<Statement*> thenBody;
        while (peek()->TYPE != TOKEN_ELSEITS &&
            peek()->TYPE != TOKEN_ELSE &&
            peek()->TYPE != TOKEN_ENDIFITS &&
            peek()->TYPE != TOKEN_EOF)
        {
            Statement* s = parseStatement();
            if (s) thenBody.push_back(s);
        }

        // Parse zero or more ElseIts branches
        std::vector<IfItsStmt::ElseIfBranch> elseIfs;
        while (peek()->TYPE == TOKEN_ELSEITS) {
            advance(); // consume 'ElseIts'
            consume(TOKEN_LEFT_PAREN, "Expected '(' after ElseIts");
            Expression* econd = parseExpression();
            consume(TOKEN_RIGHT_PAREN, "Expected ')' after ElseIts condition");

            std::vector<Statement*> ebody;
            while (peek()->TYPE != TOKEN_ELSEITS &&
                peek()->TYPE != TOKEN_ELSE &&
                peek()->TYPE != TOKEN_ENDIFITS &&
                peek()->TYPE != TOKEN_EOF)
            {
                Statement* s = parseStatement();
                if (s) ebody.push_back(s);
            }

            elseIfs.push_back(IfItsStmt::ElseIfBranch{ econd, std::move(ebody) });
        }

        // Optional Else
        std::vector<Statement*> elseBody;
        if (peek()->TYPE == TOKEN_ELSE) {
            advance(); // consume 'Else'

            while (peek()->TYPE != TOKEN_ENDIFITS &&
                peek()->TYPE != TOKEN_EOF)
            {
                Statement* s = parseStatement();
                if (s) elseBody.push_back(s);
            }
        }

        // EndIfIts;
        consume(TOKEN_ENDIFITS, "Expected 'EndIfIts' to close IfIts");
        consume(TOKEN_SEMICOLON, "Expected ';' after EndIfIts");

        return new IfItsStmt(cond, std::move(thenBody), std::move(elseIfs), std::move(elseBody));
}



Token* Parser::peek()
{
	if (pos >= tokens.size()) return tokens.back(); // assume EOF
	return tokens[pos];
}

Token* Parser::peekNext()
{  
        if (pos + 1 >= tokens.size()) return tokens.back();
        return tokens[pos + 1]; 
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
// lets us use match in error handling with ; or without ;
bool Parser::match(type t)
{
	if (peek()->TYPE == t) {
		advance();
		return true;
	}
	return false;
}
