#pragma once

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <../../../deps/stb_image.h>
#endif

class OpenGLTexture {
public:
    struct CreateInfo {
        //REQUIRED
        const char *filename{};
    };

    CreateInfo createdWith{};
    stbi_uc *data{};
    int channels{};
    GLuint ID{};
    GLsizei width{}, height{};
    GLint format{GL_RGBA};

    void create(CreateInfo *createInfo) {
        createdWith = *createInfo;
        stbi_set_flip_vertically_on_load(true);
        data = stbi_load(createdWith.filename, &width, &height, &channels, STBI_rgb_alpha);
    }

    void upload() {
        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_2D, ID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    void destroy() {
        stbi_image_free(data);
        glDeleteTextures(1, &ID);
    }
};