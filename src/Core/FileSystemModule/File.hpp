#pragma once

#include <../contrib/stb/stb_image.h>
#include <../contrib/rapidjson/include/rapidjson/document.h>
#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>


namespace IE::Core {
enum IE_FILE_TYPE {
    IE_FILE_TYPE_MODEL,
    IE_FILE_TYPE_IMAGE,
    IE_FILE_TYPE_AUDIO,
    IE_FILE_TYPE_JSON,
};

enum IE_FILESYSTEM_RESULT {
    IE_FILESYSTEM_RESULT_SUCCESS,
};

class File {
public:
    std::string           m_name;
    std::filesystem::path m_path;
    std::streamsize       m_size;
    std::string           m_extension;

    // Constructor
    explicit File(const std::filesystem::path &t_filePath);

    explicit File(File &&t_other);

    explicit File(const File &t_other);

    IE_FILESYSTEM_RESULT read(std::string &t_data, const std::streamsize &t_chars, const std::streamsize &t_pos = 0) {
        std::streamsize len = std::min(m_size, t_chars);
        t_data.resize(len);
        m_stream.seekg(t_pos, std::fstream::beg);
        m_stream.read(t_data.data(), len);
        return IE_FILESYSTEM_RESULT_SUCCESS;
    }

    IE_FILESYSTEM_RESULT
    read(std::vector<char> &t_data, const std::streamsize &t_chars, const std::streamsize &t_pos =0) {
        std::streamsize len = std::min(m_size, t_chars);
        t_data.resize(len);
        m_stream.seekg(t_pos, std::fstream::beg);
        m_stream.read(t_data.data(), len);
        return IE_FILESYSTEM_RESULT_SUCCESS;
    };

    IE_FILESYSTEM_RESULT append(const std::string t_data) {
        m_stream.seekg(0, std::fstream::end);
        m_stream.write(t_data.c_str(), t_data.size());
        m_size += t_data.size();
        return IE_FILESYSTEM_RESULT_SUCCESS;
    }

    IE_FILESYSTEM_RESULT insert(const std::string t_data, const std::streamsize &t_pos =0) {
        std::string stringEnd;
        stringEnd.resize(m_size - t_pos);
        m_stream.seekg(t_pos, std::fstream::beg);
        m_stream.read(stringEnd.data(), stringEnd.size());
        stringEnd = t_data + stringEnd;
        append(stringEnd);
        m_size += t_data.size();
        return IE_FILESYSTEM_RESULT_SUCCESS;
    }

    IE_FILESYSTEM_RESULT overwrite(const std::string t_data, const std::streamsize &t_pos =0) {
        m_stream.seekg(t_pos, std::fstream::beg);
        m_stream.write(t_data.c_str(), t_data.size());
        m_size = m_size - t_pos + t_data.size();
        return IE_FILESYSTEM_RESULT_SUCCESS;
    }

    IE_FILESYSTEM_RESULT erase(const std::streamsize &t_pos, const std::streamsize &t_chars) {
        // Figure out how to erase a block from the file.
    }

    template<IE_FILE_TYPE T = IE_FILE_TYPE_MODEL>
    const aiScene *import(unsigned int t_flags = 0) {
        return m_importer.ReadFile(m_path.string(), t_flags);
    }

    template<IE_FILE_TYPE T = IE_FILE_TYPE_IMAGE>
    stbi_uc *import() {
        return nullptr;
    }

    template<IE_FILE_TYPE T = IE_FILE_TYPE_AUDIO>
    std::vector<char> stream(size_t t_bufferSize) {
        return {};
    }

    template<IE_FILE_TYPE T = IE_FILE_TYPE_AUDIO>
    std::vector<char> import() {
        return {};
    }

    template<IE_FILE_TYPE T = IE_FILE_TYPE_JSON>
    rapidjson::Document import() {
        rapidjson::Document document;
        std::string str;
        read(str, -1);
        document.Parse(str.c_str());
        return document;
    }

private:
    Assimp::Importer m_importer;
    std::fstream     m_stream;

    // Open a file for reading and writing
    void open(std::ios::openmode mode = std::ios::in | std::ios::out | std::ios::binary);

    // Close the file
    void close();
};
}  // namespace IE::Core