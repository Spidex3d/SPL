#pragma once
#include "tokens.h"
#include <vector>

void freeTokens(std::vector<Token*>& tokens);
// Debug print: dump all tokens to console
void printTokens(const std::vector<Token*>& tokens);

