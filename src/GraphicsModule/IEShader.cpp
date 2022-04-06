/* Include this file's header. */
#include "IEShader.hpp"

/* Include dependencies within this module. */
#include "IERenderEngine.hpp"


/* Include dependencies from Core. */
#include "Core/FileSystemModule/IEFile.hpp"

void IEShader::destroy() {
    for (const std::function<void()>& function : deletionQueue) {
        function();
    }
    deletionQueue.clear();
}

IEShader IEShader::create(IERenderEngine *renderEngineLink, IEFile *shaderFile) {
    file = shaderFile;
    linkedRenderEngine = renderEngineLink;
    std::vector<std::string> extensions = file->extensions();
    if (std::find(extensions.begin(), extensions.end(), "spv") == extensions.end()) {
        compile(file->path, file->path + ".spv");
    }
    compiled = true;
    file->open();
    std::string fileContents = file->read(file->length, 0);
    file->close();
    VkShaderModuleCreateInfo shaderModuleCreateInfo{
        .sType=VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize=static_cast<size_t>(file->length),
        .pCode=reinterpret_cast<const uint32_t *>(fileContents.data()),
    };
    VkResult result = vkCreateShaderModule(linkedRenderEngine->device.device, &shaderModuleCreateInfo, nullptr, &module);
    if (result != VK_SUCCESS) {
        linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Failed to create shader module! Error: " + IERenderEngine::translateVkResultCodes(result));
    }
    else {
        deletionQueue.emplace_back([&] {
            vkDestroyShaderModule(linkedRenderEngine->device.device, module, nullptr);
        });
    }
    return *this;
}

//void IEShader::create(IERenderEngine *renderEngineLink, IEShader::CreateInfo *createInfo) {
//    linkedRenderEngine = renderEngineLink;
//    createdWith = *createInfo;
//    std::string replaceWith = linkedRenderEngine->settings->rayTracing ? "RayTrace" : "Rasterize";
//    size_t pos = createdWith.filename.find('*');
//    while(pos != std::string::npos) {
//        createdWith.filename.replace(pos, 1, replaceWith);
//        pos = createdWith.filename.find('*', pos + replaceWith.size());
//    }
//    replaceWith.~basic_string();
//    data.clear();
//    //compile(createdWith.filename);
//    compiled = true;
//    std::string compiledFileName = createdWith.filename;// + ".spv";
//    std::ifstream compiledFile(compiledFileName, std::ios::ate | std::ios::binary);
//    if (!compiledFile.is_open()) { throw std::runtime_error("failed to open file: " + compiledFileName); }
//    long fileSize = compiledFile.tellg();
//    data.resize(fileSize);
//    compiledFile.seekg(0);
//    compiledFile.read(data.data(), fileSize);
//    compiledFile.close();
//    VkShaderModuleCreateInfo shaderModuleCreateInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
//    shaderModuleCreateInfo.codeSize = data.size();
//    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t *>(data.data());
//    if (vkCreateShaderModule(linkedRenderEngine->device.device, &shaderModuleCreateInfo, nullptr, &module) != VK_SUCCESS) { throw std::runtime_error("failed to create shader module!"); }
//    deletionQueue.emplace_back([&] {
//        if (module) {
//            vkDestroyShaderModule(linkedRenderEngine->device.device, module, nullptr);
//        }
//    });
//}

IEShader::~IEShader() {
    destroy();
}

void IEShader::compile(const std::string &input, std::string output) const {
    std::ifstream rawFile(input, std::ios::ate | std::ios::binary);
    if (!rawFile.is_open()) {
        throw std::runtime_error("failed to open file: " + input);
    }
    rawFile.close();
    if (output.empty()) { output = input + ".spv"; }
    if (linkedRenderEngine->settings->rayTracing) {
        if (system((GLSLC + input + " -o " + output + " --target-env=vulkan1.2").c_str()) != 0) {
            throw std::runtime_error("failed to compile shaders: " + input);
        }
    }
    else {
        if (system((GLSLC + input + " -o " + output).c_str()) != 0) {
            throw std::runtime_error("failed to compile shaders: " + input);
        }
    }
}
