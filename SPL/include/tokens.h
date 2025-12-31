#pragma once
#include <string>
#include <unordered_map>


// Token types enumeration 
enum type {
	TOKEN_IDENTIFIER,
	TOKEN_INT,
	TOKEN_STRING,
	TOKEN_FLOAT,
	// defines 
	TOKEN_IMPORT, // replace include #import
	// Single-character tokens
	TOKEN_EQUALS,
	TOKEN_SEMICOLON,		// ;
	TOKEN_COLON,			// :
	TOKEN_COMMA,			// ,
	TOKEN_LEFT_BRACKET,		// [
	TOKEN_RIGHT_BRACKET,	// ]
	TOKEN_LEFT_PAREN,		// (
	TOKEN_RIGHT_PAREN,		// )
	TOKEN_LEFT_CURL_PAREN,
	TOKEN_RIGHT_CURL_PAREN,
	TOKEN_PLUS,
	TOKEN_MINUS,
	TOKEN_STAR,
	TOKEN_SLASH,
	TOKEN_COMMENT, // Comment
	// Key words
	// TOKEN_MOD for module all code between TOKEN_MOD and TOKEN_MODEND will be treated as a module
	TOKEN_MOD,     
	TOKEN_MODEND,  
	TOKEN_POUT,
	TOKEN_IFITS,
	TOKEN_ELSEITS,
	TOKEN_ELSE,
	TOKEN_ENDIFITS,
	TOKEN_WHILE,
	TOKEN_DO,		//  do loop
	TOKEN_SELECT,	// switch case
	TOKEN_CASE,		// case in switch
	TOKEN_DEFAULT,  // default in switch
	
	TOKEN_BREAK,	// break in switch, loops, IFITS
	TOKEN_CONTINUE, // continue in loops, switch & IFITS
	TOKEN_RETURN,
	// logical operators
	TOKEN_INCREMENT, // add ++
	TOKEN_DECREASE, // sub --
	TOKEN_ISLESS,
	TOKEN_ISMORE,
	TOKEN_ISEQUAL,
	TOKEN_ISNOTEQUAL,
	TOKEN_AND,	// && operator
	TOKEN_OR,	// || operator
	// unary operators
	TOKEN_NOT,	// ! unary operator
	// boolean literals
	TOKEN_TRUE,
	TOKEN_FALSE,
	// declare dec_function token
	// dec_int, dec_string, dec_float dec_boolean
	TOKEN_DEC_I, // declare integer
	TOKEN_DEC_S,
	TOKEN_DEC_F,
	TOKEN_DEC_B, // declare boolean
	// End of file token to stop memory leaks
	TOKEN_EOF
};
// Token structure
struct Token {
	enum type TYPE;
	std::string VALUE;
};
// Map of keywords to their token types
static const std::unordered_map<std::string, type> KEYWORDS = {
	{"import",		TOKEN_IMPORT},		// replace include #import
	{"IfIts",		TOKEN_IFITS},
	{"ElseIts",		TOKEN_ELSEITS},
	{"Else",		TOKEN_ELSE},
	{"EndIfIts",	TOKEN_ENDIFITS},
	{"while",		TOKEN_WHILE},		// while loop one condition
	{"do",			TOKEN_DO},			// loop do untill with three condition
	{"select",      TOKEN_SELECT},		// switch case
	{"case",        TOKEN_CASE},		// case in switch
	{"default",     TOKEN_DEFAULT},		// default in switch
	{"break",       TOKEN_BREAK},		// break in switch, loops, IFITS
	{"continue",    TOKEN_CONTINUE},	// continue in loops, switch & IFITS
	{"return",		TOKEN_RETURN},
	{"mod",			TOKEN_MOD},			// module
	{"modEnd",		TOKEN_MODEND},		// module end
	{"++",			TOKEN_INCREMENT},	// ++
	{"--",			TOKEN_DECREASE},	// --
	{"isLess",		TOKEN_ISLESS},		// less than
	{"isMore",		TOKEN_ISMORE},		// more than
	{"isEqual",		TOKEN_ISEQUAL},
	{"isNotEqual",	TOKEN_ISNOTEQUAL},
	{"&&",			TOKEN_AND },		// logical operators
	{"||",			TOKEN_OR },			// logical operators
	{"!",			TOKEN_NOT },		// unary operator
	{"pout",		TOKEN_POUT},
	{"dec_i",		TOKEN_DEC_I},		// declare integer
	{"dec_s",		TOKEN_DEC_S},		// declare string
	{"dec_f",		TOKEN_DEC_F},		// declare float
	{"true",		TOKEN_TRUE},		// boolean literals
	{"false",		TOKEN_FALSE},		// boolean literals
	{"dec_b",		TOKEN_DEC_B}		// declare boolean
};

