#include <stdexcept>
#include <string>
#include <vector>
#include "../include/interpreter.h"
#include <iostream>

Interpreter::Interpreter()
{

}

Var* Interpreter::lookupVar(const std::string& name)
{
	for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
		auto f = it->find(name);
		if (f != it->end()) return &f->second;
	}
	auto g = variables.find(name);
	if (g != variables.end()) return &g->second;
	return nullptr;
}

// new
void Interpreter::registerModules(const std::vector<Module*>& mods)
{
	moduleTable.clear();
	for (auto* m : mods) moduleTable[m->name] = m;
}
// new
Value Interpreter::callModule(const std::string& name, const std::vector<Value>& args)
{
	auto it = moduleTable.find(name);
	if (it == moduleTable.end()) throw std::runtime_error("Unknown module: " + name);
	Module* m = it->second;
	if (args.size() != m->params.size())
		throw std::runtime_error("Arity mismatch in call to " + name);

	// push new local scope
	scopes.emplace_back();
	auto& locals = scopes.back();

	// bind params as locals (infer type from Value)
	for (size_t i = 0; i < args.size(); ++i) {
		Var v{};
		if (args[i].kind == Value::VInt) { v.varType = TOKEN_DEC_I; v.intValue = args[i].i; }
		else if (args[i].kind == Value::VString) { v.varType = TOKEN_DEC_S; v.stringValue = args[i].s; }
		//else if (args[i].kind == Value::VInt) { v.varType = TOKEN_DEC_B; v.intValue = args[i].i ? 1 : 0; }
		//else if (args[i].kind == Value::VBool) { v.varType = TOKEN_DEC_B; v.intValue = args[i].i ? true : false; }
		locals[m->params[i]] = v;
	}

	// execute body
	returning = false;
	retVal = Value::Void();
	for (auto* s : m->body) {
		if (!s) continue;
		execute(s);
		if (returning) break;
	}

	// pop scope
	scopes.pop_back();
	Value out = retVal;
	returning = false;
	retVal = Value::Void();
	return out;
}

void Interpreter::runModule(Module* mod)
{
	for (auto s : mod->body) {
		if (!s) continue; // Ignore null statements (e.g., comments)
		if (!s) continue;
		execute(s);
		if (returning) { returning = false; retVal = Value::Void(); }
	}

}

void Interpreter::runProgramEntry(std::vector<Module*>& mods, const std::string& entryName)
{
	registerModules(mods);
	(void)callModule(entryName, {}); // ignore return
	
}


void Interpreter::execute(Statement* stmt) {
	if (!stmt) return;

	if (auto* decl = dynamic_cast<Declaration*>(stmt)) {
		Var v{};
		v.varType = decl->varType;
		if (decl->value) {
			if (v.varType == TOKEN_DEC_I) v.intValue = evalInt(decl->value);
			else if (v.varType == TOKEN_DEC_S) v.stringValue = evalString(decl->value);
			else if (v.varType == TOKEN_DEC_F) v.floatValue = static_cast<float>(evalInt(decl->value));
			//else if (v.varType == TOKEN_DEC_B) v.intValue = evalInt(decl->value); // boolean as int
			else if (v.varType == TOKEN_DEC_B) v.intValue = evalBool(decl->value) ? 1 : 0; // boolean as int
		}
		if (scopes.empty()) variables[decl->name] = v; else scopes.back()[decl->name] = v;
		return;
	}

	if (auto* asn = dynamic_cast<Assignment*>(stmt)) {
		Var* var = lookupVar(asn->name);
		if (!var) throw std::runtime_error("Undefined variable: " + asn->name);
		if (var->varType == TOKEN_DEC_I) var->intValue = evalInt(asn->value);
		else if (var->varType == TOKEN_DEC_S) var->stringValue = evalString(asn->value);
		else if (var->varType == TOKEN_DEC_F) var->floatValue = static_cast<float>(evalInt(asn->value));
		//else if (var->varType == TOKEN_DEC_B) var->intValue = evalInt(asn->value); // boolean as int
		else if (var->varType == TOKEN_DEC_B) var->intValue = evalBool(asn->value) ? 1 : 0; // boolean as int
		return;
	}

	// ----------------------------------- ifits statement -----------------------------------
	if (auto* ifs = dynamic_cast<IfItsStmt*>(stmt)) {
		// 1) main IfIts
		//if (evalInt(ifs->condition) != 0) {
		if (evalBool(ifs->condition) != 0) {
			for (auto* s : ifs->thenBody) {
				if (!s) continue;
				execute(s);
				if (returning) return; // respect 'return'
			}
			return;
		}

		// 2) ElseIts branches
		for (auto& br : ifs->elseIfBranches) {
			//if (evalInt(br.cond) != 0) {
			if (evalBool(br.cond) != 0) {
				for (auto* s : br.body) {
					if (!s) continue;
					execute(s);
					if (returning) return;
				}
				return;
			}
		}

		// 3) Else
		for (auto* s : ifs->elseBody) {
			if (!s) continue;
			execute(s);
			if (returning) return;
		}

		return;
	}


	// While loop
	if (auto* ws = dynamic_cast<WhileStmt*>(stmt)) {
		//while (evalInt(ws->condition) != 0) {
		while (evalBool(ws->condition) != 0) {

			for (auto* s : ws->body) {
				if (!s) continue;
				execute(s);

				if (returning) return; // bubble return out
			}
		}
		return;
	}

	// Do loop
	if (auto* dl = dynamic_cast<DoStmt*>(stmt)) {

		// run init
		if (dl->init)
			execute(dl->init);

		// run loop
		//while (evalInt(dl->condition) != 0) {
		while (evalBool(dl->condition) != 0) {

			for (auto* s : dl->body) {
				if (!s) continue;
				execute(s);

				if (returning) return;
			}

			// run step
			if (dl->step)
				execute(dl->step);
		}
		return;
	}


	// Pout statement for printing to console
	if (auto* pout = dynamic_cast<Pout*>(stmt)) {
		Expression* e = pout->value;
		if (isStringExpr(e)) {
			std::cout << evalString(e) << std::endl;
		}
		else if (isBoolExpr(e)) {
			std::cout << (evalBool(e) ? "true" : "false") << std::endl;
		}
		else {
			std::cout << evalInt(e) << std::endl;
		}
		return;
	}

	if (auto* ret = dynamic_cast<ReturnStmt*>(stmt)) {
		Expression* v = ret->value;
		//retVal = isStringExpr(v) ? Value::Str(evalString(v)) : Value::Int(evalInt(v));
		if (isStringExpr(v))      retVal = Value::Str(evalString(v));
		else if (isBoolExpr(v))   retVal = Value::Bool(evalBool(v));
		else                      retVal = Value::Int(evalInt(v));

		returning = true;
		return;
	}

	if (auto* es = dynamic_cast<ExprStmt*>(stmt)) {
		// Evaluate for side effects (module calls). Discard result.
		//if (isStringExpr(es->expr)) (void)evalString(es->expr);
		//else                        (void)evalInt(es->expr);
		//return;
		if (auto* call = dynamic_cast<CallExpr*>(es->expr)) {
			std::vector<Value> args;
			args.reserve(call->args.size());
			for (auto* a : call->args) {
				if (isStringExpr(a))       args.push_back(Value::Str(evalString(a)));
				else if (isBoolExpr(a))    args.push_back(Value::Bool(evalBool(a)));
				else                       args.push_back(Value::Int(evalInt(a)));
			}

			// Ignore return value (could be int/string/bool/void)
			(void)callModule(call->moduleName, args);
		}
		else {
			// For other expressions, just evaluate for side effects
			if (isStringExpr(es->expr)) (void)evalString(es->expr);
			else                        (void)evalInt(es->expr);
		}
		return;
	}

}


int Interpreter::evalInt(Expression* expr) {
	if (auto* lit = dynamic_cast<IntLiteral*>(expr)) {
		return lit->value;
	}
	if (auto* id = dynamic_cast<Identifier*>(expr)) {
		Var* v = lookupVar(id->name);
		if (!v) throw std::runtime_error("Undefined variable: " + id->name);

		if (v->varType == TOKEN_DEC_I || v->varType == TOKEN_DEC_B) {
			return v->intValue;
		}
		throw std::runtime_error("Type error: '" + id->name + "' is NOT int");
	}
	
	if (auto* bin = dynamic_cast<BinaryExpr*>(expr)) {
		// If either side is string and op is PLUS, this is string concat → not an int
		if (bin->op == TOKEN_PLUS && (isStringExpr(bin->left) || isStringExpr(bin->right))) {
			throw std::runtime_error("Type error: '+' on strings used in int context");
		}
		int l = evalInt(bin->left);
		int r = evalInt(bin->right);
		switch (bin->op) {
		case TOKEN_PLUS:		return l + r;
		case TOKEN_MINUS:		return l - r;
		case TOKEN_STAR:		return l * r;
		case TOKEN_SLASH:		return r == 0 ? 0 : l / r;	// simple guard
		case TOKEN_ISLESS:      return l < r ? 1 : 0;		// left is less than right true = 1, false = 0
		case TOKEN_ISMORE:      return l > r ? 1 : 0;		// left is more than right true = 1, false = 0
		case TOKEN_ISEQUAL:     return l == r ? 1 : 0;		// left is equal to right true = 1, false = 0
		case TOKEN_ISNOTEQUAL:  return l != r ? 1 : 0;		// left is not equal to right true = 1, false = 0
		default: break;
		}
	}
	if (auto* call = dynamic_cast<CallExpr*>(expr)) {
		std::vector<Value> args;
		args.reserve(call->args.size());
		for (auto* a : call->args) {
			if (isStringExpr(a))	args.push_back(Value::Str(evalString(a)));
			//else if (isBoolExpr(a)) args.push_back(Value::Bool(evalBool(a)));
			else					args.push_back(Value::Int(evalInt(a)));
		}
		Value v = callModule(call->moduleName, args);

		
		if (v.kind != Value::VInt)	throw std::runtime_error("Type error: expected int return from '" + call->moduleName + "'");
		return v.i;
		
	}
	 //String in an int position:
	if (dynamic_cast<StringLiteral*>(expr)) {
		throw std::runtime_error("Type error: string where int expected");
	}
	throw std::runtime_error("Invalid int expression");
}

// Evaluate expressions floats
float Interpreter::evalFloat(Expression* expr) {
	// For now treat ints as floats
	if (auto* lit = dynamic_cast<IntLiteral*>(expr)) return (float)lit->value;
	if (auto* id = dynamic_cast<Identifier*>(expr)) return variables[id->name].floatValue;
	throw std::runtime_error("Invalid float expression");
}

bool Interpreter::evalBool(Expression* expr)
{
	
	// --- 0) Logical NOT: !expr ---
	if (auto* u = dynamic_cast<UnaryExpr*>(expr)) {
		if (u->op == TOKEN_NOT) {
			return !evalBool(u->expr);
		}
		// if you ever add other unary ops, handle them here
	}

	// --- 1) Binary logical / comparison / arithmetic ---
	if (auto* bin = dynamic_cast<BinaryExpr*>(expr)) {
		switch (bin->op) {
			// logical AND: left && right  (short-circuit)
		case TOKEN_AND:
			return evalBool(bin->left) && evalBool(bin->right);

			// logical OR: left || right   (short-circuit)
		case TOKEN_OR:
			return evalBool(bin->left) || evalBool(bin->right);

			// comparison operators: already implemented in evalInt as 0/1
		case TOKEN_ISEQUAL:
		case TOKEN_ISNOTEQUAL:
		case TOKEN_ISLESS:
		case TOKEN_ISMORE:
		
			// later: add <= >= here too if you define them
			return evalInt(expr) != 0;

			// fallback: arithmetic / other ops used in boolean context
		default:
			// e.g. IfIts (x + 1) → treat non-zero as true
			return evalInt(expr) != 0;
		}
	}

	// --- 2) Literal ints: 0 = false, non-zero = true ---
	if (auto* lit = dynamic_cast<IntLiteral*>(expr)) {
		return lit->value != 0;
	}

	// --- 3) Identifier: dec_b or dec_i variable ---
	if (auto* id = dynamic_cast<Identifier*>(expr)) {
		Var* v = lookupVar(id->name);
		if (!v)
			throw std::runtime_error("Undefined variable: " + id->name);

		if (v->varType == TOKEN_DEC_B || v->varType == TOKEN_DEC_I) {
			return v->intValue != 0;
		}

		throw std::runtime_error("Type error: '" + id->name + "' is not a boolean-compatible type");
	}

	// --- 4) Call expression: boolean from a module call ---
	if (auto* call = dynamic_cast<CallExpr*>(expr)) {
		std::vector<Value> args;
		args.reserve(call->args.size());
		for (auto* a : call->args) {
			if (isStringExpr(a)) args.push_back(Value::Str(evalString(a)));
			else                 args.push_back(Value::Int(evalInt(a)));
		}

		Value v = callModule(call->moduleName, args);

		// For now we treat ints as booleans (0 = false, non-zero = true)
		if (v.kind == Value::VInt)    return v.i != 0;
		if (v.kind == Value::VString)
			throw std::runtime_error("Type error: string return used as boolean from '" + call->moduleName + "'");

		// If you later add VBool, you can do:
		// if (v.kind == Value::VBool) return v.b;

		throw std::runtime_error("Type error: non-boolean compatible return from '" + call->moduleName + "'");
	}

	// --- 5) String literal: not allowed in boolean context (for now) ---
	if (dynamic_cast<StringLiteral*>(expr)) {
		throw std::runtime_error("Type error: string used in boolean context");
	}

	// --- 6) Anything else is invalid ---
	throw std::runtime_error("Invalid boolean expression");

	
}

bool Interpreter::isBoolExpr(Expression* expr)
{
	if (auto* id = dynamic_cast<Identifier*>(expr)) {
		Var* v = lookupVar(id->name);
		if (!v) return false;
		return v->varType == TOKEN_DEC_B;
	}

	if (auto* un = dynamic_cast<UnaryExpr*>(expr)) {
		if (un->op == TOKEN_NOT) return true;
	}

	if (auto* bin = dynamic_cast<BinaryExpr*>(expr)) {
		switch (bin->op) {
		case TOKEN_ISEQUAL:
		case TOKEN_ISNOTEQUAL:
		case TOKEN_ISLESS:
		case TOKEN_ISMORE:
		case TOKEN_AND:      // <-- add
		case TOKEN_OR:       // <-- add
			return true;
		default:
			break;
		}
	}

	// (optional) Unary NOT could be considered boolean too:
	if (auto* u = dynamic_cast<UnaryExpr*>(expr)) {
		if (u->op == TOKEN_NOT) return true;
	}

	return false;

	
}


std::string Interpreter::evalString(Expression* expr) {
	if (auto* s = dynamic_cast<StringLiteral*>(expr)) {
		return s->value;
	}
	if (auto* id = dynamic_cast<Identifier*>(expr)) {
		Var* v = lookupVar(id->name);
		if (!v)	throw std::runtime_error("Undefined variable: " + id->name);
		if (v->varType == TOKEN_DEC_S) return v->stringValue;
		if (v->varType == TOKEN_DEC_B) return v->intValue ? "true" : "false"; // boolean as string
		
		if (v->varType == TOKEN_DEC_I) return std::to_string(v->intValue);
		// simple fallback for future float
		if (v->varType == TOKEN_DEC_F) return std::to_string(v->floatValue);
		return "";
	}
	if (auto* lit = dynamic_cast<IntLiteral*>(expr)) {
		return std::to_string(lit->value);
	}
	if (auto* bin = dynamic_cast<BinaryExpr*>(expr)) {
		if (bin->op == TOKEN_PLUS) {
			// Concatenate with coercion to string on both sides
			return evalString(bin->left) + evalString(bin->right);
		}
		// Other operators on strings are not supported
		throw std::runtime_error("Type error: invalid operator on string");
	}

	if (auto* call = dynamic_cast<CallExpr*>(expr)) {
		std::vector<Value> args;
		for (auto* a : call->args) {
			if (isStringExpr(a)) args.push_back(Value::Str(evalString(a)));
			else if (isBoolExpr(a))  args.push_back(Value::Bool(evalBool(a)));
			else                 args.push_back(Value::Int(evalInt(a)));
		}
		Value v = callModule(call->moduleName, args);
		if (v.kind == Value::VString) return v.s;
		if (v.kind == Value::VInt)    return std::to_string(v.i);
		if (v.kind == Value::VBool)   return v.b ? "true" : "false";
		return "";
	}
	throw std::runtime_error("Invalid string expression");
	
}
// Determine if an expression evaluates to a string --- string concatenation ---
bool Interpreter::isStringExpr(Expression* expr)
{
	if (dynamic_cast<StringLiteral*>(expr)) return true;

	if (auto* id = dynamic_cast<Identifier*>(expr)) {
		if (Var* v = lookupVar(id->name)) return v->varType == TOKEN_DEC_S;
		
		return false;
		
	}

	if (auto* bin = dynamic_cast<BinaryExpr*>(expr)) {
		// String if either side is string and op is PLUS (concat)
		if (bin->op == TOKEN_PLUS) {
			return isStringExpr(bin->left) || isStringExpr(bin->right);
		}
		return false;
	}
	if (auto* call = dynamic_cast<CallExpr*>(expr)) {
		// Assume module calls return int for now
		//return false;
		return true;
	}

	// IntLiteral or anything else defaults to non-string
	return false;
}


