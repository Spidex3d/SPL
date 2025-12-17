#pragma once

#include <string>
#include <unordered_set>

namespace SPL {
	bool loadCodeFromFile(const std::string& filepath, std::string& outSource);
	void RunProjectCode(const std::string& source, const std::string& currentPath = "");
	// Convenience: load + preprocess + run.
	void RunFile(const std::string& filepath);

   
}
