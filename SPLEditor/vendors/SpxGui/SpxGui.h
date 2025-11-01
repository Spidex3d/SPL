#pragma once
#include "glad\glad.h" // Include glad for OpenGL function loading
#include <vector>
#include <algorithm>
#include <string>
#include "Helpers.h"
#include "../vendors/GLwin/include/GLwin.h"
#include "../vendors/GLwin/include/GLwinDefs.h"
#include "stb/stb_image.h" // Include stb_image.h for image loading
#include "stb\stb_truetype.h" // Include stb_truetype.h for font rendering
#include <unordered_map>
#include <iostream>

#include <fstream> // Include fstream for file operations
#include <filesystem>
namespace fs = std::filesystem;

// Excellent resource for learning OpenGL: https://learnopengl.com/
// Excellent resource for learning modern OpenGL: https://docs.gl/
// And lets not for get YouTub The Cherno for C++ & openGL channel https://www.youtube.com/@TheCherno
// Error to fix if you resize the main window befor you move the gui window you cant move it anymore

// moveing to Retained Mode so we can have multiple windows and widgets with state - focus

// I think we need to move this in to it's own Shader file
const char* vertexSrc = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;

out vec2 TexCoord;

uniform vec2 uScreenSize;

void main() {
    vec2 ndc = (aPos / uScreenSize) * 2.0 - 1.0;
    ndc.y = -ndc.y; // flip Y
    gl_Position = vec4(ndc, 0.0, 1.0);
    TexCoord = aUV;
}
)";

const char* fragmentSrc = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D uTex;
uniform vec3 uColor;
uniform int useTex;   // 0 = solid, 1 = font alpha, 2 = full image

void main() {
    if (useTex == 0) {
        // solid color
        FragColor = vec4(uColor, 1.0);
    }
     else if (useTex == 1) {
        // font rendering with alpha from texture
        float alpha = texture(uTex, TexCoord).r; // assuming font atlas is in red channel         
        FragColor = vec4(uColor, alpha);
    } else if(useTex == 2) {
        // normal RGBA image    
        FragColor = texture(uTex, TexCoord);
    }
}
)";
namespace SpxGui { // I have never used namespaces before

// forward declarations
inline void DrawRect(float x, float y, float w, float h, float r, float g, float b);
inline void DrawText(float x, float y, const char* txt, float r, float gcol, float b);
inline void Init(int screenW, int screenH);
inline unsigned int LoadTextuer(const std::string& filename, struct Image& img);
inline void DrawImage(unsigned int texID, float x, float y, float w, float h);
inline bool ButtonNew(const char* label, float w, float h);
inline bool Button(const char* label, float w, float h);
  
// Text Input related globals
inline std::string gInputChars; // characters input this frame
inline uintptr_t activeTextID = 0; // ID of the active text box focus
inline int caretCol = 0;     // caretCol for single-line text boxes
inline int caretRow = 0;     // caretRow for multi-line text boxes
// special keys like backspace, delete, arrows input
inline std::vector<int> gInputKeys; // This for handling special keys like backspace, delete, arrows


inline int caretIndex = 0;   // caretIndex cursor position in the active text buffer (caret = carage return)
inline char* activeBuf = nullptr; // later for multiple text boxes


	// ---------------------------------- Struct for storing style settings -------------------------------------------------
    struct Style {
        float WindowRounding = 0.0f;
		// menu bar color gui background
		float MenuBarBgR = 0.20f;
		float MenuBarBgG = 0.20f;
		float MenuBarBgB = 0.55f;   
		// window background color
        float WindowBgR = 0.15f;
        float WindowBgG = 0.15f;
        float WindowBgB = 0.17f;
		// top bar color gui header
		float WindowTopBarR = 0.25f;
		float WindowTopBarG = 0.25f;
		float WindowTopBarB = 0.28f;
        // top x button color
        float WindowTopButR = 1.0f;
        float WindowTopButG = 0.25f;
        float WindowTopButB = 0.28f;
		// items like buttons, inputbox, ImageBox etc
        float ItemSpacingX = 8.0f;
        float ItemSpacingY = 6.0f;
        float WindowPaddingX = 10.0f;
        float WindowPaddingY = 28.0f; // leave room for header       
		
    };
	// ----------------------------------- struct for storing draw commands as we move to Retained Mode -----------------------------------
    struct DrawCmd {
        enum Type { RECT, TEXT, IMAGE, CARET } type;
        float x, y, w, h;
        float r, g, b;
        unsigned int texID = 0;   // for IMAGE
        std::string text;         // for TEXT

        // Rect (generic solid rectangle)
        DrawCmd(Type t, float _x, float _y, float _w, float _h,
            float _r, float _g, float _b)
            : type(t), x(_x), y(_y), w(_w), h(_h),
            r(_r), g(_g), b(_b) {
        }

        // Text structer
        DrawCmd(Type t, float _x, float _y,
            float _r, float _g, float _b, const std::string& txt)
            : type(t), x(_x), y(_y),
            r(_r), g(_g), b(_b), text(txt) {
        }

        // Image structer
        DrawCmd(Type t, unsigned int _texID, float _x, float _y, float _w, float _h)
            : type(t), x(_x), y(_y), w(_w), h(_h),
            texID(_texID) {
        }

		// Caret (always 2px wide, height = font size) what I call carridg return - cursor
        DrawCmd(Type t, float _x, float _y, float _h,
            float _r, float _g, float _b)
            : type(t), x(_x), y(_y), w(2.0f), h(_h),
            r(_r), g(_g), b(_b) {
        }
        
    };
    // ---------------------------------- Struct for storing Table settings -------------------------------------------------

    struct TableState {
        int columnCount = 0;
        int currentColumn = 0;
        float startX = 0.0f;
        float rowY = 0.0f;
        float colWidth = 0.0f;
        bool active = false;
    };
    // jump to 854
	//----------------------------------- struct for storage for the main gui window -----------------------------------
    struct SpxGuiWindow {
        int SpxGuiWinID = 0;       // ID for multiple windows
        std::string title;
        bool* open = nullptr;
        bool inWindow = false;
		bool isPopup = false; //popup
        unsigned int backgroundTex = 0; // 0 = none

        // Window position & size
        float curWinX = 100;
        float curWinY = 100;
        float curWinW = 300;
        float curWinH = 400;
		// popup window pos & size
        float curPopWinX = 50;
        float curPopWinY = 50;
        float curPopWinW = 300;
        float curPopWinH = 300;

        // Widget layout state
        float cursorX = 0.0f;
        float cursorY = 0.0f;
        float lastItemW = 0.0f;
        float lastItemH = 0.0f;

        // Header bar
        float headerHeight = 24.0f;
        bool dragging = false;
        float dragOffsetX = 0, dragOffsetY = 0;

        // Mouse state (copied from global per-frame)
        float mouseX = 0, mouseY = 0;
        bool mouseDown = false;
        bool mousePressed = false;
        bool mouseReleased = false;

		std::vector<DrawCmd> drawList; // for widgets  to add their draw commands

		TableState table; // for tables
		
    };
	// ---------------------------------- global-only things into a single ContextAddInputChar struct -------------------------------------------------
    struct Context {
        int screenW = 800;
        int screenH = 600;

        GLuint fontTex = 0;
        stbtt_bakedchar cdata[96];
        float fontSize = 16.0f;

        GLuint gShader = 0;
        GLint uScreenSizeLoc = -1;
        GLint uColorLoc = -1;

        int frameCount = 0;
        int callCount = 0;
    };

	inline std::vector<SpxGuiWindow> gWindows; // later for multiple windows
	inline SpxGuiWindow* gCurrent = nullptr; // later for multiple windows
	inline int gActiveWinID = -1; // later for multiple windows focus

    
    Style style;

    // ---------------------------------- Struct for storing Image settings -------------------------------------------------
    // needed
    struct Image {
        GLuint textureID = 0;
        int width = 0;
        int height = 0;
        int ColIndex = 0; // later for multiple colors in one texture
        GLenum i_format; // later for different image formats
    };

	// ---------------------------------- Struct for storing Tree view settings -------------------------------------------------
	struct SpxGuiTreeView {
        std::string name;
        std::string fullPath;
        bool isDir;
        bool expanded = false;
        std::vector<SpxGuiTreeView> children;
		// jump 646
	};

    // ---------------------------------- Struct for storing Tabs settings -------------------------------------------------
    struct SpxGuiTabBar {
		std::string title;
		std::vector<std::string> tabs;
		int activeTabIndex = 0;
		float startX = 0, startY = 0;
		float height = 0;
		bool inTabBar = false;
    };
    inline SpxGuiTabBar gTabBar; // only one at a time for now
    std::string editorText;
    // jump to 600
	// ----------------------------- ---- Struct for storing Open File settings TAB BAR ------------------------------------------
	// this is part of tab

    void LoadFile(const std::string& path) {
        std::ifstream ifs(path);
        std::stringstream ss;
        ss << ifs.rdbuf();
        editorText = ss.str();
    }


    struct OpenFile {
        std::string path;         // full path to file
        std::string name;         // short name for tab display
        //std::vector<char> buffer; // editable text buffer
		std::string buffer;       // editable text buffer for multi-line text
        bool modified = false;    // track if changed since last save
    };

    inline std::vector<OpenFile> gOpenFiles;
    inline int gActiveTab = -1;  // index into gOpenFiles, -1 = none


    //  ------------------------------------------------ New Menu Bar -----------------------------------
         
    // --- Globals ---
    inline int gScreenW = 0;
    inline int gScreenH = 0;

    struct MenuItem {
        std::string label;
        bool open = false;
        std::vector<std::string> subItems;
    };

    inline float gMouseX = 0.0f;
    inline float gMouseY = 0.0f;
    inline bool gMouseDown = false;
    inline bool gMousePressed = false;
    inline bool gMouseReleased = false;

	// Title Bar dimensions
    inline float gMenuBarHeight = 65.0f;    // toolbar/menu height
	inline float gTitleButton = 30.0f;      // height of title bar buttons
	inline float gIconSizeX = 376.0f;         // size of icon in title bar
	inline float gIconSizeY = 30.0f;         // size of icon in title bar
    unsigned int IconTex = 0; // 0 = none

	// Min Max & Close button size
    unsigned int closebtnSize = 30;
    
    
    static SpxGui::Image iconTex;
    static unsigned int iconTexID = 0;
	static unsigned int iconMinTexID = 0;
	static unsigned int iconMaxTexID = 0;
	static unsigned int iconCloseTexID = 0;

    // Tool bar

    // Mouse state (fed from main.cpp every frame)
    inline bool down = false;     // true while button is held
    inline bool pressed = false;  // true only on the frame mouse went down
    inline bool released = false; // true only on the frame mouse released
    inline int gMouseGlobalX = 0; // updated each frame via GLwinGetGlobalCursorPos
    inline int gMouseGlobalY = 0;

	inline int gClientScreenX = 0; // updated each frame via GLwinGetClientScreenOrigin
	inline int gClientScreenY = 0; // updated each frame via GLwinGetClientScreenOrigin

  

    inline GLWIN_window* gMainWindow = nullptr;  // set from main

    // Drag state
    inline bool gMenuBarDragging = false;
    inline int dragStartMouseX = 0;
    inline int dragStartMouseY = 0;
    inline int dragStartWinX = 0;
    inline int dragStartWinY = 0;

	inline void MenuInit() {
		if (!gMainWindow) return; // safety check
       
        int gx, gy;
        GLwinGetGlobalCursorPos(gMainWindow, &gx, &gy);
        gMouseGlobalX = gx;
        gMouseGlobalY = gy;

        int cox, coy;
        GLwinGetClientScreenOrigin(gMainWindow, &cox, &coy);
        gClientScreenX = cox;
        gClientScreenY = coy;

	}
	// ---------------------------------------- Tool bar Section for Title bar ---------------------------------------
	 
	// Bottom Bar
    struct ToolbarItem {
        std::string texPath;
        unsigned int texID = 0;
        std::string tooltip;    // optional tooltip text
        bool clicked = false;
    };

	// Global toolbar Bottom bar
    inline std::vector<ToolbarItem> gToolbar;
	inline bool gToolbarInitialized = false;
	inline int gActiveTool = -1; //-1 = none selected
    

	// Build Toolbar Resources Bottom Row
	inline void BuildToolBarResources() {
        gToolbar.push_back({ "../SPLEditor/Icon/open.jpg", 0, "Open File" });
        gToolbar.push_back({ "../SPLEditor/Icon/save.jpg",   0, "Save" });
        gToolbar.push_back({ "../SPLEditor/Icon/save_as.jpg", 0, "Save As" });
        gToolbar.push_back({ "../SPLEditor/Icon/go.jpg", 0, "Run" });
        gToolbar.push_back({ "../SPLEditor/Icon/stop.jpg", 0, "Stop" });
	}   

	// Initialize textures once Bottom Row
    inline void InitToolBar() {
        for (auto& item : gToolbar) {

            if (item.texID == 0) { // already loaded
                Image img;
                item.texID = LoadTextuer(item.texPath, img);
            }
        }
    }

	// Ensure Toolbar Bottom Row
    inline void EnsureToolbar() {
		if (!gToolbarInitialized) {
			BuildToolBarResources();
			InitToolBar();
			gToolbarInitialized = true;
		}
    }
    
	// Render Toolbar Bottom Row
    inline void RenderToolbar(float startX, float startY, float iconSize, float spacing = 6.0f) {
        float x = startX;

        for (int i = 0; i < (int)gToolbar.size(); ++i) {
            auto& item = gToolbar[i];

            bool hover = (gMouseX >= x && gMouseX <= x + iconSize &&
                gMouseY >= startY && gMouseY <= startY + iconSize);
            bool pressed = (hover && gMousePressed);

            // Selected/hover background
            if (i == gActiveTool) {
                DrawRect(x - 3, startY - 3, iconSize + 6, iconSize + 6, 0.28f, 0.28f, 0.38f);
				
            }
            else if (hover) {
                DrawRect(x - 2, startY - 2, iconSize + 4, iconSize + 4, 0.22f, 0.22f, 0.30f);
            }

            // Icon
            if (item.texID) DrawImage(item.texID, x, startY, iconSize, iconSize);
            else            DrawRect(x, startY, iconSize, iconSize, 0.6f, 0.1f, 0.1f);

            // Click handling -> make it the active tool
            item.clicked = false;
            if (pressed) {
                gActiveTool = i;
                item.clicked = true;
                // std::cout << "Clicked tool: " << item.tooltip << "\n";
            }

            x += iconSize + spacing;
        }
    }
	inline std::string filename = "";
	
	// Toolbar action handler Bottom Row
    inline void activeToolBar() {
        if (SpxGui::gActiveTool >= 0) {
            switch (SpxGui::gActiveTool) {
            case 0:
                filename = GLwinOpenDialog();
                if (!filename.empty()) {
                    // Save to this file...
                }
                std::cout << "Open File action triggered\n";
                // TODO: add your file open dialog here
                break;

            case 1:
                filename = GLwinSaveDialog();
                if (!filename.empty()) {
                    // Save to this file...
                }
                std::cout << "Save File action triggered\n";
                // TODO: save current scene/project
                break;

            case 2:
                filename = GLwinSaveDialog();
                if (!filename.empty()) {
                    // Save to this file...
                }
                std::cout << "Save As action triggered\n";
                break;

            case 3:
                std::cout << "Run action triggered\n";
                break;

            case 4:
                std::cout << "Stop action triggered\n";
                break;
            }

            // Reset so it doesn’t keep firing every frame
            SpxGui::gActiveTool = -1;
        }
    }

	// ----------------------------- Text Menu Bar Section ---------------------------------------
    
    inline void RenderMenuBar() {
        // Draw the bar at the top of the CLIENT area
        // (Your DrawRect uses client coords; that's fine for visuals.)
        DrawRect(0, 0, gScreenW, gMenuBarHeight, 0.15f, 0.15f, 0.17f);
        DrawRect(0, gMenuBarHeight - 1, gScreenW, 1, 0.05f, 0.05f, 0.05f);	

		// Load once ---------------------- Tool Bar Icon ----------------------
        
        
        // Make sure toolbar exists & textures are loaded (runs once)
        EnsureToolbar(); // bottom row

        // Draw toolbar (left side) bottom row
        RenderToolbar(6.0f, 30.0f, gMenuBarHeight - 45.0f);       

		// --------------------------------- End Tool Bar Icon ----------------------

        float iconSizeX = gIconSizeX - 8;
        float iconSizeY = gIconSizeY - 8;
        float iconY = 4.0f; // leave some space from top
        float iconX = 4.0f; // leave space for icon
		// Load icon if not already
        if (iconTexID == 0) {
            Image iconImg; // LoadTextuer
            iconTexID = LoadTextuer("../SPLEditor/Icon/icon_main_01.jpg", iconImg);
        }
        // Draw image if loaded
        if (iconTexID != 0) {
            DrawImage(iconTexID, iconX, iconY, iconSizeX, iconSizeY);
        }  

		// --------------- Close, Maximize, Minimize buttons -----
        float cr = 0.22f, cg = 0.22f, cb = 0.24f;
        float closeX = gScreenW - closebtnSize - 4.0f; // align right
       // float maxX = gScreenW - closebtnSize * 2 - 4.0f;
        float maxX = gScreenW - closebtnSize * 2 - 12.0f;
		//float minX = gScreenW - closebtnSize * 3 - 4.0f;
		float minX = gScreenW - closebtnSize * 3 - 20.0f;

        bool closeHover = (gMouseX >= closeX && gMouseX <= closeX + closebtnSize &&
            gMouseY >= iconY && gMouseY <= iconY + closebtnSize);
        // ------------------------ EXIT BUTTON --------------------
        if (iconCloseTexID == 0) {
            Image iconImg; // LoadTextuer
            iconCloseTexID = LoadTextuer("../SPLEditor/Icon/Exit.jpg", iconImg);

        }
            
        if (closeHover) { cr = 0.22f; cg = 0.22f; cb = 0.9f; } // brighter on hover
               DrawRect(closeX, iconY, closebtnSize + 2, closebtnSize + 2, cr, cg, cb);
            // Draw image if loaded
            if (iconCloseTexID != 0) {
                DrawImage(iconCloseTexID, gScreenW - closebtnSize - 4.0f, iconY, closebtnSize, closebtnSize);
                               
                if (closeHover && pressed) {
                       GLwinWindowShouldClose(gMainWindow, 1);
                }
            }
        // ------------------------ EXIT BUTTON -----------------------

		// ------------------------ MAXIMIZE BUTTON --------------------
         bool maxHover = (gMouseX >= maxX && gMouseX <= maxX + closebtnSize &&
                gMouseY >= iconY && gMouseY <= iconY + closebtnSize);
        if (iconMaxTexID == 0) {
            Image iconImg; // LoadTextuer
            iconMaxTexID = LoadTextuer("../SPLEditor/Icon/Max.jpg", iconImg);
        }
        
        if (maxHover) { cr = 0.22f; cg = 0.22f; cb = 0.9f; } // brighter on hover
        DrawRect(maxX, iconY, closebtnSize + 2, closebtnSize + 2, cr, cg, cb);
        // Draw image if loaded
        if (iconMaxTexID != 0) {
            DrawImage(iconMaxTexID, gScreenW - closebtnSize * 2 - 12.0f, iconY, 30, 30);
            if (maxHover && gMousePressed) {
                          // Toggle maximize / restore
                          static bool maximized = false;
                          if (maximized) {
                              GLwinRestoreWindow(gMainWindow); // you’ll need to implement this
                          }
                          else {
                              GLwinMaximizeWindow(gMainWindow); // you’ll need to implement this
                          }
                          maximized = !maximized;
            }
        }
		// ------------------------ END MAXIMIZE BUTTON --------------------

		// ------------------------ MINIMIZE BUTTON ------------------------
        bool minHover = (gMouseX >= maxX && gMouseX <= maxX + closebtnSize &&
            gMouseY >= iconY && gMouseY <= iconY + closebtnSize);
        if (iconMinTexID == 0) {
            Image iconImg; // LoadTextuer
            iconMinTexID = LoadTextuer("../SPLEditor/Icon/Minus.jpg", iconImg);
            
        }
        if (minHover) { cr = 0.22f; cg = 0.22f; cb = 0.9f; } // brighter on hover
       // DrawRect(minX, iconY, closebtnSize + 2, closebtnSize + 2, cr, cg, cb);
        DrawRect(minX, iconY, closebtnSize + 2, closebtnSize + 2, cr, cg, cb);
        // Draw image if loaded
        if (iconMaxTexID != 0) {
            DrawImage(iconMinTexID, gScreenW - closebtnSize * 3 - 20.0f, iconY, 30, 30);

            if (minHover && gMousePressed) {
                GLwinMinimizeWindow(gMainWindow); // you’ll need to implement this
            }
        }
		// ------------------------ END MINIMIZE BUTTON --------------------
		

        // --- Close button ---
  //      float btnSize = gTitleButton - 8.0f; // padding from top/bottom
  //      float btnY = 4.0f;

        //float cr = 0.15f, cg = 0.15f, cb = 0.17f;
        //float closeX = gScreenW - btnSize - 4.0f; // align right
        //bool closeHover = (gMouseX >= closeX && gMouseX <= closeX + btnSize &&
        //    gMouseY >= btnY && gMouseY <= btnY + btnSize);
        //if (closeHover) { cr = 0.22f; cg = 0.22f; cb = 0.24f; } // brighter on hover
        //DrawRect(closeX, btnY, btnSize, btnSize, cr, cg, cb);
        //DrawText(closeX + btnSize * 0.3f, btnY + 2, "X", 1.0f, 1.0f, 1.0f);

		//if (closeHover && pressed) {
  //          GLwinWindowShouldClose(gMainWindow, 1);
		//}
  //      // maximize / restore
  //      float maxX = closeX - btnSize - 4.0f;
        //bool maxHover = (gMouseX >= maxX && gMouseX <= maxX + btnSize &&
        //    gMouseY >= btnY && gMouseY <= btnY + btnSize);
        //if (maxHover) { cr = 0.22f; cg = 0.22f; cb = 0.24f; } // brighter on hover
        //DrawRect(maxX, btnY, btnSize, btnSize, cr, cg, cb);
        //DrawText(maxX + btnSize * 0.25f, btnY + 2, "[ ]", 1.0f, 1.0f, 1.0f);

  //      if (maxHover && gMousePressed) {
  //          // Toggle maximize / restore
  //          static bool maximized = false;
  //          if (maximized) {
  //              GLwinRestoreWindow(gMainWindow); // you’ll need to implement this
  //          }
  //          else {
  //              GLwinMaximizeWindow(gMainWindow); // you’ll need to implement this
  //          }
  //          maximized = !maximized;
  //      }

  //      // Minimize button (leftmost)
  //      float minX = maxX - btnSize - 4.0f;
  //      bool minHover = (gMouseX >= minX && gMouseX <= minX + btnSize &&
  //          gMouseY >= btnY && gMouseY <= btnY + btnSize);
  //      if (minHover) { cr = 0.22f; cg = 0.22f; cb = 0.24f; } // brighter on hover
  //      DrawRect(minX, btnY, btnSize, btnSize, cr, cg, cb);
  //      DrawText(minX + btnSize * 0.3f, btnY + 2, "_", 1, 1, 1);

  //      if (minHover && gMousePressed) {
  //          GLwinMinimizeWindow(gMainWindow); // you’ll need to implement this
  //      }
  //     


        // ------ Hover check in SCREEN coordinates over the actual client area ------
        // Client origin in screen coords was updated in main: gClientScreenX/Y
        int barLeft = gClientScreenX;
        int barTop = gClientScreenY;
        int barRight = gClientScreenX + gScreenW;       // use framebuffer/client width
        int barBottom = gClientScreenY + (int)gMenuBarHeight;

        bool hover = (gMouseGlobalX >= barLeft && gMouseGlobalX <= barRight &&
            gMouseGlobalY >= barTop && gMouseGlobalY <= barBottom);

        // ------ Drag start ------
        if (hover && pressed && !gMenuBarDragging) {
            gMenuBarDragging = true;

            dragStartMouseX = gMouseGlobalX;
            dragStartMouseY = gMouseGlobalY;

            GLwinGetWindowPos(gMainWindow, &dragStartWinX, &dragStartWinY);

            // std::cout << ">>> Drag START: win=(" << dragStartWinX << "," << dragStartWinY
            //           << ") mouse=(" << dragStartMouseX << "," << dragStartMouseY << ")\n";
        }

        // ------ Drag end ------
        if (released) {
            gMenuBarDragging = false;
        }

        // ------ Drag move ------
        if (gMenuBarDragging && down) {
            int deltaX = gMouseGlobalX - dragStartMouseX;
            int deltaY = gMouseGlobalY - dragStartMouseY;

            int newX = dragStartWinX + deltaX;
            int newY = dragStartWinY + deltaY;

            newX = Clamp(newX, 0, gScreenW);
            newY = Clamp(newY, 0, gScreenH);

            GLwinSetWindowPos(gMainWindow, newX, newY);

            
        }
    }
        
    inline void UpdateScreenSize(int w, int h) {
        gScreenW = static_cast<float>(w);
        gScreenH = static_cast<float>(h);
    }
	// --------------------------------------------- Tree view stuff ----------------------------------------------
    inline Context g;   

    inline void OpenFileInTab(const std::string& path) {
        // Prevent duplicates
        for (size_t i = 0; i < gOpenFiles.size(); i++) {
            if (gOpenFiles[i].path == path) {
                gActiveTab = (int)i;
                return;
            }
        }

        OpenFile file;
        file.path = path;

        // extract filename only (for tab title)
        size_t slash = path.find_last_of("/\\");
        file.name = (slash == std::string::npos) ? path : path.substr(slash + 1);

        // load file contents
        std::ifstream in(path);
        if (in) {
            std::string content((std::istreambuf_iterator<char>(in)),
                std::istreambuf_iterator<char>());
            file.buffer = content;   // assign directly
        }
        
        gOpenFiles.push_back(std::move(file));
        gActiveTab = (int)gOpenFiles.size() - 1;
    }
   
	// Load directory structure into SpxGuiTreeView
    inline SpxGuiTreeView LoadDirectory(const std::string& path) {
        SpxGuiTreeView root;
        root.name = path;
        root.fullPath = path;
        root.isDir = true;

        for (auto& entry : fs::directory_iterator(path)) {
            SpxGuiTreeView node;
            node.name = entry.path().filename().string();
            node.fullPath = entry.path().string();
            node.isDir = entry.is_directory();
            if (node.isDir) {
                // Load empty for now, expand lazily
            }
            root.children.push_back(node);
        }

        return root;
    }

    inline  static std::vector<char> textBuffer;
    // jump up 243
    inline void DrawFileNode(SpxGuiTreeView& node, int indent = 0) {
        float x = gCurrent->cursorX + indent * 16; // indent spacing
		float y = gCurrent->cursorY += g.fontSize - 20; // adjust for font size spacing between items

        // Draw triangle indicator for folders
        if (node.isDir) {
            const char* arrow = node.expanded ? "-" : "+";// "▼" : "▶";// "+" : "-";
            gCurrent->drawList.push_back(DrawCmd(DrawCmd::TEXT, x, y + 20, 1, 0.0f, 0.5f, arrow)); 
        }                                                                                             
       
        // Label (use your GUI button)
        bool clicked = ButtonNew(node.name.c_str(), 200 - indent * 16, 20);
        if (clicked) {
            if (node.isDir) {
                node.expanded = !node.expanded;
                if (node.expanded && node.children.empty()) {
                    // Lazy load children
                    for (auto& entry : fs::directory_iterator(node.fullPath)) {
                        SpxGuiTreeView child;
                        child.name = entry.path().filename().string();
                        child.fullPath = entry.path().string();
                        child.isDir = entry.is_directory();
                        node.children.push_back(child);
                    }
                }
            }
            else {
                // File clicked → later open in new tab  OpenFileInTab(clickedPath);
                std::cout << "Open file: " << node.fullPath << "\n";

                if (!node.isDir && clicked) {  // user clicked a file
                    std::string path = node.fullPath;

                    // Very simple extension check
                    if (path.ends_with(".txt") || path.ends_with(".spl") || path.ends_with(".splh")) {
						OpenFileInTab(path);
                        
                    }
                }



            }
        }

        gCurrent->cursorY += 20;

        // Recurse children
        if (node.isDir && node.expanded) {
            for (auto& child : node.children) {
                DrawFileNode(child, indent + 1);
            }
        }
    }

	// ------------------------------------------------- Text Input  ----------------------------------------------
   
    inline void AddInputChar(unsigned int c) {

        if (c >= 32 && c < 127) { // only printable ASCII for now
            gInputChars.push_back((char)c);
        }
        if (c == 8) { // backspace
            gInputChars.push_back((char)c);
        }
		if (c == 13){ // enter key
            // We only support ASCII in this UI right now
            gInputChars.push_back('\n');
        }
    }
    inline void AddKeyPress(int key) {
        // If no control has focus, ignore
        if (activeTextID == 0) return;

        // Determine whether the focused control is a legacy C buffer (activeBuf)
        // or a std::string (encoded in activeTextID).
        std::string* sBuf = nullptr;
        if (!activeBuf) {
            sBuf = reinterpret_cast<std::string*>(activeTextID);
        }

        int len = 0;
        if (activeBuf) len = (int)strlen(activeBuf);
        else if (sBuf) len = (int)sBuf->size();

        switch (key) {
        case GLWIN_LEFT:
            // For single-line (activeBuf) we immediately update caretIndex so InputText sees it.
            if (activeBuf) {
                if (caretIndex > 0) caretIndex--;
            }
            SpxGui::gInputKeys.push_back(key);
            break;
        case GLWIN_RIGHT:
            if (activeBuf) {
                if (caretIndex < len) caretIndex++;
            }
            SpxGui::gInputKeys.push_back(key);
            break;
        case GLWIN_UP:
            // Up/Down are handled by multiline widgets; just enqueue the key.
            SpxGui::gInputKeys.push_back(key);
            break;
        case GLWIN_DOWN:
            SpxGui::gInputKeys.push_back(key);
            break;
        case GLWIN_HOME:
            if (activeBuf) caretIndex = 0;
            SpxGui::gInputKeys.push_back(key);
            break;
        case GLWIN_END:
            if (activeBuf) caretIndex = len;
            SpxGui::gInputKeys.push_back(key);
            break;
        case GLWIN_DELETE:
            // For single-line C buffers perform the deletion immediately because InputText
            // does not consume gInputKeys for delete. For std::string (multiline) we leave
            // the actual deletion to the widget which consumes gInputKeys.
            if (activeBuf) {
                if (caretIndex < len) {
                    memmove(activeBuf + caretIndex,
                        activeBuf + caretIndex + 1,
                        len - caretIndex);
                }
            }
            SpxGui::gInputKeys.push_back(key);
            break;
        default:
            // Other special keys can be enqueued for widgets to handle
            SpxGui::gInputKeys.push_back(key);
            break;
        }
    }

  //  inline void AddKeyPress(int key) {
  //      //if (!activeBuf) return; // no active input
  //      //if (activeBuf == 0) return; // no active input
		//if (activeTextID == 0) return; // no active input

  //      switch (key) {
  //      case GLWIN_LEFT:
  //          if (SpxGui::caretIndex > 0) caretIndex--;
  //          SpxGui::gInputKeys.push_back(key);
  //          break;
  //      case GLWIN_RIGHT: {
  //          int len = (int)strlen(activeBuf);
  //          if (caretIndex < len) caretIndex++;
  //          SpxGui::gInputKeys.push_back(key);
  //          break;
  //      }
		//case GLWIN_UP:
  //          SpxGui::gInputKeys.push_back(key);
  //          break;
		//case GLWIN_DOWN:
  //          SpxGui::gInputKeys.push_back(key);
  //          break;
  //      case GLWIN_HOME:
  //          caretIndex = 0;
  //          SpxGui::gInputKeys.push_back(key);
  //          break;
  //      case GLWIN_END:
  //          caretIndex = (int)strlen(activeBuf);
  //          SpxGui::gInputKeys.push_back(key);
  //          break;
  //      }
  //  }

    // Default callbacks for GLwin integration
    inline void CharCallback(unsigned int codepoint) {
        SpxGui::AddInputChar(codepoint);
    }
    
    inline void KeyCallback(int key, int action) {
        if (action == GLWIN_PRESS) {
            SpxGui::AddKeyPress(key);

        }
         if (key == GLWIN_RETURN) {
            SpxGui::AddInputChar('\n');   // inject newline into text buffer

         }
    }
   
	// ----------------------------------------------------------- Test Area -----------------------------------------------------------

	// Use this for X and Y coordinates
	struct SpxVec2 {
		float                                   x, y;
		constexpr SpxVec2()                     : x(0), y(0) {}
		constexpr SpxVec2(float _x, float _y)   : x(_x), y(_y) {}
		float& operator[](size_t idx) {
			if (idx == 0) return x;
			else if (idx == 1) return y;
			throw std::out_of_range("Index out of range for SpxVec2");
		}
        const float& operator[](size_t idx) const {
            if (idx == 0) return x;
            else if (idx == 1) return y;
            throw std::out_of_range("Index out of range for SpxVec2");
        }

    };
	// Use this for RGB colors
	struct SpxVec3 {
	};
	// Use this for RGBA colors and XYWH coordinates
	struct SpxVec4 {
	};
        
	
	// ----------------------------------------------------------- End Test Area -----------------------------------------------------------
    	
   
	// Global variables for SpxGui Shader
    inline GLuint gShader = 0;
    inline GLint uScreenSizeLoc, uColorLoc;

    inline GLuint CompileShader(GLenum type, const char* src) {
        GLuint s = glCreateShader(type);
        glShaderSource(s, 1, &src, NULL);
        glCompileShader(s);

        GLint success;
        glGetShaderiv(s, GL_COMPILE_STATUS, &success);
        if (!success) {
            char log[512];
            glGetShaderInfoLog(s, 512, NULL, log);
            std::cerr << "Shader compile error: " << log << std::endl;
        }
        return s;
    }
	// ----------------------------------------------------- Font & Text Rendering -----------------------------------------------------
      

    
    float CalcTextWidthN(const char* text, int count) {
        float xpos = 0, ypos = 0;
        for (int i = 0; i < count && text[i]; i++) {
            unsigned char c = text[i];
            if (c < 32 || c >= 128) continue;
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(g.cdata, 512, 512, c - 32, &xpos, &ypos, &q, 1);
        }
        return xpos;
    }

    inline float CalcTextWidth(const char* text) {
       return CalcTextWidthN(text, (int)strlen(text));
   }

    inline bool LoadDefaultFont(const char* path, float size) {
        FILE* f = nullptr;
        errno_t err = fopen_s(&f, path, "rb");
        if (err != 0 || !f) {
            std::cerr << "Failed to open font file: " << path << std::endl;
            return false;
        }

        std::vector<unsigned char> ttfBuffer(1 << 20);
        size_t readBytes = fread(ttfBuffer.data(), 1, ttfBuffer.size(), f);
        fclose(f);
        if (readBytes == 0) {
            std::cerr << "Failed to read font file!\n";
            return false;
        }

        std::vector<unsigned char> tempBitmap(512 * 512);

        int result = stbtt_BakeFontBitmap(
            ttfBuffer.data(), 0, size,
            tempBitmap.data(), 512, 512,
            32, 96, g.cdata
        );

        if (result <= 0) {
            std::cerr << "stbtt_BakeFontBitmap failed!\n";
            return false;
        }

        if (g.fontTex) {
            glDeleteTextures(1, &g.fontTex);
        }

        glGenTextures(1, &g.fontTex);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glBindTexture(GL_TEXTURE_2D, g.fontTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 512, 512, 0,
            GL_RED, GL_UNSIGNED_BYTE, tempBitmap.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        g.fontSize = size;
        return true;
    }

    inline void SetScreenSize(int fbw, int fbh) {
        g.screenW = fbw;
        g.screenH = fbh;
    }

    //inline void Init(int screenW, int screenH) {
    inline void Init() {
    
        std::cout << "[SpxGui::Init] Initializing " << std::endl;

        // --- Compile & link shader ---
        GLuint vs = CompileShader(GL_VERTEX_SHADER, vertexSrc);
        GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fragmentSrc);

        gShader = glCreateProgram();
        glAttachShader(gShader, vs);
        glAttachShader(gShader, fs);
        glLinkProgram(gShader);

        GLint linked;
        glGetProgramiv(gShader, GL_LINK_STATUS, &linked);
        if (!linked) {
            char log[512];
            glGetProgramInfoLog(gShader, 512, NULL, log);
            std::cerr << "Shader link error: " << log << std::endl;
        }
        glDeleteShader(vs);
        glDeleteShader(fs);

        uScreenSizeLoc = glGetUniformLocation(gShader, "uScreenSize");
        uColorLoc = glGetUniformLocation(gShader, "uColor");
        std::cout << "uScreenSizeLoc=" << uScreenSizeLoc
            << " uColorLoc=" << uColorLoc << std::endl;

        // --- Load default font ---
        LoadDefaultFont("C:/Windows/Fonts/arial.ttf", 16.0f);

        
        // tell shader which texture unit the font atlas is bound to
        glUseProgram(gShader);
        glUniform1i(glGetUniformLocation(gShader, "uTex"), 0);

		
    }
	// ------------------------------------------------ Images ------------------------------------------------
    // global texture cache load texture only once
	inline std::unordered_map<std::string, Image> GetTextureCache; 
    //GLuint textureID;
        //int width; int height; int ColIndex; // later for multiple colors in one texture
        //int i_formatted; // later for different image formats
    
    inline unsigned int LoadTextuer(const std::string& filePath, Image& img) {
		// check if texture already loaded
		auto it = GetTextureCache.find(filePath);
		if (it != GetTextureCache.end()) {
			img = it->second;
			return img.textureID; // return existing texture ID
		}

		glGenTextures(1, &img.textureID); // Generate a texture ID
        // sets pixel storage modes that affect the operation of subsequent
        // glReadPixels as well as the unpacking of texture patterns
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Add this for safety
		// load and generate the texture
		unsigned char* data = stbi_load(filePath.c_str(), &img.width, &img.height, &img.ColIndex, 0);
        if (data) {
          
			GLWIN_LOG_INFO("Loaded texture: " << filePath
				<< " (" << img.width << " x " << img.height
				<< ", channels= " << img.ColIndex << ")");
        
            if (img.ColIndex == 4) // RGBA png
                img.i_format = GL_RGBA;
            else if (img.ColIndex == 3) // JPG RGB
                img.i_format = GL_RGB;
            else if (img.ColIndex == 1) // greyscale
                img.i_format = GL_RED;
            else
                std::cerr << "Failed to load image (Unknown format): " << filePath << std::endl;

			glBindTexture(GL_TEXTURE_2D, img.textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, img.i_format, img.width, img.height, 0, img.i_format, GL_UNSIGNED_BYTE, data);
            // set the texture wrapping/filtering options (on the currently bound texture object)
			glGenerateMipmap(GL_TEXTURE_2D); // Generate mipmaps "Esential"
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data); // Free image data after uploading to GPU
        }
        else {
			std::cerr << "Failed to load image: " << filePath << std::endl;
			stbi_image_free(data);
        }
        std::cout << "Loading textureID: " << img.textureID << std::endl;
		// save to cache
		GetTextureCache[filePath] = img;
		return img.textureID;
    }

    inline void DrawText(float x, float y, const char* title, float r, float gcol, float b) {
        if (!g.fontTex) return;

        glUseProgram(gShader);
        glUniform1i(glGetUniformLocation(gShader, "useTex"), 1); // textured
        glUniform2f(uScreenSizeLoc, (float)g.screenW, (float)g.screenH);
        glUniform3f(uColorLoc, r, gcol, b);

        glBindTexture(GL_TEXTURE_2D, g.fontTex);

        float xpos = x;
        float ypos = y + g.fontSize;

        GLuint vao, vbo;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        std::vector<float> verts;

        for (const char* p = title; *p; p++) {
            if (*p < 32 || *p >= 128) continue;
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(g.cdata, 512, 512, *p - 32, &xpos, &ypos, &q, 1);

			float quad[] = { // 6 vertices, each with pos(2) and uv(2)
                q.x0, q.y0, q.s0, q.t0,
                q.x1, q.y0, q.s1, q.t0,
                q.x1, q.y1, q.s1, q.t1,

                q.x0, q.y0, q.s0, q.t0,
                q.x1, q.y1, q.s1, q.t1,
                q.x0, q.y1, q.s0, q.t1
            };
            verts.insert(verts.end(), quad, quad + 24);
        }

        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_DYNAMIC_DRAW);

        // position(2), uv(2)
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glDrawArrays(GL_TRIANGLES, 0, verts.size() / 4);

        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);

    }
    void Init(int screenW, int screenH)
    {
    }


	// needs to be after all widgets are drawn in main
   
    inline void NewFrame(float mouseX, float mouseY, bool down, bool pressed, bool released, int fbw, int fbh) {
   // inline void NewFrame(bool down, bool pressed, bool released, int fbw, int fbh) {
        gInputChars.clear(); // clear input chars each frame
        gInputKeys.clear(); // clear input keys each frame
		g.frameCount++;

        gMouseX = mouseX;
		gMouseY = mouseY;
		gMouseDown = down;
		gMousePressed = pressed;
		gMouseReleased = released;
       
		gScreenW = fbw;
		gScreenH = fbh;

        for (auto& w : gWindows) { 
			//w.inWindow = false; // reset each frame
           w.mouseX = mouseX;
           w.mouseY = mouseY;
           w.mouseDown = down;
           w.mousePressed = pressed;
           w.mouseReleased = released;

           /* w.mouseDown = gMouseDown;
            w.mousePressed = gMousePressed;
            w.mouseReleased = gMouseReleased;*/
        }
		//gInputChars.clear(); // clear input chars each frame
		//gInputKeys.clear(); // clear input keys each frame
    }
	Style gStyle; // global style instance

	// main widget window functions
    inline void Begin(const char* title, bool* p_open = nullptr, int SpxGuiWinID = 0) {
        // find or create window
        auto it = std::find_if(gWindows.begin(), gWindows.end(),
            [&](const SpxGuiWindow& w) { return w.SpxGuiWinID == SpxGuiWinID; });

        if (it == gWindows.end()) {
            gWindows.push_back(SpxGuiWindow());
            gWindows.back().SpxGuiWinID = SpxGuiWinID;
            gWindows.back().title = title;
            gWindows.back().open = p_open;
            gCurrent = &gWindows.back();
        }
        else {
            gCurrent = &(*it);
        }

        // if window closed, skip
        if (p_open && !*p_open) {
            gCurrent = nullptr;
            return;
        }

        // mark as used this frame
        gCurrent->inWindow = true;
        gCurrent->title = title;

        // reset layout cursor
        gCurrent->cursorX = gCurrent->curWinX + gStyle.WindowPaddingX;
        gCurrent->cursorY = gCurrent->curWinY + gStyle.WindowPaddingY;

        // check header region
        bool overHeader =
            (gCurrent->mouseX >= gCurrent->curWinX && gCurrent->mouseX <= gCurrent->curWinX + gCurrent->curWinW &&
                gCurrent->mouseY >= gCurrent->curWinY && gCurrent->mouseY <= gCurrent->curWinY + gCurrent->headerHeight);

        // focus / bring to front
        if (overHeader && gCurrent->mousePressed) {
            gActiveWinID = gCurrent->SpxGuiWinID;

            // bring to front
            auto it2 = std::find_if(gWindows.begin(), gWindows.end(),
                [&](const SpxGuiWindow& w) { return w.SpxGuiWinID == gActiveWinID; });
            if (it2 != gWindows.end()) {
                SpxGuiWindow temp = *it2;
                gWindows.erase(it2);
                gWindows.push_back(temp);
                gCurrent = &gWindows.back();
            }

            // start dragging
            gCurrent->dragging = true;
            gCurrent->dragOffsetX = gCurrent->mouseX - gCurrent->curWinX;
            gCurrent->dragOffsetY = gCurrent->mouseY - gCurrent->curWinY;
        }

        // stop dragging
        if (gCurrent->mouseReleased) {
            gCurrent->dragging = false;
        }

        // apply dragging movement
        if (gCurrent->dragging && gCurrent->mouseDown) {
            gCurrent->curWinX = gCurrent->mouseX - gCurrent->dragOffsetX;
            gCurrent->curWinY = gCurrent->mouseY - gCurrent->dragOffsetY;
        }
       // gCurrent->cursorY = gCurrent->curWinY + gStyle.WindowPaddingY + gMenuBarHeight;

    }


	// End the current window
    inline void End() {
        if (!gCurrent) return;
        gCurrent->inWindow = false;
		gCurrent = nullptr;
        //std::cout << "[End Window]" << std::endl;
    }

    // Drawing functions for a popup-style window (no X button)
    inline void BeginPopUp(const char* title, bool* p_open = nullptr, int SpxGuiWinID = 0) {
        // find or create
        auto it = std::find_if(gWindows.begin(), gWindows.end(),
            [&](const SpxGuiWindow& w) { return w.SpxGuiWinID == SpxGuiWinID; });

        if (it == gWindows.end()) {
            gWindows.push_back(SpxGuiWindow());
            gWindows.back().SpxGuiWinID = SpxGuiWinID;
            gWindows.back().title = title;
            gWindows.back().open = p_open;
            gWindows.back().isPopup = true;   // mark as popup
            gCurrent = &gWindows.back();
        }
        else {
			// Set current popup window to be the top window
			SpxGuiWindow temp = *it;
			gWindows.erase(it);
			gWindows.push_back(temp);
			gCurrent = &gWindows.back();
		}
			

        if (p_open && !*p_open) {
            gCurrent = nullptr;
            return;
        }

        gCurrent->inWindow = true;
        gCurrent->title = title;

        gCurrent->cursorX = gCurrent->curWinX + gStyle.WindowPaddingX;
        gCurrent->cursorY = gCurrent->curWinY + gStyle.WindowPaddingY;

        //  close if clicked outside popup bounds
        if (gCurrent->mousePressed) {
            bool inside =
                (gCurrent->mouseX >= gCurrent->curWinX &&
                    gCurrent->mouseX <= gCurrent->curWinX + gCurrent->curWinW &&
                    gCurrent->mouseY >= gCurrent->curWinY &&
                    gCurrent->mouseY <= gCurrent->curWinY + gCurrent->curWinH);

            if (!inside) {
                if (p_open) *p_open = false;   // close
            }
        }

        // same dragging code as before…
        bool overHeader =
            (gCurrent->mouseX >= gCurrent->curWinX && gCurrent->mouseX <= gCurrent->curWinX + gCurrent->curWinW &&
                gCurrent->mouseY >= gCurrent->curWinY && gCurrent->mouseY <= gCurrent->curWinY + gCurrent->headerHeight);

        if (overHeader && gCurrent->mousePressed) {
            gActiveWinID = gCurrent->SpxGuiWinID;
            auto it2 = std::find_if(gWindows.begin(), gWindows.end(),
                [&](const SpxGuiWindow& w) { return w.SpxGuiWinID == gActiveWinID; });
            if (it2 != gWindows.end()) {
                SpxGuiWindow temp = *it2;
                gWindows.erase(it2);
                gWindows.push_back(temp);
                gCurrent = &gWindows.back();
            }

            gCurrent->dragging = true;
            gCurrent->dragOffsetX = gCurrent->mouseX - gCurrent->curWinX;
            gCurrent->dragOffsetY = gCurrent->mouseY - gCurrent->curWinY;
        }
        if (gCurrent->mouseReleased) gCurrent->dragging = false;
        if (gCurrent->dragging && gCurrent->mouseDown) {
            gCurrent->curWinX = gCurrent->mouseX - gCurrent->dragOffsetX;
            gCurrent->curWinY = gCurrent->mouseY - gCurrent->dragOffsetY;
        }
    }

    inline void EndPopUp() {
        if (!gCurrent) return;
        gCurrent->inWindow = false;
        gCurrent = nullptr;
    }
	// Color Popup
    // Drawing functions for a popup-style window (no X button)
    inline void BeginColPopUp(const char* title, bool* p_open = nullptr, int SpxGuiWinID = 0) {
   
        // find or create
        auto it = std::find_if(gWindows.begin(), gWindows.end(),
            [&](const SpxGuiWindow& w) { return w.SpxGuiWinID == SpxGuiWinID; });

        if (it == gWindows.end()) {
            gWindows.push_back(SpxGuiWindow());
            gWindows.back().SpxGuiWinID = SpxGuiWinID;
            gWindows.back().title = title;
            gWindows.back().open = p_open;
            gWindows.back().isPopup = true;   // mark as popup
            gCurrent = &gWindows.back();
        }
        else {
            // Set current popup window to be the top window
            SpxGuiWindow temp = *it;
            gWindows.erase(it);
            gWindows.push_back(temp);
            gCurrent = &gWindows.back();
        }


        if (p_open && !*p_open) {
            gCurrent = nullptr;
            return;
        }

        gCurrent->inWindow = true;
        gCurrent->title = title;

        gCurrent->cursorX = gCurrent->curWinX + gStyle.WindowPaddingX;
        gCurrent->cursorY = gCurrent->curWinY + gStyle.WindowPaddingY;

        //  close if clicked outside popup bounds
        if (gCurrent->mousePressed) {
            bool inside =
                (gCurrent->mouseX >= gCurrent->curWinX &&
                    gCurrent->mouseX <= gCurrent->curWinX + gCurrent->curWinW &&
                    gCurrent->mouseY >= gCurrent->curWinY &&
                    gCurrent->mouseY <= gCurrent->curWinY + gCurrent->curWinH);

            if (!inside) {
                if (p_open) *p_open = false;   // close
            }
        }

        // same dragging code as before…
        bool overHeader =
            (gCurrent->mouseX >= gCurrent->curWinX && gCurrent->mouseX <= gCurrent->curWinX + gCurrent->curWinW &&
                gCurrent->mouseY >= gCurrent->curWinY && gCurrent->mouseY <= gCurrent->curWinY + gCurrent->headerHeight);

        if (overHeader && gCurrent->mousePressed) {
            gActiveWinID = gCurrent->SpxGuiWinID;
            auto it2 = std::find_if(gWindows.begin(), gWindows.end(),
                [&](const SpxGuiWindow& w) { return w.SpxGuiWinID == gActiveWinID; });
            if (it2 != gWindows.end()) {
                SpxGuiWindow temp = *it2;
                gWindows.erase(it2);
                gWindows.push_back(temp);
                gCurrent = &gWindows.back();
            }

            gCurrent->dragging = true;
            gCurrent->dragOffsetX = gCurrent->mouseX - gCurrent->curWinX;
            gCurrent->dragOffsetY = gCurrent->mouseY - gCurrent->curWinY;
        }
        if (gCurrent->mouseReleased) gCurrent->dragging = false;
        if (gCurrent->dragging && gCurrent->mouseDown) {
            gCurrent->curWinX = gCurrent->mouseX - gCurrent->dragOffsetX;
            gCurrent->curWinY = gCurrent->mouseY - gCurrent->dragOffsetY;
        }
    }

    inline void EndColPopUp() {
        if (!gCurrent) return;
        gCurrent->inWindow = false;
        gCurrent = nullptr;
    }

	// ----------------------------------------------------------- Tabs -----------------------------------------------------------

    //inline void BeginTabBar(const char* title) {
    inline bool BeginTabBar(const char* title) {
        if (!gCurrent) return false;
        gTabBar.title = title;
        gTabBar.tabs.clear();
        gTabBar.startX = gCurrent->cursorX;
        gTabBar.startY = gCurrent->cursorY;
        gTabBar.height = g.fontSize + gStyle.ItemSpacingY * 2;
        gTabBar.inTabBar = true;

        // move cursor below tabs
        gCurrent->cursorY += gTabBar.height + gStyle.ItemSpacingY;
		return true;
    }

    inline void EndTabBar() {
        gTabBar.inTabBar = false;
    }

    //inline bool BeginTabItem(const char* label) {
    //    if (!gCurrent || !gTabBar.inTabBar) return false;

    //    int idx = (int)gTabBar.tabs.size();
    //    gTabBar.tabs.push_back(label);

    //    // --- if no tab is active yet, activate the first one ---
    //    if (gTabBar.activeTabIndex < 0)
    //        gTabBar.activeTabIndex = 0;

    //    float tabW = 100.0f; // fixed width for now
    //    float x = gTabBar.startX + idx * (tabW + 2);
    //    float y = gTabBar.startY;

    //    bool hover = (gCurrent->mouseX >= x && gCurrent->mouseX <= x + tabW &&
    //        gCurrent->mouseY >= y && gCurrent->mouseY <= y + gTabBar.height);
    //    bool clicked = (hover && gCurrent->mousePressed);

    //    if (clicked) {
    //        gTabBar.activeTabIndex = idx;
    //    }

    //    // style
    //    float r = (idx == gTabBar.activeTabIndex) ? 0.3f : 0.2f;
    //    float gcol = (idx == gTabBar.activeTabIndex) ? 0.4f : 0.2f;
    //    float b = (idx == gTabBar.activeTabIndex) ? 0.6f : 0.2f;

    //    gCurrent->drawList.emplace_back(DrawCmd::RECT, x, y, tabW, gTabBar.height, r, gcol, b);
    //    gCurrent->drawList.emplace_back(DrawCmd::TEXT, x + 8, y + g.fontSize * 0.2f, 1, 1, 1, label);

    //    return (idx == gTabBar.activeTabIndex);
    //}

    inline bool BeginTabItem(const char* label) {
        if (!gCurrent || !gTabBar.inTabBar) return false;

        int idx = (int)gTabBar.tabs.size();
        gTabBar.tabs.push_back(label);

        float tabW = 100.0f; // fixed width for now
        float x = gTabBar.startX + idx * (tabW + 2);
        float y = gTabBar.startY;

        bool hover = (gCurrent->mouseX >= x && gCurrent->mouseX <= x + tabW &&
            gCurrent->mouseY >= y && gCurrent->mouseY <= y + gTabBar.height);
        bool clicked = (hover && gCurrent->mousePressed);

        if (clicked) {
            gTabBar.activeTabIndex = idx;
        }

        // style
        float r = (idx == gTabBar.activeTabIndex) ? 0.3f : 0.2f;
        float gcol = (idx == gTabBar.activeTabIndex) ? 0.4f : 0.2f;
        float b = (idx == gTabBar.activeTabIndex) ? 0.6f : 0.2f;

        gCurrent->drawList.emplace_back(DrawCmd::RECT, x, y, tabW, gTabBar.height, r, gcol, b);
        gCurrent->drawList.emplace_back(DrawCmd::TEXT, x + 8, y + g.fontSize * 0.2f, 1, 1, 1, label);

        // return true if this tab is the active one
        return (idx == gTabBar.activeTabIndex);
    }

    inline void EndTabItem() {
        // Nothing yet, just symmetry
    }

	// ------------------------------------------------------------ Tables ------------------------------------------------------------

    inline void BeginTable(const char* title, int column_count, float totalWidth = 0.0f) {
        if (!gCurrent) return;

        gCurrent->table.active = true;
        gCurrent->table.columnCount = column_count;
        gCurrent->table.currentColumn = 0;
        gCurrent->table.startX = gCurrent->cursorX;
        gCurrent->table.rowY = gCurrent->cursorY;

        // auto-fit column width if not set
        if (totalWidth <= 0.0f) {
            totalWidth = gCurrent->curWinW - 2 * gStyle.WindowPaddingX;
        }
        gCurrent->table.colWidth = totalWidth / column_count;

	}	
    //TableNextColumn
    inline void TableNextColumn() {
        if (!gCurrent || !gCurrent->table.active) return;

        gCurrent->table.currentColumn++;
        if (gCurrent->table.currentColumn >= gCurrent->table.columnCount) {
            // move to next row
            gCurrent->table.currentColumn = 0;
            gCurrent->cursorY = gCurrent->table.rowY + gCurrent->lastItemH + gStyle.ItemSpacingY;
            gCurrent->table.rowY = gCurrent->cursorY;
        }

        gCurrent->cursorX = gCurrent->table.startX + gCurrent->table.currentColumn * gCurrent->table.colWidth;
    }
	//TableNextRow
    inline void TableNextRow() {
        if (!gCurrent || !gCurrent->table.active) return;

        gCurrent->cursorY = gCurrent->table.rowY + gCurrent->lastItemH + gStyle.ItemSpacingY;
        gCurrent->table.rowY = gCurrent->cursorY;
        gCurrent->table.currentColumn = 0;
        gCurrent->cursorX = gCurrent->table.startX;
    }
    inline void EndTable() {
        if (!gCurrent || !gCurrent->table.active) return;
        gCurrent->table.active = false;

        // move cursor below the whole table
        gCurrent->cursorY = gCurrent->table.rowY + gCurrent->lastItemH + gStyle.ItemSpacingY;
	}   


    inline Style& GetStyle() { return style; }

    inline void DrawRect(float x, float y, float w, float h, float r, float g, float b) {
        float verts[] = {
            x,       y,
            x + w,   y,
            x + w,   y + h,

            x,       y,
            x + w,   y + h,
            x,       y + h
        };

        static GLuint vao = 0, vbo = 0;
        if (!vao) {
            glGenVertexArrays(1, &vao);
            glGenBuffers(1, &vbo);
        }

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);       

        glUseProgram(gShader);
        glUniform1i(glGetUniformLocation(gShader, "useTex"), 0); // solid
        glUniform2f(uScreenSizeLoc, SpxGui::g.screenW, SpxGui::g.screenH);
        glUniform3f(uColorLoc, r, g, b);

        glDrawArrays(GL_TRIANGLES, 0, 6);
      

        glUniform1i(glGetUniformLocation(gShader, "useTex"), 0); // reset

    }
	// use for drawing images in the gui
    inline void DrawImage(unsigned int texID, float x, float y, float w, float h) {
        glUseProgram(gShader);
        glUniform2f(uScreenSizeLoc, (float)g.screenW, (float)g.screenH);
        glUniform1i(glGetUniformLocation(gShader, "useTex"), 2);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texID);      
        glUniform1i(glGetUniformLocation(gShader, "uTex"), 0); // sampler = unit 0

        float verts[] = {
            // pos        // uv
            x,     y,     0.0f, 0.0f,
            x + w, y,     1.0f, 0.0f,
            x + w, y + h, 1.0f, 1.0f,

            x,     y,     0.0f, 0.0f,
            x + w, y + h, 1.0f, 1.0f,
            x,     y + h, 0.0f, 1.0f,
        };

        GLuint vao, vbo;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glDrawArrays(GL_TRIANGLES, 0, 6);      

        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);

        glUniform1i(glGetUniformLocation(gShader, "useTex"), 0); // reset
    }   

    inline void Render() {
        for (auto& w : gWindows) {
            if (w.open && !*w.open) continue;

            // 1. Window background
            DrawRect(w.curWinX, w.curWinY, w.curWinW, w.curWinH,
                style.WindowBgR, style.WindowBgG, style.WindowBgB);

            // 2. Header color logic
            bool isActive = (w.SpxGuiWinID == gActiveWinID);

            float hr = style.WindowTopBarR;
            float hg = style.WindowTopBarG;
            float hb = style.WindowTopBarB;

            if (w.isPopup) {
                // Popups: header matches body color
                hr = style.WindowBgR;
                hg = style.WindowBgG;
                hb = style.WindowBgB;
            }

            float bx = w.curWinX;
            float by = w.curWinY;
            float bw = w.curWinW;
            float bh = w.curWinH;

            // 1. Window background (image if set, otherwise solid)
            if (w.backgroundTex != 0) {
                DrawImage(w.backgroundTex, bx, by, bw, bh);
            }
            else {
                DrawRect(bx, by, bw, bh,
                    style.WindowBgR, style.WindowBgG, style.WindowBgB);
            }

            // Brighten header if active
            if (isActive) {
                hr = (hr + 0.1f > 1.0f) ? 1.0f : hr + 0.1f;
                hg = (hg + 0.1f > 1.0f) ? 1.0f : hg + 0.1f;
                hb = (hb + 0.4f > 1.0f) ? 1.0f : hb + 0.4f;
            }

            // Draw header bar
            DrawRect(w.curWinX, w.curWinY, w.curWinW, w.headerHeight, hr, hg, hb);

            // 3. Close button (normal windows only)
            if (!w.isPopup && w.open) {
                float btnSize = w.headerHeight - 6.0f;
                float btnX = w.curWinX + w.curWinW - btnSize - 4.0f;
                float btnY = w.curWinY + 3.0f;

                bool hover = (w.mouseX >= btnX && w.mouseX <= btnX + btnSize &&
                    w.mouseY >= btnY && w.mouseY <= btnY + btnSize);

                float cr = style.WindowTopButR;
                float cg = style.WindowTopButG;
                float cb = style.WindowTopButB;
                if (hover) { cr = 0.8f; cg = 0.2f; cb = 0.2f; }

                DrawRect(btnX, btnY, btnSize, btnSize, cr, cg, cb);
                DrawText(btnX + 4, btnY - 2, "X", 1, 1, 1);

                if (hover && w.mousePressed) {
                    *w.open = false;
                }
            }

            // 4. Title text
            DrawText(w.curWinX + 8, w.curWinY + 4, w.title.c_str(), 1, 1, 1);


            // 5. Border (3D effect: light top/left, dark bottom/right)
        // top
            DrawRect(bx, by, bw, 1, 0.8f, 0.8f, 0.8f);
            // left
            DrawRect(bx, by, 1, bh, 0.8f, 0.8f, 0.8f);
            // bottom
            DrawRect(bx, by + bh - 1, bw, 1, 0.1f, 0.1f, 0.1f);
            // right
            DrawRect(bx + bw - 1, by, 1, bh, 0.1f, 0.1f, 0.1f);

            // 5. Draw recorded widget commands
            for (auto& cmd : w.drawList) {
                switch (cmd.type) {
                case DrawCmd::RECT:
                    DrawRect(cmd.x, cmd.y, cmd.w, cmd.h, cmd.r, cmd.g, cmd.b);
                    break;
                case DrawCmd::TEXT:
                    DrawText(cmd.x, cmd.y, cmd.text.c_str(), cmd.r, cmd.g, cmd.b);
                    break;
                case DrawCmd::IMAGE:
                    DrawImage(cmd.texID, cmd.x, cmd.y, cmd.w, cmd.h);
                    break;
                case DrawCmd::CARET:
                    DrawRect(cmd.x, cmd.y, cmd.w, cmd.h, cmd.r, cmd.g, cmd.b);
                    break;
                }
            }

            // Clear drawList for next frame
            w.drawList.clear();
        }
    }

	// ---------------------------------------------------- Color Picker ----------------------------------------------------

    inline void HSVtoRGB(float h, float s, float v, float& r, float& g, float& b) {
        int i = int(h * 6.0f);
        float f = h * 6.0f - i;
        float p = v * (1.0f - s);
        float q = v * (1.0f - f * s);
        float t = v * (1.0f - (1.0f - f) * s);
        switch (i % 6) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
        }
    }


    
} // namespace SpxGui




