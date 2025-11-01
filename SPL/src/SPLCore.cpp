#include "../include/SPLCore.h"
#include "../include/lexer.h"
#include "../include/parsing.h"
#include "../include/interpreter.h"
#include "../include/splHelper.h"

namespace SPL {
	// Runs SPL code from a source string in the editor just call RucCode(editorText.spl)
    void RunCode(const std::string& source) {
        Lexer lexer(source);
        std::vector<Token*> tokens = lexer.tokenize();

        Parser parser(tokens);
        Interpreter interp;

        while (Statement* stmt = parser.parseStatement()) {
            interp.execute(stmt);
        }

        freeTokens(tokens);
    }
}
