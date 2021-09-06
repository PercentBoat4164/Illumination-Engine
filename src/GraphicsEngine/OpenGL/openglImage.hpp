#pragma once

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif

enum OpenGLImageType {
    OPENGL_DEPTH = 0x00000000,
    OPENGL_COLOR = 0x00000001,
    OPENGL_TEXTURE = 0x00000002
};

class OpenGLImage {
public:
    struct CreateInfo {
        //Required
        OpenGLImageType format{};

        //Only required if format != OPENGL_TEXTURE_*
        int width{}, height{};
        bool mipMapping{true};
        stbi_uc *data{};

        //Only required if format == OPENGL_TEXTURE_*
        std::string filename{};
    };

    CreateInfo createdWith{};
    GLuint ID{};

    virtual void create(CreateInfo *createInfo) {
        createdWith = *createInfo;
        glGenTextures(1, &ID);
        deletionQueue.emplace_front([&] { glDeleteTextures(1, &ID); });
    }

    void unload() {
        destroy();
        glGenTextures(1, &ID);
    }

    void upload() {
        glBindTexture(GL_TEXTURE_2D, ID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, createdWith.mipMapping ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        if (createdWith.format == OPENGL_COLOR) { glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, createdWith.width, createdWith.height, 0, GL_RGB, GL_UNSIGNED_BYTE, createdWith.data); }
        if (createdWith.format == OPENGL_DEPTH) { glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, createdWith.width, createdWith.height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, createdWith.data); }
        if (createdWith.format >= OPENGL_TEXTURE) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, createdWith.width, createdWith.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, createdWith.data);
            deletionQueue.emplace_front([&] { stbi_image_free(createdWith.data); });
        }
        if (createdWith.mipMapping) { glGenerateMipmap(GL_TEXTURE_2D); }
    }

    virtual void destroy() {
        for (const std::function<void()> &function : deletionQueue) { function(); }
        deletionQueue.clear();
    }

private:
    std::deque<std::function<void()>> deletionQueue{};
};