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

// ------------------ Array Access Expression: array_name[index] ------------------
struct ArrayLiteral : Expression {
    std::vector<Expression*> elements;
    ArrayLiteral(std::vector<Expression*> elems)
        : elements(std::move(elems)) {
    }
};
// ----- Expressions ----- ARRAY ACCESS array_name[index]
struct ArrayAccessExpr : Expression {
    std::string arrayName;
    Expression* index;

    ArrayAccessExpr(const std::string& name, Expression* idx)
        : arrayName(name), index(idx) {
    }
};

//struct ArrayAccess : Expression {
//    std::string name;
//    Expression* index;
//    ArrayAccess(const std::string& n, Expression* i)
//        : name(n), index(i) {
//    }
//};

struct ReturnStmt : Statement {
	Expression* value;
	explicit ReturnStmt(Expression* v) : value(v) {}
};
struct ExprStmt : Statement {
	Expression* expr;
	explicit ExprStmt(Expression* e) : expr(e) {}
};


// ----- Statements --- ARRAY -- DECLARATION type name = value;
struct ArrayDeclaration : Statement {
    type elementType;              // TOKEN_DEC_I / S / F / B
    std::string name;
    int size;                       // -1 = unsized
    std::vector<Expression*> values;

    ArrayDeclaration(type t,
        const std::string& n,
        int sz,
        std::vector<Expression*> vals)
        : elementType(t), name(n), size(sz), values(std::move(vals)) {
    }
};


struct Declaration : Statement {
    std::string name;
    type varType;                 // TOKEN_DEC_I / TOKEN_DEC_F / TOKEN_DEC_S / TOKEN_DEC_B
    Expression* value = nullptr;  // scalar initializer (optional)

    // ---- array support ----
    bool isArray = false;
    int arraySize = -1;                           // -1 means "unspecified"
    std::vector<Expression*> arrayInit;           // { ... } initializers

    // scalar
    Declaration(const std::string& n, type t, Expression* v = nullptr)
        : name(n), varType(t), value(v) {
    }

	// --------- array support ---------
    Declaration(const std::string& n, type t, bool arr, int size, std::vector<Expression*> init)
        : name(n), varType(t), value(nullptr),
        isArray(arr), arraySize(size), arrayInit(std::move(init)) {
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
// ----- Statements ----- ARRAY ASSIGNMENT array_name[index] = value;
struct ArrayAssignStmt : Statement {
	std::string name;
	Expression* index;
	Expression* value;
	ArrayAssignStmt(const std::string& n, Expression* i, Expression* v)
		: name(n), index(i), value(v) {
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

// ----- statements ----- select case (switch case) can be added later
struct SelectStmt : Statement {
	Expression* expression;
	struct CaseBranch {
		Expression* caseExpr;
		std::vector<Statement*> body;
	};
	std::vector<CaseBranch> cases;
	std::vector<Statement*> defaultBody;
	SelectStmt(Expression* expr,
		std::vector<CaseBranch> cs,
		std::vector<Statement*> defB)
		: expression(expr),
		cases(std::move(cs)),
		defaultBody(std::move(defB)) {
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

