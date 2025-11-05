#pragma once

#include <string>

namespace SPL {

	bool loadCodeFromFile(const std::string& filepath, std::string& outSource);
	//bool GetProjectCode(const std::string& projectPath, const std::string& projectMainFile);
    void RunProjectCode(const std::string& source);
}
