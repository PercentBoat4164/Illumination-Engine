#include "IEFile.hpp"

#include <vector>
#include <string>

class IEFileSystem {
public:
    std::vector<IEFile> files{};

    void addFile(const IEFile& file) {
        files.push_back(file);
    }
};