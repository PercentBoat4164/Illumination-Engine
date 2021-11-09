#pragma once

#include <cstring>

class IeShader {
public:
    struct CreateInfo {
        //Required
        std::string filename{};
    };
};