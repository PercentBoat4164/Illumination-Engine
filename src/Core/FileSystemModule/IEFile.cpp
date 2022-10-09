#include "IEFile.hpp"

#include <iostream>

IEFile::IEFile(const std::filesystem::path &filePath) {
    path = filePath;
    name = filePath.filename().string();
    open(std::fstream::in | std::fstream::binary);
    fileIO.seekg(0, std::fstream::end);
    size      = fileIO.tellg();
    extension = filePath.extension().string();
    close();
}

IEFile::IEFile(const IEFile &file) {
    name      = file.name;
    path      = file.path;
    size      = file.size;
    extension = file.extension;
}

IEFile &IEFile::operator=(const IEFile &file) {
    if (this == &file) return *this;
    name      = file.name;
    path      = file.path;
    size      = file.size;
    extension = file.extension;
    return *this;
}

std::vector<char> IEFile::read() {
    open(std::fstream::in | std::fstream::binary);
    getSize();
    fileIO.seekg(0, std::fstream::beg);
    std::vector<char> *data =
      new std::vector<char>{std::istreambuf_iterator<char>(fileIO), std::istreambuf_iterator<char>()};
    close();
    return *data;
}

std::vector<char> IEFile::read(std::streamsize numBytes, std::streamsize startPosition) {
    open();
    fileIO.seekg(startPosition);
    std::vector<char> data;
    fileIO.read(data.data(), numBytes);
    fileIO.close();
    close();
    return data;
}

void IEFile::write(const std::vector<char> &data) {
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

void IEFile::overwrite(const std::vector<char> &data, std::streamsize startPosition) {
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

std::streamsize IEFile::getSize() {
    fileIO.seekg(0, std::fstream::end);
    return size = fileIO.tellg();
}

void IEFile::open(std::ios::openmode mode) {
    if (!fileIO.is_open()) {
        fileIO.open(path, mode);
        size = fileIO.tellg();
    }
}

void IEFile::close() {
    if (fileIO.is_open()) fileIO.close();
}