#pragma once

#include "openglImage.hpp"
#include "openglGraphicsEngineLink.hpp"

class OpenGLFramebuffer {
public:
    struct CreateInfo {
        //Required
        bool depth{};
    };

    CreateInfo createdWith{};
    GLuint ID{};
    OpenGLImage depthImage{};
    OpenGLImage colorImage{};
    OpenGLGraphicsEngineLink *linkedRenderEngine{};

    void create(OpenGLGraphicsEngineLink *engineLink, CreateInfo *createInfo) {
        linkedRenderEngine = engineLink;
        createdWith = *createInfo;
        glGenFramebuffers(1, &ID);
        glBindFramebuffer(GL_FRAMEBUFFER, ID);
        OpenGLImage::CreateInfo attachmentCreateInfo{OPENGL_COLOR, linkedRenderEngine->settings->resolution[0], linkedRenderEngine->settings->resolution[1]};
        colorImage.create(&attachmentCreateInfo);
        colorImage.upload();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorImage.ID, 0);
        if (createdWith.depth) {
            attachmentCreateInfo.format = OPENGL_DEPTH;
            depthImage.create(&attachmentCreateInfo);
            depthImage.upload();
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthImage.ID, 0);
        }
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) { throw std::runtime_error("framebuffer is incomplete!"); }
    }

    void clear() const {
        GLint oldDrawID{}, oldReadID{};
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &oldDrawID);
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &oldReadID);
        if (ID == oldDrawID && ID == oldReadID) {
            glClear(GL_COLOR_BUFFER_BIT | (createdWith.depth ? GL_DEPTH_BUFFER_BIT : 0));
            return;
        }
        glClear(GL_COLOR_BUFFER_BIT | (createdWith.depth ? GL_DEPTH_BUFFER_BIT : 0));
        glBindFramebuffer(GL_FRAMEBUFFER, ID);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER_BINDING, oldDrawID);
        glBindFramebuffer(GL_READ_FRAMEBUFFER_BINDING, oldReadID);
    }

    void destroy() {
        glDeleteFramebuffers(1, &ID);
    }
};