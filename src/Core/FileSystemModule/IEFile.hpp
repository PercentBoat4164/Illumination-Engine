#include <vector>
#include <string>

class IEFile {
private:
    bool exist{};

public:
    std::streamsize length{};
    std::string path{};
    std::fstream file;

    IEFile() = default;

    explicit IEFile(const std::string& filename) {
        // Record this filename
        path = filename;

        // Open the file to prepare for reading and writing
        file.open(path, std::ios_base::in | std::ios_base::out);

        // Get file length
        length = file.tellg();

        // This file now exists
        exist = true;
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

    bool insert(std::basic_string<char> data, std::streamsize startPosition=-1) {
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

    bool overwrite(const std::string& data, std::streamsize startPosition=-1) {
        if (startPosition == -1) {  // If no starting position
            startPosition = file.tellg();  // Start here
        }

        // Write to file.
        file.write(data.data(), static_cast<std::streamsize>(data.size()));
    }

    bool exists() const {
        return exist;
    }

    ~IEFile() {
        file.close();
    }
};