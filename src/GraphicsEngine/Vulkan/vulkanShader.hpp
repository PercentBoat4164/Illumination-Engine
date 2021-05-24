#pragma once

#include "vulkanGraphicsEngineLink.hpp"

#include <fstream>
#include <cstring>

#if defined(_WIN32)
#define GLSLC "glslc.exe "
#else
#define GLSLC "glslc "
#endif

class Shader {
public:
    struct CreateInfo {
        const char *filename{};
        VkShaderStageFlagBits stage{};
        std::vector<char> data{};
    };

    VkShaderModule module{};
    VulkanGraphicsEngineLink *linkedRenderEngine{};
    CreateInfo *createdWith{};

    void destroy() const {
        vkDestroyShaderModule(linkedRenderEngine->device->device, module, nullptr);
    }

    void create(VulkanGraphicsEngineLink *renderEngineLink, CreateInfo *createInfo) {
        linkedRenderEngine = renderEngineLink;
        createdWith = createInfo;
        if (createdWith->filename != nullptr) {
            createdWith->data.clear();
            std::string compiledFileName = ((std::string)createdWith->filename).substr(0, std::string(createdWith->filename).find_last_of('.')) + ".spv";
            if (system((GLSLC + (std::string)createdWith->filename + " -o " + compiledFileName + " --target-env=vulkan1.2").c_str()) != 0) { throw std::runtime_error("failed to compile Shaders!"); }
            std::ifstream file(compiledFileName, std::ios::ate | std::ios::binary);
            if (!file.is_open()) { throw std::runtime_error("failed to open file: " + compiledFileName.append("\n as file: " + compiledFileName)); }
            size_t fileSize = (size_t) file.tellg();
            createdWith->data.resize(fileSize);
            file.seekg(0);
            file.read(createdWith->data.data(), (std::streamsize)fileSize);
            file.close();
        }
        VkShaderModuleCreateInfo shaderModuleCreateInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        shaderModuleCreateInfo.codeSize = createdWith->data.size();
        shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t *>(createdWith->data.data());
        if (vkCreateShaderModule(linkedRenderEngine->device->device, &shaderModuleCreateInfo, nullptr, &module) != VK_SUCCESS) { throw std::runtime_error("failed to create shader module!"); }
    }
};