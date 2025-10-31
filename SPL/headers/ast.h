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

struct IntLiteral : Expression {
    int value;
    explicit IntLiteral(int val) : value(val) {}
};

struct StringLiteral : Expression {
    std::string value;
    explicit StringLiteral(const std::string& val) : value(val) {}
};

struct Identifier : Expression {
    std::string name;
    explicit Identifier(const std::string& n) : name(n) {}
};

struct BinaryExpr : Expression {
    Expression* left;
    type op;        // TOKEN_PLUS, TOKEN_MINUS, etc.
    Expression* right;
    BinaryExpr(Expression* l, type o, Expression* r)
        : left(l), op(o), right(r) {
    }
};

// ----- Statements -----
struct Statement : ASTNode {};

struct Declaration : Statement {
    std::string name;
    type varType;
    Expression* value; // optional initializer
    Declaration(const std::string& n, type t, Expression* v = nullptr)
        : name(n), varType(t), value(v) {
    }
};

struct Assignment : Statement {
    std::string name;
    Expression* value;
    Assignment(const std::string& n, Expression* val)
        : name(n), value(val) {
    }
};

struct Pout : Statement {
    Expression* value;
    explicit Pout(Expression* v) : value(v) {}
};

