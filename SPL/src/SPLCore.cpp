#include <fstream>
#include <sstream>
#include <vector>

#include "../include/SPLCore.h"
#include "../include/lexer.h"
#include "../include/parsing.h"
#include "../include/interpreter.h"
#include "../include/splHelper.h"

namespace SPL {

	bool loadCodeFromFile(const std::string& filepath, std::string& outSource)
	{
		// hard-coded test file for now load from open dialog later or file tree
		std::ifstream sourceFileStream(filepath);
		if (!sourceFileStream) {
			std::cerr << "Error: could not open .spl file" << filepath << std::endl;
			return false;
		}

		std::stringstream buffer;
		buffer << sourceFileStream.rdbuf(); // simpler way to read entire file
		outSource = buffer.str();
		return true;
	

	}

	// Runs SPL code from a source string in the editor just call RucCode(editorText.spl)
    void RunProjectCode(const std::string& source) {

		Lexer lex(source);
		std::vector<Token*> tokens = lex.tokenize();

		Parser parser(tokens);
		
		try {
			std::vector<Module*> mods = parser.parseProgram();
			Interpreter interp;
			interp.runProgramEntry(mods, "main");
		}
		catch (const std::exception& e) {
			std::cerr << "SPL error: " << e.what() << "\n";
		}

		// cleanup tokens (and optionally modules if you add destructors)
		freeTokens(tokens);
		
   
    }
}
