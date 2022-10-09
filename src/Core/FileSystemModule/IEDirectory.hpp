#pragma once

#include "IEFile.hpp"

#include <vector>

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
    explicit IEDirectory(const std::string &initialPath);

    std::vector<IEDirectory *> subDirectories{};
    std::vector<IEFile *>      files{};
    std::string                path{};
    bool                       exists{};

    void createDirectory() const;

    std::vector<IEDirectory *> allDirectories();

    std::vector<IEFile *> allFiles();

    void create();

    void remove();
};
