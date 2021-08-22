#pragma once

#include "vulkanGraphicsEngineLink.hpp"

#include <fstream>
#include <cstring>

#if defined(_WIN32)
#define GLSLC "glslc.exe "
#else
#define GLSLC "glslc "
#endif

class VulkanShader {
public:
    struct CreateInfo {
        //Required
        std::string filename{};
    };

    std::vector<char> data{};
    std::deque<std::function<void()>> deletionQueue{};
    VkShaderModule module{};
    VulkanGraphicsEngineLink *linkedRenderEngine{};
    CreateInfo createdWith{};
    bool compiled{false};

    void destroy() {
        for (const std::function<void()>& function : deletionQueue) { function(); }
        deletionQueue.clear();
    }

    void create(VulkanGraphicsEngineLink *renderEngineLink, CreateInfo *createInfo) {
        linkedRenderEngine = renderEngineLink;
        createdWith = *createInfo;
        std::ifstream precompiledFile(createdWith.filename);
        compiled = precompiledFile.good();
        compile();
        std::string compiledFileName = createdWith.filename + ".spv";
        std::ifstream compiledFile(compiledFileName, std::ios::ate | std::ios::binary);
        if (!compiledFile.is_open()) { throw std::runtime_error("failed to open file: " + compiledFileName); }
        long fileSize = compiledFile.tellg();
        data.resize(fileSize);
        compiledFile.seekg(0);
        compiledFile.read(data.data(), fileSize);
        compiledFile.close();
        VkShaderModuleCreateInfo shaderModuleCreateInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        shaderModuleCreateInfo.codeSize = data.size();
        shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t *>(data.data());
        if (vkCreateShaderModule(linkedRenderEngine->device->device, &shaderModuleCreateInfo, nullptr, &module) != VK_SUCCESS) { throw std::runtime_error("failed to create shader module!"); }
        deletionQueue.emplace_front([&] { vkDestroyShaderModule(linkedRenderEngine->device->device, module, nullptr); });
    }

    void compile() {
        std::string filenamePrefix = createdWith.filename.substr(0, createdWith.filename.substr(0, createdWith.filename.find_last_of('/')).find_last_of('/'));
        std::string filenameSuffix = createdWith.filename.substr(createdWith.filename.find_last_of('/'), createdWith.filename.length() - createdWith.filename.find_last_of('/'));
        if (linkedRenderEngine->settings->rayTracing) { createdWith.filename = filenamePrefix + "/VulkanRayTracingShaders" + filenameSuffix; } else { createdWith.filename = filenamePrefix + "/VulkanRasterizationShaders" + filenameSuffix; }
        std::ifstream rawFile(createdWith.filename, std::ios::ate | std::ios::binary);
        if (!rawFile.is_open()) { throw std::runtime_error("failed to open file: " + std::string(createdWith.filename)); }
        rawFile.close();
        data.clear();
        std::string compiledFileName = createdWith.filename + ".spv";
        if (system((GLSLC + createdWith.filename + " -o " + compiledFileName + " --target-env=vulkan1.2").c_str()) != 0) { if (compiled) { std::cout << "Could not compile shaders...using pre-compiled shaders instead." << std::endl; } else { throw std::runtime_error("failed to compile shaders: " + createdWith.filename); } }
        compiled = true;
    }
};