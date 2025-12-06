#pragma once
#include "parsing.h"

struct Var {
	type varType = TOKEN_DEC_I; // TOKEN_DEC_I, TOKEN_DEC_S, TOKEN_DEC_F
	//type varType; // TOKEN_DEC_I, TOKEN_DEC_S, TOKEN_DEC_F
	std::string stringValue; // for string variables
	int intValue = 0;	   // for int variables
	float floatValue = 0.0f;	 // for float variables
	//bool boolValue = false;   // for boolean variables
};

struct Value {
	//enum { VVoid, VInt, VString, VBool } kind = VVoid;
	enum { VVoid, VInt, VString, } kind = VVoid;
	int i = 0;
	std::string s;
	static Value Int(int v) { Value x; x.kind = VInt; x.i = v; return x; }
	static Value Str(std::string v) { Value x; x.kind = VString; x.s = std::move(v); return x; }
	//static Value Bool(bool v) { Value x; x.kind = VBool; x.i = v ? 1 : 0; return x; }
	static Value Void() { return Value{}; }
};


class Interpreter {

public:
	Interpreter(); // constructor
	

	void execute(Statement* stmt);
	void runModule(Module* mod);
	// register multiple modules
	

	// run the program starting from the specified entry module main()
	void runProgramEntry(std::vector<Module*>& mods, const std::string& entryName = "main");

private:

	// store variables in a map
	std::unordered_map<std::string, Var> variables;
	// store modules in a map + call stack
	std::unordered_map<std::string, Module*> moduleTable;
	std::vector<std::unordered_map<std::string, Var>> scopes; // for local scopes

	// return handling
	bool returning = false;
	Value retVal = Value::Void();

	// helpers
	void registerModules(const std::vector<Module*>& mods);
	Value callModule(const std::string& name, const std::vector<Value>& args);
	//  serch local and global scopes for variables
	Var* lookupVar(const std::string& name);

	// evaluate an Int expression from the AST
	int evalInt(Expression* expr);
	// evaluate a String expression from the AST
	std::string evalString(Expression* expr);
	// evaluate a Float expression from the AST
	float evalFloat(Expression* expr);
	// evaluate a boolean expression from the AST
	bool evalBool(Expression* expr);
	bool isBoolExpr(Expression* expr);
	// determine if an expression evaluates to a string
	bool isStringExpr(Expression* expr);
	
};