#pragma once

#include "openglSettings.hpp"

class OpenGLGraphicsEngineLink {
public:
    OpenGLSettings *settings{};
    int maxMSAASamples{1};
    std::string openglVersion{};
};