#pragma once
#include "SpxGui.h"
#include <unordered_map>

namespace SpxGui
{
	


	inline bool ButtonNew(const char* label, float w, float h) {
		if (!gCurrent) return false;

		float x = gCurrent->cursorX;
		float y = gCurrent->cursorY;

		bool hover = (gCurrent->mouseX >= x && gCurrent->mouseX <= x + w &&
			gCurrent->mouseY >= y && gCurrent->mouseY <= y + h);
		bool clicked = (hover && gCurrent->mousePressed);

		//float r = 0.3f, gcol = 0.3f, b = 0.3f;
		float r = 0.15f, gcol = 0.15f, b =  0.17f;
		if (hover) { r = 0.4f; gcol = 0.4f; b = 0.6f; }
		if (clicked) { r = 0.2f; gcol = 0.6f; b = 0.2f; }

		// background rect
		gCurrent->drawList.push_back({ DrawCmd::RECT, x, y, w, h, r, gcol, b });

		// text
		gCurrent->drawList.push_back(DrawCmd(DrawCmd::TEXT, x + 6, y + 4, 1, 1, 1, label));

		gCurrent->cursorY += h + gStyle.ItemSpacingY;
		gCurrent->lastItemW = w;
		gCurrent->lastItemH = h;

		return clicked;
	}
	
   
		inline bool Button(const char* label, float w, float h) {
			if (!gCurrent) return false;

			float x = gCurrent->cursorX;
			float y = gCurrent->cursorY;

			bool hover = (gCurrent->mouseX >= x && gCurrent->mouseX <= x + w &&
				gCurrent->mouseY >= y && gCurrent->mouseY <= y + h);
			bool clicked = (hover && gCurrent->mousePressed);

			float r = 0.3f, gcol = 0.3f, b = 0.3f;
			if (hover) { r = 0.4f; gcol = 0.4f; b = 0.6f; }
			if (clicked) { r = 0.2f; gcol = 0.6f; b = 0.2f; }

			// Push button rect
			gCurrent->drawList.emplace_back(DrawCmd::RECT, x, y, w, h, r, gcol, b);

			// Push button label
			gCurrent->drawList.emplace_back(DrawCmd::TEXT, x + 6, y + 4, 1, 1, 1, label);

			// Background
			//gCurrent->drawList.push_back(DrawCmd(DrawCmd::RECT, x, y, w, h, r, gcol, b));

			// Label text (center vertically)
			//gCurrent->drawList.push_back(DrawCmd(DrawCmd::TEXT,	x + 6, y + (h - g.fontSize) * 0.5f,	1, 1, 1, label));

			gCurrent->cursorY += h + gStyle.ItemSpacingY;
			gCurrent->lastItemW = w;
			gCurrent->lastItemH = h;

			return clicked;
		}

		inline bool ColorBoxLabel(const char* label, float* r, float* g, float* b, float w = 40.0f, float h = 20.0f) {
			if (!gCurrent) return false;

			float x = gCurrent->cursorX;
			float y = gCurrent->cursorY;

			bool hover = (gCurrent->mouseX >= x && gCurrent->mouseX <= x + w &&
				gCurrent->mouseY >= y && gCurrent->mouseY <= y + h);
			bool clicked = (hover && gCurrent->mousePressed);

			// --- record color box ---
			gCurrent->drawList.push_back(
				DrawCmd(DrawCmd::RECT, x, y, w, h, *r, *g, *b)
			);

			// --- record border ---
			gCurrent->drawList.push_back(DrawCmd(DrawCmd::RECT, x - 1, y - 1, w + 2, 1, 0, 0, 0)); // top
			gCurrent->drawList.push_back(DrawCmd(DrawCmd::RECT, x - 1, y + h, w + 2, 1, 0, 0, 0)); // bottom
			gCurrent->drawList.push_back(DrawCmd(DrawCmd::RECT, x - 1, y - 1, 1, h + 2, 0, 0, 0)); // left
			gCurrent->drawList.push_back(DrawCmd(DrawCmd::RECT, x + w, y - 1, 1, h + 2, 0, 0, 0)); // right

			// --- optional label ---
			if (label) {
				gCurrent->drawList.push_back(
					DrawCmd(DrawCmd::TEXT, x + w + 6, y, 1, 1, 1, label)
				);
			}

			// advance cursor
			gCurrent->cursorY += h + gStyle.ItemSpacingY;

			gCurrent->lastItemW = w;
			gCurrent->lastItemH = h;

			return clicked;
		}

    
	// ----------------------------------------------- Input Text Section ---------------------------------------
		// new InputText And working backspace, delete, caret
		inline bool InputText(const char* label, char* buf, size_t buf_size, float w, float h) {
			if (!gCurrent) return false;

			float x = gCurrent->cursorX;
			float y = gCurrent->cursorY;

			bool hover = (gCurrent->mouseX >= x && gCurrent->mouseX <= x + w &&
				gCurrent->mouseY >= y && gCurrent->mouseY <= y + h);
			bool clicked = (hover && gCurrent->mousePressed);

			// Background
			gCurrent->drawList.emplace_back(DrawCmd::RECT, x, y, w, h, 0.2f, 0.2f, 0.2f);

			// Focus
			if (clicked) {
				SpxGui::activeTextID = reinterpret_cast<uintptr_t>(buf);
				SpxGui::activeBuf = buf;
				SpxGui::caretIndex = (int)strlen(buf);
			}

			// Draw text
			gCurrent->drawList.emplace_back(DrawCmd::TEXT, x + 6, y + 4, 1, 1, 1, buf);

			// Handle input
			if (SpxGui::activeTextID == reinterpret_cast<uintptr_t>(buf)) {
				for (char c : gInputChars) {
					size_t len = strlen(buf);

					if (c == 8) { // Backspace
						if (SpxGui::caretIndex > 0 && len > 0) {
							memmove(buf + SpxGui::caretIndex - 1,
								buf + SpxGui::caretIndex,
								len - SpxGui::caretIndex + 1);
							SpxGui::caretIndex--;
						}
					}
					else if (c == 46) { // Delete set to dot key for easier testing as 127 will not work!
					//else if (c == 127 || c == 46) { // Delete
						if (SpxGui::caretIndex < (int)len) {
							memmove(buf + SpxGui::caretIndex,
								buf + SpxGui::caretIndex + 1,
								len - SpxGui::caretIndex);
						}
					}
					else if (c >= 32 && c < 126) { // Printable
						if (len + 1 < buf_size) {
							memmove(buf + SpxGui::caretIndex + 1,
								buf + SpxGui::caretIndex,
								len - SpxGui::caretIndex + 1);
							buf[SpxGui::caretIndex] = c;
							SpxGui::caretIndex++;
						}
					}
				}

				// Caret
				float caretX = x + 6 + CalcTextWidthN(buf, SpxGui::caretIndex);
				gCurrent->drawList.emplace_back(DrawCmd::CARET, caretX, y + 6, 2, g.fontSize, 1, 1, 1);
			}

			gCurrent->cursorY += h + gStyle.ItemSpacingY;
			gCurrent->lastItemW = w;
			gCurrent->lastItemH = h;

			return clicked;
		}
		// Replace the MultiLineText implementation in SpxGuiWidgets.h with this version
		inline bool MultiLineText(const char* label, std::string& text, float w, float h) {
			if (!gCurrent) return false;

			float x = gCurrent->cursorX;
			float y = gCurrent->cursorY;

			// Use same mouse source as single-line input
			bool hover = (gCurrent->mouseX >= x && gCurrent->mouseX <= x + w &&
				gCurrent->mouseY >= y && gCurrent->mouseY <= y + h);
			bool clicked = (hover && gCurrent->mousePressed);

			// Background
			gCurrent->drawList.emplace_back(DrawCmd::RECT, x, y, w, h, 0.2f, 0.2f, 0.2f);

			// Focus
			if (clicked) {
				SpxGui::activeTextID = reinterpret_cast<uintptr_t>(&text);
				SpxGui::caretIndex = (int)text.size(); // caret at end
				SpxGui::activeBuf = nullptr; // explicitly mark that active buffer is std::string
			}

			// Draw text (split into lines)
			float lineY = y + 4;
			{
				std::stringstream ss(text);
				std::string line;
				while (std::getline(ss, line)) {
					// DrawCmd stores a copy of the std::string (DrawCmd::text), so passing line.c_str() is fine here
					gCurrent->drawList.emplace_back(DrawCmd(DrawCmd::TEXT, x + 6, lineY, 1, 1, 1, line.c_str()));
					lineY += g.fontSize + 2;
				}
				if (!text.empty() && text.back() == '\n') {
					// trailing newline -> blank last line
					gCurrent->drawList.emplace_back(DrawCmd(DrawCmd::TEXT, x + 6, lineY, 1, 1, 1, ""));
				}
			}

			// Input handling
			if (SpxGui::activeTextID == reinterpret_cast<uintptr_t>(&text)) {
				// Ensure caret within range
				if (SpxGui::caretIndex < 0) SpxGui::caretIndex = 0;
				if (SpxGui::caretIndex > (int)text.size()) SpxGui::caretIndex = (int)text.size();

				// 1) Printable chars, backspace, enter/delete via gInputChars (kept behavior)
				for (char c : gInputChars) {
					if (c == 8) { // backspace
						if (SpxGui::caretIndex > 0) {
							text.erase(SpxGui::caretIndex - 1, 1);
							SpxGui::caretIndex--;
						}
					}
					else if (c == '\r' || c == '\n') {
						text.insert(SpxGui::caretIndex, 1, '\n');
						SpxGui::caretIndex++;
					}
					else if ((unsigned char)c >= 32 && (unsigned char)c < 127) { // printable
						text.insert(SpxGui::caretIndex, 1, c);
						SpxGui::caretIndex++;
					}
					// note: you used '.' (46) as a test delete key earlier; actual Delete should come via gInputKeys
				}

				// 2) Handle special keys (arrow/home/end/delete/up/down) from gInputKeys
				if (!gInputKeys.empty()) {
					// Build line start indices to make Up/Down easy and also to compute current line/col
					std::vector<int> lineStarts;
					lineStarts.push_back(0);
					for (int i = 0; i < (int)text.size(); ++i) {
						if (text[i] == '\n') {
							lineStarts.push_back(i + 1); // next line start after newline
						}
					}
					// Find current line and column
					int curLine = 0;
					for (int i = 0; i < (int)lineStarts.size(); ++i) {
						if (lineStarts[i] <= SpxGui::caretIndex) curLine = i;
						else break;
					}
					int col = SpxGui::caretIndex - lineStarts[curLine];

					for (int key : gInputKeys) {
						switch (key) {
						case GLWIN_LEFT:
							if (SpxGui::caretIndex > 0) SpxGui::caretIndex--;
							break;
						case GLWIN_RIGHT:
							if (SpxGui::caretIndex < (int)text.size()) SpxGui::caretIndex++;
							break;
						case GLWIN_HOME:
							// move to start of current line
							if (curLine < (int)lineStarts.size()) {
								SpxGui::caretIndex = lineStarts[curLine];
							}
							else {
								SpxGui::caretIndex = 0;
							}
							break;
						case GLWIN_END:
							// move to end of current line (just before next newline or end)
						{
							int lineStart = lineStarts[curLine];
							int lineEnd = (curLine + 1 < (int)lineStarts.size()) ? lineStarts[curLine + 1] - 1 : (int)text.size();
							// lineEnd is index of '\n' or text.size()
							SpxGui::caretIndex = lineEnd;
						}
						break;
						case GLWIN_UP:
							if (curLine > 0) {
								int prevStart = lineStarts[curLine - 1];
								int prevEnd = lineStarts[curLine] - 1; // index of newline at end of prev line, or start of current -1
								int prevLen = prevEnd - prevStart;
								if (prevLen < 0) prevLen = 0;
								int targetCol = (std::min)(col, prevLen);
								SpxGui::caretIndex = prevStart + targetCol;
								// update curLine/col for subsequent keys in same frame
								curLine = curLine - 1;
								col = SpxGui::caretIndex - lineStarts[curLine];
							}
							break;
						case GLWIN_DOWN:
							if (curLine + 1 < (int)lineStarts.size()) {
								int nextStart = lineStarts[curLine + 1];
								int nextEnd = (curLine + 2 < (int)lineStarts.size()) ? lineStarts[curLine + 2] - 1 : (int)text.size();
								int nextLen = nextEnd - nextStart;
								if (nextLen < 0) nextLen = 0;
								int targetCol = (std::min)(col, nextLen);
								SpxGui::caretIndex = nextStart + targetCol;
								curLine = curLine + 1;
								col = SpxGui::caretIndex - lineStarts[curLine];
							}
							break;
						case GLWIN_DELETE:
							if (SpxGui::caretIndex < (int)text.size()) {
								text.erase(SpxGui::caretIndex, 1);
							}
							break;
						default:
							// if other keys are pushed, ignore them here
							break;
						}
						// Clamp caretIndex after each key handling
						if (SpxGui::caretIndex < 0) SpxGui::caretIndex = 0;
						if (SpxGui::caretIndex > (int)text.size()) SpxGui::caretIndex = (int)text.size();
					}
				}

				// Draw caret (calculate line/col for rendering)
				int line = 0, col = 0;
				for (int i = 0; i < SpxGui::caretIndex && i < (int)text.size(); i++) {
					if (text[i] == '\n') { line++; col = 0; }
					else col++;
				}

				float caretX = x + 6 + CalcTextWidthN(text.c_str() + (SpxGui::caretIndex - col), col);
				float caretY = y + 4 + line * (g.fontSize + 2);

				if ((g.frameCount / 30) % 2 == 0) {
					gCurrent->drawList.emplace_back(DrawCmd::CARET, caretX, caretY, 2, g.fontSize, 1, 1, 1);
				}
			}

			gCurrent->cursorY += h + gStyle.ItemSpacingY;
			gCurrent->lastItemW = w;
			gCurrent->lastItemH = h;

			return clicked;
		}

		//inline bool MultiLineText(const char* label, std::string& text, float w, float h) {
		//	if (!gCurrent) return false;

		//	float x = gCurrent->cursorX;
		//	float y = gCurrent->cursorY;

		//	// Use same mouse source as single-line input
		//	bool hover = (gCurrent->mouseX >= x && gCurrent->mouseX <= x + w &&
		//		gCurrent->mouseY >= y && gCurrent->mouseY <= y + h);
		//	bool clicked = (hover && gCurrent->mousePressed);

		//	// Background
		//	gCurrent->drawList.emplace_back(DrawCmd::RECT, x, y, w, h, 0.2f, 0.2f, 0.2f);

		//	// --- focus ---
		//	if (clicked) {
		//		//SpxGui::activeTextID = reinterpret_cast<uintptr_t>(&text);
		//		//SpxGui::caretIndex = (int)text.size(); // caret at end
		//		SpxGui::activeTextID = reinterpret_cast<uintptr_t>(&text);
		//		SpxGui::caretIndex = (int)text.size(); // caret at end
		//		SpxGui::activeBuf = nullptr; // explicitly mark that active buffer is a std::string

		//	}

		//	// --- draw text ---
		//	float lineY = y + 4;
		//	{
		//		std::stringstream ss(text);
		//		std::string line;
		//		while (std::getline(ss, line)) {
		//			// IMPORTANT: DrawCmd must copy the string internally. If it only stores the const char*,
		//			// the pointer returned by line.c_str() becomes invalid after this loop.
		//			gCurrent->drawList.emplace_back(
		//				DrawCmd(DrawCmd::TEXT, x + 6, lineY, 1, 1, 1, line.c_str())
		//			);
		//			lineY += g.fontSize + 2;
		//		}
		//		if (!text.empty() && text.back() == '\n') {
		//			gCurrent->drawList.emplace_back(
		//				DrawCmd(DrawCmd::TEXT, x + 6, lineY, 1, 1, 1, "")
		//			);
		//		}
		//	}

		//	// --- input handling ---
		//	if (SpxGui::activeTextID == reinterpret_cast<uintptr_t>(&text)) {
		//		// Ensure caret is inside [0, text.size()]
		//		if (SpxGui::caretIndex < 0) SpxGui::caretIndex = 0;
		//		if (SpxGui::caretIndex > (int)text.size()) SpxGui::caretIndex = (int)text.size();

		//		for (char c : gInputChars) {
		//			if (c == 8) { // backspace
		//				if (SpxGui::caretIndex > 0) {
		//					text.erase(SpxGui::caretIndex - 1, 1);
		//					SpxGui::caretIndex--;
		//				}
		//			}
		//			else if (c == 46 || c == 127) { // delete (you used 46 earlier as a test)
		//				if (SpxGui::caretIndex < (int)text.size()) {
		//					text.erase(SpxGui::caretIndex, 1);
		//				}
		//			}
		//			else if (c == '\r' || c == '\n') {
		//				text.insert(SpxGui::caretIndex, 1, '\n');
		//				SpxGui::caretIndex++;
		//			}
		//			else if ((unsigned char)c >= 32 && (unsigned char)c < 127) { // printable
		//				text.insert(SpxGui::caretIndex, 1, c);
		//				SpxGui::caretIndex++;
		//			}
		//		}

		//		// --- caret position calculation ---
		//		int line = 0, col = 0;
		//		for (int i = 0; i < SpxGui::caretIndex && i < (int)text.size(); i++) {
		//			if (text[i] == '\n') { line++; col = 0; }
		//			else col++;
		//		}

		//		float caretX = x + 6 + CalcTextWidthN(text.c_str() + (SpxGui::caretIndex - col), col);
		//		float caretY = y + 4 + line * (g.fontSize + 2);

		//		if ((g.frameCount / 30) % 2 == 0) {
		//			gCurrent->drawList.emplace_back(DrawCmd::CARET, caretX, caretY, 2, g.fontSize, 1, 1, 1);
		//		}
		//	}

		//	gCurrent->cursorY += h + gStyle.ItemSpacingY;
		//	gCurrent->lastItemW = w;
		//	gCurrent->lastItemH = h;

		//	return clicked;
		//}

		// new MultiLineText
		//inline bool MultiLineText(const char* label, std::string& text, float w, float h) {
		//	if (!gCurrent) return false;

		//	float x = gCurrent->cursorX;
		//	float y = gCurrent->cursorY;

		//	bool hover = (gMouseX >= x && gMouseX <= x + w &&
		//		gMouseY >= y && gMouseY <= y + h);
		//	bool clicked = (hover && gMousePressed);

		//	/*bool hover = (gCurrent->mouseX >= x && gCurrent->mouseX <= x + w &&
		//		gCurrent->mouseY >= y && gCurrent->mouseY <= y + h);
		//	bool clicked = (hover && gCurrent->mousePressed);*/


		//	// Background
		//	gCurrent->drawList.emplace_back(DrawCmd::RECT, x, y, w, h, 0.2f, 0.2f, 0.2f);

		//	// --- focus ---
		//	if (clicked) {
		//		SpxGui::activeTextID = reinterpret_cast<uintptr_t>(&text);
		//		SpxGui::caretIndex = (int)text.size(); // caret at end
		//	}

		//	// --- draw text ---
		//	float lineY = y + 4;
		//	{
		//		std::stringstream ss(text);
		//		std::string line;
		//		while (std::getline(ss, line)) {
		//			gCurrent->drawList.emplace_back(
		//				DrawCmd(DrawCmd::TEXT, x + 6, lineY, 1, 1, 1, line.c_str())
		//			);
		//			lineY += g.fontSize + 2;
		//		}
		//		if (text.size() && text.back() == '\n') {
		//			gCurrent->drawList.emplace_back(
		//				DrawCmd(DrawCmd::TEXT, x + 6, lineY, 1, 1, 1, "")
		//			);
		//		}
		//	}

		//	// --- input handling ---
		//	if (SpxGui::activeTextID == reinterpret_cast<uintptr_t>(&text)) {
		//		// text insertion/deletion
		//		for (char c : gInputChars) {
		//			
		//			if (c == 8) { // backspace
		//				if (SpxGui::caretIndex >  0) { // was 0
		//					text.erase(SpxGui::caretIndex - 1, 1);
		//					SpxGui::caretIndex--;
		//				}
		//			}
		//			else if (c == 46 || c == 127) { // delete
		//				if (SpxGui::caretIndex < (int)text.size()) {
		//					text.erase(SpxGui::caretIndex, 1);
		//				}
		//			}
		//			else if (c == '\r' || c == '\n') {
		//				text.insert(SpxGui::caretIndex, 1, '\n');
		//				SpxGui::caretIndex++;
		//			}
		//			else if (c >= 32 && c < 12) {
		//				text.insert(SpxGui::caretIndex, 1, c);
		//				SpxGui::caretIndex++;
		//			}
		//		}

		//		// handle arrow/home/end via gInputKeys
		//		//HandleTextEditing(text, SpxGui::caretIndex);

		//		

		//		// --- draw caret ---
		//		int line = 0, col = 0;
		//		for (int i = 0; i < SpxGui::caretIndex && i < (int)text.size(); i++) {
		//			if (text[i] == '\n') { line++; col = 0; }
		//			else col++;
		//		}

		//		float caretX = x + 6 + CalcTextWidthN(text.c_str() + (SpxGui::caretIndex - col), col);
		//		float caretY = y + 4 + line * (g.fontSize + 2);

		//		if ((g.frameCount / 30) % 2 == 0) {
		//			gCurrent->drawList.emplace_back(DrawCmd::CARET, caretX, caretY, 2, g.fontSize, 1, 1, 1);
		//		}

		//		
		//	}

		//	gCurrent->cursorY += h + gStyle.ItemSpacingY;
		//	gCurrent->lastItemW = w;
		//	gCurrent->lastItemH = h;

		//	return clicked;
		//}

	// ----------------------------------------------- Drag Float Section ---------------------------------------

	// This dragfloat add text to the boxes R: 0.0f G: 0.0f B: 0.0f
	inline void Drag3FloatText(float* v, float wBox = 60, float hBox = 20) {
		if (!gCurrent) return;

		const char* labels[3] = { "R:", "G:", "B:" };
		char buf[64];

		float startX = gCurrent->cursorX;
		float curY = gCurrent->cursorY;

		for (int i = 0; i < 3; i++) {
			// format into one buffer with label + number
			snprintf(buf, sizeof(buf), "%s %.3f", labels[i], v[i]);

			float boxX = startX + i * (wBox + gStyle.ItemSpacingX);

			// different tint for each channel
			float cr = (i == 0) ? 0.4f : 0.2f;
			float cg = (i == 1) ? 0.4f : 0.2f;
			float cb = (i == 2) ? 0.4f : 0.2f;
			// push rect to draw list
			gCurrent->drawList.push_back(DrawCmd(DrawCmd::RECT, boxX, curY, wBox, hBox, cr, cg, cb));

			// vertically center text in the box
			float textY = curY + (hBox - g.fontSize) * 0.5f + g.fontSize * 0.75f;
			gCurrent->drawList.push_back(DrawCmd(DrawCmd::TEXT, boxX + 4, textY - 16, 1.0f, 1.0f, 1.0f, buf));
		}

		gCurrent->cursorY += hBox + gStyle.ItemSpacingY;
		gCurrent->lastItemW = 3 * wBox + 2 * gStyle.ItemSpacingX;
		gCurrent->lastItemH = hBox;
	}
	
	// just a standard drag3float with no text
	inline void Drag3Float(float* v, float wBox = 60, float hBox = 20) {
		if (!gCurrent) return;

		char buf[64];
		float startX = gCurrent->cursorX;
		float curY = gCurrent->cursorY;

		for (int i = 0; i < 3; i++) {
			// format the number only
			snprintf(buf, sizeof(buf), "%.3f", v[i]);

			// position of this box
			float boxX = startX + i * (wBox + gStyle.ItemSpacingX);

			// different background tint per channel (optional)
			float cr = (i == 0) ? 0.3f : 0.2f;
			float cg = (i == 1) ? 0.3f : 0.2f;
			float cb = (i == 2) ? 0.3f : 0.2f;
			// push rect to draw list
			gCurrent->drawList.push_back(DrawCmd(DrawCmd::RECT, boxX, curY, wBox, hBox, cr, cg, cb));

			

			// center text vertically inside the box
			float textY = curY + (hBox - g.fontSize) * 0.5f + g.fontSize * 0.75f;
			// push text to draw list
			gCurrent->drawList.push_back(DrawCmd(DrawCmd::TEXT, boxX + 4, textY - 16, 1.0f, 1.0f, 1.0f, buf));
		}

		// advance layout cursor
		gCurrent->cursorY += hBox + gStyle.ItemSpacingY;
		gCurrent->lastItemW = 3 * wBox + 2 * gStyle.ItemSpacingX;
		gCurrent->lastItemH = hBox;
	}
	// ----------------------------------------------- Drag 4 Float Section ---------------------------------------

	inline void Drag4FloatText(float* v, float wBox = 60, float hBox = 20) {
		if (!gCurrent) return;

		const char* labels[4] = { "R:", "G:", "B:", "A:" }; // or X Y Z W
		char buf[64];

		float startX = gCurrent->cursorX;
		float curY = gCurrent->cursorY;

		for (int i = 0; i < 4; i++) {
			// format "Label value"
			snprintf(buf, sizeof(buf), "%s %.3f", labels[i], v[i]);

			// position of this box
			float boxX = startX + i * (wBox + gStyle.ItemSpacingX);

			// background tint (optional)
			float cr = (i == 0) ? 0.3f : 0.2f;
			float cg = (i == 1) ? 0.3f : 0.2f;
			float cb = (i == 2) ? 0.3f : 0.2f;
			if (i == 3) { cr = 0.25f; cg = 0.25f; cb = 0.25f; } // neutral for A

			// draw background
			//DrawRect(boxX, curY, wBox, hBox, cr, cg, cb);
			gCurrent->drawList.push_back(DrawCmd(DrawCmd::RECT, boxX, curY, wBox, hBox, cr, cg, cb));

			// center text vertically inside the box
			float textY = curY + (hBox - g.fontSize) * 0.5f + g.fontSize * 0.75f;
			gCurrent->drawList.push_back(DrawCmd(DrawCmd::TEXT, boxX + 4, textY - 16, 1.0f, 1.0f, 1.0f, buf));
			//DrawText(boxX + 4, textY -16, buf, 1, 1, 1);
		}

		// advance layout cursor
		gCurrent->cursorY += hBox + gStyle.ItemSpacingY;
		gCurrent->lastItemW = 4 * wBox + 3 * gStyle.ItemSpacingX;
		gCurrent->lastItemH = hBox;
	}

	// just a standard dragfloat with no text
	inline void Drag4Float(float* v, float wBox = 60, float hBox = 20) {
		if (!gCurrent) return;

		v[0], v[1], v[2], v[3];

		char buf[64];
		float startX = gCurrent->cursorX;
		float curY = gCurrent->cursorY;

		for (int i = 0; i < 4; i++) {
			snprintf(buf, sizeof(buf), "%.3f", v[i]);
			float boxX = startX + i * (wBox + gStyle.ItemSpacingX);

			float cr = (i == 0) ? 0.4f : 0.2f;
			float cg = (i == 1) ? 0.4f : 0.2f;
			float cb = (i == 2) ? 0.4f : 0.2f;
			if (i == 3) { cr = 0.3f; cg = 0.3f; cb = 0.3f; }

			//DrawRect(boxX, curY, wBox, hBox, cr, cg, cb);
			gCurrent->drawList.push_back(DrawCmd(DrawCmd::RECT, boxX, curY, wBox, hBox, cr, cg, cb));
			float textY = curY + (hBox - g.fontSize) * 0.5f + g.fontSize * 0.75f;
			//DrawText(boxX + 6, textY - 16, buf, 1, 1, 1);
			gCurrent->drawList.push_back(DrawCmd(DrawCmd::TEXT, boxX + 4, textY - 16, 1.0f, 1.0f, 1.0f, buf));
			
		}

		gCurrent->cursorY += hBox + gStyle.ItemSpacingY;
		gCurrent->lastItemW = 4 * wBox + 3 * gStyle.ItemSpacingX;
		gCurrent->lastItemH = hBox;
	}

	// --------------------------------------- Sliders Section ---------------------------------------
	inline void Slider() {
		// Placeholder for future slider implementation
	}
	
	// --------------------------------------- Label Section ---------------------------------------

	// simple text output like a label
	inline bool ColoredLabel(float r, float gcol, float b, const char* txt) {
		if (!gCurrent) return false;
		
		float x = gCurrent->cursorX;
		float y = gCurrent->cursorY;

		// record into drawList instead of drawing immediately
		gCurrent->drawList.push_back(
			DrawCmd(DrawCmd::TEXT, x, y, r, gcol, b, txt)
		);
		

		gCurrent->cursorY += g.fontSize + gStyle.ItemSpacingY; // move cursor down for next item

		// record text size
		gCurrent->lastItemW = (float)strlen(txt) * (g.fontSize * 0.6f); // crude width estimate
		gCurrent->lastItemH = g.fontSize;

		return true;
	}
	// --------------------------------------- Separator and spacing Section ---------------------------------------
	inline bool SeparatorLine(float w) {
		if (!gCurrent) return false;

		float x = gCurrent->cursorX;
		float y = gCurrent->cursorY;

		//DrawRect(x, y, w, 1, 0.5f, 0.5f, 0.5f); // horizontal line
		gCurrent->drawList.push_back(DrawCmd(DrawCmd::RECT, x, y, w, 1, 0.5f, 0.5f, 0.5f));

		gCurrent->cursorY += 1 + gStyle.ItemSpacingY; // move cursor down for next item

		

		return true;
	}

	//inline bool SeparatorText(const char* label, float w) {
	inline void SeparatorText(const char* label, float w) {

		if (!gCurrent) return;

		float x = gCurrent->cursorX;
		float y = gCurrent->cursorY;

		// measure text width
		float xpos = 0, ypos = 0;
		float textWidth = 0;
		for (const char* p = label; *p; p++) {
			if (*p < 32 || *p >= 128) continue;
			stbtt_aligned_quad q;
			stbtt_GetBakedQuad(g.cdata, 512, 512, *p - 32, &xpos, &ypos, &q, 1);
			textWidth = q.x1; // last character right edge
		}

		// center text horizontally inside given width
		float textX = x + (w - textWidth) * 0.5f;
		float textY = y;

		// line Y = center aligned with text baseline
		float lineY = textY + g.fontSize * 0.5f;

		// left line
		if (textX - x > 8) {
			//DrawRect(x, lineY, textX - x - 8, 1, 0.5f, 0.5f, 0.5f);
			gCurrent->drawList.push_back(DrawCmd(DrawCmd::RECT, x, lineY, textX - x - 8, 1, 0.5f, 0.5f, 0.5f));
		}

		// text
		//DrawText(textX, textY, label, 1, 1, 1);
		gCurrent->drawList.push_back(DrawCmd(DrawCmd::TEXT, textX, textY, 1, 1, 1, label));

		// right line
		float rightStart = textX + textWidth + 8;
		if (rightStart < x + w) {
			//DrawRect(rightStart, lineY, (x + w) - rightStart, 1, 0.5f, 0.5f, 0.5f);
			gCurrent->drawList.push_back(DrawCmd(DrawCmd::RECT, rightStart, lineY, (x + w) - rightStart, 1, 0.5f, 0.5f, 0.5f));

		}

		// advance cursor
		gCurrent->cursorY += g.fontSize + gStyle.ItemSpacingY;

		//return true;
	}
	inline void SameLine(float spacing = -1.0f) {
		if (!gCurrent) return;

		if (spacing < 0.0f) spacing = gStyle.ItemSpacingX;

		// Move cursorX to the right of the last item
		gCurrent->cursorX += gCurrent->lastItemW + spacing;

		// Reset cursorY back to last item top (instead of advancing down)
		gCurrent->cursorY -= gCurrent->lastItemH + gStyle.ItemSpacingY;
	}


	inline void Spacing() {
		if (!gCurrent) return;

		// Advance cursor by one line height (or just spacing if you prefer)
		gCurrent->cursorY += g.fontSize + gStyle.ItemSpacingY;
	}
	

	// --------------------------------------- Image Section ---------------------------------------

	inline bool ImageButton(const std::string& texPath, float w, float h) {
		if (!gCurrent) return false;

		float x = gCurrent->cursorX;
		float y = gCurrent->cursorY;

		bool hover = (gCurrent->mouseX >= x && gCurrent->mouseX <= x + w &&
			gCurrent->mouseY >= y && gCurrent->mouseY <= y + h);
		bool clicked = (hover && gCurrent->mousePressed);

		// Load or reuse cached texture
		Image img;
		unsigned int texID = LoadTextuer(texPath, img);

		// Optional hover/click overlay
		if (hover || clicked) {
			float r = hover ? 0.8f : 0.6f;
			float gcol = hover ? 0.8f : 0.6f;
			float b = hover ? 0.8f : 0.6f;
			gCurrent->drawList.push_back(DrawCmd(DrawCmd::RECT, x - 2, y - 2, w + 4, h + 4, r, gcol, b));
			//DrawRect(x - 2, y - 2, w + 4, h + 4, r, gcol, b);
		}

		// Draw the image itself
		if (texID != 0) {
			gCurrent->drawList.push_back(DrawCmd(DrawCmd::IMAGE, texID, x, y, w, h));
			//DrawImage(texID, x, y, w, h);
		}

		// Advance cursor
		gCurrent->cursorY += h + gStyle.ItemSpacingY;

		gCurrent->lastItemW = w;
		gCurrent->lastItemH = h;



		return clicked;
	}

	inline bool ImageBox(const std::string& texPath, float w, float h, const char* label = nullptr) {
		if (!gCurrent) return false;

		float x = gCurrent->cursorX;
		float y = gCurrent->cursorY;

		bool hover = (gCurrent->mouseX >= x && gCurrent->mouseX <= x + w &&
			gCurrent->mouseY >= y && gCurrent->mouseY <= y + h);
		bool clicked = (hover && gCurrent->mousePressed);

		Image img;
		unsigned int texID = LoadTextuer(texPath, img);

		float r = 0.3f, gcol = 0.3f, b = 0.3f;
		if (hover) { r = 0.4f; gcol = 0.4f; b = 0.6f; }
		if (clicked) { r = 0.2f; gcol = 0.6f; b = 0.2f; }

		// border rect
		gCurrent->drawList.emplace_back(DrawCmd::RECT, x - 2, y - 2, w + 4, h + 4, r, gcol, b);

		// image
		if (img.textureID != 0) {
			gCurrent->drawList.emplace_back(DrawCmd::IMAGE, texID, x, y, w, h);
		}

		// caption
		float totalH = h;
		if (label) {
			float textY = y + h + 4; // 4px gap under image
			gCurrent->drawList.emplace_back(DrawCmd::TEXT, x, textY, 1, 1, 1, label);
			totalH += g.fontSize + 4;
		}

		// advance cursor normally like other widgets
		gCurrent->cursorY += totalH + gStyle.ItemSpacingY;
		gCurrent->lastItemW = w;
		gCurrent->lastItemH = totalH;

		return clicked;
	}


	// --------------------------------------- Color Picker Section ---------------------------------------

	inline bool ColorBox(const char* label, float* r, float* g, float* b, float w = 40.0f, float h = 40.0f) {
		if (!gCurrent) return false;

		float x = gCurrent->cursorX;
		float y = gCurrent->cursorY;

		bool hover = (gCurrent->mouseX >= x && gCurrent->mouseX <= x + w &&
			gCurrent->mouseY >= y && gCurrent->mouseY <= y + h);
		bool clicked = (hover && gCurrent->mousePressed);

		// --- record color box ---
		gCurrent->drawList.push_back(
			DrawCmd(DrawCmd::RECT, x, y, w, h, *r, *g, *b)
		);

		// --- record border ---
		gCurrent->drawList.push_back(DrawCmd(DrawCmd::RECT, x - 1, y - 1, w + 2, 1, 0, 0, 0)); // top
		gCurrent->drawList.push_back(DrawCmd(DrawCmd::RECT, x - 1, y + h, w + 2, 1, 0, 0, 0)); // bottom
		gCurrent->drawList.push_back(DrawCmd(DrawCmd::RECT, x - 1, y - 1, 1, h + 2, 0, 0, 0)); // left
		gCurrent->drawList.push_back(DrawCmd(DrawCmd::RECT, x + w, y - 1, 1, h + 2, 0, 0, 0)); // right

		// --- optional label ---
		if (label) {
			gCurrent->drawList.push_back(
				DrawCmd(DrawCmd::TEXT, x + w + 6, y, 1, 1, 1, label)
			);
		}

		// advance cursor
		gCurrent->cursorY += h + gStyle.ItemSpacingY;

		gCurrent->lastItemW = w;
		gCurrent->lastItemH = h;

		return clicked;
	}

	inline bool ColorSVSquare(float hue, float& sat, float& val,
		float x, float y, float size) {
		if (!gCurrent) return false;

		bool changed = false;

		// Draw square as grid of small quads
		//int steps = 16; // increase for smoother
		int steps = 32; // increase for smoother
		float step = size / steps;
		for (int i = 0; i < steps; i++) {
			for (int j = 0; j < steps; j++) {
				float s0 = (float)i / steps;
				float v0 = 1.0f - (float)j / steps;
				float s1 = (float)(i + 1) / steps;
				float v1 = 1.0f - (float)(j + 1) / steps;

				float r0, g0, b0, r1, g1, b1, r2, g2, b2, r3, g3, b3;
				HSVtoRGB(hue, s0, v0, r0, g0, b0);
				HSVtoRGB(hue, s1, v0, r1, g1, b1);
				HSVtoRGB(hue, s1, v1, r2, g2, b2);
				HSVtoRGB(hue, s0, v1, r3, g3, b3);

				// average color for crude quad
				float r = (r0 + r1 + r2 + r3) * 0.25f; //0.25f;
				float g = (g0 + g1 + g2 + g3) * 0.25f;
				float b = (b0 + b1 + b2 + b3) * 0.25f;

				gCurrent->drawList.push_back(
					DrawCmd(DrawCmd::RECT,
						x + i * step, y + j * step,
						step, step, r, g, b));
			}
		}

		// Handle mouse input
		bool hover = (gCurrent->mouseX >= x && gCurrent->mouseX <= x + size &&
			gCurrent->mouseY >= y && gCurrent->mouseY <= y + size);
		if (hover && gCurrent->mouseDown) {
			float relX = (gCurrent->mouseX - x) / size;
			float relY = (gCurrent->mouseY - y) / size;
			sat = std::clamp(relX, 0.0f, 1.0f);
			val = 1.0f - std::clamp(relY, 0.0f, 1.0f);
			changed = true;
		}

		// Draw selection marker
		float markerX = x + sat * size;
		float markerY = y + (1.0f - val) * size;
		gCurrent->drawList.push_back(
			DrawCmd(DrawCmd::RECT, markerX - 2, markerY - 2, 5, 5, 1, 1, 1));

		return changed;
	}

	inline bool HueSlider(float& hue, float x, float y, float w, float h) {
		if (!gCurrent) return false;
		bool changed = false;

		int steps = 30; // number of colored segments
		float step = w / steps;
		for (int i = 0; i < steps; i++) {
			float t0 = (float)i / steps;
			float t1 = (float)(i + 1) / steps;
			float r0, g0, b0, r1, g1, b1;
			HSVtoRGB(t0, 1.0f, 1.0f, r0, g0, b0);
			HSVtoRGB(t1, 1.0f, 1.0f, r1, g1, b1);

			// average color for strip segment
			float r = (r0 + r1) * 0.5f;
			float g = (g0 + g1) * 0.5f;
			float b = (b0 + b1) * 0.5f;

			gCurrent->drawList.push_back(
				DrawCmd(DrawCmd::RECT,
					x + i * step, y,
					step, h, r, g, b));
		}

		// Handle mouse input
		bool hover = (gCurrent->mouseX >= x && gCurrent->mouseX <= x + w &&
			gCurrent->mouseY >= y && gCurrent->mouseY <= y + h);
		if (hover && gCurrent->mouseDown) {
			float relX = (gCurrent->mouseX - x) / w;
			hue = std::clamp(relX, 0.0f, 1.0f);
			changed = true;
		}

		// Draw selection marker
		float markerX = x + hue * w;
		gCurrent->drawList.push_back(
			DrawCmd(DrawCmd::RECT, markerX - 2, y - 2, 4, h + 4, 1, 1, 1));

		return changed;
	}

	inline bool ColorEdit3(const char* label, float col[3]) {
		if (!gCurrent) return false;

		// 1) Layout & draw label + swatch
		const float x = gCurrent->cursorX;
		const float y = gCurrent->cursorY;

		// label
		gCurrent->drawList.push_back(DrawCmd(DrawCmd::TEXT, x, y, 1, 1, 1, label));

		// small swatch to the right of the label
		const float boxSize = g.fontSize;              // square
		const float boxX = x + 100.0f;              // simple offset; adjust if you like
		const float boxY = y;
		gCurrent->drawList.push_back(
			DrawCmd(DrawCmd::RECT, boxX, boxY, boxSize, boxSize, col[0], col[1], col[2])
		);

		// 2) Interaction on the swatch
		const bool hover =
			(gCurrent->mouseX >= boxX && gCurrent->mouseX <= boxX + boxSize &&
				gCurrent->mouseY >= boxY && gCurrent->mouseY <= boxY + boxSize);
		const bool clicked = (hover && gCurrent->mousePressed);

		// 3) Per-control popup state (unique id = windowID ^ label pointer)
		const uint64_t ctrlID = (uint64_t)gCurrent->SpxGuiWinID ^ (uint64_t)(uintptr_t)label;
		static std::unordered_map<uint64_t, bool> popupOpen;
		static std::unordered_map<uint64_t, bool> popupWasPositioned;

		// Open popup on click
		if (clicked) {
			popupOpen[ctrlID] = true;

			// Position the popup near the swatch on first open
			if (!popupWasPositioned[ctrlID]) {
				// Ensure a popup window with a unique ID exists, then place it
				const int popID = (int)(0x70000000u ^ (uint32_t)ctrlID);
				auto it = std::find_if(gWindows.begin(), gWindows.end(),
					[&](const SpxGuiWindow& w) { return w.SpxGuiWinID == popID; });
				if (it == gWindows.end()) {
					gWindows.push_back(SpxGuiWindow());
					gWindows.back().SpxGuiWinID = popID;
					gWindows.back().curWinX = boxX + 10.0f;
					gWindows.back().curWinY = boxY + boxSize + 10.0f;
				}
				else {
					it->curWinX = boxX + 10.0f;
					it->curWinY = boxY + boxSize + 10.0f;
				}
				popupWasPositioned[ctrlID] = true;
			}
		}

		// 4) If open, render the popup color picker (uses your implementation in SpxGui.h)
		bool anyChange = false;
		if (popupOpen[ctrlID]) {
			bool openFlag = true;  // controls popup visibility

			// Pass the popup title and an open flag
			SpxGui::BeginColPopUp(label, &openFlag, (int)(0x70000000u ^ (uint32_t)ctrlID));

			// Draw the actual popup contents here (SV square, hue slider, etc.)
			// Example: (replace with your real code)
			SpxGui::ColoredLabel(1, 1, 0, "Popup Color Picker");
			float r = 0.217, g = 0.207, b = 0.184;
			static float hue = 0.0f;   // range 0..1
			static float sat = 1.0f;   // range 0..1
			static float val = 1.0f;   // range 0..1

			float svSize = 120.0f;

			// ------------------- Table for SV square and Hue slider -------------------
			SpxGui::BeginTable("MyTable", 2);

			// -------------------------------- SV Square ------------------------------

			SpxGui::HSVtoRGB(hue, sat, val, r, g, b);

			if (ColorSVSquare(hue, sat, val, gCurrent->cursorX, gCurrent->cursorY, 120)) {
				std::cout << "SV changed: S=" << sat << " V=" << val << "\n";
			}

			SpxGui::TableNextColumn();
			SpxGui::ColorBox("Light Color", &r, &g, &b);

			SpxGui::ColorBox("Original", &r, &g, &b);

			SpxGui::TableNextRow();
			SpxGui::gCurrent->cursorY += 85 + SpxGui::gStyle.ItemSpacingY; //120

			// Hue slider (next to SV square)
			if (HueSlider(hue, gCurrent->cursorX, gCurrent->cursorY, 120, 20)) {
				std::cout << "Hue=" << hue << "\n";
			}
			SpxGui::gCurrent->cursorY += 20 + SpxGui::gStyle.ItemSpacingY;

			float tmpCol[3] = { r,g,b };
			SpxGui::Drag3FloatText(tmpCol);
			
			float color_4r[4] = { r, g, b, 1.0f };
			SpxGui::Drag4FloatText(color_4r);

			if (SpxGui::Button("Close Popup", 120, 30))
				openFlag = false;
			      
			SpxGui::EndTable();

			SpxGui::EndPopUp();

			if (!openFlag) popupOpen[ctrlID] = false;
			anyChange = true;
		}

		//gCurrent->cursorY += boxSize + gStyle.ItemSpacingY; // move cursor down for next item
		//gCurrent->lastItemW = (boxX + boxSize) - x;
		//gCurrent->lastItemH = boxSize;

		

		return anyChange || clicked;
	}

	inline bool ColorEdit4(const char* label, float col[4]) {
		// Reuse ColorEdit3 and add an alpha slider
		// Placeholder for future implementation
	}

} // namespace MyNamespace


// handle arrow keys (from AddKeyPress style)
				/*for (int key : gInputKeys) {
					switch (key) {
					case GLWIN_BACKSPACE:
						if (SpxGui::caretIndex > 0) {
							text.erase(SpxGui::caretIndex - 1, 1);
							SpxGui::caretIndex--;
							SpxGui::gInputKeys.push_back(key);
						}
						break;
					case GLWIN_DELETE:
						if (SpxGui::caretIndex < (int)text.size()) {
							text.erase(SpxGui::caretIndex, 1);
						}
						break;
					case GLWIN_LEFT:
						if (SpxGui::caretCol > 0) SpxGui::caretCol--;
						else if (SpxGui::caretRow > 0) {
							SpxGui::caretRow--;
							SpxGui::caretCol = (int)lines[SpxGui::caretRow].size();
						}
						break;
					case GLWIN_RIGHT:
						if (SpxGui::caretCol < (int)lines[SpxGui::caretRow].size())
							SpxGui::caretCol++;
						else if (SpxGui::caretRow < (int)lines.size() - 1) {
							SpxGui::caretRow++;
							SpxGui::caretCol = 0;
						}
						break;
					case GLWIN_UP:
						if (SpxGui::caretRow > 0) {
							SpxGui::caretRow--;
							SpxGui::caretCol = (std::min)(SpxGui::caretCol, (int)lines[SpxGui::caretRow].size());
						}
						break;
					case GLWIN_DOWN:
						if (SpxGui::caretRow < (int)lines.size() - 1) {
							SpxGui::caretRow++;
							SpxGui::caretCol = (std::min)(SpxGui::caretCol, (int)lines[SpxGui::caretRow].size());
						}
						break;
					case GLWIN_HOME:
						SpxGui::caretCol = 0;
						break;
					case GLWIN_END:
						SpxGui::caretCol = (int)lines[SpxGui::caretRow].size();
						break;
					}
				}*/