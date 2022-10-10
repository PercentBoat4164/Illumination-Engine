#pragma once

/* Predefine classes used with pointers or as return values for functions. */

namespace IE::Graphics { class Image; }
class IEBuffer;

class IERenderEngine;

/* Include classes used as attributes or function arguments. */
// Internal dependencies
#include "Image/Image.hpp"


// External dependencies
#include <vulkan/vulkan.h>

// System dependencies
#include <cstdint>
#include <functional>
#include <optional>
#include <variant>
#include <vector>

class IEDescriptorSet {
public:
    struct CreateInfo {
        // Required
        std::vector<VkDescriptorPoolSize>                               poolSizes{};
        std::vector<VkShaderStageFlagBits>                              shaderStages{};
        std::vector<std::optional<std::variant<IE::Graphics::Image *, IEBuffer *>>> data{};

        // Optional
        uint32_t maxIndex{1};

        // Required if maxIndex != 1
        VkDescriptorBindingFlagsEXT flags{0};
    };

    VkDescriptorPool      descriptorPool{};
    VkDescriptorSet       descriptorSet{};
    VkDescriptorSetLayout descriptorSetLayout{};
    CreateInfo            createdWith{};

    void destroy();

    void create(IERenderEngine *renderEngineLink, CreateInfo *createInfo);

    void update(
      std::vector<std::optional<std::variant<IE::Graphics::Image *, IEBuffer *>>> newData, std::vector<int> bindings = {});

	virtual ~IEDescriptorSet();

private:
    IERenderEngine                    *linkedRenderEngine{};
    std::vector<std::function<void()>> deletionQueue{};
};