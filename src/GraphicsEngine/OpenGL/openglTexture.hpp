#pragma once

#include "openglImage.hpp"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <../../../deps/stb_image.h>
#endif

class OpenGLTexture : public OpenGLImage{
public:
    void create(CreateInfo *createInfo) override {
        createdWith = *createInfo;
        deletionQueue.emplace_front([&] { stbi_image_free(data); });
        glGenTextures(1, &ID);
        deletionQueue.emplace_front([&] { glDeleteTextures(1, &ID); });
    }

    void destroy() override {
        for (const std::function<void()> &function : deletionQueue) { function(); }
        deletionQueue.clear();
    }

private:
    std::deque<std::function<void()>> deletionQueue{};
};