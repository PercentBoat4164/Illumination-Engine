#pragma once

/* Predefine classes used with pointers or as return values for functions. */
class IERenderEngine;

/* Include classes used as attributes or function arguments. */
// Internal dependencies
#include "GraphicsModule/Image/IEFramebuffer.hpp"

// External dependencies
#include <vulkan/vulkan.h>

// System dependencies
#include <cstdint>
#include <vector>
#include <functional>


class IERenderEngine;

class IERenderPass {
public:
    struct CreateInfo {
        uint8_t msaaSamples{1};
    } createdWith;

    VkRenderPass renderPass{};
    std::vector<IEFramebuffer> framebuffers{};

    void create(IERenderEngine *engineLink, CreateInfo *createInfo);

    VkRenderPassBeginInfo beginRenderPass(const IEFramebuffer &framebuffer);

    void destroy();

    ~IERenderPass();

private:
    std::vector<std::function<void()>> deletionQueue{};
    IERenderEngine *linkedRenderEngine{};
};
