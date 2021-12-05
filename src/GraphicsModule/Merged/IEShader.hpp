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

//Possible valid shader extensions - source
#define IE_RENDER_ENGINE_SHADER_EXTENSION_GLSL "glsl"
#define IE_RENDER_ENGINE_SHADER_EXTENSION_HLSL "hlsl"

//Possible valid shader extensions - language
#define IE_RENDER_ENGINE_SHADER_EXTENSION_VERTEX "vert"
#define IE_RENDER_ENGINE_SHADER_EXTENSION_FRAGMENT "frag"
#define IE_RENDER_ENGINE_SHADER_EXTENSION_TESSELLATION_CONTROL "tesc"
#define IE_RENDER_ENGINE_SHADER_EXTENSION_TESSELLATION_EVALUATION "tese"
#define IE_RENDER_ENGINE_SHADER_EXTENSION_GEOMETRY "geom"
#define IE_RENDER_ENGINE_SHADER_EXTENSION_COMPUTE "comp"
#define IE_RENDER_ENGINE_SHADER_EXTENSION_RAY_GENERATION "rgen"


extern std::vector<uint32_t> load_spirv_file();

class IEShader {
public:
    struct CreateInfo {
        //Required
        std::string filename{};
    };

    struct Created {
        bool compiled{};
        bool module{};
    };

    CreateInfo createdWith{};
    Created created{};
    IERenderEngineLink* linkedRenderEngine{};

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
        std::vector<std::string> extensions = getExtensions(createdWith.filename);
        EShLanguage language;
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
    }

    void destroy() {
    }

    ~IEShader() {
        destroy();
    }
};