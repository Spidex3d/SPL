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
    // ----------- IfIts / ElseIts / Else / EndIfIts -----------
    if (peek()->TYPE == TOKEN_IFITS) {
        return parseIfIts();
    }

    if (peek()->TYPE == TOKEN_RETURN) { advance();
	Expression* v = parseExpression();
    consume(TOKEN_SEMICOLON, "Expected ';' after return");
		return new ReturnStmt(v);
	}

    // Variable declaration
    if (peek()->TYPE == TOKEN_DEC_I ||
        peek()->TYPE == TOKEN_DEC_S ||
        peek()->TYPE == TOKEN_DEC_F ||
        peek()->TYPE == TOKEN_DEC_B) {

        type declType = advance()->TYPE; // consume dec_i/dec_s/dec_f
        Token* id = consume(TOKEN_IDENTIFIER, "Expected variable name");

        Expression* expr = nullptr;
        if (match(TOKEN_EQUALS)) {
            expr = parseExpression();
        }

        consume(TOKEN_SEMICOLON, "Expected ';' after declaration");
        return new Declaration(id->VALUE, declType, expr);
    }
    

    // ------------------------------------------------------ While loop ------------------------------------------------
    if (peek()->TYPE == TOKEN_WHILE) {
        advance(); // consume 'while'

        consume(TOKEN_LEFT_PAREN, "Expected '(' after while");

        // condition is an expression like:  i isless 10
        Expression* cond = parseExpression();

        consume(TOKEN_RIGHT_PAREN, "Expected ')' after while condition");
        consume(TOKEN_LEFT_CURL_PAREN, "Expected '{' after while(...)");

        std::vector<Statement*> body;
        while (peek()->TYPE != TOKEN_RIGHT_CURL_PAREN) {
            Statement* s = parseStatement();
            if (s) body.push_back(s);
        }

        consume(TOKEN_RIGHT_CURL_PAREN, "Expected '}' after while body");

        // Optional semicolon after the block
        match(TOKEN_SEMICOLON);

        return new WhileStmt(cond, std::move(body));
    }

	// ------------------------------------------------- DO loop with three conditions ---------------------------------------
	if (peek()->TYPE == TOKEN_DO) {
		advance(); // consume 'do'
		consume(TOKEN_LEFT_PAREN, "Expected '(' after do");
        
        Statement* init = nullptr;
        {
            // init can be: j; or j = 0; or dec_i j = 0;
			init = parseStatement();
        }

		Expression* cond = parseExpression();
		consume(TOKEN_SEMICOLON, "Expected ';' in do loop after condition");
		
			// step can be: j++; or j = j + 1;
		//Statement*	step = parseStatement();
		Statement* step = nullptr;
		//{
			// step can be: j++; or j = j + 1;
			//step = parseStatement();
		//}
        if (peek()->TYPE == TOKEN_IDENTIFIER &&
            pos + 1 < tokens.size() &&
            tokens[pos + 1]->TYPE == TOKEN_INCREMENT)
        {
            Token* idTok = advance();   // consume identifier
            advance();                  // consume '++'

            // Build: j = j + 1;
            Expression* one = new IntLiteral(1);
            Expression* leftId = new Identifier(idTok->VALUE);
            Expression* plus = new BinaryExpr(leftId, TOKEN_PLUS, one);

            step = new Assignment(idTok->VALUE, plus);
        }
        // Case 2: j = j + 1  (normal assignment step)
        else if (peek()->TYPE == TOKEN_IDENTIFIER &&
            pos + 1 < tokens.size() &&
            tokens[pos + 1]->TYPE == TOKEN_EQUALS)
        {
            Token* id = advance(); // ident
            consume(TOKEN_EQUALS, "Expected '=' in do-loop step");
            Expression* expr = parseExpression();
            step = new Assignment(id->VALUE, expr);
        }
        else {
            throw std::runtime_error("Expected step statement (e.g. j++ or j = j + 1) in do-loop");
        }


		consume(TOKEN_RIGHT_PAREN, "Expected ')' after do condition");
		
        consume(TOKEN_LEFT_CURL_PAREN, "Expected '{' after do (...)");

		
        std::vector<Statement*> body;
		while (peek()->TYPE != TOKEN_RIGHT_CURL_PAREN) {
			Statement* s = parseStatement();
			if (s) body.push_back(s);
		}
		consume(TOKEN_RIGHT_CURL_PAREN, "Expected '}' after do body");
				
        // Optional semicolon after the block
        match(TOKEN_SEMICOLON);

		return new DoStmt(init, cond, step, std::move(body));
	} 

	

	// ------------------------------------------------------------ i++  =>  i = i + 1; While loop ------------------------------------------------  
   if (peek()->TYPE == TOKEN_IDENTIFIER &&
       pos + 1 < tokens.size() &&
       tokens[pos + 1]->TYPE == TOKEN_INCREMENT)
   {
       Token* idTok = advance();                    // consume identifier
       advance();                                   // consume '++'
       consume(TOKEN_SEMICOLON, "Expected ';' after ++");

       // build: i = i + 1;
       Expression* one = new IntLiteral(1);
       Expression* leftId = new Identifier(idTok->VALUE);
       Expression* plus = new BinaryExpr(leftId, TOKEN_PLUS, one);

       return new Assignment(idTok->VALUE, plus);
   }

	// -------------------------------------------------------- i--  =>  i = i - 1; While loop ------------------------------------------------
    if (peek()->TYPE == TOKEN_IDENTIFIER &&
        pos + 1 < tokens.size() &&
        tokens[pos + 1]->TYPE == TOKEN_DECREASE)
    {
        Token* idTok = advance();                    // consume identifier
        advance();                                   // consume '++'
        consume(TOKEN_SEMICOLON, "Expected ';' after --");

        // build: i = i - 1;
        Expression* one = new IntLiteral(1);
        Expression* leftId = new Identifier(idTok->VALUE);
        Expression* minus = new BinaryExpr(leftId, TOKEN_MINUS, one);

        return new Assignment(idTok->VALUE, minus);
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

    if (t->TYPE == TOKEN_INT)  return new IntLiteral(std::stoi(t->VALUE));
    
    if (t->TYPE == TOKEN_STRING) return new StringLiteral(t->VALUE);

    if (t->TYPE == TOKEN_TRUE) return new IntLiteral(1);
    
    if (t->TYPE == TOKEN_FALSE) return new IntLiteral(0);
    
    
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

bool Parser::match(type t)
{
	if (peek()->TYPE == t) {
		advance();
		return true;
	}
	return false;
}
