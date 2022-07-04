#pragma once

#include "Core/LogModule/IELogger.hpp"

#include <vector>
#include <string>
#include <filesystem>
#include <fstream>


class IEFile {
public:

    std::string name{};
    std::filesystem::path path{};
    std::fstream fileIO{};
    std::streamsize size{};
    std::string extension{};

    IEFile(IEFile& file) {
        name = file.name;
        path = file.path;
        size = file.size;
        extension = file.extension;
    }

    IEFile(std::filesystem::path filePath) {
        path = filePath;
        fileIO.open(filePath);
        size = fileIO.tellg();
        extension = filePath.extension().string();
        fileIO.close();
    }

    //returns the file's data
    std::string read() {
        fileIO.open(path);
        std::string data((std::istreambuf_iterator<char>(fileIO)),
                            (std::istreambuf_iterator<char>()));
        fileIO.close();
        return data;
    }

    //overwrites the file
    void write(const std::string& data) {
        fileIO.clear();
        fileIO << data;
    }

    void append(const std::string& data) {
        fileIO << data;
    }

private:
};


/* old version:
    IEFile() = default;

    explicit IEFile(const std::string& initialPath) {
        // Record this filename
        path = initialPath;
    }

    std::streamsize length{};
    std::string path{};
    std::fstream file{};

    std::string getDirectory() const {
        return path.substr(0, path.find_last_of('/'));
    }

    void open() {
        if (file.is_open()) {
            return;
        }
        file.open(path, std::ios_base::in | std::ios_base::out | std::ios_base::ate);
        length = file.tellg();
    }

    void close() {
        if (!file.is_open()) {
            return;
        }
        file.close();
    }

    std::string read(std::streamsize numBytes, std::streamsize startPosition=-1) {
        if (startPosition == -1) {  // If no starting position
            startPosition = file.tellg();  // Start here
        }
        // Go to start position
        file.seekg(startPosition);

        // Create space for contents
        char bytes[numBytes];

        // Read the specified number of bytes into the char array
        file.read(bytes, numBytes);

        // Convert to string
        std::string contents(bytes);
        contents.resize(numBytes);  // This is required as two extra bytes are appended to the end of the string during conversion.
        return contents;
    }

    void insert(const std::string& data, std::streamsize startPosition=-1) {
        if (startPosition > length) {
            // Attempt to insert from beyond the length of the file!
            return;
        }
        if (startPosition == -1) {  // If no starting position
            startPosition = file.tellg();  // Start here
        }

        // Read data that is about to be overwritten
        std::string bytes = read(length - startPosition, startPosition);

        // Go to starting position
        file.seekg(startPosition);


        // Write entirety of data and file contents after data
        overwrite(data + bytes, startPosition);

        // Update file length
        auto dataSize = static_cast<std::streamsize>(startPosition + data.size());
        length = static_cast<std::streamsize>(dataSize + bytes.size());

        // Go to end of new data
        file.seekg(dataSize);
    }

    void overwrite(const std::string& data, std::streamsize startPosition=-1) {
        if (startPosition == -1) {  // If no starting position
            startPosition = file.tellg();  // Start here
        }

        // Go to starting position
        file.seekg(startPosition);

        // Write to file
        auto dataSize = static_cast<std::streamsize>(data.size());
        file.write(data.data(), dataSize);

        // Update file length
        length = std::max(dataSize + startPosition, length);
    }

    void erase(std::streamsize numBytes, std::streamsize startPosition=-1) {
        if (startPosition == -1) {  // If no starting position
            startPosition = file.tellg();  // Start here
        }

        // Go to where file should stop being erased
        file.seekg(startPosition + numBytes);

        // Read to EOF into a buffer
        std::string bytes{read(length - file.tellg())};

        // Resize file to the size of only the data to be kept
        std::filesystem::resize_file(path, startPosition);

        // Write the read data back to the file
        length = 0;  // Set length to zero so that it will be updated properly in the overwrite function
        overwrite(bytes, startPosition);
    }

    ~IEFile() {
        close();
    }

    IEFile& operator=(const IEFile& other) {
        // Copy over data from other
        path = other.path;
        length = other.length;

        // Close this file to avoid memory leaks
        close();

        // Open the new file
        open();
        return *this;
    }
};*/