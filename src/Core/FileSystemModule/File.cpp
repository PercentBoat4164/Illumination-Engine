#include "File.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

IE::Core::File::File(const std::filesystem::path &t_filePath) {
    m_path = t_filePath;
    m_name = t_filePath.filename().string();
    if (!std::filesystem::is_directory(m_path)) {
        m_size = std::filesystem::file_size(m_path);
    }
    m_extension = t_filePath.extension().string();
}

IE::Core::File::File(File &&t_other) {
    if (this != &t_other) {
        m_path = t_other.m_path;
        m_size = t_other.m_size;
        m_extension = t_other.m_extension;
        m_name = t_other.m_name;
    }
}

IE::Core::File::File(const File &t_other) {
    if (this != &t_other) {
        m_path = t_other.m_path;
        m_size = t_other.m_size;
        m_extension = t_other.m_extension;
        m_name = t_other.m_name;
    }
}

void IE::Core::File::open(std::ios::openmode mode) {
    if (!m_stream.is_open())
        m_stream.open(m_path, mode);
    m_size = std::filesystem::file_size(m_path);
}

void IE::Core::File::close() {
    if (m_stream.is_open()) m_stream.close();
}