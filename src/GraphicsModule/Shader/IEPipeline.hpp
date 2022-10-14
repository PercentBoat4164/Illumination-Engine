#pragma once

/* Predefine classes used with pointers or as return values for functions. */
class IERenderEngine;

class IEDescriptorSet;

class IERenderPass;

/* Include classes used as attributes or function arguments. */
// Internal dependencies
#include "GraphicsModule/Shader/IEShader.hpp"

// External dependencies
#include <vulkan/vulkan.h>

// System dependencies
#include <functional>
#include <vector>

class IEPipeline {
public:
    struct CreateInfo {
        // Required
        std::vector<std::shared_ptr<IEShader>> shaders{};
        std::weak_ptr<IEDescriptorSet>         descriptorSet{};
        std::weak_ptr<IERenderPass>            renderPass{};
    };

#ifndef NDEBUG
    struct Created {
        bool pipelineLayout{};
    } created;
#endif

    VkPipelineLayout pipelineLayout{};
    CreateInfo       createdWith{};
    VkPipeline       pipeline{};
    GLuint           programID{};

    void destroy();

    void create(IERenderEngine *engineLink, CreateInfo *createInfo);

    ~IEPipeline();

private:
    static std::function<void(IEPipeline &, IERenderEngine *, IEPipeline::CreateInfo *)> _create;

protected:
    void _vulkanCreate(IERenderEngine *, IEPipeline::CreateInfo *);

    void _openglCreate(IERenderEngine *, IEPipeline::CreateInfo *);

    IERenderEngine                    *linkedRenderEngine{};
    std::vector<std::function<void()>> deletionQueue{};
};