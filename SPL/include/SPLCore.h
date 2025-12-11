#pragma once

#include <string>
#include <unordered_set>

namespace SPL {
	bool loadCodeFromFile(const std::string& filepath, std::string& outSource);
	void RunProjectCode(const std::string& source);

	/*static std::string readImportFile(const std::string& path);
	static std::string preprocessImports(const std::string& source,const std::string& baseDir);
	static std::string preprocessImportSource(const std::string& source, const std::string& currentPath,
		std::unordered_set<std::string>& seen);*/

   
}
