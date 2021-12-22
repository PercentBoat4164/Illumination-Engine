#pragma once

#include "IERenderEngineLink.hpp"

#ifdef ILLUMINATION_ENGINE_VULKAN
#include <spirv_cross/spirv_glsl.hpp>
#include <glslang/Public/ShaderLang.h>
#include <vulkan/vulkan.hpp>
#endif

#include <cstring>
#include <vector>
#include <utility>
#include <fstream>

//Possible valid shader extensions - source
#define IE_RENDER_ENGINE_SHADER_EXTENSION_GLSL "glsl"
#define IE_RENDER_ENGINE_SHADER_EXTENSION_HLSL "hlsl"

//Possible valid shader extensions - language
#define IE_RENDER_ENGINE_SHADER_EXTENSION_VERTEX "vert"
#define IE_RENDER_ENGINE_SHADER_EXTENSION_TESSELLATION_CONTROL "tesc"
#define IE_RENDER_ENGINE_SHADER_EXTENSION_TESSELLATION_EVALUATION "tese"
#define IE_RENDER_ENGINE_SHADER_EXTENSION_GEOMETRY "geom"
#define IE_RENDER_ENGINE_SHADER_EXTENSION_FRAGMENT "frag"
#define IE_RENDER_ENGINE_SHADER_EXTENSION_COMPUTE "comp"
#define IE_RENDER_ENGINE_SHADER_EXTENSION_RAY_GENERATION "rgen"


class IEShader {
public:
    struct CreateInfo {
        //Required
        std::string filename{};
    };

    struct Created {
        bool compiled{};
        bool module{};

        bool all() const {
            return compiled & module;
        }
    };

    CreateInfo createdWith{};
    Created created{};
    uint32_t shaderType{};
    IERenderEngineLink* linkedRenderEngine{};
    std::vector<uint32_t> data{};
    #ifdef ILLUMINATION_ENGINE_VULKAN
    VkShaderModule shaderModule{};
    EShLanguage language{};
    #endif

    IEShader() = default;

    explicit IEShader(IERenderEngineLink* engineLink, const CreateInfo& createInfo) {
        create(engineLink, createInfo);
    }

    explicit IEShader(IERenderEngineLink* engineLink, const std::string& filename) {
        create(engineLink, filename);
    }

    void create(IERenderEngineLink* engineLink, const CreateInfo& createInfo) {
        linkedRenderEngine = engineLink;
        createdWith = createInfo;
    }

    void create(IERenderEngineLink* engineLink, const std::string& filename) {
        linkedRenderEngine = engineLink;
        createdWith.filename = filename;
    }

    static std::vector<std::string> getExtensions(std::string& path) {
        size_t lastExtensionPosition{path.find_last_of('.')};
        std::vector<std::string> extensions{};
        while (lastExtensionPosition > 0) {
            extensions.push_back(path.substr(lastExtensionPosition, extensions.size() - lastExtensionPosition));
            lastExtensionPosition = path.find_last_of('.');
        }
        return extensions;
    }

    void compile() {
        #ifdef ILLUMINATION_ENGINE_VULKAN
        std::vector<std::string> extensions = getExtensions(createdWith.filename);
        glslang::EShSource source = glslang::EShSourceGlsl;
        for (const std::string& extension : extensions) {
            language = extension == IE_RENDER_ENGINE_SHADER_EXTENSION_VERTEX ? EShLangVertex : language;
            language = extension == IE_RENDER_ENGINE_SHADER_EXTENSION_FRAGMENT ? EShLangFragment : language;
            language = extension == IE_RENDER_ENGINE_SHADER_EXTENSION_TESSELLATION_CONTROL ? EShLangTessControl : language;
            language = extension == IE_RENDER_ENGINE_SHADER_EXTENSION_TESSELLATION_EVALUATION ? EShLangTessEvaluation : language;
            language = extension == IE_RENDER_ENGINE_SHADER_EXTENSION_GEOMETRY ? EShLangGeometry : language;
            language = extension == IE_RENDER_ENGINE_SHADER_EXTENSION_COMPUTE ? EShLangCompute : language;
            language = extension == IE_RENDER_ENGINE_SHADER_EXTENSION_RAY_GENERATION ? EShLangRayGen : language;
            source = extension == IE_RENDER_ENGINE_SHADER_EXTENSION_GLSL ? glslang::EShSourceGlsl : source;
            source = extension == IE_RENDER_ENGINE_SHADER_EXTENSION_HLSL ? glslang::EShSourceHlsl : source;
        }
        glslang::EShClient client;
        glslang::EshTargetClientVersion clientVersion;
        if (linkedRenderEngine->api.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
            client = glslang::EShClientVulkan;
            switch (linkedRenderEngine->api.version.minor) {
                case 0:
                    clientVersion = static_cast<glslang::EshTargetClientVersion>(linkedRenderEngine->api.version.number);
                    break;
                case 1:
                    clientVersion = glslang::EShTargetVulkan_1_1;
                    break;
                case 2:
                    clientVersion = glslang::EShTargetVulkan_1_2;
                    break;
            }
        }
        if (linkedRenderEngine->api.name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
            client = glslang::EShClientOpenGL;
            switch ((linkedRenderEngine->api.version.major << 4) + (linkedRenderEngine->api.version.minor << 4) + linkedRenderEngine->api.version.patch) {
                case (4 << 4) + (6 << 4) + (0):
                case (4 << 4) + (5 << 4) + (0):
                    clientVersion = glslang::EShTargetOpenGL_450;
            }
        }
        glslang::TShader shader{language};
        shader.setEnvInput(source, shader.getStage(), client, 460);
        shader.setEnvClient(client, clientVersion);
        shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_5);
        //All the above can be done in the create function.
        TBuiltInResource builtInResource{};
        EShMessages messages{};
        shader.parse(&builtInResource, 1, false, messages);
        #endif
    }

    void destroy() const {
        if (created.module) {
            #ifdef ILLUMINATION_ENGINE_VULKAN
            if (linkedRenderEngine->api.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
                vkDestroyShaderModule(linkedRenderEngine->device.device, shaderModule, nullptr);
            }
            #endif
        }
    }

    ~IEShader() {
        destroy();
    }

private:
    void create() {
        std::vector<std::string> extensions = getExtensions(createdWith.filename);
        if (std::count(extensions.begin(), extensions.end(), IE_RENDER_ENGINE_SHADER_EXTENSION_VERTEX) > 0) {
            #ifdef ILLUMINATION_ENGINE_VULKAN
            if (linkedRenderEngine->api.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
                shaderType = VK_SHADER_STAGE_VERTEX_BIT;
            }
            #endif
            if (linkedRenderEngine->api.name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
                shaderType = GL_VERTEX_SHADER;
            }
        }
        else if (std::count(extensions.begin(), extensions.end(), IE_RENDER_ENGINE_SHADER_EXTENSION_FRAGMENT) > 0) {
            #ifdef ILLUMINATION_ENGINE_VULKAN
            if (linkedRenderEngine->api.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
                shaderType = VK_SHADER_STAGE_FRAGMENT_BIT;
            }
            #endif
            if (linkedRenderEngine->api.name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
                shaderType = GL_FRAGMENT_SHADER;
            }
        }
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (linkedRenderEngine->api.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
            createdWith.filename += ".spv";
            std::ifstream shaderFile{createdWith.filename, std::ios::ate | std::ios::binary};
            uint32_t fileSize = shaderFile.tellg();
            data.resize(fileSize);
            shaderFile.seekg(0);
            shaderFile.read(reinterpret_cast<char *>(data.data()), fileSize);
            shaderFile.close();
            VkShaderModuleCreateInfo shaderModuleCreateInfo{
                    .sType=VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                    .codeSize=fileSize,
                    .pCode=data.data(),
            };
            if (vkCreateShaderModule(linkedRenderEngine->device.device, &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
                IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Failed to create shader module from shader file: " + createdWith.filename);
            }
            created.module = true;
        }
        #endif
    }
};