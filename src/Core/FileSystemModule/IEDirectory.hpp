#pragma once

#include "IEFile.hpp"

#include <vector>
#include <functional>


class IEDirectory {
public:
    /**
     * @brief Default constructor
     */
    IEDirectory() = default;

    /**
     * @brief Creates a new IEDirectory with a path
     * @param initialPath
     */
    explicit IEDirectory(const std::string& initialPath) {
        path = initialPath;
    }

    std::vector<IEDirectory*> subDirectories{};
    std::vector<IEFile*> files{};
    std::string path{};
    bool exists{};

    std::vector<IEDirectory*> allDirectories() {
        std::vector<IEDirectory*> allSubDirectories{subDirectories};
        for (IEDirectory* subDirectory : subDirectories) {
            std::vector<IEDirectory*> theseSubDirectories{subDirectory->allDirectories()};
            allSubDirectories.insert(allSubDirectories.end(), theseSubDirectories.begin(), theseSubDirectories.end());
        }
        return allSubDirectories;
    }

    std::vector<IEFile*> allFiles() {
        std::vector<IEFile*> allFiles{files};
        for (IEDirectory* subDirectory : subDirectories) {
            std::vector<IEFile*> theseFiles{subDirectory->allFiles()};
            allFiles.insert(allFiles.end(), theseFiles.begin(), theseFiles.end());
        }
        return allFiles;
    }

    void create() {
        exists = true;
    }

    void remove() {
        exists = false;
    }
};
