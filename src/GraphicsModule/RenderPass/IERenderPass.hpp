#pragma once

/* Predefine classes used with pointers or as return values for functions. */
class IERenderEngine;

class IERenderPassBeginInfo;

/* Include classes used as attributes or function arguments. */
// Internal dependencies
#include "CommandBuffer/IECommandBuffer.hpp"
#include "IEFramebuffer.hpp"

// External dependencies
#include <vulkan/vulkan.h>

// System dependencies
#include <cstdint>
#include <functional>
#include <vector>


class IERenderEngine;

class IERenderPass : public IEDependency, public std::enable_shared_from_this<IERenderPass> {
public:
    struct CreateInfo {
        uint8_t msaaSamples{1};
    } createdWith;

    VkRenderPass                   renderPass{};
    std::shared_ptr<IEFramebuffer> framebuffer{};

    void create(IERenderEngine *engineLink, CreateInfo *createInfo);

    IERenderPassBeginInfo beginRenderPass(uint8_t index);

    void destroy();

    ~IERenderPass() override;

private:
    std::vector<std::function<void()>> deletionQueue{};
    IERenderEngine                    *linkedRenderEngine{};
};
