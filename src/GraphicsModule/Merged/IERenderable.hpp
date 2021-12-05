#pragma once

#include <glm.hpp>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <sstream>
#include "IEVertex.hpp"

class IERenderable {
public:
    std::string filePath{};
    IERenderEngineLink* linkedRenderEngine;
    std::vector<std::string> data{};

    void create(IERenderEngineLink* engineLink, const std::string& file) {
        linkedRenderEngine = engineLink;
        filePath = file;
        std::ifstream fileObject{filePath, std::ios::in};
        std::stringstream contents{};
        contents << fileObject.rdbuf();
        fileObject.close();
        std::string contentsString{};
        contentsString = contents.str();
        while(!contentsString.empty()) {
            std::size_t pointer{contentsString.find_first_of('\n')};
            data.push_back(contentsString.substr(0, pointer));
            contentsString = contentsString.substr(pointer, contentsString.size() - pointer);
        }
    }
};
