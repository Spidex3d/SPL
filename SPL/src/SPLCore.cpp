#include <fstream>
#include <sstream>
#include <filesystem>
#include <unordered_set>
#include <vector>
#include <iostream>

#include "../include/SPLCore.h"
#include "../include/lexer.h"
#include "../include/parsing.h"
#include "../include/interpreter.h"
#include "../include/splHelper.h"

namespace SPL {

    namespace fs = std::filesystem;

    // ------------------------------------------------------------
    // Small helpers (internal, not in the SPLCore header)
	// Will need something like pragma once support later to avoid multiple definitions.
    // And we will need to add a way to #import to import file to
    // ------------------------------------------------------------

    // Read a whole file into a string or throw on failure.
    static std::string readFileToString(const std::string& path)
    {
        std::ifstream in(path);
        if (!in) {
            throw std::runtime_error("Could not open file: " + path);
        }
        std::stringstream buffer;
        buffer << in.rdbuf();
        return buffer.str();
    }

    // Recursively preprocess a source string for #import / #include.
    // baseDir = directory relative to which imports are resolved.
    // 'seen' tracks full paths we've already included, to avoid cycles.
    static std::string preprocessImportSource(const std::string& source,
        const fs::path& baseDir,
        std::unordered_set<std::string>& seen)
    {
        std::stringstream input(source);
        std::stringstream output;

        std::string line;
        while (std::getline(input, line)) {
            std::string trimmed = line;

            // Trim leading spaces/tabs
            auto firstNonSpace = trimmed.find_first_not_of(" \t");
            if (firstNonSpace != std::string::npos) {
                trimmed.erase(0, firstNonSpace);
            }
            else {
                // Line is all whitespace; just pass it through
                output << line << "\n";
                continue;
            }

            // Check for #import or #include at the start of the trimmed line
            bool isImport = (trimmed.rfind("#import", 0) == 0);
            bool isInclude = (trimmed.rfind("#include", 0) == 0);

            if (isImport || isInclude) {
                // Expect format:   #import "file.splh"  or  #include "file.splh"
                auto firstQuote = trimmed.find('"');
                auto lastQuote = trimmed.find('"', firstQuote + 1);

                if (firstQuote == std::string::npos ||
                    lastQuote == std::string::npos ||
                    lastQuote <= firstQuote + 1)
                {
                    throw std::runtime_error("Invalid import/include directive: " + line);
                }

                std::string relPath = trimmed.substr(firstQuote + 1,
                    lastQuote - firstQuote - 1);

                fs::path fullPath = baseDir / relPath;
                // Normalize / absolutize
                fullPath = fs::absolute(fullPath).lexically_normal();
                std::string fullPathStr = fullPath.string();

                // Avoid re-importing the same file
                if (seen.count(fullPathStr)) {
                    // Already included once; skip
                    continue;
                }
                seen.insert(fullPathStr);

                // Read imported file and preprocess its contents recursively
                std::string importedSource = readFileToString(fullPathStr);
                fs::path importedBaseDir = fullPath.parent_path();

                std::string expanded = preprocessImportSource(importedSource,
                    importedBaseDir,
                    seen);
                output << expanded << "\n";  // you can add a separator newline
            }
            else {
                // Normal line: copy as-is
                output << line << "\n";
            }
        }

        return output.str();
    }

    // ------------------------------------------------------------
    // Public API
    // ------------------------------------------------------------

    bool loadCodeFromFile(const std::string& filepath, std::string& outSource)
    {
        std::ifstream sourceFileStream(filepath);
        if (!sourceFileStream) {
            std::cerr << "Error: could not open .spl file " << filepath << std::endl;
            return false;
        }

        std::stringstream buffer;
        buffer << sourceFileStream.rdbuf(); // simpler way to read entire file
        outSource = buffer.str();
        return true;
    }

    // Runs SPL code from a source string in the editor.
    // This now preprocesses #import / #include before lexing.
    void RunProjectCode(const std::string& source)
    {
        try {
            // Preprocess imports relative to current working directory
            // (If you later know the real file path, you can pass its parent instead)
            fs::path baseDir = fs::current_path();
            std::unordered_set<std::string> seen;
            std::string preprocessed = preprocessImportSource(source, baseDir, seen);

            Lexer lex(preprocessed);
            std::vector<Token*> tokens = lex.tokenize();

            Parser parser(tokens);
            std::vector<Module*> mods = parser.parseProgram();

            Interpreter interp;
            interp.runProgramEntry(mods, "main");

            freeTokens(tokens);
            // If we later add destructors or cleanup for Module*, do it here.
        }
        catch (const std::exception& e) {
            std::cerr << "SPL error: " << e.what() << "\n";
        }
    }

} // namespace SPL



