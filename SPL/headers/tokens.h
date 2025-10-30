#pragma once
#include <string>
#include <unordered_map>

// TOKEN_IDENTIFIER: identifiers (variable names, function names, etc.) looking for letters or digits
// TOKEN_INT: integer literals (numbers)
// TOKEN_LEFT_PAREN = ( - TOKEN_RIGHT_PAREN = )
// TOKEN_LEFT_CURL_PAREN = { - TOKEN_RIGHT_CURL_PAREN = }
enum type {
	TOKEN_IDENTIFIER,
	TOKEN_INT,
	TOKEN_STRING,
	// Single-character tokens
	TOKEN_EQUALS,
	TOKEN_SEMICOLON,
	TOKEN_LEFT_PAREN,
	TOKEN_RIGHT_PAREN,
	TOKEN_LEFT_CURL_PAREN,
	TOKEN_RIGHT_CURL_PAREN,
	TOKEN_PLUS,
	TOKEN_MINUS,
	TOKEN_STAR,
	TOKEN_SLASH,
	TOKEN_COMMENT,
	// Key words
	TOKEN_POUT,
	TOKEN_IF,
	TOKEN_WHILE,
	TOKEN_RETURN,
	// declare dec_function token
	// dec_int, dec_string, dec_float
	TOKEN_DEC_I,
	TOKEN_DEC_S,
	TOKEN_DEC_F,
	// End of file token
	TOKEN_EOF
};

struct Token {
	enum type TYPE;
	std::string VALUE;
};

static const std::unordered_map<std::string, type> KEYWORDS = {
	{"if", TOKEN_IF},
	{"while", TOKEN_WHILE},
	{"return", TOKEN_RETURN},
	{"pout", TOKEN_POUT},
	{"dec_i", TOKEN_DEC_I},
	{"dec_s", TOKEN_DEC_S},
	{"dec_f", TOKEN_DEC_F}
};

