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

	//bool GetProjectCode(const std::string& projectPath, const std::string& projectMainFile) {

	//	std::string fullPath = projectPath + "\\" + projectMainFile;
	//	// Load code from the main file
	//	std::string code;
	//	if (!SPL::loadCodeFromFile(fullPath, code)) {
	//		std::cerr << "Error: could not open .spl file" << fullPath << std::endl;
	//		return false;
	//	}
	//	// Run the loaded code
	//	if (!SPL::RunCode(code)) {
	//		std::cerr << "Failed: to run main file" << fullPath << std::endl;
	//		return false;
	//	}
	//	return true;
	//}




	// Runs SPL code from a source string in the editor just call RucCode(editorText.spl)
    void RunProjectCode(const std::string& source) {
        Lexer lexer(source);
        std::vector<Token*> tokens = lexer.tokenize();

        Parser parser(tokens);
        Interpreter interp;

        while (Statement* stmt = parser.parseStatement()) {
            interp.execute(stmt);

			try {
				while (true)
				{
					Statement* stmt = parser.parseStatement();
					if (stmt == nullptr) {
						if (parser.peek()->TYPE == TOKEN_EOF) break;
						else continue;
					}
					interp.execute(stmt);
				}

			}
			catch (std::runtime_error& e) {
				// Assuming the error is due to end of input
				std::cout << "Parsing completed." << e.what() << std::endl;
			}

			// Debug: print all tokens
			//printTokens(tokens);


        }

        freeTokens(tokens);
    }
}
