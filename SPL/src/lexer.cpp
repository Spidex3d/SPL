#include "../headers/lexer.h"
#include <vector>
#include <unordered_map>
#include "../headers/tokens.h"
#include <iostream>

Lexer::Lexer(const std::string& sourceCode)
	: source(sourceCode), cursor(0), size(sourceCode.length()),
		current(sourceCode.empty() ? '\0' : sourceCode[0]) {}


char Lexer::advance()
{
	if (cursor < size) {
		char temp = current;
		cursor++;
		current = (cursor < size) ? source[cursor] : '\0'; // End of source
		return temp;
	}
	else {
		return '\0';
	}
}

char Lexer::peek(int offset)
{
	if (cursor + offset < size) {
		return source[cursor + offset];
	}
	else {
		return '\0';
	}
}

std::vector<Token*> Lexer::tokenize()
{	
		std::vector<Token*> tokens;
		Token* tokenID;
		bool notEOF = true;
		while (cursor < size && notEOF) {
			current = source.at(cursor);
			// Skip whitespace
			if (isspace(current)) {
				advance();
			}

			else if (isalpha(current)) {
				std::string identifier;
				while (isalnum(current) || current == '_') {
					identifier += current;
					advance();
				}

				auto it = KEYWORDS.find(identifier);
				if (it != KEYWORDS.end()) {
					tokenID = new Token{ it->second, identifier }; // keyword
				}
				else {
					tokenID = new Token{ TOKEN_IDENTIFIER, identifier }; // normal name
				}

				tokens.push_back(tokenID);
			}
			// strings
			else if (current == '"') {
				std::string strValue;
				advance(); // skip opening quote
				while (current != '"' && current != '\0') {
					strValue += current;
					advance();
				}
				if (current == '"') {
					advance(); // skip closing quote
					tokenID = new Token{ TOKEN_STRING, strValue };
					tokens.push_back(tokenID);
				}
				else {
					std::cerr << "Lexer error: unterminated string literal at position " << cursor << std::endl;
				}
			}

			// Handle integer literals Numbers
			else if (isdigit(current)) {
				std::string number;
				while (isdigit(current)) {
					number += current;
					advance();
				}
				tokenID = new Token{ TOKEN_INT, number };
				tokens.push_back(tokenID);
			}
			// Handle comments
			else if (current == '/' && peek(1) == '/') {
				std::string comment;
				advance(); // skip first '/'
				advance(); // skip second '/'

				while (current != '\n' && current != '\0') {
					comment += current;
					advance();
				}
				tokenID = new Token{ TOKEN_COMMENT, comment };
				tokens.push_back(tokenID);
			}

			// Handle single-character tokens
			else {
				switch (current) {

				case '=':
					tokenID = new Token{ TOKEN_EQUALS, "=" };
					tokens.push_back(tokenID);
					advance();
					break;
				case ';':
					tokenID = new Token{ TOKEN_SEMICOLON, ";" };
					tokens.push_back(tokenID);
					advance();
					break;
				case '(':
					tokenID = new Token{ TOKEN_LEFT_PAREN, "(" };
					tokens.push_back(tokenID);
					advance();
					break;
				case ')':
					tokenID = new Token{ TOKEN_RIGHT_PAREN, ")" };
					tokens.push_back(tokenID);
					advance();
					break;
				case '{':
					tokenID = new Token{ TOKEN_LEFT_CURL_PAREN, "{" };
					tokens.push_back(tokenID);
					advance();
					break;
				case '}':
					tokenID = new Token{ TOKEN_RIGHT_CURL_PAREN, "}" };
					tokens.push_back(tokenID);
					advance();
					break;
				case '+':
					tokenID = new Token{ TOKEN_PLUS, "+" };
					tokens.push_back(tokenID);
					advance();
					break;
				case '-':
					tokenID = new Token{ TOKEN_MINUS, "-" };
					tokens.push_back(tokenID);
					advance();
					break;
				case '*':
					tokenID = new Token{ TOKEN_STAR, "*" };
					tokens.push_back(tokenID);
					advance();
					break;
				case '/':
					tokenID = new Token{ TOKEN_SLASH, "/" };
					tokens.push_back(tokenID);
					advance();
					break;
				case '\0':
					notEOF = false;
					break;
				default:
					std::cerr << "Lexer error: unexpected character '"
						<< current << "' at position " << cursor << std::endl;
					advance();
					break;
				}
			}
		}

		tokens.push_back(new Token{ TOKEN_EOF, "" }); // Add EOF token at the end
		return tokens;
}
