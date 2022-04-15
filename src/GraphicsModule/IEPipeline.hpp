#pragma once

/* Predefine classes used with pointers or as return values for functions. */
class IERenderEngine;

class IEDescriptorSet;

class IERenderPass;

/* Include classes used as attributes or function arguments. */
// Internal dependencies
#include "IEShader.hpp"
#include "IEDependency.hpp"

// External dependencies
#include <vulkan/vulkan.h>

// System dependencies
#include <functional>
#include <vector>


class IEPipeline : public IEDependency {
public:
    struct CreateInfo {
        //Required
        std::vector<IEShader> *shaders{};
        IEDescriptorSet *descriptorSet{};
        IERenderPass *renderPass{};
    };

    #ifndef NDEBUG
    struct Created {
        bool pipelineLayout{};
    } created;
    #endif

    VkPipelineLayout pipelineLayout{};
    CreateInfo createdWith{};
    VkPipeline pipeline{};

    void destroy() final;

    void create(IERenderEngine *engineLink, CreateInfo *createInfo);

    ~IEPipeline() override;

private:
    IERenderEngine *linkedRenderEngine{};
    std::vector<std::function<void()>> deletionQueue{};
};