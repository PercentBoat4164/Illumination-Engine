#pragma once

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

        //Only required if format != OPENGL_TEXTURE
        GLsizei width{}, height{};

        //Only required if format == OPENGL_TEXTURE
        const char *filename{};
    };

    CreateInfo createdWith{};
    stbi_uc *data{};
    GLuint ID{};

    virtual void create(CreateInfo *createInfo) {
        createdWith = *createInfo;
        glGenTextures(1, &ID);
    }

    virtual void upload() {
        glBindTexture(GL_TEXTURE_2D, ID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        if (createdWith.format == OPENGL_COLOR) { glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, createdWith.width, createdWith.height, 0, GL_RGB, GL_UNSIGNED_BYTE, data); }
        if (createdWith.format == OPENGL_DEPTH) { glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, createdWith.width, createdWith.height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, data); }
        if (createdWith.format == OPENGL_TEXTURE) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, createdWith.width, createdWith.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    virtual void destroy() {
        glDeleteTextures(1, &ID);
    }
};