#define STB_TRUETYPE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "glad\glad.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
// GLwin for window
#include "GLwin\include\GLwin.h"
#include "GLwin\include\GLwinLOG.h"
// SpxGui for GUI
#include "SpxGui\SpxGui.h"
#include "SpxGui\SpxGuiWidgets.h"
#include "SpxGui\Helpers.h" 


#include "lexer.h"
#include "parsing.h"
#include "interpreter.h"
#include "splHelper.h"

// #include "SPLCore.h"  ------------ not needed for this main.cpp test file just now -----------


int main() {

	GLWIN_LOG_INFO("Starting SPL Interpreter. Editor");

	GLWIN_window* window = GLwin_CreateWindow(800, 600, L"SPL Code Editor 2025");

	if (!window) {
		GLWIN_LOG_ERROR("Failed to create window");
		return -1;
	}

	GLwinEnableCustomTitleBar(window, 1); // enable custom title bar
	GLwinMakeContextCurrent(window);
	SpxGui::gMainWindow = window;
	GLwinSetCharCallback(window, SpxGui::CharCallback);
	GLwinSetKeyCallback(window, SpxGui::KeyCallback);

	if (!gladLoadGLLoader((GLADloadproc)GLwinGetProcAddress)) {
		GLWIN_LOG_ERROR("Failed to initialize GLAD");
		return -1;
	}
	else {
		GLWIN_LOG_INFO("GLAD initialized successfully");
	}

	SpxGui::Init();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	glGetString(GL_VERSION);
	GLWIN_LOG_INFO("OpenGL Version: " << glGetString(GL_VERSION));

	// hard-coded test file (must be in the same folder as your .exe or give full path)
	std::ifstream sourceFileStream("test.spl");
	if (!sourceFileStream) {
		std::cerr << "Error: could not open file test.spl" << std::endl;
		return 1;
	}

	std::stringstream buffer;
	buffer << sourceFileStream.rdbuf(); // simpler way to read entire file

	std::string sourceCode = buffer.str();
	// Print the source code
	//std::cout << "Source Code:\n" << sourceCode << std::endl;
	// Tokenize the source code
	Lexer lexer(sourceCode);
	std::vector<Token *> tokens = lexer.tokenize();

	// Parse the tokens
	Parser parser(tokens);
	Interpreter interp;
	
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

	
	GLWIN_LOG_INFO("This is the end of the program");

	freeTokens(tokens);

	while (!GLwinWindowShouldClose(window, 0)) {
		GLwinPollEvents();

		if (GLwinGetKey(window, GLWIN_ESCAPE) == GLWIN_PRESS) {
			GLwinWindowShouldClose(window, 1);
		}

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		GLwinSwapBuffers(window);

		GLenum err = glGetError();
		if (err != GL_NO_ERROR) {
			GLWIN_LOG_ERROR("OpenGL error: " << err);
		}
	}



	return 0;

	
}






