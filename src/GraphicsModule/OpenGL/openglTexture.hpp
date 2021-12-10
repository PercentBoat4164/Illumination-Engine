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

    void prepare(const char *filename = nullptr) {
        if (filename) { createdWith.filename = filename; }
        if (createdWith.data) { stbi_image_free(createdWith.data); }
        int channels{};
        createdWith.data = stbi_load(createdWith.filename.c_str(), &createdWith.width, &createdWith.height, &channels, STBI_rgb_alpha);
        deletionQueue.emplace_front([&] { stbi_image_free(createdWith.data); });
        if (!createdWith.data) { throw std::runtime_error("failed to prepare texture image from file: " + createdWith.filename); }
    }

    void upload() override {
        glBindTexture(GL_TEXTURE_2D, ID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, createdWith.mipMapping ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        if (createdWith.format == OPENGL_TEXTURE) { glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, createdWith.width, createdWith.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, createdWith.data); }
        if (createdWith.mipMapping) { glGenerateMipmap(GL_TEXTURE_2D); }
    }

    void destroy() override {
        for (const std::function<void()> &function : deletionQueue) { function(); }
        deletionQueue.clear();
        created = false;
    }

private:
    std::deque<std::function<void()>> deletionQueue{};
};