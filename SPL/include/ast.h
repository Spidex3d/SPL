#pragma once

#include "tokens.h"
#include <string>

// ========================
// Base AST Structures
// ========================
struct ASTNode {
    virtual ~ASTNode() = default;
};

// ----- Expressions -----
struct Expression : ASTNode {};
// ----- Expressions ----- INT
struct IntLiteral : Expression {
    int value;
    explicit IntLiteral(int val) : value(val) {}
};
// ----- Expressions ----- STRING
struct StringLiteral : Expression {
    std::string value;
    explicit StringLiteral(const std::string& val) : value(val) {}
};
// ----- Expressions ----- IDENTIFIER
struct Identifier : Expression {
    std::string name;
    explicit Identifier(const std::string& n) : name(n) {}
};
// ----- Expressions ----- ADD left and right 5 + 5 = 10
struct BinaryExpr : Expression {
    Expression* left;
    type op;        // TOKEN_PLUS, TOKEN_MINUS, etc.
    Expression* right;
    BinaryExpr(Expression* l, type o, Expression* r)
        : left(l), op(o), right(r) {
    }
};
// TOKEN_NOT !
struct UnaryExpr : Expression {
	type op;          
    Expression* expr;

    UnaryExpr(type o, Expression* e)
        : op(o), expr(e) {
    }
};

// ----- Statements -----
struct Statement : ASTNode {};

// ----- Statements ----- MODULE module_name { body }
struct Module : ASTNode {
    std::string name;
	std::vector<std::string> params;
    std::vector<Statement*> body;
    Module(const std::string& n, std::vector<std::string> p, std::vector<Statement*> b)
        : name(n), params(std::move(p)), body(std::move(b)) {
    }
};

struct CallExpr : Expression {
	std::string moduleName;
	std::vector<Expression*> args;
	CallExpr(const std::string& name, std::vector<Expression*> a)
		: moduleName(name), args(std::move(a)) {
	}
};

struct IncrementExpr : Expression {
    std::string name;
    IncrementExpr(const std::string& n) : name(n) {}
};

struct ReturnStmt : Statement {
	Expression* value;
	explicit ReturnStmt(Expression* v) : value(v) {}
};
struct ExprStmt : Statement {
	Expression* expr;
	explicit ExprStmt(Expression* e) : expr(e) {}
};


// ----- Statements ----- DECLARATION type name = value;
struct Declaration : Statement {
    std::string name;
    type varType;
    Expression* value; // optional initializer
    Declaration(const std::string& n, type t, Expression* v = nullptr)
        : name(n), varType(t), value(v) {
    }
};
// ----- Statements ----- ASSIGNMENT name = value;
struct Assignment : Statement {
    std::string name;
    Expression* value;
    Assignment(const std::string& n, Expression* val)
        : name(n), value(val) {
    }
};
// ----- Statements ----- IF condition { body } used in the while loop
struct WhileStmt : Statement {
	Expression* condition;
	std::vector<Statement*> body;
	WhileStmt(Expression* cond, std::vector<Statement*> b)
		: condition(cond), body(std::move(b)) {
	}
};

// do loop with three conditions can be added later
struct DoStmt : Statement {
    Statement* init;
	Expression* condition;
    Statement* step;
	std::vector<Statement*> body;
	DoStmt(Statement* i, Expression* c, Statement* s, std::vector<Statement*> b)
		: init(i), condition(c), step(s), body(std::move(b)) {
	}
};

struct IfItsStmt : Statement {
    Expression* condition;
    std::vector<Statement*> thenBody;

    struct ElseIfBranch {
        Expression* cond;
		std::vector<Statement*> body;
    };
	std::vector<ElseIfBranch> elseIfBranches;

	std::vector<Statement*> elseBody;

    IfItsStmt(Expression* c,
        std::vector<Statement*> thenB,
        std::vector<ElseIfBranch> elseIfs,
        std::vector<Statement*> elseB)
        : condition(c),
        thenBody(std::move(thenB)),
        elseIfBranches(std::move(elseIfs)),
        elseBody(std::move(elseB)) {
    }
    
};




// ----- Statements ----- POUT pout value;
struct Pout : Statement {
    Expression* value;
    explicit Pout(Expression* v) : value(v) {}
};

