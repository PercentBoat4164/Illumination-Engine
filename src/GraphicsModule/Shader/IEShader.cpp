/* Include this file's header. */
#include "IEShader.hpp"

/* Include dependencies within this module. */
#include "IERenderEngine.hpp"


/* Include dependencies from Core. */
#include "Core/FileSystemModule/File.hpp"

void IEShader::setAPI(const IEAPI &API) {
    if (API.name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
        _create  = &IEShader::_openglCreate;
        _compile = &IEShader::_openglCompile;
    } else if (API.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
        _create  = &IEShader::_vulkanCreate;
        _compile = &IEShader::_vulkanCompile;
    }
}

void IEShader::destroy() {
    for (const std::function<void()> &function : deletionQueue) function();
    deletionQueue.clear();
}

std::function<void(IEShader &, IERenderEngine *, IE::Core::File *)> IEShader::_create = nullptr;

void IEShader::create(IERenderEngine *engineLink, IE::Core::File *shaderFile) {
    _create(*this, engineLink, shaderFile);
}

void IEShader::_openglCreate(IERenderEngine *renderEngineLink, IE::Core::File *shaderFile) {
    file               = shaderFile;
    linkedRenderEngine = renderEngineLink;
    GLenum shaderType  = GL_VERTEX_SHADER;
    if (file->extension == ".frag") shaderType = GL_FRAGMENT_SHADER;
    shaderID = glCreateShader(shaderType);
    compile(file->path.string(), file->path.string());
}

void IEShader::_vulkanCreate(IERenderEngine *renderEngineLink, IE::Core::File *shaderFile) {
    file               = shaderFile;
    linkedRenderEngine = renderEngineLink;
    std::string fileContents;
    if (file->extension == ".spv") {
        compile(file->path.string(), file->path.string() + ".spv");
        file = new IE::Core::File{file->path.string() + ".spv"};
    }
    fileContents = file->read();
    VkShaderModuleCreateInfo shaderModuleCreateInfo{
      .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = static_cast<size_t>(file->size),
      .pCode    = reinterpret_cast<const uint32_t *>(fileContents.data()),
    };
    VkResult result =
      vkCreateShaderModule(linkedRenderEngine->device.device, &shaderModuleCreateInfo, nullptr, &module);
    if (result != VK_SUCCESS) {
        linkedRenderEngine->settings->logger.log(

          "Failed to create shader module! Error: " + IERenderEngine::translateVkResultCodes(result),
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    } else {
        deletionQueue.emplace_back([&] {
            vkDestroyShaderModule(linkedRenderEngine->device.device, module, nullptr);
        });
    }
}

IEShader::~IEShader() {
    destroy();
}

std::function<void(IEShader &, const std::string &, std::string)> IEShader::_compile{nullptr};

void IEShader::compile(const std::string &input, const std::string &output) {
    _compile(*this, input, output);
}

void IEShader::_openglCompile(const std::string &input, std::string) {
    // Compile shader
    GLint         result = GL_FALSE;
    int           infoLogLength;
    std::string   contents{file->read()};
    const GLchar *shader = contents.data();
    glShaderSource(shaderID, 1, &shader, nullptr);
    glCompileShader(shaderID);
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &result);
    glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0) {
        std::vector<char> shaderErrorMessage(infoLogLength + 1);
        glGetShaderInfoLog(shaderID, infoLogLength, nullptr, &shaderErrorMessage[0]);
        printf("%s\n", &shaderErrorMessage[0]);
    }
}

void IEShader::_vulkanCompile(const std::string &input, std::string output) {
    if (output.empty()) output = input + ".spv";
    //	if (system((GLSLC + input + " -o " + output).c_str()) != 0) {
    //		throw std::runtime_error("failed to compile shaders: " + input);
    //	}
}
