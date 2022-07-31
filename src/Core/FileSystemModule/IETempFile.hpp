#pragma once

// logging
#include "Core/LogModule/IELogger.hpp"

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

/* old version:
class IETempFile {
public:
    std::string name{}; // the filename
    std::string data{}; // the data in the file
    std::fstream fileIO{}; // fstream used to read and write to real files
    int size{}; // the number of chars in the data string

    // Create a TempFile from a file on disk
    explicit IETempFile(const std::string& filename) {
        fileIO.open(filename);
        if(fileIO.is_open()) {
            data = readFileData();
            name = filename;
            size = data.size();
            std::cout << "Opened file";
        } else {
            std::cout << "File failed to open!";
        }
    }

    // Read data from the TempFile
    std::string read(int startPosition, std::streamsize numBytes) {

        std::string output;

        // Add all the bytes to the output string
        for(int i = startPosition; i < startPosition + numBytes && i < size; i++) {
            output += data[i];
        }

        return output;
    }

private:

    //set the data variable to the real file's data
    std::string readFileData() {
        std::string content((std::istreambuf_iterator<char>(fileIO)),
                            (std::istreambuf_iterator<char>()));
        return content;
    }
};
 */