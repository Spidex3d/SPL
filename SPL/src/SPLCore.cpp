#include "../include/SPLCore.h"

#include <fstream>
#include <sstream>
#include <unordered_set>
#include <vector>
#include <iostream>
#include <algorithm>
#include <filesystem>

#include "../include/lexer.h"
#include "../include/parsing.h"
#include "../include/interpreter.h"
#include "../include/splHelper.h"


namespace fs = std::filesystem;

// ----------------- small helpers -----------------
// trims whitespace from both ends of a string
static inline std::string trim(const std::string& s) {
    size_t a = 0;
    while (a < s.size() && std::isspace((unsigned char)s[a])) a++;
    size_t b = s.size();
    while (b > a && std::isspace((unsigned char)s[b - 1])) b--;
    return s.substr(a, b - a);
}
// checks if string s starts with prefix
static bool startsWith(const std::string& s, const std::string& prefix) {
    return s.size() >= prefix.size() && std::equal(prefix.begin(), prefix.end(), s.begin());
}

// Accept: #import std.splh
//     or: #import "std.splh"
static bool parseImportLine(const std::string& line, std::string& outFile) {
    std::string t = trim(line);
    if (!startsWith(t, "#import")) return false;
	// get rest of line after #import
    std::string rest = trim(t.substr(std::string("#import").size()));
    if (rest.empty()) return false;

    // strip optional quotes
    if (rest.size() >= 2 && rest.front() == '"' && rest.back() == '"') {
        rest = rest.substr(1, rest.size() - 2);
        rest = trim(rest);
    }

    outFile = rest;
    return !outFile.empty();
}

// Recursively expands imports and returns a combined source.
// All imports are resolved relative to baseDir.
// importedPaths prevents duplicates.
static bool preprocessImportsRec(const std::string& source,
    const fs::path& baseDir,
    std::unordered_set<std::string>& importedPaths,
    std::string& outCombined)
{   
	// read source line by line
    std::istringstream in(source);
    std::string line;

    std::string combined;
	// process line by line
    while (std::getline(in, line)) {
        std::string importName;
        if (parseImportLine(line, importName)) {
            // Only allow .splh (your current rule)
            fs::path importPath = baseDir / importName;

            // If user wrote "#import std" you could auto-append .splh;
            // but you asked for explicit .splh. So we enforce it:
            if (importPath.extension() != ".splh") {
                std::cerr << "SPL error: import must end with .splh: " << importName << "\n";
                return false;
            }

            std::error_code ec;
            importPath = fs::weakly_canonical(importPath, ec);
            if (ec) importPath = fs::absolute(importPath);

            std::string key = importPath.string();
            if (importedPaths.find(key) != importedPaths.end()) {
                // already imported, skip
                continue;
            }
            importedPaths.insert(key);

            // load import file
            std::ifstream f(importPath);
            if (!f) {
                std::cerr << "SPL error: could not open import file: " << importPath.string() << "\n";
                return false;
            }
            std::stringstream buf;
            buf << f.rdbuf();
            std::string importedSource = buf.str();

            // recursively expand imports inside imported file, using its directory
            fs::path nextBase = importPath.parent_path();
            std::string expandedImport;
            if (!preprocessImportsRec(importedSource, nextBase, importedPaths, expandedImport))
                return false;

            combined += expandedImport;
            combined += "\n";
            continue;
        }

        // non-import line
        combined += line;
        combined += "\n";
    }

    outCombined = std::move(combined);
    return true;
}

// Runs tokenizing/parsing/interpreting on already-preprocessed source
static bool runPipeline(const std::string& finalSource) {
    try {
        Lexer lex(finalSource);
        std::vector<Token*> toks = lex.tokenize();

        Parser parser(toks);
        std::vector<Module*> mods = parser.parseProgram();

        Interpreter interp;
        interp.runProgramEntry(mods, "main");

        freeTokens(toks);
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "SPL error: " << e.what() << "\n";
        return false;
    }
}

// ----------------- SPL API -----------------

namespace SPL {
	// Loads whole file to outSource (returns true on success)
    bool loadCodeFromFile(const std::string& filepath, std::string& outSource) {
        std::ifstream sourceFileStream(filepath);
		// check if file opened successfully
        if (!sourceFileStream) {
            std::cerr << "SPL error: could not open file: " << filepath << "\n";
            return false;
        }
		// read whole file
        std::stringstream buffer;
		// put file content into buffer
        buffer << sourceFileStream.rdbuf();
        outSource = buffer.str();
        return true;
    }
	// Run already-loaded source code with a base directory for resolving #import
    bool RunSource(const std::string& source, const std::string& baseDir) {
        std::unordered_set<std::string> imported;
        std::string finalSource;
		// determine base path
        fs::path base = baseDir.empty() ? fs::current_path() : fs::path(baseDir);
		// preprocess imports
        if (!preprocessImportsRec(source, base, imported, finalSource))
            return false;

        return runPipeline(finalSource);
    }
	// Run a file by path (this is what you want from the editor)
    bool RunFile(const std::string& filepath) {
        std::string source;
        if (!loadCodeFromFile(filepath, source))
            return false;

        fs::path p(filepath);
        fs::path base = p.has_parent_path() ? p.parent_path() : fs::current_path();

        return RunSource(source, base.string());
    }

} // namespace SPL









