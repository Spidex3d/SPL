#pragma once

#include <string>
#include <unordered_set>

namespace SPL {
	
	// Loads whole file to outSource (returns true on success)
	bool loadCodeFromFile(const std::string& filepath, std::string& outSource);

	// Run a file by path (this is the file you want from the editor)
	bool RunFile(const std::string& filepath);

	// Run already-loaded source code with a base directory for resolving #import
	// At the moment all files are assumed to be in the same directory (baseDir)
	// This could be improved later with virtual file system support (#import\imports\stb.splh)
	bool RunSource(const std::string& source, const std::string& baseDir);



   
}
