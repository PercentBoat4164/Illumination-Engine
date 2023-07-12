#include "File.hpp"

#include <filesystem>
#include <iostream>

IE::Core::File::File(const std::filesystem::path &filePath) {
    m_path = filePath;
    m_name = m_path.filename().string();
    open();
    m_stream.ignore(std::numeric_limits<std::streamsize>::max());
    m_size = m_stream.gcount();
    m_stream.clear();  //  Since ignore will have set eof.
    m_stream.seekg(0, std::fstream::end);
    m_extension = m_path.extension().string();
}

IE::Core::File::File(File &&t_other) noexcept {
    m_name      = t_other.m_name;
    m_path      = t_other.m_path;
    m_size      = t_other.m_size;
    m_extension = t_other.m_extension;
}

IE::Core::File::File(const File &t_other) {
    m_name      = t_other.m_name;
    m_path      = t_other.m_path;
    m_size      = t_other.m_size;
    m_extension = t_other.m_extension;
}

IE::Core::File &IE::Core::File::operator=(const IE::Core::File &t_other) {
    if (this == &t_other) return *this;
    m_name      = t_other.m_name;
    m_path      = t_other.m_path;
    m_size      = t_other.m_size;
    m_extension = t_other.m_extension;
    return *this;
}

IE::Core::File::~File() {
    close();
}

void IE::Core::File::write(const std::vector<char> &data) {
    std::cout << "writing to file\n" << m_path.generic_string() << "\n";
    if (m_stream.is_open()) {
        std::cout << "file opened\n";
        m_stream.clear();
        m_stream << &data;
        m_size = m_stream.tellg();
    } else {
        std::cout << "file failed to open\n";
    }
}

void IE::Core::File::overwrite(const std::vector<char> &data, std::streamsize startPosition) {
    if (startPosition == -1)               // If no starting position
        startPosition = m_stream.tellg();  // Start here

    // Go to starting position
    m_stream.seekg(startPosition);

    // Write to file
    auto dataSize = static_cast<std::streamsize>(data.size());
    m_stream.write(data.data(), dataSize);

    // Update file length
    m_size = std::max(dataSize + startPosition, m_size);
}

void IE::Core::File::open(std::ios::openmode mode) {
    if (!m_stream.is_open()) {
        m_stream.open(m_path, mode);
        m_size = m_stream.tellg();
    }
}

void IE::Core::File::close() {
    if (m_stream.is_open()) m_stream.close();
}