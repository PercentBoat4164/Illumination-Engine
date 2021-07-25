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
        deletionQueue.emplace_back([&] { stbi_image_free(data); });
        glGenTextures(1, &ID);
        deletionQueue.emplace_back([&] { glDeleteTextures(1, &ID); });
    }

    void destroy() override {
        for (const std::function<void()> &function : deletionQueue) { function(); }
        deletionQueue.clear();
    }

private:
    std::deque<std::function<void()>> deletionQueue{};
};