#include "File.hpp"

#include <iostream>

IE::Core::File::File(const std::filesystem::path &filePath) {
    path = filePath;
    name = filePath.filename().string();
    open(std::fstream::in | std::fstream::binary);
    fileIO.seekg(0, std::fstream::end);
    size      = fileIO.tellg();
    extension = filePath.extension().string();
    close();
}

IE::Core::File::File(const IE::Core::File &file) {
    name      = file.name;
    path      = file.path;
    size      = file.size;
    extension = file.extension;
}

IE::Core::File &IE::Core::File::operator=(const IE::Core::File &file) {
    if (this == &file) return *this;
    name      = file.name;
    path      = file.path;
    size      = file.size;
    extension = file.extension;
    return *this;
}

std::vector<char> IE::Core::File::read() {
    std::vector<char> data(size);
    open(std::fstream::in | std::fstream::binary);
    getSize();
    fileIO.seekg(0, std::fstream::beg);
    fileIO.read(data.data(), size);
    close();
    return data;
}

std::vector<char> IE::Core::File::read(std::streamsize numBytes, std::streamsize startPosition) {
    std::vector<char> data(size);
    open();
    fileIO.seekg(startPosition);
    fileIO.read(data.data(), numBytes);
    close();
    return data;
}

void IE::Core::File::write(const std::vector<char> &data) {
    std::cout << "writing to file\n" << path.generic_string() << "\n";
    open();
    if (fileIO.is_open()) {
        std::cout << "file opened\n";
        fileIO.clear();
        fileIO << &data;
        size = fileIO.tellg();
        close();
    } else {
        std::cout << "file failed to open\n";
    }
}

void IE::Core::File::overwrite(const std::vector<char> &data, std::streamsize startPosition) {
    open();
    if (startPosition == -1)             // If no starting position
        startPosition = fileIO.tellg();  // Start here

    // Go to starting position
    fileIO.seekg(startPosition);

    // Write to file
    auto dataSize = static_cast<std::streamsize>(data.size());
    fileIO.write(data.data(), dataSize);

    // Update file length
    size = std::max(dataSize + startPosition, size);
    close();
}

std::streamsize IE::Core::File::getSize() {
    fileIO.seekg(0, std::fstream::end);
    return size = fileIO.tellg();
}

void IE::Core::File::open(std::ios::openmode mode) {
    if (!fileIO.is_open()) {
        fileIO.open(path, mode);
        size = fileIO.tellg();
    }
}

void IE::Core::File::close() {
    if (fileIO.is_open()) fileIO.close();
}