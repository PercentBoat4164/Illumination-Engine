#pragma once

#include "IEGraphicsLink.hpp"

#include <vulkan/vulkan.hpp>

#include <fstream>
#include <cstring>
#include <vector>
#include <deque>

#if defined(_WIN32)
#define GLSLC "glslc.exe "
#else
#define GLSLC "glslc "
#endif

class IEShader {
public:
    struct CreateInfo {
        //Required
        std::string filename{};
    };

    std::vector<char> data{};
    std::deque<std::function<void()>> deletionQueue{};
    VkShaderModule module{};
    IEGraphicsLink *linkedRenderEngine{};
    CreateInfo createdWith{};
    bool compiled{false};

    void destroy() {
        for (const std::function<void()>& function : deletionQueue) { function(); }
        deletionQueue.clear();
    }

    void create(IEGraphicsLink *renderEngineLink, CreateInfo *createInfo) {
        linkedRenderEngine = renderEngineLink;
        createdWith = *createInfo;
        std::string replaceWith = linkedRenderEngine->settings->rayTracing ? "RayTracing" : "Rasterizing";
        size_t pos = createdWith.filename.find('*');
        while( pos != std::string::npos) {
            createdWith.filename.replace(pos, 1, replaceWith);
            pos = createdWith.filename.find('*', pos + replaceWith.size());
        }
        replaceWith.~basic_string();
        data.clear();
        compile(createdWith.filename);
        compiled = true;
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
        if (vkCreateShaderModule(linkedRenderEngine->device.device, &shaderModuleCreateInfo, nullptr, &module) != VK_SUCCESS) { throw std::runtime_error("failed to create shader module!"); }
        deletionQueue.emplace_front([&] { vkDestroyShaderModule(linkedRenderEngine->device.device, module, nullptr); });
    }

    static void compile(const std::string& input, std::string output = "") {
        std::ifstream rawFile(input, std::ios::ate | std::ios::binary);
        if (!rawFile.is_open()) { throw std::runtime_error("failed to open file: " + input); }
        rawFile.close();
        if (output.empty()) { output = input + ".spv"; }
        if (system((GLSLC + input + " -o " + output + " --target-env=vulkan1.2").c_str()) != 0) { throw std::runtime_error("failed to compile shaders: " + input); }
    }
};