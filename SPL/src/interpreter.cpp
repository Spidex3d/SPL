#include <stdexcept>
#include <string>
#include <vector>
#include "../include/interpreter.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

Interpreter::Interpreter()
{

}
Var* Interpreter::lookupVar(const std::string& name) {
	for (int i = (int)scopes.size() - 1; i >= 0; --i) {
		auto it = scopes[i].find(name);
		if (it != scopes[i].end()) return &it->second;
	}
	auto it = variables.find(name);
	if (it != variables.end()) return &it->second;
	return nullptr;
}
//Var* Interpreter::lookupVar(const std::string& name)
//{
//	for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
//		auto f = it->find(name);
//		if (f != it->end()) return &f->second;
//	}
//	auto g = variables.find(name);
//	if (g != variables.end()) return &g->second;
//	return nullptr;
//}

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

// ------- Declaration and Assignment -------
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
		if (scopes.empty()) variables[decl->name] = v;
		else scopes.back()[decl->name] = v;
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
	// ----------------------------------- Array Assignment -----------------------------------
	if (auto* ad = dynamic_cast<ArrayDeclaration*>(stmt)) {
		Var v{};
		v.varType = ad->elementType;
		
		v.isArray = true;

		// 1) determine final size
		int finalSize = ad->size;

		// If size omitted, infer from initializer count
		if (finalSize < 0) {
			finalSize = (int)ad->values.size();
		}

		// 2) allocate + fill
		if (ad->elementType == TOKEN_DEC_I || ad->elementType == TOKEN_DEC_B) {
			v.intArray.resize(finalSize, 0);
			for (int i = 0; i < (int)ad->values.size() && i < finalSize; i++) {
				v.intArray[i] = evalInt(ad->values[i]);
			}
		}
		else if (ad->elementType == TOKEN_DEC_S) {
			v.stringArray.resize(finalSize, "");
			for (int i = 0; i < (int)ad->values.size() && i < finalSize; i++) {
				v.stringArray[i] = evalString(ad->values[i]);
			}
		}
		else if (ad->elementType == TOKEN_DEC_F) {
			v.floatArray.resize(finalSize, 0.0f);
			for (int i = 0; i < (int)ad->values.size() && i < finalSize; i++) {
				//v.floatArray[i] = (float)evalInt(ad->values[i]); // or evalFloat if you have it
				v.floatArray[i] = (float)evalFloat(ad->values[i]); // update to evalFloat 
			}
		}
		else {
			throw std::runtime_error("Unsupported array type for '" + ad->name + "'");
		}

		// 3) store in current scope
		if (scopes.empty()) {
			variables[ad->name] = v;
		}
		else {
			scopes.back()[ad->name] = v;
		}
		// debug output array size
		std::cout << "[DEBUG] Stored array: " << ad->name
			<< " size=" << finalSize << std::endl;

		return;
	}

	
	if (auto* decl = dynamic_cast<Declaration*>(stmt)) {
		Var v{};
		v.varType = decl->varType;

		// ----------------------------------- ARRAY DECL -----------------------------------
		if (decl->isArray) {
			v.isArray = true;

			// decide final size
			int finalSize = decl->arraySize;
			if (finalSize < 0) {
				// [] with initializer -> size = initializer count
				finalSize = (int)decl->arrayInit.size();
			}

			// allocate + fill defaults
			if (v.varType == TOKEN_DEC_I || v.varType == TOKEN_DEC_B) {
				v.intArray.assign(finalSize, 0);
				for (int i = 0; i < (int)decl->arrayInit.size() && i < finalSize; ++i) {
					v.intArray[i] = evalInt(decl->arrayInit[i]);
					if (v.varType == TOKEN_DEC_B) v.intArray[i] = (v.intArray[i] != 0) ? 1 : 0;
				}
			}
			else if (v.varType == TOKEN_DEC_F) {
				v.floatArray.assign(finalSize, 0.0f);
				for (int i = 0; i < (int)decl->arrayInit.size() && i < finalSize; ++i) {
					v.floatArray[i] = (float)evalFloat(decl->arrayInit[i]);
				}
			}
			else if (v.varType == TOKEN_DEC_S) {
				v.stringArray.assign(finalSize, "");
				for (int i = 0; i < (int)decl->arrayInit.size() && i < finalSize; ++i) {
					v.stringArray[i] = evalString(decl->arrayInit[i]);
				}
			}
			else {
				throw std::runtime_error("Unsupported array type");
			}

			// store variable (global/local)
			if (scopes.empty()) variables[decl->name] = v;
			else scopes.back()[decl->name] = v;
			return;
		}

		// -------- SCALAR DECL --------
		if (decl->value) {
			if (v.varType == TOKEN_DEC_I)      v.intValue = evalInt(decl->value);
			else if (v.varType == TOKEN_DEC_B) v.intValue = evalBool(decl->value) ? 1 : 0;
			else if (v.varType == TOKEN_DEC_F) v.floatValue = (float)evalFloat(decl->value);
			else if (v.varType == TOKEN_DEC_S) v.stringValue = evalString(decl->value);
		}

		if (scopes.empty()) variables[decl->name] = v;
		else scopes.back()[decl->name] = v;
		return;
	}


	// -------------------------- select case statement --------------------------
	// 
	if (auto* sc = dynamic_cast<SelectStmt*>(stmt)) {

		// String select?
		if (isStringExpr(sc->expression)) {
			std::string selectVal = evalString(sc->expression);

			for (auto& caseBranch : sc->cases) {
				// enforce string cases for string select
				if (!isStringExpr(caseBranch.caseExpr)) {
					throw std::runtime_error("Type error: string select requires string case labels");
				}

				std::string caseVal = evalString(caseBranch.caseExpr);
				if (selectVal == caseVal) {
					for (auto* s : caseBranch.body) {
						if (!s) continue;
						execute(s);
						if (returning) return;
					}
					return; // stop after first match
				}
			}

			// default
			for (auto* s : sc->defaultBody) {
				if (!s) continue;
				execute(s);
				if (returning) return;
			}
			return;
		}

		// Otherwise: int select (existing behavior)
		int selectVal = evalInt(sc->expression);

		for (auto& caseBranch : sc->cases) {
			// enforce int cases for int select
			if (isStringExpr(caseBranch.caseExpr)) {
				throw std::runtime_error("Type error: int select cannot use string case labels");
			}

			int caseVal = evalInt(caseBranch.caseExpr);
			if (selectVal == caseVal) {
				for (auto* s : caseBranch.body) {
					if (!s) continue;
					execute(s);
					if (returning) return;
				}
				return;
			}
		}

		// default
		for (auto* s : sc->defaultBody) {
			if (!s) continue;
			execute(s);
			if (returning) return;
		}
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
		else if (isFloatExpr(e)) std::cout << evalFloat(e) << "\n";
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
	// --------------- Array access ---------------
	if (auto* acc = dynamic_cast<ArrayAccessExpr*>(expr)) {
		Var* v = lookupVar(acc->arrayName);
		if (!v) {
			std::cout << "[DEBUG] lookupVar failed for array '" << acc->arrayName << "'\n";
			std::cout << "[DEBUG] Globals contain:\n";

			for (auto& kv : variables) std::cout << "  " << kv.first << "\n";
			throw std::runtime_error("Undefined array: " + acc->arrayName);
		}

		if (!v->isArray) {
			throw std::runtime_error("'" + acc->arrayName + "' is not an array");
		}

		int idx = evalInt(acc->index);
		if (idx < 0) {
			throw std::runtime_error("Array index out of range (negative): " + std::to_string(idx));
		}

		// Choose correct backing storage based on declared type
		if (v->varType == TOKEN_DEC_I || v->varType == TOKEN_DEC_B) {
			if (idx >= (int)v->intArray.size()) {
				throw std::runtime_error("Array index out of range: " + std::to_string(idx) +
					" (size " + std::to_string((int)v->intArray.size()) + ")");
			}
			return v->intArray[idx];
		}

		if (v->varType == TOKEN_DEC_F) {
			if (idx >= (int)v->floatArray.size()) {
				throw std::runtime_error("Array index out of range: " + std::to_string(idx) +
					" (size " + std::to_string((int)v->floatArray.size()) + ")");
			}
			// If your evalInt must return int, you can cast float->int here,
			// but better is to support evalFloat for float expressions.
			return (int)v->floatArray[idx];
		}

		if (v->varType == TOKEN_DEC_S) {
			throw std::runtime_error("Type error: string array used in int expression: " + acc->arrayName);
			// (Handle string arrays in evalString instead)
		}

		throw std::runtime_error("Unsupported array type for '" + acc->arrayName + "'");
	}

	 //String in an int position:
	if (dynamic_cast<StringLiteral*>(expr)) {
		throw std::runtime_error("Type error: string where int expected");
	}
	throw std::runtime_error("Invalid int expression");
}

// Evaluate expressions floats

float Interpreter::evalFloat(Expression* expr) {
	if (auto* u = dynamic_cast<UnaryExpr*>(expr)) {
		if (u->op == TOKEN_MINUS) return -evalFloat(u->expr);
		if (u->op == TOKEN_NOT)   return evalBool(u->expr) ? 0.0f : 1.0f; // optional
	}

	if (auto* f = dynamic_cast<FloatLiteral*>(expr)) {
		return f->value;
	}
	if (auto* i = dynamic_cast<IntLiteral*>(expr)) {
		return (float)i->value;
	}
	if (auto* id = dynamic_cast<Identifier*>(expr)) {
		Var* v = lookupVar(id->name);
		if (!v) throw std::runtime_error("Undefined variable: " + id->name);
		if (v->varType != TOKEN_DEC_F)
			throw std::runtime_error("Type error: expected float variable: " + id->name);
		return v->floatValue;
	}

	// Array access: vector[2]
	if (auto* acc = dynamic_cast<ArrayAccessExpr*>(expr)) {
		Var* v = lookupVar(acc->arrayName);
		if (!v || !v->isArray) throw std::runtime_error("Not an array: " + acc->arrayName);

		int idx = evalInt(acc->index); // index must be int
		if (idx < 0 || idx >= (int)v->floatArray.size())
			throw std::runtime_error("Array index out of bounds: " + acc->arrayName);

		return v->floatArray[idx];
	}

	if (auto* bin = dynamic_cast<BinaryExpr*>(expr)) {
		float L = evalFloat(bin->left);
		float R = evalFloat(bin->right);
		switch (bin->op) {
		case TOKEN_PLUS:  return L + R;
		case TOKEN_MINUS: return L - R;
		case TOKEN_STAR:  return L * R;
		case TOKEN_SLASH: return L / R;
		default: break;
		}
	}

	throw std::runtime_error("Invalid float expression");
}


bool Interpreter::isFloatExpr(Expression* expr)
{
	if (dynamic_cast<FloatLiteral*>(expr)) return true;

	if (auto* id = dynamic_cast<Identifier*>(expr)) {
		Var* v = lookupVar(id->name);
		return v && v->varType == TOKEN_DEC_F;
	}

	if (auto* acc = dynamic_cast<ArrayAccessExpr*>(expr)) {
		Var* v = lookupVar(acc->arrayName);
		return v && v->isArray && v->varType == TOKEN_DEC_F;
	}

	if (auto* bin = dynamic_cast<BinaryExpr*>(expr)) {
		// if either side is float -> float math
		return isFloatExpr(bin->left) || isFloatExpr(bin->right);
	}

	return false;
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
	// -------------------- Arrays ---------------------------------
	if (auto* acc = dynamic_cast<ArrayAccessExpr*>(expr)) {
		Var* v = lookupVar(acc->arrayName);
		if (!v) throw std::runtime_error("Undefined array: " + acc->arrayName);
		if (!v->isArray) throw std::runtime_error("'" + acc->arrayName + "' is not an array");

		int idx = evalInt(acc->index);
		if (idx < 0) throw std::runtime_error("Array index out of range (negative): " + std::to_string(idx));

		if (v->varType != TOKEN_DEC_S) {
			throw std::runtime_error("Type error: non-string array used as string: " + acc->arrayName);
		}
		if (idx >= (int)v->stringArray.size()) {
			throw std::runtime_error("Array index out of range: " + std::to_string(idx) +
				" (size " + std::to_string((int)v->stringArray.size()) + ")");
		}
		return v->stringArray[idx];
	}
	// -------------------- Module call -----------------------------
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

	//  NEW: ---- ARRAY ---- name[index] returns string if name is a string array
	if (auto* acc = dynamic_cast<ArrayAccessExpr*>(expr)) {
		Var* v = lookupVar(acc->arrayName);
		if (!v) return false;
		return v->isArray && v->varType == TOKEN_DEC_S;
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


