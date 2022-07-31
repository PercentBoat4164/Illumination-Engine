#pragma once

//internal dependencies
#include "IEFile.hpp"

// std dependencies
#include <vector>
#include <string>
#include <fstream>

/**
 * A collection of data stored in RAM
 */

class IETempFile {
public:
    std::string name{};
    std::vector<char> data{}; // the data in the file
    std::string extension{};

    explicit IETempFile(const std::string& fileName, const std::vector<char>& fileData = {}) {
        name = fileName;
        data = fileData;
        if(int extensionStart = fileName.find_last_of('.') != std::string::npos) {
            extension = name.substr(extensionStart);
        }
    }

    explicit IETempFile(IEFile file) {
        name = file.name;
        extension = file.extension;
        data = file.read();
    }

    [[nodiscard]] std::vector<char> read() const {
        return data;
    }

    [[nodiscard]] std::vector<char> read(std::streamsize numBytes, std::streamsize startPosition) const {
        return {data.begin() + startPosition, data.begin() + startPosition + numBytes};
    }

    void write(const std::vector<char> &newData) {
        data = newData;
    }

    void overwrite(const std::vector<char> &newData, std::streamsize numBytes) {
        data = newData;
    }

    void getFileData(IEFile file) {
        data = file.read();
    }

    IEFile *createRealFile(const std::filesystem::path& path) {
        auto *file = new IEFile{path};
        file->write(data);
        return file;
    }
};