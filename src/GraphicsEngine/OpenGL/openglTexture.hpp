#pragma once

#include "openglImage.hpp"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <../../../deps/stb_image.h>
#endif

class OpenGLTexture : public OpenGLImage{
public:
    int channels{};

    void create(CreateInfo *createInfo) override {
        createdWith = *createInfo;
        stbi_set_flip_vertically_on_load(true);
        data = stbi_load(createdWith.filename, &createdWith.width, &createdWith.height, &channels, STBI_rgb_alpha);
        glGenTextures(1, &ID);
    }

    void destroy() override {
        stbi_image_free(data);
        glDeleteTextures(1, &ID);
    }
};