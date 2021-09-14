#pragma once

#include "RenderEngineLink.hpp"

class RenderEngine {
    explicit RenderEngine(const std::string& API = "OpenGL") {
        engineData.api.name = API;
    }

    RenderEngineLink engineData;
};