#pragma once

#include "openglImage.hpp"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif

class OpenGLTexture : public OpenGLImage{
public:
    bool created{false};

    void create(CreateInfo *createInfo) override {
        createdWith = *createInfo;
        glGenTextures(1, &ID);
        deletionQueue.emplace_front([&] { glDeleteTextures(1, &ID); });
        created = true;
    }

    void destroy() override {
        for (const std::function<void()> &function : deletionQueue) { function(); }
        deletionQueue.clear();
        created = false;
    }

private:
    std::deque<std::function<void()>> deletionQueue{};
};