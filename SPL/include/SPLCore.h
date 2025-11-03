#pragma once

#include <string>

namespace SPL {

	bool loadCodeFromFile(const std::string& filepath, std::string& outSource);
    void RunCode(const std::string& source);
}
