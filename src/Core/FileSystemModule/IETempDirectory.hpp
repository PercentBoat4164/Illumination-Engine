#pragma once;

#include <vector>
#include <string>
#include <filesystem>
#include <fstream>

#include "IETempFile.hpp"

//A directory stored in RAM
class IETempDirectory {
private:
    std::vector<std::string> subdirectories;
    std::vector<IETempFile> files;
};