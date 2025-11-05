#define STB_TRUETYPE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "glad\glad.h"
#include <iostream>
//#include <fstream>
//#include <sstream>
//#include <vector>
// GLwin for window
#include "GLwin\include\GLwin.h"
#include "GLwin\include\GLwinLOG.h"
// SpxGui for GUI
#include "SpxGui\SpxGui.h"
#include "SpxGui\SpxGuiWidgets.h"
 
// SPL Core for Interpreter / Compiler
#include "SPLCore.h"


int main() {

	GLWIN_LOG_INFO("Starting SPL Interpreter. Editor");
	// ------------------------- GLwin Window initialization -------------------------
	GLWIN_window* window = GLwin_CreateWindow(800, 600, L"SPL Code Editor 2025");

	if (!window) {
		GLWIN_LOG_ERROR("Failed to create window");
		return -1;
	}

	GLwinEnableCustomTitleBar(window, 1); // enable custom title bar
	GLwinMakeContextCurrent(window);

	// Set global main window pointer for SpxGui
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

	// ------------------------- End of Window initialization -------------------------
		
	// run the Compiler / Interpreter
	/*std::string code;
	if (SPL::loadCodeFromFile("test.spl", code)) {
		SPL::RunProjectCode(code);
		GLWIN_LOG_INFO("Loaded SPL code from file successfully.");

	}
	else {
		GLWIN_LOG_ERROR("Failed to load SPL code from file.");
					
	}*/

	static SpxGui::SpxGuiTreeView root = SpxGui::LoadDirectory("../SPLEditor");
	std::string code;
	//bool RunCode = false;
	while (!GLwinWindowShouldClose(window, 0)) {
		GLwinPollEvents();

		double cx, cy;
		GLwinGetCursorPos(window, &cx, &cy);
		int fbw, fbh;
		GLwinGetFramebufferSize(window, &fbw, &fbh);
		int w, h;
		SpxGui::UpdateScreenSize(fbw, fbh);
		glViewport(0, 0, fbw, fbh);
		SpxGui::SetScreenSize(fbw, fbh);

		SpxGui::MenuInit();

		bool downNow = (GLwinGetMouseButton(window, GLWIN_MOUSE_BUTTON_LEFT) == GLWIN_PRESS);

		static bool prevDown = false;
		SpxGui::pressed = (downNow && !prevDown);
		SpxGui::released = (!downNow && prevDown);
		SpxGui::down = downNow;
		prevDown = downNow;

		if (SpxGui::gCurrent) {
			SpxGui::gCurrent->mouseX = (float)cx;
			SpxGui::gCurrent->mouseY = (float)cy;	
		}

		if (GLwinGetKey(window, GLWIN_ESCAPE) == GLWIN_PRESS) {
			GLwinWindowShouldClose(window, 1);
		}

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Editor File Explorer set up
		int treeWidth = 250;
		int tabWidth = fbw - treeWidth;
		int tabHeight = fbh - SpxGui::gMenuBarHeight;

		{
			SpxGui::Begin("File Explorer", nullptr, 1);
			SpxGui::gCurrent->curWinX = 0;
			SpxGui::gCurrent->curWinY = SpxGui::gMenuBarHeight;
			SpxGui::gCurrent->curWinW = treeWidth;
			SpxGui::gCurrent->curWinH = tabHeight;

			SpxGui::DrawFileNode(root);

			SpxGui::End();

		}

		// Editor Tabs set up
		
			SpxGui::Begin("Editor Tabs", nullptr, 2);
			SpxGui::gCurrent->curWinX = treeWidth;
			SpxGui::gCurrent->curWinY = SpxGui::gMenuBarHeight;
			SpxGui::gCurrent->curWinW = tabWidth;
			SpxGui::gCurrent->curWinH = tabHeight;
			// Display opened files in tabs
			if (SpxGui::BeginTabBar("FileTabs")) {
				for (size_t i = 0; i < SpxGui::gOpenFiles.size(); i++) {
					auto& f = SpxGui::gOpenFiles[i];
					if (SpxGui::BeginTabItem(f.name.c_str())) {
						SpxGui::gActiveTab = (int)i;
						static char buf[265] = "Input Text";

						SpxGui::MultiLineText(f.name.c_str(), f.buffer, tabWidth - 30.0f, tabHeight - 60.0f);
						
						SpxGui::EndTabItem();
					}
				}

				SpxGui::EndTabBar();
			}
			SpxGui::End();
		
			if (SpxGui::RunCode) {
				if (SPL::loadCodeFromFile("test.spl", code)) {
					SPL::RunProjectCode(code);
					GLWIN_LOG_INFO("Loaded SPL code from file successfully.");

				}
				else {
					GLWIN_LOG_ERROR("Failed to load SPL code from file.");

				}
				SpxGui::RunCode = false;
			}



		SpxGui::NewFrame((float)cx, (float)cy, downNow, SpxGui::pressed, SpxGui::released, fbw, fbh);

		SpxGui::RenderMenuBar();

		SpxGui::activeToolBar();

		SpxGui::Render();
		
		GLwinSwapBuffers(window);

		GLenum err = glGetError();
		if (err != GL_NO_ERROR) {
			GLWIN_LOG_ERROR("OpenGL error: " << err);
		}
	}

	//freeTokens(tokens);
	GLWIN_LOG_INFO("This is the end of the program");
	return 0;

	
}






