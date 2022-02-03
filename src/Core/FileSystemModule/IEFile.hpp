#pragma once

#include <vector>
#include <string>

class IEFile {
public:
    IEFile() = default;

    explicit IEFile(const std::string& initialPath) {
        // Record this filename
        path = initialPath;
    }

    std::streamsize length{};
    std::string path{};
    std::fstream file{};

    void open() {
        if (file.is_open()) {
            return;
        }
        file.open(path, std::ios_base::in | std::ios_base::out | std::ios_base::ate);
        if (!file.is_open()) {
            file.open(path, std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }
    }

    void close() {
        if (!file.is_open()) {
            return;
        }
        file.close();
    }

    std::string read(std::string& data, std::streamsize numBytes, std::streamsize startPosition=-1) {
        if (startPosition == -1) {  // If no starting position
            startPosition = file.tellg();  // Start here
        }
        // Go to start position
        file.seekg(startPosition);

        // Read the specified number of bytes into the contents string
        file.read(data.data(), numBytes);
        return data;
    }

    void insert(std::basic_string<char> data, std::streamsize startPosition=-1) {
        if (startPosition == -1) {  // If no starting position
            startPosition = file.tellg();  // Start here
        }

        // Read data that is about to be overwritten
        auto dataSize = static_cast<std::streamsize>(data.size());
        read(data, dataSize, startPosition);

        file.seekg(startPosition);  // Go to starting position
        file.write(data.data(), length - startPosition);  // Write entirety of data and file contents after data
        file.seekg(startPosition + dataSize);  // Go to where the new data ends
    }

    void overwrite(const std::string& data, std::streamsize startPosition=-1) {
        if (startPosition == -1) {  // If no starting position
            startPosition = file.tellg();  // Start here
        }

        // Write to file.
        file.write(data.data(), static_cast<std::streamsize>(data.size()));
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
};