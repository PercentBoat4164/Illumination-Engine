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

    std::vector<IEDirectory*> subdirectories{};
    std::vector<IEFile*> files{};
    std::string path{};
    bool exists{};

    void createDirectory() const {
        for (int i = 0; i < path.size(); ++i) {
            if (path[i] == '/') {
                std::filesystem::create_directory(path.substr(0, i));
            }
        }
        std::filesystem::create_directory(path);
    }

    std::vector<IEDirectory*> allDirectories() {
        std::vector<IEDirectory*> allSubDirectories{subdirectories};
        for (IEDirectory* subDirectory : subdirectories) {
            std::vector<IEDirectory*> theseSubDirectories{subDirectory->allDirectories()};
            allSubDirectories.insert(allSubDirectories.end(), theseSubDirectories.begin(), theseSubDirectories.end());
        }
        return allSubDirectories;
    }

    std::vector<IEFile*> allFiles() {
        std::vector<IEFile*> allFiles{files};
        for (IEDirectory* subDirectory : subdirectories) {
            std::vector<IEFile*> theseFiles{subDirectory->allFiles()};
            allFiles.insert(allFiles.end(), theseFiles.begin(), theseFiles.end());
        }
        return allFiles;
    }

    void create() {
        createDirectory();
        exists = true;
    }

    void remove() {
        std::filesystem::remove_all(path);
        exists = false;
    }
};