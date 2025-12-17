#include <fstream>
#include <sstream>
#include <unordered_set>
#include <vector>
#include <iostream>

#include "../include/SPLCore.h"
#include "../include/lexer.h"
#include "../include/parsing.h"
#include "../include/interpreter.h"
#include "../include/splHelper.h"

namespace {

    // --- small path helpers (no <filesystem> needed) ---
    static std::string dirnameOf(const std::string& path) {
        size_t s1 = path.find_last_of("/\\");
        if (s1 == std::string::npos) return "";
        return path.substr(0, s1);
    }

    static std::string joinPath(const std::string& a, const std::string& b) {
        if (a.empty()) return b;
        char last = a.back();
        if (last == '/' || last == '\\') return a + b;
        return a + "/" + b;
    }

    static std::string trim(const std::string& s) {
        size_t i = 0, j = s.size();
        while (i < j && (s[i] == ' ' || s[i] == '\t' || s[i] == '\r' || s[i] == '\n')) i++;
        while (j > i && (s[j - 1] == ' ' || s[j - 1] == '\t' || s[j - 1] == '\r' || s[j - 1] == '\n')) j--;
        return s.substr(i, j - i);
    }

    // Parse: #import "file.splh"
    static bool parseImportLine(const std::string& line, std::string& outFile) {
        std::string t = trim(line);
        if (t.rfind("#import", 0) != 0) return false;

        // find first quote
        size_t q1 = t.find('"');
        if (q1 == std::string::npos) return false;
        size_t q2 = t.find('"', q1 + 1);
        if (q2 == std::string::npos) return false;

        outFile = t.substr(q1 + 1, q2 - (q1 + 1));
        return !outFile.empty();
    }

    // Recursively expand imports.
    // currentPath = base directory for relative imports.
    static bool preprocessImportFile(
        const std::string& source,
        const std::string& currentPath,
        std::unordered_set<std::string>& alreadyImported,
        std::string& outExpanded)
    {
        std::stringstream in(source);
        std::string line;
        std::ostringstream out;

        while (std::getline(in, line)) {
            std::string imp;
            if (parseImportLine(line, imp)) {
                std::string full = joinPath(currentPath, imp);

                // avoid duplicate imports
                if (alreadyImported.find(full) != alreadyImported.end()) {
                    continue;
                }
                alreadyImported.insert(full);

                std::string importedCode;
                std::ifstream fs(full);
                if (!fs) {
                    std::cerr << "SPL error: could not open import '" << full << "'\n";
                    return false;
                }
                std::stringstream buf;
                buf << fs.rdbuf();
                importedCode = buf.str();

                // recurse using imported file's directory as base
                std::string importedExpanded;
                if (!preprocessImportFile(importedCode, dirnameOf(full), alreadyImported, importedExpanded))
                    return false;

                out << "\n// --- begin import: " << full << " ---\n";
                out << importedExpanded;
                out << "\n// --- end import: " << full << " ---\n\n";
            }
            else {
                out << line << "\n";
            }
        }

        outExpanded = out.str();
        return true;
    }

} // anonymous namespace

namespace SPL {

    bool loadCodeFromFile(const std::string& filepath, std::string& outSource) {
        std::ifstream sourceFileStream(filepath);
        if (!sourceFileStream) {
            std::cerr << "SPL error: could not open file '" << filepath << "'\n";
            return false;
        }
        std::stringstream buffer;
        buffer << sourceFileStream.rdbuf();
        outSource = buffer.str();
        return true;
    }


    void RunProjectCode(const std::string& source, const std::string& currentPath) {
        // --- OPTIONAL: auto-import std.splh if you want ---
        // If you *only* want explicit imports, delete this block.
        std::string src = source;
        if (src.find("#import \"std.splh\"") == std::string::npos) {
          // prepend std import
            src = "#import \"std.splh\"\n" + src;
        }
       
        // preprocess imports
        std::unordered_set<std::string> already;
        std::string expanded;
        if (!preprocessImportFile(src, currentPath, already, expanded)) {
            return;
        }

        // tokenize / parse / run
        Lexer lex(expanded);
        std::vector<Token*> toks = lex.tokenize();

        try {
            Parser parser(toks);
            std::vector<Module*> mods = parser.parseProgram();

            Interpreter interp;
            interp.runProgramEntry(mods, "main");
        }
        catch (const std::exception& e) {
            std::cerr << "SPL error: " << e.what() << "\n";
        }

        freeTokens(toks);
    }

    void RunFile(const std::string& filepath) {
        std::string code;
        if (!loadCodeFromFile(filepath, code)) return;

        // base directory for relative imports
        std::string base = dirnameOf(filepath);
        RunProjectCode(code, base);
    }

} // namespace SPL





